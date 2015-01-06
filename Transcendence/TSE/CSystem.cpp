//	CSystem.cpp
//
//	CSystem class
//
//	VERSION HISTORY
//
//	 0: 0.95
//	 1: 0.95a
//	 2: 0.96a
//		m_pEncounterInfo in CShip
//		m_rItemMass in CShip
//
//	 3: 0.97
//		m_sDeferredDestruction string in CPlayerShipController.
//		m_iAngryCounter in CStation
//		m_vOldPos in CParticleDamage
//
//	 4: 0.97
//		m_iCountdown in CStandardShipAI
//
//	 5: 0.97
//		m_iStealth in CShip
//
//	 6-14: 0.98
//		m_Blacklist in CBaseShipAI
//		m_iDeviceCount in CShip
//		m_sName in CMarker
//		m_rKlicksPerPixel in CSystem
//		m_rTimeScale in CSystem
//		m_Blacklist in CStation
//		m_iReinforceRequestCount in CStation
//		m_NavPaths in CSystem
//		m_pNavPath in CBaseShipAI
//		m_iNavPathPos in CBaseShipAI
//		m_iBalance in CStation
//		m_dwID in CSpaceObject
//		m_vStart in CNavigationPath
//		m_pCommandCode in CStandardShipAI
//		m_fIdentified in CShip
//
//	15: 0.98a
//		UNIDs for painters are saved as strings
//
//	16: 0.98c
//		m_sStargateDestNode in CStation
//		m_sStargateDestEntryPoint in CStation
//
//	17: 0.98c
//		m_iRotationCount in CObjectImageArray
//
//	18: 0.99
//		m_iCause in DamageDesc
//		m_iCause in CAreaDamage
//		m_iCause in CMissile
//		m_iCause in CBeam
//		m_iCause in CParticleDamage
//		m_iCause in CRadiusDamage
//
//	19: 0.99
//		m_pTarget in CRadiusDamage
//
//	20: 0.99
//		m_iTick in CShockwavePainter
//
//	21: 0.99
//		m_iInitialDelay in CAreaDamage
//		new particle array in CParticleDamage
//
//	22: 0.99
//		Experiment: CannotBeHit is set to TRUE for background objects
//
//	23: 0.99
//		SExtra structure for CItem
//
//	24: 0.99
//		m_iRotation in CDockingPorts
//
//	25: 0.99
//		m_iPower in CPlayerShipController
//		m_iMaxPower in CPlayerShipController
//		m_iPowerRate in CPlayerShipController
//
//	26: 0.99
//		m_dwNameFlags in CShip
//
//	27: 0.99
//		DiceRange uses 32-bits for all values
//
//	28: 0.99
//		m_iHitPoints in CMissile
//
//	29: 0.99
//		m_iDeviceSlot in CInstalledDevice
//
//	30: 0.99
//		CDamageSource
//
//	31: 0.99
//		m_iMaxStructuralHP in CStation
//
//	32: 0.99
//		m_pSecondarySource in CDamageSource
//
//	33: 0.99b
//		m_iManeuverCounter in CBaseShipAI
//
//	34: 1.0 RC1
//		m_vPotential in CBaseShipAI
//
//	35: 1.0 RC1
//		m_EventHandlers in CSystem
//
//	36: 1.0 RC1
//		m_dwNameFlags in CStation
//
//	37: 1.0 RC1
//		m_pTrade in CStation
//
//	38: 1.0 RC1
//		m_dwID in CEnergyField
//		m_iPosAngle, m_iPosRadius, m_iRotation in CEnergyField
//
//	39: 1.0 RC1
//		m_Data in CEnergyField
//
//	40: 1.0 RC1
//		Save class with IEffectPainter
//
//	41: 1.0 RC1
//		CSystem saves object references using ObjID (instead of index)
//
//	42: 1.0 RC1
//		m_iLastFireTime in CShip
//
//	43: 1.0 RC1
//		Start saving UNID/class for NULL effect creators
//
//	44: 1.0 RC1
//		m_iActivateDelayAdj in CInstalledDevice
//
//	45: 1.0 RC2
//		m_iCause in CDamageSource
//
//	46: 1.0 RC3
//		m_pEnhancer and m_iExpireTime in CItemEnhancement
//
//	47: 1.0 RC3
//		m_pTarget and m_pBase in CStation
//
//	48: 1.0 RC3
//		m_pOverride in CSpaceObject
//
//	49: 1.02
//		m_Stats in CPlayerShipController
//		m_Credits in CPlayerShipController
//
//	50: 1.02
//		Removed m_sDeferredDestruction from CPlayerShipController
//
//	51: 1.02
//		Added m_iRotation to CEffect
//		Added iRotation to SEffectNode in CSpaceObject
//
//	52: 1.03
//		Added flags for CTimedEvent
//
//	53: 1.03
//		Added m_PlayTime and m_GameTime for CPlayerGameStats
//
//	54: 1.03
//		Added m_iSect to CInstalledArmor
//
//	55: 1.04
//		Added m_dwFlags to CDamageSource
//
//	56: 1.04
//		Added m_Overlays to CStation
//
//	57: 1.04
//		Added m_iLastUpdated to CSystem
//
//	58: 1.04
//		Added m_dwDisruptedTime to CItem::SExtra
//		CItemList no longer saves allocation size
//
//	59: 1.05
//		CInstalledArmor no longer saves m_Mods
//
//	60: 1.06
//		Added m_Territories in CSystem
//
//	61: 1.06
//		Added m_ImageSelector to CStation (and removed m_iVariant)
//
//	62: 1.06
//		CCurrencyBlock saves CEconomyType UNIDs instead of strings
//
//	63: 1.06
//		Added m_iDevice to CEnergyField
//		Added m_UIMsgs to CPlayerShipController
//
//	64: 1.06
//		Added Pos and Vel to CParticleArray::SParticle
//
//	65: 1.06
//		Added m_vOldPos to CSpaceObject
//
//	66: 1.06
//		Added m_dwTargetID to CInstalledDevice
//
//	67: 1.06
//		Stop saving m_vOldPos in CParticleDamage
//		Added m_vOrigin to CParticleArray
//
//	68: 1.06
//		Added m_AISettings to CBaseShipAI
//
//	69: 1.06
//		Added m_HitTest to CShockwavePainter
//
//	70: 1.06
//		Removed m_Hit from CAreaDamage
//		Added m_iRadiusInc to CShockwavePainter
//
//	71: 1.07
//		m_dwCharges in CItem is now 32 bits
//		Added m_dwVariant to CItem; removed m_dwCondition
//
//	72: 1.07
//		Added m_Locations to CSystem
//
//	73: 1.07
//		Added BYTE-length m_ShieldDamage to DamageDesc
//
//	74: 1.07
//		Added m_KeyEventStats to CPlayerGameStats
//
//	75: 1.08
//		Added m_pOrderModule to CBaseShipAI
//
//	76: 1.08c
//		Added m_iState to CAttackOrder
//		Added m_iState to CAttackStationOrder
//		Added m_iState to CGuardOrder
//
//	77: 1.08g
//		Added m_SubscribedObjs to CSpaceObject
//		Added m_iMaxHitPoints to CStation
//
//	78: 1.08h
//		Fixed a bug in m_SubscribedObjs.
//
//	79: 1.08k
//		Fixed another bug in m_SubscribedObjs.
//
//	80: 1.08l
//		Fixed another(!) bug in m_SubscribedObjs.
//
//	81: 1.1
//		m_iMaxDist to CDockingPorts
//
//	82:	1.2 Alpha 1
//		m_pEnvironment in CSystem is saved differently.
//
//	83: 1.2 Alpha 1
//		iService in CTradingDesc
//
//	84: 1.2 Alpha 1
//		m_dwLeftSystemOn in CMission
//
//	85: 1.2 Alpha 1
//		m_dwCreatedOn in CMission
//
//	86: 1.2 Alpha 1
//		m_dwAcceptedOn in CMission
//		m_fAcceptedByPlayer in CMission
//
//	87: 1.2 Alpha 2
//		m_Orders in CBaseShipAI has new structure
//
//	88: 1.2 Alpha 3
//		Save size of tiles in CEnvironmentGrid
//
//	89: 1.2 Alpha 3
//		m_pDebriefer in CMission
//
//	91: 1.2 Alpha 3
//		m_Interior in CShip
//
//	92: 1.2 Alpha 3
//		m_pEnhancements in CInstalledDevice
//		m_pEnhancements in CMissile
//
//	93: 1.2 Beta 2
//		m_iActivateDelay in CInstalledDevice (instead of m_iActivateDelayAdj)
//		CEffectGroupPainter saves a count
//
//	94: 1.2 Beta 3
//		m_rParallaxDist in CStation
//
//	95: 1.2 RC 2
//		Updated to fix a bug in 94 in which asteroids were marked as immutable
//		(preventing mining from working).
//
//	96: 1.3
//		m_iLastHitTime in CShip
//
//	97: 1.3
//		m_Rotation in CShip
//
//	98: 1.3
//		m_bUseObjectCenter in CParticleCloudPainter
//
//	99: 1.3
//		CEffectParamDesc saving
//
//	100: 1.3
//		m_iOrder in CPlayerShipController
//
//	101: 1.4
//		Flags in CEnergyField
//
//	102: 1.4
//		iHPDamaged in SItemTypeStats
//
//	103: 1.4
//		m_iCounter in CEnergyField
//		m_sMessage in CEnergyField
//
//	104: 1.4
//		m_iFramesPerColumn in CObjectImageArray
//
//	105: 1.4
//		m_pTrade in CShip
//
//	106: 1.4
//		m_EncounterRecord in CStationType changed.
//
//	107: 1.5
//		m_fExtrenal flag in CInstalledDevice
//
//	See: TSEUtil.h for definition of SYSTEM_SAVE_VERSION

#include "PreComp.h"

#include "math.h"

#define ENHANCED_SRS_BLOCK_SIZE			6

#define LEVEL_ENCOUNTER_CHANCE			10

#define MAX_TARGET_RANGE				(24.0 * LIGHT_SECOND)

const Metric MAX_AUTO_TARGET_DISTANCE =			30.0 * LIGHT_SECOND;
const Metric MAX_ENCOUNTER_DIST	=				30.0 * LIGHT_MINUTE;

const Metric GRAVITY_WARNING_THRESHOLD =		40.0;	//	Acceleration value at which we start warning
const Metric TIDAL_KILL_THRESHOLD =				7250.0;	//	Acceleration at which we get ripped apart

#define ON_CREATE_EVENT					CONSTLIT("OnCreate")
#define ON_OBJ_JUMP_POS_ADJ				CONSTLIT("OnObjJumpPosAdj")

#define SPECIAL_ATTRIB_INNER_SYSTEM		CONSTLIT("innerSystem")
#define SPECIAL_ATTRIB_LIFE_ZONE		CONSTLIT("lifeZone")
#define SPECIAL_ATTRIB_NEAR_STATIONS	CONSTLIT("nearStations")
#define SPECIAL_ATTRIB_OUTER_SYSTEM		CONSTLIT("outerSystem")

static CObjectClass<CSystem>g_Class(OBJID_CSYSTEM, NULL);

int g_iGateTimer = 0;
int g_iGateTimerTick = -1;
int g_cxStarField = -1;
int g_cyStarField = -1;

enum LabelPositionTypes
	{
	labelPosNone,

	labelPosRight,
	labelPosLeft,
	labelPosBottom,
	};

struct SLabelEntry
	{
	CSpaceObject *pObj;
	int x;
	int y;
	int cxLabel;

	RECT rcLabel;
	int iPosition;
	int iNewPosition;
	};

const int STARFIELD_COUNT =						5000;
const int STARFIELD_DENSITY =					300;	//	Lower is denser (0 is invalid)
const int MIN_STAR_DISTANCE =					2;
const int MAX_STAR_DISTANCE =					20;
const int BRIGHT_STAR_CHANCE =					20;

const int g_iStarFieldWidth = 1200;
const int g_iStarFieldHeight = 1200;

const COLORREF g_rgbSpaceColor = RGB(0,0,8);
//const COLORREF g_rgbSpaceColor = RGB(0,0,0);
const Metric g_MetersPerKlick = 1000.0;
const Metric MAP_VERTICAL_ADJUST =						1.4;

const WORD RGB_GRID_LINE =								CG16bitImage::RGBValue(65, 68, 77);

const int LABEL_SPACING_X =								8;
const int LABEL_SPACING_Y =								4;
const int LABEL_OVERLAP_Y =								1;

const Metric BACKGROUND_OBJECT_FACTOR =					4.0;

const int GRID_SIZE =									128;
#define CELL_SIZE										(1024.0 * g_KlicksPerPixel)
#define CELL_BORDER										(128.0 * g_KlicksPerPixel)

const Metric SAME_POS_THRESHOLD2 =						(g_KlicksPerPixel * g_KlicksPerPixel);

const Metric MAP_GRID_SIZE =							3000.0 * LIGHT_SECOND;

bool CalcOverlap (SLabelEntry *pEntries, int iCount);
void SetLabelBelow (SLabelEntry &Entry, int cyChar);
void SetLabelLeft (SLabelEntry &Entry, int cyChar);
void SetLabelRight (SLabelEntry &Entry, int cyChar);

CSystem::CSystem (void) : CObject(&g_Class),
		m_iTick(0),
		m_AllObjects(TRUE),
		m_iTimeStopped(0),
		m_rKlicksPerPixel(KLICKS_PER_PIXEL),
		m_rTimeScale(TIME_SCALE),
		m_iLastUpdated(-1),
		m_fInCreate(false),
		m_fEncounterTableValid(false),
		m_StarField(sizeof(CStar), STARFIELD_COUNT),
		m_ObjGrid(GRID_SIZE, CELL_SIZE, CELL_BORDER),
		m_fEnemiesInLRS(false),
		m_fEnemiesInSRS(false),
		m_fPlayerUnderAttack(false)

//	CSystem constructor

	{
	ASSERT(FALSE);
	}

CSystem::CSystem (CUniverse *pUniv, CTopologyNode *pTopology) : CObject(&g_Class),
		m_dwID(OBJID_NULL),
		m_iTick(0),
		m_pTopology(pTopology),
		m_AllObjects(TRUE),
		m_pEnvironment(NULL),
		m_iTimeStopped(0),
		m_rKlicksPerPixel(KLICKS_PER_PIXEL),
		m_rTimeScale(TIME_SCALE),
		m_iLastUpdated(-1),
		m_fNoRandomEncounters(false),
		m_fInCreate(false),
		m_fEncounterTableValid(false),
		m_fUseDefaultTerritories(true),
		m_StarField(sizeof(CStar), STARFIELD_COUNT),
		m_ObjGrid(GRID_SIZE, CELL_SIZE, CELL_BORDER)

//	CSystem constructor

	{
	//	Make sure our vectors are initialized
	EuclidInit();
	}

CSystem::~CSystem (void)

//	CSystem destructor

	{
	//	Set our topology node to NULL so that a new system is
	//	created next time we access this node.

	if (m_pTopology)
		m_pTopology->SetSystem(NULL);

	if (m_pEnvironment)
		delete m_pEnvironment;
	}

ALERROR CSystem::AddTerritory (CTerritoryDef *pTerritory)

//	AddTerritory
//
//	Adds a territory

	{
	m_Territories.Insert(pTerritory);

	//	If we are defining one of the special attributes, then remember
	//	that so we don't implicitly define them.

	if (m_fUseDefaultTerritories
		&& (pTerritory->HasAttribute(SPECIAL_ATTRIB_INNER_SYSTEM)
			|| pTerritory->HasAttribute(SPECIAL_ATTRIB_LIFE_ZONE)
			|| pTerritory->HasAttribute(SPECIAL_ATTRIB_OUTER_SYSTEM)))
		m_fUseDefaultTerritories = false;

	return NOERROR;
	}

ALERROR CSystem::AddTimedEvent (CTimedEvent *pEvent)

//	AddTimedEvent
//
//	Adds a timed event

	{
	m_TimedEvents.AddEvent(pEvent);
	return NOERROR;
	}

ALERROR CSystem::AddToSystem (CSpaceObject *pObj, int *retiIndex)

//	AddToSystem
//
//	Adds an object to the system

	{
	int i;

	//	If this object affects the enemy object cache, then
	//	flush the cache

	if (pObj->ClassCanAttack())
		FlushEnemyObjectCache();

	//	Reuse a slot first

	for (i = 0; i < m_AllObjects.GetCount(); i++)
		{
		if (m_AllObjects.GetObject(i) == NULL)
			{
			m_AllObjects.ReplaceObject(i, pObj);
			if (retiIndex)
				*retiIndex = i;
			return NOERROR;
			}
		}

	//	If we could not find a free place, add a new object

	return m_AllObjects.AppendObject(pObj, retiIndex);
	}

bool CSystem::AscendObject (CSpaceObject *pObj, CString *retsError)

//	AscendObject
//
//	Removes the object from the system and adds it to the global list of 
//	ascended objects. Return FALSE if there was an error.

	{
	if (pObj->IsAscended())
		return true;

	if (pObj->IsDestroyed())
		{
		if (retsError)
			*retsError = CONSTLIT("Cannot ascend destroyed objects.");
		return false;
		}

	if (pObj->IsPlayer())
		{
		if (retsError)
			*retsError = CONSTLIT("Cannot ascend the player ship.");
		return false;
		}

	if (pObj->GetSystem() != this)
		{
		if (retsError)
			*retsError = CONSTLIT("Cannot ascend object in another system.");
		return false;
		}

	//	Ascend the object

	pObj->Ascend();

	//	Add to the list of ascended objects

	g_pUniverse->AddAscendedObj(pObj);

	//	Done

	return true;
	}

int CSystem::CalculateLightIntensity (const CVector &vPos, CSpaceObject **retpStar)

//	CalculateLightIntensity
//
//	Returns 0-100% the intensity of the light at this point
//	in space.

	{
	int i;

	//	Find the nearest star to the position. We optimize the case where
	//	there is only a single star in the system.

	int iBestDist;
	CSpaceObject *pBestObj;

	if (m_Stars.GetCount() == 1)
		{
		pBestObj = m_Stars.GetObj(0);
		iBestDist = (int)(vPos.Longest() / LIGHT_SECOND);
		}
	else
		{
		pBestObj = NULL;
		iBestDist = 100000000;

		for (i = 0; i < m_Stars.GetCount(); i++)
			{
			CSpaceObject *pStar = m_Stars.GetObj(i);
			CVector vDist = vPos - pStar->GetPos();

			int iDistFromCenter = (int)(vDist.Longest() / LIGHT_SECOND);
			if (iDistFromCenter < iBestDist)
				{
				iBestDist = iDistFromCenter;
				pBestObj = pStar;
				}
			}

		if (pBestObj == NULL)
			{
			if (retpStar)
				*retpStar = NULL;
			return 0;
			}
		}

	//	Compute the percentage

	int iMaxDist = pBestObj->GetMaxLightDistance();
	int iDistFromCenter = (iBestDist < 15 ? 0 : iBestDist - 15);
	int iPercent = 100 - (iDistFromCenter * 100 / iMaxDist);

	if (retpStar)
		*retpStar = pBestObj;

	return Max(0, iPercent);
	}

int CSystem::CalcLocationWeight (CLocationDef *pLoc, const CAttributeCriteria &Criteria)

//	CalcLocationWeight
//
//	Calculates the weight of the given location relative to the given
//	criteria.
//
//	See: CAttributeCriteria::CalcWeightAdj
//
//	EXAMPLES:
//
//	Criteria = "*"			LocationAttribs = "{anything}"		Result = 1000
//	Criteria = "+asteroids"	LocationAttribs = "asteroids"		Result = 2000
//	Criteria = "+asteroids"	LocationAttribs = "foo"				Result = 1000
//	Criteria = "-asteroids"	LocationAttribs = "asteroids"		Result = 500
//	Criteria = "-asteroids"	LocationAttribs = "foo"				Result = 1000

	{
	int i;
	int iWeight = 1000;

	//	Handle edge cases

	if (Criteria.MatchesAll())
		return iWeight;

	//	Adjust weight based on criteria

	const CString &sAttributes = pLoc->GetAttributes();
	CVector vPos = pLoc->GetOrbit().GetObjectPos();

	for (i = 0; i < Criteria.GetCount(); i++)
		{
		DWORD dwMatchStrength;
		const CString &sAttrib = Criteria.GetAttribAndWeight(i, &dwMatchStrength);

		//	Do we have the attribute? Check the location and any attributes
		//	inherited from territories and the system.

		bool bHasAttrib = (::HasModifier(sAttributes, sAttrib)
				|| HasAttribute(vPos, sAttrib));

		//	Compute the frequency of the given attribute

		int iAttribFreq = g_pUniverse->GetAttributeDesc().GetLocationAttribFrequency(sAttrib);

		//	Adjust probability based on the match strength

		int iAdj = CAttributeCriteria::CalcWeightAdj(bHasAttrib, dwMatchStrength, iAttribFreq);
		iWeight = iWeight * iAdj / 1000;

		//	If weight is 0, then no need to continue

		if (iWeight == 0)
			return 0;
		}

	//	Done

	return iWeight;
	}

WORD CSystem::CalculateSpaceColor (CSpaceObject *pPOV)

//	CalculateSpaceColor
//
//	Calculates the color of space from the given object

	{
	CSpaceObject *pStar;
	int iPercent = CalculateLightIntensity(pPOV->GetPos(), &pStar);

	COLORREF rgbStarColor = (pStar ? pStar->GetSpaceColor() : 0);

	int iRed = GetRValue(rgbStarColor) * iPercent / 100;
	int iGreen = GetGValue(rgbStarColor) * iPercent / 100;
	int iBlue = GetBValue(rgbStarColor) * iPercent / 100;

	COLORREF rgbSpaceColor = RGB(iRed, iGreen, iBlue);

	return CG16bitImage::RGBValue(GetRValue(rgbSpaceColor),
			GetGValue(rgbSpaceColor),
			GetBValue(rgbSpaceColor));
	}

void CSystem::CalcViewportCtx (SViewportPaintCtx &Ctx, const RECT &rcView, CSpaceObject *pCenter, DWORD dwFlags)

//	CalcViewportCtx
//
//	Initializes the viewport context

	{
	DEBUG_TRY
	ASSERT(pCenter);

	Ctx.pCenter = pCenter;
	Ctx.vCenterPos = pCenter->GetPos();
	Ctx.rcView = rcView;
	Ctx.vDiagonal = CVector(g_KlicksPerPixel * (Metric)(RectWidth(rcView)) / 2,
				g_KlicksPerPixel * (Metric)(RectHeight(rcView)) / 2);
	Ctx.vUR = Ctx.vCenterPos + Ctx.vDiagonal;
	Ctx.vLL = Ctx.vCenterPos - Ctx.vDiagonal;

	//	Perception

	Ctx.iPerception = pCenter->GetPerception();

	//	Compute the transformation to map world coordinates to the viewport

	Ctx.xCenter = rcView.left + RectWidth(rcView) / 2;
	Ctx.yCenter = rcView.top + RectHeight(rcView) / 2;
	Ctx.XForm = ViewportTransform(Ctx.vCenterPos, g_KlicksPerPixel, Ctx.xCenter, Ctx.yCenter);
	Ctx.XFormRel = Ctx.XForm;

	//	Figure out the extended boundaries. This is used for enhanced display.

	const Metric ENHANCED_DISPLAY_RANGE = 4.0;
	Ctx.vEnhancedDiagonal = CVector(g_KlicksPerPixel * ENHANCED_DISPLAY_RANGE * (Metric)RectWidth(rcView) / 2.0,
			g_KlicksPerPixel * ENHANCED_DISPLAY_RANGE * (Metric)RectHeight(rcView) / 2.0);
	Ctx.vEnhancedUR = Ctx.vCenterPos + Ctx.vEnhancedDiagonal;
	Ctx.vEnhancedLL = Ctx.vCenterPos - Ctx.vEnhancedDiagonal;

	//	Initialize some flags

	Ctx.fEnhancedDisplay = ((dwFlags & VWP_ENHANCED_DISPLAY) ? true : false);
	Ctx.fNoStarfield = ((dwFlags & VWP_NO_STAR_FIELD) ? true : false);
	Ctx.fShowManeuverEffects = ((dwFlags & VWP_SHOW_MANEUVER_EFFECTS) ? true : false);

	//	Figure out what color space should be. Space gets lighter as we get
	//	near the central star

	Ctx.wSpaceColor = CalculateSpaceColor(pCenter);

	//	Compute the radius of the circle on which we'll show target indicators
	//	(in pixels)

	Ctx.rIndicatorRadius = Min(RectWidth(rcView), RectHeight(rcView)) / 2.0;

	DEBUG_CATCH
	}

void CSystem::CancelTimedEvent (CSpaceObject *pSource, const CString &sEvent, bool bInDoEvent)

//	CancelTimedEvent
//
//	Cancel event by name

	{
	m_TimedEvents.CancelEvent(pSource, sEvent, bInDoEvent);
	}

void CSystem::CancelTimedEvent (CDesignType *pSource, const CString &sEvent, bool bInDoEvent)

//	CancelTimedEvent
//
//	Cancel event by name

	{
	int i;

	for (i = 0; i < GetTimedEventCount(); i++)
		{
		CTimedEvent *pEvent = GetTimedEvent(i);
		if (pEvent->GetEventHandlerType() == pSource 
				&& strEquals(pEvent->GetEventHandlerName(), sEvent))
			{
			if (bInDoEvent)
				pEvent->SetDestroyed();
			else
				{
				m_TimedEvents.RemoveEvent(i);
				i--;
				}
			}
		}
	}

void CSystem::ComputeMapLabels (void)

//	ComputeMapLabels
//
//	Positions the labels for all objects that need one

	{
	int i;
	const int MAX_LABELS = 100;
	int iLabelCount = 0;
	SLabelEntry Labels[MAX_LABELS];

	//	Compute some font metrics

	int cxChar = g_pUniverse->GetNamedFont(CUniverse::fontMapLabel).GetAverageWidth();
	int cyChar = g_pUniverse->GetNamedFont(CUniverse::fontMapLabel).GetHeight();

	//	Compute a transform for map coordinate

	ViewportTransform Trans(CVector(), 
			g_MapKlicksPerPixel, 
			g_MapKlicksPerPixel * MAP_VERTICAL_ADJUST,
			0, 
			0);

	//	Loop over all objects and see if they have a map label

	for (i = 0; i < GetObjectCount() && iLabelCount < MAX_LABELS; i++)
		{
		CSpaceObject *pObj = GetObject(i);

		if (pObj && pObj->HasMapLabel())
			{
			Labels[iLabelCount].pObj = pObj;
			Trans.Transform(pObj->GetPos(), &Labels[iLabelCount].x, &Labels[iLabelCount].y);
			Labels[iLabelCount].cxLabel = g_pUniverse->GetNamedFont(CUniverse::fontMapLabel).MeasureText(pObj->GetName(NULL));

			SetLabelRight(Labels[iLabelCount], cyChar);

			iLabelCount++;
			}
		}

	//	Keep looping until we minimize overlap

	bool bOverlap;
	int iIteration = 0;

	do
		{
		bOverlap = CalcOverlap(Labels, iLabelCount);
		if (bOverlap)
			{
			//	Modify the label location of any overlapping labels

			for (i = 0; i < iLabelCount; i++)
				{
				switch (Labels[i].iNewPosition)
					{
					case labelPosRight:
						{
						SetLabelRight(Labels[i], cyChar);
						break;
						}

					case labelPosLeft:
						{
						SetLabelLeft(Labels[i], cyChar);
						break;
						}

					case labelPosBottom:
						{
						SetLabelBelow(Labels[i], cyChar);
						break;
						}
					}
				}

			iIteration++;
			}
		}
	while (bOverlap && iIteration < 10);

	//	Set the label position for all the objects

	for (i = 0; i < iLabelCount; i++)
		Labels[i].pObj->SetMapLabelPos(Labels[i].rcLabel.left - Labels[i].x, Labels[i].rcLabel.top - Labels[i].y - LABEL_OVERLAP_Y);
	}

void CSystem::ComputeRandomEncounters (void)

//	ComputeRandomEncounters
//
//	Creates the table that lists all objects in the system that
//	can generate random encounters

	{
	int i;

	if (m_fEncounterTableValid)
		return;

	m_EncounterObjs.RemoveAll();
	if (!m_fNoRandomEncounters)
		{
		for (i = 0; i < GetObjectCount(); i++)
			{
			CSpaceObject *pObj = GetObject(i);

			if (pObj 
					&& pObj->HasRandomEncounters())
				m_EncounterObjs.Add(pObj);
			}
		}

	m_fEncounterTableValid = true;
	}

void CSystem::ComputeStars (void)

//	ComputeStars
//
//	Keep a list of the stars in the system

	{
	int i;

	m_Stars.RemoveAll();

	for (i = 0; i < GetObjectCount(); i++)
		{
		CSpaceObject *pObj = GetObject(i);

		if (pObj 
				&& pObj->GetScale() == scaleStar
				&& !pObj->IsDestroyed())
			m_Stars.Add(pObj);
		}
	}

ALERROR CSystem::CreateFlotsam (const CItem &Item, 
								const CVector &vPos, 
								const CVector &vVel, 
								CSovereign *pSovereign, 
								CStation **retpFlotsam)

//	CreateFlotsam
//
//	Creates a floating item

	{
	CItemType *pItemType = Item.GetType();
	if (pItemType == NULL)
		return ERR_FAIL;

	//	Create the station

	CStation *pFlotsam;
	pItemType->CreateEmptyFlotsam(this, vPos, vVel, pSovereign, &pFlotsam);

	//	Add the items to the station

	CItemListManipulator ItemList(pFlotsam->GetItemList());
	ItemList.AddItem(Item);

	//	Done

	if (retpFlotsam)
		*retpFlotsam = pFlotsam;

	return NOERROR;
	}

ALERROR CSystem::CreateFromStream (CUniverse *pUniv, 
								   IReadStream *pStream, 
								   CSystem **retpSystem,
								   CString *retsError,
								   DWORD dwObjID,
								   CSpaceObject **retpObj,
								   CSpaceObject *pPlayerShip)

//	CreateFromStream
//
//	Creates the star system from the stream
//
//	DWORD		m_dwID
//	DWORD		m_iTick
//	DWORD		m_iTimeStopped
//	CString		m_sName
//	CString		Topology node ID
//	DWORD		(unused)
//	DWORD		m_iNextEncounter
//	DWORD		flags
//	DWORD		SAVE VERSION (only if [flags & 0x02])
//	Metric		m_rKlicksPerPixel
//	Metric		m_rTimeScale
//	DWORD		m_iLastUpdated
//
//	DWORD		Number of CNavigationPath
//	CNavigationPath
//
//	CEventHandlers
//
//	DWORD		Number of mission objects
//	CSpaceObject
//
//	DWORD		Number of objects
//	CSpaceObject
//
//	DWORD		Number of named objects
//	CString		entrypoint: name
//	DWORD		entrypoint: CSpaceObject ref
//
//	DWORD		Number of timed events
//	CTimedEvent
//
//	DWORD		Number of environment maps
//	CTileMap
//
//	CLocationList	m_Locations
//	CTerritoryList	m_Territories

	{
	int i;
	DWORD dwLoad;
	DWORD dwCount;

	//	Create a context block for loading

	SLoadCtx Ctx;
	Ctx.dwVersion = 0;	//	Default to 0
	Ctx.pStream = pStream;

	//	Add all missions to the map so that they can be resolved

	for (i = 0; i < pUniv->GetMissionCount(); i++)
		{
		CMission *pMission = pUniv->GetMission(i);
		Ctx.ObjMap.AddEntry(pMission->GetID(), pMission);
		}

	//	Create the new star system

	Ctx.pSystem = new CSystem(pUniv, NULL);
	if (Ctx.pSystem == NULL)
		{
		*retsError = CONSTLIT("Out of memory.");
		return ERR_MEMORY;
		}

	//	Load some misc info

	Ctx.pStream->Read((char *)&Ctx.pSystem->m_dwID, sizeof(DWORD));
	Ctx.pStream->Read((char *)&Ctx.pSystem->m_iTick, sizeof(DWORD));
	Ctx.pStream->Read((char *)&Ctx.pSystem->m_iTimeStopped, sizeof(DWORD));
	Ctx.pSystem->m_sName.ReadFromStream(Ctx.pStream);

	//	Load the topology node

	CString sNodeID;
	sNodeID.ReadFromStream(Ctx.pStream);
	Ctx.pSystem->m_pTopology = pUniv->FindTopologyNode(sNodeID);
	Ctx.pSystem->m_pType = pUniv->FindSystemType(Ctx.pSystem->m_pTopology->GetSystemTypeUNID());

	//	More misc info

	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	Ctx.pStream->Read((char *)&Ctx.pSystem->m_iNextEncounter, sizeof(DWORD));

	//	Flags

	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	Ctx.pSystem->m_fNoRandomEncounters =	((dwLoad & 0x00000001) ? true : false);
	if (dwLoad & 0x00000002)
		Ctx.pStream->Read((char *)&Ctx.dwVersion, sizeof(DWORD));
	Ctx.pSystem->m_fUseDefaultTerritories =	((dwLoad & 0x00000004) ? false : true);
	Ctx.pSystem->m_fEncounterTableValid = false;
	Ctx.pSystem->m_fEnemiesInLRS =			((dwLoad & 0x00000008) ? false : true);
	Ctx.pSystem->m_fEnemiesInSRS =			((dwLoad & 0x00000010) ? false : true);
	Ctx.pSystem->m_fPlayerUnderAttack =		((dwLoad & 0x00000020) ? false : true);

	//	Scales

	if (Ctx.dwVersion >= 9)
		{
		Ctx.pStream->Read((char *)&Ctx.pSystem->m_rKlicksPerPixel, sizeof(Metric));
		Ctx.pStream->Read((char *)&Ctx.pSystem->m_rTimeScale, sizeof(Metric));
		}
	else
		{
		Ctx.pSystem->m_rKlicksPerPixel = KLICKS_PER_PIXEL;
		Ctx.pSystem->m_rTimeScale = TIME_SCALE;
		}

	if (Ctx.dwVersion >= 57)
		Ctx.pStream->Read((char *)&Ctx.pSystem->m_iLastUpdated, sizeof(DWORD));
	else
		Ctx.pSystem->m_iLastUpdated = -1;

	//	Load the navigation paths (we load these before objects
	//	because objects might have references to paths)

	if (Ctx.dwVersion >= 10)
		Ctx.pSystem->m_NavPaths.ReadFromStream(Ctx);

	//	Load the system event handlers

	if (Ctx.dwVersion >= 35)
		Ctx.pSystem->m_EventHandlers.ReadFromStream(Ctx);

	//	Load all objects

	Ctx.pStream->Read((char *)&dwCount, sizeof(DWORD));
	for (i = 0; i < (int)dwCount; i++)
		{
		//	Load the object

		CSpaceObject *pObj;
		try
			{
			CSpaceObject::CreateFromStream(Ctx, &pObj);
			}
		catch (...)
			{
			*retsError = CSpaceObject::DebugLoadError(Ctx);
			return ERR_FAIL;
			}

		//	Add this object to the map

		DWORD dwID = (Ctx.dwVersion >= 41 ? pObj->GetID() : pObj->GetIndex());
		Ctx.ObjMap.AddEntry(dwID, pObj);

		//	Update any previous objects that are waiting for this reference

		Ctx.ForwardReferences.ResolveRefs(dwID, pObj);

		//	Set the system (note: this will change the index to the new
		//	system)

		pObj->AddToSystem(Ctx.pSystem, true);
		}

	//	If we have old style registrations then we need to convert to subscriptions

	if (Ctx.dwVersion < 77)
		{
		for (i = 0; i < Ctx.pSystem->GetObjectCount(); i++)
			{
			CSpaceObject *pObj = Ctx.pSystem->GetObject(i);
			if (pObj)
				{
				TArray<CSpaceObject *> *pList = Ctx.Subscribed.GetAt(pObj->GetID());
				if (pList)
					{
					for (int j = 0; j < pList->GetCount(); j++)
						pObj->AddEventSubscriber(pList->GetAt(j));
					}
				}
			}
		}

	//	If there are still references to the player, resolve them now.
	//	[This happens because of a bug in 1.0 RC1]

	if (pPlayerShip)
		Ctx.ForwardReferences.ResolveRefs(pPlayerShip->GetID(), pPlayerShip);

	//	If we've got left over references, then dump debug output

	if (Ctx.ForwardReferences.HasUnresolved())
		{
		*retsError = strPatternSubst(CONSTLIT("Undefined object references"));
		return ERR_FAIL;
		}

	//	Load named objects table

	Ctx.pStream->Read((char *)&dwCount, sizeof(DWORD));
	for (i = 0; i < (int)dwCount; i++)
		{
		CString sName;
		sName.ReadFromStream(Ctx.pStream);

		Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
		CSpaceObject *pObj;
		if (Ctx.ObjMap.Lookup(dwLoad, (CObject **)&pObj) != NOERROR)
			{
			*retsError = strPatternSubst(CONSTLIT("Save file error: Unable to find named object: %s [%x]"), sName, dwLoad);
			return ERR_FAIL;
			}

		Ctx.pSystem->NameObject(sName, pObj);
		}

	//	Load all timed events

	Ctx.pSystem->m_TimedEvents.ReadFromStream(Ctx);

	//	Load environment map

	Ctx.pStream->Read((char *)&dwCount, sizeof(DWORD));
	if (dwCount)
		{
		Ctx.pSystem->m_pEnvironment = new CEnvironmentGrid(0);
		Ctx.pSystem->m_pEnvironment->ReadFromStream(Ctx);
		}
	else
		Ctx.pSystem->m_pEnvironment = NULL;

	//	Load locations

	if (Ctx.dwVersion >= 72)
		Ctx.pSystem->m_Locations.ReadFromStream(Ctx);

	//	Load territories

	if (Ctx.dwVersion >= 60)
		Ctx.pSystem->m_Territories.ReadFromStream(Ctx);

	//	Create the background star field

	Ctx.pSystem->ResetStarField();

	//	Compute some tables

	Ctx.pSystem->ComputeStars();
	
	//	Map the POV object

	if (retpObj)
		{
		if (Ctx.ObjMap.Lookup(dwObjID, (CObject **)retpObj) != NOERROR)
			{
			*retsError = strPatternSubst(CONSTLIT("Unable to find POV object: %x"), dwObjID);

			//	Look for the player object

			bool bFound = false;
			for (i = 0; i < Ctx.pSystem->GetObjectCount(); i++)
				{
				CSpaceObject *pObj = Ctx.pSystem->GetObject(i);

				if (pObj && pObj->IsPlayer())
					{
					*retpObj = pObj;
					bFound = true;
					break;
					}
				}

			if (!bFound)
				retsError->Append(CONSTLIT("\r\nUnable to find player ship."));

			return ERR_FAIL;
			}
		}

	//	Tell all objects that the system has been loaded

	for (i = 0; i < Ctx.pSystem->GetObjectCount(); i++)
		{
		CSpaceObject *pObj = Ctx.pSystem->GetObject(i);

		if (pObj)
			{
			pObj->LoadObjReferences(Ctx.pSystem);
			pObj->OnSystemLoaded();

			pObj->FireOnLoad(Ctx);
			}
		}

	//	Done

	*retpSystem = Ctx.pSystem;

	return NOERROR;
	}

ALERROR CSystem::CreateLocation (const CString &sID, const COrbit &Orbit, const CString &sAttributes, CLocationDef **retpLocation)

//	CreateLocation
//
//	Creates a location

	{
	CLocationDef *pLocation = m_Locations.Insert(sID);
	pLocation->SetOrbit(Orbit);
	pLocation->SetAttributes(sAttributes);
	if (retpLocation)
		*retpLocation = pLocation;

	return NOERROR;
	}

ALERROR CSystem::CreateRandomEncounter (IShipGenerator *pTable, 
										CSpaceObject *pBase,
										CSovereign *pBaseSovereign,
										CSpaceObject *pTarget,
										CSpaceObject *pGate)

//	CreateRandomEncounter
//
//	Creates a random ship encounter

	{
	ASSERT(pTable);

	SShipCreateCtx Ctx;
	Ctx.pSystem = this;
	Ctx.pBase = pBase;
	Ctx.pBaseSovereign = pBaseSovereign;
	Ctx.pTarget = pTarget;

	//	Figure out where the encounter will come from

	if (pGate && pGate->IsActiveStargate())
		Ctx.pGate = pGate;
	else if (pGate)
		{
		Ctx.vPos = pGate->GetPos();
		Ctx.PosSpread = DiceRange(2, 1, 2);
		}
	else if (pTarget)
		//	Exclude uncharted stargates
		Ctx.pGate = pTarget->GetNearestStargate(true);

	//	Generate ship

	pTable->CreateShips(Ctx);

	return NOERROR;
	}

ALERROR CSystem::CreateShip (DWORD dwClassID,
							 IShipController *pController,
							 CDesignType *pOverride,
							 CSovereign *pSovereign,
							 const CVector &vPos,
							 const CVector &vVel,
							 int iRotation,
							 CSpaceObject *pExitGate,
							 SShipGeneratorCtx *pCtx,
							 CShip **retpShip,
							 CSpaceObjectList *retpShipList)

//	CreateShip
//
//	Creates a ship based on the class.
//
//	pController is owned by the ship if this call is successful.

	{
	ALERROR error;
	CDesignType *pType = g_pUniverse->FindDesignType(dwClassID);

	//	If we have a ship table, then we go through a totally different path

	if (pType->GetType() == designShipTable)
		{
		CShipTable *pTable = CShipTable::AsType(pType);
		if (pTable == NULL)
			return ERR_FAIL;

		SShipCreateCtx CreateCtx;
		CreateCtx.pSystem = this;
		CreateCtx.pGate = pExitGate;
		CreateCtx.vPos = vPos;
		CreateCtx.pBaseSovereign = pSovereign;
		CreateCtx.pEncounterInfo = NULL;
		CreateCtx.pOverride = pOverride;
		CreateCtx.dwFlags = SShipCreateCtx::RETURN_RESULT;

		//	Create

		pTable->CreateShips(CreateCtx);

		//	Return at least one of the ships created

		if (CreateCtx.Result.GetCount() == 0)
			return ERR_NOTFOUND;

		if (retpShip)
			*retpShip = CreateCtx.Result.GetObj(0)->AsShip();

		if (retpShipList)
			*retpShipList = CreateCtx.Result;

		return NOERROR;
		}

	//	Find the class

	CShipClass *pClass = CShipClass::AsType(pType);
	if (pClass == NULL)
		return ERR_FAIL;

	//	Create the controller

	if (pController == NULL)
		{
		pController = new CStandardShipAI;
		if (pController == NULL)
			return ERR_MEMORY;
		}

	//	Create a new ship based on the class

	CShip *pShip;
	if (error = CShip::CreateFromClass(this, 
			pClass, 
			pController, 
			pOverride,
			pSovereign, 
			vPos, 
			vVel, 
			iRotation,
			pCtx,
			&pShip))
		return error;

	//	If we're coming out of a gate, set the timer

	if (pExitGate)
		PlaceInGate(pShip, pExitGate);

	//	Load images, if necessary

	if (!IsCreationInProgress())
		pShip->MarkImages();

	//	Create escorts, if necessary

	IShipGenerator *pEscorts = pClass->GetEscorts();
	if (pEscorts)
		{
		//	If the ship has changed its position, then the escorts should not
		//	appear at a gate. [This happens when an override handler moves the
		//	ship at create time to make the ship appear near the player.]

		CSpaceObject *pEscortGate = pExitGate;
		if (pExitGate == NULL || (pExitGate->GetPos() - pShip->GetPos()).Length2() > (LIGHT_SECOND * LIGHT_SECOND))
			pEscortGate = pShip;
		else
			pEscortGate = pExitGate;

		//	Create escorts

		SShipCreateCtx ECtx;
		ECtx.pSystem = this;
		ECtx.vPos = pEscortGate->GetPos();
		ECtx.pBase = pShip;
		ECtx.pTarget = NULL;
		ECtx.pGate = pEscortGate;

		pEscorts->CreateShips(ECtx);
		}

	if (retpShip)
		*retpShip = pShip;

	if (retpShipList)
		{
		retpShipList->RemoveAll();
		retpShipList->Add(pShip);
		}

	return NOERROR;
	}

ALERROR CSystem::CreateShipwreck (CShipClass *pClass,
								  const CVector &vPos,
								  const CVector &vVel,
								  CSovereign *pSovereign,
								  CStation **retpWreck)

//	CreateShipWreck
//
//	Creates an empty ship wreck of the given class

	{
	pClass->CreateEmptyWreck(this,
			NULL,
			vPos,
			vVel,
			pSovereign,
			retpWreck);

	return NOERROR;
	}

void GenerateSquareDist (int iTotalCount, int iMinValue, int iMaxValue, int *Dist)

//	GenerateSquareDist
//
//	Generates buckets such that:
//
//	1. The sum of the buckets = iTotalCount
//	2. Each bucket has units proportional to the square of its index value
//
//	Dist must be allocated to at least iMaxValue + 1

	{
	int i;

	//	First generate a square distribution

	int iTotalProb = 0;
	for (i = 0; i < iMaxValue + 1; i++)
		{
		if (i >= iMinValue)
			Dist[i] = i * i;
		else
			Dist[i] = 0;

		iTotalProb += Dist[i];
		}

	ASSERT(iTotalProb > 0);
	if (iTotalProb == 0)
		return;

	//	Scale the distribution to the total count

	int iLeft = iTotalCount;
	for (i = 0; i < iMaxValue + 1; i++)
		{
		int iNumerator = Dist[i] * iTotalCount;
		int iBucketCount = iNumerator / iTotalProb;
		int iBucketCountRemainder = iNumerator % iTotalProb;
		if (mathRandom(0, iTotalProb - 1) < iBucketCountRemainder)
			iBucketCount++;

		iBucketCount = Min(iBucketCount, iLeft);
		Dist[i] = iBucketCount;
		iLeft -= iBucketCount;
		}
	}

ALERROR CSystem::CreateStarField (int cxFieldWidth, int cyFieldHeight)

//	CreateStarField
//
//	Create the system's background star field

	{
	ALERROR error;
	int i, j;

	if (g_cxStarField == cxFieldWidth && g_cyStarField == cyFieldHeight)
		return NOERROR;

	//	Compute count

	int iStarCount = cxFieldWidth * cyFieldHeight / STARFIELD_DENSITY;

	//	Figure out how many stars at each distance

	int DistCount[MAX_STAR_DISTANCE + 1];
	GenerateSquareDist(iStarCount, MIN_STAR_DISTANCE, MAX_STAR_DISTANCE, DistCount);

	//	Generate stars at each distance

	int iBrightAdj = 180 / MAX_STAR_DISTANCE;
	for (i = 0; i < MAX_STAR_DISTANCE + 1; i++)
		for (j = 0; j < DistCount[i]; j++)
			{
			CStar Star;

			Star.x = mathRandom(0, cxFieldWidth);
			Star.y = mathRandom(0, cyFieldHeight);

			Star.wDistance = i;

			int iBrightness = 225 - i * iBrightAdj;
			int iBlueAdj = 2 * Min(25, MAX_STAR_DISTANCE - i);
			int iRedAdj = 2 * Min(25, i);

			Star.wColor = CG16bitImage::RGBValue(iBrightness + mathRandom(-25 + iRedAdj, 25),
					iBrightness + mathRandom(-5, 5),
					iBrightness + mathRandom(-25 + iBlueAdj, 25));

			if (Star.bBrightStar = (mathRandom(1, 100) <= BRIGHT_STAR_CHANCE))
				Star.wSpikeColor = CG16bitImage::BlendPixel(0, Star.wColor, 128);

			if (error = m_StarField.AppendStruct(&Star, NULL))
				return error;
			}

	g_cxStarField = cxFieldWidth;
	g_cyStarField = cyFieldHeight;

	return NOERROR;
	}

ALERROR CSystem::CreateStargate (CStationType *pType,
								 CVector &vPos,
								 const CString &sStargateID,
								 const CString &sDestNodeID,
								 const CString &sDestStargateID,
								 CSpaceObject **retpStation)

//	CreateStargate
//
//	Creates a stargate in the system

	{
	ALERROR error;
	CStation *pStation;

	CTopologyNode *pDestNode = g_pUniverse->FindTopologyNode(sDestNodeID);
	if (pDestNode == NULL)
		return ERR_FAIL;

	//	Create the station

	CSpaceObject *pObj;
	if (error = CreateStation(pType, NULL, vPos, &pObj))
		return error;

	pStation = pObj->AsStation();
	if (pStation == NULL)
		return ERR_FAIL;

	//	Create stargate stuff

	if (error = StargateCreated(pStation, sStargateID, sDestNodeID, sDestStargateID))
		return error;

	//	Set stargate properties (note: CreateStation also looks at objName and adds the name
	//	to the named-objects system table.)

	pStation->SetStargate(sDestNodeID, sDestStargateID);

	//	If we haven't already set the name, set the name of the stargate
	//	to include the name of the destination system

	if (!pStation->IsNameSet())
		pStation->SetName(strPatternSubst(CONSTLIT("%s Stargate"), pDestNode->GetSystemName()), nounDefiniteArticle);

	//	Done

	if (retpStation)
		*retpStation = pStation;

	return NOERROR;
	}

ALERROR CSystem::CreateStation (CStationType *pType,
							    CDesignType *pEventHandler,
								CVector &vPos,
								CSpaceObject **retpStation)

//	CreateStation
//
//	Creates a station outside of a system definition

	{
	ALERROR error;

	//	Generate context block

	SSystemCreateCtx Ctx;
	Ctx.pTopologyNode = GetTopology();
	Ctx.pSystem = this;
	Ctx.pStats = NULL;

	CXMLElement *pLocalTable = (m_pType ? m_pType->GetLocalSystemTables() : NULL);
	if (pLocalTable)
		Ctx.LocalTables.Insert(pLocalTable);

	//	Generate an orbit. First we look for the nearest object with
	//	an orbit.

	CSpaceObject *pBestObj = NULL;
	Metric rBestDist2 = g_InfiniteDistance * g_InfiniteDistance;
	for (int i = 0; i < GetObjectCount(); i++)
		{
		CSpaceObject *pObj = GetObject(i);
		if (pObj && pObj->GetMapOrbit())
			{
			Metric rDist2 = (pObj->GetPos() - vPos).Length2();
			if (rDist2 < rBestDist2)
				{
				pBestObj = pObj;
				rBestDist2 = rDist2;
				}
			}
		}

	//	If we found an object, create an orbit around it. Otherwise, orbit around origin

	CVector vCenter = (pBestObj ? pBestObj->GetPos() : CVector());
	Metric rRadius;
	Metric rAngle = VectorToPolarRadians(vPos - vCenter, &rRadius);
	COrbit NewOrbit(vCenter, rRadius, rAngle);

	//	Create the station

	SObjCreateCtx CreateCtx;
	CreateCtx.vPos = vPos;
	CreateCtx.pOrbit = &NewOrbit;
	CreateCtx.bCreateSatellites = true;
	CreateCtx.pEventHandler = pEventHandler;

	CSpaceObject *pStation;
	if (error = CreateStation(&Ctx,
			pType,
			CreateCtx,
			&pStation))
		return error;

	//	Fire deferred OnCreate

	if (error = Ctx.Events.FireDeferredEvent(ON_CREATE_EVENT, &Ctx.sError))
		kernelDebugLogMessage("Deferred OnCreate: %s", Ctx.sError);

	//	Recompute encounter table

	m_fEncounterTableValid = false;

	//	Done

	if (retpStation)
		*retpStation = pStation;

	return NOERROR;
	}

ALERROR CSystem::CreateWeaponFire (CWeaponFireDesc *pDesc,
								   CItemEnhancementStack *pEnhancements,
								   DestructionTypes iCause,
								   const CDamageSource &Source,
								   const CVector &vPos,
								   const CVector &vVel,
								   int iDirection,
								   CSpaceObject *pTarget,
								   DWORD dwFlags,
								   CSpaceObject **retpShot)

//	CreateWeaponFire
//
//	Creates a shot

	{
	CSpaceObject *pShot;

	switch (pDesc->m_iFireType)
		{
		case ftBeam:
		case ftMissile:
			{
			CMissile *pMissile;

			CMissile::Create(this,
					pDesc,
					pEnhancements,
					iCause,
					Source,
					vPos,
					vVel,
					iDirection,
					pTarget,
					&pMissile);

			pShot = pMissile;
			break;
			}

		case ftArea:
			{
			CAreaDamage *pArea;

			CAreaDamage::Create(this,
					pDesc,
					pEnhancements,
					iCause,
					Source,
					vPos,
					vVel,
					&pArea);

			pShot = pArea;
			break;
			}

		case ftParticles:
			{
			CParticleDamage *pParticles;

			CParticleDamage::Create(this,
					pDesc,
					pEnhancements,
					iCause,
					Source,
					vPos,
					vVel,
					iDirection,
					pTarget,
					&pParticles);

			pShot = pParticles;
			break;
			}

		case ftRadius:
			{
			CRadiusDamage *pRadius;

			CRadiusDamage::Create(this,
					pDesc,
					pEnhancements,
					iCause,
					Source,
					vPos,
					vVel,
					pTarget,
					&pRadius);

			pShot = pRadius;
			break;
			}

		default:
			ASSERT(FALSE);
			pShot = NULL;
		}

	//	Fire a system events, if we have any handlers

	if (!m_EventHandlers.IsEmpty() && pShot)
		{
		//	Skip any fragments

		if (dwFlags & CWF_FRAGMENT)
			NULL;

		//	If this is an explosion, then fire explosion event

		else if (dwFlags & CWF_EXPLOSION)
			{
			FireOnSystemExplosion(pShot, pDesc, Source);
			}

		//	If this is weapons fire, we fire that event

		else if (dwFlags & CWF_WEAPON_FIRE)
			{
			if (Source.GetObj() && Source.GetObj()->CanAttack())
				FireOnSystemWeaponFire(pShot, pDesc, Source);
			}
		}

	//	Done

	if (retpShot)
		*retpShot = pShot;

	return NOERROR;
	}

ALERROR CSystem::CreateWeaponFragments (CWeaponFireDesc *pDesc,
									    CItemEnhancementStack *pEnhancements,
									    DestructionTypes iCause,
									    const CDamageSource &Source,
									    CSpaceObject *pTarget,
									    const CVector &vPos,
										const CVector &vVel,
									    CSpaceObject *pMissileSource)

//	CreateWeaponFragments
//
//	Creates the fragments from a fragmentation weapon

	{
	DEBUG_TRY

	ALERROR error;
	int i;

	CWeaponFireDesc::SFragmentDesc *pFragDesc = pDesc->GetFirstFragment();
	while (pFragDesc)
		{
		int iFragmentCount = pFragDesc->Count.Roll();
		if (iFragmentCount > 0)
			{
			int *iAngle = new int [iFragmentCount];

			//	If we have lots of fragments then we just pick random angles

			if (iFragmentCount > 90)
				{
				for (i = 0; i < iFragmentCount; i++)
					iAngle[i] = mathRandom(0, 359);
				}

			//	Otherwise, we try to distribute evenly

			else
				{
				int iAngleOffset = mathRandom(0, 359);
				int iAngleVar = 90 / iFragmentCount;

				//	Compute angles for each fragment

				int iAngleInc = 360 / iFragmentCount;
				for (i = 0; i < iFragmentCount; i++)
					iAngle[i] = (360 + iAngleOffset + (iAngleInc * i) + mathRandom(-iAngleVar, iAngleVar)) % 360;
				}

			//	For multitargets, we need to find a target 
			//	for each fragment

			if (pFragDesc->bMIRV)
				{
				TArray<CSpaceObject *> TargetList;

				if (pMissileSource)
					{
					int iFound = pMissileSource->GetNearestVisibleEnemies(iFragmentCount, 
							MAX_TARGET_RANGE, 
							&TargetList, 
							NULL, 
							FLAG_INCLUDE_NON_AGGRESSORS);

					Metric rSpeed = pFragDesc->pDesc->GetInitialSpeed();

					if (iFound > 0)
						{
						for (i = 0; i < iFragmentCount; i++)
							{
							CSpaceObject *pTarget = TargetList[i % iFound];

							//	Calculate direction to fire in

							CVector vTarget = pTarget->GetPos() - vPos;
							Metric rTimeToIntercept = CalcInterceptTime(vTarget, pTarget->GetVel(), rSpeed);
							CVector vInterceptPoint = vTarget + pTarget->GetVel() * rTimeToIntercept;
							iAngle[i] = VectorToPolar(vInterceptPoint, NULL);
							}
						}
					}
				}

			//	The initial velocity is the velocity of the missile
			//	(unless we are MIRVed)

			CVector vInitVel;
			if (!pFragDesc->bMIRV)
				vInitVel = vVel;

			//	Create the fragments

			for (i = 0; i < iFragmentCount; i++)
				{
				CSpaceObject *pNewObj;
				Metric rSpeed = pFragDesc->pDesc->GetInitialSpeed();

				if (error = CreateWeaponFire(pFragDesc->pDesc,
						pEnhancements,
						iCause,
						Source,
						vPos + CVector(mathRandom(-10, 10) * g_KlicksPerPixel / 10.0, mathRandom(-10, 10) * g_KlicksPerPixel / 10.0),
						vInitVel + PolarToVector(iAngle[i], rSpeed),
						iAngle[i],
						pTarget,
						CSystem::CWF_FRAGMENT,
						&pNewObj))
					return error;

				//	Preserve automated weapon flag

				if (pMissileSource && pMissileSource->IsAutomatedWeapon())
					pNewObj->SetAutomatedWeapon();
				}

			delete [] iAngle;
			}

		pFragDesc = pFragDesc->pNext;
		}

	return NOERROR;

	DEBUG_CATCH
	}

bool CSystem::DescendObject (DWORD dwObjID, const CVector &vPos, CSpaceObject **retpObj, CString *retsError)

//	DescendObject
//
//	Descends the object back to the system.

	{
	CSpaceObject *pObj = g_pUniverse->RemoveAscendedObj(dwObjID);
	if (pObj == NULL)
		{
		if (retsError)
			*retsError = CONSTLIT("Object not ascended.");
		return false;
		}

	pObj->SetAscended(false);

	//	Clear the time-stop flag if necessary

	if (pObj->IsTimeStopped())
		pObj->RestartTime();

	//	Place the ship at the gate in the new system

	pObj->Place(vPos);
	pObj->AddToSystem(this);
	pObj->Resume();

	//	Done

	if (retpObj)
		*retpObj = pObj;

	return true;
	}

CSpaceObject *CSystem::FindObject (DWORD dwID)

//	FindObject
//
//	Finds the object with the given ID (or NULL)

	{
	int i;

	for (i = 0; i < GetObjectCount(); i++)
		{
		CSpaceObject *pObj = GetObject(i);
		if (pObj && pObj->GetID() == dwID && !pObj->IsDestroyed())
			return pObj;
		}

	return NULL;
	}

bool CSystem::FindObjectName (CSpaceObject *pObj, CString *retsName)

//	FindObjectName
//
//	Finds the name of the given object (if it has one)

	{
	int i;

	for (i = 0; i < m_NamedObjects.GetCount(); i++)
		if (m_NamedObjects[i] == pObj)
			{
			if (retsName)
				*retsName = m_NamedObjects.GetKey(i);
			return true;
			}

	return false;
	}

bool CSystem::FindRandomLocation (const SLocationCriteria &Criteria, DWORD dwFlags, const COrbit &CenterOrbitDesc, CStationType *pStationToPlace, int *retiLocID)

//	FindRandomLocation
//
//	Finds a random location weighted towards those that match the
//	given criteria.

	{
	//	Generate a table of LocationIDs that match the given criteria.

	TProbabilityTable<int> Table;
	if (!GetEmptyLocations(Criteria, CenterOrbitDesc, pStationToPlace, &Table))
		return false;

	//	Done

	if (retiLocID)
		*retiLocID = Table[Table.RollPos()];

	return true;
	}

void CSystem::FireOnSystemExplosion (CSpaceObject *pExplosion, CWeaponFireDesc *pDesc, const CDamageSource &Source)

//	FireOnSystemExplosion
//
//	Fires OnSystemExplosion event to all handlers

	{
	CSpaceObject *pSource = Source.GetObj();
	CItemType *pWeapon = pDesc->GetWeaponType();
	DWORD dwWeaponUNID = (pWeapon ? pWeapon->GetUNID() : 0);
	CVector vPos = pExplosion->GetPos();

	CSystemEventHandler *pHandler = m_EventHandlers.GetNext();
	while (pHandler)
		{
		if (pHandler->InRange(vPos))
			pHandler->GetObj()->FireOnSystemExplosion(pExplosion, pSource, dwWeaponUNID);

		pHandler = pHandler->GetNext();
		}
	}

void CSystem::FireOnSystemObjAttacked (SDamageCtx &Ctx)

//	FireOnSystemObjAttacked
//
//	Fires OnSystemObjAttacked event to all handlers

	{
	DEBUG_TRY

	CSystemEventHandler *pHandler = m_EventHandlers.GetNext();
	while (pHandler)
		{
		if (pHandler->InRange(Ctx.pObj->GetPos()))
			pHandler->GetObj()->FireOnSystemObjAttacked(Ctx);

		pHandler = pHandler->GetNext();
		}

	DEBUG_CATCH
	}

void CSystem::FireOnSystemObjDestroyed (SDestroyCtx &Ctx)

//	FireOnSystemObjDestroyed
//
//	Fires OnSystemObjDestroyed event to all handlers

	{
	CSystemEventHandler *pHandler = m_EventHandlers.GetNext();
	while (pHandler)
		{
		if (pHandler->InRange(Ctx.pObj->GetPos()))
			pHandler->GetObj()->FireOnSystemObjDestroyed(Ctx);

		pHandler = pHandler->GetNext();
		}
	}

void CSystem::FireOnSystemWeaponFire (CSpaceObject *pShot, CWeaponFireDesc *pDesc, const CDamageSource &Source)

//	FireOnSystemWeaponFire
//
//	Fires OnSystemWeaponFire event to all handlers

	{
	CSpaceObject *pSource = Source.GetObj();
	CItemType *pWeapon = pDesc->GetWeaponType();
	DWORD dwWeaponUNID = (pWeapon ? pWeapon->GetUNID() : 0);
	CVector vPos = pShot->GetPos();

	CSystemEventHandler *pHandler = m_EventHandlers.GetNext();
	while (pHandler)
		{
		if (pHandler->InRange(vPos))
			pHandler->GetObj()->FireOnSystemWeaponFire(pShot, pSource, dwWeaponUNID);

		pHandler = pHandler->GetNext();
		}
	}

void CSystem::FlushEnemyObjectCache (void)

//	FlushEnemyObjectCache
//
//	Flush the enemy object cache

	{
	int i;

	for (i = 0; i < g_pUniverse->GetSovereignCount(); i++)
		g_pUniverse->GetSovereign(i)->FlushEnemyObjectCache();
	}

CString CSystem::GetAttribsAtPos (const CVector &vPos)

//	GetAttribsAtPos
//
//	Returns the attributes at the given position

	{
	CString sAttribs = (m_pTopology ? m_pTopology->GetAttributes() : NULL_STR);
	return ::AppendModifiers(sAttribs, m_Territories.GetAttribsAtPos(vPos));
	}

int CSystem::GetEmptyLocationCount (void)

//	GetEmptyLocationCount
//
//	Returns the number of empty locations

	{
	TArray<int> EmptyLocations;
	m_Locations.GetEmptyLocations(&EmptyLocations);
	return EmptyLocations.GetCount();
	}

bool CSystem::GetEmptyLocations (const SLocationCriteria &Criteria, const COrbit &CenterOrbitDesc, CStationType *pStationToPlace, TProbabilityTable<int> *retTable)

//	GetEmptyLocations
//
//	Returns a list of empty locations that match the given criteria.

	{
	int i;

	retTable->DeleteAll();

	//	Check labels for overlap

	BlockOverlappingLocations();

	//	See if we need to check for distance from center

	CVector vCenter;
	bool bCheckMin = (Criteria.rMinDist != 0.0);
	bool bCheckMax = (Criteria.rMaxDist != 0.0);
	Metric rMinDist2;
	Metric rMaxDist2;
	if (bCheckMin || bCheckMax)
		{
		vCenter = CenterOrbitDesc.GetObjectPos();
		rMinDist2 = Criteria.rMinDist * Criteria.rMinDist;
		rMaxDist2 = Criteria.rMaxDist * Criteria.rMaxDist;
		}
	else
		{
		rMinDist2 = 0.0;
		rMaxDist2 = 0.0;
		}

	//	Loop over all locations and add as appropriate

	for (i = 0; i < m_Locations.GetCount(); i++)
		{
		CLocationDef *pLoc = m_Locations.GetLocation(i);

		//	Skip locations that are not empty.

		if (!pLoc->IsEmpty())
			continue;

		//	Compute the probability based on attributes

		int iChance = CalcLocationWeight(pLoc, Criteria.AttribCriteria);
		if (iChance == 0)
			continue;

		//	If we need to check distance, do it now

		if (bCheckMin || bCheckMax)
			{
			CVector vDist = pLoc->GetOrbit().GetObjectPos() - vCenter;
			Metric rDist2 = vDist.Length2();

			if (bCheckMin && rDist2 < rMinDist2)
				continue;
			else if (bCheckMax && rDist2 > rMaxDist2)
				continue;
			}

		//	Make sure the area is clear

		if (pStationToPlace)
			{
			if (!IsAreaClear(pLoc->GetOrbit().GetObjectPos(), 0.0, 0, pStationToPlace))
				continue;
			}

		//	Add to the table

		retTable->Insert(i, iChance);
		}

	//	Done

	return (retTable->GetCount() > 0);
	}

int CSystem::GetLevel (void)

//	GetLevel
//
//	Returns the level of the system

	{
	if (m_pTopology)
		return m_pTopology->GetLevel();
	else
		return 1;
	}

CSpaceObject *CSystem::GetNamedObject (const CString &sName)

//	GetNamedObject
//
//	Returns the object by name

	{
	CSpaceObject **pPoint;

	pPoint = m_NamedObjects.GetAt(sName);
	if (pPoint == NULL)
		return NULL;

	return *pPoint;
	}

CNavigationPath *CSystem::GetNavPath (CSovereign *pSovereign, CSpaceObject *pStart, CSpaceObject *pEnd)

//	GetNavPath
//
//	Returns the navigation path for the given parameters

	{
	CNavigationPath *pNext = m_NavPaths.GetNext();
	while (pNext)
		{
		if (pNext->Matches(pSovereign, pStart, pEnd))
			return pNext;

		pNext = pNext->GetNext();
		}

	//	If we cannot find an appropriate path, we create a new one

	CNavigationPath *pPath;
	CNavigationPath::Create(this, pSovereign, pStart, pEnd, &pPath);

	m_NavPaths.Insert(pPath);

	return pPath;
	}

CNavigationPath *CSystem::GetNavPathByID (DWORD dwID)

//	GetNavPathByID
//
//	Returns the nav path with the given ID (or NULL if not found)

	{
	CNavigationPath *pNext = m_NavPaths.GetNext();
	while (pNext)
		{
		if (pNext->GetID() == dwID)
			return pNext;

		pNext = pNext->GetNext();
		}

	return NULL;
	}

CSpaceObject *CSystem::GetPlayer (void) const

//	GetPlayer
//
//	Returns the player, if she is in the system (NULL otherwise)

	{
	CSpaceObject *pPlayer = g_pUniverse->GetPlayer();
	if (pPlayer && pPlayer->GetSystem() == this)
		return pPlayer;
	else
		return NULL;
	}

DWORD CSystem::GetSaveVersion (void)

//	GetSaveVersion
//
//	Returns the save version

	{
	return SYSTEM_SAVE_VERSION;
	}

CSpaceEnvironmentType *CSystem::GetSpaceEnvironment (int xTile, int yTile)

//	GetSpaceEnvironment
//
//	Returns the given tile

	{
	if (m_pEnvironment)
		return m_pEnvironment->GetTileType(xTile, yTile);
	else
		return NULL;
	}

CSpaceEnvironmentType *CSystem::GetSpaceEnvironment (const CVector &vPos, int *retxTile, int *retyTile)

//	GetSpaceEnvironment
//
//	Returns the tile at the given position

	{
	if (m_pEnvironment)
		{
		int x, y;

		VectorToTile(vPos, &x, &y);

		if (retxTile)
			*retxTile = x;

		if (retyTile)
			*retyTile = y;

		return m_pEnvironment->GetTileType(x, y);
		}
	else
		{
		if (retxTile)
			*retxTile = -1;

		if (retyTile)
			*retyTile = -1;

		return NULL;
		}
	}

CTopologyNode *CSystem::GetStargateDestination (const CString &sStargate, CString *retsEntryPoint)

//	GetStargateDestination
//
//	Get the destination topology node and entry point

	{
	return m_pTopology->GetGateDest(sStargate, retsEntryPoint);
	}

int CSystem::GetTileSize (void) const

//	GetTileSize
//
//	Returns the tile size in pixels

	{
	InitSpaceEnvironment();
	return m_pEnvironment->GetTileSize();
	}

bool CSystem::HasAttribute (const CVector &vPos, const CString &sAttrib)

//	HasAttribute
//
//	Returns TRUE if the system has the attribute at the given position

	{
	//	If this is the special innerSystem attribute and we're using
	//	defaults, then check the default distance

	if (m_fUseDefaultTerritories && strEquals(sAttrib, SPECIAL_ATTRIB_INNER_SYSTEM))
		return (vPos.Length() < (360 * LIGHT_SECOND));

	//	If this is the special lifeZone attribute and we're using
	//	defaults, the check the default distance

	else if (m_fUseDefaultTerritories && strEquals(sAttrib, SPECIAL_ATTRIB_LIFE_ZONE))
		{
		Metric rDist = vPos.Length();
		return (rDist >= (360 * LIGHT_SECOND) && rDist <= (620 * LIGHT_SECOND));
		}

	//	If this is the special outerSystem attribute and we're using
	//	defaults, the check the default distance

	else if (m_fUseDefaultTerritories && strEquals(sAttrib, SPECIAL_ATTRIB_OUTER_SYSTEM))
		return (vPos.Length() > (620 * LIGHT_SECOND));

	//	If this is the special nearStations attribute, then check to see
	//	if we're near any station

	else if (strEquals(sAttrib, SPECIAL_ATTRIB_NEAR_STATIONS))
		{
		for (int i = 0; i < GetObjectCount(); i++)
			{
			CSpaceObject *pObj = GetObject(i);

			if (pObj && pObj->GetScale() == scaleStructure)
				{
				CVector vDist = vPos - pObj->GetPos();
				Metric rDist = vDist.Length();

				if (rDist < 30 * LIGHT_SECOND)
					return true;
				}
			}

		return false;
		}

	//	Otherwise, check the topology and territories

	else
		{
		if (m_pTopology && m_pTopology->HasAttribute(sAttrib))
			return true;

		return m_Territories.HasAttribute(vPos, sAttrib);
		}
	}

void CSystem::InitSpaceEnvironment (void) const

//	InitSpaceEnvironment
//
//	Initialize if not already

	{
	if (m_pEnvironment == NULL)
		m_pEnvironment = new CEnvironmentGrid(m_pType->GetAPIVersion());
	}

bool CSystem::IsAreaClear (const CVector &vPos, Metric rRadius, DWORD dwFlags, CStationType *pType)

//	IsAreaClear
//
//	Returns TRUE if the region around vPos is clear.

	{
	int i;

	//	If we have a station type, then compute some stuff

	CSovereign *pSourceSovereign;
	Metric rSourceExclusionDist2;
	if (pType)
		{
		pSourceSovereign = pType->GetControllingSovereign();
		rSourceExclusionDist2 = pType->GetEnemyExclusionRadius();
		rSourceExclusionDist2 *= rSourceExclusionDist2;
		}
	else
		{
		pSourceSovereign = NULL;
		rSourceExclusionDist2 = rRadius * rRadius;
		}

	//	Now check against all objects in the system

	for (i = 0; i < GetObjectCount(); i++)
		{
		CSpaceObject *pObj = GetObject(i);

		if (pObj 
				&& pObj->GetScale() == scaleStructure
				&& (pSourceSovereign == NULL 
					|| (dwFlags & IAC_INCLUDE_NON_ENEMIES)
					|| (pObj->GetSovereign() && pObj->GetSovereign()->IsEnemy(pSourceSovereign)))
				&& ((dwFlags & IAC_INCLUDE_NON_ATTACKING) || pObj->CanAttack()))
			{
			//	Compute the distance to this obj

			CVector vDist = vPos - pObj->GetPos();
			Metric rDist2 = vDist.Length2();

			//	Compute the exclusion radius of the object

			Metric rExclusionDist2;
			if (dwFlags & IAC_FIXED_RADIUS)
				rExclusionDist2 = rSourceExclusionDist2;
			else
				{
				CStationType *pObjType = pObj->GetEncounterInfo();
				if (pObjType)
					{
					rExclusionDist2 = pObjType->GetEnemyExclusionRadius();
					rExclusionDist2 *= rExclusionDist2;
					rExclusionDist2 = Max(rSourceExclusionDist2, rExclusionDist2);
					}
				else
					rExclusionDist2 = rSourceExclusionDist2;
				}

			//	If we're too close to an enemy then zone is not clear

			if (rDist2 < rExclusionDist2)
				return false;
			}
		}

	//	If we get this far, then zone is clear

	return true;
	}

bool CSystem::IsStarAtPos (const CVector &vPos)

//	IsStarAtPos
//
//	Returns TRUE if there is a star at the given position.

	{
	int i;

	for (i = 0; i < m_Stars.GetCount(); i++)
		{
		CSpaceObject *pStar = m_Stars.GetObj(i);
		CVector vDist = vPos - pStar->GetPos();
		if (vDist.Length2() < SAME_POS_THRESHOLD2)
			return true;
		}

	return false;
	}

bool CSystem::IsStationInSystem (CStationType *pType)

//	IsStationInSystem
//
//	Returns TRUE if the given station type has already been created in the system

	{
	int i;

	for (i = 0; i < GetObjectCount(); i++)
		{
		for (i = 0; i < GetObjectCount(); i++)
			{
			CSpaceObject *pObj = GetObject(i);
			if (pObj && pObj->GetEncounterInfo() == pType)
				return true;
			}
		}

	return false;
	}

void CSystem::MarkImages (void)

//	MarkImages
//
//	Mark images in use

	{
	int i;

	g_pUniverse->SetLogImageLoad(false);

	//	Mark images for all objects that currently exist in the system.

	for (i = 0; i < GetObjectCount(); i++)
		{
		CSpaceObject *pObj = GetObject(i);

		if (pObj)
			pObj->MarkImages();
		}

	//	Give all types a chance to mark images

	if (m_pEnvironment)
		m_pEnvironment->MarkImages();

	g_pUniverse->SetLogImageLoad(true);
	}

void CSystem::NameObject (const CString &sName, CSpaceObject *pObj)

//	NameObject
//
//	Name an object

	{
	m_NamedObjects.Insert(sName, pObj);
	pObj->SetNamed();
	}

CVector CSystem::OnJumpPosAdj (CSpaceObject *pObj, const CVector &vPos)

//	OnJumpPosAdj
//
//	The object wants to jump to the given coordinates. This function will call
//	other objects in the system and see if the coords need to be adjusted

	{
	//	See if the system wants to change jump coordinates

	CVector vNewPos = vPos;
	if (m_pType->FireOnObjJumpPosAdj(pObj, &vNewPos))
		return vNewPos;

	return vPos;
	}

void CSystem::PaintDestinationMarker (SViewportPaintCtx &Ctx, CG16bitImage &Dest, int x, int y, CSpaceObject *pObj)

//	PaintDestinationMarker
//
//	Paints a directional indicator

	{
	CVector vPos;

	//	Figure out the bearing for the destination object.

	int iBearing = VectorToPolar(pObj->GetPos() - Ctx.pCenter->GetPos());
	WORD wColor = pObj->GetSymbolColor();

	//	Paint the arrow

	CPaintHelper::PaintArrow(Dest, x, y, iBearing, wColor);

	//	Paint the text

	const CG16bitFont &Font = g_pUniverse->GetNamedFont(CUniverse::fontSRSObjName);
	vPos = PolarToVector(iBearing, 5 * ENHANCED_SRS_BLOCK_SIZE);
	int xText = x - (int)vPos.GetX();
	int yText = y + (int)vPos.GetY();

	DWORD iAlign = alignCenter;
	if (iBearing <= 180)
		yText += 2 * ENHANCED_SRS_BLOCK_SIZE;
	else
		{
		yText -= (2 * ENHANCED_SRS_BLOCK_SIZE);
		iAlign |= alignBottom;
		}

	pObj->PaintHighlightText(Dest, xText, yText, Ctx, (AlignmentStyles)iAlign, wColor);
	}

void CSystem::PaintStarField(CG16bitImage &Dest, const RECT &rcView, CSpaceObject *pCenter, Metric rKlicksPerPixel, WORD wSpaceColor)

//	PaintStarField
//
//	Paints the system star field

	{
	int i;

	int cxField = RectWidth(rcView);
	int cyField = RectHeight(rcView);

	//	Make sure the star field is created to fit the viewport

	CreateStarField(cxField, cyField);

	//	Compute the minimum brightness to paint

	WORD wMaxColor = (WORD)(Max(Max(CG16bitImage::RedValue(wSpaceColor), CG16bitImage::GreenValue(wSpaceColor)), CG16bitImage::BlueValue(wSpaceColor)));
	WORD wSpaceValue = CG16bitImage::RGBValue(wMaxColor, wMaxColor, wMaxColor);

	//	Get the absolute position of the center

	int xCenter = (int)(pCenter->GetPos().GetX() / rKlicksPerPixel);
	int yCenter = (int)(pCenter->GetPos().GetY() / rKlicksPerPixel);

	//	Precompute the star distance adj

	int xDistAdj[MAX_STAR_DISTANCE + 1];
	int yDistAdj[MAX_STAR_DISTANCE + 1];
	xDistAdj[0] = 1;
	yDistAdj[0] = 1;
	for (i = 1; i < MAX_STAR_DISTANCE + 1; i++)
		{
		xDistAdj[i] = 4 * xCenter / (i * i);
		yDistAdj[i] = 4 * yCenter / (i * i);
		}

	//	Paint each star

	WORD *pStart = Dest.GetRowStart(0);
	int cyRow = Dest.GetRowStart(1) - pStart;

	pStart += cyRow * rcView.top + rcView.left;

	for (i = 0; i < m_StarField.GetCount(); i++)
		{
		CStar *pStar = (CStar *)m_StarField.GetStruct(i);

		//	Adjust the coordinates of the star based on the position
		//	of the center and the distance

		int x = (pStar->x - xDistAdj[pStar->wDistance]) % cxField;
		if (x < 0)
			x += cxField;
		int y = (pStar->y + yDistAdj[pStar->wDistance]) % cyField;
		if (y < 0)
			y += cyField;

		//	Blt the star

		WORD *pPixel = pStart + cyRow * y + x;

		//	Cheap (if inaccurate) test to see if the star is brighter than background

		if (wSpaceValue < pStar->wColor)
			{
			if (pStar->bBrightStar && wSpaceValue < pStar->wSpikeColor)
				{
				if (y < cyField - 1)
					{
					*(pPixel + 1) = pStar->wSpikeColor;
					*(pPixel + cyRow) = pStar->wSpikeColor;
					}

				if (y > 0)
					{
					*(pPixel - 1) = pStar->wSpikeColor;
					*(pPixel - cyRow) = pStar->wSpikeColor;
					}
				}

			*pPixel = pStar->wColor;
			}
		}
	}

void CSystem::PaintViewport (CG16bitImage &Dest, 
							 const RECT &rcView, 
							 CSpaceObject *pCenter, 
							 DWORD dwFlags)

//	PaintViewport
//
//	Paints the system in the viewport

	{
	int i;
	int iLayer;

	ASSERT(pCenter);

	//	Initialize the viewport context

	SViewportPaintCtx Ctx;
	CalcViewportCtx(Ctx, rcView, pCenter, dwFlags);

	//	Keep track of the player object because sometimes we do special processing

	CSpaceObject *pPlayerCenter = (pCenter->IsPlayer() ? pCenter : NULL);

	//	Clear the rect

	Dest.SetClipRect(rcView);
	Dest.Fill(rcView.left, rcView.top, RectWidth(rcView), RectHeight(rcView), Ctx.wSpaceColor);

	//	Paint the star field

	if (!Ctx.fNoStarfield)
		PaintStarField(Dest, rcView, pCenter, g_KlicksPerPixel, Ctx.wSpaceColor);

	//	Compute the bounds relative to the center

	RECT rcBounds;
	rcBounds.left = rcView.left - Ctx.xCenter + (ENHANCED_SRS_BLOCK_SIZE / 2);
	rcBounds.top = rcView.top - Ctx.yCenter + (ENHANCED_SRS_BLOCK_SIZE / 2);
	rcBounds.right = rcView.right - Ctx.xCenter - (ENHANCED_SRS_BLOCK_SIZE / 2);
	rcBounds.bottom = rcView.bottom - Ctx.yCenter - (ENHANCED_SRS_BLOCK_SIZE / 2);

	//	Generate lists of all objects to paint by layer

	for (iLayer = layerSpace; iLayer < layerCount; iLayer++)
		m_LayerObjs[iLayer].RemoveAll();

	m_BackgroundObjs.RemoveAll();
	m_ForegroundObjs.RemoveAll();
	m_EnhancedDisplayObjs.RemoveAll();

	for (i = 0; i < GetObjectCount(); i++)
		{
		CSpaceObject *pObj = GetObject(i);
		if (pObj 
				&& !pObj->IsVirtual() 
				&& pObj != pPlayerCenter)
			{
			Metric rParallaxDist;

			if (pObj->IsOutOfPlaneObj()
					&& ((rParallaxDist = pObj->GetParallaxDist()) != 1.0))
				{
				//	Compute the size of the viewport at the given object's distance

				CVector vParallaxDiag = Ctx.vDiagonal * rParallaxDist;
				CVector vParallaxUR = Ctx.vCenterPos + vParallaxDiag;
				CVector vParallaxLL = Ctx.vCenterPos - vParallaxDiag;

				//	If we're in the viewport, then we add it

				if (pObj->InBox(vParallaxUR, vParallaxLL) && !pObj->IsVirtual())
					{
					pObj->SetPaintNeeded();

					if (rParallaxDist > 1.0)
						m_BackgroundObjs.FastAdd(pObj);
					else
						m_ForegroundObjs.FastAdd(pObj);
					}
				}
			else
				{
				bool bInViewport = pObj->InBox(Ctx.vUR, Ctx.vLL);

				//	If we're in the viewport, then we need to paint on the main screen

				if (bInViewport)
					{
					m_LayerObjs[pObj->GetPaintLayer()].FastAdd(pObj);
					pObj->SetPaintNeeded();
					}

				//	See if we need to paint a marker. Note that sometimes we end up 
				//	painting both because we might be in-bounds (because of effects)
				//	but still off-screen.

				bool bMarker = pObj->IsPlayerTarget()
						|| pObj->IsPlayerDestination()
						|| pObj->IsHighlighted()
						|| (Ctx.fEnhancedDisplay
							&& (pObj->GetScale() == scaleShip || pObj->GetScale() == scaleStructure)
							&& pObj->PosInBox(Ctx.vEnhancedUR, Ctx.vEnhancedLL)
							&& !pObj->IsInactive());

				if (bMarker
						&& (!bInViewport || !pObj->HitSizeInBox(Ctx.vUR, Ctx.vLL)))
					m_EnhancedDisplayObjs.FastAdd(pObj);
				}
			}
		}

	//	Always add the player at the end (so we paint on top of our layer)

	if (pPlayerCenter)
		{
		m_LayerObjs[pPlayerCenter->GetPaintLayer()].FastAdd(pPlayerCenter);
		pPlayerCenter->SetPaintNeeded();
		}

	//	Paint background objects

	ViewportTransform SavedXForm = Ctx.XForm;
	for (i = 0; i < m_BackgroundObjs.GetCount(); i++)
		{
		CSpaceObject *pObj = m_BackgroundObjs.GetObj(i);

		//	Adjust the transform to deal with parallax

		Ctx.XForm = ViewportTransform(Ctx.vCenterPos, pObj->GetParallaxDist() * g_KlicksPerPixel, Ctx.xCenter, Ctx.yCenter);
		Ctx.XFormRel = Ctx.XForm;

		//	Figure out the position of the object in pixels

		int x, y;
		Ctx.XForm.Transform(pObj->GetPos(), &x, &y);

		//	Paint the object in the viewport

		SetProgramState(psPaintingSRS, pObj);

		Ctx.pObj = pObj;
		pObj->Paint(Dest, 
				x,
				y,
				Ctx);

		SetProgramState(psPaintingSRS);
		}
	Ctx.XForm = SavedXForm;
	Ctx.XFormRel = Ctx.XForm;

	//	Paint any space environment

	if (m_pEnvironment)
		m_pEnvironment->Paint(Dest, Ctx, Ctx.vUR, Ctx.vLL);

	//	Paint all the objects by layer

	for (iLayer = layerSpace; iLayer < layerCount; iLayer++)
		for (i = 0; i < m_LayerObjs[iLayer].GetCount(); i++)
			{
			CSpaceObject *pObj = m_LayerObjs[iLayer].GetObj(i);

			if (pObj->IsPaintNeeded())
				{
				//	Figure out the position of the object in pixels

				int x, y;
				Ctx.XForm.Transform(pObj->GetPos(), &x, &y);

				//	Paint the object in the viewport

				SetProgramState(psPaintingSRS, pObj);

				Ctx.pObj = pObj;
				pObj->Paint(Dest, 
						x,
						y,
						Ctx);

				SetProgramState(psPaintingSRS);
				}

			//	Clear destination, if necessary

			if (pObj->IsAutoClearDestination())
				pObj->ClearPlayerDestination();
			}

	//	Paint foreground objects

	SavedXForm = Ctx.XForm;
	for (i = 0; i < m_ForegroundObjs.GetCount(); i++)
		{
		CSpaceObject *pObj = m_ForegroundObjs.GetObj(i);

		//	Compute the transform

		Ctx.XForm = ViewportTransform(Ctx.vCenterPos, pObj->GetParallaxDist() * g_KlicksPerPixel, Ctx.xCenter, Ctx.yCenter);
		Ctx.XFormRel = Ctx.XForm;

		//	Figure out the position of the object in pixels

		int x, y;
		Ctx.XForm.Transform(pObj->GetPos(), &x, &y);

		//	Paint the object in the viewport

		SetProgramState(psPaintingSRS, pObj);

		Ctx.pObj = pObj;
		pObj->Paint(Dest, 
				x,
				y,
				Ctx);

		SetProgramState(psPaintingSRS);
		}
	Ctx.XForm = SavedXForm;
	Ctx.XFormRel = Ctx.XForm;

	//	Paint all the enhanced display markers

	for (i = 0; i < m_EnhancedDisplayObjs.GetCount(); i++)
		{
		CSpaceObject *pObj = m_EnhancedDisplayObjs.GetObj(i);

		//	If this is a destination marker then we paint it at the circumference
		//	of a circle around the center.

		if (pObj->IsPlayerTarget()
				|| pObj->IsPlayerDestination()
				|| pObj->IsHighlighted())
			{
			CVector vDir = (pObj->GetPos() - Ctx.vCenterPos).Normal();

			PaintDestinationMarker(Ctx,
					Dest, 
					Ctx.xCenter + (int)(Ctx.rIndicatorRadius * vDir.GetX()), 
					Ctx.yCenter - (int)(Ctx.rIndicatorRadius * vDir.GetY()),
					pObj);
			}

		//	Otherwise we paint this as a block at the edge of the screen

		else
			{
			//	Figure out the position of the object in pixels
			//	relative to the center of the screen

			int x, y;
			Ctx.XForm.Transform(pObj->GetPos(), &x, &y);
			x = x - Ctx.xCenter;
			y = y - Ctx.yCenter;

			//	Now clip the position to the side of the screen

			if (x >= rcBounds.right)
				{
				y = y * (rcBounds.right - 1) / x;
				x = rcBounds.right - 1;
				}
			else if (x < rcBounds.left)
				{
				y = y * (rcBounds.left) / x;
				x = rcBounds.left;
				}

			if (y >= rcBounds.bottom)
				{
				x = x * (rcBounds.bottom - 1) / y;
				y = rcBounds.bottom - 1;
				}
			else if (y < rcBounds.top)
				{
				x = x * rcBounds.top / y;
				y = rcBounds.top;
				}

			//	Draw the indicator

			WORD wColor = pObj->GetSymbolColor();

			Dest.Fill(Ctx.xCenter + x - (ENHANCED_SRS_BLOCK_SIZE / 2), 
					Ctx.yCenter + y - (ENHANCED_SRS_BLOCK_SIZE / 2),
					ENHANCED_SRS_BLOCK_SIZE, 
					ENHANCED_SRS_BLOCK_SIZE, 
					wColor);
			}
		}

	//	Let the POV paint any other enhanced displays

	pCenter->PaintSRSEnhancements(Dest, Ctx);

	//	Done

	Dest.ResetClipRect();
	}

void CSystem::PaintViewportGrid (CMapViewportCtx &Ctx, CG16bitImage &Dest, Metric rGridSize)

//	PaintViewportGrid
//
//	Paints a grid

	{
	const RECT &rcView = Ctx.GetViewportRect();

	int cxWidth = RectWidth(rcView);
	int cyHeight = RectHeight(rcView);

	//	Figure out where the center is

	int xCenter, yCenter;
	Ctx.Transform(Ctx.GetCenterPos(), &xCenter, &yCenter);

	//	Figure out the grid spacing

	int xSpacing, ySpacing;
	Ctx.Transform(Ctx.GetCenterPos() + CVector(rGridSize, -rGridSize), &xSpacing, &ySpacing);
	xSpacing -= xCenter;
	ySpacing -= yCenter;
	ySpacing = xSpacing;

	if (xSpacing <= 0 || ySpacing <= 0)
		return;

	//	Find the x coordinate of the left-most grid line

	int xStart;
	if (xCenter < rcView.left)
		xStart = xCenter + ((rcView.left - xCenter) / xSpacing) * xSpacing;
	else
		xStart = xCenter - ((xCenter - rcView.left) / xSpacing) * xSpacing;

	//	Paint vertical grid lines

	int x = xStart;
	while (x < rcView.right)
		{
		Dest.FillColumn(x, rcView.top, cyHeight, RGB_GRID_LINE);
		x += xSpacing;
		}

	//	Find the y coordinate of the top-most grid line

	int yStart;
	if (yCenter < rcView.top)
		yStart = yCenter + ((rcView.top - yCenter) / ySpacing) * ySpacing;
	else
		yStart = yCenter - ((yCenter - rcView.top) / ySpacing) * ySpacing;

	//	Paint horizontal grid lines

	int y = yStart;
	while (y < rcView.bottom)
		{
		Dest.FillLine(rcView.left, y, cxWidth, RGB_GRID_LINE);
		y += ySpacing;
		}
	}

void CSystem::PaintViewportObject (CG16bitImage &Dest, const RECT &rcView, CSpaceObject *pCenter, CSpaceObject *pObj)

//	PaintViewportObject
//
//	Paints a single object

	{
	//	Figure out the boundary of the viewport in system coordinates

	int xCenter = rcView.left + RectWidth(rcView) / 2;
	int yCenter = rcView.top + RectHeight(rcView) / 2;

	CVector vDiagonal(g_KlicksPerPixel * (Metric)(RectWidth(rcView) + 256) / 2,
				g_KlicksPerPixel * (Metric)(RectHeight(rcView) + 256) / 2);
	CVector vUR = pCenter->GetPos() + vDiagonal;
	CVector vLL = pCenter->GetPos() - vDiagonal;

	//	Compute the transformation to map world coordinates to the viewport

	SViewportPaintCtx Ctx;
	Ctx.wSpaceColor = CalculateSpaceColor(pCenter);
	Ctx.XForm = ViewportTransform(pCenter->GetPos(), g_KlicksPerPixel, xCenter, yCenter);
	Ctx.XFormRel = Ctx.XForm;

	//	Paint object

	if (pObj && pObj->InBox(vUR, vLL))
		{
		//	Figure out the position of the object in pixels

		int x, y;
		Ctx.XForm.Transform(pObj->GetPos(), &x, &y);

		//	Paint the object in the viewport

		Ctx.pObj = pObj;
		pObj->Paint(Dest, 
				x,
				y,
				Ctx);
		}
	}

void CSystem::PaintViewportLRS (CG16bitImage &Dest, const RECT &rcView, CSpaceObject *pCenter, bool *retbNewEnemies)

//	PaintViewportLRS
//
//	Paints an LRS from the point of view of the given object. We assume that
//	the destination bitmap is already clear; we just draw on top of it.

	{
	int i;
	Metric rKlicksPerPixel = g_LRSRange * 2 / RectWidth(rcView);

	//	Figure out the boundary of the viewport in system coordinates. We generate
	//	a viewport for each detection range 1-5.

	CVector vUR[RANGE_INDEX_COUNT];
	CVector vLL[RANGE_INDEX_COUNT];
	Metric rMaxDist2[RANGE_INDEX_COUNT];

	for (i = 0; i < RANGE_INDEX_COUNT; i++)
		{
		Metric rRange = RangeIndex2Range(i);

		//	Player ship can't see beyond LRS range

		if (rRange > g_LRSRange)
			rRange = g_LRSRange;

		CVector vRange(rRange, rRange);
		vUR[i] = pCenter->GetPos() + vRange;
		vLL[i] = pCenter->GetPos() - vRange;
		rMaxDist2[i] = rRange * rRange;
		}

	int iPerception = pCenter->GetPerception();

	//	For planetary and stellar objects we use a larger box to make sure that 
	//	we include it even if it is slightly off screen.

	CVector vLargeDiagonal(rKlicksPerPixel * ((RectWidth(rcView) / 2) + 128),
				rKlicksPerPixel * ((RectHeight(rcView) / 2) + 128));
	CVector vLargeUR = pCenter->GetPos() + vLargeDiagonal;
	CVector vLargeLL = pCenter->GetPos() - vLargeDiagonal;

	//	Compute the transformation to map world coordinates to the viewport

	int xCenter = rcView.left + RectWidth(rcView) / 2;
	int yCenter = rcView.top + RectHeight(rcView) / 2;
	ViewportTransform Trans(pCenter->GetPos(), rKlicksPerPixel, xCenter, yCenter);

	//	Paint any space environment

	if (m_pEnvironment)
		{
		int x, y, x1, y1, x2, y2;

		VectorToTile(vUR[0], &x2, &y1);
		VectorToTile(vLL[0], &x1, &y2);
		
		//	Increase bounds (so we can paint the edges)

		x1--; y1--;
		x2++; y2++;

		for (x = x1; x <= x2; x++)
			for (y = y1; y <= y2; y++)
				{
				CSpaceEnvironmentType *pEnv = m_pEnvironment->GetTileType(x, y);
				if (pEnv)
					{
					int xCenter, yCenter;
					CVector vCenter = TileToVector(x, y);
					Trans.Transform(vCenter, &xCenter, &yCenter);

					pEnv->PaintLRS(Dest, xCenter, yCenter);
					}
				}
		}

	//	Loop over all objects

	m_fEnemiesInLRS = false;
	bool bNewEnemies = false;
	for (i = 0; i < GetObjectCount(); i++)
		{
		CSpaceObject *pObj = GetObject(i);
		if (pObj == NULL)
			continue;

		int iRange;
		if (!pObj->IsInactive()
				&& !pObj->IsVirtual()
				&& pObj->InBox(vLargeUR, vLargeLL)
				&& (pObj->GetScale() == scaleStar 
					|| pObj->GetScale() == scaleWorld 
					|| ((iRange = pObj->GetDetectionRangeIndex(iPerception)) < RANGE_INDEX_COUNT
						&& pCenter->GetDistance2(pObj) <= rMaxDist2[iRange])))
			{
			//	Figure out the position of the object in pixels

			int x, y;
			Trans.Transform(pObj->GetPos(), &x, &y);

			//	Paint the object in the viewport

			pObj->PaintLRS(Dest, x, y, Trans);

			//	This object is now in the LRS

			bool bNewInLRS = pObj->SetPOVLRS();

			//	If an enemy, keep track

			if (pCenter->IsEnemy(pObj))
				{
				if (bNewInLRS 
						&& pObj->GetCategory() == CSpaceObject::catShip)
					bNewEnemies = true;

				m_fEnemiesInLRS = true;
				}
			}
		else
			{
			//	This object is not in the LRS

			pObj->ClearPOVLRS();
			}
		}

	//	If new enemies have appeared in LRS, tell the POV

	if (retbNewEnemies)
		*retbNewEnemies = bNewEnemies;
	}

void CSystem::PaintViewportMap (CG16bitImage &Dest, const RECT &rcView, CSpaceObject *pCenter, Metric rMapScale)

//	PaintViewportMap
//
//	Paints a system map

	{
	int i;
	int x, y;

	//	Initialize context

	CMapViewportCtx Ctx(pCenter->GetPos(), rcView, rMapScale);

	//	Make sure we've initialized the grid

	if (m_GridPainter.IsEmpty())
		{
		for (i = 0; i < m_Stars.GetCount(); i++)
			{
			CSpaceObject *pStar = m_Stars.GetObj(i);
			m_GridPainter.AddRegion(pStar->GetPos(), MAP_GRID_SIZE, MAP_GRID_SIZE);
			}
		}

	//	Clear the rect

	Dest.FillRGB(rcView.left, rcView.top, RectWidth(rcView), RectHeight(rcView), g_rgbSpaceColor);

	//	Paint space environment

	if (m_pEnvironment)
		m_pEnvironment->PaintMap(Ctx, Dest);

	//	Paint grid

	m_GridPainter.Paint(Dest, Ctx);

	//	Paint the glow from all stars

	for (i = 0; i < m_Stars.GetCount(); i++)
		{
		CSpaceObject *pStar = m_Stars.GetObj(i);

		//	Paint glow

		Ctx.Transform(pStar->GetPos(), &x, &y);
		int iGlowRadius = (int)((pStar->GetMaxLightDistance() * LIGHT_SECOND) / rMapScale);

		DrawAlphaGradientCircle(Dest, x, y, iGlowRadius, (WORD)CG16bitImage::PixelFromRGB(pStar->GetSpaceColor()));
		}

	//	Paint all planets and stars first

	for (i = 0; i < GetObjectCount(); i++)
		{
		CSpaceObject *pObj = GetObject(i);

		if (pObj 
				&& (pObj->GetScale() == scaleStar || pObj->GetScale() == scaleWorld)
				&& (pObj->GetMapOrbit() || Ctx.IsInViewport(pObj)))
			{
			//	Figure out the position of the object in pixels

			Ctx.Transform(pObj->GetPos(), &x, &y);

			//	Paint the object in the viewport

			pObj->PaintMap(Ctx,
					Dest, 
					x,
					y);
			}
		}

	//	Paint all structures next

	for (i = 0; i < GetObjectCount(); i++)
		{
		CSpaceObject *pObj = GetObject(i);

		if (pObj 
				&& (pObj->GetScale() == scaleStructure
					|| pObj->GetScale() == scaleShip)
				&& Ctx.IsInViewport(pObj))
			{
			//	Figure out the position of the object in pixels

			Ctx.Transform(pObj->GetPos(), &x, &y);

			//	Paint the object in the viewport

			pObj->PaintMap(Ctx,
					Dest, 
					x,
					y);
			}
		}

	//	Paint NavPaths

#ifdef DEBUG_NAV_PATH
	CNavigationPath *pNext = m_NavPaths.GetNext();
	while (pNext)
		{
		pNext->DebugPaintInfo(Dest, 0, 0, Ctx);

		pNext = pNext->GetNext();
		}
#endif

	//	Paint the POV

	Ctx.Transform(pCenter->GetPos(), &x, &y);
	pCenter->PaintMap(Ctx, Dest, x, y);
	}

void CSystem::PaintViewportMapObject (CG16bitImage &Dest, const RECT &rcView, CSpaceObject *pCenter, CSpaceObject *pObj)

//	PaintViewportMapObject
//
//	Paints a system map object

	{
	int x, y;

	//	Initialize context

	CMapViewportCtx Ctx(pCenter->GetPos(), rcView, g_KlicksPerPixel);

	//	Paint the obj

	Ctx.Transform(pObj->GetPos(), &x, &y);
	pObj->PaintMap(Ctx, Dest, x, y);
	}

void CSystem::PlaceInGate (CSpaceObject *pObj, CSpaceObject *pGate)

//	PlaceInGate
//
//	Place the object at the gate

	{
	ASSERT(pGate);

	CShip *pShip = pObj->AsShip();
	if (pShip == NULL)
		return;

	//	Set at gate position

	pShip->Place(pGate->GetPos(), pGate->GetVel());

	//	We keep on incrementing the timer as long as we are creating ships
	//	in the same tick. [But only if we're not creating the system.]

	if (!m_fInCreate)
		{
		if (m_iTick != g_iGateTimerTick)
			{
			g_iGateTimer = 0;
			g_iGateTimerTick = m_iTick;
			}

		pShip->SetInGate(pGate, g_iGateTimer);
		g_iGateTimer += mathRandom(11, 22);
		}
	else
		pShip->SetInGate(pGate, 0);
	}

void CSystem::PlayerEntered (CSpaceObject *pPlayer)

//	PlayerEntered
//
//	Player has entered the system

	{
	int i;

	//	Tell other objects that the player has entered

	for (i = 0; i < GetObjectCount(); i++)
		{
		CSpaceObject *pObj = GetObject(i);

		if (pObj && pObj != pPlayer)
			pObj->OnPlayerObj(pPlayer);
		}

	//	Mark the node as known

	if (m_pTopology)
		m_pTopology->SetKnown();
	}

void CSystem::ReadObjRefFromStream (SLoadCtx &Ctx, CSpaceObject **retpObj)

//	ReadObjRefFromStream
//
//	Reads the object reference from the stream

	{
	//	Initialize

	*retpObj = NULL;

	//	Load the ID

	DWORD dwID;
	Ctx.pStream->Read((char *)&dwID, sizeof(DWORD));
	if (dwID == OBJID_NULL)
		return;

	//	Lookup the ID in the map

	if (Ctx.ObjMap.Lookup(dwID, (CObject **)retpObj) == NOERROR)
		return;

	//	If we could not find it, add the return pointer as a reference

	Ctx.ForwardReferences.InsertRef(dwID, retpObj);
	}

void CSystem::ReadObjRefFromStream (SLoadCtx &Ctx, void *pCtx, PRESOLVEOBJIDPROC pfnResolveProc)

//	ReadObjRefFromStream
//
//	Reads the object reference from the stream

	{
	//	Load the ID

	DWORD dwID;
	Ctx.pStream->Read((char *)&dwID, sizeof(DWORD));
	if (dwID == OBJID_NULL)
		return;

	//	Lookup the ID in the map. If we find it, then resolve it now.

	CSpaceObject *pObj;
	if (Ctx.ObjMap.Lookup(dwID, (CObject **)&pObj) == NOERROR)
		(pfnResolveProc)(pCtx, dwID, pObj);

	//	If we could not find it, add the return pointer as a reference

	else
		Ctx.ForwardReferences.InsertRef(dwID, pCtx, pfnResolveProc);
	}

void CSystem::ReadSovereignRefFromStream (SLoadCtx &Ctx, CSovereign **retpSovereign)

//	ReadSovereignRefFromStream
//
//	Reads the sovereign reference

	{
	DWORD dwUNID;
	Ctx.pStream->Read((char *)&dwUNID, sizeof(DWORD));
	if (dwUNID == 0xffffffff)
		{
		*retpSovereign = NULL;
		return;
		}

	*retpSovereign = g_pUniverse->FindSovereign(dwUNID);
	}

void CSystem::RegisterEventHandler (CSpaceObject *pObj, Metric rRange)

//	RegisterEventHandler
//
//	Register the object as a handler of system events

	{
	//	See if the object is already registered. If so, then we just
	//	take the new range and return.

	CSystemEventHandler *pNext = m_EventHandlers.GetNext();
	while (pNext)
		{
		if (pNext->GetObj() == pObj)
			{
			pNext->SetRange(rRange);
			return;
			}

		pNext = pNext->GetNext();
		}

	//	Otherwise, we add the event

	CSystemEventHandler *pNew;
	CSystemEventHandler::Create(pObj, rRange, &pNew);
	m_EventHandlers.Insert(pNew);
	}

void CSystem::RemoveObject (SDestroyCtx &Ctx)

//	RemoveObject
//
//	Removes the object from the system without destroying it

	{
	int i;

	//	Tell all other objects that the given object was destroyed
	//	NOTE: The destroyed flag is already set on the object

	ASSERT(Ctx.pObj->IsDestroyed());
	if (Ctx.pObj->NotifyOthersWhenDestroyed())
		{
		DEBUG_SAVE_PROGRAMSTATE;

		//	Notify subscribers

		Ctx.pObj->NotifyOnObjDestroyed(Ctx);

		//	Notify other objects in the system

		for (i = 0; i < GetObjectCount(); i++)
			{
			CSpaceObject *pObj = GetObject(i);

			if (pObj && pObj != Ctx.pObj)
				{
				SetProgramState(psOnObjDestroyed, pObj);

				pObj->OnObjDestroyed(Ctx);
				}
			}

		//	System-level notifications

		CSpaceObject::Categories iObjCat = Ctx.pObj->GetCategory();
		if (iObjCat == CSpaceObject::catShip || iObjCat == CSpaceObject::catStation)
			{
			FireOnSystemObjDestroyed(Ctx);
			g_pUniverse->FireOnGlobalObjDestroyed(Ctx);
			}

		DEBUG_RESTORE_PROGRAMSTATE;
		}

	//	Check to see if the object being destroyed was held by
	//	a timed encounter

	RemoveTimersForObj(Ctx.pObj);

	//	Deal with event handlers

	m_EventHandlers.ObjDestroyed(Ctx.pObj);

	//	Check to see if this was the POV

	if (Ctx.pObj == g_pUniverse->GetPOV())
		{
		//	If this was not the player, then set back to the player

		if (Ctx.pObj != g_pUniverse->GetPlayer() && g_pUniverse->GetPlayer() && !g_pUniverse->GetPlayer()->IsDestroyed())
			g_pUniverse->SetPOV(g_pUniverse->GetPlayer());

		//	Otherwise, set to a marker

		else
			{
			CPOVMarker *pMarker;
			CPOVMarker::Create(this, Ctx.pObj->GetPos(), NullVector, &pMarker);
			g_pUniverse->SetPOV(pMarker);
			}
		}

	m_AllObjects.ReplaceObject(Ctx.pObj->GetIndex(), NULL, false);

	//	Invalidate cache of enemy objects

	FlushEnemyObjectCache();

	//	Invalidate encounter table cache

	if (Ctx.pObj->HasRandomEncounters())
		m_fEncounterTableValid = false;

	//	If this was a star then recalc the list of stars

	if (Ctx.pObj->GetScale() == scaleStar)
		ComputeStars();

	//	Debug code to see if we ever delete a barrier in the middle of move

#ifdef DEBUG_PROGRAMSTATE
	if (g_iProgramState == psUpdatingMove)
		{
		if (Ctx.pObj->IsBarrier())
			{
			CString sObj = CONSTLIT("ERROR: Destroying barrier during move.\r\n");

			ReportCrashObj(&sObj, Ctx.pObj);
			kernelDebugLogMessage(sObj);

#ifdef DEBUG
			DebugBreak();
#endif
			}
		}
#endif
	}

void CSystem::RemoveTimersForObj (CSpaceObject *pObj)

//	RemoveTimersForObj
//
//	Remove timers for the given object

	{
	int i;

	for (i = 0; i < GetTimedEventCount(); i++)
		{
		CTimedEvent *pEvent = GetTimedEvent(i);
		if (pEvent->OnObjDestroyed(pObj))
			pEvent->SetDestroyed();
		}
	}

void CSystem::ResetStarField (void)

//	ResetStarField
//
//	Reset the star field

	{
	g_cxStarField = -1;
	g_cyStarField = -1;

	m_StarField.RemoveAll();
	}

void CSystem::RestartTime (void)

//	RestartTime
//
//	Restart time for all

	{
	for (int i = 0; i < GetObjectCount(); i++)
		{
		CSpaceObject *pObj = GetObject(i);

		if (pObj)
			pObj->RestartTime();
		}

	m_iTimeStopped = 0;
	}

ALERROR CSystem::SaveToStream (IWriteStream *pStream)

//	SaveToStream
//
//	Save the system to a stream
//
//	DWORD		m_dwID
//	DWORD		m_iTick
//	DWORD		m_iTimeStopped
//	CString		m_sName
//	CString		Topology node ID
//	DWORD		(unused)
//	DWORD		m_iNextEncounter
//	DWORD		flags
//	DWORD		SAVE VERSION (only if [flags & 0x02])
//	Metric		m_rKlicksPerPixel
//	Metric		m_rTimeScale
//	DWORD		m_iLastUpdated
//
//	DWORD		Number of CNavigationPath
//	CNavigationPath
//
//	CEventHandlers
//
//	DWORD		Number of mission objects
//	CSpaceObject
//
//	DWORD		Number of objects
//	CSpaceObject
//
//	DWORD		Number of named objects
//	CString		entrypoint: name
//	DWORD		entrypoint: CSpaceObject ref
//
//	DWORD		Number of timed events
//	CTimedEvent
//
//	DWORD		Number of environment maps
//	CTileMap
//
//	CLocationList	m_Locations
//	CTerritoryList	m_Territories

	{
	int i;
	DWORD dwSave;

	//	Write basic data

	pStream->Write((char *)&m_dwID, sizeof(DWORD));
	pStream->Write((char *)&m_iTick, sizeof(DWORD));
	pStream->Write((char *)&m_iTimeStopped, sizeof(DWORD));
	m_sName.WriteToStream(pStream);
	m_pTopology->GetID().WriteToStream(pStream);
	dwSave = 0;
	pStream->Write((char *)&dwSave, sizeof(DWORD));
	pStream->Write((char *)&m_iNextEncounter, sizeof(DWORD));

	//	Write flags

	dwSave = 0;
	dwSave |= (m_fNoRandomEncounters ?		0x00000001 : 0);
	dwSave |= 0x00000002;	//	Include version (this is a hack for backwards compatibility)
	dwSave |= (!m_fUseDefaultTerritories ?	0x00000004 : 0);
	dwSave |= (m_fEnemiesInLRS ?			0x00000008 : 0);
	dwSave |= (m_fEnemiesInSRS ?			0x00000010 : 0);
	dwSave |= (m_fPlayerUnderAttack ?		0x00000020 : 0);
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	//	Save version

	dwSave = SYSTEM_SAVE_VERSION;
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	//	Scale

	pStream->Write((char *)&m_rKlicksPerPixel, sizeof(Metric));
	pStream->Write((char *)&m_rTimeScale, sizeof(Metric));
	pStream->Write((char *)&m_iLastUpdated, sizeof(DWORD));

	//	Save navigation paths

	m_NavPaths.WriteToStream(this, pStream);

	//	Save event handlers

	m_EventHandlers.WriteToStream(this, pStream);

	//	Save all objects in the system

	DWORD dwCount = 0;
	for (i = 0; i < GetObjectCount(); i++)
		if (GetObject(i))
			dwCount++;

	pStream->Write((char *)&dwCount, sizeof(DWORD));
	for (i = 0; i < GetObjectCount(); i++)
		{
		CSpaceObject *pObj = GetObject(i);

		if (pObj)
			{
			try
				{
				pObj->WriteToStream(pStream);
				}
			catch (...)
				{
				CString sError = CONSTLIT("Unable to save object:\r\n");
				ReportCrashObj(&sError, pObj);
				kernelDebugLogMessage(sError);
				return ERR_FAIL;
				}
			}
		}

	//	Save all named objects

	dwCount = m_NamedObjects.GetCount();
	pStream->Write((char *)&dwCount, sizeof(DWORD));

	for (i = 0; i < (int)dwCount; i++)
		{
		CSpaceObject *pObj = m_NamedObjects[i];
		m_NamedObjects.GetKey(i).WriteToStream(pStream);
		WriteObjRefToStream(pObj, pStream);
		}

	//	Save timed events

	m_TimedEvents.WriteToStream(this, pStream);

	//	Save environment maps

	dwCount = (m_pEnvironment ? 1 : 0);
	pStream->Write((char *)&dwCount, sizeof(DWORD));
	if (m_pEnvironment)
		m_pEnvironment->WriteToStream(pStream);

	//	Locations & Territories

	m_Locations.WriteToStream(pStream);
	m_Territories.WriteToStream(pStream);

	return NOERROR;
	}

void CSystem::SetLastUpdated (void)

//	SetLastUpdated
//
//	Marks this time as the last time the system was updated.
//	We use this to figure out how much time passed since we last updated

	{
	m_iLastUpdated = g_pUniverse->GetTicks();
	}

void CSystem::SetObjectSovereign (CSpaceObject *pObj, CSovereign *pSovereign)

//	SetObjectSovereign
//
//	Sets the sovereign for the object. We need to do this through the system
//	because we need to flush the enemy object cache.

	{
	pObj->SetSovereign(pSovereign);
	FlushEnemyObjectCache();
	}

void CSystem::SetPOVLRS (CSpaceObject *pCenter)

//	SetPOVLRS
//
//	Sets the POVLRS flag for all objects in the viewport

	{
	//	Figure out the boundary of the viewport in system coordinates

	CVector vDiagonal(g_LRSRange, g_LRSRange);
	CVector vUR = pCenter->GetPos() + vDiagonal;
	CVector vLL = pCenter->GetPos() - vDiagonal;

	//	Loop over all objects

	for (int i = 0; i < GetObjectCount(); i++)
		{
		CSpaceObject *pObj = GetObject(i);

		if (pObj)
			{
			if (pObj->InBox(vUR, vLL))
				pObj->SetPOVLRS();
			else
				pObj->ClearPOVLRS();
			}
		}
	}

void CSystem::SetSpaceEnvironment (int xTile, int yTile, CSpaceEnvironmentType *pEnvironment)

//	SetSpaceEnvironment
//
//	Sets the space environment

	{
	InitSpaceEnvironment();
	m_pEnvironment->SetTileType(xTile, yTile, pEnvironment);
	}

ALERROR CSystem::StargateCreated (CSpaceObject *pGate, const CString &sStargateID, const CString &sDestNodeID, const CString &sDestEntryPoint)

//	StargateCreated
//
//	Called whenever a stargate object is created in the system. This code will
//	fix up the topology nodes as appropriate.

	{
	//	Get the ID of the stargate

	CString sGateID;
	if (sStargateID.IsBlank())
		sGateID = strPatternSubst(CONSTLIT("ID%x"), pGate->GetID());
	else
		sGateID = sStargateID;

	//	Look for the stargate in the topology; if we don't find it, then we need to add it

	if (!m_pTopology->FindStargate(sGateID))
		m_pTopology->AddStargate(sGateID, sDestNodeID, sDestEntryPoint);

	//	Add this as a named object (so we can come back here)

	if (GetNamedObject(sGateID) == NULL)
		NameObject(sGateID, pGate);

	return NOERROR;
	}

void CSystem::StopTime (const CSpaceObjectList &Targets, int iDuration)

//	StopTime
//
//	Stops time for all targets

	{
	if (IsTimeStopped() || iDuration == 0)
		return;

	for (int i = 0; i < Targets.GetCount(); i++)
		{
		CSpaceObject *pObj = Targets.GetObj(i);

		if (pObj && !pObj->IsTimeStopImmune())
			pObj->StopTime();
		}

	m_iTimeStopped = iDuration;
	}

void CSystem::StopTimeForAll (int iDuration, CSpaceObject *pExcept)

//	StopTimeForAll
//
//	Stop time for all objects in the system (except exception)

	{
	if (IsTimeStopped() || iDuration == 0)
		return;

	for (int i = 0; i < GetObjectCount(); i++)
		{
		CSpaceObject *pObj = GetObject(i);

		if (pObj && pObj != pExcept && !pObj->IsTimeStopImmune())
			pObj->StopTime();
		}

	m_iTimeStopped = iDuration;
	}

CVector CSystem::TileToVector (int x, int y) const

//	TileToVector
//
//	Convert space environment coordinates

	{
	InitSpaceEnvironment();
	return m_pEnvironment->TileToVector(x, y);
	}

void CSystem::TransferObjEventsIn (CSpaceObject *pObj, CTimedEventList &ObjEvents)

//	TransferObjEventsIn
//
//	Moves all of the timed events in ObjEvents to the system

	{
	int i;

	for (i = 0; i < ObjEvents.GetCount(); i++)
		{
		CTimedEvent *pEvent = ObjEvents.GetEvent(i);

		//	Set tick to be relative to new system

		pEvent->SetTick(m_iTick + pEvent->GetTick());

		//	Move

		ObjEvents.MoveEvent(i, m_TimedEvents);
		i--;
		}
	}

void CSystem::TransferObjEventsOut (CSpaceObject *pObj, CTimedEventList &ObjEvents)

//	TransferObjEventsOut
//
//	Moves any timed events for the given object out of the system and into
//	ObjEvents.

	{
	int i;

	for (i = 0; i < GetTimedEventCount(); i++)
		{
		CTimedEvent *pEvent = GetTimedEvent(i);
		if (pEvent->OnObjChangedSystems(pObj))
			{
			//	Set the tick to an offset from system time

			pEvent->SetTick(pEvent->GetTick() - m_iTick);

			//	Move to list

			m_TimedEvents.MoveEvent(i, ObjEvents);
			i--;
			}
		}
	}

void CSystem::UnnameObject (CSpaceObject *pObj)

//	UnnameObject
//
//	Remove the name for the object

	{
	int i;

	for (i = 0; i < m_NamedObjects.GetCount(); i++)
		if (m_NamedObjects[i] == pObj)
			m_NamedObjects.Delete(i);
	}

void CSystem::UnregisterEventHandler (CSpaceObject *pObj)

//	UnregisterEventHandler
//
//	Removes the handler

	{
	CSystemEventHandler *pNext = m_EventHandlers.GetNext();
	while (pNext)
		{
		if (pNext->GetObj() == pObj)
			{
			m_EventHandlers.Remove(pNext);
			break;
			}

		pNext = pNext->GetNext();
		}
	}

void CSystem::Update (SSystemUpdateCtx &SystemCtx)

//	Update
//
//	Updates the system

	{
	int i;
#ifdef DEBUG_PERFORMANCE
	int iUpdateObj = 0;
	int iMoveObj = 0;
#endif

	//	Set up context

	SUpdateCtx Ctx;
	Ctx.pSystem = this;
	Ctx.pPlayer = GetPlayer();

	//	Initialize the player weapon context so that we can select the auto-
	//	target.

	if (Ctx.pPlayer)
		{
		//	Check to see if the primary weapon requires autotargetting

		CInstalledDevice *pWeapon = Ctx.pPlayer->GetNamedDevice(devPrimaryWeapon);
		if (pWeapon)
			{
			CItemCtx ItemCtx(Ctx.pPlayer, pWeapon);
			Ctx.bNeedsAutoTarget = pWeapon->GetClass()->NeedsAutoTarget(ItemCtx, &Ctx.iMinFireArc, &Ctx.iMaxFireArc);
			}

		//	If the primary does not need it, check the missile launcher

		CInstalledDevice *pLauncher;
		if (!Ctx.bNeedsAutoTarget
				&& (pLauncher = Ctx.pPlayer->GetNamedDevice(devMissileWeapon)))
			{
			CItemCtx ItemCtx(Ctx.pPlayer, pLauncher);
			Ctx.bNeedsAutoTarget = pLauncher->GetClass()->NeedsAutoTarget(ItemCtx, &Ctx.iMinFireArc, &Ctx.iMaxFireArc);
			}

		//	Set up perception and max target dist

		Ctx.iPlayerPerception = Ctx.pPlayer->GetPerception();
		Ctx.rTargetDist2 = MAX_AUTO_TARGET_DISTANCE * MAX_AUTO_TARGET_DISTANCE;
		}

	//	Delete all objects in the deleted list (we do this at the
	//	beginning because we want to keep the list after the update
	//	so that callers can examine it).

	for (i = 0; i < m_DeletedObjects.GetCount(); i++)
		{
		CSpaceObject *pObj = m_DeletedObjects.GetObj(i);
		if (pObj->IsNamed())
			{
			}

		delete pObj;
		}
	m_DeletedObjects.RemoveAll();

	//	Fire timed events
	//	NOTE: We only do this if we have a player because otherwise, some
	//	of the scripts might crash. We won't have a player when we first
	//	create the universe.

	SetProgramState(psUpdatingEvents);
	if (!IsTimeStopped() && (g_pUniverse->GetPlayer() || SystemCtx.bForceEventFiring))
		m_TimedEvents.Update(m_iTick, this);

	//	Add all objects to the grid so that we can do faster
	//	hit tests

	DebugStartTimer();
	m_ObjGrid.DeleteAll();
	for (i = 0; i < GetObjectCount(); i++)
		{
		CSpaceObject *pObj = GetObject(i);
		if (pObj && pObj->CanBeHit())
			m_ObjGrid.AddObject(pObj);
		}
	DebugStopTimer("Adding objects to grid");

	//	If necessary, mark as painted so that objects update correctly.

	if (SystemCtx.bForcePainted)
		{
		for (i = 0; i < GetObjectCount(); i++)
			{
			CSpaceObject *pObj = GetObject(i);
			if (pObj)
				pObj->SetPainted();
			}
		}

	//	Give all objects a chance to react

	m_fPlayerUnderAttack = false;
	DebugStartTimer();
	for (i = 0; i < GetObjectCount(); i++)
		{
		CSpaceObject *pObj = GetObject(i);

		if (pObj && !pObj->IsTimeStopped())
			{
			SetProgramState(psUpdatingBehavior, pObj);
			pObj->Behavior(Ctx);

			//	Update the objects

			SetProgramState(psUpdatingObj, pObj);
			pObj->Update(Ctx);

			//	NOTE: pObj may have been destroyed after
			//	Update(). Do not use the pointer.

#ifdef DEBUG_PERFORMANCE
			iUpdateObj++;
#endif
			}
		}
	DebugStopTimer("Updating objects");

	//	Initialize a structure that holds context for motion

	DebugStartTimer();

	m_BarrierObjects.SetAllocSize(GetObjectCount());
	m_GravityObjects.SetAllocSize(GetObjectCount());

	//	Make a list of all barrier and gravity objects

	for (i = 0; i < GetObjectCount(); i++)
		{
		CSpaceObject *pObj = GetObject(i);
		if (pObj && !pObj->IsDestroyed())
			{
			if (pObj->IsBarrier())
				m_BarrierObjects.FastAdd(pObj);

			if (pObj->HasGravity())
				m_GravityObjects.FastAdd(pObj);
			}
		}

	//	Accelerate objects affected by gravity

	for (i = 0; i < m_GravityObjects.GetCount(); i++)
		UpdateGravity(Ctx, m_GravityObjects.GetObj(i));

	//	Move all objects. Note: We always move last because we want to
	//	paint right after a move. Otherwise, when a laser/missile hits
	//	an object, the laser/missile is deleted (in update) before it
	//	gets a chance to paint.

	for (i = 0; i < GetObjectCount(); i++)
		{
		CSpaceObject *pObj = GetObject(i);

		if (pObj && pObj->IsMobile() && !pObj->IsTimeStopped())
			{
			//	Move the objects

			SetProgramState(psUpdatingMove, pObj);
			pObj->Move(m_BarrierObjects, SystemCtx.rSecondsPerTick);

#ifdef DEBUG_PERFORMANCE
			iMoveObj++;
#endif
			}
		}
	DebugStopTimer("Moving objects");

	//	Update random encounters

	SetProgramState(psUpdatingEncounters);
	if (m_iTick >= m_iNextEncounter
			&& !IsTimeStopped())
		UpdateRandomEncounters();

	//	Update time stopped

	SetProgramState(psUpdating);
	if (IsTimeStopped())
		if (m_iTimeStopped > 0 && --m_iTimeStopped == 0)
			RestartTime();

	//	Give the player ship a chance to do something with data that we've
	//	accumulated during update. For example, we use this to set the nearest
	//	docking port.

	CSpaceObject *pPlayer = GetPlayer();
	if (pPlayer && !pPlayer->IsDestroyed())
		pPlayer->UpdatePlayer(Ctx);

	//	Perf output

#ifdef DEBUG_PERFORMANCE
	{
	char szBuffer[1024];
	wsprintf(szBuffer, "Objects: %d  Updating: %d  Moving: %d  Barriers: %d\n", 
			GetObjectCount(), 
			iUpdateObj, 
			iMoveObj,
			m_BarrierObjects.GetCount());
	::OutputDebugString(szBuffer);
	}
#endif

	//	Next

	m_iTick++;
	}

void CSystem::UpdateExtended (const CTimeSpan &ExtraTime)

//	UpdateExtended
//
//	Updates the system for many ticks

	{
	int i;

	SSystemUpdateCtx UpdateCtx;

	//	Update for a few seconds

	int iTime = mathRandom(250, 350);
	for (i = 0; i < iTime; i++)
		Update(UpdateCtx);

	//	Give all objects a chance to update

	for (i = 0; i < GetObjectCount(); i++)
		{
		CSpaceObject *pObj = GetObject(i);

		if (pObj)
			{
			SetProgramState(psUpdatingExtended, pObj);
			pObj->UpdateExtended(ExtraTime);
			}
		}

	SetProgramState(psUpdating);
	}

void CSystem::UpdateGravity (SUpdateCtx &Ctx, CSpaceObject *pGravityObj)

//	UpdateGravity
//
//	Accelerates objects around high-gravity fields

	{
	int i;

	//	Compute the acceleration due to gravity at the scale radius
	//	(in kilometers per second-squared).

	Metric rScaleRadius;
	Metric r1EAccel = pGravityObj->GetGravity(&rScaleRadius);
	if (r1EAccel <= 0.0)
		return;

	Metric rScaleRadius2 = rScaleRadius * rScaleRadius;

	//	Compute the radius at which we get ripped apart

	Metric rTidalKillDist2 = r1EAccel * rScaleRadius2 / TIDAL_KILL_THRESHOLD;

	//	We don't care about accelerations less than 1 km/sec^2.

	const Metric MIN_ACCEL = 1.0;

	//	Compute the radius at which the acceleration is the minimum that we 
	//	care about.
	//
	//	minA = A/r^2
	//	r = sqrt(A/minA) * Earth-radius

	Metric rMaxDist = sqrt(r1EAccel / MIN_ACCEL) * rScaleRadius;
	Metric rMaxDist2 = rMaxDist * rMaxDist;

	//	Loop over all objects inside the given distance and accelerate them.

	CSpaceObjectList Objs;
	GetObjectsInBox(pGravityObj->GetPos(), rMaxDist, Objs);

	for (i = 0; i < Objs.GetCount(); i++)
		{
		//	Skip objects not affected by gravity

		CSpaceObject *pObj = Objs.GetObj(i);
		if (pObj == pGravityObj 
				|| pObj->IsDestroyed()
				|| !pObj->IsMobile()
				|| pObj->GetDockedObj() != NULL)
			continue;

		//	Skip objects outside the maximum range

		CVector vDist = (pGravityObj->GetPos() - pObj->GetPos());
		Metric rDist2 = pGravityObj->GetDistance2(pObj);
		if (rDist2 > rMaxDist2)
			continue;

		//	Inside the kill radius, we destroy the object

		if (rDist2 < rTidalKillDist2)
			{
			if (pObj->OnDestroyCheck(killedByGravity, pGravityObj))
				pObj->Destroy(killedByGravity, pGravityObj);

			continue;
			}

		//	Compute acceleration

		Metric rAccel = r1EAccel * rScaleRadius2 / rDist2;

		//	Accelerate towards the center

		pObj->DeltaV(g_SecondsPerUpdate * rAccel * vDist / sqrt(rDist2));
		pObj->ClipSpeed(LIGHT_SPEED);

		//	If this is the player, then gravity warning

		if (pObj == Ctx.pPlayer && rAccel > GRAVITY_WARNING_THRESHOLD)
			Ctx.bGravityWarning = true;
		}
	}

void CSystem::UpdateRandomEncounters (void)

//	UpdateRandomEncounters
//
//	Updates random encounters

	{
	struct SEncounter
		{
		int iWeight;
		CSpaceObject *pObj;
		IShipGenerator *pTable;
		};

	int i;

	if (m_fNoRandomEncounters)
		return;

	//	No need for random encounters if the player isn't in the system

	CSpaceObject *pPlayer = GetPlayer();
	if (pPlayer == NULL || pPlayer->IsDestroyed())
		return;

	IShipGenerator *pTable = NULL;
	CSpaceObject *pBase = NULL;
	CDesignType *pType = NULL;
	CSovereign *pBaseSovereign = NULL;

	//	Some percent of the time we generate a generic level encounter; the rest of the
	//	time, the encounter is based on the stations in this system.

	if (mathRandom(1, 100) <= LEVEL_ENCOUNTER_CHANCE)
		g_pUniverse->GetRandomLevelEncounter(GetLevel(), &pType, &pTable, &pBaseSovereign);
	else
		{
		//	Compute the list of all objects that have encounters (and cache it)

		ComputeRandomEncounters();

		//	Allocate and fill-in the table

		if (m_EncounterObjs.GetCount() > 0)
			{
			SEncounter *pMainTable = new SEncounter [m_EncounterObjs.GetCount()];
			int iCount = 0;
			int iTotal = 0;
			for (i = 0; i < m_EncounterObjs.GetCount(); i++)
				{
				CSpaceObject *pObj = m_EncounterObjs.GetObj(i);

				//	Get frequency and (optionally) table

				int iFreq;
				IShipGenerator *pTable = pObj->GetRandomEncounterTable(&iFreq);

				//	Adjust frequency to account for the player's distance from the object

				Metric rDist = Max(LIGHT_MINUTE, pPlayer->GetDistance(pObj));
				Metric rDistAdj = (rDist <= MAX_ENCOUNTER_DIST ? LIGHT_MINUTE / rDist : 0.0);
				iFreq = (int)(iFreq * 10.0 * rDistAdj);

				//	Add to table

				if (iFreq > 0)
					{
					pMainTable[iCount].iWeight = iFreq;
					pMainTable[iCount].pObj = pObj;
					pMainTable[iCount].pTable = pTable;

					iTotal += iFreq;
					iCount++;
					}
				}

			//	Pick a random entry in the table

			if (iTotal > 0)
				{
				int iRoll = mathRandom(0, iTotal - 1);
				int iPos = 0;

				//	Get the position

				while (pMainTable[iPos].iWeight <= iRoll)
					iRoll -= pMainTable[iPos++].iWeight;

				//	Done

				pTable = pMainTable[iPos].pTable;
				pBase = pMainTable[iPos].pObj;
				if (pBase)
					pType = pBase->GetType();
				}

			delete [] pMainTable;
			}
		}

	//	If we've got a table, then create the random encounter

	if (pTable)
		CreateRandomEncounter(pTable, pBase, pBaseSovereign, pPlayer);

	//	Otherwise, fire the OnRandomEncounter event

	else if (pType)
		pType->FireOnRandomEncounter(pBase);

	//	Next encounter

	m_iNextEncounter = m_iTick + mathRandom(6000, 9000);
	}

void CSystem::VectorToTile (const CVector &vPos, int *retx, int *rety) const

//	VectorToTile
//
//	Convert space environment coordinates

	{
	InitSpaceEnvironment();
	m_pEnvironment->VectorToTile(vPos, retx, rety);
	}

void CSystem::WriteObjRefToStream (CSpaceObject *pObj, IWriteStream *pStream, CSpaceObject *pReferrer)

//	WriteObjRefToStream
//
//	DWORD		0xffffffff if NULL
//				Otherwise, index of object in system

	{
	DWORD dwSave = OBJID_NULL;
	if (pObj)
		{
		dwSave = pObj->GetID();
		ASSERT(dwSave != 0xDDDDDDDD);

		//	Make sure we save references to objects in the current system.
		//	This will help to track a bug in gating objects.
		//
		//	If the object has no system then it means that it is not tied to
		//	a particular system.

		CSystem *pObjSystem = pObj->GetSystem();
		if (pObjSystem && pObjSystem != this)
			{
			kernelDebugLogMessage("Save file error: Saving reference to object in another system");

			CString sError;
			ReportCrashObj(&sError, pObj);
			kernelDebugLogMessage("Object being referenced:");
			kernelDebugLogMessage(sError);

			if (pReferrer)
				{
				ReportCrashObj(&sError, pReferrer);
				kernelDebugLogMessage("Referring object:");
				kernelDebugLogMessage(sError);
				}

			dwSave = OBJID_NULL;
			}
		}

	pStream->Write((char *)&dwSave, sizeof(DWORD));
	}

void CSystem::WriteSovereignRefToStream (CSovereign *pSovereign, IWriteStream *pStream)

//	WriteSovereignRefToStream
//
//	DWORD		0xffffffff if NULL
//				Otherwise, UNID

	{
	DWORD dwSave = 0xffffffff;
	if (pSovereign)
		dwSave = pSovereign->GetUNID();

	pStream->Write((char *)&dwSave, sizeof(DWORD));
	}

//	Miscellaneous --------------------------------------------------------------

bool CalcOverlap (SLabelEntry *pEntries, int iCount)
	{
	bool bOverlap = false;
	int i, j;

	for (i = 0; i < iCount; i++)
		{
		pEntries[i].iNewPosition = labelPosNone;

		for (j = 0; j < iCount; j++)
			if (i != j)
				{
				if (RectsIntersect(pEntries[i].rcLabel, pEntries[j].rcLabel))
					{
					int xDelta = pEntries[j].x - pEntries[i].x;
					int yDelta = pEntries[j].y - pEntries[i].y;

					switch (pEntries[i].iPosition)
						{
						case labelPosRight:
							{
							if (xDelta > 0)
								pEntries[i].iNewPosition = labelPosLeft;
							break;
							}

						case labelPosLeft:
							{
							if (xDelta < 0)
								pEntries[i].iNewPosition = labelPosBottom;
							break;
							}
						}

					bOverlap = true;
					break;
					}
				}
		}

	return bOverlap;
	}

void SetLabelBelow (SLabelEntry &Entry, int cyChar)
	{
	Entry.rcLabel.top = Entry.y + LABEL_SPACING_Y + LABEL_OVERLAP_Y;
	Entry.rcLabel.bottom = Entry.rcLabel.top + cyChar - (2 * LABEL_OVERLAP_Y);
	Entry.rcLabel.left = Entry.x - (Entry.cxLabel / 2);
	Entry.rcLabel.right = Entry.rcLabel.left + Entry.cxLabel;

	Entry.iPosition = labelPosBottom;
	}

void SetLabelLeft (SLabelEntry &Entry, int cyChar)
	{
	Entry.rcLabel.left = Entry.x - (LABEL_SPACING_X + Entry.cxLabel);
	Entry.rcLabel.top = Entry.y - (cyChar / 2) + LABEL_OVERLAP_Y;
	Entry.rcLabel.right = Entry.rcLabel.left + Entry.cxLabel;
	Entry.rcLabel.bottom = Entry.rcLabel.top + cyChar - (2 * LABEL_OVERLAP_Y);

	Entry.iPosition = labelPosLeft;
	}

void SetLabelRight (SLabelEntry &Entry, int cyChar)
	{
	Entry.rcLabel.left = Entry.x + LABEL_SPACING_X;
	Entry.rcLabel.top = Entry.y - (cyChar / 2) + LABEL_OVERLAP_Y;
	Entry.rcLabel.right = Entry.rcLabel.left + Entry.cxLabel;
	Entry.rcLabel.bottom = Entry.rcLabel.top + cyChar - (2 * LABEL_OVERLAP_Y);

	Entry.iPosition = labelPosRight;
	}
