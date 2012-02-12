//	CreateUniverse.cpp
//
//	Implements Universe creation routines

#include "Alchemy.h"
#include "NPEngine.h"

//	Universe creation constants

const int g_iSpacing = 4;				//	Controls the spacing between null points

enum HexDirections
	{
	hexNorth = 0,
	hexNorthEast,
	hexSouthEast,
	hexSouth,
	hexSouthWest,
	hexNorthWest
	};

const int g_iHexDirectionsCount = 6;

const int MAX_INCOMPATIBLE = 6;

CXMLElement *FindTraitInTable (CXMLElement *pTraitTable, const CString &sTrait);
CXMLElement *FindElement (CXMLElement *pParent, const CString &sTag, CString *retsError);

//	Local functions

bool AddTrait (CNPWorld *pWorld, 
			   CXMLElement *pTraitTable, 
			   CXMLElement *pTrait, 
			   CSymbolTable &Symbols)
	{
	int i;

	//	First check to see if the world has any of the traits
	//	listed in the "unless" sections

	for (i = 0; i < pTrait->GetContentElementCount(); i++)
		{
		CXMLElement *pUnless = pTrait->GetContentElement(i);
		if (strCompare(pUnless->GetTag(), CONSTLIT("Unless")) != 0)
			continue;

		//	If the world has the prohibited trait then we cannot
		//	add the desired trait.

		if (pWorld->HasTrait(GetSymbolicAttribute(Symbols, pUnless, CONSTLIT("Trait"))))
			return false;
		}

	//	Add the trait

	pWorld->SetTrait(GetSymbolicAttribute(Symbols, pTrait, CONSTLIT("Trait")));

	//	Add any traits that are implied by this trait

	for (i = 0; i < pTrait->GetContentElementCount(); i++)
		{
		CXMLElement *pImply = pTrait->GetContentElement(i);
		if (strCompare(pImply->GetTag(), CONSTLIT("Imply")) != 0)
			continue;

		//	Random chance of actually having this trait

		if (mathRandom(1, 100) > pImply->GetAttributeInteger(CONSTLIT("Prob")))
			continue;

		//	If we already have this trait the don't bother

		if (pWorld->HasTrait(GetSymbolicAttribute(Symbols, pImply, CONSTLIT("Trait"))))
			continue;

		//	Look for the implied trait in the table

		CXMLElement *pNewTrait = FindTraitInTable(pTraitTable, pImply->GetAttribute(CONSTLIT("Trait")));

		//	Add it. Note that we don't care if we cannot

		if (pNewTrait)
			AddTrait(pWorld, pTraitTable, pNewTrait, Symbols);
		}

	return true;
	}

CXMLElement *FindElement (CXMLElement *pParent, const CString &sTag, CString *retsError)
	{
	CXMLElement *pElement = pParent->GetContentElementByTag(sTag);
	if (pElement == NULL)
		{
		*retsError = strPatternSubst(CONSTLIT("<%s> tag not found"), sTag);
		return NULL;
		}

	return pElement;
	}

CXMLElement *FindTraitInTable (CXMLElement *pTraitTable, const CString &sTrait)
	{
	for (int i = 0; i < pTraitTable->GetContentElementCount(); i++)
		{
		CXMLElement *pTrait = pTraitTable->GetContentElement(i);

		if (strCompare(pTrait->GetAttribute(CONSTLIT("Trait")), sTrait) == 0)
			return pTrait;
		}

	return NULL;
	}

int GetSymbolicAttribute (CSymbolTable &Symbols, CXMLElement *pElement, const CString &sAttribute)
	{
	CString sValue = pElement->GetAttribute(sAttribute);
	int iValue;

	if (Symbols.Lookup(sValue, (CObject **)&iValue) == NOERROR)
		return iValue;
	else
		return strToInt(sValue, 0, NULL);
	}

bool IsWorldSuitableAsCapital (CNPUniverse *pUniv, CNPWorld *pWorld)
	{
	//	Must be AI tech

	if (pWorld->GetTechLevel() != techAI)
		return false;

	//	We better have a reasonable efficiency

	if (pWorld->GetEfficiency() < 50)
		return false;

	//	Must have a good population

	if (pWorld->GetPopulation() < 6000000)
		return false;

	//	The world better not have major disadvantages

	if (pWorld->HasTrait(traitAirless)
			|| pWorld->HasTrait(traitRadioactive))
		return false;

	return true;
	}

void HexMove (int x, int y, int iDir, int *retx, int *rety)
	{
	const int xOff[g_iHexDirectionsCount] = { 0, +1, +1, 0, -1, -1 };
	const int yEvenOff[g_iHexDirectionsCount] = { +1, +1, 0, -1, 0, +1 };
	const int yOddOff[g_iHexDirectionsCount]  = { +1, 0, -1, -1, -1, 0 };

	ASSERT(iDir >= 0 && iDir < g_iHexDirectionsCount);

	*retx = x + xOff[iDir];
	*rety = y + ((x % 2) == 0 ? yEvenOff[iDir] : yOddOff[iDir]);
	}

//	Member Functions ----------------------------------------------------------

ALERROR CNPUniverse::AddRandomTrait (CreateCtx &Ctx,
									 CNPWorld *pWorld, 
									 TraitType iSection, 
									 int iCount, 
									 CXMLElement *pTraitTable)

//	AddRandomTrait
//
//	Adds traits from the given table

	{
	int iSectionStart;

	//	Find the beginning of the section

	iSectionStart = 0;
	while (iSectionStart < pTraitTable->GetContentElementCount()
			&& pTraitTable->GetContentElement(iSectionStart)->GetAttributeInteger(CONSTLIT("Table")) != iSection)
		iSectionStart++;

	//	If not found, we fail

	if (iSectionStart == pTraitTable->GetContentElementCount())
		{
		Ctx.sError = LITERAL("Unable to find appropriate trait table section");
		return ERR_FAIL;
		}

	//	Keep looping for each trait to add

	for (int i = 0; i < iCount; i++)
		{
		//	Keep trying until we successfully add some trait or
		//	until we get sick of trying.

		for (int j = 0; j < 10; j++)
			{
			int iRoll = mathRandom(1, 100);
			int iEntry = iSectionStart;
			CXMLElement *pEntry;

			while (iEntry < pTraitTable->GetContentElementCount()
					&& (pEntry = pTraitTable->GetContentElement(iEntry))
					&& (pEntry->GetAttributeInteger(CONSTLIT("Table")) == iSection)
					&& (iRoll = iRoll - pEntry->GetAttributeInteger(CONSTLIT("Prob"))) > 0)
				iEntry++;

			//	If not found, we fail

			if (iRoll > 0)
				{
				Ctx.sError = LITERAL("Trait table probabilities do not add up to 100%");
				return ERR_FAIL;
				}

			//	Add the trait. If we add it successfully then we stop
			//	the inner loop.

			if (AddTrait(pWorld, pTraitTable, pEntry, *Ctx.pSymbols))
				break;
			}
		}

	return NOERROR;
	}

ALERROR CNPUniverse::CreateFleet (CNPNullPoint *pNP, CNPSovereign *pSovereign, CNPFleet **retpFleet)

//	CreateFleet
//
//	Creates a new empty fleet

	{
	ALERROR error;
	CNPFleet *pFleet;

	if (error = CNPFleet::Create(m_Fleets.RegisterEntry(), pNP->GetUNID(), pSovereign, &pFleet))
		return error;

	m_Fleets.SetEntry(pFleet->GetUNID(), pFleet);

	//	Sovereign knowns about the fleet

	pSovereign->SetKnowledge(pFleet->GetUNID());

	//	Add to null point

	pNP->PlaceFleet(pFleet);

	//	Done

	if (retpFleet)
		*retpFleet = pFleet;

	return NOERROR;
	}

ALERROR CNPUniverse::CreateNullPointNetwork (int iNullPoints)

//	CreateNullPointNetwork
//
//	Creates a network of null points

	{
	ALERROR error;
	int iSide, iNullPointsCreated, x, y, iNextNP;
	CNPNullPoint **pSpace = NULL;
	CObjectArray NPStack(false);

	//	We start by figuring out how
	//	much space we need to fit all the null points

	iSide = g_iSpacing * mathSqrt(iNullPoints);

	//	Now create an array and initialize it to 0

	pSpace = new CNPNullPoint * [iSide * iSide];
	ZeroMemory(pSpace, iSide * iSide * sizeof(DWORD));

	//	Prepare a stack of null points that need connections
	//	We start with a single null point at the center of the space

	x = iSide / 2;
	y = iSide / 2;
	if (error = CNPNullPoint::Create(m_NullPoints.RegisterEntry(),
			x, y, &pSpace[x * iSide + y]))
		goto Fail;

	m_NullPoints.SetEntry(pSpace[x * iSide + y]->GetUNID(), pSpace[x * iSide + y]);
		
	if (error = NPStack.AppendObject(pSpace[x * iSide + y], NULL))
		goto Fail;

	iNextNP = 0;

	//	Pull objects from the stack and move to random hexes creating
	//	new null points. Keep on looping until we're done

	iNullPointsCreated = 1;
	while (iNullPointsCreated < iNullPoints)
		{
		//	Pull a null point from the stack

		CNPNullPoint *pNullPoint = (CNPNullPoint *)NPStack.GetObject(iNextNP++);

		//	Figure out how many connections we want out of here. The probability
		//	is:
		//		1/6 chance: 1 link
		//		1/3 chance: 2 links
		//		1/3 chance: 3 links
		//		1/6 chance: 4 links

		int iConnections = mathRandom(1, 2) + mathRandom(0, 2);

		//	Make sure we never create more than 6 connections

		iConnections = min(iConnections, g_iHexDirectionsCount - pNullPoint->GetLinkCount());

		//	Create the connections

		while (iConnections > 0 && iNullPointsCreated < iNullPoints)
			{
			//	Create a connection in a random direction that
			//	we don't already have a connection to.

			do
				{
				HexMove(pNullPoint->GetX(), 
						pNullPoint->GetY(),
						mathRandom(0, g_iHexDirectionsCount - 1),
						&x,
						&y);
				ASSERT(x >= 0 && y >= 0 && x < iSide && y < iSide);
				}
			while (pSpace[x * iSide + y] && pNullPoint->IsLinkedTo(pSpace[x * iSide + y]));

			//	If the destination is empty then add a null point there

			if (pSpace[x * iSide + y] == NULL)
				{
				if (error = CNPNullPoint::Create(m_NullPoints.RegisterEntry(),
						x, y, &pSpace[x * iSide + y]))
					goto Fail;

				m_NullPoints.SetEntry(pSpace[x * iSide + y]->GetUNID(), pSpace[x * iSide + y]);

				//	If we're not at the edge of space, add the null point
				//	to the stack so that we can connect this one too.

				if (x > 0 && y > 0 && x < iSide-1 && y < iSide-1)
					{
					if (error = NPStack.AppendObject(pSpace[x * iSide + y], NULL))
						goto Fail;
					}

				iNullPointsCreated++;
				}

			//	Now connect to the destination

			if (error = pNullPoint->CreateLinkTo(pSpace[x * iSide + y]))
				goto Fail;

			iConnections--;
			}
		}

	delete [] pSpace;
	pSpace = NULL;

	return NOERROR;

Fail:

	if (pSpace)
		delete [] pSpace;

	return error;
	}

ALERROR CNPUniverse::CreateRandomWorld (CreateCtx &Ctx, CNPNullPoint *pNP, CNPSovereign *pSovereign)

//	CreateRandomWorld
//
//	Creates a random world at the given null point

	{
	ALERROR error;
	CNPWorld *pWorld;
	int i, iRoll;

	//	Create the world

	if (error = CNPWorld::Create(m_Worlds.RegisterEntry(),
			pNP->GetUNID(),
			pSovereign,
			&pWorld))
		{
		Ctx.sError = LITERAL("Out of memory");
		return error;
		}

	m_Worlds.SetEntry(pWorld->GetUNID(), pWorld);

	//	Add to the null point

	if (error = pNP->AddWorld(pWorld))
		{
		delete pWorld;
		Ctx.sError = LITERAL("Out of memory");
		return error;
		}

	//	Add some traits to the world

	CXMLElement *pTraitTable = Ctx.pTemplate->GetContentElementByTag(CONSTLIT("DefineWorldTraitTable"));

	if (mathRandom(1, 100) <= 20)
		AddRandomTrait(Ctx, pWorld, ttMajorDisadvantage, 1, pTraitTable);

	if (mathRandom(1, 100) <= 35)
		AddRandomTrait(Ctx, pWorld, ttMinorDisadvantage, 1, pTraitTable);

	if (mathRandom(1, 100) <= 20)
		AddRandomTrait(Ctx, pWorld, ttMinorDisadvantage, 1, pTraitTable);


	if (mathRandom(1, 100) <= 20)
		AddRandomTrait(Ctx, pWorld, ttMajorAdvantage, 1, pTraitTable);

	if (mathRandom(1, 100) <= 35)
		AddRandomTrait(Ctx, pWorld, ttMinorAdvantage, 1, pTraitTable);

	if (mathRandom(1, 100) <= 20)
		AddRandomTrait(Ctx, pWorld, ttMinorAdvantage, 1, pTraitTable);

	//	Pick a tech level for the world

	CXMLElement *pTechTable = Ctx.pTemplate->GetContentElementByTag(CONSTLIT("DefineWorldTechLevelTable"));
	iRoll = mathRandom(1, 100);
	NPTechLevels iMinTech = pWorld->GetMinTech();
	for (i = 0; i < pTechTable->GetContentElementCount(); i++)
		{
		CXMLElement *pEntry = pTechTable->GetContentElement(i);
		int iProb = pEntry->GetAttributeInteger(CONSTLIT("Prob"));
		if (iRoll <= iProb)
			{
			NPTechLevels iTechLevel = (NPTechLevels)GetSymbolicAttribute(*Ctx.pSymbols, pEntry, CONSTLIT("Tech"));

			//	Make sure this tech level is appropriate for the world

			while (iTechLevel < iMinTech)
				iTechLevel = (NPTechLevels)(iTechLevel + 1);

			pWorld->SetTechLevel(iTechLevel);
			break;
			}

		iRoll -= iProb;
		}

	//	Set the initial population

	int iPopulation = 0;
	switch (pWorld->GetTechLevel())
		{
		case techAgricultural:
			iPopulation = mathRandom(250, 750);
			break;

		case techSteam:
		case techCyber:
			{
			for (i = 0; i < 12; i++)
				iPopulation += mathRandom(0, 500);
			break;
			}

		case techBio:
			{
			for (i = 0; i < 10; i++)
				iPopulation += mathRandom(0, 1000);
			break;
			}

		case techFusion:
		case techAI:
			{
			for (i = 0; i < 8; i++)
				iPopulation += mathRandom(0, 1600);
			break;
			}

		default:
			{
			for (i = 0; i < 10; i++)
				iPopulation += mathRandom(0, 2000);
			break;
			}
		}

	pWorld->SetPopulation((1000 * iPopulation) + mathRandom(-500, 500));

	//	Set the efficiency

	pWorld->SetEfficiency(25 + mathRandom(0, 25) 
			+ mathRandom(0, 25) 
			+ mathRandom(0, 25));

	//	Figure out if this world is a suitable candidate for a capital

	if (IsWorldSuitableAsCapital(this, pWorld))
		pWorld->SetTrait(traitReservedCapital);

	return NOERROR;
	}

ALERROR CNPUniverse::CreateRandomWorlds (CreateCtx &Ctx)

//	CreateRandomWorlds
//
//	Creates a random world in every null point

	{
	ALERROR error;
	int i;
	CNPSovereign *pIndep = GetSovereign(m_dwIndependent);

	for (i = 0; i < m_NullPoints.GetCount(); i++)
		{
		CNPNullPoint *pNP = m_NullPoints.GetEntryByIndex(i);

		if (error = CreateRandomWorld(Ctx, pNP, pIndep))
			return error;
		}

	return NOERROR;
	}

ALERROR CNPUniverse::CreateSovereign (CString sName, CNPWorld *pCapital, CXMLElement *pSovTemplate, CNPSovereign **retpSovereign)

//	CreateSovereign
//
//	Creates a new sovereign

	{
	ALERROR error;
	CNPSovereign *pSovereign;
	int i;

	if (error = CNPSovereign::Create(m_Sovereigns.RegisterEntry(), sName, &pSovereign))
		return error;

	m_Sovereigns.SetEntry(pSovereign->GetUNID(), pSovereign);

	//	Obviously we know about ourselves

	pSovereign->SetKnowledge(pSovereign->GetUNID());

	//	Set a capital

	if (pCapital)
		{
		pSovereign->SetCapital(pCapital->GetUNID());

		pCapital->RemoveTrait(traitReservedCapital);
		pCapital->SetTrait(traitCapital);
		pCapital->SetSovereign(pSovereign);

		//	Scan the capital

		CNPNullPoint *pNP = GetNullPoint(pCapital->GetLocation());
		ScanNullPoint(pSovereign, pNP, 1);

		//	Scan the area around the capital

		for (i = 0; i < pNP->GetLinkCount(); i++)
			{
			CNPNullPoint *pDest = pNP->GetLinkDest(i);
			ScanNullPoint(pSovereign, pDest, 1);
			}

		//	Create ships

		if (pSovTemplate)
			{
			CNPFleet *pFleet = NULL;
			for (i = 0; i < pSovTemplate->GetContentElementCount(); i++)
				{
				CXMLElement *pShip = pSovTemplate->GetContentElement(i);
				if (strCompare(pShip->GetTag(), CONSTLIT("Unit")) == 0)
					{
					//	Create the fleet, if needed

					if (pFleet == NULL)
						if (error = CreateFleet(pNP, pSovereign, &pFleet))
							return error;

					//	Add the ship to the fleet

					CNPUnit Unit(pShip->GetAttributeInteger(CONSTLIT("Class")), 0, 0);
					pFleet->GetAssetList().AddUnit(Unit);
					}
				}
			}
		}

	//	Done

	if (retpSovereign)
		*retpSovereign = pSovereign;

	return NOERROR;
	}

ALERROR CNPUniverse::InitFromXML (CXMLElement *pTemplate, CSymbolTable &Symbols, CString *retsError)

//	InitFromXML
//
//	Initializes from an XML template

	{
	ALERROR error;
	int iNullPoints;
	CreateCtx Ctx;

	//	Initialize context

	Ctx.pTemplate = pTemplate;
	Ctx.pSymbols = &Symbols;

	//	Get some elements

	CXMLElement *pOptions = pTemplate->GetContentElementByTag(CONSTLIT("DefineOptions"));

	//	Initialize

	iNullPoints = pOptions->GetAttributeInteger(CONSTLIT("NullPointCount"));
	m_iMaxPlayers = pOptions->GetAttributeInteger(CONSTLIT("MaxPlayers"));
	m_iBaseAnno = pOptions->GetAttributeInteger(CONSTLIT("BaseAnno"));
	m_iTurn = 0;

	//	Create the Overlord sovereign

	CNPSovereign *pOverlord;
	if (error = CreateSovereign(LITERAL("The Universe"), NULL, NULL, &pOverlord))
		{
		*retsError = LITERAL("Unable to create Overlord sovereign");
		return error;
		}

	m_dwOverlord = pOverlord->GetUNID();

	//	Create the default, independent sovereign

	CNPSovereign *pIndep;
	if (error = CreateSovereign(LITERAL("Independent"), NULL, NULL, &pIndep))
		{
		*retsError = LITERAL("Unable to create independent sovereign");
		return error;
		}

	m_dwIndependent = pIndep->GetUNID();

	//	Create the null point network.

	if (error = CreateNullPointNetwork(iNullPoints))
		{
		*retsError = LITERAL("Unable to create null point network");
		return error;
		}

	//	Create worlds

	if (error = CreateRandomWorlds(Ctx))
		{
		*retsError = LITERAL("Unable to create worlds");
		return error;
		}

	//	Next turn

	m_dwNextTurn = ::GetTickCount() + (DWORD)(SecondsPerOro * 1000);

	return NOERROR;
	}

ALERROR CNPUniverse::LoadClassesFromXML (CXMLElement *pTemplate, CSymbolTable &Symbols, CString *retsError)

//	LoadClassesFromXML
//
//	Loads the unit classes, etc.

	{
	ALERROR error;
	CXMLElement *pSection;
	CXMLElement *pTable;
	int i;

	//	Load universal constants section

	if ((pSection = FindElement(pTemplate, CONSTLIT("DefineUniversalConstants"), retsError)) == NULL)
		return ERR_FAIL;

	//	Load tech table

	if ((pTable = FindElement(pSection, CONSTLIT("TechTable"), retsError)) == NULL)
		return ERR_FAIL;

	if (pTable->GetContentElementCount() != techCount - 1)
		{
		*retsError = CONSTLIT("<TechTable> does not have the right number of entries.");
		return ERR_FAIL;
		}

	for (i = 0; i < pTable->GetContentElementCount(); i++)
		{
		CXMLElement *pEntry = pTable->GetContentElement(i);
		m_TechTable[i + 1].sName = pEntry->GetAttribute(CONSTLIT("Name"));
		m_TechTable[i + 1].iProductionFactor = pEntry->GetAttributeInteger(CONSTLIT("Prod"));
		m_TechTable[i + 1].iStdLivingFactor = pEntry->GetAttributeInteger(CONSTLIT("StdLiving"));
		m_TechTable[i + 1].iPollutionFactor = pEntry->GetAttributeInteger(CONSTLIT("Pollution"));
		}

	//	Load unit classes

	if ((pSection = FindElement(pTemplate, CONSTLIT("DefineUnitClasses"), retsError)) == NULL)
		return ERR_FAIL;

	for (i = 0; i < pSection->GetContentElementCount(); i++)
		{
		CNPUnitClass *pClass = new CNPUnitClass;
		if (pClass == NULL)
			{
			*retsError = CONSTLIT("Out of memory");
			return ERR_FAIL;
			}

		if (error = pClass->LoadFromXML(pSection->GetContentElement(i), Symbols))
			{
			*retsError = CONSTLIT("Unable to load unit class");
			return ERR_FAIL;
			}

		m_UnitClasses.SetEntry(pClass->GetUNID(), pClass);
		}

	//	Get the default sovereign

	if ((pSection = FindElement(pTemplate, CONSTLIT("DefineSovereigns"), retsError)) == NULL)
		return ERR_FAIL;

	if ((m_pPlayerTemplate = FindElement(pSection, CONSTLIT("DefineDefaultPlayer"), retsError)) == NULL)
		return ERR_FAIL;

	m_pPlayerTemplate = m_pPlayerTemplate->OrphanCopy();

	return NOERROR;
	}
