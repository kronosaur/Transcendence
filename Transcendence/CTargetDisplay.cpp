//	CTargetDisplay.cpp
//
//	CTargetDisplay class

#include "PreComp.h"
#include "Transcendence.h"

#define DISPLAY_WIDTH						360
#define DISPLAY_HEIGHT						120

#define PRIMARY_WEAPON_X					300
#define PRIMARY_WEAPON_Y					80
#define MISSILE_WEAPON_X					300
#define MISSILE_WEAPON_Y					100

#define TARGET_IMAGE_X						60
#define TARGET_IMAGE_Y						60

#define TARGET_NAME_X						122
#define TARGET_NAME_Y						27
#define TARGET_NAME_COLOR					(CG32bitPixel(0,128,0))

#define TARGET_INFO_COLOR					(CG32bitPixel(0,255,0))

#define DEVICE_STATUS_HEIGHT				20

#define DEVICE_LABEL_COLOR					(CG32bitPixel(255,255,255))
#define DEVICE_LABEL_FAINT_COLOR			(CG32bitPixel(0,0,0))
#define DISABLED_LABEL_COLOR				CG32bitPixel(128,0,0)

#define STR_UNKNOWN_HOSTILE					CONSTLIT("Unknown Hostile")
#define STR_UNKNOWN_FRIENDLY				CONSTLIT("Unknown Friendly")

CTargetDisplay::CTargetDisplay (void) : m_pPlayer(NULL),
		m_bInvalid(true),
		m_pBackground(NULL)

//	CTargetDisplay constructor

	{
	}

CTargetDisplay::~CTargetDisplay (void)

//	CTargetDisplay destructor

	{
	CleanUp();
	}

void CTargetDisplay::CleanUp (void)

//	CleanUp
//
//	Frees up all resources

	{
	m_pPlayer = NULL;
	}

ALERROR CTargetDisplay::Init (CPlayerShipController *pPlayer, const RECT &rcRect)

//	Init
//
//	Initializes display

	{
	CleanUp();

	m_pPlayer = pPlayer;
	m_rcRect = rcRect;
	m_pBackground = g_pUniverse->GetLibraryBitmap(g_TargetingImageUNID, CDesignCollection::FLAG_IMAGE_LOCK);
	m_bInvalid = true;

	//	Create the off-screen buffer

	if (!m_Buffer.Create(DISPLAY_WIDTH, DISPLAY_HEIGHT, CG32bitImage::alpha8))
		return ERR_FAIL;

	//m_Buffer.SetBlending(200);
	//m_Buffer.SetTransparentColor();

	return NOERROR;
	}

void CTargetDisplay::Paint (CG32bitImage &Dest)

//	Paint
//
//	Paints to the screen

	{
	if (m_bInvalid)
		Update();

	Dest.Blt(0,
			0,
			RectWidth(m_rcRect),
			RectHeight(m_rcRect),
			200,
			m_Buffer,
			m_rcRect.left,
			m_rcRect.top);
	}

void CTargetDisplay::PaintDeviceStatus (CShip *pShip, DeviceNames iDev, int x, int y)

//	PaintDeviceStatus
//
//	Paints the status and ammo for a device

	{
	const CVisualPalette &VI = g_pHI->GetVisuals();
	CInstalledDevice *pDevice = pShip->GetNamedDevice(iDev);
	if (pDevice)
		{
		CDeviceClass *pClass = pDevice->GetClass();

		CString sVariant;
		int iAmmoLeft;
		pClass->GetSelectedVariantInfo(pShip, pDevice, &sVariant, &iAmmoLeft);
		CString sDevName = pClass->GetName();

		//	Paint the bonus

		int cxBonus = 0;

		CString sBonus = pDevice->GetEnhancedDesc(pShip);
		if (!sBonus.IsBlank())
			{
			int cyHeight;
			cxBonus = m_pFonts->Small.MeasureText(sBonus, &cyHeight);

			//	Background

			RECT rcRect;
			rcRect.left = x - cxBonus - 8;
			rcRect.right = rcRect.left + cxBonus;
			rcRect.top = y + (DEVICE_STATUS_HEIGHT - cyHeight) / 2;
			rcRect.bottom = rcRect.top + cyHeight;

			bool bDisadvantage = (*(sBonus.GetASCIIZPointer()) == '-');
			m_Buffer.Fill(rcRect.left - 2, 
					rcRect.top + 1, 
					cxBonus + 4, 
					cyHeight - 2, 
					(bDisadvantage ? VI.GetColor(colorAreaDisadvantage) : VI.GetColor(colorAreaAdvantage)));

			//	Bonus text

			m_pFonts->Small.DrawText(m_Buffer,
					rcRect.left,
					rcRect.top,
					(bDisadvantage ? VI.GetColor(colorTextDisadvantage) : VI.GetColor(colorTextAdvantage)),
					sBonus);

			cxBonus += 4;
			}

		//	Figure out what color to use

		CG32bitPixel rgbColor;
		if (pDevice->IsEnabled() && !pDevice->IsDamaged() && !pDevice->IsDisrupted())
			rgbColor = m_pFonts->rgbTitleColor;
		else
			rgbColor = DISABLED_LABEL_COLOR;

		//	Paint the name

		if (sVariant.IsBlank())
			sVariant = sDevName;

		int cyHeight;
		int cxWidth = m_pFonts->Medium.MeasureText(sVariant, &cyHeight);

		m_pFonts->Medium.DrawText(m_Buffer,
				x - cxWidth - 8 - cxBonus,
				y + (DEVICE_STATUS_HEIGHT - cyHeight) / 2,
				rgbColor,
				sVariant);

		//	Paint the ammo counter

		if (iAmmoLeft != -1)
			{
			CString sAmmo = strFromInt(iAmmoLeft);
			m_pFonts->LargeBold.DrawText(m_Buffer,
					x,
					y,
					m_pFonts->rgbTitleColor,
					sAmmo);
			}
		}
	}

void CTargetDisplay::Update (void)

//	Update
//
//	Updates buffer

	{
	DEBUG_TRY

	//	Nothing to do if no player

	if (m_pPlayer == NULL)
		return;

	CShip *pShip = m_pPlayer->GetShip();
	const CPlayerSettings *pSettings = pShip->GetClass()->GetPlayerSettings();
	const SWeaponImageDesc *pDisplayDesc = pSettings->GetWeaponDesc();

	//	Erase

	m_Buffer.Set(CG32bitPixel::Null());

	//	Paint the buffer with the appropriate background bitmap

	if (pDisplayDesc)
		{
		const RECT &rcImage = pDisplayDesc->Image.GetImageRect();

		m_Buffer.Blt(rcImage.left, 
				rcImage.top, 
				RectWidth(rcImage), 
				RectHeight(rcImage), 
				255,
				pDisplayDesc->Image.GetImage(NULL_STR),
				0,
				0);
		}
	else if (m_pBackground)
		m_Buffer.Blt(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, *m_pBackground, 0, 0);

	//	Draw the primary weapon status

	PaintDeviceStatus(m_pPlayer->GetShip(), 
			devPrimaryWeapon,
			PRIMARY_WEAPON_X,
			PRIMARY_WEAPON_Y);

	//	Draw the missile weapon status

	PaintDeviceStatus(m_pPlayer->GetShip(),
			devMissileWeapon,
			MISSILE_WEAPON_X,
			MISSILE_WEAPON_Y);

	//	Paint the target

	CSpaceObject *pTarget = m_pPlayer->GetSelectedTarget();
	if (pTarget)
		{
		//	Paint image

		if (pTarget->IsIdentified())
			{
			SViewportPaintCtx Ctx;
			Ctx.pObj = pTarget;
			Ctx.XForm = ViewportTransform(pTarget->GetPos(), 
					g_KlicksPerPixel, 
					TARGET_IMAGE_X, 
					TARGET_IMAGE_Y);
			Ctx.XFormRel = Ctx.XForm;
			Ctx.fNoRecon = true;
			Ctx.fNoDockedShips = true;
			Ctx.fNoSelection = true;

			pTarget->Paint(m_Buffer, TARGET_IMAGE_X, TARGET_IMAGE_Y, Ctx);

			//	Erase the area outside the target pane

			if (pDisplayDesc)
				{
				const RECT &rcImage = pDisplayDesc->Image.GetImageRect();

				m_Buffer.CopyChannel(channelAlpha,
						rcImage.left, 
						rcImage.top, 
						RectWidth(rcImage), 
						RectHeight(rcImage), 
						pDisplayDesc->Image.GetImage(NULL_STR),
						0,
						0);
				}
			else if (m_pBackground)
				{
				m_Buffer.CopyChannel(channelAlpha,
						0, 
						0, 
						DISPLAY_WIDTH, 
						DISPLAY_HEIGHT, 
						*m_pBackground,
						0,
						0);
				}
			}

		//	Paint the name of the target

		int x = TARGET_NAME_X;
		int y = TARGET_NAME_Y;

		CString sName;
		if (pTarget->IsIdentified())
			sName = pTarget->GetNounPhrase(nounCapitalize);
		else if (m_pPlayer->GetShip()->IsEnemy(pTarget))
			sName = STR_UNKNOWN_HOSTILE;
		else
			sName = STR_UNKNOWN_FRIENDLY;

		m_pFonts->MediumHeavyBold.DrawText(m_Buffer,
				x,
				y,
				CG32bitPixel::Darken(m_pFonts->rgbAltGreenColor, 180),
				sName);
		y += m_pFonts->MediumHeavyBold.GetHeight();

		//	Paint the range

		CVector vDist = pTarget->GetPos() - m_pPlayer->GetShip()->GetPos();
		int iDist = (int)((vDist.Length() / LIGHT_SECOND) + 0.5);
		CString sStatus = strPatternSubst(CONSTLIT("Range: %d"), iDist);
		m_pFonts->Medium.DrawText(m_Buffer,
				x,
				y,
				m_pFonts->rgbAltGreenColor,
				sStatus);
		y += m_pFonts->Medium.GetHeight();

		//	Paint the damage

		if (pTarget->IsIdentified())
			{
			int iDamage = pTarget->GetVisibleDamage();
			int iShields = pTarget->GetShieldLevel();
			if (iDamage > 0 || iShields > 0)
				{
				CString sDamage;
				
				if (iDamage == 0 && iShields > 0)
					sDamage = strPatternSubst(CONSTLIT("Shields: %d%%"), iShields);
				else if (iDamage > 0 && iShields <= 0)
					sDamage = strPatternSubst(CONSTLIT("Damage: %d%%"), iDamage);
				else if (iDamage > 0)
					sDamage = strPatternSubst(CONSTLIT("Damage: %d%%  Shields: %d%%"), iDamage, iShields);

				m_pFonts->Medium.DrawText(m_Buffer,
						x,
						y,
						m_pFonts->rgbAltGreenColor,
						sDamage);
				}
			}
		}

	m_bInvalid = false;

	DEBUG_CATCH
	}

