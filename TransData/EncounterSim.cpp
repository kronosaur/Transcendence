//	EncounterSim.cpp
//
//	Simulates a ship attacking a station.

#include <stdio.h>

#include <windows.h>
#include "Alchemy.h"
#include "XMLUtil.h"
#include "TransData.h"

const int DEFAULT_RUN_COUNT =				10;
const int DEFAULT_TIME_OUT =				30 * 60 * 5;
const int DESTROY_TIME =					30 * 4;
const Metric INITIAL_DISTANCE =				(100.0 * LIGHT_SECOND);

#define ATTACKERS_ATTRIB					CONSTLIT("attackers")
#define COUNT_ATTRIB						CONSTLIT("count")
#define DEFENDERS_ATTRIB					CONSTLIT("defenders")
#define VIEWER_ATTRIB						CONSTLIT("viewer")

enum EResults
	{
	resultError,

	resultAttackerDestroyed,
	resultDefenderDestroyed,
	resultTimeout,
	};

CSovereign *GetAttackerSovereign (CUniverse &Universe, CStationType *pDefenderType);
EResults RunEncounter (CUniverse &Universe, CSimViewer &Viewer, CStationType *pDefenderType, CShipClass *pAttackerClass, CSovereign *pAttackerSovereign);

void RunEncounterSim (CUniverse &Universe, CXMLElement *pCmdLine)
	{
	int iRuns = pCmdLine->GetAttributeIntegerBounded(COUNT_ATTRIB, 1, -1, DEFAULT_RUN_COUNT);
	int iTimeOut = DEFAULT_TIME_OUT;

	//	Get the station criteria

	CString sCriteria = strPatternSubst(CONSTLIT("%s t"), pCmdLine->GetAttribute(DEFENDERS_ATTRIB));

	CDesignTypeCriteria DefenderCriteria;
	if (CDesignTypeCriteria::ParseCriteria(sCriteria, &DefenderCriteria) != NOERROR)
		{
		printf("ERROR: Unable to parse defenders criteria.\n");
		return;
		}

	bool bAll = pCmdLine->GetAttributeBool(CONSTLIT("all"));

	//	Get the criteria from the command line. Always append 's' because we
	//	want ship classes.

	sCriteria = strPatternSubst(CONSTLIT("%s s"), pCmdLine->GetAttribute(ATTACKERS_ATTRIB));

	CDesignTypeCriteria AttackerCriteria;
	if (CDesignTypeCriteria::ParseCriteria(sCriteria, &AttackerCriteria) != NOERROR)
		{
		printf("ERROR: Unable to parse attackers criteria.\n");
		return;
		}

	//	Create a viewer, if desired

	CSimViewer Viewer;
	if (pCmdLine->GetAttributeBool(VIEWER_ATTRIB))
		Viewer.Create();

	//	Print headers

	printf("Encounter\tLevel");

	int iAttacker;
	for (iAttacker = 0; iAttacker < Universe.GetShipClassCount(); iAttacker++)
		{
		CShipClass *pAttackerClass = Universe.GetShipClass(iAttacker);
		if (!pAttackerClass->MatchesCriteria(AttackerCriteria))
			continue;

		printf("\t%s", pAttackerClass->GetNounPhrase(nounGeneric).GetASCIIZPointer());
		}

	printf("\n");

	//	Loop over all defenders

	int iDefender;
	for (iDefender = 0; iDefender < Universe.GetStationTypeCount(); iDefender++)
		{
		CStationType *pDefenderType = Universe.GetStationType(iDefender);
		if (!bAll && pDefenderType->GetLevel() == 0)
			continue;

		if (!pDefenderType->MatchesCriteria(DefenderCriteria))
			continue;

		//	Compute an enemy of the station

		CSovereign *pAttackerSovereign = GetAttackerSovereign(Universe, pDefenderType);
		if (pAttackerSovereign == NULL)
			{
			printf("ERROR: Unable to find enemy sovereign of encounter: %s [%x].\n", pDefenderType->GetNounPhrase().GetASCIIZPointer(), pDefenderType->GetUNID());
			return;
			}

		printf("%s [%x]\t%d", pDefenderType->GetNounPhrase().GetASCIIZPointer(), pDefenderType->GetUNID(), pDefenderType->GetLevel());

		//	Loop over all attackers

		for (iAttacker = 0; iAttacker < Universe.GetShipClassCount(); iAttacker++)
			{
			CShipClass *pAttackerClass = Universe.GetShipClass(iAttacker);
			if (!pAttackerClass->MatchesCriteria(AttackerCriteria))
				continue;

			//	Do several runs so we can get some statistical data

			int iAttackerSurvived = 0;
			int iRun;
			for (iRun = 0; iRun < iRuns; iRun++)
				{
				EResults iResult = RunEncounter(Universe, Viewer, pDefenderType, pAttackerClass, pAttackerSovereign);
				if (iResult == resultError)
					return;

				//	Did the attacker survive?

				if (iResult != resultAttackerDestroyed)
					iAttackerSurvived++;
				}

			//	Output results for this attacker

			int iSurvivability = 100 * iAttackerSurvived / iRuns;
			printf("\t%d%%", iSurvivability);
			}

		printf("\n");
		}

	Viewer.Destroy();
	}

CSovereign *GetAttackerSovereign (CUniverse &Universe, CStationType *pDefenderType)
	{
	int i;

	CSovereign *pDefenderSovereign = pDefenderType->GetSovereign();
	if (pDefenderSovereign)
		{
		for (i = 0; i < Universe.GetSovereignCount(); i++)
			{
			CSovereign *pSovereign = Universe.GetSovereign(i);
			if (pDefenderSovereign->IsEnemy(pSovereign))
				return pSovereign;
			}
		}

	return NULL;
	}

EResults RunEncounter (CUniverse &Universe, CSimViewer &Viewer, CStationType *pDefenderType, CShipClass *pAttackerClass, CSovereign *pAttackerSovereign)
	{
	int iTimeOut = DEFAULT_TIME_OUT;

	//	Make sure the universe is clean

	CString sError;
	if (Universe.InitGame(0, &sError) != NOERROR)
		{
		printf("ERROR: %s", sError.GetASCIIZPointer());
		return resultError;
		}

	//	Create an empty system

	CSystem *pSystem;
	if (Universe.CreateEmptyStarSystem(&pSystem) != NOERROR)
		{
		printf("ERROR: Unable to create empty star system.\n");
		return resultError;
		}

	//	Create a station in the center of the system

	CSpaceObject *pStation;
	if (pSystem->CreateStation(pDefenderType, NULL, CVector(), &pStation) != NOERROR)
		{
		printf("ERROR: Unable to create station.\n");
		return resultError;
		}

	//	Set the POV

	Universe.SetPOV(pStation);
	pSystem->SetPOVLRS(pStation);

	//	Prepare system

	Universe.UpdateExtended();
	Universe.GarbageCollectLibraryBitmaps();
	Universe.StartGame(true);

	//	Now create an attacker some distance away

	CVector vPos = PolarToVector(mathRandom(0, 359), INITIAL_DISTANCE);

	CShip *pAttacker;
	if (pSystem->CreateShip(pAttackerClass->GetUNID(), NULL, NULL, pAttackerSovereign, vPos, CVector(), 0, NULL, NULL, &pAttacker) != NOERROR)
		{
		printf("ERROR: Unable to create attacking ship.\n");
		return resultError;
		}

	//	Set the attacker to attack the station

	IShipController *pController = pAttacker->GetController();
	if (pController == NULL)
		{
		printf("ERROR: No controller for ship.\n");
		return resultError;
		}

	pController->AddOrder(IShipController::orderAttackStation, pStation, IShipController::SData());

	//	Watch the attacker

	Universe.SetPOV(pAttacker);
	pSystem->SetPOVLRS(pAttacker);

	//	Update context

	SSystemUpdateCtx Ctx;
	Ctx.bForceEventFiring = true;
	Ctx.bForcePainted = true;

	//	Now keep updating until either the station is destroyed, the ship is destroyed, or we time-out

	int iTime = 0;
	int iDestroyedTime = (Viewer.IsEmpty() ? 0 : DESTROY_TIME);
	bool bDestroyed = false;
	EResults iResult = resultTimeout;
	while (iTime < iTimeOut && (!bDestroyed || iDestroyedTime > 0))
		{
		iTime++;
		Universe.Update(Ctx);

		if (!Viewer.IsEmpty())
			Viewer.PaintViewport(Universe);

		if (bDestroyed)
			iDestroyedTime--;
		else if (pStation->IsDestroyed() || pStation->IsAbandoned())
			{
			bDestroyed = true;
			iResult = resultDefenderDestroyed;
			}
		else if (pAttacker->IsDestroyed())
			{
			bDestroyed = true;
			iResult = resultAttackerDestroyed;
			}
		}

	//	Done

	return iResult;
	}
