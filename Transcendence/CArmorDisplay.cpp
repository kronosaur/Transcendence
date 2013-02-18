//	CArmorDisplay.cpp
//
//	CArmorDisplay class

#include "PreComp.h"
#include "Transcendence.h"

#define ARMOR_SECTION_COUNT				4

#define DISPLAY_WIDTH					360
#define DISPLAY_HEIGHT					136

#define SHIELD_IMAGE_WIDTH				136
#define SHIELD_IMAGE_HEIGHT				136

#define DESCRIPTION_WIDTH				(DISPLAY_WIDTH - SHIELD_IMAGE_WIDTH)

#define HP_DISPLAY_WIDTH				26
#define HP_DISPLAY_HEIGHT				14

#define HP_DISPLAY_BACK_COLOR			CG16bitImage::RGBValue(0,23,167)
#define HP_DISPLAY_TEXT_COLOR			CG16bitImage::RGBValue(150,180,255)

#define SHIELD_HP_DISPLAY_X				DESCRIPTION_WIDTH
#define SHIELD_HP_DISPLAY_Y				(DISPLAY_HEIGHT - 16)
#define SHIELD_HP_DISPLAY_WIDTH			26
#define SHIELD_HP_DISPLAY_HEIGHT		14
#define SHIELD_HP_DISPLAY_BACK_COLOR	CG16bitImage::RGBValue(0,117,16)
#define SHIELD_HP_DISPLAY_TEXT_COLOR	CG16bitImage::RGBValue(150,255,180)
#define SHIELD_HP_DISPLAY_LINE_COLOR	CG16bitImage::RGBValue(0,117,16)
#define DISABLED_TEXT_COLOR				CG16bitImage::RGBValue(128,0,0)

#define ARMOR_NAME_COLOR				CG16bitImage::RGBValue(150,180,255)
#define ARMOR_LINE_COLOR				CG16bitImage::RGBValue(0,23,167)
#define ARMOR_ENHANCE_X					224
#define ARMOR_ENHANCE_BACK_COLOR		CG16bitImage::RGBValue(0,23,167)
#define ARMOR_ENHANCE_TEXT_COLOR		CG16bitImage::RGBValue(150,180,255)
#define ARMOR_DAMAGED_BACK_COLOR		CG16bitImage::RGBValue(167,23,0)
#define ARMOR_DAMAGED_TEXT_COLOR		CG16bitImage::RGBValue(150,180,255)

#define RGB_NAME_BACKGROUND				CG16bitImage::RGBValue(16,16,16)

CArmorDisplay::CArmorDisplay (void) : m_pUniverse(NULL),
		m_pPlayer(NULL),
		m_iSelection(-1),
		m_dwCachedShipID(0),
		m_pShieldPainter(NULL)

//	CArmorDisplay constructor

	{
	}

CArmorDisplay::~CArmorDisplay (void)

//	CArmorDisplay destructor

	{
	CleanUp();
	}

void CArmorDisplay::CleanUp (void)

//	CleanUp
//
//	Delete relevant stuff

	{
	if (m_pShieldPainter)
		{
		m_pShieldPainter->Delete();
		m_pShieldPainter = NULL;
		}

	m_pPlayer = NULL;
	}

ALERROR CArmorDisplay::Init (CPlayerShipController *pPlayer, const RECT &rcRect)

//	Init
//
//	Initialize

	{
	ALERROR error;

	CleanUp();

	m_pUniverse = pPlayer->GetShip()->GetUniverse();
	m_pPlayer = pPlayer;
	m_rcRect = rcRect;

	//	Create the off-screen buffer

	if (error = m_Buffer.CreateBlank(DISPLAY_WIDTH, DISPLAY_HEIGHT, false, DEFAULT_TRANSPARENT_COLOR))
		return error;

	m_Buffer.SetTransparentColor();

	return NOERROR;
	}

void CArmorDisplay::Paint (CG16bitImage &Dest)

//	Paint
//
//	Paints to the destination

	{
	int i;

	Dest.ColorTransBlt(0,
			0,
			RectWidth(m_rcRect),
			RectHeight(m_rcRect),
			255,
			m_Buffer,
			m_rcRect.left,
			m_rcRect.top);

	//	We paint some text on top so that we get the proper antialiasing

	for (i = 0; i < m_Text.GetCount(); i++)
		{
		STextPaint *pPaint = &m_Text[i];
		pPaint->pFont->DrawText(Dest,
				m_rcRect.left + pPaint->x,
				m_rcRect.top + pPaint->y,
				pPaint->wColor,
				pPaint->sText);
		}
	}

void CArmorDisplay::SetSelection (int iSelection)

//	SetSelection
//
//	Selects an armor segment

	{
	if (iSelection != m_iSelection)
		{
		m_iSelection = iSelection;
		Update();
		}
	}

void CArmorDisplay::Update (void)

//	Update
//
//	Updates buffer from data

	{
	int i;

	if (m_pPlayer == NULL)
		return;

	CShip *pShip = m_pPlayer->GetShip();
	const CPlayerSettings *pSettings = pShip->GetClass()->GetPlayerSettings();
	CItemListManipulator ItemList(pShip->GetItemList());
	const CG16bitFont &SmallFont = m_pPlayer->GetTrans()->GetFonts().Small;
	m_Text.DeleteAll();

	//	If we've changed ships then we need to delete the painters

	if (m_dwCachedShipID != pShip->GetID())
		{
		if (m_pShieldPainter)
			{
			m_pShieldPainter->Delete();
			m_pShieldPainter = NULL;
			}

		m_dwCachedShipID = pShip->GetID();
		}

	//	Erase everything

	m_Buffer.Fill(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, DEFAULT_TRANSPARENT_COLOR);

	//	Figure out the status of the shields

	int iHP = 0;
	int iMaxHP = 10;
	CInstalledDevice *pShield = pShip->GetNamedDevice(devShields);
	if (pShield)
		pShield->GetStatus(pShip, &iHP, &iMaxHP);

	//	Draw the base ship image, if we have it

	const SArmorImageDesc &ArmorDesc = pSettings->GetArmorDesc();
	if (!ArmorDesc.ShipImage.IsEmpty())
		{
		const RECT &rcShip = ArmorDesc.ShipImage.GetImageRect();

		m_Buffer.ColorTransBlt(rcShip.left, 
				rcShip.top, 
				RectWidth(rcShip), 
				RectHeight(rcShip), 
				255,
				ArmorDesc.ShipImage.GetImage(NULL_STR), 
				DESCRIPTION_WIDTH + ((SHIELD_IMAGE_WIDTH - RectWidth(rcShip)) / 2),
				(SHIELD_IMAGE_HEIGHT - RectHeight(rcShip)) / 2);
		}

	//	Draw the old-style shields

	const SShieldImageDesc &ShieldDesc = pSettings->GetShieldDesc();
	if (!ShieldDesc.pShieldEffect)
		{
		int iWhole = (iMaxHP > 0 ? (iHP * 100) / iMaxHP : 100);
		int iIndex = (100 - iWhole) / 20;

		const RECT &rcShield = ShieldDesc.Image.GetImageRect();
		m_Buffer.ColorTransBlt(rcShield.left, 
				rcShield.top + (RectHeight(rcShield) * iIndex), 
				RectWidth(rcShield), 
				RectHeight(rcShield), 
				255,
				ShieldDesc.Image.GetImage(NULL_STR), 
				DESCRIPTION_WIDTH + ((SHIELD_IMAGE_WIDTH - RectWidth(rcShield)) / 2),
				(SHIELD_IMAGE_HEIGHT - RectHeight(rcShield)) / 2);
		}

	if (pShield)
		{
		m_Buffer.Fill(SHIELD_HP_DISPLAY_X,
				SHIELD_HP_DISPLAY_Y, 
				SHIELD_HP_DISPLAY_WIDTH, 
				SHIELD_HP_DISPLAY_HEIGHT,
				SHIELD_HP_DISPLAY_BACK_COLOR);
		
		CString sHP = strFromInt(iHP);
		int cxWidth = m_pFonts->Medium.MeasureText(sHP, NULL);
		m_pFonts->Medium.DrawText(m_Buffer,
				SHIELD_HP_DISPLAY_X + (SHIELD_HP_DISPLAY_WIDTH - cxWidth) / 2,
				SHIELD_HP_DISPLAY_Y - 1,
				CG16bitImage::LightenPixel(m_pFonts->wAltGreenColor, 60),
				sHP);

		DrawBrokenLine(m_Buffer,
				0,
				SHIELD_HP_DISPLAY_Y,
				SHIELD_HP_DISPLAY_X,
				SHIELD_HP_DISPLAY_Y,
				0,
				SHIELD_HP_DISPLAY_LINE_COLOR);

		WORD wColor;
		if (pShield->IsEnabled() && !pShield->IsDamaged() && !pShield->IsDisrupted())
			wColor = m_pFonts->wAltGreenColor;
		else
			wColor = DISABLED_TEXT_COLOR;

		CString sShieldName = pShield->GetClass()->GetName();
		int cyHeight;
		cxWidth = m_pFonts->Medium.MeasureText(sShieldName, &cyHeight);

		//	Add the shield name to list of text to paint

		STextPaint *pPaint = m_Text.Insert();
		pPaint->sText = sShieldName;
		pPaint->x = 0;
		pPaint->y = SHIELD_HP_DISPLAY_Y;
		pPaint->pFont = &m_pFonts->Medium;
		pPaint->wColor = wColor;

		//	Paint the modifiers

		if (pShield->GetMods().IsNotEmpty() || pShield->GetBonus() != 0)
			{
			pShip->SetCursorAtNamedDevice(ItemList, devShields);
			CString sMods = pShield->GetEnhancedDesc(pShip, &ItemList.GetItemAtCursor());
			if (!sMods.IsBlank())
				{
				bool bDisadvantage = (*(sMods.GetASCIIZPointer()) == '-');

				int cx = SmallFont.MeasureText(sMods);
				m_Buffer.Fill(SHIELD_HP_DISPLAY_X - cx - 8,
						SHIELD_HP_DISPLAY_Y,
						cx + 8,
						SHIELD_HP_DISPLAY_HEIGHT,
						(bDisadvantage ? ARMOR_DAMAGED_BACK_COLOR : ARMOR_ENHANCE_BACK_COLOR));

				SmallFont.DrawText(m_Buffer,
						SHIELD_HP_DISPLAY_X - cx - 4,
						SHIELD_HP_DISPLAY_Y + (SHIELD_HP_DISPLAY_HEIGHT - SmallFont.GetHeight()) / 2,
						(bDisadvantage ? ARMOR_DAMAGED_TEXT_COLOR : ARMOR_ENHANCE_TEXT_COLOR),
						sMods);
				}
			}
		}

	//	Draw armor

	int iArmorCount = Min(pShip->GetArmorSectionCount(), pSettings->GetArmorDescCount());
	for (i = 0; i < iArmorCount; i++)
		{
		const SArmorSegmentImageDesc *pImage = &pSettings->GetArmorDesc(i);

		CInstalledArmor *pArmor = pShip->GetArmorSection(i);
		int iMaxHP = pArmor->GetMaxHP(pShip);
		int iWhole = (iMaxHP == 0 ? 100 : (pArmor->GetHitPoints() * 100) / iMaxHP);
		int iIndex = (100 - iWhole) / 20;
		
		if (iIndex < 5)
			{
			const RECT &rcImage = pImage->Image.GetImageRect();
			m_Buffer.ColorTransBlt(rcImage.left,
					rcImage.top + iIndex * RectHeight(rcImage),
					RectWidth(rcImage),
					RectHeight(rcImage),
					255,
					pImage->Image.GetImage(NULL_STR),
					DESCRIPTION_WIDTH + pImage->xDest,
					pImage->yDest);
			}
		}

	//	Draw the new style shields on top

	if (ShieldDesc.pShieldEffect)
		{
		int x = DESCRIPTION_WIDTH + SHIELD_IMAGE_WIDTH / 2;
		int y = SHIELD_IMAGE_HEIGHT / 2;

		SViewportPaintCtx Ctx;
		Ctx.iTick = g_pUniverse->GetTicks();
		Ctx.iVariant = (iMaxHP > 0 ? (iHP * 100) / iMaxHP : 0);
		Ctx.iDestiny = pShip->GetDestiny();
		Ctx.iRotation = 90;

		if (m_pShieldPainter == NULL)
			m_pShieldPainter = ShieldDesc.pShieldEffect->CreatePainter();

		m_pShieldPainter->Paint(m_Buffer, x, y, Ctx);
		}

	//	Draw armor names

	for (i = 0; i < iArmorCount; i++)
		{
		const SArmorSegmentImageDesc *pImage = &pSettings->GetArmorDesc(i);
		CInstalledArmor *pArmor = pShip->GetArmorSection(i);

		//	Paint the HPs

		if (i == m_iSelection)
			{
			m_Buffer.Fill(DESCRIPTION_WIDTH + pImage->xHP - 1, 
					pImage->yHP - 1, 
					HP_DISPLAY_WIDTH + 2, 
					HP_DISPLAY_HEIGHT + 2,
					CG16bitImage::DarkenPixel(m_pFonts->wSelectBackground, 128));
			}
		else
			{
			m_Buffer.Fill(DESCRIPTION_WIDTH + pImage->xHP, 
					pImage->yHP, 
					HP_DISPLAY_WIDTH, 
					HP_DISPLAY_HEIGHT,
					HP_DISPLAY_BACK_COLOR);
			}

		CString sHP = strFromInt(pArmor->GetHitPoints());
		int cxWidth = m_pFonts->Medium.MeasureText(sHP, NULL);
		m_pFonts->Medium.DrawText(m_Buffer,
				DESCRIPTION_WIDTH + pImage->xHP + (HP_DISPLAY_WIDTH - cxWidth) / 2,
				pImage->yHP - 1,
				m_pFonts->wTitleColor,
				sHP);

		//	Paint the armor name line

		DrawBrokenLine(m_Buffer,
				0,
				pImage->yName + m_pFonts->Medium.GetHeight(),
				DESCRIPTION_WIDTH + pImage->xHP + pImage->xNameDestOffset,
				pImage->yHP + pImage->yNameDestOffset,
				pImage->cxNameBreak,
				(i == m_iSelection ? CG16bitImage::DarkenPixel(m_pFonts->wSelectBackground, 128) : ARMOR_LINE_COLOR));

		//	Paint the armor names

		CString sName = pArmor->GetClass()->GetShortName();
		int cy;
		int cx = m_pFonts->Medium.MeasureText(sName, &cy) + 4;
		if (i == m_iSelection)
			{
			m_Buffer.Fill(0, 
					pImage->yName, 
					cx, 
					cy,
					CG16bitImage::DarkenPixel(m_pFonts->wSelectBackground, 128));
			}

		STextPaint *pPaint = m_Text.Insert();
		pPaint->sText = sName;
		pPaint->x = 2;
		pPaint->y = pImage->yName;
		pPaint->pFont = &m_pFonts->Medium;
		pPaint->wColor = m_pFonts->wTitleColor;

		//	Paint the modifiers

		if (pArmor->GetMods().IsNotEmpty())
			{
			pShip->SetCursorAtArmor(ItemList, i);
			CString sMods = ItemList.GetItemAtCursor().GetEnhancedDesc(pShip);
			if (!sMods.IsBlank())
				{
				int cx = SmallFont.MeasureText(sMods);
				m_Buffer.Fill(ARMOR_ENHANCE_X - cx - 4,
						pImage->yName + m_pFonts->Medium.GetHeight() - HP_DISPLAY_HEIGHT,
						cx + 8,
						HP_DISPLAY_HEIGHT,
						ARMOR_ENHANCE_BACK_COLOR);

				SmallFont.DrawText(m_Buffer,
						ARMOR_ENHANCE_X - cx,
						pImage->yName + 3,
						ARMOR_ENHANCE_TEXT_COLOR,
						sMods);
				}
			}
		}
	}
