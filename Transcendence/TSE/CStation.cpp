//	CStation.cpp
//
//	CStation class

#include "PreComp.h"

#ifdef DEBUG
//#define DEBUG_DOCKING
//#define DEBUG_ALERTS
#endif

#define ITEM_TAG								CONSTLIT("Item")
#define INITIAL_DATA_TAG						CONSTLIT("InitialData")
#define DEVICES_TAG								CONSTLIT("Devices")
#define ITEMS_TAG								CONSTLIT("Items")

#define COUNT_ATTRIB							CONSTLIT("count")
#define ITEM_ATTRIB								CONSTLIT("item")
#define DEVICE_ID_ATTRIB						CONSTLIT("deviceID")
#define X_ATTRIB								CONSTLIT("x")
#define Y_ATTRIB								CONSTLIT("y")
#define NO_MAP_LABEL_ATTRIB						CONSTLIT("noMapLabel")
#define SHIPWRECK_UNID_ATTRIB					CONSTLIT("shipwreckID")
#define NAME_ATTRIB								CONSTLIT("name")

#define PROPERTY_ABANDONED						CONSTLIT("abandoned")
#define PROPERTY_HP								CONSTLIT("hp")
#define PROPERTY_IMMUTABLE						CONSTLIT("immutable")
#define PROPERTY_INSTALL_DEVICE_MAX_LEVEL		CONSTLIT("installDeviceMaxLevel")
#define PROPERTY_MAX_HP							CONSTLIT("maxHP")
#define PROPERTY_MAX_STRUCTURAL_HP				CONSTLIT("maxStructuralHP")
#define PROPERTY_ORBIT							CONSTLIT("orbit")
#define PROPERTY_PLAYER_BACKLISTED				CONSTLIT("playerBlacklisted")
#define PROPERTY_REPAIR_ARMOR_MAX_LEVEL			CONSTLIT("repairArmorMaxLevel")
#define PROPERTY_STRUCTURAL_HP					CONSTLIT("structuralHP")

#define STR_TRUE								CONSTLIT("true")

const int TRADE_UPDATE_FREQUENCY =		1801;			//	Interval for checking trade
const int STATION_SCAN_TARGET_FREQUENCY	= 29;
const int STATION_ATTACK_FREQUENCY =	67;
const int STATION_REINFORCEMENT_FREQUENCY =	607;
const int STATION_TARGET_FREQUENCY =	503;
const int DAYS_TO_REFRESH_INVENTORY =	5;
const int INVENTORY_REFRESHED_PER_UPDATE = 5;			//	% of inventory refreshed on each update frequency

#define MAX_ATTACK_DISTANCE				(LIGHT_SECOND * 25)
#define BEACON_RANGE					(LIGHT_SECOND * 20)
#define MAX_SUBORDINATES				12
#define BLACKLIST_HIT_LIMIT				3
#define BLACKLIST_DECAY_RATE			150

#define MIN_ANGER						300
#define MAX_ANGER						1800
#define ANGER_INC						30

const WORD RGB_SIGN_COLOR =				CG16bitImage::RGBValue(196, 223, 155);
const COLORREF RGB_ORBIT_LINE =			RGB(115, 149, 229);
const WORD RGB_MAP_LABEL =				CG16bitImage::RGBValue(255, 217, 128);

static CObjectClass<CStation>g_Class(OBJID_CSTATION);

static char g_ImageTag[] = "Image";
static char g_ShipsTag[] = "Ships";
static char g_DockScreensTag[] = "DockScreens";
static char g_ShipTag[] = "Ship";

static char g_DockScreenAttrib[] = "dockScreen";
static char g_AbandonedScreenAttrib[] = "abandonedScreen";
static char g_HitPointsAttrib[] = "hitPoints";
static char g_ArmorIDAttrib[] = "armorID";
static char g_ProbabilityAttrib[] = "probability";
static char g_TableAttrib[] = "table";

#define MIN_DOCK_APPROACH_SPEED			(g_KlicksPerPixel * 25.0 / g_TimeScale);
#define MAX_DOCK_APPROACH_SPEED			(g_KlicksPerPixel * 50.0 / g_TimeScale);
#define MAX_DOCK_TANGENT_SPEED			(g_KlicksPerPixel / g_TimeScale);
const Metric g_DockBeamStrength =		1000.0;
const Metric g_DockBeamTangentStrength = 250.0;

const int g_iMapScale = 5;

CStation::CStation (void) : CSpaceObject(&g_Class),
		m_fArmed(false),
		m_dwSpare(0),
		m_pType(NULL),
		m_pMapOrbit(NULL),
		m_pArmorClass(NULL),
		m_pTarget(NULL),
		m_pBase(NULL),
		m_pDevices(NULL),
		m_iAngryCounter(0),
		m_iReinforceRequestCount(0),
		m_pMoney(NULL),
		m_pTrade(NULL)

//	CStation constructor

	{
	}

CStation::~CStation (void)

//	CStation destructor

	{
	if (m_pMapOrbit)
		delete m_pMapOrbit;

	if (m_pDevices)
		delete [] m_pDevices;

	if (m_pMoney)
		delete m_pMoney;

	if (m_pTrade)
		delete m_pTrade;
	}

void CStation::AddBuyOrder (CItemType *pType, const CString &sCriteria, int iPriceAdj)

//	AddBuyOrder
//
//	Adds an override buy order

	{
	AllocTradeOverride();
	m_pTrade->AddBuyOrder(pType, sCriteria, iPriceAdj);
	}

void CStation::AddOverlay (CEnergyFieldType *pType, int iPosAngle, int iPosRadius, int iRotation, int iLifeLeft, DWORD *retdwID)

//	AddOverlay
//
//	Adds an overlay to the ship

	{
	m_Overlays.AddField(this, pType, iPosAngle, iPosRadius, iRotation, iLifeLeft, retdwID);

#if 0
	//	Recalc bonuses, etc.

	CalcArmorBonus();
	CalcDeviceBonus();
	m_pController->OnWeaponStatusChanged();
	m_pController->OnArmorRepaired(-1);
#endif
	}

void CStation::AddSellOrder (CItemType *pType, const CString &sCriteria, int iPriceAdj)

//	AddSellOrder
//
//	Adds an override sell order

	{
	AllocTradeOverride();
	m_pTrade->AddSellOrder(pType, sCriteria, iPriceAdj);
	}

void CStation::AddSubordinate (CSpaceObject *pSubordinate)

//	AddSubordinate
//
//	Add this object to our list of subordinates

	{
	m_Subordinates.Add(pSubordinate);

	//	HACK: If we're adding a station, set it to point back to us

	CStation *pSatellite = pSubordinate->AsStation();
	if (pSatellite)
		pSatellite->SetBase(this);
	}

void CStation::AllocTradeOverride (void)

//	AllocTradeOverride
//
//	Makes sure that we have the m_pTrade structure allocated.
//	This is an override of the trade desc in the type

	{
	if (m_pTrade == NULL)
		{
		m_pTrade = new CTradingDesc;

		//	Set the same economy type

		CTradingDesc *pBaseTrade = m_pType->GetTradingDesc();
		if (pBaseTrade)
			{
			m_pTrade->SetEconomyType(pBaseTrade->GetEconomyType());
			m_pTrade->SetMaxCurrency(pBaseTrade->GetMaxCurrency());
			m_pTrade->SetReplenishCurrency(pBaseTrade->GetReplenishCurrency());
			}
		}
	}

void CStation::Blacklist (CSpaceObject *pObj)

//	Blacklist
//
//	pObj is blacklisted (this only works for the player)

	{
	if (pObj && !pObj->IsPlayer())
		return;

	//	No need if we don't support blacklist

	if (!m_pType->IsBlacklistEnabled())
		return;

	//	Remember if we need to send out an event

	bool bFireEvent = !m_Blacklist.IsBlacklisted();

	//	Remember that player is blacklisted
	//	(We do this early in case we recurse)

	m_Blacklist.Blacklist();

	//	Tell our base to attack

	if (pObj)
		{
		if (m_pBase)
			Communicate(m_pBase, msgAttack, pObj);

		//	Send all our subordinates to attack

		for (int i = 0; i < m_Subordinates.GetCount(); i++)
			Communicate(m_Subordinates.GetObj(i), msgAttackDeter, pObj);
		}

	//	Fire event (we do this at the end because the event could reverse it).

	if (bFireEvent)
		{
		FireOnPlayerBlacklisted();
		}
	}

int CStation::CalcNumberOfShips (void)

//	CalcNumberOfShips
//
//	Returns the number of ships associated with this station

	{
	int i;
	int iCount = 0;

	for (i = 0; i < GetSystem()->GetObjectCount(); i++)
		{
		CSpaceObject *pObj = GetSystem()->GetObject(i);

		if (pObj
				&& pObj->GetBase() == this
				&& pObj->GetCategory() == catShip
				&& !pObj->IsInactive()
				&& pObj != this)
			iCount++;
		}

	return iCount;
	}

bool CStation::CanAttack (void) const

//	CanAttack
//
//	TRUE if the object can attack

	{
	return (!IsAbandoned() 
			&& (m_fArmed 
				|| (m_Subordinates.GetCount() > 0)
				|| m_pType->CanAttack()));
	}

bool CStation::CanBlock (CSpaceObject *pObj)

//	CanBlock
//
//	Returns TRUE if this object can block the given object

	{
	return (m_pType->IsWall() 
			|| (pObj->GetCategory() == catStation && pObj->IsMobile()));
	}

CurrencyValue CStation::ChargeMoney (DWORD dwEconomyUNID, CurrencyValue iValue)

//	ChargeMoney
//
//	Charge the amount out of the station's balance
//	Returns the remaining balance (or -1 if there is not enough)

	{
	if (dwEconomyUNID == 0)
		dwEconomyUNID = GetDefaultEconomyUNID();

	if (m_pMoney && m_pMoney->GetCredits(dwEconomyUNID) >= iValue && iValue >= 0)
		return m_pMoney->IncCredits(dwEconomyUNID, -iValue);
	else
		return -1;
	}

bool CStation::ClassCanAttack (void)

//	ClassCanAttack
//
//	Only returns FALSE if this object can never attack

	{
	return (m_pType->CanAttack());
	}

void CStation::ClearBlacklist (CSpaceObject *pObj)

//	ClearBlacklist
//
//	Removes blacklist

	{
	if (pObj && !pObj->IsPlayer())
		return;

	//	No need if we don't support blacklist

	if (!m_pType->IsBlacklistEnabled())
		return;

	//	Remember that player is not blacklisted
	//	(We do this early in case we recurse)

	m_Blacklist.ClearBlacklist();

	if (pObj)
		{
		//	Remove object from target

		m_Targets.Remove(pObj);

		//	Send all our subordinates to cancel attack

		for (int i = 0; i < m_Subordinates.GetCount(); i++)
			Communicate(m_Subordinates.GetObj(i), msgAbort, pObj);
		}
	}

CurrencyValue CStation::CreditMoney (DWORD dwEconomyUNID, CurrencyValue iValue)

//	CreditMoney
//
//	Credits the amount to the station's balance. Return the new balance.

	{
	if (dwEconomyUNID == 0)
		dwEconomyUNID = GetDefaultEconomyUNID();

	if (m_pMoney == NULL)
		{
		if (iValue > 0)
			{
			m_pMoney = new CCurrencyBlock;
			m_pMoney->SetCredits(dwEconomyUNID, iValue);
			return iValue;
			}
		else
			return 0;
		}
	else
		{
		if (iValue > 0)
			return m_pMoney->IncCredits(dwEconomyUNID, iValue);
		else
			return m_pMoney->GetCredits(dwEconomyUNID);
		}
	}

void CStation::CreateDestructionEffect (void)

//	CreateDestructionEffect
//
//	Create the effect when the station is destroyed

	{
	//	Start destruction animation

	m_iDestroyedAnimation = 60;

	//	Explosion effect and damage

	SExplosionType Explosion;
	FireGetExplosionType(&Explosion);
	if (Explosion.pDesc == NULL)
		Explosion.pDesc = m_pType->GetExplosionType();

	if (Explosion.pDesc)
		{
		CItemEnhancementStack *pEnhancements = NULL;
		if (Explosion.iBonus != 0)
			{
			pEnhancements = new CItemEnhancementStack;
			pEnhancements->InsertHPBonus(Explosion.iBonus);
			}

		GetSystem()->CreateWeaponFire(Explosion.pDesc,
				pEnhancements,
				Explosion.iCause,
				CDamageSource(this, Explosion.iCause),
				GetPos(),
				GetVel(),
				0,
				NULL,
				CSystem::CWF_EXPLOSION,
				NULL);

		if (pEnhancements)
			pEnhancements->Delete();
		}

	//	Some air leaks

	CParticleEffect *pEffect;
	CParticleEffect::CreateEmpty(GetSystem(),
			GetPos(),
			GetVel(),
			&pEffect);

	int iAirLeaks = mathRandom(0, 5);
	for (int j = 0; j < iAirLeaks; j++)
		{
		CParticleEffect::SParticleType *pType = new CParticleEffect::SParticleType;
		pType->m_fWake = true;

		pType->m_fLifespan = true;
		pType->iLifespan = 30;

		pType->m_fRegenerate = true;
		pType->iRegenerationTimer = 300 + mathRandom(0, 200);

		pType->iDirection = mathRandom(0, 359);
		pType->iDirRange = 3;
		pType->rAveSpeed = 0.1 * LIGHT_SPEED;

		pType->m_fMaxRadius = false;
		pType->rRadius = pType->iLifespan * pType->rAveSpeed;
		pType->rDampening = 0.75;

		pType->iPaintStyle = CParticleEffect::paintSmoke;

		pEffect->AddGroup(pType, mathRandom(50, 150));
		}

	//	Sound effects

	g_pUniverse->PlaySound(this, g_pUniverse->FindSound(g_StationExplosionSoundUNID));
	}

void CStation::CreateEjectaFromDamage (int iDamage, const CVector &vHitPos, int iDirection, const DamageDesc &Damage)

//	CreateEjectaFromDamage
//
//	Create ejecta when hit by damage

	{
	int i;

	int iEjectaAdj;
	if (iEjectaAdj = m_pType->GetEjectaAdj())
		{
		//	Ignore if damage came from ejecta (so that we avoid chain reactions)

		if (Damage.GetCause() == killedByEjecta && mathRandom(1, 100) <= 90)
			return;

		//	Adjust for the station propensity to create ejecta.

		iDamage = iEjectaAdj * iDamage / 100;
		if (iDamage == 0)
			return;

		//	Compute the number of pieces of ejecta

		int iCount;
		if (iDamage <= 5)
			iCount = ((mathRandom(1, 100) <= (iDamage * 20)) ? 1 : 0);
		else if (iDamage <= 12)
			iCount = mathRandom(1, 3);
		else if (iDamage <= 24)
			iCount = mathRandom(2, 6);
		else
			iCount = mathRandom(4, 12);

		//	Generate ejecta

		CWeaponFireDesc *pEjectaType = m_pType->GetEjectaType();
		for (i = 0; i < iCount; i++)
			{
			int iTrajectoryAngle = iDirection 
					+ (mathRandom(0, 12) + mathRandom(0, 12) + mathRandom(0, 12) + mathRandom(0, 12) + mathRandom(0, 12))
					+ (360 - 30);
			iTrajectoryAngle = iTrajectoryAngle % 360;

			Metric rSpeed = pEjectaType->GetInitialSpeed();
			CVector vVel = GetVel() + PolarToVector(iTrajectoryAngle, rSpeed);

			GetSystem()->CreateWeaponFire(pEjectaType,
					0,
					killedByEjecta,
					CDamageSource(this, killedByEjecta),
					vHitPos,
					vVel,
					iTrajectoryAngle,
					NULL,
					CSystem::CWF_EJECTA,
					NULL);
			}

		//	Create geyser effect

		CParticleEffect::CreateGeyser(GetSystem(),
				this,
				vHitPos,
				NullVector,
				mathRandom(5, 15),
				mathRandom(50, 150),
				CParticleEffect::paintFlame,
				iDamage + 2,
				0.15 * LIGHT_SPEED,
				iDirection,
				20,
				NULL);
		}
	}

ALERROR CStation::CreateFromType (CSystem *pSystem,
								  CStationType *pType,
								  const CVector &vPos,
								  const CVector &vVel,
								  CXMLElement *pExtraData,
								  CStation **retpStation,
								  CString *retsError)

//	CreateFromType
//
//	Creates a new station based on the type

	{
	ALERROR error;
	CStation *pStation;
	CXMLElement *pDesc = pType->GetDesc();
	int i;

	if (!pType->CanBeEncountered())
		{
		if (retsError)
			*retsError = CONSTLIT("Cannot be encountered");
		return ERR_FAIL;
		}

	//	Create the new station

	pStation = new CStation;
	if (pStation == NULL)
		{
		if (retsError)
			*retsError = CONSTLIT("Out of memory");
		return ERR_MEMORY;
		}

	//	Initialize

	pStation->m_pType = pType;
	pStation->Place(vPos, vVel);
	pStation->m_pMapOrbit = NULL;
	pStation->m_pTrade = NULL;
	pStation->m_iDestroyedAnimation = 0;
	pStation->m_fKnown = false;
	pStation->m_fReconned = false;
	pStation->m_fExplored = false;
	pStation->m_fFireReconEvent = false;
	pStation->m_fNoMapLabel = false;
	pStation->m_fActive = pType->IsActive();
	pStation->m_fNoReinforcements = false;
	pStation->m_fRadioactive = false;
	pStation->m_xMapLabel = 10;
	pStation->m_yMapLabel = -6;
	pStation->m_rMass = pType->GetMass();
	pStation->m_dwWreckUNID = 0;

	//	We generally don't move

	if (!pType->IsMobile())
		pStation->SetCannotMove();
	else
		pStation->SetCanBounce();

	//	Background objects cannot be hit

	if (pType->IsBackgroundObject() || pType->IsVirtual())
		pStation->SetCannotBeHit();

	//	Friendly fire?

	if (!pType->CanHitFriends())
		pStation->SetNoFriendlyFire();

	if (!pType->CanBeHitByFriends())
		pStation->SetNoFriendlyTarget();

	//	Other

	pStation->SetEventFlags();

	//	Name

	if (pType->HasRandomNames())
		pStation->m_sName = pType->GenerateRandomName(pSystem->GetName(), &pStation->m_dwNameFlags);
	else
		pStation->m_dwNameFlags = 0;

	//	Stargates

	pStation->m_sStargateDestNode = pType->GetDestNodeID();
	pStation->m_sStargateDestEntryPoint = pType->GetDestEntryPoint();

	//	Get the scale

	pStation->m_Scale = pType->GetScale();

	if (pDesc->GetAttributeBool(NO_MAP_LABEL_ATTRIB))
		pStation->m_fNoMapLabel = true;

	//	We block others (CanBlock returns TRUE only for other stations)

	if (pStation->m_Scale != scaleStar && pStation->m_Scale != scaleWorld)
		pStation->SetIsBarrier();

	//	Load hit points and armor information

	pStation->m_fImmutable = pType->IsImmutable();
	pStation->m_iHitPoints = pType->GetInitialHitPoints();
	pStation->m_iMaxHitPoints = pType->GetMaxHitPoints();
	pStation->m_pArmorClass = pType->GetArmorClass();
	pStation->m_iMaxStructuralHP = pType->GetMaxStructuralHitPoints();
	pStation->m_iStructuralHP = pType->GetStructuralHitPoints();

	//	Pick an appropriate image. This call will set the shipwreck image, if
	//	necessary or the variant (if appropriate). HACK: In the case of a shipwreck,
	//	this call also sets the name and other properties (such as structuralHP)
	//
	//	This call also sets the bounds (since setting the image sets the bounds)

	pType->SetImageSelector(&pStation->m_ImageSelector);

	//	Now that we have an image, set the bound

	const CObjectImageArray &Image = pStation->GetImage(NULL, NULL);
	const RECT &rcImage = Image.GetImageRect();
	pStation->SetBounds(rcImage);

	//	If we are a wreck, set the wreck parameters (mass, etc.)

	if (pStation->m_ImageSelector.HasShipwreckImage())
		{
		CShipClass *pWreckClass = pStation->m_ImageSelector.GetShipwreckClass();
		pStation->SetWreckParams(pWreckClass);
		}

	//	Create any items on the station

	if (error = pStation->CreateRandomItems(pType->GetRandomItemTable(), pSystem->GetLevel()))
		{
		if (retsError)
			*retsError = CONSTLIT("Unable to create random items");
		delete pStation;
		return error;
		}

	//	Initialize devices

	CXMLElement *pDevices = pDesc->GetContentElementByTag(DEVICES_TAG);
	if (pDevices)
		{
		CItemListManipulator Items(pStation->GetItemList());

		for (i = 0; 
				(i < pDevices->GetContentElementCount() && i < maxDevices);
				i++)
			{
			CXMLElement *pDeviceDesc = pDevices->GetContentElement(i);
			DWORD dwDeviceID = pDeviceDesc->GetAttributeInteger(DEVICE_ID_ATTRIB);
			CDeviceClass *pClass = g_pUniverse->FindDeviceClass(dwDeviceID);
			if (pClass == NULL)
				{
				if (retsError)
					*retsError = strPatternSubst(CONSTLIT("Cannot find deviceID: %08x"), dwDeviceID);
				return ERR_FAIL;
				}

			//	Allocate the devices structure

			if (pStation->m_pDevices == NULL)
				pStation->m_pDevices = new CInstalledDevice [maxDevices];

			//	Add as an item

			CItem DeviceItem(pClass->GetItemType(), 1);
			Items.AddItem(DeviceItem);

			//	Initialize properties of the device slot

			SDesignLoadCtx Ctx;
			pStation->m_pDevices[i].InitFromXML(Ctx, pDeviceDesc);
			pStation->m_pDevices[i].OnDesignLoadComplete(Ctx);

			//	Install the device

			pStation->m_pDevices[i].Install(pStation, Items, i, true);

			//	Is this a weapon? If so, set a flag so that we know that
			//	this station is armed. This is an optimization so that we don't
			//	do a lot of work for stations that have no weapons (e.g., asteroids)

			if (pStation->m_pDevices[i].GetCategory() == itemcatWeapon
					|| pStation->m_pDevices[i].GetCategory() == itemcatLauncher)
				{
				pStation->m_fArmed = true;
				pStation->m_pDevices[i].SelectFirstVariant(pStation);
				}
			}
		}

	//	Get notifications when other objects are destroyed

	pStation->SetObjectDestructionHook();

	//	Figure out the sovereign

	pStation->m_pSovereign = pType->GetSovereign();

	//	Initialize docking ports structure

	pStation->m_DockingPorts.InitPortsFromXML(pStation, pDesc);

	//	Make radioactive, if necessary

	if (pType->IsRadioactive())
		pStation->MakeRadioactive();

	//	Add to system (note that we must add the station to the system
	//	before creating any ships).

	if (error = pStation->AddToSystem(pSystem))
		{
		if (retsError)
			*retsError = CONSTLIT("Unable to add to system.");
		delete pStation;
		return error;
		}

	//	Initialize any CodeChain data

	CXMLElement *pInitialData = pDesc->GetContentElementByTag(INITIAL_DATA_TAG);
	if (pInitialData)
		pStation->SetDataFromXML(pInitialData);

	if (pExtraData)
		pStation->SetDataFromXML(pExtraData);

	//	Create any ships registered to this station

	IShipGenerator *pShipGenerator = pType->GetInitialShips();
	if (pShipGenerator)
		pStation->CreateRandomDockedShips(pShipGenerator);

	//	If we have a trade descriptor, create appropriate items

	CTradingDesc *pTrade = pType->GetTradingDesc();
	if (pTrade)
		pTrade->RefreshInventory(pStation);

	//	If this is a world or a star, create a small image

	pStation->CreateMapImage();

	//	This type has now been encountered

	pType->SetEncountered(pSystem ? pSystem->GetLevel() : 0);

	//	Fire events on devices

	if (pStation->m_pDevices)
		{
		for (i = 0; i < maxDevices; i++)
			if (!pStation->m_pDevices[i].IsEmpty())
				pStation->m_pDevices[i].FinishInstall(pStation);
		}

	//	If we're not in the middle of creating the system, call OnCreate
	//	(otherwise we will call OnCreate in OnSystemCreated)

	if (!pSystem->IsCreationInProgress())
		pStation->FinishCreation();
	else
		{
		//	Make sure we get an OnSystemCreated (and that we get it after
		//	our subordinates).

		pSystem->RegisterForOnSystemCreated(pStation);
		}

	//	If this is a stargate, tell the system that we've got a stargate

	if (!pStation->m_sStargateDestNode.IsBlank())
		pSystem->StargateCreated(pStation, NULL_STR, pStation->m_sStargateDestNode, pStation->m_sStargateDestEntryPoint);

	//	Return station

	if (retpStation)
		*retpStation = pStation;

	return NOERROR;
	}

ALERROR CStation::CreateMapImage (void)

//	CreateMapImage
//
//	Creates a small version of the station image

	{
	ALERROR error;

	//	Only do this for stars and planets

	if (m_Scale != scaleStar && m_Scale != scaleWorld)
		return NOERROR;

	//	Scale is 0.5 light-second per pixel (we compute the fraction of a full-sized image,
	//	while is at 12500 klicks per pixel.)

	Metric rScale = g_KlicksPerPixel / (0.3 * LIGHT_SECOND);

	//	Make sure we have an image

	int iTick, iRotation;
	const CObjectImageArray &Image = GetImage(&iTick, &iRotation);
	if (Image.IsEmpty())
		return NOERROR;

	CG16bitImage &BmpImage = Image.GetImage(strFromInt(m_pType->GetUNID()));
	const RECT &rcImage = Image.GetImageRect();

	if (error = m_MapImage.CreateFromImageTransformed(BmpImage,
			rcImage.left,
			rcImage.top + RectHeight(rcImage) * iRotation,
			RectWidth(rcImage),
			RectHeight(rcImage),
			rScale,
			rScale,
			0.0))
		return error;

	return NOERROR;
	}

void CStation::CreateRandomDockedShips (IShipGenerator *pShipGenerator)

//	CreateRandomDockedShips
//
//	Creates all the ships that are registered at this station

	{
	SShipCreateCtx Ctx;

	Ctx.pSystem = GetSystem();
	Ctx.pBase = this;
	Ctx.vPos = GetPos();
	//	1d8+1 light-second spread
	Ctx.PosSpread = DiceRange(8, 1, 1);
	Ctx.dwFlags = SShipCreateCtx::SHIPS_FOR_STATION;

	//	Create the ships

	pShipGenerator->CreateShips(Ctx);
	}

void CStation::CreateStructuralDestructionEffect (SDestroyCtx &Ctx)

//	CreateStructuralDestructionEffect
//
//	Create effect when station structure is destroyed

	{
	//	Create fracture effect

	int iTick, iVariant;
	const CObjectImageArray &Image = GetImage(&iTick, &iVariant);

	if (!Image.IsEmpty())
		{
		CFractureEffect::Create(GetSystem(),
				GetPos(),
				GetVel(),
				Image,
				iTick,
				iVariant,
				CFractureEffect::styleExplosion,
				NULL);
		}

	//	Create explosion

	SExplosionType Explosion;
	FireGetExplosionType(&Explosion);
	if (Explosion.pDesc == NULL)
		Explosion.pDesc = m_pType->GetExplosionType();

	if (Explosion.pDesc)
		{
		CItemEnhancementStack *pEnhancements = NULL;
		if (Explosion.iBonus != 0)
			{
			pEnhancements = new CItemEnhancementStack;
			pEnhancements->InsertHPBonus(Explosion.iBonus);
			}

		GetSystem()->CreateWeaponFire(Explosion.pDesc,
				pEnhancements,
				Explosion.iCause,
				CDamageSource(this, Explosion.iCause),
				GetPos(),
				GetVel(),
				0,
				NULL,
				CSystem::CWF_EXPLOSION,
				NULL);

		if (pEnhancements)
			pEnhancements->Delete();
		}
	else
		{
		//	Create Particles

		CObjectImageArray PartImage;
		RECT rcRect;
		rcRect.left = 0;
		rcRect.top = 0;
		rcRect.right = 4;
		rcRect.bottom = 4;
		PartImage.Init(g_ShipExplosionParticlesUNID,
				rcRect,
				8,
				3);

		CParticleEffect::CreateExplosion(GetSystem(),
				NULL,
				GetPos(),
				GetVel(),
				mathRandom(1, 50),
				LIGHT_SPEED * 0.25,
				0,
				300,
				PartImage,
				NULL);

		CEffectCreator *pEffect = g_pUniverse->FindEffectType(g_ExplosionUNID);
		if (pEffect)
			pEffect->CreateEffect(GetSystem(),
				NULL,
				GetPos(),
				GetVel(),
				0);
		}

	g_pUniverse->PlaySound(this, g_pUniverse->FindSound(g_ShipExplosionSoundUNID));
	}

CString CStation::DebugCrashInfo (void)

//	DebugCrashInfo
//
//	Dump crash information

	{
	int i;
	CString sResult;

	if (IsAbandoned())
		sResult.Append(CONSTLIT("abandoned\r\n"));

	sResult.Append(strPatternSubst(CONSTLIT("m_pBase: %s\r\n"), CSpaceObject::DebugDescribe(m_pBase)));
	sResult.Append(strPatternSubst(CONSTLIT("m_pTarget: %s\r\n"), CSpaceObject::DebugDescribe(m_pTarget)));

	for (i = 0; i < m_Targets.GetCount(); i++)
		sResult.Append(strPatternSubst(CONSTLIT("m_Targets[%d]: %s\r\n"), i, CSpaceObject::DebugDescribe(m_Targets.GetObj(i))));

	for (i = 0; i < m_Subordinates.GetCount(); i++)
		sResult.Append(strPatternSubst(CONSTLIT("m_Subordinates[%d]: %s\r\n"), i, CSpaceObject::DebugDescribe(m_Subordinates.GetObj(i))));

	for (i = 0; i < m_DockingPorts.GetPortCount(this); i++)
		{
		CSpaceObject *pDockedObj = m_DockingPorts.GetPortObj(this, i);
		if (pDockedObj)
			sResult.Append(strPatternSubst(CONSTLIT("m_DockingPorts[%d]: %s\r\n"), i, CSpaceObject::DebugDescribe(pDockedObj)));
		}

	return sResult;
	}

void CStation::FinishCreation (void)

//	FinishCreation
//
//	If we're created with a system then this is called after the system has
//	completed creation.

	{
	int i;

	//	If this is a beacon, scan all stations in range

	if (m_pType->IsBeacon())
		{
		for (i = 0; i < GetSystem()->GetObjectCount(); i++)
			{
			CSpaceObject *pObj = GetSystem()->GetObject(i);

			if (pObj 
					&& pObj->GetScale() == scaleStructure
					&& pObj != this)
				{
				CVector vDist = pObj->GetPos() - GetPos();
				Metric rDist = vDist.Length();
				if (rDist < BEACON_RANGE)
					pObj->SetKnown();
				}
			}
		}

	//	Initialize trading

	CTradingDesc *pTrade = m_pType->GetTradingDesc();
	if (pTrade)
		pTrade->OnCreate(this);

	//	Fire OnCreate

	FireOnCreate();
	}

void CStation::FriendlyFire (CSpaceObject *pAttacker)

//	FriendlyFire
//
//	Station is hit by friendly fire. See if we need to blacklist
//	the attacker.

	{
	//	For now, only blacklist the player

	if (!pAttacker->IsPlayer())
		return;

	//	No need if we don't support blacklist

	if (!m_pType->IsBlacklistEnabled())
		return;

	//	See if we need to blacklist

	if (m_Blacklist.Hit(GetSystem()->GetTick()))
		{
		Blacklist(pAttacker);
		SetAngry();
		}
	}

bool CStation::GetArmorInstallPrice (const CItem &Item, DWORD dwFlags, int *retiPrice)

//	GetArmorInstallPrice
//
//	Returns the price to install the given armor

	{
	//	See if we have an override

	if (m_pTrade && m_pTrade->GetArmorInstallPrice(this, Item, dwFlags, retiPrice))
		return true;

	//	Otherwise, ask our design type

	CTradingDesc *pTrade = m_pType->GetTradingDesc();
	if (pTrade && pTrade->GetArmorInstallPrice(this, Item, dwFlags, retiPrice))
		return true;

	//	Otherwise, we do not repair

	return false;
	}

bool CStation::GetArmorRepairPrice (const CItem &Item, int iHPToRepair, DWORD dwFlags, int *retiPrice)

//	GetArmorRepairPrice
//
//	Returns the price to repair the given number of HP for the given armor item.

	{
	//	See if we have an override

	if (m_pTrade && m_pTrade->GetArmorRepairPrice(this, Item, iHPToRepair, dwFlags, retiPrice))
		return true;

	//	Otherwise, ask our design type

	CTradingDesc *pTrade = m_pType->GetTradingDesc();
	if (pTrade && pTrade->GetArmorRepairPrice(this, Item, iHPToRepair, dwFlags, retiPrice))
		return true;

	//	Otherwise, we do not repair

	return false;
	}

CurrencyValue CStation::GetBalance (DWORD dwEconomyUNID)

//	GetBalance
//
//	Returns the amount of money the station has left

	{
	if (dwEconomyUNID == 0)
		dwEconomyUNID = GetDefaultEconomyUNID();

	if (m_pMoney)
		return (int)m_pMoney->GetCredits(dwEconomyUNID);
	else
		return 0;
	}

int CStation::GetBuyPrice (const CItem &Item, DWORD dwFlags, int *retiMaxCount)

//	GetBuyPrice
//
//	Returns the price at which the station will buy the given
//	item. Also returns the max number of items that the station
//	will buy at that price.
//
//	Returns -1 if the station will not buy the item.

	{
	int iPrice;

	//	First see if we have an override

	if (m_pTrade)
		{
		if (m_pTrade->Buys(this, Item, dwFlags, &iPrice, retiMaxCount))
			return iPrice;
		}

	//	See if our type has a price

	CTradingDesc *pTrade = m_pType->GetTradingDesc();
	if (pTrade)
		{
		if (pTrade->Buys(this, Item, dwFlags, &iPrice, retiMaxCount))
			{
			if (m_pTrade)
				return (int)m_pTrade->GetEconomyType()->Exchange(pTrade->GetEconomyType(), iPrice);
			else
				return iPrice;
			}
		}

	//	Otherwise, we will not buy the item

	return -1;
	}

CDesignType *CStation::GetDefaultDockScreen (CString *retsName)

//	GetDockScreen
//
//	Returns the screen on dock (NULL_STR if none)

	{
	if (IsAbandoned() && m_pType->GetAbandonedScreen(retsName))
		return m_pType->GetAbandonedScreen(retsName);
	else
		return m_pType->GetFirstDockScreen(retsName);
	}

CEconomyType *CStation::GetDefaultEconomy (void)

//	GetDefaultEconomy
//
//	Returns the default economy

	{
	CTradingDesc *pTrade = GetDefaultTradingDesc();
	if (pTrade)
		return pTrade->GetEconomyType();

	return CEconomyType::AsType(g_pUniverse->FindDesignType(DEFAULT_ECONOMY_UNID));
	}

DWORD CStation::GetDefaultEconomyUNID (void)

//	GetDefaultEconomyUNID
//
//	Returns the default economy

	{
	CTradingDesc *pTrade = GetDefaultTradingDesc();
	if (pTrade)
		return pTrade->GetEconomyType()->GetUNID();

	return DEFAULT_ECONOMY_UNID;
	}

const CObjectImageArray &CStation::GetImage (int *retiTick, int *retiRotation)

//	GetImage
//
//	Returns the image of this station

	{
	//	Tick

	if (retiTick)
		{
		if (m_fActive && !IsTimeStopped())
			*retiTick = GetSystem()->GetTick() + GetDestiny();
		else
			*retiTick = 0;
		}

	//	Modifiers (such as station damage)

	DWORD dwModifiers = 0;
	if (ShowWreckImage())
		dwModifiers |= CCompositeImageDesc::modStationDamage;

	//	Image

	return m_pType->GetImage(m_ImageSelector, dwModifiers, retiRotation);
	}

CString CStation::GetName (DWORD *retdwFlags)

//	GetName
//
//	Returns the name of the station

	{
	if (m_sName.IsBlank())
		return m_pType->GetName(retdwFlags);

	if (retdwFlags)
		*retdwFlags = m_dwNameFlags;

	return m_sName;
	}

int CStation::GetNearestDockPort (CSpaceObject *pRequestingObj, CVector *retvPort)

//	GetNearestDockVector
//
//	Returns a vector from the given position to the nearest
//	dock position

	{
	int iPort = m_DockingPorts.FindNearestEmptyPort(this, pRequestingObj);

	if (retvPort)
		*retvPort = m_DockingPorts.GetPortPos(this, iPort);

	return iPort;
	}

CSystem::LayerEnum CStation::GetPaintLayer (void)

//	GetPaintLayer
//
//	Returns the layer on which we should paint
	
	{
	switch (m_Scale)
		{
		case scaleStar:
		case scaleWorld:
			return CSystem::layerSpace;

		case scaleStructure:
		case scaleShip:
			return CSystem::layerStations;

		case scaleFlotsam:
			return CSystem::layerShips;

		default:
			return CSystem::layerStations;
		}
	}

ICCItem *CStation::GetProperty (const CString &sName)

//	GetProperty
//
//	Returns a property

	{
	CCodeChain &CC = g_pUniverse->GetCC();

	if (strEquals(sName, PROPERTY_ABANDONED))
		return CC.CreateBool(IsAbandoned());

	else if (strEquals(sName, PROPERTY_HP))
		return CC.CreateInteger(m_iHitPoints);

	else if (strEquals(sName, PROPERTY_IMMUTABLE))
		return CC.CreateBool(IsImmutable());

	else if (strEquals(sName, PROPERTY_INSTALL_DEVICE_MAX_LEVEL))
		{
		int iMaxLevel = -1;
		if (m_pTrade)
			{
			int iLevel = m_pTrade->GetMaxLevelMatched(serviceInstallDevice);
			if (iLevel > iMaxLevel)
				iMaxLevel = iLevel;
			}

		CTradingDesc *pTrade = m_pType->GetTradingDesc();
		if (pTrade)
			{
			int iLevel = pTrade->GetMaxLevelMatched(serviceInstallDevice);
			if (iLevel > iMaxLevel)
				iMaxLevel = iLevel;
			}

		return (iMaxLevel != -1 ? CC.CreateInteger(iMaxLevel) : CC.CreateNil());
		}

	else if (strEquals(sName, PROPERTY_MAX_HP))
		return CC.CreateInteger(m_iMaxHitPoints);

	else if (strEquals(sName, PROPERTY_MAX_STRUCTURAL_HP))
		return CC.CreateInteger(m_iMaxStructuralHP);

	else if (strEquals(sName, PROPERTY_ORBIT))
		return (m_pMapOrbit ? CreateListFromOrbit(CC, *m_pMapOrbit) : CC.CreateNil());

	else if (strEquals(sName, PROPERTY_PLAYER_BACKLISTED))
		return CC.CreateBool(IsBlacklisted(NULL));

	else if (strEquals(sName, PROPERTY_REPAIR_ARMOR_MAX_LEVEL))
		{
		int iMaxLevel = -1;
		if (m_pTrade)
			{
			int iLevel = m_pTrade->GetMaxLevelMatched(serviceRepairArmor);
			if (iLevel > iMaxLevel)
				iMaxLevel = iLevel;
			}

		CTradingDesc *pTrade = m_pType->GetTradingDesc();
		if (pTrade)
			{
			int iLevel = pTrade->GetMaxLevelMatched(serviceRepairArmor);
			if (iLevel > iMaxLevel)
				iMaxLevel = iLevel;
			}

		return (iMaxLevel != -1 ? CC.CreateInteger(iMaxLevel) : CC.CreateNil());
		}

	else if (strEquals(sName, PROPERTY_STRUCTURAL_HP))
		return CC.CreateInteger(m_iStructuralHP);

	else
		return CSpaceObject::GetProperty(sName);
	}

IShipGenerator *CStation::GetRandomEncounterTable (int *retiFrequency) const

//	GetRandomEncounterTable
//
//	Returns a random encounter table for this station
//
//	Note that we can return a NULL table, which means that we
//	we want to be called at <OnRandomEncounter>

	{
	if (retiFrequency)
		*retiFrequency = m_pType->GetEncounterFrequency();
	return m_pType->GetEncountersTable();
	}

bool CStation::GetRefuelItemAndPrice (CSpaceObject *pObjToRefuel, CItemType **retpItemType, int *retiPrice)

//	GetRefuelItemAndPrice
//
//	Returns the appropriate item to use for refueling (based on the trading
//	directives).

	{
	//	See if we have an override

	if (m_pTrade && m_pTrade->GetRefuelItemAndPrice(this, pObjToRefuel, 0, retpItemType, retiPrice))
		return true;

	//	Otherwise, ask our design type

	CTradingDesc *pTrade = m_pType->GetTradingDesc();
	if (pTrade && pTrade->GetRefuelItemAndPrice(this, pObjToRefuel, 0, retpItemType, retiPrice))
		return true;

	//	Otherwise, we do not refuel

	return false;
	}

int CStation::GetSellPrice (const CItem &Item, DWORD dwFlags)

//	GetSellPrice
//
//	Returns the price at which the station will sell the given
//	item. Returns 0 if the station cannot or will not sell the
//	item.

	{
	bool bHasTradeDirective = false;
	int iPrice = -1;

	//	See if we have an override price

	if (m_pTrade)
		{
		m_pTrade->Sells(this, Item, dwFlags, &iPrice);
		bHasTradeDirective = true;
		}

	//	See if our type has a price

	if (iPrice == -1)
		{
		CTradingDesc *pTrade = m_pType->GetTradingDesc();
		if (pTrade)
			{
			pTrade->Sells(this, Item, dwFlags, &iPrice);

			if (m_pTrade && iPrice != -1)
				iPrice = (int)m_pTrade->GetEconomyType()->Exchange(pTrade->GetEconomyType(), iPrice);

			bHasTradeDirective = true;
			}
		}

	//	If we have Trade directives and they specify no price, then we can't
	//	sell any.

	if (iPrice == -1)
		{
		if (bHasTradeDirective)
			return 0;

		//	Otherwise, get the price from the item itself

		iPrice = (int)GetDefaultEconomy()->Exchange(Item.GetCurrencyType(), Item.GetTradePrice(this));
		}

	//	If we don't have any of the item, then we don't sell any

	if (!(dwFlags & CTradingDesc::FLAG_NO_INVENTORY_CHECK))
		{
		CItemListManipulator ItemList(GetItemList());
		if (!ItemList.SetCursorAtItem(Item))
			return 0;
		}

	//	Return the price

	return iPrice;
	}

CString CStation::GetStargateID (void) const

//	GetStargateID
//
//	Returns the stargate ID

	{
	if (m_sStargateDestNode.IsBlank())
		return NULL_STR;

	CSystem *pSystem = GetSystem();
	if (pSystem == NULL)
		return NULL_STR;

	CTopologyNode *pNode = pSystem->GetTopology();
	if (pNode == NULL)
		return NULL_STR;

	return pNode->FindStargateName(m_sStargateDestNode, m_sStargateDestEntryPoint);
	}

int CStation::GetVisibleDamage (void)

//	GetVisibleDamage
//
//	Returns the amount of damage (%) that the object has taken

	{
	int iMaxHP;
	int iHP;

	if (IsAbandoned() && m_iStructuralHP > 0)
		{
		iMaxHP = m_iMaxStructuralHP;
		iHP = m_iStructuralHP;
		}
	else
		{
		iMaxHP = m_iMaxHitPoints;
		iHP = m_iHitPoints;
		}

	if (iMaxHP > 0)
		return 100 - (iHP * 100 / iMaxHP);
	else
		return 0;
	}

CDesignType *CStation::GetWreckType (void) const

//	GetWreckType
//
//	Returns the type of the ship class that this is a wreck of (or NULL)

	{
	if (m_dwWreckUNID == 0)
		return NULL;

	return g_pUniverse->FindDesignType(m_dwWreckUNID);
	}

bool CStation::HasAttribute (const CString &sAttribute) const

//	HasAttribute
//
//	Returns TRUE if it has the given attribute

	{
	return m_pType->HasLiteralAttribute(sAttribute);
	}

bool CStation::HasMapLabel (void)

//	HasMapLabel
//
//	Returns TRUE if the object has a map label

	{
	return m_Scale != scaleWorld
			&& m_Scale != scaleStar
			&& m_pType->ShowsMapIcon() 
			&& !m_fNoMapLabel;
	}

bool CStation::ImageInObject (const CVector &vObjPos, const CObjectImageArray &Image, int iTick, int iRotation, const CVector &vImagePos)

//	ImageInObject
//
//	Returns TRUE if the given image at the given position intersects this
//	station

	{
	int iDestTick, iDestVariant;
	const CObjectImageArray &DestImage = GetImage(&iDestTick, &iDestVariant);

	return ImagesIntersect(Image, iTick, iRotation, vImagePos,
			DestImage, iDestTick, iDestVariant, vObjPos);
	}

bool CStation::IsBlacklisted (CSpaceObject *pObj)

//	IsBlacklisted
//
//	Returns TRUE if we are blacklisted
	
	{
	if (pObj)
		return (pObj->IsPlayer() && m_Blacklist.IsBlacklisted());
	else
		return m_Blacklist.IsBlacklisted();
	}

EDamageResults CStation::GetPassthroughDefault (void)

//	GetPassthroughDefault
//
//	Returns the default damage result when hit by passthrough

	{
	if (IsImmutable())
		return damageNoDamageNoPassthrough;
	else if (IsAbandoned())
		{
		if (m_iStructuralHP > 0)
			return damageStructuralHit;
		else
			return damageNoDamageNoPassthrough;
		}
	else
		return damageArmorHit;
	}

EDamageResults CStation::OnDamage (SDamageCtx &Ctx)

//	Damage
//
//	Station takes damage

	{
	int i;

	//	Stations don't have armor segments

	Ctx.iSectHit = -1;

	//	Roll for damage

	Ctx.iDamage = Ctx.Damage.RollDamage();
	if (Ctx.iDamage == 0)
		{
		if (IsImmutable())
			return damageNoDamageNoPassthrough;
		else
			return damageNoDamage;
		}

	//	OnAttacked event

	if (HasOnAttackedEvent())
		FireOnAttacked(Ctx);

	GetSystem()->FireOnSystemObjAttacked(Ctx);

	//	See if the damage is blocked by some external defense

	if (m_Overlays.AbsorbDamage(this, Ctx))
		{
		if (IsDestroyed())
			return damageDestroyed;
		else if (Ctx.iDamage == 0)
			{
			if (IsImmutable())
				return damageNoDamageNoPassthrough;
			else
				return damageNoDamage;
			}
		}

	//	If this is a momentum attack then we are pushed

	int iMomentum;
	if (IsMobile() && (iMomentum = Ctx.Damage.GetMomentumDamage()))
		{
		CVector vAccel = PolarToVector(Ctx.iDirection, -10 * iMomentum * iMomentum);
		Accelerate(vAccel, g_MomentumConstant);
		ClipSpeed(0.25 * LIGHT_SPEED);
		}

	//	Let our shield generators take a crack at it

	int iOriginalDamage = Ctx.iDamage;
	if (m_pDevices)
		{
		for (i = 0; i < maxDevices; i++)
			if (!m_pDevices[i].IsEmpty())
				{
				bool bAbsorbed = m_pDevices[i].AbsorbDamage(this, Ctx);
				if (IsDestroyed())
					return damageDestroyed;
				else if (bAbsorbed)
					return damageAbsorbedByShields;
				}
		}

	//	If we're immutable, then nothing else happens.

	if (IsImmutable())
		{
		Ctx.iDamage = 0;
		Ctx.pDesc->CreateHitEffect(GetSystem(), Ctx);

		return damageNoDamageNoPassthrough;
		}

	//	We go through a different path if we're already abandoned
	//	(i.e., there is no life on the station)

	if (IsAbandoned())
		{
		EDamageResults iResult = damageNoDamageNoPassthrough;

		//	If this is a paralysis attack then no damage

		int iEMP = Ctx.Damage.GetEMPDamage();
		if (iEMP)
			Ctx.iDamage = 0;

		//	If this is blinding attack then no damage

		int iBlinding = Ctx.Damage.GetBlindingDamage();
		if (iBlinding)
			Ctx.iDamage = 0;

		//	If this is a radioactive attack then there is a chance that we will
		//	be contaminated. (Note: We can only be contaminated by an attack if
		//	we're abandoned)

		int iRadioactive = Ctx.Damage.GetRadiationDamage();
		if (iRadioactive 
				&& GetScale() != scaleStar
				&& GetScale() != scaleWorld)
			{
			int iChance = 4 * iRadioactive * iRadioactive;
			if (mathRandom(1, 100) <= iChance)
				MakeRadioactive();
			}

		//	If we have mining damage then call OnMining

		if (Ctx.Damage.GetMiningAdj())
			FireOnMining(Ctx);

		//	Once the station is abandoned, only WMD damage can destroy it

		if (Ctx.Damage.GetMassDestructionAdj() > 0)
			Ctx.iDamage = Max(1, Ctx.Damage.GetMassDestructionAdj() * Ctx.iDamage / 100);
		else
			Ctx.iDamage = 0;

		//	Hit effect

		Ctx.pDesc->CreateHitEffect(GetSystem(), Ctx);

		//	Give events a chance to change the damage

		if (HasOnDamageEvent())
			FireOnDamage(Ctx);

		//	Take damage

		if (Ctx.iDamage > 0)
			{
			//	See if this hit destroyed us

			if (m_iStructuralHP > 0 && m_iStructuralHP <= Ctx.iDamage)
				{
				//	Destroy

				Destroy(Ctx.Damage.GetCause(), Ctx.Attacker);
				iResult = damageDestroyed;
				}

			//	Otherwise, take damage

			else
				{
				//	See if we should generate ejecta

				CreateEjectaFromDamage(Ctx.iDamage, Ctx.vHitPos, Ctx.iDirection, Ctx.Damage);

				//	If we can be destroyed, subtract from hp

				if (m_iStructuralHP)
					{
					m_iStructuralHP -= Ctx.iDamage;
					iResult = damageStructuralHit;
					}
				}
			}

		//	Done

		return iResult;
		}

	//	If we're not abandoned, we go through a completely different code-path

	CSpaceObject *pOrderGiver = (Ctx.Attacker.GetObj() ? Ctx.Attacker.GetObj()->GetOrderGiver(Ctx.Damage.GetCause()) : NULL);

	if (pOrderGiver && pOrderGiver->CanAttack())
		{
		//	If the attacker is a friend then we should keep track of
		//	friendly fire hits

		if (!IsEnemy(pOrderGiver) && !IsBlacklisted(pOrderGiver))
			{
			//	Ignore automated attacks

			if (!Ctx.Damage.IsAutomatedWeapon())
				{
				Communicate(pOrderGiver, msgWatchTargets);
				FriendlyFire(pOrderGiver);
				}
			}

		//	Otherwise, consider this a hostile act.

		else
			{
			//	Tell our guards that we were attacked

			for (int i = 0; i < m_Subordinates.GetCount(); i++)
				Communicate(m_Subordinates.GetObj(i), msgAttackDeter, Ctx.Attacker.GetObj());

			//	Alert other stations

			if (m_pType->AlertWhenAttacked())
				RaiseAlert(pOrderGiver);

			//	We get angry

			SetAngry();
			}
		}

	//	Armor effects

	bool bCustomDamage = false;
	if (m_pArmorClass)
		{
		//	Create an item context

		CItem ArmorItem(m_pArmorClass->GetItemType(), 1);
		CItemCtx ItemCtx(&ArmorItem, this);

		//	Compute the effects based on damage and our armor

		m_pArmorClass->CalcDamageEffects(ItemCtx, Ctx);

		//	Give custom weapons a chance

		bCustomDamage = Ctx.pDesc->FireOnDamageArmor(Ctx);
		if (IsDestroyed())
			return damageDestroyed;

		//	Adjust the damage for the armor

		m_pArmorClass->CalcAdjustedDamage(ItemCtx, Ctx);

		//	If this armor section reflects this kind of damage then
		//	send the damage on

		if (Ctx.bReflect && Ctx.pCause)
			{
			Ctx.pCause->CreateReflection(Ctx.vHitPos, (Ctx.iDirection + 120 + mathRandom(0, 120)) % 360);
			return damageNoDamage;
			}
		}

	//	If we're a multi-hull object then we adjust for mass destruction
	//	effects (non-mass destruction weapons don't hurt us very much)

	if (Ctx.iDamage > 0 && m_pType->IsMultiHull())
		{
		int iWMD = Ctx.Damage.GetMassDestructionAdj();
		Ctx.iDamage = Max(1, iWMD * Ctx.iDamage / 100);
		}

	//	Hit effect

	Ctx.pDesc->CreateHitEffect(GetSystem(), Ctx);

	//	If no damage, we're done

	if (Ctx.iDamage == 0 && !bCustomDamage)
		return damageNoDamage;

	//	Give events a chance to change the damage

	if (HasOnDamageEvent())
		FireOnDamage(Ctx);

	//	If we've still got armor left, then we take damage but otherwise
	//	we're OK.

	if (Ctx.iDamage < m_iHitPoints)
		{
		m_iHitPoints -= Ctx.iDamage;
		return damageArmorHit;
		}

	//	Otherwise we're in big trouble

	else
		{
		m_iHitPoints = 0;

		SDestroyCtx DestroyCtx;
		DestroyCtx.pObj = this;
		DestroyCtx.Attacker = Ctx.Attacker;
		DestroyCtx.pWreck = this;
		DestroyCtx.iCause = Ctx.Damage.GetCause();

		//	Run OnDestroy script

		m_Overlays.FireOnObjDestroyed(this, DestroyCtx);
		FireOnItemObjDestroyed(DestroyCtx);
		FireOnDestroy(DestroyCtx);

		//	Station is destroyed. Take all the installed devices and turn
		//	them into normal damaged items

		CItemListManipulator Items(GetItemList());
		while (Items.MoveCursorForward())
			{
			CItem Item = Items.GetItemAtCursor();

			if (Item.IsInstalled())
				{
				//	Uninstall the device

				int iDevSlot = Item.GetInstalled();
				CInstalledDevice *pDevice = &m_pDevices[iDevSlot];
				pDevice->Uninstall(this, Items);

				//	Chance that the item is destroyed

				if (Item.GetType()->IsVirtual() || mathRandom(1, 100) <= 50)
					Items.DeleteAtCursor(1);
				else
					Items.SetDamagedAtCursor(true);

				//	Reset cursor because we may have changed position

				Items.ResetCursor();
				}
			}

		InvalidateItemListAddRemove();

		//	Tell all objects that we've been destroyed

		NotifyOnObjDestroyed(DestroyCtx);

		for (int i = 0; i < GetSystem()->GetObjectCount(); i++)
			{
			CSpaceObject *pObj = GetSystem()->GetObject(i);

			if (pObj && pObj != this)
				pObj->OnStationDestroyed(DestroyCtx);
			}

		GetSystem()->FireOnSystemObjDestroyed(DestroyCtx);
		g_pUniverse->FireOnGlobalObjDestroyed(DestroyCtx);

		//	Alert others, if necessary

		if (pOrderGiver && pOrderGiver->CanAttack())
			{
			//	Tell our base that we've been destroyed

			if (m_pBase)
				Communicate(m_pBase, msgAttack, pOrderGiver);

			if (m_pType->AlertWhenDestroyed())
				RaiseAlert(pOrderGiver);
			}

		//	Explosion effect

		CreateDestructionEffect();

		return damageDestroyedAbandoned;
		}
	}

void CStation::OnDestroyed (SDestroyCtx &Ctx)

//	OnDestroyed
//
//	Station has been destroyed

	{
	//	Run OnDestroy script

	m_Overlays.FireOnObjDestroyed(this, Ctx);
	FireOnItemObjDestroyed(Ctx);
	FireOnDestroy(Ctx);

	//	Release docking port objects

	m_DockingPorts.OnDestroyed();

	//	Create effect

	switch (Ctx.iCause)
		{
		case removedFromSystem:
		case enteredStargate:
			//	No effect
			break;

		case killedByRadiationPoisoning:
		case killedByRunningOutOfFuel:
		case killedByOther:
			//	No effect
			break;

		default:
			CreateStructuralDestructionEffect(Ctx);
		}
	}

void CStation::OnMove (const CVector &vOldPos, Metric rSeconds)

//	OnMove
//
//	Do stuff when station moves

	{
	//	If the station is moving then make sure all docked ships
	//	move along with it.

	m_DockingPorts.MoveAll(this);
	}

void CStation::ObjectDestroyedHook (const SDestroyCtx &Ctx)

//	ObjectDestroyedHook
//
//	Notification of another object being destroyed

	{
	bool bAttackDestroyer;

	//	Figure out who gave the orders

	CSpaceObject *pOrderGiver = NULL;
	if (Ctx.Attacker.GetObj())
		pOrderGiver = Ctx.Attacker.GetObj()->GetOrderGiver(Ctx.iCause);

	//	If this object is docked with us, remove it from the
	//	docking table.

	m_DockingPorts.OnObjDestroyed(this, Ctx.pObj, &bAttackDestroyer);

	//	Remove the object from any lists that it may be on

	m_Targets.Remove(Ctx.pObj);

	if (Ctx.pObj == m_pTarget)
		m_pTarget = NULL;

	//	If our base got destroyed, then send alert

	if (Ctx.pObj == m_pBase)
		{
		m_pBase = NULL;
		bAttackDestroyer = true;
		}

	//	If this was a subordinate, then send an alert, if necessary

	if (m_Subordinates.Remove(Ctx.pObj))
		bAttackDestroyer = true;

	//	Retaliate against the attacker

	if (bAttackDestroyer 
			&& pOrderGiver
			&& pOrderGiver->CanAttack())
		{
		if (!IsEnemy(pOrderGiver) && !IsBlacklisted(pOrderGiver))
			Blacklist(pOrderGiver);
		else if (m_pType->AlertWhenAttacked())
			RaiseAlert(pOrderGiver);

		SetAngry();
		}
	}

bool CStation::ObjectInObject (const CVector &vObj1Pos, CSpaceObject *pObj2, const CVector &vObj2Pos)

//	ObjectInObject
//
//	Returns TRUE if the given object intersects this object

	{
	int iTick, iVariant;
	const CObjectImageArray &Image = GetImage(&iTick, &iVariant);

	return pObj2->ImageInObject(vObj2Pos, Image, iTick, iVariant, vObj1Pos);
	}

DWORD CStation::OnCommunicate (CSpaceObject *pSender, MessageTypes iMessage, CSpaceObject *pParam1, DWORD dwParam2)

//	OnCommunicate
//
//	Handle communications

	{
	switch (iMessage)
		{
		case msgAttack:
		case msgAttackDeter:
		case msgDestroyBroadcast:
			{
			CSpaceObject *pTarget = pParam1;

			if (!IsEnemy(pTarget) && !IsBlacklisted(pTarget))
				{
				Blacklist(pTarget);
				}

			//	Attack the target, unless we've already got it on our
			//	target list.

			else if (!m_Targets.FindObj(pTarget))
				{
				m_Targets.Add(pTarget);

#ifdef DEBUG_ALERTS
				g_pUniverse->DebugOutput("%d: Received msgDestroyBroadcast", this);
#endif

				//	Order out some number of subordinates to attack
				//	the target.

				int iLeft = mathRandom(3, 5);
				int iCount = m_Subordinates.GetCount();
				for (int i = 0; i < iCount && iLeft > 0; i++)
					{
					if (Communicate(m_Subordinates.GetObj(i), 
							msgDestroyBroadcast, 
							pTarget) == resAck)
						{
						iLeft--;
#ifdef DEBUG_ALERTS
						g_pUniverse->DebugOutput("   %d acknowledges attack order", m_Subordinates.GetObj(i));
#endif
						}
					}
				}

			return resAck;
			}

		default:
			return resNoAnswer;
		}
	}

void CStation::OnComponentChanged (ObjectComponentTypes iComponent)

//	OnComponentChanged
//
//	Some part of the object has changed

	{
	int i;

	switch (iComponent)
		{
		case comCargo:
			{
			if (m_pDevices)
				{
				for (i = 0; i < maxDevices; i++)
					{
					CInstalledDevice *pDevice = &m_pDevices[i];
					if (pDevice->IsEmpty())
						continue;

					//	If one of our weapons doesn't have a variant selected, then
					//	try to select it now (if we just got some new ammo, this will
					//	select the ammo)

					if (!pDevice->IsVariantSelected(this))
						pDevice->SelectFirstVariant(this);
					}
				}

			break;
			}
		}
	}

void CStation::OnPaint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx)

//	OnPaint
//
//	Paint the station

	{
	int i;

	//	Known

	m_fKnown = true;
	if (!m_fReconned && !Ctx.fNoRecon)
		{
		if (m_fFireReconEvent)
			{
			Reconned();
			m_fFireReconEvent = false;
			}

		m_fReconned = true;
		}

	//	First paint any object that are docked behind us

	if (!Ctx.fNoDockedShips)
		{
		for (i = 0; i < m_DockingPorts.GetPortCount(this); i++)
			{
			CSpaceObject *pObj = m_DockingPorts.GetPortObj(this, i);
			if (pObj && pObj->IsPaintNeeded() && !pObj->IsPlayer())
				{
				int xObj, yObj;
				Ctx.XForm.Transform(pObj->GetPos(), &xObj, &yObj);

				if (yObj < y)
					pObj->Paint(Dest, xObj, yObj, Ctx);
				}
			}
		}

	//	Paint

	int iTick, iVariant;
	const CObjectImageArray &Image = GetImage(&iTick, &iVariant);
	if (m_fRadioactive)
		Image.PaintImageWithGlow(Dest, x, y, iTick, iVariant, RGB(0, 255, 0));
	else
		Image.PaintImage(Dest, x, y, iTick, iVariant);

	//	Paint animations

	if (!IsAbandoned() && m_pType->HasAnimations())
		m_pType->PaintAnimations(Dest, x, y, iTick);

	//	If this is a sign, then paint the name of the station

	if (m_pType->IsSign() && !IsAbandoned())
		{
		RECT rcRect;

		rcRect.left = x - 26;
		rcRect.top = y - 20;
		rcRect.right = x + 40;
		rcRect.bottom = y + 20;

		g_pUniverse->GetNamedFont(CUniverse::fontSign).DrawText(Dest, rcRect, RGB_SIGN_COLOR, GetName(), -2);
		}

	//	Paint energy fields

	Ctx.iTick = iTick;
	Ctx.iVariant = 0;
	Ctx.iDestiny = GetDestiny();
	Ctx.iRotation = GetRotation();
	m_Overlays.Paint(Dest, x, y, Ctx);

	//	Now paint any object that are docked in front of us

	if (!Ctx.fNoDockedShips)
		{
		for (i = 0; i < m_DockingPorts.GetPortCount(this); i++)
			{
			CSpaceObject *pObj = m_DockingPorts.GetPortObj(this, i);
			if (pObj && !pObj->IsPlayer())
				{
				int xObj, yObj;
				Ctx.XForm.Transform(pObj->GetPos(), &xObj, &yObj);

				if (yObj >= y)
					pObj->Paint(Dest, xObj, yObj, Ctx);
				}
			}
		}

	//	Highlight

	if (IsHighlighted() && !Ctx.fNoSelection)
		PaintHighlight(Dest, Image.GetImageRectAtPoint(x, y), Ctx);

#ifdef DEBUG_BOUNDING_RECT
	{
	CVector vUR, vLL;
	GetBoundingRect(&vUR, &vLL);

	int xLeft, xRight, yTop, yBottom;
	Ctx.XForm.Transform(vUR, &xRight, &yTop);
	Ctx.XForm.Transform(vLL, &xLeft, &yBottom);

	DrawRectDotted(Dest, 
			xLeft, 
			yTop, 
			xRight - xLeft, 
			yBottom - yTop, 
			CG16bitImage::RGBValue(220,220,220));
	}
#endif

#ifdef DEBUG_DOCK_PORT_POS
	for (int i = 0; i < m_DockingPorts.GetPortCount(this); i++)
		{
		int x, y;
		Ctx.XForm.Transform(m_DockingPorts.GetPortPos(this, i), &x, &y);
		Dest.Fill(x - 2, y - 2, 4, 4, CG16bitImage::RGBValue(0, 255, 0));
		}
#endif
	}

void CStation::OnObjBounce (CSpaceObject *pObj, const CVector &vPos)

//	OnObjBounce
//
//	An object has just bounced off the station

	{
	CEffectCreator *pEffect;
	if (pEffect = m_pType->GetBarrierEffect())
		pEffect->CreateEffect(GetSystem(),
				this,
				vPos,
				GetVel(),
				0);
	}

void CStation::OnObjEnteredGate (CSpaceObject *pObj, CTopologyNode *pDestNode, const CString &sDestEntryPoint, CSpaceObject *pStargate)

//	OnObjEnteredGate
//
//	The given object has just entered some random gate (no necessarily us).

	{
	//	If the player entered a gate, fire a special event

	if (pObj->IsPlayer())
		FireOnPlayerLeftSystem(pObj, pDestNode, sDestEntryPoint, pStargate);
	}

void CStation::OnObjLeaveGate (CSpaceObject *pObj)

//	OnObjLeaveGate
//
//	An object has just passed through gate

	{
	//	Create gating effect

	if (!pObj->IsVirtual())
		{
		CEffectCreator *pEffect = m_pType->GetGateEffect();
		if (pEffect)
			pEffect->CreateEffect(GetSystem(),
					NULL,
					GetPos(),
					GetVel(),
					0);
		}
	}

void CStation::OnPlayerObj (CSpaceObject *pPlayer)

//	OnPlayObj
//
//	Player has entered the system

	{
	FireOnPlayerEnteredSystem(pPlayer);
	}

void CStation::OnReadFromStream (SLoadCtx &Ctx)

//	OnReadFromStream
//
//	Read object data from a stream
//
//	DWORD		station type UNID
//	CString		m_sName
//	DWORD		sovereign UNID
//	DWORD		m_Scale
//	Metric		m_rMass
//	CCompositeImageSelector m_ImageSelector
//	DWORD		m_iDestroyedAnimation
//	DWORD		1 if orbit, 0xffffffff if no orbit
//	Orbit		System orbit
//	DWORD		m_xMapLabel
//	DWORD		m_yMapLabel
//	CString		m_sStargateDestNode
//	CString		m_sStargateDestEntryPoint
//	DWORD		armor class UNID, 0xffffffff if no armor
//	DWORD		m_iHitPoints
//	DWORD		m_iMaxHitPoints
//	DWORD		m_iStructuralHP
//	DWORD		m_iMaxStructuralHP
//
//	DWORD		No of devices
//	DWORD		device: class UNID
//	DWORD		device: m_dwData
//	DWORD		device: low = m_iPosAngle; hi = m_iPosRadius
//	DWORD		device: low = m_iMinFireArc; hi = m_iMaxFireArc
//	DWORD		device: low = m_iActivationDelay
//	DWORD		device: flags
//
//	Overlays	m_Overlays
//
//	DWORD		No of registered objects
//	DWORD		registered object (CSpaceObject ref)
//
//	DWORD		No of docking ports
//	DWORD		port: iStatus
//	DWORD		port: pObj (CSpaceObject ref)
//	Vector		port: vPos
//
//	DWORD		No of subordinates
//	DWORD		subordinate (CSpaceObject ref)
//
//	DWORD		No of targets
//	DWORD		target (CSpaceObject ref)
//
//	CAttackDetector m_Blacklist
//	DWORD		m_iAngryCounter
//	DWORD		m_iReinforceRequestCount
//	CCurrencyBlock	m_pMoney
//	CTradeDesc	m_pTrade
//
//	DWORD		m_dwWreckUNID
//	DWORD		flags

	{
#ifdef DEBUG_LOAD
	::OutputDebugString("CStation::OnReadFromStream\n");
#endif
	int i;
	DWORD dwLoad;

	//	Station type

	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	m_pType = g_pUniverse->FindStationType(dwLoad);

	//	Read name

	m_sName.ReadFromStream(Ctx.pStream);
	if (Ctx.dwVersion >= 36)
		Ctx.pStream->Read((char *)&m_dwNameFlags, sizeof(DWORD));
	else
		m_dwNameFlags = 0;

	//	Stuff

	CSystem::ReadSovereignRefFromStream(Ctx, &m_pSovereign);
	Ctx.pStream->Read((char *)&m_Scale, sizeof(DWORD));
	Ctx.pStream->Read((char *)&m_rMass, sizeof(Metric));

	//	Read composite image selector

	if (Ctx.dwVersion >= 61)
		m_ImageSelector.ReadFromStream(Ctx);
	else
		{
		Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
		IImageEntry *pRoot = m_pType->GetImageRoot();
		DWORD dwID = (pRoot ? pRoot->GetID() : DEFAULT_SELECTOR_ID);
		m_ImageSelector.AddVariant(dwID, dwLoad);
		}

	//	Animation data

	Ctx.pStream->Read((char *)&m_iDestroyedAnimation, sizeof(DWORD));

	//	Load orbit

	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	if (dwLoad != 0xffffffff)
		{
		m_pMapOrbit = new COrbit;
		Ctx.pStream->Read((char *)m_pMapOrbit, sizeof(COrbit));
		}

	//	More stuff

	Ctx.pStream->Read((char *)&m_xMapLabel, sizeof(DWORD));
	Ctx.pStream->Read((char *)&m_yMapLabel, sizeof(DWORD));

	//	Load the stargate info

	if (Ctx.dwVersion >= 16)
		{
		m_sStargateDestNode.ReadFromStream(Ctx.pStream);
		m_sStargateDestEntryPoint.ReadFromStream(Ctx.pStream);
		}
	else
		{
		CString sStargate;
		sStargate.ReadFromStream(Ctx.pStream);

		if (!sStargate.IsBlank())
			{
			CTopologyNode *pNode = Ctx.pSystem->GetStargateDestination(sStargate, &m_sStargateDestEntryPoint);
			if (pNode)
				m_sStargateDestNode = pNode->GetID();
			}
		}

	//	Armor class

	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	if (dwLoad != 0xffffffff)
		m_pArmorClass = g_pUniverse->FindArmor(dwLoad);

	Ctx.pStream->Read((char *)&m_iHitPoints, sizeof(DWORD));
	if (Ctx.dwVersion >= 77)
		Ctx.pStream->Read((char *)&m_iMaxHitPoints, sizeof(DWORD));
	else
		m_iMaxHitPoints = m_pType->GetMaxHitPoints();
	Ctx.pStream->Read((char *)&m_iStructuralHP, sizeof(DWORD));
	if (Ctx.dwVersion >= 31)
		Ctx.pStream->Read((char *)&m_iMaxStructuralHP, sizeof(DWORD));

	//	Devices

	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	if (dwLoad)
		{
		m_pDevices = new CInstalledDevice [dwLoad];

		for (i = 0; i < (int)dwLoad; i++)
			m_pDevices[i].ReadFromStream(this, Ctx);
		}

	//	Overlays

	if (Ctx.dwVersion >= 56)
		m_Overlays.ReadFromStream(Ctx, this);

	//	Registered objects / subscriptions

	if (Ctx.dwVersion < 77)
		{
		DWORD dwCount;
		Ctx.pStream->Read((char *)&dwCount, sizeof(DWORD));
		if (dwCount)
			{
			for (i = 0; i < (int)dwCount; i++)
				{
				DWORD dwObjID;
				Ctx.pStream->Read((char *)&dwObjID, sizeof(DWORD));

				TArray<CSpaceObject *> *pList = Ctx.Subscribed.SetAt(dwObjID);
				pList->Insert(this);
				}
			}
		}

	//	Docking ports

	m_DockingPorts.ReadFromStream(this, Ctx);

	//	Subordinates

	if (Ctx.dwVersion >= 47)
		{
		CSystem::ReadObjRefFromStream(Ctx, &m_pTarget);
		CSystem::ReadObjRefFromStream(Ctx, &m_pBase);
		}
	else
		{
		m_pTarget = NULL;
		m_pBase = NULL;
		}

	m_Subordinates.ReadFromStream(Ctx);
	m_Targets.ReadFromStream(Ctx);

	//	More

	if (Ctx.dwVersion >= 9)
		m_Blacklist.ReadFromStream(Ctx);
	else
		{
		CSovereign *pBlacklist;
		int iCounter;

		CSystem::ReadSovereignRefFromStream(Ctx, &pBlacklist);
		Ctx.pStream->Read((char *)&iCounter, sizeof(DWORD));

		if (pBlacklist != NULL)
			m_Blacklist.Blacklist();
		}

	if (Ctx.dwVersion >= 3)
		Ctx.pStream->Read((char *)&m_iAngryCounter, sizeof(DWORD));
	else
		m_iAngryCounter = 0;

	if (Ctx.dwVersion >= 9)
		Ctx.pStream->Read((char *)&m_iReinforceRequestCount, sizeof(DWORD));
	else
		m_iReinforceRequestCount = 0;

	//	Money

	if (Ctx.dwVersion >= 62)
		{
		Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
		if (dwLoad != 0xffffffff)
			{
			m_pMoney = new CCurrencyBlock;
			m_pMoney->ReadFromStream(Ctx);
			}
		else
			m_pMoney = NULL;
		}
	else if (Ctx.dwVersion >= 12)
		{
		Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
		if (dwLoad)
			{
			m_pMoney = new CCurrencyBlock;
			m_pMoney->SetCredits(CONSTLIT("credit"), dwLoad);
			}
		else
			m_pMoney = NULL;
		}
	else
		m_pMoney = NULL;

	//	Trade desc

	if (Ctx.dwVersion >= 37)
		{
		Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
		if (dwLoad != 0xffffffff)
			{
			m_pTrade = new CTradingDesc;
			m_pTrade->ReadFromStream(Ctx);
			}
		else
			m_pTrade = NULL;
		}
	else
		m_pTrade = NULL;

	//	Wreck UNID

	Ctx.pStream->Read((char *)&m_dwWreckUNID, sizeof(DWORD));

	//	Previous versions didn't have m_ImageSelector

	if (m_dwWreckUNID && Ctx.dwVersion < 61)
		{
		CShipClass *pClass = g_pUniverse->FindShipClass(m_dwWreckUNID);
		if (pClass)
			{
			m_ImageSelector.DeleteAll();
			m_ImageSelector.AddShipwreck(DEFAULT_SELECTOR_ID, pClass);
			}
		}

	//	Flags

	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	m_fArmed =			((dwLoad & 0x00000001) ? true : false);
	m_fKnown =			((dwLoad & 0x00000002) ? true : false);
	m_fNoMapLabel =		((dwLoad & 0x00000004) ? true : false);
	m_fRadioactive =	((dwLoad & 0x00000008) ? true : false);
	//	0x00000010 UNUSED m_fCustomImage
	m_fActive =			((dwLoad & 0x00000020) ? true : false);
	m_fNoReinforcements =((dwLoad & 0x00000040) ? true : false);
	m_fReconned =		((dwLoad & 0x00000080) ? true : false);
	m_fFireReconEvent =	((dwLoad & 0x00000100) ? true : false);
	bool fNoArticle =	((dwLoad & 0x00000200) ? true : false);
	m_fImmutable =		((dwLoad & 0x00000400) ? true : false);
	m_fExplored =		((dwLoad & 0x00000800) ? true : false);

	//	Init name flags

	if (Ctx.dwVersion < 36 && m_dwNameFlags == 0)
		{
		if (fNoArticle)
			m_dwNameFlags = nounNoArticle;
		else if (m_pType->HasRandomNames())
			m_dwNameFlags = m_pType->GetRandomNameFlags();
		else
			m_dwNameFlags = m_pType->GetNameFlags();
		}

	//	Previous versions did not store m_fImmutable

	if (Ctx.dwVersion < 77)
		m_fImmutable = m_pType->IsImmutable();

	//	If this is a world or a star, create a small image

	CreateMapImage();

	//	For previous version, we have to set CannotBeHit manually

	if (m_pType->IsBackgroundObject())
		SetCannotBeHit();
	}

void CStation::OnSetEventFlags (void)

//	OnSetEventFlags
//
//	Sets the flags the cache whether the object has certain events

	{
	}

void CStation::OnStationDestroyed (const SDestroyCtx &Ctx)

//	OnStationDestroyed
//
//	Station in the system has been destroyed

	{
	}

void CStation::OnSystemCreated (void)

//	OnSystemCreated
//
//	Called when the system is created

	{
	FinishCreation();
	}

void CStation::OnUpdate (SUpdateCtx &Ctx, Metric rSecondsPerTick)

//	OnUpdate
//
//	Update

	{
	int i;
	int iTick = GetSystem()->GetTick() + GetDestiny();

	//	Basic update

	UpdateAttacking(iTick);
	m_DockingPorts.UpdateAll(Ctx, this);
	UpdateReinforcements(iTick);

	//	Trade

	if ((iTick % TRADE_UPDATE_FREQUENCY) == 0
			 && !IsAbandoned())
		{
		if (m_pTrade)
			{
			m_pTrade->OnUpdate(this);

			if (!IsPlayerDocked())
				m_pTrade->RefreshInventory(this, INVENTORY_REFRESHED_PER_UPDATE);
			}

		CTradingDesc *pTrade = m_pType->GetTradingDesc();
		if (pTrade)
			{
			//	If we have a trade desc override, then don't update. [Otherwise
			//	we will replenish currency at double the rate.]

			if (m_pTrade == NULL)
				pTrade->OnUpdate(this);

			//	But we still need to refresh inventory, since the base 
			//	may contain items not in the override.
			//
			//	LATER: Note that this doesn't handle the case where we try
			//	to override a specific item. The fix is to add the concept
			//	of overriding directly into the class.

			if (!IsPlayerDocked())
				pTrade->RefreshInventory(this, INVENTORY_REFRESHED_PER_UPDATE);
			}
		}

	//	Update each device

	if (m_pDevices)
		{
		bool bSourceDestroyed = false;
		for (i = 0; i < maxDevices; i++)
			{
			m_pDevices[i].Update(this, iTick, &bSourceDestroyed);
			if (bSourceDestroyed)
				return;
			}
		}

	//	Update destroy animation

	if (m_iDestroyedAnimation)
		{
		int iTick, iRotation;
		const CObjectImageArray &Image = GetImage(&iTick, &iRotation);
		int cxWidth = RectWidth(Image.GetImageRect());

		CEffectCreator *pEffect = g_pUniverse->FindEffectType(g_StationDestroyedUNID);
		if (pEffect)
			{
			for (int i = 0; i < mathRandom(1, 3); i++)
				{
				CVector vPos = GetPos() 
						+ PolarToVector(mathRandom(0, 359), g_KlicksPerPixel * mathRandom(1, cxWidth / 3));

				pEffect->CreateEffect(GetSystem(),
						this,
						vPos,
						GetVel(),
						0);
				}
			}

		m_iDestroyedAnimation--;
		}

	//	If we're moving, slow down

	if (IsMobile() && !GetVel().IsNull())
		{
		//	If we're moving really slowly, force to 0. We do this so that we can optimize calculations
		//	and not have to compute wreck movement down to infinitesimal distances.

		if (GetVel().Length2() < g_MinSpeed2)
			SetVel(NullVector);
		else
			SetVel(CVector(GetVel().GetX() * g_SpaceDragFactor, GetVel().GetY() * g_SpaceDragFactor));
		}

	//	Overlays

	if (!m_Overlays.IsEmpty())
		{
		bool bModified;
		m_Overlays.Update(this, &bModified);
		if (CSpaceObject::IsDestroyedInUpdate())
			return;
		else if (bModified)
			{
#if 0
			bWeaponStatusChanged = true;
			bArmorStatusChanged = true;
			bCalcDeviceBonus = true;
#endif
			}
		}
	}

void CStation::OnUpdateExtended (const CTimeSpan &ExtraTime)

//	OnUpdateExtended
//
//	Update after an extended period of time

	{
	//	Refresh inventory, if necessary

	CTradingDesc *pTrade = m_pType->GetTradingDesc();
	if ((pTrade || m_pTrade) && ExtraTime.Days() > 0 && !IsAbandoned())
		{
		//	Compute the percent of the inventory that need to refresh

		int iRefreshPercent;
		if (ExtraTime.Days() >= DAYS_TO_REFRESH_INVENTORY)
			iRefreshPercent = 100;
		else
			iRefreshPercent = 100 * ExtraTime.Days() / DAYS_TO_REFRESH_INVENTORY;

		//	Do it

		if (m_pTrade)
			m_pTrade->RefreshInventory(this, iRefreshPercent);

		if (pTrade)
			pTrade->RefreshInventory(this, iRefreshPercent);
		}
	}

void CStation::OnWriteToStream (IWriteStream *pStream)

//	OnWriteToStream
//
//	Write the object's data to stream
//
//	DWORD		station type UNID
//	CString		m_sName
//	DWORD		m_dwNameFlags
//	DWORD		sovereign UNID
//	DWORD		m_Scale
//	Metric		m_rMass
//	CCompositeImageSelector m_ImageSelector
//	DWORD		m_iDestroyedAnimation
//	DWORD		1 if orbit, 0xffffffff if no orbit
//	Orbit		System orbit
//	DWORD		m_xMapLabel
//	DWORD		m_yMapLabel
//	CString		m_sStargateDestNode
//	CString		m_sStargateDestEntryPoint
//	DWORD		armor class UNID, 0xffffffff if no armor
//	DWORD		m_iHitPoints
//	DWORD		m_iMaxHitPoints
//	DWORD		m_iStructuralHP
//	DWORD		m_iMaxStructuralHP
//
//	DWORD		device: class UNID
//	DWORD		device: m_dwData
//	DWORD		device: low = m_iPosAngle; hi = m_iPosRadius
//	DWORD		device: low = m_iMinFireArc; hi = m_iMaxFireArc
//	DWORD		device: low = m_iActivationDelay
//	DWORD		device: flags
//
//	Overlays	m_Overlays
//
//	DWORD		No of registered objects
//	DWORD		registered object (CSpaceObject ref)
//
//	DWORD		No of docking ports
//	DWORD		port: iStatus
//	DWORD		port: pObj (CSpaceObject ref)
//	Vector		port: vPos
//
//	DWORD		m_pTarget
//	DWORD		m_pBase
//
//	DWORD		No of subordinates
//	DWORD		subordinate (CSpaceObject ref)
//
//	DWORD		No of targets
//	DWORD		target (CSpaceObject ref)
//
//	CAttackDetector m_Blacklist
//	DWORD		m_iAngryCounter
//	DWORD		m_iReinforceRequestCount
//	CCurrencyBlock	m_pMoney
//	CTradingDesc	m_pTrade
//
//	DWORD		m_dwWreckUNID
//	DWORD		flags

	{
	int i;
	DWORD dwSave;

	dwSave = m_pType->GetUNID();
	pStream->Write((char *)&dwSave, sizeof(DWORD));
	m_sName.WriteToStream(pStream);
	pStream->Write((char *)&m_dwNameFlags, sizeof(DWORD));
	GetSystem()->WriteSovereignRefToStream(m_pSovereign, pStream);
	pStream->Write((char *)&m_Scale, sizeof(DWORD));
	pStream->Write((char *)&m_rMass, sizeof(Metric));
	m_ImageSelector.WriteToStream(pStream);
	pStream->Write((char *)&m_iDestroyedAnimation, sizeof(DWORD));

	if (m_pMapOrbit)
		{
		dwSave = 1;
		pStream->Write((char *)&dwSave, sizeof(DWORD));
		pStream->Write((char *)m_pMapOrbit, sizeof(COrbit));
		}
	else
		{
		dwSave = 0xffffffff;
		pStream->Write((char *)&dwSave, sizeof(DWORD));
		}

	pStream->Write((char *)&m_xMapLabel, sizeof(DWORD));
	pStream->Write((char *)&m_yMapLabel, sizeof(DWORD));
	m_sStargateDestNode.WriteToStream(pStream);
	m_sStargateDestEntryPoint.WriteToStream(pStream);

	if (m_pArmorClass)
		dwSave = m_pArmorClass->GetUNID();
	else
		dwSave = 0xffffffff;
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	pStream->Write((char *)&m_iHitPoints, sizeof(DWORD));
	pStream->Write((char *)&m_iMaxHitPoints, sizeof(DWORD));
	pStream->Write((char *)&m_iStructuralHP, sizeof(DWORD));
	pStream->Write((char *)&m_iMaxStructuralHP, sizeof(DWORD));

	dwSave = (m_pDevices ? maxDevices : 0);
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	for (i = 0; i < (int)dwSave; i++)
		m_pDevices[i].WriteToStream(pStream);

	m_Overlays.WriteToStream(pStream);

	m_DockingPorts.WriteToStream(this, pStream);

	GetSystem()->WriteObjRefToStream(m_pTarget, pStream, this);
	GetSystem()->WriteObjRefToStream(m_pBase, pStream, this);
	m_Subordinates.WriteToStream(GetSystem(), pStream);
	m_Targets.WriteToStream(GetSystem(), pStream);

	m_Blacklist.WriteToStream(pStream);
	pStream->Write((char *)&m_iAngryCounter, sizeof(DWORD));
	pStream->Write((char *)&m_iReinforceRequestCount, sizeof(DWORD));

	//	Money

	if (m_pMoney)
		{
		dwSave = 1;
		pStream->Write((char *)&dwSave, sizeof(DWORD));

		m_pMoney->WriteToStream(pStream);
		}
	else
		{
		dwSave = 0xffffffff;
		pStream->Write((char *)&dwSave, sizeof(DWORD));
		}

	//	Trade desc

	if (m_pTrade)
		{
		dwSave = 1;
		pStream->Write((char *)&dwSave, sizeof(DWORD));

		m_pTrade->WriteToStream(pStream);
		}
	else
		{
		dwSave = 0xffffffff;
		pStream->Write((char *)&dwSave, sizeof(DWORD));
		}

	pStream->Write((char *)&m_dwWreckUNID, sizeof(DWORD));

	dwSave = 0;
	dwSave |= (m_fArmed ?				0x00000001 : 0);
	dwSave |= (m_fKnown ?				0x00000002 : 0);
	dwSave |= (m_fNoMapLabel ?			0x00000004 : 0);
	dwSave |= (m_fRadioactive ?			0x00000008 : 0);
	//	0x00000010 retired
	dwSave |= (m_fActive ?				0x00000020 : 0);
	dwSave |= (m_fNoReinforcements ?	0x00000040 : 0);
	dwSave |= (m_fReconned ?			0x00000080 : 0);
	dwSave |= (m_fFireReconEvent ?		0x00000100 : 0);
	//	0x00000200 retired
	dwSave |= (m_fImmutable ?			0x00000400 : 0);
	dwSave |= (m_fExplored ?			0x00000800 : 0);
	pStream->Write((char *)&dwSave, sizeof(DWORD));
	}

void CStation::PaintLRS (CG16bitImage &Dest, int x, int y, const ViewportTransform &Trans)

//	PaintLRS
//
//	Paints the object on an LRS

	{
	if (m_pType->IsVirtual())
		return;

	//	Paint worlds and stars fully

	if (m_Scale == scaleWorld || m_Scale == scaleStar)
		{
		Dest.ColorTransBlt(0, 0, m_MapImage.GetWidth(), m_MapImage.GetHeight(), 255,
				m_MapImage,
				x - (m_MapImage.GetWidth() / 2),
				y - (m_MapImage.GetHeight() / 2));
		}

	//	Other kinds of stations are just dots

	else
		{
		//	Paint red if enemy, green otherwise

		WORD wColor = GetSymbolColor();
		if (m_Scale == scaleStructure && m_rMass > 100000.0)
			{
			if (IsActiveStargate())
				{
				Dest.DrawDot(x, y, wColor, CG16bitImage::markerSmallSquare);
				Dest.DrawDot(x, y, wColor, CG16bitImage::markerMediumCross);
				}
			else if (!IsAbandoned() || IsImmutable())
				{
				Dest.DrawDot(x+1, y+1, 0, CG16bitImage::markerSmallSquare);
				Dest.DrawDot(x, y, wColor, CG16bitImage::markerSmallFilledSquare);
				}
			else
				{
				Dest.DrawDot(x+1, y+1, 0, CG16bitImage::markerSmallSquare);
				Dest.DrawDot(x, y, wColor, CG16bitImage::markerSmallSquare);
				}
			}
		else
			{
			if (!m_pType->ShowsMapIcon() && m_fExplored)
				Dest.DrawDot(x, y, 
						CG16bitImage::RGBValue(128, 128, 128), 
						CG16bitImage::markerTinyCircle);
			else
				Dest.DrawDot(x, y, 
						wColor, 
						CG16bitImage::markerTinyCircle);
			}
		}
	}

void CStation::PaintMap (CG16bitImage &Dest, int x, int y, const ViewportTransform &Trans)

//	PaintMap
//
//	Paint the station

	{
	if (m_pType->IsVirtual())
		return;

	//	Draw an orbit

	if (m_pMapOrbit)
		m_pMapOrbit->Paint(Dest, Trans, RGB_ORBIT_LINE);

	//	Draw the station

	if (m_Scale == scaleWorld)
		Dest.ColorTransBlt(0, 0, m_MapImage.GetWidth(), m_MapImage.GetHeight(), 255,
				m_MapImage,
				x - (m_MapImage.GetWidth() / 2),
				y - (m_MapImage.GetHeight() / 2));

	else if (m_Scale == scaleStar)
		Dest.BltLighten(0, 0, m_MapImage.GetWidth(), m_MapImage.GetHeight(), 255,
				m_MapImage,
				x - (m_MapImage.GetWidth() / 2),
				y - (m_MapImage.GetHeight() / 2));

	else if (m_pType->ShowsMapIcon() && m_fKnown)
		{
		//	Figure out the color

		WORD wColor;
		if (IsEnemy(GetUniverse()->GetPOV()))
			wColor = CG16bitImage::RGBValue(255, 0, 0);
		else
			wColor = CG16bitImage::RGBValue(0, 192, 0);

		//	Paint the marker

		if (m_Scale == scaleStructure && m_rMass > 100000.0)
			{
			if (IsActiveStargate())
				{
				Dest.DrawDot(x, y, wColor, CG16bitImage::markerSmallSquare);
				Dest.DrawDot(x, y, wColor, CG16bitImage::markerMediumCross);
				}
			else if (!IsAbandoned() || IsImmutable())
				{
				Dest.DrawDot(x+1, y+1, 0, CG16bitImage::markerSmallSquare);
				Dest.DrawDot(x, y, wColor, CG16bitImage::markerSmallFilledSquare);
				}
			else
				{
				Dest.DrawDot(x+1, y+1, 0, CG16bitImage::markerSmallSquare);
				Dest.DrawDot(x, y, wColor, CG16bitImage::markerSmallSquare);
				}
			}
		else
			Dest.DrawDot(x, y, 
					wColor, 
					CG16bitImage::markerSmallRound);

		//	Paint the label

		if (!m_fNoMapLabel)
			{
			if (m_sMapLabel.IsBlank())
				{
				DWORD dwFlags;
				CString sName = GetName(&dwFlags);
				m_sMapLabel = ::ComposeNounPhrase(sName, 1, NULL_STR, dwFlags, nounTitleCapitalize);
				}

			g_pUniverse->GetNamedFont(CUniverse::fontMapLabel).DrawText(Dest, 
					x + m_xMapLabel + 1, 
					y + m_yMapLabel + 1, 
					0,
					m_sMapLabel);
			g_pUniverse->GetNamedFont(CUniverse::fontMapLabel).DrawText(Dest, 
					x + m_xMapLabel, 
					y + m_yMapLabel, 
					RGB_MAP_LABEL,
					m_sMapLabel);
			}
		}
	}

bool CStation::PointInObject (const CVector &vObjPos, const CVector &vPointPos)

//	PointInObject
//
//	Returns TRUE if the given point is inside the object

	{
	//	Figure out the coordinates of vPos relative to the center of the
	//	ship, in pixels.

	CVector vOffset = vPointPos - vObjPos;
	int x = (int)((vOffset.GetX() / g_KlicksPerPixel) + 0.5);
	int y = -(int)((vOffset.GetY() / g_KlicksPerPixel) + 0.5);

	//	Ask the image if the point is inside or not

	int iTick, iVariant;
	const CObjectImageArray &Image = GetImage(&iTick, &iVariant);

	return Image.PointInImage(x, y, iTick, iVariant);
	}

bool CStation::PointInObject (SPointInObjectCtx &Ctx, const CVector &vObjPos, const CVector &vPointPos)

//	PointInObject
//
//	Returns TRUE if the given point is inside the object

	{
	//	Figure out the coordinates of vPos relative to the center of the
	//	ship, in pixels.

	CVector vOffset = vPointPos - vObjPos;
	int x = (int)((vOffset.GetX() / g_KlicksPerPixel) + 0.5);
	int y = -(int)((vOffset.GetY() / g_KlicksPerPixel) + 0.5);

	return Ctx.pObjImage->PointInImage(Ctx, x, y);
	}

void CStation::PointInObjectInit (SPointInObjectCtx &Ctx)

//	PointInObjectInit
//
//	Initializes context for PointInObject (for improved performance in loops)

	{
	int iTick, iVariant;
	Ctx.pObjImage = &GetImage(&iTick, &iVariant);

	Ctx.pObjImage->PointInImageInit(Ctx, iTick, iVariant);
	}

void CStation::RaiseAlert (CSpaceObject *pTarget)

//	RaiseAlert
//
//	Raise an alert (if we're that kind of station)

	{
	if (pTarget == NULL)
		return;
	
	if (m_Targets.FindObj(pTarget))
		return;

	//	Add this to our target list

	m_Targets.Add(pTarget);

#ifdef DEBUG_ALERTS
	g_pUniverse->DebugOutput("%d: Raising alert...", this);
#endif

	//	Tell all other friendly stations in the system that they
	//	should attack the target.

	CSovereign *pSovereign = GetSovereign();
	for (int i = 0; i < GetSystem()->GetObjectCount(); i++)
		{
		CSpaceObject *pObj = GetSystem()->GetObject(i);

		if (pObj 
				&& pObj->GetCategory() == catStation
				&& pObj->GetSovereign() == pSovereign
				&& pObj->CanAttack()
				&& pObj != this)
			Communicate(pObj, msgAttackDeter, pTarget);
		}
	}

bool CStation::RemoveSubordinate (CSpaceObject *pSubordinate)

//	RemoveSubordinate
//
//	If the object is a subordinate, it removes it (and returns TRUE)

	{
	return m_Subordinates.Remove(pSubordinate);
	}

bool CStation::RequestDock (CSpaceObject *pObj, int iPort)

//	RequestDock
//
//	pObj requests docking services with the station. Returns TRUE
//	if docking is engaged.

	{
	//	If time has stopped for this object, then we cannot allow docking

	if (IsTimeStopped())
		{
		pObj->SendMessage(this, CONSTLIT("Unable to dock"));
		return false;
		}

	//	If the object requesting docking services is an enemy,
	//	then deny docking services.

	if (!IsAbandoned() 
			&& !m_pType->IsEnemyDockingAllowed()
			&& (IsEnemy(pObj) || IsBlacklisted(pObj)))
		{
		pObj->SendMessage(this, CONSTLIT("Docking request denied"));
		return false;
		}

	//	If we don't have any docking screens then do not let the
	//	object dock.

	if (!SupportsDocking())
		{
		pObj->SendMessage(this, CONSTLIT("No docking services available"));
		return false;
		}

	//	Get the nearest free port

	return m_DockingPorts.RequestDock(this, pObj, iPort);
	}

bool CStation::RequestGate (CSpaceObject *pObj)

//	RequestGate
//
//	Requests that the given object be transported through the gate

	{
	//	Create gating effect

	if (!pObj->IsVirtual())
		{
		CEffectCreator *pEffect = m_pType->GetGateEffect();
		if (pEffect)
			pEffect->CreateEffect(GetSystem(),
					NULL,
					GetPos(),
					GetVel(),
					0);
		}

	//	Get the destination node for this gate
	//	(If pNode == NULL then it means that we are gating to nowhere;
	//	This is used by ships that "gate" back into their carrier or their
	//	station.)

	CTopologyNode *pNode = g_pUniverse->FindTopologyNode(m_sStargateDestNode);

	//	Let the object gate itself

	pObj->EnterGate(pNode, m_sStargateDestEntryPoint, this);

	return true;
	}

void CStation::SetAngry (void)

//	SetAngry
//
//	Station is angry

	{
	if (m_iAngryCounter < MAX_ANGER)
		m_iAngryCounter = Max(MIN_ANGER, m_iAngryCounter + ANGER_INC);
	}

void CStation::SetFlotsamImage (CItemType *pItemType)

//	SetFlotsamImage
//
//	Sets the image for the station

	{
	m_ImageSelector.DeleteAll();
	m_ImageSelector.AddFlotsam(DEFAULT_SELECTOR_ID, pItemType);

	//	Set bounds

	//	We don't care about iTick or iRotation because 
	//	the image rect dimensions won't change
	const CObjectImageArray &Image = GetImage(NULL, NULL);

	const RECT &rcImage = Image.GetImageRect();
	SetBounds(rcImage);
	}

int CStation::GetImageVariant (void)

//	GetImageVariant
//
//	Returns the current image variant

	{
	IImageEntry *pRoot = m_pType->GetImageRoot();
	DWORD dwID = (pRoot ? pRoot->GetID() : DEFAULT_SELECTOR_ID);
	return m_ImageSelector.GetVariant(dwID);
	}

void CStation::SetImageVariant (int iVariant)

//	SetImageVariant
//
//	Sets the given variant
	
	{
	IImageEntry *pRoot = m_pType->GetImageRoot();
	DWORD dwID = (pRoot ? pRoot->GetID() : DEFAULT_SELECTOR_ID);

	m_ImageSelector.DeleteAll();
	m_ImageSelector.AddVariant(dwID, iVariant);

	//	Set bounds

	//	We don't care about iTick or iRotation because 
	//	the image rect dimensions won't change
	const CObjectImageArray &Image = GetImage(NULL, NULL);

	const RECT &rcImage = Image.GetImageRect();
	SetBounds(rcImage);
	}

void CStation::SetMapOrbit (const COrbit &oOrbit)

//	SetMapOrbit
//
//	Sets the orbit description

	{
	if (m_pMapOrbit)
		delete m_pMapOrbit;

	m_pMapOrbit = new COrbit(oOrbit);
	}

void CStation::SetName (const CString &sName, DWORD dwFlags)

//	SetName
//
//	Sets the name of the station

	{
	m_sName = sName;
	m_dwNameFlags = dwFlags;
	}

void CStation::SetStargate (const CString &sDestNode, const CString &sDestEntryPoint)

//	SetStargate
//
//	Sets the stargate label

	{
	m_sStargateDestNode = sDestNode;
	m_sStargateDestEntryPoint = sDestEntryPoint;
	}

void CStation::SetTradeDesc (CEconomyType *pCurrency, int iMaxCurrency, int iReplenishCurrency)

//	SetTradeDesc
//
//	Overrides trade desc

	{
	AllocTradeOverride();
	m_pTrade->SetEconomyType(pCurrency);
	m_pTrade->SetMaxCurrency(iMaxCurrency);
	m_pTrade->SetReplenishCurrency(iReplenishCurrency);

	//	This call will set up the currency.

	m_pTrade->OnCreate(this);
	}

void CStation::SetWreckImage (CShipClass *pWreckClass)

//	SetImage
//
//	Sets the image for the station

	{
	m_ImageSelector.DeleteAll();
	m_ImageSelector.AddShipwreck(DEFAULT_SELECTOR_ID, pWreckClass);

	//	Set bounds

	//	We don't care about iTick or iRotation because 
	//	the image rect dimensions won't change
	const CObjectImageArray &Image = GetImage(NULL, NULL);

	const RECT &rcImage = Image.GetImageRect();
	SetBounds(rcImage);
	}

void CStation::SetWreckParams (CShipClass *pWreckClass, CShip *pShip)

//	SetWreckParams
//
//	Sets the mass and name for the station based on the wreck class

	{
	//	If the station doesn't have a name, set it now

	if (!IsNameSet())
		{
		CString sName;
		DWORD dwFlags;

		//	If the ship has a proper name, then use that

		if (pShip && pWreckClass->HasShipName())
			{
			DWORD dwNounFlags;
			sName = pShip->GetName(&dwNounFlags);

			if (dwNounFlags & nounPersonalName)
				sName = strPatternSubst(CONSTLIT("wreck of %s's %s"),
						ComposeNounPhrase(sName, 1, CString(), dwNounFlags, nounArticle),
						(!pWreckClass->GetShipTypeName().IsBlank() ? pWreckClass->GetShipTypeName() : pWreckClass->GetShortName()));
			else
				sName = strPatternSubst(CONSTLIT("wreck of %s"),
						ComposeNounPhrase(sName, 1, CString(), dwNounFlags, nounArticle));

			dwFlags = nounDefiniteArticle;
			}

		//	Otherwise, use the generic name for the class

		else
			{
			sName = strPatternSubst(CONSTLIT("%s wreck"), pWreckClass->GetName());
			dwFlags = 0;
			}

		SetName(sName, dwFlags);
		}

	//	Set the mass

	SetMass(pWreckClass->GetHullMass());

	//	Set hit points for the structure

	int iHP = pWreckClass->GetMaxStructuralHitPoints();
	SetStructuralHitPoints(iHP);
	SetMaxStructuralHitPoints(iHP);

	//	Set the wreck UNID

	m_dwWreckUNID = pWreckClass->GetUNID();
	}

bool CStation::SetProperty (const CString &sName, ICCItem *pValue, CString *retsError)

//	SetProperty
//
//	Sets a station property

	{
	CCodeChain &CC = g_pUniverse->GetCC();

	if (strEquals(sName, PROPERTY_HP))
		{
		//	Nil means that we don't want to make a change

		if (pValue->IsNil())
			return true;

		m_iHitPoints = Min(Max(0, pValue->GetIntegerValue()), m_iMaxHitPoints);
		return true;
		}
	else if (strEquals(sName, PROPERTY_IMMUTABLE))
		{
		m_fImmutable = !pValue->IsNil();
		return true;
		}
	else if (strEquals(sName, PROPERTY_MAX_HP))
		{
		//	Nil means that we don't want to make a change

		if (pValue->IsNil())
			return true;

		m_iMaxHitPoints = Max(0, pValue->GetIntegerValue());
		m_iHitPoints = Min(m_iHitPoints, m_iMaxHitPoints);
		return true;
		}
	else if (strEquals(sName, PROPERTY_MAX_STRUCTURAL_HP))
		{
		//	Nil means that we don't want to make a change

		if (pValue->IsNil())
			return true;

		m_iMaxStructuralHP = Max(0, pValue->GetIntegerValue());
		m_iStructuralHP = Min(m_iStructuralHP, m_iMaxStructuralHP);
		return true;
		}
	else if (strEquals(sName, PROPERTY_PLAYER_BACKLISTED))
		{
		CSpaceObject *pPlayer = g_pUniverse->GetPlayer();

		if (pValue->IsNil())
			ClearBlacklist(pPlayer);
		else
			Blacklist(pPlayer);

		return true;
		}
	else if (strEquals(sName, PROPERTY_STRUCTURAL_HP))
		{
		//	Nil means that we don't want to make a change

		if (pValue->IsNil())
			return true;

		m_iStructuralHP = Min(Max(0, pValue->GetIntegerValue()), m_iMaxStructuralHP);
		return true;
		}
	else
		return CSpaceObject::SetProperty(sName, pValue, retsError);
	}

void CStation::Undock (CSpaceObject *pObj)

//	Undock
//
//	Undocks from the station

	{
	m_DockingPorts.Undock(this, pObj);

	//	If we're set to destroy when empty AND we're empty
	//	AND no one else is docked, then destroy the station

	if (m_pType->IsDestroyWhenEmpty() 
			&& GetItemList().GetCount() == 0
			&& m_DockingPorts.GetPortsInUseCount(this) == 0)
		{
		Destroy(removedFromSystem, CDamageSource());
		}
	}

void CStation::UpdateAttacking (int iTick)

//	UpdateAttacking
//
//	Station attacks any enemies in range

	{
	DEBUG_TRY

	int i;
	
	//	Update blacklist counter
	//	NOTE: Once the player is blacklisted by this station, there is
	//	no way to get off the blacklist. (At least no automatic way).

	m_Blacklist.Update(iTick);

	//	If we're abandoned or if we have no weapons then
	//	there's nothing we can do

	if (IsAbandoned() || !m_fArmed || m_pType->IsVirtual())
		return;

	//	Compute the range at which we attack enemies

	Metric rAttackRange;
	if (m_iAngryCounter > 0)
		{
		rAttackRange = Max(MAX_ATTACK_DISTANCE, m_pType->GetMaxEffectiveRange());
		m_iAngryCounter--;
		}
	else
		rAttackRange = MAX_ATTACK_DISTANCE;

	//	Look for the nearest enemy ship to attack

	if ((iTick % STATION_SCAN_TARGET_FREQUENCY) == 0)
		{
		//	Look for a target

		m_pTarget = NULL;
		Metric rBestDist = rAttackRange * rAttackRange;
		CSystem *pSystem = GetSystem();
		for (i = 0; i < pSystem->GetObjectCount(); i++)
			{
			CSpaceObject *pObj = pSystem->GetObject(i);

			if (pObj
					&& pObj->GetCategory() == catShip
					&& (IsEnemy(pObj) || IsBlacklisted(pObj))
					&& pObj->CanAttack()
					&& pObj != this)
				{
				CVector vDist = pObj->GetPos() - GetPos();
				Metric rDist = vDist.Length2();

				if (rDist < rBestDist
						&& !pObj->IsEscortingFriendOf(this))
					{
					rBestDist = rDist;
					m_pTarget = pObj;
					}
				}
			}
		}

	//	Fire with all weapons (if we've got a target)

	if (m_pTarget && m_pDevices)
		{
		bool bSourceDestroyed = false;

		for (i = 0; i < maxDevices; i++)
			{
			CInstalledDevice *pWeapon = &m_pDevices[i];
			int iFireAngle;

			if (!pWeapon->IsEmpty() 
					&& (pWeapon->GetCategory() == itemcatWeapon || pWeapon->GetCategory() == itemcatLauncher)
					&& pWeapon->IsReady()
					&& pWeapon->IsWeaponAligned(this, m_pTarget, NULL, &iFireAngle)
					&& IsLineOfFireClear(pWeapon, m_pTarget, iFireAngle, rAttackRange))
				{
				pWeapon->SetFireAngle(iFireAngle);
				pWeapon->SetTarget(m_pTarget);
				pWeapon->Activate(this, m_pTarget, &bSourceDestroyed);
				if (bSourceDestroyed)
					return;

				pWeapon->SetTimeUntilReady(m_pType->GetFireRateAdj() * pWeapon->GetActivateDelay(this) / 10);
				}
			}
		}

	DEBUG_CATCH
	}

void CStation::UpdateReinforcements (int iTick)

//	UpdateReinforcements
//
//	Check to see if it is time to send reinforcements to the station

	{
	DEBUG_TRY

	//	Nothing to do if we're abandoned

	if (IsAbandoned())
		return;

	//	Repair damage

	if ((iTick % STATION_REPAIR_FREQUENCY) == 0)
		{
		//	Repair damage to station

		if (m_pType->GetRepairRate() > 0 && m_iHitPoints < m_iMaxHitPoints)
			m_iHitPoints = Min(m_iMaxHitPoints, m_iHitPoints + m_pType->GetRepairRate());

		//	Repair damage to ships

		m_DockingPorts.RepairAll(this, m_pType->GetShipRepairRate());
		}

	//	Construction

	if (m_pType->GetShipConstructionRate()
			&& (iTick % m_pType->GetShipConstructionRate()) == 0)
		{
		//	Iterate over all ships and count the number that are
		//	associated with the station.

		int iCount = CalcNumberOfShips();

		//	If we already have the maximum number, then don't bother

		if (iCount < m_pType->GetMaxShipConstruction())
			{
			SShipCreateCtx Ctx;
			Ctx.pSystem = GetSystem();
			Ctx.pBase = this;
			Ctx.pGate = this;
			m_pType->GetConstructionTable()->CreateShips(Ctx);
			}
		}

	//	Get reinforcements

	if ((iTick % STATION_REINFORCEMENT_FREQUENCY) == 0
			&& m_pType->GetMinShips() > 0
			&& !m_fNoReinforcements)
		{
		//	Iterate over all ships and count the number that are
		//	associated with the station.

		int iCount = CalcNumberOfShips();

		//	If we don't have the minimum number of ships at the
		//	station then send reinforcements.

		if (iCount < m_pType->GetMinShips())
			{
			//	If we've requested several rounds of reinforcements but have
			//	never received any, then it's likely that they are being
			//	destroyed at the gate, so we stop requesting so many

			if (m_iReinforceRequestCount > 0)
				{
				int iLongTick = (iTick / STATION_REINFORCEMENT_FREQUENCY);
				int iCycle = Min(32, m_iReinforceRequestCount * m_iReinforceRequestCount);
				if ((iLongTick % iCycle) != 0)
					return;
				}

			//	Find a stargate

			CSpaceObject *pGate = GetNearestStargate(true);
			if (pGate == NULL)
				return;

			//	Generate reinforcements

			SShipCreateCtx Ctx;
			Ctx.pSystem = GetSystem();
			Ctx.pBase = this;
			Ctx.pGate = pGate;
			m_pType->GetReinforcementsTable()->CreateShips(Ctx);

			//	Increment counter

			m_iReinforceRequestCount++;
			}

		//	If we have the required number of ships, then reset the reinforcement
		//	request count

		else
			{
			m_iReinforceRequestCount = 0;
			}
		}

	//	Attack targets on the target list

	if ((iTick % STATION_TARGET_FREQUENCY) == 0)
		{
		int i;

#ifdef DEBUG_ALERTS
		g_pUniverse->DebugOutput("%d: Attack target list", this);
#endif

		for (i = 0; i < m_Targets.GetCount(); i++)
			{
			CSpaceObject *pTarget = m_Targets.GetObj(i);

			int iLeft = mathRandom(3, 5);
			int iCount = m_Subordinates.GetCount();
			for (int j = 0; j < iCount && iLeft > 0; j++)
				{
				if (Communicate(m_Subordinates.GetObj(j), 
						msgDestroyBroadcast, 
						pTarget) == resAck)
					{
					iLeft--;
#ifdef DEBUG_ALERTS
					g_pUniverse->DebugOutput("   %d acknowledges attack order", m_Subordinates.GetObj(i));
#endif
					}
				}
			}
		}

	DEBUG_CATCH
	}
