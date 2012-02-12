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
//	See: TSEUtil.h for definition of SYSTEM_SAVE_VERSION

#include "PreComp.h"

#include "math.h"

#define ENHANCED_SRS_BLOCK_SIZE			6

#define LEVEL_ENCOUNTER_CHANCE			10
const Metric MAX_ENCOUNTER_DIST	=		(30.0 * LIGHT_MINUTE);

#define MAX_TARGET_RANGE				(24.0 * LIGHT_SECOND)

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

bool CalcOverlap (SLabelEntry *pEntries, int iCount);
void SetLabelBelow (SLabelEntry &Entry, int cyChar);
void SetLabelLeft (SLabelEntry &Entry, int cyChar);
void SetLabelRight (SLabelEntry &Entry, int cyChar);

CSystem::CSystem (void) : CObject(&g_Class),
		m_iTick(0),
		m_AllObjects(TRUE),
		m_NamedObjects(FALSE, FALSE),
		m_iTimeStopped(0),
		m_rKlicksPerPixel(KLICKS_PER_PIXEL),
		m_rTimeScale(TIME_SCALE),
		m_iLastUpdated(-1),
		m_fInCreate(false),
		m_fEncounterTableValid(false),
		m_StarField(sizeof(CStar), STARFIELD_COUNT),
		m_ObjGrid(GRID_SIZE, CELL_SIZE, CELL_BORDER)

//	CSystem constructor

	{
	ASSERT(FALSE);
	}

CSystem::CSystem (CUniverse *pUniv, CTopologyNode *pTopology) : CObject(&g_Class),
		m_dwID(OBJID_NULL),
		m_iTick(0),
		m_pTopology(pTopology),
		m_AllObjects(TRUE),
		m_NamedObjects(FALSE, FALSE),
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

	int iDistFromCenter = (iBestDist < 15 ? 0 : iBestDist - 15);
	int iPercent = 100 - (iDistFromCenter * 100 / 120);

	if (retpStar)
		*retpStar = pBestObj;

	return max(0, iPercent);
	}

int CSystem::CalcLocationWeight (int iLocID, const CAttributeCriteria &Criteria)

//	CalcLocationWeight
//
//	Calculates the weight of the given location relative to the given
//	criteria.
//
//	See: ComputeWeightAdjFromMatchStrength
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

	CLocationDef *pLoc = GetLocation(iLocID);
	const CString &sAttributes = pLoc->GetAttributes();
	CVector vPos = pLoc->GetOrbit().GetObjectPos();

	for (i = 0; i < Criteria.GetCount(); i++)
		{
		int iMatchStrength;
		const CString &sAttrib = Criteria.GetAttribAndWeight(i, &iMatchStrength);

		//	Do we have the attribute? Check the location and any attributes
		//	inherited from territories and the system.

		bool bHasAttrib = (::HasModifier(sAttributes, sAttrib)
				|| HasAttribute(vPos, sAttrib));

		//	Adjust probability based on the match strength

		int iAdj = ComputeWeightAdjFromMatchStrength(bHasAttrib, iMatchStrength);
		iWeight = iWeight * iAdj / 1000;

		//	If weight is 0, then no need to continue

		if (iWeight == 0)
			return 0;
		}

	//	Done

	return iWeight;
	}

COLORREF CSystem::CalculateSpaceColor (CSpaceObject *pPOV)

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

	return RGB(iRed, iGreen, iBlue);
	}

void CSystem::CancelTimedEvent (CSpaceObject *pSource, const CString &sEvent, bool bInDoEvent)

//	CancelTimedEvent
//
//	Cancel event by name

	{
	int i;

	for (i = 0; i < GetTimedEventCount(); i++)
		{
		CTimedEvent *pEvent = GetTimedEvent(i);
		if (pEvent->GetEventHandlerObj() == pSource 
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

	int cxChar = g_pUniverse->GetMapLabelFont().GetAverageWidth();
	int cyChar = g_pUniverse->GetMapLabelFont().GetHeight();

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
			Labels[iLabelCount].cxLabel = g_pUniverse->GetMapLabelFont().MeasureText(pObj->GetName(NULL));

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

		if (pObj && pObj->GetScale() == scaleStar)
			m_Stars.Add(pObj);
		}
	}

void CSystem::ConvertSpaceEnvironmentToPointers (CTileMap &UNIDs, CTileMap **retpPointers)

//	ConvertSpaceEnvironmentToPointers
//
//	Converts m_pEnvironment to pointers (restore from save file)

	{
	*retpPointers = new CTileMap(UNIDs.GetSize(), UNIDs.GetScale());

	STileMapEnumerator k;
	while (UNIDs.HasMore(k))
		{
		DWORD dwTile;
		int xTile;
		int yTile;

		UNIDs.GetNext(k, &xTile, &yTile, &dwTile);
		CSpaceEnvironmentType *pEnv = g_pUniverse->FindSpaceEnvironment(dwTile);
		if (pEnv)
			(*retpPointers)->SetTile(xTile, yTile, (DWORD)pEnv);
		}
	}

void CSystem::ConvertSpaceEnvironmentToUNIDs (CTileMap &Pointers, CTileMap **retpUNIDs)

//	ConvertSpaceEnvironmentToUNIDs
//
//	Converts m_pEnvironment to UNIDs

	{
	*retpUNIDs = new CTileMap(Pointers.GetSize(), Pointers.GetScale());

	STileMapEnumerator k;
	while (Pointers.HasMore(k))
		{
		DWORD dwTile;
		int xTile;
		int yTile;

		Pointers.GetNext(k, &xTile, &yTile, &dwTile);
		CSpaceEnvironmentType *pEnv = (CSpaceEnvironmentType *)dwTile;
		if (pEnv)
			(*retpUNIDs)->SetTile(xTile, yTile, pEnv->GetUNID());
		}
	}

ALERROR CSystem::CreateFromStream (CUniverse *pUniv, 
								   IReadStream *pStream, 
								   CSystem **retpSystem,
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
	ALERROR error;
	int i;
	DWORD dwLoad;
	DWORD dwCount;

	//	Create a context block for loading

	SLoadCtx Ctx;
	Ctx.dwVersion = 0;	//	Default to 0
	Ctx.pStream = pStream;

	//	Create the new star system

	Ctx.pSystem = new CSystem(pUniv, NULL);
	if (Ctx.pSystem == NULL)
		return ERR_MEMORY;

	//	Load some misc info

	Ctx.pStream->Read((char *)&Ctx.pSystem->m_dwID, sizeof(DWORD));
	Ctx.pStream->Read((char *)&Ctx.pSystem->m_iTick, sizeof(DWORD));
	Ctx.pStream->Read((char *)&Ctx.pSystem->m_iTimeStopped, sizeof(DWORD));
	Ctx.pSystem->m_sName.ReadFromStream(Ctx.pStream);

	//	Load the topology node

	CString sNodeID;
	sNodeID.ReadFromStream(Ctx.pStream);
	Ctx.pSystem->m_pTopology = pUniv->FindTopologyNode(sNodeID);
	Ctx.pSystem->m_pType = pUniv->FindSystemType(Ctx.pSystem->m_pTopology->GetSystemDescUNID());

	//	More misc info

	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	Ctx.pStream->Read((char *)&Ctx.pSystem->m_iNextEncounter, sizeof(DWORD));

	//	Flags

	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	Ctx.pSystem->m_fNoRandomEncounters = ((dwLoad & 0x00000001) ? true : false);
	if (dwLoad & 0x00000002)
		Ctx.pStream->Read((char *)&Ctx.dwVersion, sizeof(DWORD));
	Ctx.pSystem->m_fUseDefaultTerritories = ((dwLoad & 0x00000004) ? false : true);
	Ctx.pSystem->m_fEncounterTableValid = false;

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
			kernelDebugLogMessage("Error loading object");
			return ERR_FAIL;
			}

		//	Add this object to the map

		DWORD dwID = (Ctx.dwVersion >= 41 ? pObj->GetID() : pObj->GetIndex());
		Ctx.ObjMap.AddEntry(dwID, pObj);

		//	Update any previous objects that are waiting for this reference

		CIntArray *pList;
		if (Ctx.ForwardReferences.Lookup(dwID, (CObject **)&pList) == NOERROR)
			{
			for (int j = 0; j < pList->GetCount(); j++)
				{
				CSpaceObject **pAddr = (CSpaceObject **)pList->GetElement(j);
				*pAddr = pObj;
				}

			Ctx.ForwardReferences.RemoveEntry(dwID, NULL);
			}

		//	Set the system (note: this will change the index to the new
		//	system)

		pObj->AddToSystem(Ctx.pSystem);
		}

	//	If there are still references to the player, resolve them now.
	//	[This happens because of a bug in 1.0 RC1]

	if (pPlayerShip)
		{
		CIntArray *pList;
		if (Ctx.ForwardReferences.Lookup(pPlayerShip->GetID(), (CObject **)&pList) == NOERROR)
			{
			for (int j = 0; j < pList->GetCount(); j++)
				{
				CSpaceObject **pAddr = (CSpaceObject **)pList->GetElement(j);
				*pAddr = pPlayerShip;
				}

			Ctx.ForwardReferences.RemoveEntry(pPlayerShip->GetID(), NULL);
			}
		}

	//	If we've got left over references, then dump debug output

	if (Ctx.ForwardReferences.GetCount() > 0)
		{
		kernelDebugLogMessage("Save file error: %d undefined object reference(s)", Ctx.ForwardReferences.GetCount());

		for (i = 0; i < Ctx.ForwardReferences.GetCount(); i++)
			{
			DWORD dwID = Ctx.ForwardReferences.GetKey(i);
			CIntArray *pList = (CIntArray *)Ctx.ForwardReferences.GetValue(i);
			kernelDebugLogMessage("Reference: %d", dwID);

			for (int j = 0; j < pList->GetCount(); j++)
				{
				CSpaceObject **pAddr = (CSpaceObject **)pList->GetElement(j);
				kernelDebugLogMessage("Address: %x", (DWORD)pAddr);
				}
			}

		ASSERT(false);
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
			ASSERT(false);
			kernelDebugLogMessage("Save file error: Unable to find named object: %s [%x]", sName.GetASCIIZPointer(), dwLoad);
			return ERR_FAIL;
			}

		Ctx.pSystem->NameObject(sName, pObj);
		}

	//	Load all timed events

	Ctx.pStream->Read((char *)&dwCount, sizeof(DWORD));
	for (i = 0; i < (int)dwCount; i++)
		{
		CTimedEvent *pEvent;
		CTimedEvent::CreateFromStream(Ctx, &pEvent);
		Ctx.pSystem->AddTimedEvent(pEvent);
		}

	//	Load environment map

	Ctx.pStream->Read((char *)&dwCount, sizeof(DWORD));
	if (dwCount)
		{
		CTileMap *pEnv;
		if (error = CTileMap::CreateFromStream(Ctx.pStream, &pEnv))
			return error;
		ConvertSpaceEnvironmentToPointers(*pEnv, &Ctx.pSystem->m_pEnvironment);
		delete pEnv;
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
			kernelDebugLogMessage("Save file error: Unable to find POV object: %x", dwObjID);

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
				{
				kernelDebugLogMessage("Save file error: Unable to find player ship");
				return ERR_FAIL;
				}

			return NOERROR;
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

	if (pGate && pGate->IsStargate())
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
							 CShip **retpShip)

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

		pTable->CreateShips(CreateCtx);

		//	Return at least one of the ships created

		if (CreateCtx.Result.GetCount() == 0)
			return ERR_NOTFOUND;

		if (retpShip)
			*retpShip = CreateCtx.Result.GetObj(0)->AsShip();

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

	if (error = CShip::CreateFromClass(this, 
			pClass, 
			pController, 
			pOverride,
			pSovereign, 
			vPos, 
			vVel, 
			iRotation,
			pCtx,
			retpShip))
		return error;

	//	If we're coming out of a gate, set the timer

	if (pExitGate)
		PlaceInGate(*retpShip, pExitGate);

	//	Load images, if necessary

	if (!IsCreationInProgress())
		(*retpShip)->LoadImages();

	//	Create escorts, if necessary

	IShipGenerator *pEscorts = pClass->GetEscorts();
	if (pEscorts)
		{
		//	If the ship has changed its position, then the escorts should not
		//	appear at a gate. [This happens when an override handler moves the
		//	ship at create time to make the ship appear near the player.]

		CSpaceObject *pEscortGate = pExitGate;
		if (pExitGate == NULL || (pExitGate->GetPos() - (*retpShip)->GetPos()).Length2() > (LIGHT_SECOND * LIGHT_SECOND))
			pEscortGate = *retpShip;
		else
			pEscortGate = pExitGate;

		//	Create escorts

		SShipCreateCtx ECtx;
		ECtx.pSystem = this;
		ECtx.vPos = pEscortGate->GetPos();
		ECtx.pBase = *retpShip;
		ECtx.pTarget = NULL;
		ECtx.pGate = pEscortGate;

		pEscorts->CreateShips(ECtx);
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
	if (error = CreateStation(pType, vPos, &pObj))
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
	Ctx.pLocalTables = (m_pType ? m_pType->GetLocalSystemTables() : NULL);
	Ctx.pStats = NULL;

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

	CSpaceObject *pStation;
	if (error = CreateStation(&Ctx,
			pType,
			vPos,
			NewOrbit,
			true,
			NULL,
			&pStation))
		return error;

	//	Fire deferred OnCreate

	if (error = Ctx.Events.FireDeferredEvent(ON_CREATE_EVENT, &Ctx.sError))
		kernelDebugLogMessage("Deferred OnCreate: %s", Ctx.sError.GetASCIIZPointer());

	//	Recompute encounter table

	m_fEncounterTableValid = false;

	//	Done

	if (retpStation)
		*retpStation = pStation;

	return NOERROR;
	}

ALERROR CSystem::CreateWeaponFire (CWeaponFireDesc *pDesc,
								   int iBonus,
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
					iBonus,
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
					iBonus,
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
					iBonus,
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
					iBonus,
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
									    int iBonus,
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
						iBonus,
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

			delete iAngle;
			}

		pFragDesc = pFragDesc->pNext;
		}

	return NOERROR;
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

bool CSystem::FindRandomLocation (const CAttributeCriteria &Criteria, DWORD dwFlags, CStationType *pStationToPlace, int *retiLocID)

//	FindRandomLocation
//
//	Finds a random location weighted towards those that match the
//	given criteria.

	{
	//	Check labels for overlap

	BlockOverlappingLocations();

	//	Generate a list of empty locations. If none are left, we
	//	could not find a location

	TArray<int> EmptyLocations;
	if (!GetEmptyLocations(&EmptyLocations))
		return false;

	int iSize = EmptyLocations.GetCount();

	//	Build up a probability table to see which of the entries
	//	in the label list fit the match best.

	int i;
	int *pProbTable = new int[iSize];

	//	Initialize the table

	int iTotal = 0;
	for (i = 0; i < iSize; i++)
		{
		pProbTable[i] = CalcLocationWeight(EmptyLocations[i], Criteria);
		iTotal += pProbTable[i];
		}

	//	If no entries match, then we're done

	if (iTotal == 0)
		{
		delete [] pProbTable;
		return false;
		}

	//	Keep trying until we find something or until we run out of tries

	int iTries = 20;
	int iPos;
	while (iTries)
		{
		//	Pick a random entry in the table

		iPos = 0;
		int iRoll = mathRandom(0, iTotal - 1);

		//	Get the position

		while (pProbTable[iPos] <= iRoll)
			iRoll -= pProbTable[iPos++];

		//	If we're not done trying, see if this position is near enemies
		//	if it is, then pick again

		if (pStationToPlace && iTries > 1)
			{
			CLocationDef *pLoc = GetLocation(EmptyLocations[iPos]);

			if (!IsAreaClear(pLoc->GetOrbit().GetObjectPos(), 0.0, 0, pStationToPlace))
				{
				iTries--;
				continue;
				}
			}

		//	We found something

		break;
		}

	delete [] pProbTable;

	//	If we ran out of tries, then we didn't find anything

	if (iTries == 0)
		return false;

	//	Done

	if (retiLocID)
		*retiLocID = EmptyLocations[iPos];

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
	CSystemEventHandler *pHandler = m_EventHandlers.GetNext();
	while (pHandler)
		{
		if (pHandler->InRange(Ctx.pObj->GetPos()))
			pHandler->GetObj()->FireOnSystemObjAttacked(Ctx);

		pHandler = pHandler->GetNext();
		}
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

int CSystem::GetEmptyLocationCount (void)

//	GetEmptyLocationCount
//
//	Returns the number of empty locations

	{
	TArray<int> EmptyLocations;
	m_Locations.GetEmptyLocations(&EmptyLocations);
	return EmptyLocations.GetCount();
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
	CSpaceObject *pPoint;

	if (m_NamedObjects.Lookup(sName, (CObject **)&pPoint) == NOERROR)
		return pPoint;
	else
		return NULL;
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
		return (CSpaceEnvironmentType *)m_pEnvironment->GetTile(xTile, yTile);
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

		return (CSpaceEnvironmentType *)m_pEnvironment->GetTile(x, y);
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

void CSystem::LoadImages (void)

//	LoadImages
//
//	Load images for use

	{
	int i;

	for (i = 0; i < GetObjectCount(); i++)
		{
		CSpaceObject *pObj = GetObject(i);

		if (pObj)
			{
			SetProgramState(psLoadingImages, pObj);
			pObj->LoadImages();
			}
		}
	}

void CSystem::MarkImages (void)

//	MarkImages
//
//	Mark images in use

	{
	int i;

	for (i = 0; i < GetObjectCount(); i++)
		{
		CSpaceObject *pObj = GetObject(i);

		if (pObj)
			pObj->MarkImages();
		}
	}

void CSystem::NameObject (const CString &sName, CSpaceObject *pObj)

//	NameObject
//
//	Name an object

	{
	m_NamedObjects.AddEntry(sName, pObj);
	}

CVector CSystem::OnJumpPosAdj (CSpaceObject *pObj, const CVector &vPos)

//	OnJumpPosAdj
//
//	The object wants to jump to the given coordinates. This function will call
//	other objects in the system and see if the coords need to be adjusted

	{
	int i;
	int iTries = 20;
	CVector vNewPos = vPos;
	bool bSystemChecked = false;
	bool bObjsChecked = false;

	do
		{
		//	See if the system wants to change jump coordinates

		if (!bSystemChecked)
			{
			if (m_pType->FireOnObjJumpPosAdj(pObj, &vNewPos))
				bObjsChecked = false;

			bSystemChecked = true;
			}

		//	See if any objects want to change the jump coordinates

		if (!bObjsChecked)
			{
			int iEnd = GetObjectCount();

			do
				{
				int iNewEnd = -1;

				for (i = 0; i < iEnd; i++)
					{
					CSpaceObject *pAdjObj = GetObject(i);

					if (pAdjObj 
							&& pAdjObj != pObj 
							&& pAdjObj->OnObjJumpPosAdj(pObj, &vNewPos))
						{
						//	If we adjust the coordinates, then we start looping again
						//	but we end just before the object that changed the coords

						iNewEnd = i;

						//	We need to recheck the system

						bSystemChecked = false;
						}
					}

				iEnd = iNewEnd;
				}
			while (iTries-- > 0 && iEnd != -1);

			bObjsChecked = true;
			}
		}
	while (iTries-- > 0 && (!bSystemChecked || !bObjsChecked));

	return vNewPos;
	}

void CSystem::PaintDestinationMarker (CG16bitImage &Dest, int x, int y, CSpaceObject *pObj, CSpaceObject *pCenter)

//	PaintDestinationMarker
//
//	Paints a directional indicator

	{
	CVector vPos;

	//	Figure out the bearing for the destination object
	//	(We want the angle of the center with respect to the object because we
	//	start at the edge of the screen and point inward).

	int iBearing = VectorToPolar(pCenter->GetPos() - pObj->GetPos());

	//	Generate a set of points for the directional indicator

	SPoint Poly[5];

	//	Start at the origin

	Poly[0].x = 0;
	Poly[0].y = 0;

	//	Do one side first

	vPos = PolarToVector(iBearing + 30, ENHANCED_SRS_BLOCK_SIZE);
	Poly[1].x = (int)vPos.GetX();
	Poly[1].y = -(int)vPos.GetY();

	vPos = vPos + PolarToVector(iBearing, 3 * ENHANCED_SRS_BLOCK_SIZE);
	Poly[2].x = (int)vPos.GetX();
	Poly[2].y = -(int)vPos.GetY();

	//	The other side

	vPos = PolarToVector(iBearing + 330, ENHANCED_SRS_BLOCK_SIZE);
	CVector vPos2 = vPos + PolarToVector(iBearing, 3 * ENHANCED_SRS_BLOCK_SIZE);

	Poly[3].x = (int)vPos2.GetX();
	Poly[3].y = -(int)vPos2.GetY();

	Poly[4].x = (int)vPos.GetX();
	Poly[4].y = -(int)vPos.GetY();

	//	Paint the directional indicator

	CG16bitRegion Region;
	Region.CreateFromConvexPolygon(5, Poly);
	WORD wColor = pObj->GetSymbolColor();
	Region.Fill(Dest, x, y, wColor);

	//	Paint the text

	CG16bitFont &Font = g_pUniverse->GetSignFont();
	vPos = PolarToVector(iBearing, 5 * ENHANCED_SRS_BLOCK_SIZE);
	int xText = x + (int)vPos.GetX();
	int yText = y - (int)vPos.GetY();

	if (iBearing > 180)
		yText += 2 * ENHANCED_SRS_BLOCK_SIZE;
	else
		yText -= (2 * ENHANCED_SRS_BLOCK_SIZE) + Font.GetHeight();

	CString sName;
	if (pObj->IsIdentified())
		sName = pObj->GetNounPhrase(0);
	else if (pCenter->IsEnemy(pObj))
		sName = CONSTLIT("Unknown Hostile");
	else
		sName = CONSTLIT("Unknown Friendly");

	int cxText = Font.MeasureText(sName);

	//	Center text

	const RECT &rcClip = Dest.GetClipRect();
	xText = xText - cxText / 2;
	xText = Max((int)rcClip.left, xText);
	xText = Min((int)(rcClip.right - cxText), xText);

	//	Paint

	Font.DrawText(Dest, xText, yText, wColor, sName);

	//	If necessary, draw distance and bearing

	if (pObj->IsShowingDistanceAndBearing())
		{
		Metric rDist = (pObj->GetPos() - pCenter->GetPos()).Length();
		CString sText = strPatternSubst(CONSTLIT("Distance: %d"), (int)(rDist / LIGHT_SECOND));

		cxText = Font.MeasureText(sText);
		xText = x + (int)vPos.GetX() - (cxText / 2);
		xText = Max((int)rcClip.left, xText);
		xText = Min((int)(rcClip.right - cxText), xText);

		Font.DrawText(Dest, xText, yText + Font.GetHeight(), wColor, sText);
		}
	}

void CSystem::PaintStarField(CG16bitImage &Dest, const RECT &rcView, CSpaceObject *pCenter, Metric rKlicksPerPixel, COLORREF rgbSpaceColor)

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

	WORD wMaxColor = (WORD)(Max(Max(GetRValue(rgbSpaceColor), GetGValue(rgbSpaceColor)), GetBValue(rgbSpaceColor)));
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
							 bool bEnhanced)

//	PaintViewport
//
//	Paints the system in the viewport

	{
	int i;
	int iLayer;

	//	Figure out what color space should be. Space gets lighter as we get
	//	near the central star

	COLORREF rgbSpaceColor = CalculateSpaceColor(pCenter);

	//	Clear the rect

	Dest.SetClipRect(rcView);
	Dest.FillRGB(rcView.left, rcView.top, RectWidth(rcView), RectHeight(rcView), rgbSpaceColor);

	//	Paint the star field

	PaintStarField(Dest, rcView, pCenter, g_KlicksPerPixel, rgbSpaceColor);

	//	Figure out the boundary of the viewport in system coordinates. 

	CVector vDiagonal(g_KlicksPerPixel * (Metric)(RectWidth(rcView)) / 2,
				g_KlicksPerPixel * (Metric)(RectHeight(rcView)) / 2);
	CVector vUR = pCenter->GetPos() + vDiagonal;
	CVector vLL = pCenter->GetPos() - vDiagonal;

	//	Figure out the extended boundaries. This is used for enhanced display.

	CVector vEnhancedDiagonal(g_KlicksPerPixel * (Metric)(2 * RectWidth(rcView)),
			g_KlicksPerPixel * (Metric)(2 * RectHeight(rcView)));

	CVector vEnhancedUR = pCenter->GetPos() + vEnhancedDiagonal;
	CVector vEnhancedLL = pCenter->GetPos() - vEnhancedDiagonal;

	//	Compose the paint context

	SViewportPaintCtx Ctx;
	Ctx.wSpaceColor = CG16bitImage::RGBValue(GetRValue(rgbSpaceColor),
			GetGValue(rgbSpaceColor),
			GetBValue(rgbSpaceColor));

	//	Compute the transformation to map world coordinates to the viewport

	int xCenter = rcView.left + RectWidth(rcView) / 2;
	int yCenter = rcView.top + RectHeight(rcView) / 2;
	Ctx.XForm = ViewportTransform(pCenter->GetPos(), g_KlicksPerPixel, xCenter, yCenter);

	//	Compute the bounds relative to the center

	RECT rcBounds;
	rcBounds.left = rcView.left - xCenter + (ENHANCED_SRS_BLOCK_SIZE / 2);
	rcBounds.top = rcView.top - yCenter + (ENHANCED_SRS_BLOCK_SIZE / 2);
	rcBounds.right = rcView.right - xCenter - (ENHANCED_SRS_BLOCK_SIZE / 2);
	rcBounds.bottom = rcView.bottom - yCenter - (ENHANCED_SRS_BLOCK_SIZE / 2);

	//	Generate lists of all objects to paint by layer

	for (iLayer = layerSpace; iLayer < layerCount; iLayer++)
		m_LayerObjs[iLayer].RemoveAll();

	m_BackgroundObjs.RemoveAll();
	m_ForegroundObjs.RemoveAll();
	m_EnhancedDisplayObjs.RemoveAll();

	for (i = 0; i < GetObjectCount(); i++)
		{
		CSpaceObject *pObj = GetObject(i);
		if (pObj)
			{
			Metric rParallaxDist;

			if (pObj->IsBackgroundObj()
					&& ((rParallaxDist = pObj->GetParallaxDist()) != 1.0))
				{
				//	Compute the size of the viewport at the given object's distance

				CVector vParallaxDiag = vDiagonal * rParallaxDist;
				CVector vParallaxUR = pCenter->GetPos() + vParallaxDiag;
				CVector vParallaxLL = pCenter->GetPos() - vParallaxDiag;

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
			else if (pObj->InBox(vUR, vLL) && !pObj->IsVirtual())
				{
				m_LayerObjs[pObj->GetPaintLayer()].FastAdd(pObj);
				pObj->SetPaintNeeded();
				}
			else if ((bEnhanced
							&& (pObj->GetScale() == scaleShip || pObj->GetScale() == scaleStructure)
							&& pObj->PosInBox(vEnhancedUR, vEnhancedLL)
							&& !pObj->IsInactive()
							&& !pObj->IsVirtual())
						|| pObj->IsPlayerTarget()
						|| pObj->IsPlayerDestination())
				m_EnhancedDisplayObjs.FastAdd(pObj);
			}
		}

	//	Paint background objects

	for (i = 0; i < m_BackgroundObjs.GetCount(); i++)
		{
		CSpaceObject *pObj = m_BackgroundObjs.GetObj(i);

		//	Compute the transform

		SViewportPaintCtx Ctx;
		Ctx.wSpaceColor = Ctx.wSpaceColor;
		Ctx.XForm = ViewportTransform(pCenter->GetPos(), pObj->GetParallaxDist() * g_KlicksPerPixel, xCenter, yCenter);

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

	//	Paint any space environment

	if (m_pEnvironment)
		{
		int x, y, x1, y1, x2, y2;

		VectorToTile(vUR, &x2, &y1);
		VectorToTile(vLL, &x1, &y2);
		
		//	Increase bounds (so we can paint the edges)

		x1--; y1--;
		x2++; y2++;

		for (x = x1; x <= x2; x++)
			for (y = y1; y <= y2; y++)
				{
				CSpaceEnvironmentType *pEnv = (CSpaceEnvironmentType *)m_pEnvironment->GetTile(x, y);
				if (pEnv)
					{
					int xCenter, yCenter;
					CVector vCenter = TileToVector(x, y);
					Ctx.XForm.Transform(vCenter, &xCenter, &yCenter);

					pEnv->Paint(Dest, xCenter, yCenter);
					}
				}
		}

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

				Ctx.pObj = pObj;
				pObj->Paint(Dest, 
						x,
						y,
						Ctx);
				}

			//	Clear destination, if necessary

			if (pObj->IsAutoClearDestination())
				{
				pObj->ClearPlayerDestination();
				pObj->ClearShowDistanceAndBearing();
				pObj->ClearAutoClearDestination();
				}
			}

	//	Paint foreground objects

	for (i = 0; i < m_ForegroundObjs.GetCount(); i++)
		{
		CSpaceObject *pObj = m_ForegroundObjs.GetObj(i);

		//	Compute the transform

		SViewportPaintCtx Ctx;
		Ctx.wSpaceColor = Ctx.wSpaceColor;
		Ctx.XForm = ViewportTransform(pCenter->GetPos(), pObj->GetParallaxDist() * g_KlicksPerPixel, xCenter, yCenter);

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

	//	Paint all the enhanced display markers

	for (i = 0; i < m_EnhancedDisplayObjs.GetCount(); i++)
		{
		CSpaceObject *pObj = m_EnhancedDisplayObjs.GetObj(i);

		//	Figure out the position of the object in pixels
		//	relative to the center of the screen

		int x, y;
		Ctx.XForm.Transform(pObj->GetPos(), &x, &y);
		x = x - xCenter;
		y = y - yCenter;

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

		if (pObj->IsPlayerTarget() || pObj->IsPlayerDestination())
			{
			PaintDestinationMarker(Dest, 
					xCenter + x, 
					yCenter + y,
					pObj,
					pCenter);
			}
		else
			{
			WORD wColor = pObj->GetSymbolColor();

			Dest.Fill(xCenter + x - (ENHANCED_SRS_BLOCK_SIZE / 2), 
					yCenter + y - (ENHANCED_SRS_BLOCK_SIZE / 2),
					ENHANCED_SRS_BLOCK_SIZE, 
					ENHANCED_SRS_BLOCK_SIZE, 
					wColor);
			}
		}

	Dest.ResetClipRect();
	}

void CSystem::PaintViewportGrid (CG16bitImage &Dest, const RECT &rcView, const ViewportTransform &Trans, const CVector &vCenter, Metric rGridSize)

//	PaintViewportGrid
//
//	Paints a grid

	{
	int cxWidth = RectWidth(rcView);
	int cyHeight = RectHeight(rcView);

	//	Figure out where the center is

	int xCenter, yCenter;
	Trans.Transform(vCenter, &xCenter, &yCenter);

	//	Figure out the grid spacing

	int xSpacing, ySpacing;
	Trans.Transform(vCenter + CVector(rGridSize, -rGridSize), &xSpacing, &ySpacing);
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
	COLORREF rgbSpaceColor = CalculateSpaceColor(pCenter);
	Ctx.wSpaceColor = CG16bitImage::RGBValue(GetRValue(rgbSpaceColor),
			GetGValue(rgbSpaceColor),
			GetBValue(rgbSpaceColor));
	Ctx.XForm = ViewportTransform(pCenter->GetPos(), g_KlicksPerPixel, xCenter, yCenter);

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

	CVector vDiagonal(rKlicksPerPixel * (Metric)(RectWidth(rcView)) / 2,
				rKlicksPerPixel * (Metric)(RectHeight(rcView)) / 2);
	for (i = 0; i < RANGE_INDEX_COUNT; i++)
		{
		Metric rRange = RangeIndex2Range(i);

		//	Player ship can't see beyond LRS range

		if (rRange > g_LRSRange)
			rRange = g_LRSRange;

		CVector vRange(rRange, rRange);
		vUR[i] = pCenter->GetPos() + vRange;
		vLL[i] = pCenter->GetPos() - vRange;
		}

	int iPerception = pCenter->GetPerception();

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
				CSpaceEnvironmentType *pEnv = (CSpaceEnvironmentType *)m_pEnvironment->GetTile(x, y);
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

	bool bNewEnemies = false;
	for (i = 0; i < GetObjectCount(); i++)
		{
		CSpaceObject *pObj = GetObject(i);

		if (pObj)
			{
			int iRange = pObj->GetDetectionRangeIndex(iPerception);

			if (pObj->InBox(vUR[iRange], vLL[iRange]) 
					&& !pObj->IsInactive() 
					&& !pObj->IsVirtual())
				{
				//	Figure out the position of the object in pixels

				int x, y;
				Trans.Transform(pObj->GetPos(), &x, &y);

				//	Paint the object in the viewport

				pObj->PaintLRS(Dest, x, y, Trans);

				//	This object is now in the LRS

				bool bNewInLRS = pObj->SetPOVLRS();
				if (bNewInLRS 
						&& pCenter->IsEnemy(pObj) 
						&& pObj->GetCategory() == CSpaceObject::catShip)
					bNewEnemies = true;
				}
			else
				{
				//	This object is not in the LRS

				pObj->ClearPOVLRS();
				}
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

	//	Clear the rect

	Dest.FillRGB(rcView.left, rcView.top, RectWidth(rcView), RectHeight(rcView), g_rgbSpaceColor);

	//	Figure out the boundary of the viewport in system coordinates

	int xCenter = rcView.left + RectWidth(rcView) / 2;
	int yCenter = rcView.top + RectHeight(rcView) / 2;

	CVector vDiagonal(rMapScale * (Metric)(RectWidth(rcView) + 256) / 2,
				rMapScale * (Metric)(RectHeight(rcView) + 256) / 2);
	CVector vUR = pCenter->GetPos() + vDiagonal;
	CVector vLL = pCenter->GetPos() - vDiagonal;

	//	Compute the transformation to map world coordinates to the viewport

	ViewportTransform Trans(pCenter->GetPos(), 
			rMapScale, 
			rMapScale * MAP_VERTICAL_ADJUST,
			xCenter, 
			yCenter);

	//	Paint the grid

	PaintViewportGrid(Dest, rcView, Trans, CVector(), 100.0 * LIGHT_SECOND);

	//	Paint space environment

	if (m_pEnvironment)
		{
		STileMapEnumerator k;
		while (m_pEnvironment->HasMore(k))
			{
			DWORD dwTile;
			int xTile;
			int yTile;

			m_pEnvironment->GetNext(k, &xTile, &yTile, &dwTile);
			CVector vPos = TileToVector(xTile, yTile);

			Trans.Transform(vPos, &x, &y);
			Dest.DrawPixel(x, y, CG16bitImage::RGBValue(255,0,255));
			}
		}

	//	Paint all planets and stars first

	for (i = 0; i < GetObjectCount(); i++)
		{
		CSpaceObject *pObj = GetObject(i);

		if (pObj 
				&& (pObj->GetScale() == scaleStar || pObj->GetScale() == scaleWorld)
				&& (pObj->GetMapOrbit() || pObj->InBox(vUR, vLL)))
			{
			//	Figure out the position of the object in pixels

			Trans.Transform(pObj->GetPos(), &x, &y);

			//	Paint the object in the viewport

			pObj->PaintMap(Dest, 
					x,
					y,
					Trans);
			}
		}

	//	Paint all structures next

	for (i = 0; i < GetObjectCount(); i++)
		{
		CSpaceObject *pObj = GetObject(i);

		if (pObj 
				&& pObj->InBox(vUR, vLL) 
				&& (pObj->GetScale() == scaleStructure
					|| pObj->GetScale() == scaleShip))
			{
			//	Figure out the position of the object in pixels

			Trans.Transform(pObj->GetPos(), &x, &y);

			//	Paint the object in the viewport

			pObj->PaintMap(Dest, 
					x,
					y,
					Trans);

			//	Paint destination marker

			if (pObj->IsPlayerDestination())
				{
				Dest.FillColumn(x, y - 4, 9, CG16bitImage::RGBValue(255, 255, 0));
				Dest.FillLine(x - 4, y, 9, CG16bitImage::RGBValue(255, 255, 0));
				}
			}
		}

	//	Paint the POV

	Trans.Transform(pCenter->GetPos(), &x, &y);
	pCenter->PaintMap(Dest, x, y, Trans);
	}

void CSystem::PaintViewportMapObject (CG16bitImage &Dest, const RECT &rcView, CSpaceObject *pCenter, CSpaceObject *pObj)

//	PaintViewportMapObject
//
//	Paints a system map object

	{
	int x, y;

	//	Figure out the boundary of the viewport in system coordinates

	int xCenter = rcView.left + RectWidth(rcView) / 2;
	int yCenter = rcView.top + RectHeight(rcView) / 2;

	CVector vDiagonal(g_MapKlicksPerPixel * (Metric)(RectWidth(rcView) + 256) / 2,
				g_MapKlicksPerPixel * (Metric)(RectHeight(rcView) + 256) / 2);
	CVector vUR = pCenter->GetPos() + vDiagonal;
	CVector vLL = pCenter->GetPos() - vDiagonal;

	//	Compute the transformation to map world coordinates to the viewport

	ViewportTransform Trans(pCenter->GetPos(), 
			g_MapKlicksPerPixel, 
			g_MapKlicksPerPixel * MAP_VERTICAL_ADJUST,
			xCenter, 
			yCenter);

	//	Paint the obj

	Trans.Transform(pObj->GetPos(), &x, &y);
	pObj->PaintMap(Dest, x, y, Trans);
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

	//	We keep on incrementing the timer as long as we are creating ships
	//	in the same tick.

	if (m_iTick != g_iGateTimerTick)
		{
		g_iGateTimer = 0;
		g_iGateTimerTick = m_iTick;
		}

	pShip->Place(pGate->GetPos(), pGate->GetVel());
	pShip->SetInGate(pGate, g_iGateTimer);
	g_iGateTimer += mathRandom(11, 22);
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

	//	If we could not find it, add the return pointer to a map so that
	//	we can fill it in later (when we load the actual object)

	CIntArray *pList;
	if (Ctx.ForwardReferences.Lookup(dwID, (CObject **)&pList) != NOERROR)
		{
		pList = new CIntArray;
		Ctx.ForwardReferences.AddEntry(dwID, pList);
		}

	pList->AppendElement((int)retpObj, NULL);
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

	if (Ctx.pObj->NotifyOthersWhenDestroyed())
		{
		DEBUG_SAVE_PROGRAMSTATE;

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

		if (Ctx.pObj != g_pUniverse->GetPlayer() && g_pUniverse->GetPlayer())
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

	//	Debug code to see if we ever delete a barrier in the middle of move

#ifdef DEBUG_PROGRAMSTATE
	if (g_iProgramState == psUpdatingMove)
		{
		if (Ctx.pObj->IsBarrier())
			{
			CString sObj = CONSTLIT("ERROR: Destroying barrier during move.\r\n");

			ReportCrashObj(&sObj, Ctx.pObj);
			kernelDebugLogMessage(sObj.GetASCIIZPointer());

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
	dwSave |= (m_fNoRandomEncounters ? 0x00000001 : 0);
	dwSave |= 0x00000002;	//	Include version (this is a hack for backwards compatibility)
	dwSave |= (!m_fUseDefaultTerritories ? 0x00000004 : 0);
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
				kernelDebugLogMessage(sError.GetASCIIZPointer());
				return ERR_FAIL;
				}
			}
		}

	//	Save all named objects

	dwCount = m_NamedObjects.GetCount();
	pStream->Write((char *)&dwCount, sizeof(DWORD));

	for (i = 0; i < (int)dwCount; i++)
		{
		CSpaceObject *pObj = (CSpaceObject *)m_NamedObjects.GetValue(i);
		m_NamedObjects.GetKey(i).WriteToStream(pStream);
		WriteObjRefToStream(pObj, pStream);
		}

	//	Save timed events

	dwCount = m_TimedEvents.GetCount();
	pStream->Write((char *)&dwCount, sizeof(DWORD));

	for (i = 0; i < (int)dwCount; i++)
		{
		CTimedEvent *pEvent = GetTimedEvent(i);
		pEvent->WriteToStream(this, pStream);
		}

	//	Save environment maps

	dwCount = (m_pEnvironment ? 1 : 0);
	pStream->Write((char *)&dwCount, sizeof(DWORD));
	if (m_pEnvironment)
		{
		CTileMap *pEnv;
		ConvertSpaceEnvironmentToUNIDs(*m_pEnvironment, &pEnv);
		pEnv->WriteToStream(pStream);
		delete pEnv;
		}

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
	if (m_pEnvironment == NULL)
		m_pEnvironment = new CTileMap(seaArraySize, seaScale);

	m_pEnvironment->SetTile(xTile, yTile, (DWORD)pEnvironment);
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

void CSystem::Update (Metric rSecondsPerTick, bool bForceEventFiring)

//	Update
//
//	Updates the system

	{
	int i;
#ifdef DEBUG_PERFORMANCE
	int iUpdateObj = 0;
	int iMoveObj = 0;
#endif

	//	Delete all objects in the deleted list (we do this at the
	//	beginning because we want to keep the list after the update
	//	so that callers can examine it).

	for (i = 0; i < m_DeletedObjects.GetCount(); i++)
		delete m_DeletedObjects.GetObj(i);
	m_DeletedObjects.RemoveAll();

	//	Fire timed events
	//	NOTE: We only do this if we have a player because otherwise, some
	//	of the scripts might crash. We won't have a player when we first
	//	create the universe.

	SetProgramState(psUpdatingEvents);
	if (!IsTimeStopped() && (g_pUniverse->GetPlayer() || bForceEventFiring))
		{
		for (i = 0; i < GetTimedEventCount(); i++)
			{
			CTimedEvent *pEvent = GetTimedEvent(i);
			SetProgramEvent(pEvent);

			if (!pEvent->IsDestroyed() && pEvent->GetTick() <= m_iTick)
				pEvent->DoEvent(this);
			}

		//	Delete events that were destroyed

		for (i = 0; i < GetTimedEventCount(); i++)
			{
			CTimedEvent *pEvent = GetTimedEvent(i);
			if (pEvent->IsDestroyed())
				{
				m_TimedEvents.RemoveEvent(i);
				i--;
				}
			}
		}

	SetProgramEvent(NULL);

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

	//	Give all objects a chance to react

	DebugStartTimer();
	for (i = 0; i < GetObjectCount(); i++)
		{
		CSpaceObject *pObj = GetObject(i);

		if (pObj && !pObj->IsTimeStopped())
			{
			SetProgramState(psUpdatingBehavior, pObj);
			pObj->Behavior();

			//	Update the objects

			SetProgramState(psUpdatingObj, pObj);
			pObj->Update();

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

	m_BarrierObjects.RemoveAll();
	m_BarrierObjects.SetAllocSize(GetObjectCount());

	//	Make a list of all barrier objects

	for (i = 0; i < GetObjectCount(); i++)
		{
		CSpaceObject *pObj = GetObject(i);
		if (pObj)
			{
			if (pObj->IsBarrier() && !pObj->IsDestroyed())
				m_BarrierObjects.FastAdd(pObj);
			}
		}

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
			pObj->Move(m_BarrierObjects, rSecondsPerTick);

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

	//	Update for a few seconds

	int iTime = mathRandom(250, 350);
	for (i = 0; i < iTime; i++)
		Update(g_SecondsPerUpdate);

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

void CSystem::WriteObjRefToStream (CSpaceObject *pObj, IWriteStream *pStream, CSpaceObject *pReferrer)

//	WriteObjRefToStream
//
//	DWORD		0xffffffff if NULL
//				Otherwise, index of object in system

	{
	DWORD dwSave = OBJID_NULL;
	if (pObj)
		{
		//	Make sure we save references to objects in the current system.
		//	This will help to track a bug in gating objects.

		if (pObj->GetSystem() != this)
			{
			kernelDebugLogMessage("Save file error: Saving reference to object in another system");

			CString sError;
			ReportCrashObj(&sError, pObj);
			kernelDebugLogMessage("Object being referenced:");
			kernelDebugLogMessage(sError.GetASCIIZPointer());

			if (pReferrer)
				{
				ReportCrashObj(&sError, pReferrer);
				kernelDebugLogMessage("Referring object:");
				kernelDebugLogMessage(sError.GetASCIIZPointer());
				}
			}

		dwSave = pObj->GetID();
		ASSERT(dwSave != 0xDDDDDDDD);
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
