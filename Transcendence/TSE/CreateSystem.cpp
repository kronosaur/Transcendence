//	CreateSystem.cpp
//
//	CreateSystem class

#include "PreComp.h"
#include "math.h"

#ifdef DEBUG
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
#define VARIANTS_TAG					CONSTLIT("Variants")

#define ANGLE_ATTRIB					CONSTLIT("angle")
#define ANGLE_ADJ_ATTRIB				CONSTLIT("angleAdj")
#define ANGLE_INC_ATTRIB				CONSTLIT("angleInc")
#define ARC_INC_ATTRIB					CONSTLIT("arcInc")
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
#define INTERVAL_ATTRIB					CONSTLIT("interval")
#define LEVEL_FREQUENCY_ATTRIB			CONSTLIT("levelFrequency")
#define LOCATION_ATTRIBS_ATTRIB			CONSTLIT("locationAttribs")
#define LOCATION_CRITERIA_ATTRIB		CONSTLIT("locationCriteria")
#define MATCH_ATTRIB					CONSTLIT("match")
#define MAX_ATTRIB						CONSTLIT("max")
#define MAX_RADIUS_ATTRIB				CONSTLIT("maxRadius")
#define MAX_SHIPS_ATTRIB				CONSTLIT("maxShips")
#define MIN_ATTRIB						CONSTLIT("min")
#define MIN_RADIUS_ATTRIB				CONSTLIT("minRadius")
#define NAME_ATTRIB						CONSTLIT("name")
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
#define PATROL_DIST_ATTRIB				CONSTLIT("patrolDist")
#define PERCENT_ENEMIES_ATTRIB			CONSTLIT("percentEnemies")
#define PERCENT_FULL_ATTRIB				CONSTLIT("percentFull")
#define PROBABILITY_ATTRIB				CONSTLIT("probability")
#define RADIUS_DEC_ATTRIB				CONSTLIT("radiusDec")
#define RADIUS_INC_ATTRIB				CONSTLIT("radiusInc")
#define ROTATION_ATTRIB					CONSTLIT("rotation")
#define SEPARATE_ENEMIES_ATTRIB			CONSTLIT("separateEnemies")
#define SHAPE_ATTRIB					CONSTLIT("shape")
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
#define MAX_NEBULAE						10000
#define OVERLAP_DIST					(25.0 * LIGHT_SECOND)

static char g_ProbabilitiesAttrib[] = "probabilities";
static char g_ShowOrbitAttrib[] = "showOrbit";

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
int ComputeLocationWeight (SSystemCreateCtx *pCtx, 
						   const CString &sLocationAttribs,
						   const CVector &vPos,
						   const CString &sAttrib, 
						   int iMatchStrength);
const COrbit *ComputeOffsetOrbit (CXMLElement *pObj, const COrbit &Original, COrbit *retOrbit);
int ComputeStationWeight (SSystemCreateCtx *pCtx, CStationType *pType, const CString &sAttrib, int iMatchStrength);
ALERROR CreateAppropriateStationAtRandomLocation (SSystemCreateCtx *pCtx, 
												  CXMLElement *pDesc, 
												  const CString &sAdditionCriteria,
												  bool *retbEnemy = NULL);
ALERROR CreateLabel (SSystemCreateCtx *pCtx,
					 CXMLElement *pObj,
					 const COrbit &OrbitDesc);
ALERROR CreateLevelTable (SSystemCreateCtx *pCtx, CXMLElement *pDesc, const COrbit &OrbitDesc);
ALERROR CreateLocationCriteriaTable (SSystemCreateCtx *pCtx, CXMLElement *pDesc, const COrbit &OrbitDesc);
ALERROR CreateObjectAtRandomLocation (SSystemCreateCtx *pCtx, CXMLElement *pDesc);
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
bool IsExclusionZoneClear (SSystemCreateCtx *pCtx, const CVector &vPos, Metric rRadius);
bool IsExclusionZoneClear (SSystemCreateCtx *pCtx, const CVector &vPos, CStationType *pType);
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
	STATION_PLACEMENT_OUTPUT("ChooseRandomLocation\n");

	//	Parse the criteria

	CAttributeCriteria Criteria;
	if (error = Criteria.Parse(sCriteria, 0, &pCtx->sError))
		return error;

	//	Choose a random location

	int iLocID;
	if (!pCtx->pSystem->FindRandomLocation(Criteria, 0, pStationToPlace, &iLocID))
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

ALERROR ChooseRandomStation (SSystemCreateCtx *pCtx, 
							 const CString &sCriteria, 
							 const CString &sLocationAttribs,
							 const CVector &vPos,
							 bool bSeparateEnemies,
							 CStationType **retpType)

//	ChooseRandomStation
//
//	Picks a random station to create. The station is appropriate to the level
//	of the system and to the given criteria.

	{
	ALERROR error;

#ifdef DEBUG_STATION_PLACEMENT
	{
	char szBuffer[1024];
	wsprintf(szBuffer, "ChooseRandomStation (%s)\n", sCriteria.GetASCIIZPointer());
	::OutputDebugString(szBuffer);
	}
#endif

	int iLevel = pCtx->pSystem->GetLevel();

	//	Build up a probability table that gives the chance to create
	//	each station type

	int i;
	int iSize = g_pUniverse->GetStationTypeCount();
	int *pProbTable = new int[iSize];

	//	Initialize the table

	for (i = 0; i < iSize; i++)
		{
		CStationType *pType = g_pUniverse->GetStationType(i);
		pProbTable[i] = pType->GetFrequencyForSystem(pCtx->pSystem);
		}

	//	Loop over each station type and adjust for the location that
	//	we want to create the station at

	if (!strEquals(sLocationAttribs, MATCH_ALL))
		{
		for (i = 0; i < iSize; i++)
			{
			if (pProbTable[i])
				{
				CStationType *pType = g_pUniverse->GetStationType(i);
				if (!strEquals(pType->GetLocationCriteria(), MATCH_ALL))
					{
					CAttributeCriteria Criteria;
					if (error = Criteria.Parse(pType->GetLocationCriteria(), 0, &pCtx->sError))
						{
						pCtx->sError = strPatternSubst(CONSTLIT("StationType %x: Invalid locationCriteria"), pType->GetUNID());
						return error;
						}

					for (int j = 0; j < Criteria.GetCount(); j++)
						{
						int iMatchStrength;
						const CString &sAttrib = Criteria.GetAttribAndWeight(j, &iMatchStrength);

						int iAdj = ComputeLocationWeight(pCtx,
								sLocationAttribs,
								vPos,
								sAttrib,
								iMatchStrength);
						pProbTable[i] = (pProbTable[i] * iAdj) / 1000;
						}
					}

				//	If we want to separate enemies, then see if there are any
				//	enemies of this station type at this location.

				if (bSeparateEnemies && pProbTable[i] > 0)
					{
					if (!IsExclusionZoneClear(pCtx, vPos, pType))
						pProbTable[i] = 0;
					}
				}
			}
		}

	//	Loop over each part of the criteria and refine the table

	if (!strEquals(sCriteria, MATCH_ALL))
		{
		CAttributeCriteria Criteria;
		if (error = Criteria.Parse(sCriteria, 0, &pCtx->sError))
			return error;

		for (i = 0; i < Criteria.GetCount(); i++)
			{
			int iMatchStrength;
			const CString &sAttrib = Criteria.GetAttribAndWeight(i, &iMatchStrength);

			for (int j = 0; j < iSize; j++)
				{
				CStationType *pType = g_pUniverse->GetStationType(j);

				//	Adjust probability

				if (pProbTable[j])
					{
					int iAdj = ComputeStationWeight(pCtx, pType, sAttrib, iMatchStrength);
					pProbTable[j] = (pProbTable[j] * iAdj) / 1000;
					}
				}
			}
		}

	//	Add up the total probabilities in the table

	int iTotal = 0;
	for (i = 0; i < iSize; i++)
		iTotal += pProbTable[i];

	//	If no entries match, then we're done

	if (iTotal == 0)
		{
		delete [] pProbTable;

		STATION_PLACEMENT_OUTPUT("   no appropriate station found for this location\n");
		return ERR_NOTFOUND;
		}

#ifdef DEBUG_STATION_TABLES
	{
	int i;
	char szBuffer[1024];
	wsprintf(szBuffer, "ChooseRandomStation\nCriteria: %s\nLocation: %s\n\n",
			sCriteria.GetASCIIZPointer(),
			sLocationAttribs.GetASCIIZPointer());
	::OutputDebugString(szBuffer);

	//	First list all the stations that are in the list

	for (i = 0; i < iSize; i++)
		{
		if (pProbTable[i])
			{
			CStationType *pType = g_pUniverse->GetStationType(i);
			double rProb = 100.0 * (double)pProbTable[i] / (double)iTotal;
			wsprintf(szBuffer, "%3d.%02d %s\n", 
					(int)rProb,
					((int)(rProb * 100)) % 100,
					pType->GetName().GetASCIIZPointer());

			::OutputDebugString(szBuffer);
			}
		}

	::OutputDebugString("\n");

	//	Then include the stations that are not

#if 0
	for (i = 0; i < iSize; i++)
		{
		if (pProbTable[i] == 0)
			{
			CStationType *pType = g_pUniverse->GetStationType(i);
			wsprintf(szBuffer, "%s\n", pType->GetName().GetASCIIZPointer());
			::OutputDebugString(szBuffer);
			}
		}
#endif
	}
#endif

	//	Pick a random entry in the table

	int iRoll = mathRandom(1, iTotal);
	int iPos = 0;

	//	Get the position

	while (pProbTable[iPos] < iRoll)
		iRoll -= pProbTable[iPos++];

	//	Found it

	*retpType = g_pUniverse->GetStationType(iPos);
	delete [] pProbTable;

#ifdef DEBUG_STATION_PLACEMENT
	{
	char szBuffer[1024];
	wsprintf(szBuffer, "   chose station: %s\n", (*retpType)->GetName().GetASCIIZPointer());
	::OutputDebugString(szBuffer);
	}
#endif

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
						   int iMatchStrength)

//	ComputeLocationWeight
//
//	Computes the weight of the given location if we're looking for
//	locations with the given criteria.

	{
	//	Check to see if either the label
	//	or the node/system has the attribute.

	bool bHasAttrib = (::HasModifier(sLocationAttribs, sAttrib)
			|| pCtx->pSystem->HasAttribute(vPos, sAttrib));

	//	Adjust probability based on the match strength

	return ComputeWeightAdjFromMatchStrength(bHasAttrib, iMatchStrength);
	}

int ComputeStationWeight (SSystemCreateCtx *pCtx, CStationType *pType, const CString &sAttrib, int iMatchStrength)

//	ComputeStationWeight
//
//	Returns the weight of this station type given the attribute and match weight

	{
	return ComputeWeightAdjFromMatchStrength(
			pType->HasAttribute(sAttrib),
			iMatchStrength);
	}

ALERROR DistributeStationsAtRandomLocations (SSystemCreateCtx *pCtx, CXMLElement *pDesc, int iCount)

//	DistributeStationsAtRandomLocations
//
//	Fills several locations with random stations

	{
	ALERROR error;
	int i;

	STATION_PLACEMENT_OUTPUT("DistributeStationsAtRandomLocations\n");

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

	//	Create the stations

	for (i = 0; i < iCount; i++)
		{
		if (iEnemies && iFriends)
			{
			bool bEnemy;
			if (error = CreateAppropriateStationAtRandomLocation(pCtx, pDesc, NULL_STR, &bEnemy))
				return error;

			if (bEnemy)
				iEnemies--;
			else
				iFriends--;
			}
		else if (iEnemies)
			{
			if (error = CreateAppropriateStationAtRandomLocation(pCtx, pDesc, REQUIRE_ENEMY))
				return error;

			iEnemies--;
			}
		else if (iFriends)
			{
			if (error = CreateAppropriateStationAtRandomLocation(pCtx, pDesc, REQUIRE_FRIEND))
				return error;

			iFriends--;
			}
		}

	return NOERROR;
	}

ALERROR CreateAppropriateStationAtRandomLocation (SSystemCreateCtx *pCtx, 
												  CXMLElement *pDesc, 
												  const CString &sAdditionalCriteria,
												  bool *retbEnemy)

//	CreateAppropriateStationAtRandomLocation
//
//	Picks a random location and fills it with a randomly chosen station approriate
//	to the location.

	{
	ALERROR error;

	STATION_PLACEMENT_OUTPUT("CreateAppropriateStationAtRandomLocation\n");

	CString sLocationCriteria = pDesc->GetAttribute(LOCATION_CRITERIA_ATTRIB);
	CString sStationCriteria = pDesc->GetAttribute(STATION_CRITERIA_ATTRIB);
	bool bSeparateEnemies = pDesc->GetAttributeBool(SEPARATE_ENEMIES_ATTRIB);

	//	Add addition criteria

	if (!sAdditionalCriteria.IsBlank())
		{
		if (sStationCriteria.IsBlank())
			sStationCriteria = sAdditionalCriteria;
		else
			sStationCriteria = strPatternSubst(CONSTLIT("%s,%s"), sStationCriteria, sAdditionalCriteria);
		}

	STATION_PLACEMENT_OUTPUT(strPatternSubst(CONSTLIT("locationCriteria=%s\n"), sLocationCriteria).GetASCIIZPointer());
	STATION_PLACEMENT_OUTPUT(strPatternSubst(CONSTLIT("stationCriteria=%s\n"), sStationCriteria).GetASCIIZPointer());

	//	Keep trying for a while to make sure that we find something that fits

	int iTries = 10;
	while (iTries > 0)
		{
		STATION_PLACEMENT_OUTPUT(strPatternSubst(CONSTLIT("try %d\n"), 11 - iTries).GetASCIIZPointer());

		//	Pick a random location that fits the criteria

		int iLabelPos;
		COrbit OrbitDesc;
		CString sLocationAttribs;
		if (error = ChooseRandomLocation(pCtx,
				sLocationCriteria,
				NULL,
				&OrbitDesc,
				&sLocationAttribs,
				&iLabelPos))
			{
			if (error == ERR_NOTFOUND)
				return NOERROR;
			else
				return error;
			}

		//	Now look for the most appropriate station to place at the location

		CStationType *pType;
		if (error = ChooseRandomStation(pCtx, 
				sStationCriteria, 
				sLocationAttribs,
				OrbitDesc.GetObjectPos(),
				bSeparateEnemies,
				&pType))
			{
			//	If we couldn't find an appropriate location then try picking
			//	a different location.

			if (error == ERR_NOTFOUND)
				{
				iTries--;
				continue;
				}
			else
				return error;
			}

		//	Remember if this is friend or enemy

		if (retbEnemy)
			*retbEnemy = pType->HasAttribute(ENEMY_ATTRIBUTE);

		//	Remember object created

		DWORD dwSavedLastObjID = pCtx->dwLastObjID;
		pCtx->dwLastObjID = 0;

		//	Create the station at the location

		if (error = pCtx->pSystem->CreateStation(pCtx,
				pType,
				OrbitDesc.GetObjectPos(),
				OrbitDesc,
				true,
				NULL))
			return error;

		//	Remove the location so it doesn't match again

		pCtx->pSystem->SetLocationObjID(iLabelPos, pCtx->dwLastObjID);
		pCtx->dwLastObjID = dwSavedLastObjID;

		//	No more tries

		break;
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
		pCtx->pStats->AddLabel(pLoc->GetAttributes());

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
					int iMatchStrength;
					const CString &sAttrib = Criteria.GetAttribAndWeight(j, &iMatchStrength);

					int iAdj = ComputeLocationWeight(pCtx, 
							pCtx->sLocationAttribs,
							OrbitDesc.GetObjectPos(),
							sAttrib,
							iMatchStrength);

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

ALERROR CreateObjectAtRandomLocation (SSystemCreateCtx *pCtx, CXMLElement *pDesc)

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

	//	Figure out the number of objects to create, ...

	int iCount;
	int iPercent;
	if (pDesc->FindAttributeInteger(PERCENT_FULL_ATTRIB, &iPercent))
		{
		TArray<int> EmptyLocations;
		pCtx->pSystem->GetEmptyLocations(&EmptyLocations);
		iCount = Max(0, iPercent * EmptyLocations.GetCount() / 100);
		}
	else
		iCount = GetDiceCountFromAttribute(pDesc->GetAttribute(COUNT_ATTRIB));

	if (iChildCount > 1)
		iCount = Max(iCount, iChildCount);

	//	Loop

	CString sCriteria = pDesc->GetAttribute(LOCATION_CRITERIA_ATTRIB);
	CString sSavedLocationAttribs = pCtx->sLocationAttribs;

	for (i = 0; i < iCount; i++)
		{
		COrbit NewOrbit;

		int iLabelPos;
		CString sLocationAttribs;
		if (error = ChooseRandomLocation(pCtx, 
				sCriteria,
				NULL,
				&NewOrbit, 
				&sLocationAttribs,
				&iLabelPos))
			{
			//	NOTFOUND means that an appropriate entry could
			//	not be found. This is not an error since sometimes
			//	we fill-up all labels.

			if (error == ERR_NOTFOUND)
				{
				pCtx->sLocationAttribs = CString();
				return NOERROR;
				}
			else
				return error;
			}

		//	Create a superset of location attributes

		pCtx->sLocationAttribs = ::AppendModifiers(sSavedLocationAttribs, sLocationAttribs);

		//	Create the object

		DWORD dwSavedLastObjID = pCtx->dwLastObjID;
		pCtx->dwLastObjID = 0;

		if (error = CreateSystemObject(pCtx, pDesc->GetContentElement(i % iChildCount), NewOrbit))
			return error;

		//	If we actually created an object, then remove the label

		pCtx->pSystem->SetLocationObjID(iLabelPos, pCtx->dwLastObjID);
		pCtx->dwLastObjID = dwSavedLastObjID;

#ifdef DEBUG_STATION_EXCLUSION_ZONE
		::kernelDebugLogMessage("CreateObjectAtRandomLocation: Created %s",
				pDesc->GetContentElement(i % iChildCount)->GetTag());
#endif
		}

	pCtx->sLocationAttribs = sSavedLocationAttribs;

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

	//	Get the scale

	Metric rScale = GetScale(pObj);

	//	Create all the object

	for (i = 0; i < iCount; i++)
		{
		Metric rX = rScale * GetDiceCountFromAttribute(pObj->GetAttribute(X_OFFSET_ATTRIB));
		Metric rY = rScale * GetDiceCountFromAttribute(pObj->GetAttribute(Y_OFFSET_ATTRIB));

		//	Generate an orbit that passes through the point

		Metric rRadius;
		Metric rAngle = VectorToPolarRadians(CVector(rX, rY), &rRadius);

		//	Create the orbit

		COrbit NewOrbit(OrbitDesc.GetObjectPos(), rRadius, rAngle);

		//	Create the object

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

	//	Pick a random station type that fits the criteria

	CStationType *pType;
	if (error = ChooseRandomStation(pCtx, 
			sStationCriteria, 
			sLocationAttribs,
			OrbitDesc.GetObjectPos(),
			false,
			&pType))
		{
		if (error == ERR_NOTFOUND)
			return NOERROR;
		else
			return error;
		}

	//	Create the station at the location

	if (error = pCtx->pSystem->CreateStation(pCtx,
			pType,
			OrbitDesc.GetObjectPos(),
			OrbitDesc,
			true,
			NULL))
		return error;

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
				&pType))
			{
			if (error == ERR_NOTFOUND)
				return NOERROR;
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

		if (error = pCtx->pSystem->CreateStation(pCtx,
				pType,
				OrbitDesc.GetObjectPos(),
				OrbitDesc,
				true,
				NULL))
			return error;

		//	Remember that we filled this location

		pCtx->pSystem->SetLocationObjID(iLocation, pCtx->dwLastObjID);
		pCtx->dwLastObjID = dwSavedLastObjID;

		//	No more tries

		break;
		}

	return NOERROR;
	}

ALERROR CreateSiblings (SSystemCreateCtx *pCtx, 
						CXMLElement *pObj, 
						const COrbit &OrbitDesc)

//	CreateSiblings
//
//	Creates objects in similar orbits

	{
	ALERROR error;
	CString sDistribution;

	if (pObj->GetContentElementCount() == 0)
		return NOERROR;

	//	Get the number of objects to create

	int iCount = GetDiceCountFromAttribute(pObj->GetAttribute(COUNT_ATTRIB));

	//	Create a random distribution

	if (pObj->FindAttribute(DISTRIBUTION_ATTRIB, &sDistribution))
		{
		DiceRange Distribution;
		Distribution.LoadFromXML(sDistribution);
		Metric rScale = GetScale(pObj);

		for (int i = 0; i < iCount; i++)
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

		//	Loop over count

		for (int i = 0; i < iCount; i++)
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

			COrbit SiblingOrbit(OrbitDesc.GetFocus(),
					OrbitDesc.GetSemiMajorAxis() + rRadiusAdj,
					OrbitDesc.GetEccentricity(),
					OrbitDesc.GetRotation(),
					OrbitDesc.GetObjectAngle() + rAngleAdj);

			if (error = CreateSystemObject(pCtx,
					pObj->GetContentElement(0),
					SiblingOrbit))
				return error;
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

	CSpaceEnvironmentType *pEnvironment = g_pUniverse->FindSpaceEnvironment(pDesc->GetAttributeInteger(TYPE_ATTRIB));
	if (pEnvironment == NULL)
		{
		pCtx->sError = CONSTLIT("Invalid space environment type");
		return ERR_FAIL;
		}

	//	Patches

	CEffectCreator *pPatchType = g_pUniverse->FindEffectType(pDesc->GetAttributeInteger(PATCHES_ATTRIB));
	int iPatchFrequency = GetDiceCountFromAttribute(pDesc->GetAttribute(PATCH_FREQUENCY_ATTRIB));

	//	Keep track of all the places where we've added nebulae

	int *xNebulae = new int [MAX_NEBULAE];
	int *yNebulae = new int [MAX_NEBULAE];
	int iNebulaeCount = 0;

	//	Fill the appropriate shape

	CString sShape = pDesc->GetAttribute(SHAPE_ATTRIB);
	if (strEquals(sShape, SHAPE_CIRCULAR))
		{
		Metric rHalfWidth = GetDiceCountFromAttribute(pDesc->GetAttribute(WIDTH_ATTRIB)) * LIGHT_SECOND / 2.0;
		Metric rMaxVariation = GetDiceCountFromAttribute(pDesc->GetAttribute(WIDTH_VARIATION_ATTRIB)) * rHalfWidth / 100.0;
		Metric rHalfVariation = rMaxVariation / 2.0;

		CVector vCenter = OrbitDesc.GetFocus();

		//	Compute the inner and outer radius for each degree around the orbit
		//	Also, while we're at it, we compute the upper-left and lower-right
		//	bounds of the nebula

		Metric rOuterRadius[360];
		Metric rInnerRadius[360];
		CVector vUL = vCenter;
		CVector vLR = vCenter;

		Metric rOuterVariation = rHalfVariation;
		Metric rInnerVariation = rHalfVariation;
		for (i = 0; i < 360; i++)
			{
			Metric rAngle = i * g_Pi / 180.0;
			Metric rRadius;
			OrbitDesc.GetPointAndRadius(rAngle, &rRadius);

			rOuterRadius[i] = rRadius + rHalfWidth + rOuterVariation - rHalfVariation;
			rInnerRadius[i] = rRadius - rHalfWidth + rInnerVariation - rHalfVariation;

			//	Adjust inner and outer radius in a random walk

			ChangeVariation(&rOuterVariation, rMaxVariation);
			ChangeVariation(&rInnerVariation, rMaxVariation);

			//	Adjust bounds

			AdjustBounds(&vUL, &vLR, vCenter + PolarToVector(i, rOuterRadius[i]));
			}

		//	Now iterate over every tile in bounds and see if it is within
		//	the band that we have defined.

		int xTileStart, yTileStart, xTileEnd, yTileEnd;
		VectorToTile(vUL, &xTileStart, &yTileStart);
		VectorToTile(vLR, &xTileEnd, &yTileEnd);

		int x, y;
		for (x = xTileStart; x <= xTileEnd; x++)
			for (y = yTileStart; y <= yTileEnd; y++)
				{
				CVector vTile = TileToVector(x, y);
				CVector vRadius = vTile - vCenter;

				Metric rRadius;
				int iAngle = VectorToPolar(vRadius, &rRadius);

				if (rRadius > rInnerRadius[iAngle] && rRadius < rOuterRadius[iAngle])
					{
					CreateSpaceEnvironmentTile(pCtx, vTile, x, y, pEnvironment, pPatchType, iPatchFrequency);

					if (iNebulaeCount < MAX_NEBULAE - 1)
						{
						xNebulae[iNebulaeCount] = x;
						yNebulae[iNebulaeCount++] = y;
						}
					}
				}
		}
	else if (strEquals(sShape, SHAPE_ARC))
		{
		Metric rHalfWidth = GetDiceCountFromAttribute(pDesc->GetAttribute(WIDTH_ATTRIB)) * LIGHT_SECOND / 2.0;
		Metric rMaxVariation = GetDiceCountFromAttribute(pDesc->GetAttribute(WIDTH_VARIATION_ATTRIB)) * rHalfWidth / 100.0;
		Metric rHalfVariation = rMaxVariation / 2.0;
		int iHalfSpan = GetDiceCountFromAttribute(pDesc->GetAttribute(SPAN_ATTRIB)) / 2;

		CVector vCenter = OrbitDesc.GetFocus();
		int iArcCenterAngle = (((int)((180.0 * OrbitDesc.GetObjectAngle() / g_Pi) + 0.5)) % 360);

		//	Compute the inner and outer radius for each degree around the orbit
		//	Also, while we're at it, we compute the upper-left and lower-right
		//	bounds of the nebula

		Metric rOuterRadius[360];
		Metric rInnerRadius[360];
		CVector vUL = CVector(g_InfiniteDistance, -g_InfiniteDistance);
		CVector vLR = CVector(-g_InfiniteDistance, g_InfiniteDistance);

		Metric rOuterVariation = 0.0;
		Metric rInnerVariation = rMaxVariation;
		for (i = 0; i < 360; i++)
			{
			rOuterRadius[i] = 0.0;
			rInnerRadius[i] = 0.0;
			}

		for (i = -iHalfSpan; i <= iHalfSpan; i++)
			{
			int iAngle = ((iArcCenterAngle + i + 360) % 360);

			Metric rAngle = iAngle * g_Pi / 180.0;
			Metric rRadius;
			OrbitDesc.GetPointAndRadius(rAngle, &rRadius);

			rOuterRadius[iAngle] = rRadius + rHalfWidth + rOuterVariation - rHalfVariation;
			rInnerRadius[iAngle] = rRadius - rHalfWidth + rInnerVariation - rHalfVariation;

			//	Adjust inner and outer radius in a random walk

			ChangeVariation(&rOuterVariation, rMaxVariation);
			ChangeVariation(&rInnerVariation, rMaxVariation);

			//	Adjust bounds

			AdjustBounds(&vUL, &vLR, vCenter + PolarToVector(iAngle, rOuterRadius[iAngle]));
			AdjustBounds(&vUL, &vLR, vCenter + PolarToVector(iAngle, rInnerRadius[iAngle]));
			}

		//	Now iterate over every tile in bounds and see if it is within
		//	the band that we have defined.

		int xTileStart, yTileStart, xTileEnd, yTileEnd;
		VectorToTile(vUL, &xTileStart, &yTileStart);
		VectorToTile(vLR, &xTileEnd, &yTileEnd);

		int x, y;
		for (x = xTileStart; x <= xTileEnd; x++)
			for (y = yTileStart; y <= yTileEnd; y++)
				{
				CVector vTile = TileToVector(x, y);
				CVector vRadius = vTile - vCenter;

				Metric rRadius;
				int iAngle = VectorToPolar(vRadius, &rRadius);

				if (rRadius > rInnerRadius[iAngle] && rRadius < rOuterRadius[iAngle])
					{
					CreateSpaceEnvironmentTile(pCtx, vTile, x, y, pEnvironment, pPatchType, iPatchFrequency);

					if (iNebulaeCount < MAX_NEBULAE - 1)
						{
						xNebulae[iNebulaeCount] = x;
						yNebulae[iNebulaeCount++] = y;
						}
					}
				}
		}
	else
		{
		pCtx->sError = CONSTLIT("Invalid space environment shape");
		delete xNebulae;
		delete yNebulae;
		return ERR_FAIL;
		}

	//	Create random encounters

	int iEncounters = GetDiceCountFromAttribute(pDesc->GetAttribute(ENCOUNTERS_ATTRIB));

	CXMLElement *pEncounter = NULL;
	if (pDesc->GetContentElementCount())
		pEncounter = pDesc->GetContentElement(0);

	if (pEncounter && iNebulaeCount > 0)
		{
		if (iEncounters == 0)
			iEncounters = 1;

		for (i = 0; i < iEncounters; i++)
			{
			//	Pick a random tile

			int iTile = mathRandom(0, iNebulaeCount-1);
			CVector vCenter = TileToVector(xNebulae[iTile], yNebulae[iTile]);
			CVector vOffset(
					g_KlicksPerPixel * (mathRandom(0, seaTileSize) - (seaTileSize / 2)),
					g_KlicksPerPixel * (mathRandom(0, seaTileSize) - (seaTileSize / 2)));

			COrbit NewOrbit(vCenter + vOffset, 0.0);

			//	Create the object

			if (error = CreateSystemObject(pCtx, pEncounter, NewOrbit))
				{
				delete xNebulae;
				delete yNebulae;
				return error;
				}
			}
		}

	delete xNebulae;
	delete yNebulae;
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
		CVector vOffset(
				g_KlicksPerPixel * (mathRandom(0, seaTileSize) - (seaTileSize / 2)),
				g_KlicksPerPixel * (mathRandom(0, seaTileSize) - (seaTileSize / 2)));

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
		//	If we've got an offset, change the orbit

		COrbit NewOrbit;
		const COrbit *pOrbitDesc = ComputeOffsetOrbit(pObj, OrbitDesc, &NewOrbit);

		//	Keep track of the current extension, because we may change it below

		CExtension *pOldExtension = pCtx->pExtension;

		//	Find the appropriate table. First we look in the local table:

		CXMLElement *pTableDesc = NULL;
		CString sTable = pObj->GetAttribute(TABLE_ATTRIB);
		if (pCtx->pLocalTables)
			pTableDesc = pCtx->pLocalTables->GetContentElementByTag(sTable);

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

		//	Create all the objects

		for (int i = 0; i < pTableDesc->GetContentElementCount(); i++)
			{
			CXMLElement *pResult = pTableDesc->GetContentElement(i);

			//	Recurse

			if (error = CreateSystemObject(pCtx, pResult, *pOrbitDesc))
				return error;
			}

		pCtx->pExtension = pOldExtension;
		}
	else if (strEquals(sTag, ORBITALS_TAG))
		{
		if (error = CreateOrbitals(pCtx, pObj, OrbitDesc))
			return error;
		}
	else if (strEquals(sTag, TROJAN_TAG))
		{
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
		}
	else if (strEquals(sTag, ANTI_TROJAN_TAG))
		{
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
		}
	else if (strEquals(sTag, RANDOM_LOCATION_TAG))
		{
		if (error = CreateObjectAtRandomLocation(pCtx, pObj))
			return error;
		}
	else if (strEquals(sTag, PLACE_RANDOM_STATION_TAG))
		{
		int iCount = GetDiceCountFromAttribute(pObj->GetAttribute(COUNT_ATTRIB));
		for (int i = 0; i < iCount; i++)
			if (error = CreateRandomStationAtAppropriateLocation(pCtx, pObj))
				return error;
		}
	else if (strEquals(sTag, FILL_LOCATIONS_TAG))
		{
		int iCount = pCtx->pSystem->GetEmptyLocationCount();
		int iPercent = pObj->GetAttributeInteger(PERCENT_FULL_ATTRIB);
		iCount = iCount * iPercent / 100;

		if (error = DistributeStationsAtRandomLocations(pCtx, pObj, iCount))
			return error;
		}
	else if (strEquals(sTag, FILL_RANDOM_LOCATION_TAG))
		{
		int iCount = GetDiceCountFromAttribute(pObj->GetAttribute(COUNT_ATTRIB));

		if (error = DistributeStationsAtRandomLocations(pCtx, pObj, iCount))
			return error;
		}
	else if (strEquals(sTag, RANDOM_STATION_TAG))
		{
		if (error = CreateRandomStation(pCtx, pObj, OrbitDesc))
			return error;
		}
	else if (strEquals(sTag, SIBLINGS_TAG))
		{
		if (error = CreateSiblings(pCtx, pObj, OrbitDesc))
			return error;
		}
	else if (strEquals(sTag, OFFSET_TAG))
		{
		if (error = CreateOffsetObjects(pCtx, pObj, OrbitDesc))
			return error;
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
		if (error = CreateLevelTable(pCtx, pObj, OrbitDesc))
			return error;
		}
	else if (strEquals(sTag, LOCATION_CRITERIA_TABLE_TAG))
		{
		if (error = CreateLocationCriteriaTable(pCtx, pObj, OrbitDesc))
			return error;
		}
	else if (strEquals(sTag, VARIANTS_TAG))
		{
		if (error = CreateVariantsTable(pCtx, pObj, OrbitDesc))
			return error;
		}
	else if (strEquals(sTag, PRIMARY_TAG))
		{
		for (int i = 0; i < pObj->GetContentElementCount(); i++)
			{
			CXMLElement *pItem = pObj->GetContentElement(i);

			if (error = CreateSystemObject(pCtx, pItem, OrbitDesc))
				return error;
			}
		}
	else if (strEquals(sTag, STARGATE_TAG))
		{
		//	If we've got an offset, change the orbit

		COrbit NewOrbit;
		const COrbit *pOrbitDesc = ComputeOffsetOrbit(pObj, OrbitDesc, &NewOrbit);

		//	Create

		if (error = CreateStargate(pCtx, pObj, *pOrbitDesc))
			return error;
		}
	else if (strEquals(sTag, SHIP_TAG))
		{
		int iCount = GetDiceCountFromAttribute(pObj->GetAttribute(COUNT_ATTRIB));

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
		}
	else if (strEquals(sTag, PARTICLES_TAG))
		{
		CParticleEffect *pSObj;

		if (error = pCtx->pSystem->CreateParticles(pObj, OrbitDesc, &pSObj))
			{
			pCtx->sError = CONSTLIT("Unable to create particle field");
			return error;
			}

		pCtx->dwLastObjID = pSObj->GetID();
		}
	else if (strEquals(sTag, MARKER_TAG))
		{
		CMarker *pSObj;

		if (error = pCtx->pSystem->CreateMarker(pObj, OrbitDesc, &pSObj))
			{
			pCtx->sError = CONSTLIT("Unable to create marker");
			return error;
			}

		pCtx->dwLastObjID = pSObj->GetID();
		}
	else if (strEquals(sTag, SPACE_ENVIRONMENT_TAG))
		{
		if (error = CreateSpaceEnvironment(pCtx, pObj, OrbitDesc))
			return error;
		}
	else if (strEquals(sTag, CODE_TAG))
		{
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
		if (error = pCtx->pSystem->GetType()->FireSystemCreateCode(*pCtx, pCode, &sError))
			{
			pCtx->sError = strPatternSubst(CONSTLIT("<Code>: %s"), sError);
			pCode->Discard(&g_pUniverse->GetCC());
			return error;
			}

		//	Done

		pCode->Discard(&g_pUniverse->GetCC());
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
			if (!pCtx->pTopologyNode->HasVariantLabel(sVariant))
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
							(bRequired ? 4 : -4));

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

	//	Create

	if (pFound)
		{
		if (error = CreateSystemObject(pCtx, pFound, OrbitDesc))
			return error;
		}
	
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
	int i;

	CXMLElement *pDesc = pType->GetDesc();
	if (pDesc == NULL)
		return ERR_FAIL;

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
	pSystem->m_fNoRandomEncounters = pDesc->GetAttributeBool(NO_RANDOM_ENCOUNTERS_ATTRIB);
	pSystem->m_fUseDefaultTerritories = true;

	//	Set scales

	CString sAttrib;
	if (pDesc->FindAttribute(SPACE_SCALE_ATTRIB, &sAttrib))
		{
		int iScale = strToInt(sAttrib, 0, NULL);
		if (iScale > 0)
			pSystem->m_rKlicksPerPixel = (Metric)iScale;
		}
	if (pDesc->FindAttribute(TIME_SCALE_ATTRIB, &sAttrib))
		{
		int iScale = strToInt(sAttrib, 0, NULL);
		if (iScale > 0)
			pSystem->m_rTimeScale = (Metric)iScale;
		}

	//	Get a pointer to the tables element (may be NULL)

	CXMLElement *pTables = pType->GetLocalSystemTables();

	//	Look for the outer-most group tag

	CXMLElement *pPrimary = pDesc->GetContentElementByTag(SYSTEM_GROUP_TAG);
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
	Ctx.pLocalTables = pTables;
	Ctx.pStats = pStats;

	if (error = CreateSystemObject(&Ctx,
			pPrimary,
			COrbit()))
		{
		if (retsError)
			*retsError = Ctx.sError;
		kernelDebugLogMessage("Unable to create system: %s", Ctx.sError);
#ifdef DEBUG
		::OutputDebugString(Ctx.sError.GetASCIIZPointer());
		::OutputDebugString("\n");
#endif
		return error;
		}

	//	Invoke OnCreate event

	if (error = pType->FireOnCreate(Ctx, &Ctx.sError))
		kernelDebugLogMessage("%s", Ctx.sError);

	//	Now invoke OnGlobalSystemCreated

	g_pUniverse->FireOnGlobalSystemCreated(Ctx);

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
			kernelDebugLogMessage("Unable to find required stargate: %s", pTopology->GetStargate(i));
			}

	//	Done

	*retpSystem = pSystem;

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

ALERROR CSystem::CreateStationInt (CStationType *pType,
								   const CVector &vPos,
								   const CVector &vVel,
								   CXMLElement *pExtraData,
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

		SShipCreateCtx CreateCtx;
		CreateCtx.pSystem = this;
		CreateCtx.vPos = vPos;
		CreateCtx.pBaseSovereign = pType->GetSovereign();
		CreateCtx.pEncounterInfo = pType;
		CreateCtx.dwFlags = SShipCreateCtx::RETURN_RESULT;

		pGenerator->CreateShips(CreateCtx);

		//	If no ships are created we return

		if (CreateCtx.Result.GetCount() == 0)
			{
			if (retpStation)
				*retpStation = NULL;
			return NOERROR;
			}

		//	Return the first ship created

		pStation = CreateCtx.Result.GetObj(0);

		//	This type has now been encountered

		pType->SetEncountered();
		}

	//	If this is static, create a static object

	else if (pType->IsStatic())
		{
		//	Create the station

		if (error = CStation::CreateFromType(this,
				pType,
				vPos,
				vVel,
				pExtraData,
				(CStation **)&pStation))
			{
			if (retsError)
				*retsError = strPatternSubst(CONSTLIT("Unable to create station from type: %s"), pType->GetName());
			return error;
			}
		}

	//	Otherwise, create the station

	else
		{
		//	Create the station

		if (error = CStation::CreateFromType(this,
				pType,
				vPos,
				vVel,
				pExtraData,
				(CStation **)&pStation))
			{
			if (retsError)
				*retsError = strPatternSubst(CONSTLIT("Unable to create station from type: %s"), pType->GetName());
			return error;
			}
		}

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
								const CVector &vPos,
								const COrbit &OrbitDesc,
								bool bCreateSatellites,
								CXMLElement *pExtraData,
								CSpaceObject **retpStation)

//	CreateStation
//
//	Creates a station of the given type

	{
	ALERROR error;
	CSpaceObject *pStation = NULL;

	//	Create the station (or ship encounter). Note that pStation may come back NULL

	if (error = CreateStationInt(pType,
			vPos,
			NullVector,
			pExtraData,
			&pStation,
			&pCtx->sError))
		return ERR_FAIL;

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
	if (pSatellites && bCreateSatellites)
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
			if (error = CreateSystemObject(pCtx, pSatDesc, OrbitDesc))
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
	int i;
	CStationType *pStationType;

	//	Get the type of the station

	pStationType = g_pUniverse->FindStationType((DWORD)pDesc->GetAttributeInteger(TYPE_ATTRIB));
	if (pStationType == NULL)
		{
		pCtx->sError = strPatternSubst(CONSTLIT("Unknown station type: %s"), pDesc->GetAttribute(TYPE_ATTRIB));
		return ERR_FAIL;
		}

	//	If this is a unique station that we've already encountered, then bail out (this is not
	//	an error because sometimes we explicitly place a station even though there is a chance
	//	that it might have been encountered previously).

	if (!pStationType->CanBeEncountered(pCtx->pSystem))
		return NOERROR;

	//	Get offsets

	int x = pDesc->GetAttributeInteger(X_OFFSET_ATTRIB);
	int y = pDesc->GetAttributeInteger(Y_OFFSET_ATTRIB);

	//	Compute position of station

	CVector vPos(OrbitDesc.GetObjectPos());
	if (x != 0 || y != 0)
		vPos = vPos + CVector(x * g_KlicksPerPixel, y * g_KlicksPerPixel);

	//	Create the station

	CStation *pStation = NULL;
	CSpaceObject *pObj;
	if (error = pCtx->pSystem->CreateStation(pCtx,
			pStationType,
			vPos,
			OrbitDesc,
			!pDesc->GetAttributeBool(NO_SATELLITES_ATTRIB),
			pDesc->GetContentElementByTag(INITIAL_DATA_TAG),
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
		if (retpStation)
			*retpStation = NULL;
		return NOERROR;
		}

	//	Set the name of the station, if specified by the system

	CString sName = pDesc->GetAttribute(NAME_ATTRIB);
	if (!sName.IsBlank())
		pObj->SetName(sName, LoadNameFlags(pDesc));

	//	If we want to show the orbit for this station, set the orbit desc

	if (pDesc->GetAttributeBool(CONSTLIT(g_ShowOrbitAttrib)))
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

	if (retpStation)
		*retpStation = pStation;

	return NOERROR;
	}
