//	CreateSystem.cpp
//
//	CreateSystem class

#include "PreComp.h"
#include "math.h"

#ifdef DEBUG
//#define DEBUG_STATION_TABLE_CACHE
//#define DEBUG_STRESS_TEST
//#define DEBUG_STATION_TABLES
//#define DEBUG_STATION_PLACEMENT
//#define DEBUG_STATION_SEPARATION
#endif

#define ADD_ATTRIBUTE_TAG				CONSTLIT("AddAttribute")
#define ADD_TERRITORY_TAG				CONSTLIT("AddTerritory")
#define ANTI_TROJAN_TAG					CONSTLIT("AntiTrojan")
#define CODE_TAG						CONSTLIT("Code")
#define ESCORTS_TAG						CONSTLIT("Escorts")
#define EVENTS_TAG						CONSTLIT("Events")
#define FILL_LOCATIONS_TAG				CONSTLIT("FillLocations")
#define FILL_RANDOM_LOCATION_TAG		CONSTLIT("FillRandomLocation")
#define GROUP_TAG						CONSTLIT("Group")
#define INITIAL_DATA_TAG				CONSTLIT("InitialData")
#define ITEM_TAG						CONSTLIT("Item")
#define ITEMS_TAG						CONSTLIT("Items")
#define LABEL_TAG						CONSTLIT("Label")
#define LEVEL_TABLE_TAG					CONSTLIT("LevelTable")
#define LOCATION_CRITERIA_TABLE_TAG		CONSTLIT("LocationCriteriaTable")
#define LOOKUP_TAG						CONSTLIT("Lookup")
#define MARKER_TAG						CONSTLIT("Marker")
#define NULL_TAG						CONSTLIT("Null")
#define OFFSET_TAG						CONSTLIT("Offset")
#define ON_CREATE_TAG					CONSTLIT("OnCreate")
#define ORBITAL_DISTRIBUTION_TAG		CONSTLIT("OrbitalDistribution")
#define ORBITALS_TAG					CONSTLIT("Orbitals")
#define PARTICLES_TAG					CONSTLIT("Particles")
#define PLACE_RANDOM_STATION_TAG		CONSTLIT("PlaceRandomStation")
#define PRIMARY_TAG						CONSTLIT("Primary")
#define RANDOM_LOCATION_TAG				CONSTLIT("RandomLocation")
#define RANDOM_STATION_TAG				CONSTLIT("RandomStation")
#define SATELLITES_TAG					CONSTLIT("Satellites")
#define SHIP_TAG						CONSTLIT("Ship")
#define SHIPS_TAG						CONSTLIT("Ships")
#define SIBLINGS_TAG					CONSTLIT("Siblings")
#define SPACE_ENVIRONMENT_TAG			CONSTLIT("SpaceEnvironment")
#define STARGATE_TAG					CONSTLIT("Stargate")
#define STATION_TAG						CONSTLIT("Station")
#define SYSTEM_GROUP_TAG				CONSTLIT("SystemGroup")
#define TABLE_TAG						CONSTLIT("Table")
#define TABLES_TAG						CONSTLIT("Tables")
#define TROJAN_TAG						CONSTLIT("Trojan")
#define VARIANT_TAG						CONSTLIT("Variant")
#define VARIANTS_TAG					CONSTLIT("Variants")
#define VARIANTS_TABLE_TAG				CONSTLIT("VariantTable")

#define ANGLE_ATTRIB					CONSTLIT("angle")
#define ANGLE_ADJ_ATTRIB				CONSTLIT("angleAdj")
#define ANGLE_INC_ATTRIB				CONSTLIT("angleInc")
#define ARC_INC_ATTRIB					CONSTLIT("arcInc")
#define ARC_LENGTH_ATTRIB				CONSTLIT("arcLength")
#define ATTRIBUTES_ATTRIB				CONSTLIT("attributes")
#define BODE_DISTANCE_END_ATTRIB		CONSTLIT("BodeDistanceEnd")
#define BODE_DISTANCE_START_ATTRIB		CONSTLIT("BodeDistanceStart")
#define CHANCE_ATTRIB					CONSTLIT("chance")
#define CLASS_ATTRIB					CONSTLIT("class")
#define CONTROLLER_ATTRIB				CONSTLIT("controller")
#define COUNT_ATTRIB					CONSTLIT("count")
#define CRITERIA_ATTRIB					CONSTLIT("criteria")
#define DEBUG_ONLY_ATTRIB				CONSTLIT("debugOnly")
#define DISTANCE_ATTRIB					CONSTLIT("distance")
#define DISTRIBUTION_ATTRIB				CONSTLIT("distribution")
#define ECCENTRICITY_ATTRIB				CONSTLIT("eccentricity")
#define ENCOUNTERS_ATTRIB				CONSTLIT("encountersCount")
#define EXCLUSION_RADIUS_ATTRIB			CONSTLIT("exclusionRadius")
#define ID_ATTRIB						CONSTLIT("id")
#define IMAGE_VARIANT_ATTRIB			CONSTLIT("imageVariant")
#define INCLUDE_ALL_ATTRIB				CONSTLIT("includeAll")
#define INTERVAL_ATTRIB					CONSTLIT("interval")
#define LEVEL_FREQUENCY_ATTRIB			CONSTLIT("levelFrequency")
#define LOCATION_ATTRIBS_ATTRIB			CONSTLIT("locationAttribs")
#define LOCATION_CRITERIA_ATTRIB		CONSTLIT("locationCriteria")
#define MATCH_ATTRIB					CONSTLIT("match")
#define MAX_ATTRIB						CONSTLIT("max")
#define MAX_DIST_ATTRIB					CONSTLIT("maxDist")
#define MAX_RADIUS_ATTRIB				CONSTLIT("maxRadius")
#define MAX_SHIPS_ATTRIB				CONSTLIT("maxShips")
#define MIN_ATTRIB						CONSTLIT("min")
#define MIN_DIST_ATTRIB					CONSTLIT("minDist")
#define MIN_RADIUS_ATTRIB				CONSTLIT("minRadius")
#define NAME_ATTRIB						CONSTLIT("name")
#define NO_CONSTRUCTION_ATTRIB			CONSTLIT("noConstruction")
#define NO_MAP_LABEL_ATTRIB				CONSTLIT("noMapLabel")
#define NO_OVERLAP_ATTRIB				CONSTLIT("noOverlap")
#define NO_RANDOM_ENCOUNTERS_ATTRIB		CONSTLIT("noRandomEncounters")
#define NO_REINFORCEMENTS_ATTRIB		CONSTLIT("noReinforcements")
#define NO_SATELLITES_ATTRIB			CONSTLIT("noSatellites")
#define OBJ_NAME_ATTRIB					CONSTLIT("objName")
#define OFFSET_ATTRIB					CONSTLIT("offset")
#define ORDERS_ATTRIB					CONSTLIT("orders")
#define PATCHES_ATTRIB					CONSTLIT("patchType")
#define PATCH_FREQUENCY_ATTRIB			CONSTLIT("patchFrequency")
#define PERCENT_ENEMIES_ATTRIB			CONSTLIT("percentEnemies")
#define PERCENT_FULL_ATTRIB				CONSTLIT("percentFull")
#define PROBABILITY_ATTRIB				CONSTLIT("probability")
#define RADIAL_EDGE_WIDTH_ATTRIB		CONSTLIT("radialEdgeWidth")
#define RADIAL_WIDTH_ATTRIB				CONSTLIT("radialWidth")
#define RADIUS_ATTRIB					CONSTLIT("radius")
#define RADIUS_DEC_ATTRIB				CONSTLIT("radiusDec")
#define RADIUS_INC_ATTRIB				CONSTLIT("radiusInc")
#define ROTATION_ATTRIB					CONSTLIT("rotation")
#define SEPARATE_ENEMIES_ATTRIB			CONSTLIT("separateEnemies")
#define SHAPE_ATTRIB					CONSTLIT("shape")
#define SHOW_ORBIT_ATTRIB				CONSTLIT("showOrbit")
#define SOVEREIGN_ATTRIB				CONSTLIT("sovereign")
#define SPACE_SCALE_ATTRIB				CONSTLIT("spaceScale")
#define SPAN_ATTRIB						CONSTLIT("span")
#define STARGATE_ATTRIB					CONSTLIT("stargate")
#define STATION_CRITERIA_ATTRIB			CONSTLIT("stationCriteria")
#define TABLE_ATTRIB					CONSTLIT("table")
#define TIME_SCALE_ATTRIB				CONSTLIT("timeScale")
#define TYPE_ATTRIB						CONSTLIT("type")
#define VARIANT_ATTRIB					CONSTLIT("variant")
#define VARIANT_LOCATION_CRITERIA_ATTRIB	CONSTLIT("variantLocationCriteria")
#define WIDTH_ATTRIB					CONSTLIT("width")
#define WIDTH_VARIATION_ATTRIB			CONSTLIT("widthVariation")
#define WRECK_TYPE_ATTRIB				CONSTLIT("wreckType")
#define X_OFFSET_ATTRIB					CONSTLIT("xOffset")
#define Y_OFFSET_ATTRIB					CONSTLIT("yOffset")

#define SPECIAL_ATTRIB_INNER_SYSTEM		CONSTLIT("innerSystem")
#define SPECIAL_ATTRIB_OUTER_SYSTEM		CONSTLIT("outerSystem")
#define SPECIAL_ATTRIB_LIFE_ZONE		CONSTLIT("lifeZone")
#define SPECIAL_ATTRIB_NEAR_FRIENDS		CONSTLIT("nearFriends")
#define SPECIAL_ATTRIB_NEAR_ENEMIES		CONSTLIT("nearEnemies")
#define SPECIAL_ATTRIB_NEAR_STATIONS	CONSTLIT("nearStations")

#define RANDOM_ANGLE					CONSTLIT("random")
#define EQUIDISTANT_ANGLE				CONSTLIT("equidistant")
#define INCREMENTING_ANGLE				CONSTLIT("incrementing")
#define MIN_SEPARATION_ANGLE			CONSTLIT("minSeparation")

#define ORDER_TYPE_ATTACK				CONSTLIT("attack")
#define ORDER_TYPE_GUARD				CONSTLIT("guard")
#define ORDER_TYPE_GATE					CONSTLIT("gate")
#define ORDER_TYPE_PATROL				CONSTLIT("patrol")
#define ORDER_TYPE_ESCORT				CONSTLIT("escort")
#define ORDER_TYPE_SCAVENGE				CONSTLIT("scavenge")
#define ORDER_TYPE_TRADE_ROUTE			CONSTLIT("trade route")
#define ORDER_TYPE_WANDER				CONSTLIT("wander")
#define ORDER_TYPE_HOLD					CONSTLIT("hold")
#define ORDER_TYPE_MINE					CONSTLIT("mine")
#define ORDER_TYPE_FOLLOW				CONSTLIT("follow")

#define SHAPE_CIRCULAR					CONSTLIT("circular")
#define SHAPE_ARC						CONSTLIT("arc")

#define TYPE_NEBULA						CONSTLIT("nebula")

#define MATCH_ALL						CONSTLIT("*")
#define ENEMY_ATTRIBUTE					CONSTLIT("enemy")
#define FRIEND_ATTRIBUTE				CONSTLIT("friendly")
#define REQUIRE_ENEMY					CONSTLIT("*enemy")
#define REQUIRE_FRIEND					CONSTLIT("*friendly")

#define ON_CREATE_EVENT					CONSTLIT("OnCreate")

//	Minimum distance that two enemy stations can be (in light-seconds)

#define MIN_ENEMY_DIST					30
#define MAX_NEBULAE						100000
#define OVERLAP_DIST					(25.0 * LIGHT_SECOND)

static char g_ProbabilitiesAttrib[] = "probabilities";

//	Debugging Support

#ifdef DEBUG_STRESS_TEST

#define STRESS_ITERATIONS		50

#ifdef DEBUG_STRING_LEAKS
#define START_STRING_LEAK_TEST		CString::DebugMark();
#define STOP_STRING_LEAK_TEST		CString::DebugOutputLeakedStrings();	\
									char szBuffer[1024];					\
									wsprintf(szBuffer, "Total Strings: %d\n", CString::DebugGetStringCount());	\
									::OutputDebugString(szBuffer);
#else
#define START_STRING_LEAK_TEST
#define STOP_STRING_LEAK_TEST
#endif

#define START_STRESS_TEST					\
	for (int k = 0; k < STRESS_ITERATIONS; k++)	\
		{									\
		START_STRING_LEAK_TEST

#define STOP_STRESS_TEST					\
		if (k < (STRESS_ITERATIONS-1))		\
			{								\
			delete pSystem;					\
			STOP_STRING_LEAK_TEST			\
			}								\
		}

#else

#define START_STRESS_TEST
#define STOP_STRESS_TEST

#endif

#ifdef DEBUG_STATION_PLACEMENT
#define STATION_PLACEMENT_OUTPUT(x)		::OutputDebugString(x)
#else
#define STATION_PLACEMENT_OUTPUT(x)
#endif

//	Classes and structures

ALERROR AddAttribute (SSystemCreateCtx *pCtx, CXMLElement *pObj, const COrbit &OrbitDesc);
ALERROR AddTerritory (SSystemCreateCtx *pCtx, CXMLElement *pObj, const COrbit &OrbitDesc);
bool CheckForOverlap (SSystemCreateCtx *pCtx, const CVector &vPos);
ALERROR ChooseRandomLocation (SSystemCreateCtx *pCtx, 
							  const SLocationCriteria &Criteria, 
							  const COrbit &OrbitDesc,
							  CStationType *pStationToPlace,
							  COrbit *retOrbitDesc, 
							  CString *retsAttribs,
							  int *retiLabelPos);
ALERROR ChooseRandomLocation (SSystemCreateCtx *pCtx, 
							  const CString &sCriteria, 
							  CStationType *pStationToPlace,
							  COrbit *retOrbitDesc, 
							  CString *retsAttribs,
							  int *retiLabelPos);
int ComputeLocationWeight (SSystemCreateCtx *pCtx, 
						   const CString &sLocationAttribs,
						   const CVector &vPos,
						   const CString &sAttrib, 
						   DWORD dwMatchStrength);
const COrbit *ComputeOffsetOrbit (CXMLElement *pObj, const COrbit &Original, COrbit *retOrbit);
int ComputeStationWeight (SSystemCreateCtx *pCtx, CStationType *pType, const CString &sAttrib, DWORD dwMatchStrength);
ALERROR CreateAppropriateStationAtRandomLocation (SSystemCreateCtx *pCtx, 
												  TProbabilityTable<int> &LocationTable,
												  const CString &sStationCriteria,
												  bool bSeparateEnemies,
												  bool *retbEnemy = NULL);
ALERROR CreateArcDistribution (SSystemCreateCtx *pCtx, CXMLElement *pObj, const COrbit &OrbitDesc);
ALERROR CreateLabel (SSystemCreateCtx *pCtx,
					 CXMLElement *pObj,
					 const COrbit &OrbitDesc);
ALERROR CreateLevelTable (SSystemCreateCtx *pCtx, CXMLElement *pDesc, const COrbit &OrbitDesc);
ALERROR CreateLocationCriteriaTable (SSystemCreateCtx *pCtx, CXMLElement *pDesc, const COrbit &OrbitDesc);
ALERROR CreateObjectAtRandomLocation (SSystemCreateCtx *pCtx, CXMLElement *pDesc, const COrbit &OrbitDesc);
ALERROR CreateOffsetObjects (SSystemCreateCtx *pCtx, CXMLElement *pObj, const COrbit &OrbitDesc);
ALERROR CreateOrbitals (SSystemCreateCtx *pCtx, 
						CXMLElement *pObj, 
						const COrbit &OrbitDesc);
ALERROR CreateRandomStation (SSystemCreateCtx *pCtx, CXMLElement *pDesc, const COrbit &OrbitDesc);
ALERROR CreateRandomStationAtAppropriateLocation (SSystemCreateCtx *pCtx, CXMLElement *pDesc);
ALERROR CreateShipsForStation (CSpaceObject *pStation, CXMLElement *pShips);
ALERROR CreateSiblings (SSystemCreateCtx *pCtx, 
						CXMLElement *pObj, 
						const COrbit &OrbitDesc);
ALERROR CreateSpaceEnvironment (SSystemCreateCtx *pCtx, CXMLElement *pDesc, const COrbit &OrbitDesc);
void CreateSpaceEnvironmentTile (SSystemCreateCtx *pCtx,
								 const CVector &vPos,
								 int xTile,
								 int yTile,
								 CSpaceEnvironmentType *pEnvironment,
								 CEffectCreator *pPatch,
								 int iPatchFrequency);
ALERROR CreateStargate (SSystemCreateCtx *pCtx, CXMLElement *pDesc, const COrbit &OrbitDesc);
ALERROR CreateStationFromElement (SSystemCreateCtx *pCtx,
								  CXMLElement *pDesc,
								  const COrbit &OrbitDesc,
								  CStation **retpStation = NULL);
ALERROR CreateSystemObject (SSystemCreateCtx *pCtx, 
							CXMLElement *pObj, 
							const COrbit &OrbitDesc,
							bool bIgnoreChance = false);
ALERROR CreateVariantsTable (SSystemCreateCtx *pCtx, CXMLElement *pDesc, const COrbit &OrbitDesc);
ALERROR GenerateAngles (SSystemCreateCtx *pCtx, const CString &sAngle, int iCount, Metric *pAngles);
void DumpDebugStack (SSystemCreateCtx *pCtx);
void GenerateRandomPosition (SSystemCreateCtx *pCtx, COrbit *retOrbit);
ALERROR GenerateRandomStationTable (SSystemCreateCtx *pCtx,
									const CString &sCriteria,
									const CString &sLocationAttribs,
									const CVector &vPos,
									bool bIncludeAll,
									TArray<CStationTableCache::SEntry> **retpTable,
									bool *retbAddToCache);
ALERROR GetLocationCriteria (SSystemCreateCtx *pCtx, CXMLElement *pDesc, SLocationCriteria *retCriteria);
bool IsExclusionZoneClear (SSystemCreateCtx *pCtx, const CVector &vPos, Metric rRadius);
bool IsExclusionZoneClear (SSystemCreateCtx *pCtx, const CVector &vPos, CStationType *pType);
ALERROR ModifyCreatedStation (SSystemCreateCtx *pCtx, CStation *pStation, CXMLElement *pDesc, const COrbit &OrbitDesc);
inline void PopDebugStack (SSystemCreateCtx *pCtx) { if (g_pUniverse->InDebugMode()) pCtx->DebugStack.Pop(); }
inline void PushDebugStack (SSystemCreateCtx *pCtx, const CString &sLine) { if (g_pUniverse->InDebugMode()) pCtx->DebugStack.Push(sLine); }
void RemoveOverlappingLabels (SSystemCreateCtx *pCtx, Metric rMinDistance);

//	Helper functions

ALERROR AddAttribute (SSystemCreateCtx *pCtx, CXMLElement *pObj, const COrbit &OrbitDesc)

//	AddAttribute
//
//	Adds one or more attributes to the system/node

	{
	//	Define the attributes

	CString sAttribs;
	if (pObj->FindAttribute(ATTRIBUTES_ATTRIB, &sAttribs))
		pCtx->pTopologyNode->AddAttributes(sAttribs);

	return NOERROR;
	}

ALERROR AddTerritory (SSystemCreateCtx *pCtx, CXMLElement *pObj, const COrbit &OrbitDesc)

//	AddTerritory
//
//	Creates a simple territory

	{
	ALERROR error;

	//	Load the territory

	CTerritoryDef *pTerritory;
	if (error = CTerritoryDef::CreateFromXML(pObj, OrbitDesc, &pTerritory))
		return error;

	//	Add to system

	if (error = pCtx->pSystem->AddTerritory(pTerritory))
		return error;

	return NOERROR;
	}

ALERROR ChooseRandomLocation (SSystemCreateCtx *pCtx, 
							  const SLocationCriteria &Criteria, 
							  const COrbit &OrbitDesc,
							  CStationType *pStationToPlace,
							  COrbit *retOrbitDesc, 
							  CString *retsAttribs,
							  int *retiLabelPos)

//	ChooseRandomLocation
//
//	Returns the orbital position for a random label that
//	matches the desired characteristics in sCriteria. If ERR_NOTFOUND
//	is returned then it means that a label of that characteristic could
//	not be found.
//
//	If pStationToPlace is specified then we make sure that we don't pick
//	a location near enemies of the station.
//
//	If retiLabelPos is passed-in then we do not automatically remove the label
//	from the list.

	{
	STATION_PLACEMENT_OUTPUT("ChooseRandomLocation\n");

	//	Choose a random location

	int iLocID;
	if (!pCtx->pSystem->FindRandomLocation(Criteria, 0, OrbitDesc, pStationToPlace, &iLocID))
		return ERR_NOTFOUND;

	//	Return info

	if (retOrbitDesc || retsAttribs)
		{
		CLocationDef *pLoc = pCtx->pSystem->GetLocation(iLocID);
		if (retOrbitDesc)
			*retOrbitDesc = pLoc->GetOrbit();

		if (retsAttribs)
			*retsAttribs = pLoc->GetAttributes();
		}

	if (retiLabelPos)
		*retiLabelPos = iLocID;

#ifdef DEBUG_STATION_PLACEMENT
	{
	char szBuffer[1024];
	wsprintf(szBuffer, "   found random location (%s)\n", retsAttribs->GetASCIIZPointer());
	::OutputDebugString(szBuffer);
	}
#endif

	return NOERROR;
	}

ALERROR ChooseRandomLocation (SSystemCreateCtx *pCtx, 
							  const CString &sCriteria, 
							  CStationType *pStationToPlace,
							  COrbit *retOrbitDesc, 
							  CString *retsAttribs,
							  int *retiLabelPos)

//	ChooseRandomLocation
//
//	Returns the orbital position for a random label that
//	matches the desired characteristics in sCriteria. If ERR_NOTFOUND
//	is returned then it means that a label of that characteristic could
//	not be found.
//
//	If pStationToPlace is specified then we make sure that we don't pick
//	a location near enemies of the station.
//
//	If retiLabelPos is passed-in then we do not automatically remove the label
//	from the list.

	{
	ALERROR error;

	//	Parse the criteria

	SLocationCriteria Criteria;
	if (error = Criteria.AttribCriteria.Parse(sCriteria, 0, &pCtx->sError))
		return error;

	//	Choose a random location

	return ChooseRandomLocation(pCtx, Criteria, COrbit(), pStationToPlace, retOrbitDesc, retsAttribs, retiLabelPos);
	}

ALERROR ChooseRandomStation (SSystemCreateCtx *pCtx, 
							 const CString &sCriteria, 
							 const CString &sLocationAttribs,
							 const CVector &vPos,
							 bool bSeparateEnemies,
							 bool bIncludeAll,
							 CStationType **retpType)

//	ChooseRandomStation
//
//	Picks a random station to create. The station is appropriate to the level
//	of the system and to the given criteria.

	{
	ALERROR error;
	int i;

	//	Generate a description of the table that we are about to generate
	//	to see if we've already got this table in the cache.

	CString sTableDesc = strPatternSubst(CONSTLIT("%s|||%s|||%s|||%s"),
			sCriteria,
			sLocationAttribs,
			pCtx->pSystem->GetAttribsAtPos(vPos),
			(bIncludeAll ? CONSTLIT("ALL") : NULL_STR));

	//	If this table is not cached then generate it.

	bool bFreeTable = false;
	TArray<CStationTableCache::SEntry> *pTable;
	if (!pCtx->StationTables.FindTable(sTableDesc, &pTable))
		{
		bool bAddToCache;
		if (error = GenerateRandomStationTable(pCtx, sCriteria, sLocationAttribs, vPos, bIncludeAll, &pTable, &bAddToCache))
			return error;

		if (bAddToCache)
			pCtx->StationTables.AddTable(sTableDesc, pTable);
		else
			bFreeTable = true;
		}

	//	Add to stats

	if (pCtx->pStats)
		pCtx->pStats->AddStationTable(pCtx->pSystem, sCriteria, ::AppendModifiers(sLocationAttribs, pCtx->pSystem->GetAttribsAtPos(vPos)), *pTable);

	//	Now generate a probability table and add all the entries

	TProbabilityTable<CStationType *> Table;
	for (i = 0; i < pTable->GetCount(); i++)
		{
		const CStationTableCache::SEntry &Entry = pTable->GetAt(i);

		//	Make sure we can still encounter this type. [If we've already created
		//	a system-unique object then it will still be in the cached tables.]

		if (!Entry.pType->CanBeEncountered(pCtx->pSystem))
			continue;

		//	If we want to separate enemies, then see if there are any
		//	enemies of this station type at this location.

		if (bSeparateEnemies && !IsExclusionZoneClear(pCtx, vPos, Entry.pType))
			continue;

		//	Add it

		Table.Insert(Entry.pType, Entry.iChance);
		}

	//	Short-circuit

	if (Table.GetCount() == 0)
		{
		if (bFreeTable)
			delete pTable;
		STATION_PLACEMENT_OUTPUT("   no appropriate station found for this location\n");
		return ERR_NOTFOUND;
		}

	//	Roll

	*retpType = Table.GetAt(Table.RollPos());

	if (bFreeTable)
		delete pTable;
	return NOERROR;
	}

const COrbit *ComputeOffsetOrbit (CXMLElement *pObj, const COrbit &Original, COrbit *retOrbit)

//	ComputeOffsetOrbit
//
//	If the element has xOffset and yOffset attributes, it generates a new orbit

	{
	int xOffset;
	if (!pObj->FindAttributeInteger(X_OFFSET_ATTRIB, &xOffset))
		return &Original;

	int yOffset = pObj->GetAttributeInteger(Y_OFFSET_ATTRIB);

	if (xOffset == 0 && yOffset == 0)
		return &Original;

	Metric rRadius;
	Metric rAngle = VectorToPolarRadians(CVector(xOffset * g_KlicksPerPixel, yOffset * g_KlicksPerPixel), &rRadius);

	*retOrbit = COrbit(Original.GetObjectPos(), rRadius, rAngle);

	return retOrbit;
	}

int ComputeLocationWeight (SSystemCreateCtx *pCtx, 
						   const CString &sLocationAttribs,
						   const CVector &vPos,
						   const CString &sAttrib, 
						   DWORD dwMatchStrength)

//	ComputeLocationWeight
//
//	Computes the weight of the given location if we're looking for
//	locations with the given criteria.

	{
	return CAttributeCriteria::CalcLocationWeight(pCtx->pSystem, sLocationAttribs, vPos, sAttrib, dwMatchStrength);
	}

int ComputeStationWeight (SSystemCreateCtx *pCtx, CStationType *pType, const CString &sAttrib, DWORD dwMatchStrength)

//	ComputeStationWeight
//
//	Returns the weight of this station type given the attribute and match weight

	{
	return CAttributeCriteria::CalcWeightAdj(pType->HasAttribute(sAttrib), dwMatchStrength);
	}

ALERROR DistributeStationsAtRandomLocations (SSystemCreateCtx *pCtx, CXMLElement *pDesc, const COrbit &OrbitDesc)

//	DistributeStationsAtRandomLocations
//
//	Fills several locations with random stations

	{
	ALERROR error;
	int i;

	STATION_PLACEMENT_OUTPUT("DistributeStationsAtRandomLocations\n");

	//	Load location criteria

	SLocationCriteria LocationCriteria;
	if (error = GetLocationCriteria(pCtx, pDesc, &LocationCriteria))
		return error;

	//	Create a list of appropriate locations

	TProbabilityTable<int> LocationTable;
	if (!pCtx->pSystem->GetEmptyLocations(LocationCriteria, OrbitDesc, NULL, &LocationTable))
		{
		if (g_pUniverse->InDebugMode())
			{
			PushDebugStack(pCtx, strPatternSubst(CONSTLIT("FillLocations locationCriteria=%s"), pDesc->GetAttribute(LOCATION_CRITERIA_ATTRIB)));
			kernelDebugLogMessage("Warning: No locations found.");
			DumpDebugStack(pCtx);
			PopDebugStack(pCtx);
			}
		return NOERROR;
		}

	//	Figure out how many stations to create

	int iCount;
	int iPercent;
	if (pDesc->FindAttributeInteger(PERCENT_FULL_ATTRIB, &iPercent))
		iCount = iPercent * LocationTable.GetCount() / 100;
	else
		iCount = GetDiceCountFromAttribute(pDesc->GetAttribute(COUNT_ATTRIB));

	//	Figure out how many friends and enemies we need to create

	int iEnemies = 0;
	int iFriends = 0;
	int iPercentEnemies;
	if (pDesc->FindAttributeInteger(PERCENT_ENEMIES_ATTRIB, &iPercentEnemies))
		{
		for (i = 0; i < iCount; i++)
			{
			if (mathRandom(1, 100) <= iPercentEnemies)
				iEnemies++;
			else
				iFriends++;
			}
		}
	else
		{
		iFriends = iCount;
		iEnemies = iCount;
		}

	//	Station criteria

	CString sStationCriteria = pDesc->GetAttribute(STATION_CRITERIA_ATTRIB);
	CString sEnemyOnlyCriteria = (sStationCriteria.IsBlank() ? REQUIRE_ENEMY : strPatternSubst(CONSTLIT("%s,%s"), sStationCriteria, REQUIRE_ENEMY));
	CString sFriendOnlyCriteria = (sStationCriteria.IsBlank() ? REQUIRE_FRIEND : strPatternSubst(CONSTLIT("%s,%s"), sStationCriteria, REQUIRE_FRIEND));

	bool bSeparateEnemies = pDesc->GetAttributeBool(SEPARATE_ENEMIES_ATTRIB);

	PushDebugStack(pCtx, strPatternSubst(CONSTLIT("FillLocations locationCriteria=%s stationCriteria=%s"), pDesc->GetAttribute(LOCATION_CRITERIA_ATTRIB), sStationCriteria));

	//	If required, calculate stats

	if (pCtx->pStats && bSeparateEnemies)
		pCtx->pStats->AddFillLocationsTable(pCtx->pSystem, LocationTable, sStationCriteria);

	//	Create the stations

	for (i = 0; i < iCount; i++)
		{
		//	Create

		if (iEnemies && iFriends)
			{
			bool bEnemy;
			if (error = CreateAppropriateStationAtRandomLocation(pCtx, LocationTable, sStationCriteria, bSeparateEnemies, &bEnemy))
				return error;

			if (bEnemy)
				iEnemies--;
			else
				iFriends--;
			}
		else if (iEnemies)
			{
			if (error = CreateAppropriateStationAtRandomLocation(pCtx, LocationTable, sEnemyOnlyCriteria, bSeparateEnemies))
				return error;

			iEnemies--;
			}
		else if (iFriends)
			{
			if (error = CreateAppropriateStationAtRandomLocation(pCtx, LocationTable, sFriendOnlyCriteria, bSeparateEnemies))
				return error;

			iFriends--;
			}

		//	If no more locations then we're done

		if (LocationTable.GetCount() == 0)
			break;
		}

	PopDebugStack(pCtx);
	return NOERROR;
	}

ALERROR CreateAppropriateStationAtRandomLocation (SSystemCreateCtx *pCtx, 
												  TProbabilityTable<int> &LocationTable,
												  const CString &sStationCriteria,
												  bool bSeparateEnemies,
												  bool *retbEnemy)

//	CreateAppropriateStationAtRandomLocation
//
//	Picks a random location and fills it with a randomly chosen station approriate
//	to the location.

	{
	ALERROR error;

	STATION_PLACEMENT_OUTPUT("CreateAppropriateStationAtRandomLocation\n");

	//	Keep trying for a while to make sure that we find something that fits

	int iTries = 10;
	while (iTries > 0)
		{
		STATION_PLACEMENT_OUTPUT(strPatternSubst(CONSTLIT("try %d\n"), 11 - iTries).GetASCIIZPointer());

		//	Pick a random location that fits the criteria

		int iTablePos = LocationTable.RollPos();
		if (iTablePos == -1)
			return NOERROR;

		int iLocID = LocationTable[iTablePos];
		CLocationDef *pLoc = pCtx->pSystem->GetLocation(iLocID);

		//	Now look for the most appropriate station to place at the location

		CStationType *pType;
		if (error = ChooseRandomStation(pCtx, 
				sStationCriteria, 
				pLoc->GetAttributes(),
				pLoc->GetOrbit().GetObjectPos(),
				bSeparateEnemies,
				false,
				&pType))
			{
			//	If we couldn't find an appropriate location then try picking
			//	a different location.

			if (error == ERR_NOTFOUND)
				{
				//	No more tries

				if (--iTries == 0 && g_pUniverse->InDebugMode())
					{
					kernelDebugLogMessage("Warning: Ran out of tries in FillLocations");
					DumpDebugStack(pCtx);
					}

				continue;
				}
			else
				return error;
			}

		//	Remember if this is friend or enemy

		if (retbEnemy)
			*retbEnemy = pType->HasLiteralAttribute(ENEMY_ATTRIBUTE);

		//	Remember object created

		DWORD dwSavedLastObjID = pCtx->dwLastObjID;
		pCtx->dwLastObjID = 0;

#ifdef DEBUG
		CString sLocAttribs = pLoc->GetAttributes();
		CString sPosAttribs = pCtx->pSystem->GetAttribsAtPos(pLoc->GetOrbit().GetObjectPos());
#endif

		//	Create the station at the location

		SObjCreateCtx CreateCtx;
		CreateCtx.vPos = pLoc->GetOrbit().GetObjectPos();
		CreateCtx.pLoc = pLoc;
		CreateCtx.pOrbit = &pLoc->GetOrbit();
		CreateCtx.bCreateSatellites = true;

		if (error = pCtx->pSystem->CreateStation(pCtx, pType, CreateCtx))
			return error;

		//	Remove the location so it doesn't match again

		pCtx->pSystem->SetLocationObjID(iLocID, pCtx->dwLastObjID);
		pCtx->dwLastObjID = dwSavedLastObjID;
		LocationTable.Delete(iTablePos);

		break;
		}

	return NOERROR;
	}

ALERROR CreateArcDistribution (SSystemCreateCtx *pCtx, CXMLElement *pObj, const COrbit &OrbitDesc)

//	CreateArcDistribution
//
//	Creates a set of objects arranged along an arc, with Gaussian distribution.

	{
	ALERROR error;
	int i;

	//	Get the number of objects to create

	int iCount = GetDiceCountFromAttribute(pObj->GetAttribute(COUNT_ATTRIB));

	//	Short-circuit if nothing to do

	if (pObj->GetContentElementCount() == 0 || iCount <= 0)
		return NOERROR;

	//	Get parameters

	int iArcLength = pObj->GetAttributeIntegerBounded(ARC_LENGTH_ATTRIB, 0, -1, 0);
	int iRadialWidth = pObj->GetAttributeIntegerBounded(RADIAL_WIDTH_ATTRIB, 0, -1, 0);
	int iRadialEdgeWidth = pObj->GetAttributeIntegerBounded(RADIAL_EDGE_WIDTH_ATTRIB, 0, -1, 0);
	Metric rScale = GetScale(pObj);
	Metric rMaxStdDev = 3.0;

	//	If arc length is 0 then it means that we distribute evenly around the
	//	entire orbit.

	if (iArcLength == 0)
		{
		Metric rWidth = rScale * iRadialWidth / 2.0;

		for (i = 0; i < iCount; i++)
			{
			Metric rPos = mathRandomGaussian() / rMaxStdDev;

			COrbit SiblingOrbit(OrbitDesc.GetFocus(),
					OrbitDesc.GetSemiMajorAxis() + (rWidth * rPos),
					OrbitDesc.GetEccentricity(),
					OrbitDesc.GetRotation(),
					mathDegreesToRadians(mathRandom(0,3599) / 10.0));

			if (error = CreateSystemObject(pCtx, 
					pObj->GetContentElement(0), 
					SiblingOrbit))
				return error;
			}
		}

	//	Otherwise, we focus on an arc centered on the orbit position

	else
		{
		Metric rArc = rScale * iArcLength / 2.0;
		Metric rMaxWidth = rScale * iRadialWidth / 2.0;
		Metric rMinWidth = rScale * iRadialEdgeWidth / 2.0;
		Metric rRadialRange = rMaxWidth - rMinWidth;

		for (i = 0; i < iCount; i++)
			{
			//	First compute a random offset along the orbit.

			Metric rArcOffset = (mathRandomGaussian() / rMaxStdDev) * rArc;
			Metric rAngleOffset = rArcOffset / OrbitDesc.GetSemiMajorAxis();

			//	The radial offset depends on the arc offset.

			Metric rWidth = 1.0 - (Absolute(rArcOffset) / rArc);
			Metric rRadialOffset = (mathRandomGaussian() / rMaxStdDev) * (rMinWidth + (rRadialRange * rWidth));

			//	Create orbit

			COrbit SiblingOrbit(OrbitDesc.GetFocus(),
					OrbitDesc.GetSemiMajorAxis() + rRadialOffset,
					OrbitDesc.GetEccentricity(),
					OrbitDesc.GetRotation(),
					OrbitDesc.GetObjectAngle() + rAngleOffset);

			if (error = CreateSystemObject(pCtx, 
					pObj->GetContentElement(0), 
					SiblingOrbit))
				return error;
			}
		}

	return NOERROR;
	}

ALERROR CreateLabel (SSystemCreateCtx *pCtx,
					 CXMLElement *pObj,
					 const COrbit &OrbitDesc)

//	CreateLabel
//
//	Creates a labeled point

	{
	//	Add the Orbit to the list

	CLocationDef *pLoc;
	pCtx->pSystem->CreateLocation(NULL_STR, OrbitDesc, pObj->GetAttribute(ATTRIBUTES_ATTRIB), &pLoc);

	//	Keep stats

	if (pCtx->pStats)
		{
		CString sAttribs = ::AppendModifiers(pLoc->GetAttributes(), pCtx->pSystem->GetAttribsAtPos(pLoc->GetOrbit().GetObjectPos()));
		pCtx->pStats->AddLabel(sAttribs);
		}

	STATION_PLACEMENT_OUTPUT("+create label\n");

	return NOERROR;
	}

ALERROR CreateLevelTable (SSystemCreateCtx *pCtx, CXMLElement *pDesc, const COrbit &OrbitDesc)

//	CreateLevelTable
//
//	Parses a LevelTable

	{
	ALERROR error;
	int i;

	int iLevel = pCtx->pSystem->GetLevel();

	//	First we generate a table of probabilities

	TArray<int> ProbTable;
	ProbTable.InsertEmpty(pDesc->GetContentElementCount());
	int iTotalChance = 0;
	for (i = 0; i < pDesc->GetContentElementCount(); i++)
		{
		CXMLElement *pEntry = pDesc->GetContentElement(i);
		ProbTable[i] = GetFrequencyByLevel(pEntry->GetAttribute(LEVEL_FREQUENCY_ATTRIB), iLevel);
		iTotalChance += ProbTable[i];
		}

	//	Now roll a probability

	if (iTotalChance == 0)
		return NOERROR;

	int iRoll = mathRandom(1, iTotalChance);
	for (i = 0; i < pDesc->GetContentElementCount(); i++)
		{
		iRoll -= ProbTable[i];
		if (iRoll <= 0)
			{
			if (error = CreateSystemObject(pCtx, pDesc->GetContentElement(i), OrbitDesc))
				return error;
			break;
			}
		}
	
	return NOERROR;
	}

ALERROR CreateLocationCriteriaTable (SSystemCreateCtx *pCtx, CXMLElement *pDesc, const COrbit &OrbitDesc)

//	CreateLocationCriteriaTable
//
//	Parses a LevelTable

	{
	ALERROR error;
	int i, j;

	//	First we generate a table of probabilities

	TArray<int> ProbTable;
	ProbTable.InsertEmpty(pDesc->GetContentElementCount());
	int iTotalChance = 0;
	for (i = 0; i < pDesc->GetContentElementCount(); i++)
		{
		CXMLElement *pEntry = pDesc->GetContentElement(i);

		//	Initialize

		ProbTable[i] = 1000;

		//	Adjust the probability based on location criteria

		CString sCriteria;
		if (pEntry->FindAttribute(CRITERIA_ATTRIB, &sCriteria))
			{
			if (!strEquals(sCriteria, MATCH_ALL))
				{
				CAttributeCriteria Criteria;
				if (error = Criteria.Parse(sCriteria, 0, &pCtx->sError))
					return error;

				for (j = 0; j < Criteria.GetCount(); j++)
					{
					DWORD dwMatchStrength;
					const CString &sAttrib = Criteria.GetAttribAndWeight(j, &dwMatchStrength);

					int iAdj = ComputeLocationWeight(pCtx, 
							pCtx->sLocationAttribs,
							OrbitDesc.GetObjectPos(),
							sAttrib,
							dwMatchStrength);

					ProbTable[i] = (ProbTable[i] * iAdj) / 1000;
					}
				}
			}

		//	Add up

		iTotalChance += ProbTable[i];
		}

	//	Now roll a probability

	if (iTotalChance == 0)
		return NOERROR;

	int iRoll = mathRandom(1, iTotalChance);
	for (i = 0; i < pDesc->GetContentElementCount(); i++)
		{
		iRoll -= ProbTable[i];
		if (iRoll <= 0)
			{
			if (error = CreateSystemObject(pCtx, pDesc->GetContentElement(i), OrbitDesc))
				return error;
			break;
			}
		}
	
	return NOERROR;
	}

ALERROR CreateObjectAtRandomLocation (SSystemCreateCtx *pCtx, CXMLElement *pDesc, const COrbit &OrbitDesc)

//	CreateObjectAtRandomLocation
//
//	Creates one or more objects at a random location

	{
	ALERROR error;
	int i;

	//	If we have no elements, then there is nothing to do

	int iChildCount = pDesc->GetContentElementCount();
	if (iChildCount == 0)
		return NOERROR;

	PushDebugStack(pCtx, strPatternSubst(CONSTLIT("RandomLocation criteria=%s"), pDesc->GetAttribute(LOCATION_CRITERIA_ATTRIB)));

	//	Compute criteria

	SLocationCriteria Criteria;
	if (error = GetLocationCriteria(pCtx, pDesc, &Criteria))
		return error;

	//	Generate a list of all locations that match the given criteria.

	TProbabilityTable<int> Table;
	if (!pCtx->pSystem->GetEmptyLocations(Criteria, OrbitDesc, NULL, &Table))
		{
		if (g_pUniverse->InDebugMode())
			{
			kernelDebugLogMessage("Warning: No locations found for RandomLocation");
			DumpDebugStack(pCtx);
			}

		PopDebugStack(pCtx);
		return NOERROR;
		}

	//	Figure out the number of objects to create, ...

	int iCount;
	int iPercent;
	if (pDesc->FindAttributeInteger(PERCENT_FULL_ATTRIB, &iPercent))
		iCount = Max(0, iPercent * Table.GetCount() / 100);
	else
		iCount = GetDiceCountFromAttribute(pDesc->GetAttribute(COUNT_ATTRIB));

	if (iChildCount > 1)
		iCount = Max(iCount, iChildCount);

	//	Loop

	CString sSavedLocationAttribs = pCtx->sLocationAttribs;

	for (i = 0; i < iCount; i++)
		{
		COrbit NewOrbit;

		//	If we're out of labels then that's OK

		if (Table.IsEmpty())
			{
			if (g_pUniverse->InDebugMode())
				{
				kernelDebugLogMessage("Warning: Ran out of locations in RandomLocation directive");
				DumpDebugStack(pCtx);
				}

			pCtx->sLocationAttribs = sSavedLocationAttribs;
			PopDebugStack(pCtx);
			return NOERROR;
			}

		//	Pick a random location from our table

		int iRollPos = Table.RollPos();
		int iLocID = Table[iRollPos];
		CLocationDef *pLoc = pCtx->pSystem->GetLocation(iLocID);

		//	Create a superset of location attributes

		pCtx->sLocationAttribs = ::AppendModifiers(sSavedLocationAttribs, pLoc->GetAttributes());

		//	Create the object

		DWORD dwSavedLastObjID = pCtx->dwLastObjID;
		pCtx->dwLastObjID = 0;

		if (error = CreateSystemObject(pCtx, pDesc->GetContentElement(i % iChildCount), pLoc->GetOrbit()))
			return error;

		//	If we actually created an object, then remove the label

		pCtx->pSystem->SetLocationObjID(iLocID, pCtx->dwLastObjID);
		pCtx->dwLastObjID = dwSavedLastObjID;

		//	Also remove from our table

		Table.Delete(iRollPos);

#ifdef DEBUG_STATION_EXCLUSION_ZONE
		::kernelDebugLogMessage("CreateObjectAtRandomLocation: Created %s",
				pDesc->GetContentElement(i % iChildCount)->GetTag());
#endif
		}

	pCtx->sLocationAttribs = sSavedLocationAttribs;
	PopDebugStack(pCtx);

	return NOERROR;
	}

ALERROR CreateOrbitals (SSystemCreateCtx *pCtx, 
						CXMLElement *pObj, 
						const COrbit &OrbitDesc)

//	CreateOrbitals
//
//	Create multiple objects in orbit around the given center

	{
	ALERROR error;

	//	If we have no elements, then there is nothing to do

	if (pObj->GetContentElementCount() == 0)
		return NOERROR;

	PushDebugStack(pCtx, CONSTLIT("Orbitals"));

	//	Figure out the number of objects to create, ...

	int iCount = GetDiceCountFromAttribute(pObj->GetAttribute(COUNT_ATTRIB));
	if (pObj->GetContentElementCount() > 1)
		iCount = Max(iCount, pObj->GetContentElementCount());

	//	...the distance range

	CString sDistance = pObj->GetAttribute(DISTANCE_ATTRIB);
	CString sBodeDistanceStart = pObj->GetAttribute(BODE_DISTANCE_START_ATTRIB);

	//	...the angle of the objects

	CString sAngle = pObj->GetAttribute(ANGLE_ATTRIB);

	//	...elliptical orbit parameters

	DiceRange EccentricityRange(0, 0, 0);
	CString sEccentricity = pObj->GetAttribute(ECCENTRICITY_ATTRIB);
	if (!sEccentricity.IsBlank())
		{
		if (error = EccentricityRange.LoadFromXML(sEccentricity))
			{
			pCtx->sError = CONSTLIT("Invalid eccentricity in orbitals");
			return error;
			}
		}

	CString sEllipseRotation = pObj->GetAttribute(ROTATION_ATTRIB);

	//	...other parameters

	bool bNoOverlap = pObj->GetAttributeBool(NO_OVERLAP_ATTRIB);
	int iExclusionRadius = pObj->GetAttributeIntegerBounded(EXCLUSION_RADIUS_ATTRIB, 0, -1, 0);

	//	Calculate the number of objects

	int i, j;

	//	Create an array of position vectors for the objects

	if (iCount > 0)
		{
		Metric *rDistance = (Metric *)_alloca(iCount * sizeof(Metric));
		Metric *rAngle = (Metric *)_alloca(iCount * sizeof(Metric));
		Metric *rEccentricity = (Metric *)_alloca(iCount * sizeof(Metric));
		Metric *rRotation = (Metric *)_alloca(iCount * sizeof(Metric));

		//	Calculate the distance for each object. If the distance is specified
		//	then use that.

		if (!sDistance.IsBlank())
			{
			DiceRange DistanceRange;

			if (error = DistanceRange.LoadFromXML(sDistance))
				{
				pCtx->sError = CONSTLIT("Invalid distance in orbitals");
				return error;
				}

			Metric rScale = GetScale(pObj);

			for (i = 0; i < iCount; i++)
				rDistance[i] = rScale * DistanceRange.Roll();
			}

		//	If Bode distance range specified then compute that

		else if (!sBodeDistanceStart.IsBlank())
			{
			DiceRange Range;

			Range.LoadFromXML(sBodeDistanceStart);
			Metric rDistanceStart = Range.Roll();

			Range.LoadFromXML(pObj->GetAttribute(BODE_DISTANCE_END_ATTRIB));
			Metric rDistanceEnd = Range.Roll();

			Metric rScale = GetScale(pObj);

			if (iCount == 1)
				rDistance[0] = rScale * (rDistanceStart + ((rDistanceEnd - rDistanceStart) / 2.0));
			else if (iCount == 2)
				{
				rDistance[0] = rScale * rDistanceStart;
				rDistance[1] = rScale * rDistanceEnd;
				}
			else
				{
				const Metric K = 1.35;		//	Orbital ratio (for Bode's Law, this
											//	is actually around 2, but we adjust
											//	it so that the planets aren't so far
											//	apart.

				Metric rDen = 1.0;
				Metric rKSum = K;
				for (i = 2; i < iCount; i++)
					{
					rDen += rKSum;
					rKSum *= K;
					}

				Metric rDist = rDistanceStart;
				Metric rDistInc = (rDistanceEnd - rDistanceStart) / rDen;

				for (i = 0; i < iCount; i++)
					{
					rDistance[i] = rScale * rDist;
					rDist += rDistInc;
					rDistInc *= K;
					}
				}
			}

		//	Otherwise, distance is specified in children

		else
			{
			int iSubCount = pObj->GetContentElementCount();

			for (i = 0; i < iCount; i++)
				{
				CXMLElement *pSub = pObj->GetContentElement(i % iSubCount);

				DiceRange DistanceRange;
				if (error = DistanceRange.LoadFromXML(pSub->GetAttribute(DISTANCE_ATTRIB)))
					{
					pCtx->sError = CONSTLIT("Invalid distance in orbitals");
					return error;
					}

				Metric rScale = GetScale(pSub);

				rDistance[i] = rScale * DistanceRange.Roll();
				}
			}

		//	Calculate eccentricity

		for (i = 0; i < iCount; i++)
			{
			rEccentricity[i] = EccentricityRange.Roll() / 100.0;
			if (rEccentricity[i] > 0.99)
				rEccentricity[i] = 0.99;
			}

		//	Calculate rotation angles for each object

		if (!sEllipseRotation.IsBlank())
			{
			if (error = GenerateAngles(pCtx, sEllipseRotation, iCount, rRotation))
				return error;
			}
		else
			{
			for (i = 0; i < iCount; i++)
				rRotation[i] = 0.0;
			}

		//	Calculate the angle for each object. If we want to check for overlap, we
		//	iterate several times to make sure that we have chosen non-overlapping
		//	points.

		bool bConfigurationOK = true;
		Metric rExclusionRadius = iExclusionRadius * LIGHT_SECOND;
		Metric rExclusionRadius2 = rExclusionRadius * rExclusionRadius;

		//	For random angles we try a slightly different algorithm

		if (strEquals(sAngle, RANDOM_ANGLE))
			{
			for (i = 0; i < iCount; i++)
				{
				bool bAngleOK = true;
				int iTries = 20;

				do
					{
					rAngle[i] = mathDegreesToRadians(mathRandom(0,3599) / 10.0);
					bAngleOK = true;

					if (iExclusionRadius != 0)
						{
						COrbit NewOrbit(OrbitDesc.GetObjectPos(),
								rDistance[i],
								rEccentricity[i],
								rRotation[i],
								rAngle[i]);

						if (!IsExclusionZoneClear(pCtx, NewOrbit.GetObjectPos(), rExclusionRadius))
							bAngleOK = false;

						//	Make none we are not near any other point that we 
						//	just generated.

						if (bAngleOK)
							{
							for (j = 0; j < i; j++)
								{
								COrbit OtherOrbit(OrbitDesc.GetObjectPos(),
										rDistance[j],
										rEccentricity[j],
										rRotation[j],
										rAngle[j]);

								CVector vDist = OtherOrbit.GetObjectPos() - NewOrbit.GetObjectPos();
								if (vDist.Length2() < rExclusionRadius2)
									{
									bAngleOK = false;
									break;
									}
								}
							}
						}
					else if (bNoOverlap)
						{
						COrbit NewOrbit(OrbitDesc.GetObjectPos(),
								rDistance[i],
								rEccentricity[i],
								rRotation[i],
								rAngle[i]);

						if (CheckForOverlap(pCtx, NewOrbit.GetObjectPos()))
							bAngleOK = false;
						}
					}
				while (!bAngleOK && --iTries > 0);

				//	If one of the angles is not OK then the configuration is not OK

				if (!bAngleOK)
					bConfigurationOK = false;
				}
			}

		//	Otherwise, keep generating a complete set of angles until we find
		//	something that fits

		else
			{
			int iTries = 10;

			do
				{
				if (error = GenerateAngles(pCtx, sAngle, iCount, rAngle))
					return error;

				//	If any of the positions overlap, then the configuration is not OK

				if (iExclusionRadius != 0)
					{
					bConfigurationOK = true;
					for (i = 0; i < iCount; i++)
						{
						COrbit NewOrbit(OrbitDesc.GetObjectPos(),
								rDistance[i],
								rEccentricity[i],
								rRotation[i],
								rAngle[i]);

						if (!IsExclusionZoneClear(pCtx, NewOrbit.GetObjectPos(), rExclusionRadius))
							{
							bConfigurationOK = false;
							break;
							}
						}
					}
				else if (bNoOverlap)
					{
					bConfigurationOK = true;
					for (i = 0; i < iCount; i++)
						{
						COrbit NewOrbit(OrbitDesc.GetObjectPos(),
								rDistance[i],
								rEccentricity[i],
								rRotation[i],
								rAngle[i]);

						if (CheckForOverlap(pCtx, NewOrbit.GetObjectPos()))
							{
							bConfigurationOK = false;
							break;
							}
						}
					}
				}
			while (!bConfigurationOK && --iTries > 0);
			}

		//	Log error

#ifdef DEBUG_STATION_SEPARATION
		if (!bConfigurationOK)
			{
			if (iExclusionRadius != 0)
				kernelDebugLogMessage("<Orbitals>: Unable to find clear exclusion zone: %d ls radius.", iExclusionRadius);
			else if (bNoOverlap)
				kernelDebugLogMessage("<Orbitals>: Unable to find non-overlapping configuration.");
			else
				kernelDebugLogMessage("<Orbitals>: Unable to find valid configuration.");
			}
#endif

		//	Create each object

		int iObj = 0;
		int iPos = 0;
		int iLoops = Max(iCount, pObj->GetContentElementCount());
		for (i = 0; i < iLoops; i++)
			{
			COrbit NewOrbit(OrbitDesc.GetObjectPos(),
					rDistance[iPos],
					rEccentricity[iPos],
					rRotation[iPos],
					rAngle[iPos]);

			if (error = CreateSystemObject(pCtx, pObj->GetContentElement(iObj), NewOrbit))
				return error;

			iObj = (iObj + 1) % pObj->GetContentElementCount();
			iPos = (iPos + 1) % iCount;
			}
		}

	//	Done

	PopDebugStack(pCtx);

	return NOERROR;
	}

ALERROR CreateOffsetObjects (SSystemCreateCtx *pCtx, CXMLElement *pObj, const COrbit &OrbitDesc)

//	CreateOffsetObjects
//
//	Create objects offset from orbit

	{
	ALERROR error;
	int i;

	//	Figure out the number of objects to create, ...

	int iObjCount = pObj->GetContentElementCount();
	if (iObjCount == 0)
		return NOERROR;

	int iCount = Max(GetDiceCountFromAttribute(pObj->GetAttribute(COUNT_ATTRIB)), iObjCount);
	if (iCount <= 0)
		return NOERROR;

	//	Generate an array of radius/angle pairs for each object we create

	TArray<CVector> Points;
	Points.InsertEmpty(iCount);

	//	Get the scale

	Metric rScale = GetScale(pObj);

	//	Get the offset. We look for radius and angle first.

	CString sRadius;
	if (pObj->FindAttribute(RADIUS_ATTRIB, &sRadius))
		{
		DiceRange Radius;
		if (Radius.LoadFromXML(sRadius) != NOERROR)
			{
			pCtx->sError = CONSTLIT("Invalid dice count for radius.");
			return ERR_FAIL;
			}

		DiceRange Angle;
		if (Angle.LoadFromXML(pObj->GetAttribute(ANGLE_ATTRIB)) != NOERROR)
			{
			pCtx->sError = CONSTLIT("Invalid dice count for angle.");
			return ERR_FAIL;
			}

		//	Generate values

		for (i = 0; i < iCount; i++)
			Points[i] = CVector(rScale * Radius.Roll(), mathDegreesToRadians(Angle.Roll()));
		}

	//	Otherwise, we expect Cartessian offset

	else
		{
		DiceRange X;
		if (X.LoadFromXML(pObj->GetAttribute(X_OFFSET_ATTRIB)) != NOERROR)
			{
			pCtx->sError = CONSTLIT("Invalid dice count for xOffset.");
			return ERR_FAIL;
			}

		DiceRange Y;
		if (Y.LoadFromXML(pObj->GetAttribute(Y_OFFSET_ATTRIB)) != NOERROR)
			{
			pCtx->sError = CONSTLIT("Invalid dice count for yOffset.");
			return ERR_FAIL;
			}

		//	Generate values

		for (i = 0; i < iCount; i++)
			{
			Metric rRadius;
			Metric rAngle = VectorToPolarRadians(CVector(rScale * X.Roll(), rScale * Y.Roll()), &rRadius);

			Points[i] = CVector(rRadius, rAngle);
			}
		}

	//	Create all the object

	for (i = 0; i < iCount; i++)
		{
		COrbit NewOrbit(OrbitDesc.GetObjectPos(), Points[i].GetX(), Points[i].GetY());

		if (error = CreateSystemObject(pCtx, pObj->GetContentElement(i % iObjCount), NewOrbit))
			return error;
		}

	return NOERROR;
	}

ALERROR CreateRandomStation (SSystemCreateCtx *pCtx, 
							 CXMLElement *pDesc, 
							 const COrbit &OrbitDesc)

//	CreateRandomStation
//
//	Creates a random station at the given location

	{
	ALERROR error;

	CString sStationCriteria = pDesc->GetAttribute(STATION_CRITERIA_ATTRIB);
	CString sLocationAttribs = pDesc->GetAttribute(LOCATION_ATTRIBS_ATTRIB);
	if (sLocationAttribs.IsBlank())
		sLocationAttribs = pCtx->sLocationAttribs;
	bool bIncludeAll = pDesc->GetAttributeBool(INCLUDE_ALL_ATTRIB);

	PushDebugStack(pCtx, strPatternSubst(CONSTLIT("RandomStation stationCriteria=%s locationAttribs=%s"), sStationCriteria, sLocationAttribs));

	//	Pick a random station type that fits the criteria

	CStationType *pType;
	if (error = ChooseRandomStation(pCtx, 
			sStationCriteria, 
			sLocationAttribs,
			OrbitDesc.GetObjectPos(),
			false,
			bIncludeAll,
			&pType))
		{
		if (error == ERR_NOTFOUND)
			{
			PopDebugStack(pCtx);
			return NOERROR;
			}
		else
			return error;
		}

	//	Create the station at the location

	SObjCreateCtx CreateCtx;
	CreateCtx.vPos = OrbitDesc.GetObjectPos();
	CreateCtx.pOrbit = &OrbitDesc;
	CreateCtx.bCreateSatellites = !pDesc->GetAttributeBool(NO_SATELLITES_ATTRIB);
	CreateCtx.pExtraData = pDesc->GetContentElementByTag(INITIAL_DATA_TAG);

	CSpaceObject *pObj;
	if (error = pCtx->pSystem->CreateStation(pCtx,
			pType,
			CreateCtx,
			&pObj))
		return error;

	CStation *pStation = (pObj ? pObj->AsStation() : NULL);
	if (pStation == NULL)
		{
		PopDebugStack(pCtx);
		return NOERROR;
		}

	//	Modify the station, if necessary

	if (error = ModifyCreatedStation(pCtx, pStation, pDesc, OrbitDesc))
		return error;

	PopDebugStack(pCtx);
	return NOERROR;
	}

ALERROR CreateRandomStationAtAppropriateLocation (SSystemCreateCtx *pCtx, CXMLElement *pDesc)

//	CreateRandomStationAtAppropriateLocation
//
//	Picks a random station and then picks a random location appropriate
//	for the given station.

	{
	ALERROR error;

	STATION_PLACEMENT_OUTPUT("CreateRandomStationAtAppropriateLocation\n");

	CString sStationCriteria = pDesc->GetAttribute(STATION_CRITERIA_ATTRIB);
	bool bSeparateEnemies = pDesc->GetAttributeBool(SEPARATE_ENEMIES_ATTRIB);

	PushDebugStack(pCtx, strPatternSubst(CONSTLIT("PlaceRandomStation stationCriteria=%s"), sStationCriteria));

	//	Keep trying for a while to make sure that we find something that fits

	int iTries = 10;
	while (iTries > 0)
		{
		//	Pick a random station type that fits the criteria

		CStationType *pType;
		if (error = ChooseRandomStation(pCtx, 
				sStationCriteria, 
				MATCH_ALL,
				NullVector,
				false,
				false,
				&pType))
			{
			if (error == ERR_NOTFOUND)
				{
				PopDebugStack(pCtx);
				return NOERROR;
				}
			else
				return error;
			}

		//	Get the sovereign for this station (we need it is we want to separate
		//	stations that are enemies).

		CSovereign *pSovereign = NULL;
		if (bSeparateEnemies)
			pSovereign = pType->GetSovereign();

		//	Now look for the most appropriate location to place the station

		COrbit OrbitDesc;
		CString sLocationAttribs;
		int iLocation;
		if (error = ChooseRandomLocation(pCtx, 
				pType->GetLocationCriteria(), 
				(bSeparateEnemies ? pType : NULL),
				&OrbitDesc, 
				&sLocationAttribs,
				&iLocation))
			{
			//	If we couldn't find an appropriate location then try picking
			//	a different kind of station.

			if (error == ERR_NOTFOUND)
				{
				iTries--;
				continue;
				}
			else
				return error;
			}

		//	Remember saved last obj

		DWORD dwSavedLastObjID = pCtx->dwLastObjID;
		pCtx->dwLastObjID = 0;

		//	Create the station at the location

		SObjCreateCtx CreateCtx;
		CreateCtx.vPos = OrbitDesc.GetObjectPos();
		CreateCtx.pLoc = pCtx->pSystem->GetLocation(iLocation);
		CreateCtx.pOrbit = &OrbitDesc;
		CreateCtx.bCreateSatellites = true;

		if (error = pCtx->pSystem->CreateStation(pCtx, pType, CreateCtx))
			return error;

		//	Remember that we filled this location

		pCtx->pSystem->SetLocationObjID(iLocation, pCtx->dwLastObjID);
		pCtx->dwLastObjID = dwSavedLastObjID;

		//	No more tries

		break;
		}

	PopDebugStack(pCtx);
	return NOERROR;
	}

ALERROR CreateSiblings (SSystemCreateCtx *pCtx, 
						CXMLElement *pObj, 
						const COrbit &OrbitDesc)

//	CreateSiblings
//
//	Creates objects in similar orbits

	{
	int i;
	ALERROR error;
	CString sDistribution;

	if (pObj->GetContentElementCount() == 0)
		return NOERROR;

	//	Get the number of objects to create

	int iCount = GetDiceCountFromAttribute(pObj->GetAttribute(COUNT_ATTRIB));
	if (iCount <= 0)
		return NOERROR;

	//	Create a random distribution

	if (pObj->FindAttribute(DISTRIBUTION_ATTRIB, &sDistribution))
		{
		DiceRange Distribution;
		Distribution.LoadFromXML(sDistribution);
		Metric rScale = GetScale(pObj);

		for (i = 0; i < iCount; i++)
			{
			COrbit SiblingOrbit(OrbitDesc.GetFocus(),
					OrbitDesc.GetSemiMajorAxis() + (rScale * Distribution.Roll()),
					OrbitDesc.GetEccentricity(),
					OrbitDesc.GetRotation(),
					mathDegreesToRadians(mathRandom(0,3599) / 10.0));

			if (error = CreateSystemObject(pCtx, 
					pObj->GetContentElement(0), 
					SiblingOrbit))
				return error;
			}
		}

	//	Create objects with radial and angular offsets

	else
		{
		CString sAttrib;

		//	Load the radial increment

		Metric rScale = GetScale(pObj);
		DiceRange RadiusAdj;
		Metric rRadiusAdjScale;
		if (pObj->FindAttribute(RADIUS_INC_ATTRIB, &sAttrib))
			{
			RadiusAdj.LoadFromXML(sAttrib);
			rRadiusAdjScale = rScale;
			}
		else if (pObj->FindAttribute(RADIUS_DEC_ATTRIB, &sAttrib))
			{
			RadiusAdj.LoadFromXML(sAttrib);
			rRadiusAdjScale = -rScale;
			}
		else
			rRadiusAdjScale = 0.0;

		//	Load the angle or arc increment

		enum IncTypes { incNone, incAngle, incArc, incFixed };
		IncTypes iAngleInc;
		DiceRange AngleAdj;
		Metric *pAngles = NULL;
		if (pObj->FindAttribute(ANGLE_ATTRIB, &sAttrib))
			{
			iAngleInc = incFixed;
			pAngles = new Metric [iCount];
			if (error = GenerateAngles(pCtx, sAttrib, iCount, pAngles))
				return error;
			}
		else if (pObj->FindAttribute(ARC_INC_ATTRIB, &sAttrib))
			{
			iAngleInc = incArc;
			AngleAdj.LoadFromXML(sAttrib);
			}
		else if (pObj->FindAttribute(ANGLE_ADJ_ATTRIB, &sAttrib))
			{
			iAngleInc = incAngle;
			AngleAdj.LoadFromXML(sAttrib);
			}
		else if (pObj->FindAttribute(ANGLE_INC_ATTRIB, &sAttrib))
			{
			iAngleInc = incAngle;
			AngleAdj.LoadFromXML(sAttrib);
			}
		else
			{
			iAngleInc = incNone;
			}

		//	Generate all the orbits

		TArray<COrbit> Orbits;
		Orbits.InsertEmpty(iCount);
		for (i = 0; i < iCount; i++)
			{
			Metric rRadiusAdj = rRadiusAdjScale * (Metric)RadiusAdj.Roll();
			Metric rAngleAdj;
			switch (iAngleInc)
				{
				case incArc:
					{
					//	Convert from linear to angular
					Metric rCirc = (OrbitDesc.GetSemiMajorAxis() + rRadiusAdj);
					rAngleAdj = (rCirc > 0.0 ? ((Metric)AngleAdj.Roll() * rScale) / rCirc : 0.0);
					break;
					}

				case incAngle:
					rAngleAdj = mathDegreesToRadians((360 + AngleAdj.Roll()) % 360);
					break;

				case incFixed:
					rAngleAdj = pAngles[i] - OrbitDesc.GetObjectAngle();
					break;

				default:
					rAngleAdj = 0.0;
				}

			Orbits[i] = COrbit(OrbitDesc.GetFocus(),
					OrbitDesc.GetSemiMajorAxis() + rRadiusAdj,
					OrbitDesc.GetEccentricity(),
					OrbitDesc.GetRotation(),
					OrbitDesc.GetObjectAngle() + rAngleAdj);
			}

		//	Loop over count

		int iPos = 0;
		int iObj = 0;
		int iLoops = Max(iCount, pObj->GetContentElementCount());

		for (i = 0; i < iLoops; i++)
			{
			if (error = CreateSystemObject(pCtx,
					pObj->GetContentElement(iObj),
					Orbits[iPos]))
				return error;

			iObj = (iObj + 1) % pObj->GetContentElementCount();
			iPos = (iPos + 1) % iCount;
			}

		if (pAngles)
			delete [] pAngles;
		}

	return NOERROR;
	}

ALERROR CreateShipsForStation (CSpaceObject *pStation, CXMLElement *pShips)

//	CreateShipsForStation
//
//	Creates random ships for a station

	{
	ALERROR error;

	//	Load the generator

	SDesignLoadCtx Ctx;

	IShipGenerator *pGenerator;
	if (error = IShipGenerator::CreateFromXMLAsGroup(Ctx, pShips, &pGenerator))
		{
		ASSERT(false);
		kernelDebugLogMessage("Unable to load ship generator: %s", Ctx.sError);
		return error;
		}

	if (error = pGenerator->OnDesignLoadComplete(Ctx))
		{
		ASSERT(false);
		kernelDebugLogMessage("Unable to load ship generator: %s", Ctx.sError);
		return error;
		}

	//	Create the ships

	pStation->CreateRandomDockedShips(pGenerator);

	//	Done

	delete pGenerator;
	return NOERROR;
	}

void AdjustBounds (CVector *pUL, CVector *pLR, const CVector &vPos)
	{
	if (vPos.GetX() < pUL->GetX())
		pUL->SetX(vPos.GetX());
	else if (vPos.GetX() > pLR->GetX())
		pLR->SetX(vPos.GetX());

	if (vPos.GetY() < pLR->GetY())
		pLR->SetY(vPos.GetY());
	else if (vPos.GetY() > pUL->GetY())
		pUL->SetY(vPos.GetY());
	}

void ChangeVariation (Metric *pVariation, Metric rMaxVariation)
	{
	Metric rChange = (rMaxVariation / 15.0);

	int iChange = mathRandom(1, 3);
	if (iChange == 1)
		{
		if (*pVariation > 0.0)
			*pVariation -= rChange;
		else
			*pVariation += rChange;
		}
	else if (iChange == 3)
		{
		if (*pVariation < rMaxVariation)
			*pVariation += rChange;
		else
			*pVariation -= rChange;
		}
	}

ALERROR CreateSpaceEnvironment (SSystemCreateCtx *pCtx, CXMLElement *pDesc, const COrbit &OrbitDesc)

//	CreateSpaceEnvironment
//
//	Creates a space environment, such as nebulae

	{
	ALERROR error;
	int i;

	//	Figure out what tile we need here

	CSpaceEnvironmentType *pEnvType = g_pUniverse->FindSpaceEnvironment(pDesc->GetAttributeInteger(TYPE_ATTRIB));
	if (pEnvType == NULL)
		{
		pCtx->sError = CONSTLIT("Invalid space environment type");
		return ERR_FAIL;
		}

	//	Keep track of all the places where we've added nebulae

	TArray<CEnvironmentGrid::STileDesc> NebulaTiles;

	//	Initialize some context

	CEnvironmentGrid *pEnvironment = pCtx->pSystem->GetEnvironmentGrid();
	CEnvironmentGrid::SCreateCtx CreateCtx;
	CreateCtx.pSystem = pCtx->pSystem;
	CreateCtx.pOrbitDesc = &OrbitDesc;
	CreateCtx.pEnv = pEnvType;

	//	Fill the appropriate shape

	CString sShape = pDesc->GetAttribute(SHAPE_ATTRIB);
	if (strEquals(sShape, SHAPE_CIRCULAR))
		{
		CreateCtx.rWidth = GetDiceCountFromAttribute(pDesc->GetAttribute(WIDTH_ATTRIB)) * LIGHT_SECOND;
		CreateCtx.iWidthVariation = GetDiceCountFromAttribute(pDesc->GetAttribute(WIDTH_VARIATION_ATTRIB));

		pEnvironment->CreateCircularNebula(CreateCtx, &NebulaTiles);
		}
	else if (strEquals(sShape, SHAPE_ARC))
		{
		CreateCtx.rWidth = GetDiceCountFromAttribute(pDesc->GetAttribute(WIDTH_ATTRIB)) * LIGHT_SECOND;
		CreateCtx.iWidthVariation = GetDiceCountFromAttribute(pDesc->GetAttribute(WIDTH_VARIATION_ATTRIB));
		CreateCtx.iSpan = GetDiceCountFromAttribute(pDesc->GetAttribute(SPAN_ATTRIB));

		pEnvironment->CreateArcNebula(CreateCtx, &NebulaTiles);
		}
	else
		{
		pCtx->sError = CONSTLIT("Invalid space environment shape");
		return ERR_FAIL;
		}

	//	Patches

	CEffectCreator *pPatchType = g_pUniverse->FindEffectType(pDesc->GetAttributeInteger(PATCHES_ATTRIB));
	int iPatchFrequency = GetDiceCountFromAttribute(pDesc->GetAttribute(PATCH_FREQUENCY_ATTRIB));

	if (pPatchType && iPatchFrequency > 0)
		{
		int iCount = NebulaTiles.GetCount();

		//	Adjust patch frequency based on size of tile (based on 512x512 tiles).

		int iSize = 512 / pCtx->pSystem->GetTileSize();
		iSize *= iSize;
		if (iSize > 1)
			iCount /= iSize;

		//	Create all patches

		for (i = 0; i < iCount; i++)
			{
			if (mathRandom(1, 100) <= iPatchFrequency)
				{
				int iTileSize = pCtx->pSystem->GetTileSize();
				const CEnvironmentGrid::STileDesc &TileDesc = NebulaTiles[mathRandom(0, NebulaTiles.GetCount() - 1)];
				CVector vPos = pCtx->pSystem->TileToVector(TileDesc.x, TileDesc.y);

				CVector vOffset(
						g_KlicksPerPixel * (mathRandom(0, iTileSize) - (iTileSize / 2)),
						g_KlicksPerPixel * (mathRandom(0, iTileSize) - (iTileSize / 2)));

				CStaticEffect::Create(pPatchType,
						pCtx->pSystem,
						vPos + vOffset,
						NULL);
				}
			}
		}

	//	Create random encounters

	int iEncounters = GetDiceCountFromAttribute(pDesc->GetAttribute(ENCOUNTERS_ATTRIB));

	CXMLElement *pEncounter = NULL;
	if (pDesc->GetContentElementCount())
		pEncounter = pDesc->GetContentElement(0);

	if (pEncounter && NebulaTiles.GetCount() > 0)
		{
		if (iEncounters == 0)
			iEncounters = 1;

		int iTileSize = pCtx->pSystem->GetTileSize();

		for (i = 0; i < iEncounters; i++)
			{
			//	Pick a random tile

			const CEnvironmentGrid::STileDesc &TileDesc = NebulaTiles[mathRandom(0, NebulaTiles.GetCount() - 1)];

			CVector vCenter = pCtx->pSystem->TileToVector(TileDesc.x, TileDesc.y);
			CVector vOffset(
					g_KlicksPerPixel * (mathRandom(0, iTileSize) - (iTileSize / 2)),
					g_KlicksPerPixel * (mathRandom(0, iTileSize) - (iTileSize / 2)));

			COrbit NewOrbit(vCenter + vOffset, 0.0);

			//	Create the object

			if (error = CreateSystemObject(pCtx, pEncounter, NewOrbit))
				return error;
			}
		}

	return NOERROR;
	}

void CreateSpaceEnvironmentTile (SSystemCreateCtx *pCtx,
								 const CVector &vPos,
								 int xTile,
								 int yTile,
								 CSpaceEnvironmentType *pEnvironment,
								 CEffectCreator *pPatch,
								 int iPatchFrequency)
	{
	pCtx->pSystem->SetSpaceEnvironment(xTile, yTile, pEnvironment);

	//	Create some patches

	if (pPatch && mathRandom(1, 100) <= iPatchFrequency)
		{
		int iTileSize = pCtx->pSystem->GetTileSize();

		CVector vOffset(
				g_KlicksPerPixel * (mathRandom(0, iTileSize) - (iTileSize / 2)),
				g_KlicksPerPixel * (mathRandom(0, iTileSize) - (iTileSize / 2)));

		CStaticEffect::Create(pPatch,
				pCtx->pSystem,
				vPos + vOffset,
				NULL);
		}
	}

ALERROR CreateStargate (SSystemCreateCtx *pCtx, CXMLElement *pDesc, const COrbit &OrbitDesc)

//	CreateStargate
//
//	Creates a stargate (and sub-objects) based on the topology

	{
	ALERROR error;

	//	Get the name of this stargate

	CString sStargate = pDesc->GetAttribute(OBJ_NAME_ATTRIB);
	if (sStargate.IsBlank())
		{
		pCtx->sError = CONSTLIT("<Stargate> must specify objName");
		return ERR_FAIL;
		}

	//	Figure out the destination node for the stargate

	CString sDestEntryPoint;
	CTopologyNode *pDestNode = pCtx->pSystem->GetStargateDestination(sStargate, &sDestEntryPoint);

	//	If we don't have a destination node, then it means that this is an optional stargate
	//	so we exit with no error.

	if (pDestNode == NULL)
		return NOERROR;

	//	We create the stargate (as if the element were a <Station> element)

	CStation *pStation;
	if (error = CreateStationFromElement(pCtx, pDesc, OrbitDesc, &pStation))
		return error;

	if (pStation == NULL)
		{
		pCtx->sError = CONSTLIT("<Stargate> must be a station");
		return ERR_FAIL;
		}

	//	Object created

	pCtx->dwLastObjID = pStation->GetID();

	//	Set stargate properties (note: CreateStation also looks at objName and adds the name
	//	to the named-objects system table.)

	pStation->SetStargate(pDestNode->GetID(), sDestEntryPoint);

	//	If we haven't already set the name, set the name of the stargate
	//	to include the name of the destination system

	if (!pStation->IsNameSet())
		pStation->SetName(strPatternSubst(CONSTLIT("%s Stargate"), pDestNode->GetSystemName()), nounDefiniteArticle);

	return NOERROR;
	}

ALERROR CreateSystemObject (SSystemCreateCtx *pCtx, 
							CXMLElement *pObj, 
							const COrbit &OrbitDesc,
							bool bIgnoreChance)

//	CreateSystemObject
//
//	Create an object in the system. The pObj element may be one of
//	the following tags:
//
//		<AntiTrojan ...>
//		<ArcDistribution ...>
//		<Code ...>
//		<Encounter ...>
//		<Group ...>
//		<Label ...>
//		<LabelStation ...>
//		<LevelTable ...>
//		<Lookup ...>
//		<Null/>
//		<Offset ...>
//		<Orbitals ...>
//		<Particles ...>
//		<Primary ...>
//		<Siblings ...>
//		<SpaceEnvironment ...>
//		<Station ...>
//		<Table ...>
//		<Trojan ...>
//		<Variants ...>

	{
	ALERROR error;
	CString sTag = pObj->GetTag();

	STATION_PLACEMENT_OUTPUT(strPatternSubst(CONSTLIT("<%s>\n"), sTag).GetASCIIZPointer());

	//	Nothing to do if this is a debug-only object and we are not in
	//	debug mode.

	if (pObj->GetAttributeBool(DEBUG_ONLY_ATTRIB) && !g_pUniverse->InDebugMode())
		return NOERROR;

	//	See if we've got a probability

	if (!bIgnoreChance)
		{
		int iProb = Max(pObj->GetAttributeInteger(PROBABILITY_ATTRIB), pObj->GetAttributeInteger(CHANCE_ATTRIB));
		if (iProb > 0 && mathRandom(1, 100) > iProb)
			return NOERROR;
		}

	//	Act based on the tag

	if (strEquals(sTag, STATION_TAG))
		{
		if (error = CreateStationFromElement(pCtx, pObj, OrbitDesc))
			return error;
		}
	else if (strEquals(sTag, TABLE_TAG))
		{
		CRandomEntryResults Results;

		PushDebugStack(pCtx, TABLE_TAG);

		if (error = CRandomEntryGenerator::Generate(pObj, Results))
			{
			pCtx->sError = CONSTLIT("<Table> error");
			return error;
			}

		for (int i = 0; i < Results.GetCount(); i++)
			{
			CXMLElement *pResult = Results.GetResult(i);

			if (error = CreateSystemObject(pCtx, pResult, OrbitDesc, true))
				return error;
			}

		PopDebugStack(pCtx);
		}
	else if (strEquals(sTag, GROUP_TAG) || strEquals(sTag, SYSTEM_GROUP_TAG))
		{
		for (int i = 0; i < pObj->GetContentElementCount(); i++)
			{
			CXMLElement *pItem = pObj->GetContentElement(i);

			if (error = CreateSystemObject(pCtx, pItem, OrbitDesc))
				return error;
			}
		}
	else if (strEquals(sTag, LOOKUP_TAG))
		{
		CString sTable = pObj->GetAttribute(TABLE_ATTRIB);

		//	If we've got an offset, change the orbit

		COrbit NewOrbit;
		const COrbit *pOrbitDesc = ComputeOffsetOrbit(pObj, OrbitDesc, &NewOrbit);

		//	Create

		if (error = pCtx->pSystem->CreateLookup(pCtx, sTable, *pOrbitDesc, pObj))
			return error;
		}
	else if (strEquals(sTag, ORBITALS_TAG))
		{
		if (error = CreateOrbitals(pCtx, pObj, OrbitDesc))
			return error;
		}
	else if (strEquals(sTag, TROJAN_TAG))
		{
		PushDebugStack(pCtx, TROJAN_TAG);

		Metric rOffset;
		CString sOffset;
		if (pObj->FindAttribute(OFFSET_ATTRIB, &sOffset))
			{
			DiceRange Offset;
			if (error = Offset.LoadFromXML(sOffset))
				return error;

			rOffset = mathDegreesToRadians(Offset.Roll());
			}
		else
			rOffset = (g_Pi / 3.0);

		COrbit TrojanOrbit(OrbitDesc.GetFocus(),
				OrbitDesc.GetSemiMajorAxis(),
				OrbitDesc.GetEccentricity(),
				OrbitDesc.GetRotation(),
				OrbitDesc.GetObjectAngle() + rOffset);

		for (int i = 0; i < pObj->GetContentElementCount(); i++)
			{
			CXMLElement *pItem = pObj->GetContentElement(i);

			if (error = CreateSystemObject(pCtx, pItem, TrojanOrbit))
				return error;
			}

		PopDebugStack(pCtx);
		}
	else if (strEquals(sTag, ANTI_TROJAN_TAG))
		{
		PushDebugStack(pCtx, TROJAN_TAG);

		Metric rOffset;
		CString sOffset;
		if (pObj->FindAttribute(OFFSET_ATTRIB, &sOffset))
			{
			DiceRange Offset;
			if (error = Offset.LoadFromXML(sOffset))
				return error;

			rOffset = mathDegreesToRadians(Offset.Roll());
			}
		else
			rOffset = (g_Pi / 3.0);

		COrbit TrojanOrbit(OrbitDesc.GetFocus(),
				OrbitDesc.GetSemiMajorAxis(),
				OrbitDesc.GetEccentricity(),
				OrbitDesc.GetRotation(),
				OrbitDesc.GetObjectAngle() - rOffset);

		for (int i = 0; i < pObj->GetContentElementCount(); i++)
			{
			CXMLElement *pItem = pObj->GetContentElement(i);

			if (error = CreateSystemObject(pCtx, pItem, TrojanOrbit))
				return error;
			}

		PopDebugStack(pCtx);
		}
	else if (strEquals(sTag, RANDOM_LOCATION_TAG))
		{
		if (error = CreateObjectAtRandomLocation(pCtx, pObj, OrbitDesc))
			return error;
		}
	else if (strEquals(sTag, PLACE_RANDOM_STATION_TAG))
		{
		int iCount = GetDiceCountFromAttribute(pObj->GetAttribute(COUNT_ATTRIB));
		for (int i = 0; i < iCount; i++)
			if (error = CreateRandomStationAtAppropriateLocation(pCtx, pObj))
				return error;
		}
	else if (strEquals(sTag, FILL_LOCATIONS_TAG) || strEquals(sTag, FILL_RANDOM_LOCATION_TAG))
		{
		if (error = DistributeStationsAtRandomLocations(pCtx, pObj, OrbitDesc))
			return error;
		}
	else if (strEquals(sTag, RANDOM_STATION_TAG))
		{
		if (error = CreateRandomStation(pCtx, pObj, OrbitDesc))
			return error;
		}
	else if (strEquals(sTag, SIBLINGS_TAG))
		{
		PushDebugStack(pCtx, SIBLINGS_TAG);

		if (error = CreateSiblings(pCtx, pObj, OrbitDesc))
			return error;

		PopDebugStack(pCtx);
		}
	else if (strEquals(sTag, OFFSET_TAG))
		{
		PushDebugStack(pCtx, OFFSET_TAG);

		if (error = CreateOffsetObjects(pCtx, pObj, OrbitDesc))
			return error;

		PopDebugStack(pCtx);
		}
	else if (strEquals(sTag, LABEL_TAG))
		{
		if (error = CreateLabel(pCtx, pObj, OrbitDesc))
			return error;
		}
	else if (strEquals(sTag, ADD_ATTRIBUTE_TAG))
		{
		if (error = AddAttribute(pCtx, pObj, OrbitDesc))
			return error;
		}
	else if (strEquals(sTag, ADD_TERRITORY_TAG))
		{
		if (error = AddTerritory(pCtx, pObj, OrbitDesc))
			return error;
		}
	else if (strEquals(sTag, LEVEL_TABLE_TAG))
		{
		PushDebugStack(pCtx, LEVEL_TABLE_TAG);

		if (error = CreateLevelTable(pCtx, pObj, OrbitDesc))
			return error;

		PopDebugStack(pCtx);
		}
	else if (strEquals(sTag, LOCATION_CRITERIA_TABLE_TAG))
		{
		PushDebugStack(pCtx, LOCATION_CRITERIA_TABLE_TAG);

		if (error = CreateLocationCriteriaTable(pCtx, pObj, OrbitDesc))
			return error;

		PopDebugStack(pCtx);
		}
	else if (strEquals(sTag, VARIANT_TAG))
		{
		bool bDeleteVariant = false;
		CString sVariant = pObj->GetAttribute(VARIANT_ATTRIB);
		if (!sVariant.IsBlank())
			{
			pCtx->Variants.Insert(sVariant);
			bDeleteVariant = true;
			}

		PushDebugStack(pCtx, strPatternSubst(CONSTLIT("Variant variant=%s"), sVariant));

		for (int i = 0; i < pObj->GetContentElementCount(); i++)
			{
			CXMLElement *pItem = pObj->GetContentElement(i);

			if (error = CreateSystemObject(pCtx, pItem, OrbitDesc))
				return error;
			}

		PopDebugStack(pCtx);

		if (bDeleteVariant)
			pCtx->Variants.Delete(pCtx->Variants.GetCount() - 1);
		}
	else if (strEquals(sTag, VARIANTS_TAG)
			|| strEquals(sTag, VARIANTS_TABLE_TAG))
		{
		PushDebugStack(pCtx, VARIANTS_TABLE_TAG);

		if (error = CreateVariantsTable(pCtx, pObj, OrbitDesc))
			return error;

		PopDebugStack(pCtx);
		}
	else if (strEquals(sTag, PRIMARY_TAG))
		{
		PushDebugStack(pCtx, PRIMARY_TAG);

		for (int i = 0; i < pObj->GetContentElementCount(); i++)
			{
			CXMLElement *pItem = pObj->GetContentElement(i);

			if (error = CreateSystemObject(pCtx, pItem, OrbitDesc))
				return error;
			}

		PopDebugStack(pCtx);
		}
	else if (strEquals(sTag, STARGATE_TAG))
		{
		PushDebugStack(pCtx, STARGATE_TAG);

		//	If we've got an offset, change the orbit

		COrbit NewOrbit;
		const COrbit *pOrbitDesc = ComputeOffsetOrbit(pObj, OrbitDesc, &NewOrbit);

		//	Create

		if (error = CreateStargate(pCtx, pObj, *pOrbitDesc))
			return error;

		PopDebugStack(pCtx);
		}
	else if (strEquals(sTag, SHIP_TAG))
		{
		int iCount = GetDiceCountFromAttribute(pObj->GetAttribute(COUNT_ATTRIB));

		PushDebugStack(pCtx, SHIP_TAG);

		//	Load generator

		SDesignLoadCtx Ctx;

		IShipGenerator *pGenerator;
		if (error = IShipGenerator::CreateFromXML(Ctx, pObj, &pGenerator))
			{
			pCtx->sError = strPatternSubst(CONSTLIT("Unable to create ship: %s"), Ctx.sError);
			return error;
			}

		if (error = pGenerator->OnDesignLoadComplete(Ctx))
			{
			pCtx->sError = strPatternSubst(CONSTLIT("Unable to create ship: %s"), Ctx.sError);
			return error;
			}

		SShipCreateCtx CreateCtx;
		CreateCtx.pSystem = pCtx->pSystem;
		CreateCtx.vPos = OrbitDesc.GetObjectPos();
		if (iCount > 0)
			CreateCtx.PosSpread = DiceRange(6, 2, 1);

		if (error = pGenerator->ValidateForRandomEncounter())
			{
			pCtx->sError = CONSTLIT("Unable to create ship: sovereign must be specified");
			return error;
			}

		//	Create the ships

		for (int i = 0; i < iCount; i++)
			pGenerator->CreateShips(CreateCtx);

		if (CreateCtx.Result.GetCount() > 0)
			pCtx->dwLastObjID = CreateCtx.Result.GetObj(0)->GetID();

		PopDebugStack(pCtx);
		}
	else if (strEquals(sTag, PARTICLES_TAG))
		{
		CParticleEffect *pSObj;

		PushDebugStack(pCtx, PARTICLES_TAG);

		if (error = pCtx->pSystem->CreateParticles(pObj, OrbitDesc, &pSObj))
			{
			pCtx->sError = CONSTLIT("Unable to create particle field");
			return error;
			}

		pCtx->dwLastObjID = pSObj->GetID();
		PopDebugStack(pCtx);
		}
	else if (strEquals(sTag, MARKER_TAG))
		{
		CMarker *pSObj;

		PushDebugStack(pCtx, MARKER_TAG);

		if (error = pCtx->pSystem->CreateMarker(pObj, OrbitDesc, &pSObj))
			{
			pCtx->sError = CONSTLIT("Unable to create marker");
			return error;
			}

		pCtx->dwLastObjID = pSObj->GetID();
		PopDebugStack(pCtx);
		}
	else if (strEquals(sTag, SPACE_ENVIRONMENT_TAG))
		{
		PushDebugStack(pCtx, SPACE_ENVIRONMENT_TAG);

		if (error = CreateSpaceEnvironment(pCtx, pObj, OrbitDesc))
			return error;

		PopDebugStack(pCtx);
		}
	else if (strEquals(sTag, ORBITAL_DISTRIBUTION_TAG))
		{
		PushDebugStack(pCtx, ORBITAL_DISTRIBUTION_TAG);

		if (error = CreateArcDistribution(pCtx, pObj, OrbitDesc))
			return error;

		PopDebugStack(pCtx);
		}
	else if (strEquals(sTag, CODE_TAG))
		{
		PushDebugStack(pCtx, CODE_TAG);

		//	Parse the code

		ICCItem *pCode = g_pUniverse->GetCC().Link(pObj->GetContentText(0), 0, NULL);
		if (pCode->IsError())
			{
			pCtx->sError = strPatternSubst(CONSTLIT("<Code>: %s"), pCode->GetStringValue());
			pCode->Discard(&g_pUniverse->GetCC());
			return ERR_FAIL;
			}

		//	Execute it

		CString sError;
		if (error = pCtx->pSystem->GetType()->FireSystemCreateCode(*pCtx, pCode, OrbitDesc, &sError))
			{
			pCtx->sError = strPatternSubst(CONSTLIT("<Code>: %s"), sError);
			pCode->Discard(&g_pUniverse->GetCC());
			return error;
			}

		//	Done

		pCode->Discard(&g_pUniverse->GetCC());
		PopDebugStack(pCtx);
		}
	else if (strEquals(sTag, NULL_TAG))
		NULL;
	else
		{
		pCtx->sError = strPatternSubst(CONSTLIT("Unknown object tag: %s"), sTag);
		return ERR_FAIL;
		}

	STATION_PLACEMENT_OUTPUT(strPatternSubst(CONSTLIT("</%s>\n"), sTag).GetASCIIZPointer());

	return NOERROR;
	}

ALERROR CreateVariantsTable (SSystemCreateCtx *pCtx, CXMLElement *pDesc, const COrbit &OrbitDesc)

//	CreateVariantsTable
//
//	Parses a LevelTable

	{
	ALERROR error;
	int i, j;

	//	Loop over all elements and return the first one that matches
	//	all the conditions

	CXMLElement *pFound = NULL;
	for (i = 0; i < pDesc->GetContentElementCount(); i++)
		{
		CXMLElement *pVariant = pDesc->GetContentElement(i);

		//	Check for variant attribute

		CString sVariant;
		if (pVariant->FindAttribute(VARIANT_ATTRIB, &sVariant))
			{
			if (!pCtx->pTopologyNode->HasVariantLabel(sVariant)
					&& !pCtx->Variants.Find(sVariant))
				continue;
			}

		//	Check for radius

		int iMaxRadius;
		if (pVariant->FindAttributeInteger(MAX_RADIUS_ATTRIB, &iMaxRadius))
			{
			Metric rMaxRadius = LIGHT_SECOND * iMaxRadius;
			if (OrbitDesc.GetSemiMajorAxis() > rMaxRadius)
				continue;
			}

		//	Check for attributes

		CString sCriteria;
		if (pVariant->FindAttribute(VARIANT_LOCATION_CRITERIA_ATTRIB, &sCriteria))
			{
			if (!strEquals(sCriteria, MATCH_ALL))
				{
				CAttributeCriteria Criteria;
				if (error = Criteria.Parse(sCriteria, 0, &pCtx->sError))
					return error;

				bool bMatched = true;
				for (j = 0; j < Criteria.GetCount(); j++)
					{
					bool bRequired;
					const CString &sAttrib = Criteria.GetAttribAndRequired(j, &bRequired);

					int iAdj = ComputeLocationWeight(pCtx, 
							pCtx->sLocationAttribs,
							OrbitDesc.GetObjectPos(),
							sAttrib,
							(bRequired ? CAttributeCriteria::matchRequired : CAttributeCriteria::matchExcluded));

					if (iAdj == 0)
						{
						bMatched = false;
						break;
						}
					}

				if (!bMatched)
					continue;
				}
			}

		//	If we get this far, then this location matched the variant

		pFound = pVariant;
		break;
		}

	//	If not found, warning (but no error because this happens at run-time)

	if (pFound == NULL)
		{
		if (g_pUniverse->InDebugMode())
			{
			CString sOutput = CONSTLIT("Warning: no match for variant table:");

			for (i = 0; i < pDesc->GetContentElementCount(); i++)
				{
				CXMLElement *pVariant = pDesc->GetContentElement(i);

				CString sValue;
				if (pVariant->FindAttribute(VARIANT_ATTRIB, &sValue))
					sOutput = strPatternSubst(CONSTLIT("%s\n\tvariant: %s"), sOutput, sValue);
				else if (pVariant->FindAttribute(MAX_RADIUS_ATTRIB, &sValue))
					sOutput = strPatternSubst(CONSTLIT("%s\n\tmaxRadius: %s"), sOutput, sValue);
				else if (pVariant->FindAttribute(VARIANT_LOCATION_CRITERIA_ATTRIB, &sValue))
					sOutput = strPatternSubst(CONSTLIT("%s\n\tvariantLocationCriteria: %s"), sOutput, sValue);
				else
					sOutput = strPatternSubst(CONSTLIT("%s\n\tdefault"), sOutput);
				}

			CString sActive;
			for (i = 0; i < pCtx->Variants.GetCount(); i++)
				if (i == 0)
					sActive = pCtx->Variants[i];
				else
					sActive = strPatternSubst(CONSTLIT("%s, %s"), sActive, pCtx->Variants[i]);

			sOutput = strPatternSubst(CONSTLIT("%s\n\tActive variants: %s"), sOutput, sActive);

			kernelDebugLogMessage((LPSTR)sOutput);
			DumpDebugStack(pCtx);
			}

		return NOERROR;
		}

	//	Create

	if (error = CreateSystemObject(pCtx, pFound, OrbitDesc))
		return error;
	
	return NOERROR;
	}

bool CheckForOverlap (SSystemCreateCtx *pCtx, const CVector &vPos)

//	CheckForOverlap
//
//	Returns TRUE if the given position overlaps an existing object

	{
	int i;

	CVector vUR(vPos.GetX() + OVERLAP_DIST, vPos.GetY() + OVERLAP_DIST);
	CVector vLL(vPos.GetX() - OVERLAP_DIST, vPos.GetY() - OVERLAP_DIST);

	for (i = 0; i < pCtx->pSystem->GetObjectCount(); i++)
		{
		CSpaceObject *pObj = pCtx->pSystem->GetObject(i);

		if (pObj && pObj->InBox(vUR, vLL))
			return true;
		}

	return false;
	}

void DumpDebugStack (SSystemCreateCtx *pCtx)

//	DumpDebugStack
//
//	Output the current debug stack

	{
	int i;

	if (g_pUniverse->InDebugMode())
		{
		CString sStack;

		for (i = 0; i < pCtx->DebugStack.GetCount(); i++)
			{
			if (i == 0)
				sStack = strPatternSubst(CONSTLIT("Create system stack:\n\t 1: %s"), pCtx->DebugStack[i]);
			else
				sStack = strPatternSubst(CONSTLIT("%s\n\t%2d: %s"), sStack, i+1, pCtx->DebugStack[i]);
			}

		kernelDebugLogMessage((LPSTR)sStack);
		}
	}

ALERROR GenerateAngles (SSystemCreateCtx *pCtx, const CString &sAngle, int iCount, Metric *pAngles)

//	GenerateAngles
//
//	Generates random angles based on the angle type

	{
	ALERROR error;
	int i;

	//	Separate value after colon

	char *pString = sAngle.GetASCIIZPointer();
	char *pColon = pString;
	while (pColon && *pColon != ':' && *pColon != '\0')
		pColon++;

	CString sKeyword;
	CString sValue;
	if (pColon && *pColon == ':')
		{
		sKeyword = CString(pString, pColon - pString);
		sValue = CString(pColon+1);
		}
	else
		sKeyword = sAngle;

	//	Generate list of angles

	if (strEquals(sKeyword, RANDOM_ANGLE))
		{
		for (i = 0; i < iCount; i++)
			pAngles[i] = mathDegreesToRadians(mathRandom(0,3599) / 10.0);
		}
	else if (strEquals(sKeyword, MIN_SEPARATION_ANGLE))
		{
		DiceRange SepRange;
		if (error = SepRange.LoadFromXML(sValue))
			{
			pCtx->sError = CONSTLIT("Invalid minSeparation range in orbitals");
			return error;
			}

		Metric rMinSep = mathDegreesToRadians(SepRange.Roll());

		for (i = 0; i < iCount; i++)
			{
			bool bAngleIsOK;
			int iTries = 20;

			do 
				{
				pAngles[i] = mathDegreesToRadians(mathRandom(0,3599) / 10.0);
				bAngleIsOK = true;

				for (int k = 0; k < i; k++)
					{
					if (Absolute(pAngles[i] - pAngles[k]) < rMinSep
							|| pAngles[i] + (2.0 * g_Pi) - pAngles[k] < rMinSep
							|| pAngles[k] + (2.0 * g_Pi) - pAngles[i] < rMinSep)
						{
						bAngleIsOK = false;
						break;
						}
					}
				}
			while (iTries-- > 0 && !bAngleIsOK);
			}
		}
	else if (strEquals(sKeyword, EQUIDISTANT_ANGLE))
		{
		DiceRange OffsetRange(0, 0, 0);
		if (!sValue.IsBlank())
			{
			if (error = OffsetRange.LoadFromXML(sValue))
				{
				pCtx->sError = CONSTLIT("Invalid equidistant range in orbitals");
				return error;
				}
			}

		int iStart = mathRandom(0, 3599);
		int iSeparation = 3600 / iCount;

		for (i = 0; i < iCount; i++)
			pAngles[i] = mathDegreesToRadians(OffsetRange.Roll() + ((iStart + iSeparation * i) % 3600) / 10.0);
		}
	else if (strEquals(sKeyword, INCREMENTING_ANGLE))
		{
		DiceRange IncRange;
		if (error = IncRange.LoadFromXML(sValue))
			{
			pCtx->sError = CONSTLIT("Invalid increment range in orbitals");
			return error;
			}

		int iAngle = mathRandom(0, 359);

		for (i = 0; i < iCount; i++)
			{
			pAngles[i] = mathDegreesToRadians(iAngle % 360);
			iAngle += IncRange.Roll();
			}
		}
	else
		{
		DiceRange AngleRange;
		if (error = AngleRange.LoadFromXML(sAngle))
			{
			pCtx->sError = CONSTLIT("Invalid angle in orbitals");
			return error;
			}

		for (i = 0; i < iCount; i++)
			pAngles[i] = mathDegreesToRadians(AngleRange.Roll());
		}

	return NOERROR;
	}

void GenerateRandomPosition (SSystemCreateCtx *pCtx, COrbit *retOrbit)

//	GenerateRandomPosition
//
//	Generates a random position

	{
	int i;

	//	Make a list of all the objects that we could orbit.

	TArray<CSpaceObject *> CenterObj;
	for (i = 0; i < pCtx->pSystem->GetObjectCount(); i++)
		{
		CSpaceObject *pObj = pCtx->pSystem->GetObject(i);
		if (pObj
				&& pObj->GetScale() == scaleWorld)
			{
			CStationType *pType = CStationType::AsType(pObj->GetType());
			if (pType 
					&& pType->GetSize() >= 1000
					&& !pObj->HasAttribute(CONSTLIT("asteroid")))
				CenterObj.Insert(pObj);
			}
		}

	//	Keep trying to find random positions that are not too close to any other
	//	objects.

	Metric rMinActive2 = (50 * 50) * (LIGHT_SECOND * LIGHT_SECOND);
	COrbit NewOrbit;
	int iTries = 100;
	bool bFound = false;
	while (iTries-- > 0 && !bFound)
		{
		//	If we have no objects then just pick a random spot around the
		//	central star.

		CSpaceObject *pCenter = NULL;
		if (CenterObj.GetCount() == 0)
			NewOrbit = COrbit(CVector(), LIGHT_SECOND * mathRandom(30, 800));

		//	Otherwise, pick a spot orbiting a random planet

		else
			{
			pCenter = CenterObj[mathRandom(0, CenterObj.GetCount() - 1)];
			NewOrbit = COrbit(pCenter->GetPos(), LIGHT_SECOND * mathRandom(15, 50));
			}

		//	Check to see if the chosen position is too close to anything.

		CVector vTry = NewOrbit.GetObjectPos();
		bFound = true;
		for (i = 0; i < pCtx->pSystem->GetObjectCount(); i++)
			{
			CSpaceObject *pObj = pCtx->pSystem->GetObject(i);
			if (pObj 
					&& pObj != pCenter
					&& !pObj->IsInactive()
					&& !pObj->IsVirtual()
					&& pObj->CanAttack())
				{
				Metric rDist2 = (pObj->GetPos() - vTry).Length2();
				if (rDist2 < rMinActive2)
					{
					bFound = false;
					break;
					}
				}
			}
		}

	//	Done

	*retOrbit = NewOrbit;
	}

ALERROR GenerateRandomStationTable (SSystemCreateCtx *pCtx,
									const CString &sCriteria,
									const CString &sLocationAttribs,
									const CVector &vPos,
									bool bIncludeAll,
									TArray<CStationTableCache::SEntry> **retpTable,
									bool *retbAddToCache)

//	GenerateRandomStationTable
//
//	Returns an array of station type matching the given criteria and sets the
//	temp value to the chance.

	{
	ALERROR error;
	int i, j;

	//	Loop over all station types

	int iLevel = pCtx->pSystem->GetLevel();
	int iCount = g_pUniverse->GetStationTypeCount();

	//	Initialize the table
	//
	//	If we're including all it usually means that we're by-passing the normal
	//	encounter probabilities and going straight by station criteria.
	//	(We do this, e.g., to create random asteroids of appropriate type.)

	if (bIncludeAll)
		{
		//	If we're including all, then we ignore the levelFrequency property
		//	of a type.

		for (i = 0; i < iCount; i++)
			g_pUniverse->GetStationType(i)->SetTempChance(1000);
		}
	else
		{
		for (i = 0; i < iCount; i++)
			{
			CStationType *pType = g_pUniverse->GetStationType(i);
			pType->SetTempChance((1000 / ftCommon) * pType->GetFrequencyForSystem(pCtx->pSystem));
			}
		}

	//	Loop over each part of the criteria and refine the table

	if (!strEquals(sCriteria, MATCH_ALL))
		{
		//	Parse station criteria if we've got it.

		CAttributeCriteria StationCriteria;
		if (error = StationCriteria.Parse(sCriteria, 0, &pCtx->sError))
			return error;

		for (i = 0; i < iCount; i++)
			{
			CStationType *pType = g_pUniverse->GetStationType(i);
			if (pType->GetTempChance())
				{
				for (j = 0; j < StationCriteria.GetCount(); j++)
					{
					DWORD dwMatchStrength;
					const CString &sAttrib = StationCriteria.GetAttribAndWeight(j, &dwMatchStrength);

					int iAdj = ComputeStationWeight(pCtx, pType, sAttrib, dwMatchStrength);

					if (iAdj > 0)
						pType->SetTempChance((pType->GetTempChance() * iAdj) / 1000);
					else
						{
						pType->SetTempChance(0);
						break;
						}
					}
				}
			}
		}

	//	Loop over each station type and adjust for the location that
	//	we want to create the station at

	if (!strEquals(sLocationAttribs, MATCH_ALL))
		{
		for (i = 0; i < iCount; i++)
			{
			CStationType *pType = g_pUniverse->GetStationType(i);
			if (pType->GetTempChance())
				{
				if (!strEquals(pType->GetLocationCriteria(), MATCH_ALL))
					{
					CAttributeCriteria Criteria;
					if (error = Criteria.Parse(pType->GetLocationCriteria(), 0, &pCtx->sError))
						{
						pCtx->sError = strPatternSubst(CONSTLIT("StationType %x: Invalid locationCriteria"), pType->GetUNID());
						return error;
						}

					for (j = 0; j < Criteria.GetCount(); j++)
						{
						DWORD dwMatchStrength;
						const CString &sAttrib = Criteria.GetAttribAndWeight(j, &dwMatchStrength);

						int iAdj = ComputeLocationWeight(pCtx,
								sLocationAttribs,
								vPos,
								sAttrib,
								dwMatchStrength);
						pType->SetTempChance((pType->GetTempChance() * iAdj) / 1000);
						}
					}
				}
			}
		}

	//	Check to see if any of the selected stations has priority (because of node
	//	minimums). If so, then we need to zero out any other stations.

	bool bPrioritizeRequiredEncounters = false;
	if (pCtx->pTopologyNode)
		{
		for (i = 0; i < iCount; i++)
			{
			CStationType *pType = g_pUniverse->GetStationType(i);

			//	If we need a minimum number of stations in this node, then we
			//	prioritize these types.

			if (pType->GetTempChance() > 0
					&& pType->GetEncounterRequired(pCtx->pTopologyNode) > 0)
				{
				bPrioritizeRequiredEncounters = true;
				break;
				}
			}

		//	If we're prioritizing required encounters, then zero-out any probabilities
		//	for non-required types

		if (bPrioritizeRequiredEncounters)
			{
			for (i = 0; i < iCount; i++)
				{
				CStationType *pType = g_pUniverse->GetStationType(i);
				if (pType->GetEncounterRequired(pCtx->pTopologyNode) == 0)
					pType->SetTempChance(0);
				}
			}
		}

	//	Now that we've initialized all the types, return the ones with a non-zero
	//	chance.

	TArray<CStationTableCache::SEntry> *pTable = new TArray<CStationTableCache::SEntry>;
	for (i = 0; i < iCount; i++)
		{
		CStationType *pType = g_pUniverse->GetStationType(i);
		if (pType->GetTempChance())
			{
			CStationTableCache::SEntry *pEntry = pTable->Insert();
			pEntry->pType = pType;
			pEntry->iChance = pType->GetTempChance();
			}
		}

	//	Done

	*retpTable = pTable;
	*retbAddToCache = !bPrioritizeRequiredEncounters;

	return NOERROR;
	}

ALERROR GetLocationCriteria (SSystemCreateCtx *pCtx, CXMLElement *pDesc, SLocationCriteria *retCriteria)

//	GetLocationCriteria
//
//	Parses location criteria:
//
//	locationCriteria=
//	maxDist=
//	minDist=

	{
	ALERROR error;

	if (error = retCriteria->AttribCriteria.Parse(pDesc->GetAttribute(LOCATION_CRITERIA_ATTRIB), 0, &pCtx->sError))
		return error;

	Metric rScale = GetScale(pDesc);
	retCriteria->rMaxDist = pDesc->GetAttributeIntegerBounded(MAX_DIST_ATTRIB, 0, -1, 0) * rScale;
	retCriteria->rMinDist = pDesc->GetAttributeIntegerBounded(MIN_DIST_ATTRIB, 0, -1, 0) * rScale;

	return NOERROR;
	}

bool IsExclusionZoneClear (SSystemCreateCtx *pCtx, const CVector &vPos, Metric rRadius)

//	IsExclusionZoneClear
//
//	Returns TRUE if the area around vPos is free of objects out to rRadius

	{
	int j;
	Metric rExclusionDist2 = rRadius * rRadius;

	//	See if we are close to any objects

	for (j = 0; j < pCtx->pSystem->GetObjectCount(); j++)
		{
		CSpaceObject *pObj = pCtx->pSystem->GetObject(j);

		if (pObj 
				&& pObj->GetScale() == scaleStructure
				&& pObj->CanAttack())
			{
			//	Compute the distance to this obj

			CVector vDist = vPos - pObj->GetPos();
			Metric rDist2 = vDist.Length2();

			//	If we're too close to an enemy then zone is not clear

			if (rDist2 < rExclusionDist2)
				return false;
			}
		}

	//	See if we are close to any labels

	TArray<int> EmptyLocations;
	pCtx->pSystem->GetEmptyLocations(&EmptyLocations);
	for (j = 0; j < EmptyLocations.GetCount(); j++)
		{
		CLocationDef *pLoc = pCtx->pSystem->GetLocation(EmptyLocations[j]);
		CVector vDist = vPos - pLoc->GetOrbit().GetObjectPos();
		Metric rDist2 = vDist.Length2();

		if (rDist2 < rExclusionDist2)
			return false;
		}

	//	If we get this far, then zone is clear

	return true;
	}

bool IsExclusionZoneClear (SSystemCreateCtx *pCtx, const CVector &vPos, CStationType *pType)

//	IsExclusionZoneClear
//
//	Returns TRUE if the area around vPos is free of enemies of pType. The exclusion zone is
//	defined by the greatest separation required by pType and any enemies of pType

	{
	int j;
	CSovereign *pSovereign = pType->GetControllingSovereign();
	Metric rExclusionDist2 = pType->GetEnemyExclusionRadius();
	rExclusionDist2 *= rExclusionDist2;

	for (j = 0; j < pCtx->pSystem->GetObjectCount(); j++)
		{
		CSpaceObject *pObj = pCtx->pSystem->GetObject(j);

		if (pObj 
				&& pObj->GetScale() == scaleStructure
				&& pObj->GetSovereign()
				&& pObj->GetSovereign()->IsEnemy(pSovereign))
			{
			//	Compute the distance to this obj

			CVector vDist = vPos - pObj->GetPos();
			Metric rDist2 = vDist.Length2();

			//	Compute the exclusion radius of the object

			CStationType *pObjType = pObj->GetEncounterInfo();
			Metric rObjExclusionDist2 = (pObjType ? pObjType->GetEnemyExclusionRadius() : 30.0 * LIGHT_SECOND);
			rObjExclusionDist2 *= rObjExclusionDist2;

			//	If we're too close to an enemy then zone is not clear

			if (rDist2 < Max(rExclusionDist2, rObjExclusionDist2))
				return false;
			}
		}

	//	If we get this far, then zone is clear

	return true;
	}

ALERROR ModifyCreatedStation (SSystemCreateCtx *pCtx, CStation *pStation, CXMLElement *pDesc, const COrbit &OrbitDesc)

//	ModifyCreatedStation
//
//	Modifies a newly created station based on commands in the original element
//	descriptor.

	{
	ALERROR error;
	int i;

	//	Set the name of the station, if specified by the system

	CString sName = pDesc->GetAttribute(NAME_ATTRIB);
	if (!sName.IsBlank())
		pStation->SetName(sName, LoadNameFlags(pDesc));

	//	If we want to show the orbit for this station, set the orbit desc

	if (pDesc->GetAttributeBool(SHOW_ORBIT_ATTRIB)
			&& !OrbitDesc.IsNull())
		pStation->SetMapOrbit(OrbitDesc);

	//	Set the image variant

	int iVariant;
	if (pDesc->FindAttributeInteger(IMAGE_VARIANT_ATTRIB, &iVariant))
		pStation->SetImageVariant(iVariant);

	//	If this station is a gate entry-point, then add it to
	//	the table in the system.

	CString sEntryPoint = pDesc->GetAttribute(OBJ_NAME_ATTRIB);
	if (!sEntryPoint.IsBlank())
		pCtx->pSystem->NameObject(sEntryPoint, pStation);

	//	If we don't want to show a map label

	if (pDesc->GetAttributeBool(NO_MAP_LABEL_ATTRIB))
		pStation->SetNoMapLabel();

	//	No reinforcements

	if (pDesc->GetAttributeBool(NO_CONSTRUCTION_ATTRIB))
		pStation->SetNoConstruction();

	if (pDesc->GetAttributeBool(NO_REINFORCEMENTS_ATTRIB))
		pStation->SetNoReinforcements();

	//	Create additional satellites

	CXMLElement *pSatellites = pDesc->GetContentElementByTag(SATELLITES_TAG);
	if (pSatellites)
		{
		for (i = 0; i < pSatellites->GetContentElementCount(); i++)
			{
			CXMLElement *pSatDesc = pSatellites->GetContentElement(i);
			if (error = CreateSystemObject(pCtx, pSatDesc, OrbitDesc))
				return error;
			}
		}

	//	See if we need to create additional ships

	CXMLElement *pShips = pDesc->GetContentElementByTag(SHIPS_TAG);
	if (pShips)
		{
		if (error = CreateShipsForStation(pStation, pShips))
			{
			pCtx->sError = CONSTLIT("Unable to create ships for station");
			return error;
			}
		}

	//	If we have an OnCreate block then add it to the set of deferred code

	CXMLElement *pOnCreate = pDesc->GetContentElementByTag(ON_CREATE_EVENT);
	if (pOnCreate)
		pCtx->Events.AddDeferredEvent(pStation, pCtx->pExtension, pOnCreate);

	//	Done

	return NOERROR;
	}

void RemoveOverlappingLabels (SSystemCreateCtx *pCtx, Metric rMinDistance)

//	RemoveOverlappingLabels
//
//	Removes labels that are too close together

	{
	pCtx->pSystem->BlockOverlappingLocations();
	}

//	CSystem methods

ALERROR CSystem::CreateEmpty (CUniverse *pUniv, CTopologyNode *pTopology, CSystem **retpSystem)

//	CreateEmpty
//
//	Creates an empty system

	{
	CSystem *pSystem;

	pSystem = new CSystem(pUniv, pTopology);
	if (pSystem == NULL)
		return ERR_MEMORY;

	//	Initialize

	pSystem->m_iTick = pUniv->GetTicks();

	//	Create the background star field

	pSystem->ResetStarField();

	//	Set the name

	if (pTopology)
		pSystem->m_sName = pTopology->GetSystemName();
	else
		pSystem->m_sName = CONSTLIT("George's Star");

	//	Set random encounter timer

	pSystem->m_pType = NULL;
	pSystem->m_iNextEncounter = pSystem->m_iTick + mathRandom(500, 2500);
	pSystem->m_fEncounterTableValid = false;

	//	Done

	*retpSystem = pSystem;

	return NOERROR;
	}

ALERROR CSystem::CreateFromXML (CUniverse *pUniv, 
								CSystemType *pType, 
								CTopologyNode *pTopology, 
								CSystem **retpSystem,
								CString *retsError,
								CSystemCreateStats *pStats)

//	CreateFromXML
//
//	Creates a new system from description

	{
	ALERROR error;
	int i, j;

#ifdef DEBUG_STATION_PLACEMENT
	{
	char szBuffer[1024];
	wsprintf(szBuffer, "CSystem::CreateFromXML: %s\n", pTopology->GetSystemName().GetASCIIZPointer());
	::OutputDebugString(szBuffer);
	}
#endif

	START_STRESS_TEST;

	CSystem *pSystem;
	if (error = CreateEmpty(pUniv, pTopology, &pSystem))
		{
		if (retsError)
			*retsError = CONSTLIT("Unable to create empty system");
		return error;
		}

	//	System is being created

	pSystem->m_fInCreate = true;

	//	Load some data

	pSystem->m_pType = pType;
	pSystem->m_fNoRandomEncounters = !pType->HasRandomEncounters();
	pSystem->m_fUseDefaultTerritories = true;

	//	Set scales

	pSystem->m_rKlicksPerPixel = pType->GetSpaceScale();
	pSystem->m_rTimeScale = pType->GetTimeScale();

	//	Get a pointer to the tables element (may be NULL)

	CXMLElement *pTables = pType->GetLocalSystemTables();

	//	Look for the outer-most group tag

	CXMLElement *pPrimary = pType->GetDesc();
	if (pPrimary == NULL)
		{
		if (retsError)
			*retsError = CONSTLIT("Cannot find root <SystemGroup> element");
		return ERR_FAIL;
		}

	//	Store the current system. We need this so that any OnCreate code can
	//	get the right system.

	pUniv->SetCurrentSystem(pSystem);

	//	Create the group

	SSystemCreateCtx Ctx;
	Ctx.pExtension = pType->GetExtension();
	Ctx.pTopologyNode = pTopology;
	Ctx.pSystem = pSystem;
	Ctx.pStats = pStats;

	//	Add local tables

	if (pTables)
		Ctx.LocalTables.Insert(pTables);

	//	Start the debug stack

	PushDebugStack(&Ctx, strPatternSubst(CONSTLIT("SystemType nodeID=%s unid=%x"), pTopology->GetID(), pType->GetUNID()));

	//	Create

	try
		{
		error = CreateSystemObject(&Ctx,
				pPrimary,
				COrbit());
		}
	catch (...)
		{
		Ctx.sError = CONSTLIT("Crash in CreateSystemObject.");
		error = ERR_FAIL;
		}

	if (error)
		{
		if (retsError)
			*retsError = Ctx.sError;
		kernelDebugLogMessage("Unable to create system: %s", Ctx.sError);
		DumpDebugStack(&Ctx);
		return error;
		}

	//	Invoke OnCreate event

	if (error = pType->FireOnCreate(Ctx, &Ctx.sError))
		kernelDebugLogMessage("%s", Ctx.sError);

	//	Now invoke OnGlobalSystemCreated

	g_pUniverse->FireOnGlobalSystemCreated(Ctx);

	//	Make sure this system has all the required objects that we want

	if (pTopology)
		{
		for (i = 0; i < g_pUniverse->GetStationTypeCount(); i++)
			{
			CStationType *pType = g_pUniverse->GetStationType(i);
			int iToCreate;
			if (iToCreate = pType->GetEncounterRequired(pTopology))
				{
				for (j = 0; j < iToCreate; j++)
					{
					//	Look for an appropriate location to create the station at.

					COrbit OrbitDesc;
					CString sLocationAttribs;
					int iLocation;
					if (error = ChooseRandomLocation(&Ctx, 
							pType->GetLocationCriteria(), 
							NULL,
							&OrbitDesc, 
							&sLocationAttribs,
							&iLocation))
						{
						//	If we couldn't find an appropriate location then we try 
						//	picking a random location in the system that is away from
						//	other stations.

						if (error == ERR_NOTFOUND)
							{
							GenerateRandomPosition(&Ctx, &OrbitDesc);
							iLocation = -1;
							}
						else
							return error;
						}

					//	Remember saved last obj

					DWORD dwSavedLastObjID = Ctx.dwLastObjID;
					Ctx.dwLastObjID = 0;

					//	Create the station at the location

					SObjCreateCtx CreateCtx;
					CreateCtx.vPos = OrbitDesc.GetObjectPos();
					CreateCtx.pLoc = (iLocation != -1 ? Ctx.pSystem->GetLocation(iLocation) : NULL);
					CreateCtx.pOrbit = &OrbitDesc;
					CreateCtx.bCreateSatellites = true;

					if (error = pSystem->CreateStation(&Ctx, pType, CreateCtx))
						return error;

					//	Remember that we filled this location

					if (iLocation != -1)
						pSystem->SetLocationObjID(iLocation, Ctx.dwLastObjID);
					Ctx.dwLastObjID = dwSavedLastObjID;
					}
				}
			}
		}

#ifdef DEBUG_STATION_TABLE_CACHE
	kernelDebugLogMessage("Station table cache hit rate: %3d%%  size: %d", Ctx.StationTables.GetCacheHitRate(), Ctx.StationTables.GetCacheSize());
#endif

	STOP_STRESS_TEST;

	//	Done creating

	pSystem->m_fInCreate = false;

	//	Arrange all map labels so that they don't overlap

	pSystem->ComputeMapLabels();
	pSystem->ComputeStars();

	//	Call each object and tell it that the system has been
	//	created.

	for (i = 0; i < pSystem->m_DeferredOnCreate.GetCount(); i++)
		{
		CSpaceObject *pObj = pSystem->m_DeferredOnCreate.GetObj(i);

		if (pObj && !pObj->IsDestroyed())
			pObj->OnSystemCreated();
		}

	//	No need for this list anymore.

	pSystem->m_DeferredOnCreate.CleanUp();

	//	Fire any deferred OnCreate events

	if (error = Ctx.Events.FireDeferredEvent(ON_CREATE_EVENT, &Ctx.sError))
		kernelDebugLogMessage("Deferred OnCreate: %s", Ctx.sError);

	//	Make sure this system has all the stargates that it needs

	for (i = 0; i < pTopology->GetStargateCount(); i++)
		if (pSystem->GetNamedObject(pTopology->GetStargate(i)) == NULL)
			{
			//	Log, but for backwards compatibility with <1.1 extensions continue running.
			kernelDebugLogMessage("Warning: Unable to find required stargate: %s", pTopology->GetStargate(i));
			DumpDebugStack(&Ctx);
			}

	//	Done

	*retpSystem = pSystem;

	return NOERROR;
	}

ALERROR CSystem::CreateLookup (SSystemCreateCtx *pCtx, const CString &sTable, const COrbit &OrbitDesc, CXMLElement *pSubTables)

//	CreateLookup
//
//	Creates objects from a table lookup.

	{
	ALERROR error;
	int i;

	PushDebugStack(pCtx, strPatternSubst(CONSTLIT("Lookup table=%s"), sTable));

	//	Keep track of the current extension, because we may change it below

	CExtension *pOldExtension = pCtx->pExtension;

	//	Find the table

	CXMLElement *pTableDesc = NULL;
	for (i = 0; i < pCtx->LocalTables.GetCount(); i++)
		{
		pTableDesc = pCtx->LocalTables[i]->GetContentElementByTag(sTable);
		if (pTableDesc)
			break;
		}

	//	If not found, we look in all global tables

	if (pTableDesc == NULL)
		{
		CSystemTable *pTable;
		pTableDesc = g_pUniverse->FindSystemFragment(sTable, &pTable);
		if (pTableDesc == NULL)
			{
			pCtx->sError = strPatternSubst(CONSTLIT("Unable to find table in <Lookup>: %s"), sTable);
			return ERR_FAIL;
			}

		//	Set the extension

		pCtx->pExtension = pTable->GetExtension();
		}

	//	Add any tables that we define inside the lookup (we use these for
	//	variable substitution).

	if (pSubTables)
		pCtx->LocalTables.Insert(pSubTables, 0);

	//	Create all the objects

	for (i = 0; i < pTableDesc->GetContentElementCount(); i++)
		{
		CXMLElement *pResult = pTableDesc->GetContentElement(i);

		//	Recurse

		if (error = CreateSystemObject(pCtx, pResult, OrbitDesc))
			return error;
		}

	//	Restore

	pCtx->LocalTables.Delete(0);
	pCtx->pExtension = pOldExtension;

	PopDebugStack(pCtx);

	return NOERROR;
	}

ALERROR CSystem::CreateMarker (CXMLElement *pDesc, const COrbit &oOrbit, CMarker **retpObj)

//	CreateMarker
//
//	Creates a marker from an XML description

	{
	ALERROR error;

	//	Create the marker

	CMarker *pMarker;
	if (error = CMarker::Create(this,
			NULL,
			oOrbit.GetObjectPos(),
			NullVector,
			NULL_STR,
			&pMarker))
		return error;

	//	If this station is a gate entry-point, then add it to
	//	the table in the system.

	CString sEntryPoint = pDesc->GetAttribute(OBJ_NAME_ATTRIB);
	if (!sEntryPoint.IsBlank())
		NameObject(sEntryPoint, pMarker);

	//	Done

	if (retpObj)
		*retpObj = pMarker;

	return NOERROR;
	}

ALERROR CSystem::CreateParticles (CXMLElement *pDesc, const COrbit &oOrbit, CParticleEffect **retpObj)

//	CreateParticles
//
//	Create a particle field from an XML description

	{
	ALERROR error;

	//	Create the field

	CParticleEffect *pParticles;
	if (error = CParticleEffect::Create(this,
			pDesc,
			oOrbit.GetObjectPos(),
			NullVector,
			&pParticles))
		return error;

	//	Done

	if (retpObj)
		*retpObj = pParticles;

	return NOERROR;
	}

ALERROR CSystem::CreateStationInt (SSystemCreateCtx *pCtx,
								   CStationType *pType,
								   SObjCreateCtx &CreateCtx,
								   CSpaceObject **retpStation,
								   CString *retsError)

//	CreateStationInt
//
//	Creates a station from a type

	{
	ALERROR error;
	CSpaceObject *pStation = NULL;

	//	If this is a ship encounter, then just create the ship

	if (pType->IsShipEncounter())
		{
		CXMLElement *pShipRegistry = pType->GetDesc()->GetContentElementByTag(SHIPS_TAG);
		if (pShipRegistry == NULL)
			{
			ASSERT(false);
			kernelDebugLogMessage("No ship for ship encounter: %s", pType->GetName());
			DumpDebugStack(pCtx);
			if (retpStation)
				*retpStation = NULL;
			return NOERROR;
			}

		//	Load the generator

		SDesignLoadCtx Ctx;

		IShipGenerator *pGenerator;
		if (error = IShipGenerator::CreateFromXMLAsGroup(Ctx, pShipRegistry, &pGenerator))
			{
			if (retsError)
				*retsError = strPatternSubst(CONSTLIT("Unable to create ship for %s: %s"), pType->GetName(), Ctx.sError);
			return error;
			}

		if (error = pGenerator->OnDesignLoadComplete(Ctx))
			{
			if (retsError)
				*retsError = strPatternSubst(CONSTLIT("Unable to create ship for %s: %s"), pType->GetName(), Ctx.sError);
			return error;
			}

		//	Create the ships

		SShipCreateCtx ShipCreateCtx;
		ShipCreateCtx.pSystem = this;
		ShipCreateCtx.vPos = CreateCtx.vPos;
		ShipCreateCtx.pBaseSovereign = pType->GetSovereign();
		ShipCreateCtx.pEncounterInfo = pType;
		ShipCreateCtx.dwFlags = SShipCreateCtx::RETURN_RESULT;

		pGenerator->CreateShips(ShipCreateCtx);

		//	If no ships are created we return

		if (ShipCreateCtx.Result.GetCount() == 0)
			{
			if (retpStation)
				*retpStation = NULL;
			return NOERROR;
			}

		//	Return the first ship created

		pStation = ShipCreateCtx.Result.GetObj(0);

		//	This type has now been encountered

		pType->SetEncountered(this);
		}

	//	If this is static, create a static object

	else if (pType->IsStatic())
		{
		//	Create the station

		CString sError;
		if (error = CStation::CreateFromType(this,
				pType,
				CreateCtx,
				(CStation **)&pStation,
				&sError))
			{
			if (retsError)
				*retsError = strPatternSubst(CONSTLIT("Unable to create station from type %s [%08x]: %s"), pType->GetName(), pType->GetUNID(), sError);
			return error;
			}
		}

	//	Otherwise, create the station

	else
		{
		//	Create the station

		CString sError;
		if (error = CStation::CreateFromType(this,
				pType,
				CreateCtx,
				(CStation **)&pStation,
				&sError))
			{
			if (retsError)
				*retsError = strPatternSubst(CONSTLIT("Unable to create station from type %s [%08x]: %s"), pType->GetName(), pType->GetUNID(), sError);
			return error;
			}
		}

	//	Check station separation

#ifdef DEBUG_STATION_SEPARATION
	if (pStation->CanAttack())
		{
		//	Count to see how many enemy stations are in range

		for (int k = 0; k < GetObjectCount(); k++)
			{
			CSpaceObject *pEnemy = GetObject(k);
			if (pEnemy
					&& pEnemy->GetCategory() == CSpaceObject::catStation
					&& pEnemy->CanAttack()
					&& (pEnemy->IsEnemy(pStation) || pStation->IsEnemy(pEnemy)))
				{
				Metric rDist = pStation->GetDistance(pEnemy);
				int iLSDist = (int)((rDist / LIGHT_SECOND) + 0.5);
				if (iLSDist < 30)
					{
					::kernelDebugLogMessage("%s: %s (%x) and %s (%x) within %d ls.",
							GetName(),
							pStation->GetName(),
							pStation->GetID(),
							pEnemy->GetName(),
							pEnemy->GetID(),
							iLSDist);
					}
				}
			}
		}
#endif

	//	Done

	if (retpStation)
		*retpStation = pStation;

	return NOERROR;
	}

ALERROR CSystem::CreateStation (SSystemCreateCtx *pCtx, 
								CStationType *pType,
								SObjCreateCtx &CreateCtx,
								CSpaceObject **retpStation)

//	CreateStation
//
//	Creates a station of the given type

	{
	ALERROR error;
	CSpaceObject *pStation = NULL;

	//	Create the station (or ship encounter). Note that pStation may come back NULL

#ifdef DEBUG_RANDOM_SEED
	DWORD dwOldSeed = mathGetSeed();
	static DWORD g_Count = 0;
	static bool g_Collect = true;
	static int g_Index = 0;
	static DWORD g_Values[10000];
	static DWORD g_ValuesOld[10000];
#endif

	if (error = CreateStationInt(pCtx,
			pType,
			CreateCtx,
			&pStation,
			&pCtx->sError))
		return ERR_FAIL;

#ifdef DEBUG_RANDOM_SEED
	if (dwOldSeed != mathGetSeed())
		{
		DWORD dwNewSeed = mathGetSeed();

		if (dwNewSeed == 0x2e809d20 && g_Index > 0)
			{
			g_Collect = false;
			g_Index = 0;
			}
		else if (g_Collect && g_Index < 9999)
			{
			g_ValuesOld[g_Index] = dwOldSeed;
			g_Values[g_Index] = dwNewSeed;

			g_Index++;
			}

		if (!g_Collect && g_Index < 9999)
			{
			if (dwOldSeed != g_ValuesOld[g_Index])
				::kernelDebugLogMessage("%s (%x): Discrepancy before create", pType->GetName(), pType->GetUNID());
			else if (dwNewSeed != g_Values[g_Index])
				::kernelDebugLogMessage("%s (%x): Discrepancy during create", pType->GetName(), pType->GetUNID());

			g_Index++;
			}
		}
#endif

	if (pStation)
		pCtx->dwLastObjID = pStation->GetID();

	//	If this is a satellite, then add it as a subordinate

	if (pStation && pCtx->pStation && pStation->CanAttack())
		{
		pCtx->pStation->AddSubordinate(pStation);

		//	And remember our base

		CStation *pStationCast = pStation->AsStation();
		if (pStationCast)
			pStationCast->SetBase(pCtx->pStation);
		}

	//	Create any satellites of the station

	CSpaceObject *pSavedStation = pCtx->pStation;
	pCtx->pStation = pStation;

	CXMLElement *pSatellites = pType->GetSatellitesDesc();
	if (pSatellites 
			&& CreateCtx.bCreateSatellites
			&& CreateCtx.pOrbit)
		{
		//	Set the extension to be where the stationType came from. NOTE: This
		//	works only because the satellite descriptor cannot be inherited.
		//	(If it were, then we would need to get the extension that the
		//	satellite desc came from).

		CExtension *pOldExtension = pCtx->pExtension;
		pCtx->pExtension = pType->GetExtension();

		for (int i = 0; i < pSatellites->GetContentElementCount(); i++)
			{
			CXMLElement *pSatDesc = pSatellites->GetContentElement(i);
			if (error = CreateSystemObject(pCtx, pSatDesc, *CreateCtx.pOrbit))
				return error;
			}

		pCtx->pExtension = pOldExtension;
		}

	pCtx->pStation = pSavedStation;

#ifdef DEBUG_STATION_PLACEMENT2
	if (pStation && pStation->GetScale() == scaleStructure)
		{
		char szBuffer[1024];
		wsprintf(szBuffer, "CreateStation: %s\n", pType->GetName().GetASCIIZPointer());
		::OutputDebugString(szBuffer);
		}
#endif

	//	Load images, if necessary

	if (!IsCreationInProgress())
		pStation->MarkImages();

	//	Done

	if (retpStation)
		*retpStation = pStation;

	return NOERROR;
	}

ALERROR CreateStationFromElement (SSystemCreateCtx *pCtx, CXMLElement *pDesc, const COrbit &OrbitDesc, CStation **retpStation)

//	CreateStation
//
//	Creates a station from an XML description

	{
	ALERROR error;
	CStationType *pStationType;

	//	Get the type of the station

	pStationType = g_pUniverse->FindStationType((DWORD)pDesc->GetAttributeInteger(TYPE_ATTRIB));
	if (pStationType == NULL)
		{
		pCtx->sError = strPatternSubst(CONSTLIT("Unknown station type: %s"), pDesc->GetAttribute(TYPE_ATTRIB));
		return ERR_FAIL;
		}

	PushDebugStack(pCtx, strPatternSubst(CONSTLIT("Station unid=%x"), pStationType->GetUNID()));

	//	Get offsets

	int x = pDesc->GetAttributeInteger(X_OFFSET_ATTRIB);
	int y = pDesc->GetAttributeInteger(Y_OFFSET_ATTRIB);

	//	Compute position of station

	CVector vPos(OrbitDesc.GetObjectPos());
	if (x != 0 || y != 0)
		vPos = vPos + CVector(x * g_KlicksPerPixel, y * g_KlicksPerPixel);


	//	Set up parameters for station creation

	SObjCreateCtx CreateCtx;
	CreateCtx.vPos = vPos;
	CreateCtx.pOrbit = &OrbitDesc;
	CreateCtx.bCreateSatellites = !pDesc->GetAttributeBool(NO_SATELLITES_ATTRIB);
	CreateCtx.pExtraData = pDesc->GetContentElementByTag(INITIAL_DATA_TAG);

	//	Since this is an explicit creation of a station, ignore limits

	CreateCtx.bIgnoreLimits = true;

	//	Sovereign override

	DWORD dwSovereign;
	if (pDesc->FindAttributeInteger(SOVEREIGN_ATTRIB, (int *)&dwSovereign))
		CreateCtx.pSovereign = g_pUniverse->FindSovereign(dwSovereign);

	//	Create the station

	CStation *pStation = NULL;
	CSpaceObject *pObj;
	if (error = pCtx->pSystem->CreateStation(pCtx,
			pStationType,
			CreateCtx,
			&pObj))
		return error;

	if (pObj)
		pStation = pObj->AsStation();

#ifdef DEBUG_STATION_PLACEMENT2
	if (pStation->GetScale() == scaleStructure)
		{
		char szBuffer[1024];
		wsprintf(szBuffer, "CreateStation: %s\n", pStationType->GetName().GetASCIIZPointer());
		::OutputDebugString(szBuffer);
		}
#endif

	//	Done if this is a ship encounter

	if (pStationType->IsShipEncounter())
		{
		PopDebugStack(pCtx);
		if (retpStation)
			*retpStation = NULL;
		return NOERROR;
		}

	//	Modify the station based on attributes

	if (error = ModifyCreatedStation(pCtx, pStation, pDesc, OrbitDesc))
		return error;

	//	Done

	PopDebugStack(pCtx);
	if (retpStation)
		*retpStation = pStation;

	return NOERROR;
	}
