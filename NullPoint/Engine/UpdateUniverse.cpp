//	UpdateUniverse.cpp
//
//	Implements Universe update routines

#include "Alchemy.h"
#include "NPEngine.h"

bool CNPUniverse::Update (void)

//	Update
//
//	Updates the universe. This function should be called approximately
//	every second.

	{
	int i;

	//	If it is not yet time to update then we're done. Note that
	//	we handle the case where m_dwNextTurn has rolled-over.

	DWORD dwNow = ::GetTickCount();
	if ((dwNow < m_dwNextTurn) || (dwNow - m_dwNextTurn > 100000))
		return false;

	//	Move fleets

	for (i = 0; i < m_Fleets.GetCount(); i++)
		{
		CNPFleet *pFleet = m_Fleets.GetEntryByIndex(i);
		if (pFleet->InHyperspace())
			{
			pFleet->SetInTransit(pFleet->GetInTransit() - 1);
			if (!pFleet->InHyperspace())
				{
				CNPNullPoint *pPos = GetNullPoint(pFleet->GetLocation());
				CNPNullPoint *pDest = GetNullPoint(pFleet->GetDest());

				pPos->RemoveFleet(pFleet);
				pDest->PlaceFleet(pFleet);
				pFleet->SetLocation(pDest);

				ScanNullPoint(pFleet->GetSovereign(), pDest, 1);
				}
			}
		}

	//	Update the worlds

	for (i = 0; i < m_Worlds.GetCount(); i++)
		{
		CNPWorld *pWorld = m_Worlds.GetEntryByIndex(i);

		//	Update once every 10 dias (10 times per day realtime)

		if (CycleTime(pWorld->GetDestiny(), 10 * OrosPerDia))
			UpdateWorld(pWorld);
		}

	//	Update time

	m_iTurn++;
	m_dwNextTurn += (DWORD)(SecondsPerOro * 1000);

	return true;
	}

void CNPUniverse::UpdateWorld (CNPWorld *pWorld)

//	UpdateWorld
//
//	Updates the world. This function should be called on each world
//	every 10 dias

	{
	//	Get some constants

	TechTableEntry *pTechData = &m_TechTable[pWorld->GetTechLevel()];

	//	Calculate the population of the world in millions.

	double rPopulation = (double)pWorld->GetPopulation() / 1000.0;

	//	Calculate the raw labor of the world. The labor is proportional
	//	to the population, but we add some factor at the low-end.

	double rLabor = 500.0 + rPopulation;

	//	Calculate the adjusted labor available to the world. This is
	//	the total population of the world times a factor based
	//	on the tech level and the efficiency.

	double rAdjustedLabor = rLabor 
			* ((double)pTechData->iProductionFactor / 10.0)
			* (0.5 + ((double)pWorld->GetEfficiency() / 200.0));

	//	Calculate industrial production (IP) by taking rAdjustedLabor and
	//	compensating for various factors.

	double rIP = rAdjustedLabor;

	if (pWorld->HasTrait(traitNeothals))
		rIP += rIP * 0.5;

	if (pWorld->HasTrait(traitDesert))
		rIP -= rIP * 0.1;

	if (pWorld->HasTrait(traitOcean))
		rIP += rIP * 0.1;

	if (pWorld->HasTrait(traitPermafrost))
		rIP -= rIP * 0.2;

	//	Calculate how much labor is required to maintain the expected
	//	standard of living for this tech level.

	double rStdLivingLabor = rPopulation * ((double)pTechData->iStdLivingFactor / 10.0);

	//	Adjust standard of living labor based on various factors

	if (pWorld->HasTrait(traitFertile))
		rStdLivingLabor -= rStdLivingLabor * 0.2;

	if (pWorld->HasTrait(traitAirless))
		rStdLivingLabor += rStdLivingLabor * 0.25;

	if (pWorld->HasTrait(traitRadioactive))
		rStdLivingLabor += rStdLivingLabor * 0.1;

	//	If the labor required to maintain the expected standard of living
	//	is less than the labor available then we are in trouble.

	if (rStdLivingLabor < rIP)
		{

		}
	}
