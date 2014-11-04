//	CShipTable.cpp
//
//	CShipTable object

#include "PreComp.h"

#define ESCORTS_TAG					CONSTLIT("Escorts")
#define GROUP_TAG					CONSTLIT("Group")
#define INITIAL_DATA_TAG			CONSTLIT("InitialData")
#define ITEMS_TAG					CONSTLIT("Items")
#define LEVEL_TABLE_TAG				CONSTLIT("LevelTable")
#define LOOKUP_TAG					CONSTLIT("Lookup")
#define NAMES_TAG					CONSTLIT("Names")
#define NULL_TAG					CONSTLIT("Null")
#define ON_CREATE_TAG				CONSTLIT("OnCreate")
#define SHIP_TAG					CONSTLIT("Ship")
#define SHIPS_TAG					CONSTLIT("Ships")
#define TABLE_TAG					CONSTLIT("Table")

#define CHANCE_ATTRIB				CONSTLIT("chance")
#define CLASS_ATTRIB				CONSTLIT("class")
#define CONTROLLER_ATTRIB			CONSTLIT("controller")
#define COUNT_ATTRIB				CONSTLIT("count")
#define EVENT_HANDLER_ATTRIB		CONSTLIT("eventHandler")
#define ITEM_TABLE_ATTRIB			CONSTLIT("itemTable")
#define LEVEL_FREQUENCY_ATTRIB		CONSTLIT("levelFrequency")
#define MAX_SHIPS_ATTRIB			CONSTLIT("maxShips")
#define NAME_ATTRIB					CONSTLIT("name")
#define ORDERS_ATTRIB				CONSTLIT("orders")
#define PATROL_DIST_ATTRIB			CONSTLIT("patrolDist")
#define SOVEREIGN_ATTRIB			CONSTLIT("sovereign")
#define TABLE_ATTRIB				CONSTLIT("table")
#define UNID_ATTRIB					CONSTLIT("unid")

class CLevelTableOfShipGenerators : public IShipGenerator
	{
	public:
		virtual ~CLevelTableOfShipGenerators (void);
		virtual void AddTypesUsed (TSortMap<DWORD, bool> *retTypesUsed);
		virtual void CreateShips (SShipCreateCtx &Ctx);
		virtual Metric GetAverageLevelStrength (int iLevel);
		virtual ALERROR LoadFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);
		virtual ALERROR OnDesignLoadComplete (SDesignLoadCtx &Ctx);
		virtual ALERROR ValidateForRandomEncounter (void);

	private:
		struct SEntry
			{
			SEntry (void) :
					pEntry(NULL),
					iChance(0)
				{ }

			IShipGenerator *pEntry;
			CString sLevelFrequency;
			int iChance;
			};

		DiceRange m_Count;
		TArray<SEntry> m_Table;
		int m_iComputedLevel;
		int m_iTotalChance;
	};

class CLookupShipTable : public IShipGenerator
	{
	public:
		virtual void AddTypesUsed (TSortMap<DWORD, bool> *retTypesUsed);
		virtual void CreateShips (SShipCreateCtx &Ctx);
		virtual Metric GetAverageLevelStrength (int iLevel) { return m_Count.GetAveValueFloat() * m_pTable->GetAverageLevelStrength(iLevel); }
		virtual ALERROR LoadFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);
		virtual ALERROR OnDesignLoadComplete (SDesignLoadCtx &Ctx);

	private:
		DiceRange m_Count;							//	Number of ships to create
		CShipTableRef m_pTable;
	};

class CNullShip : public IShipGenerator
	{
	};

class CSingleShip : public IShipGenerator
	{
	public:
		CSingleShip (void);
		virtual ~CSingleShip (void);
		virtual void AddTypesUsed (TSortMap<DWORD, bool> *retTypesUsed);
		virtual void CreateShips (SShipCreateCtx &Ctx);
		virtual Metric GetAverageLevelStrength (int iLevel);
		virtual ALERROR LoadFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);
		virtual ALERROR OnDesignLoadComplete (SDesignLoadCtx &Ctx);
		virtual ALERROR ValidateForRandomEncounter (void);

	private:
		void CreateShip (SShipCreateCtx &Ctx,
						 CSovereign *pSovereign,
						 const CVector &vPos,
						 CSpaceObject *pExitGate,
						 CShip **retpShip = NULL);

		DiceRange m_Count;							//	Number of ships to create
		int m_iMaxCountInSystem;					//	Do not exceed this number of ship of this class in system (or -1)

		CShipClassRef m_pShipClass;					//	Ship class to create
		CSovereignRef m_pSovereign;					//	Sovereign
		CString m_sShipName;						//	Name of ship
		DWORD m_dwShipNameFlags;					//	Flags for ship name

		IItemGenerator *m_pItems;					//	Random table of items (or NULL)
		IShipGenerator *m_pEscorts;					//	Random table of escorts (or NULL)
		CAttributeDataBlock m_InitialData;			//	Initial data for ship
		ICCItem *m_pOnCreate;						//	Create call

		CDesignTypeRef<CDesignType> m_pOverride;	//	Override (event handler)
		CString m_sController;						//	Controller to use (or "" to use default)
		IShipController::OrderTypes m_iOrder;		//	Ship order
		IShipController::SData m_OrderData;			//	Order data
	};

class CTableOfShipGenerators : public IShipGenerator
	{
	public:
		CTableOfShipGenerators (void) : m_iTotalChance(0) { }
		virtual ~CTableOfShipGenerators (void);
		virtual void AddTypesUsed (TSortMap<DWORD, bool> *retTypesUsed);
		virtual void CreateShips (SShipCreateCtx &Ctx);
		virtual Metric GetAverageLevelStrength (int iLevel);
		virtual ALERROR LoadFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);
		virtual ALERROR OnDesignLoadComplete (SDesignLoadCtx &Ctx);
		virtual ALERROR ValidateForRandomEncounter (void);

	private:
		struct SEntry
			{
			SEntry (void) :
					pEntry(NULL),
					iChance(0)
				{ }

			IShipGenerator *pEntry;
			int iChance;
			};

		DiceRange m_Count;
		TArray<SEntry> m_Table;
		int m_iTotalChance;
	};

class CGroupOfShipGenerators : public IShipGenerator
	{
	public:
		CGroupOfShipGenerators (void) : m_Table(NULL) { }
		virtual ~CGroupOfShipGenerators (void);
		virtual void AddTypesUsed (TSortMap<DWORD, bool> *retTypesUsed);
		virtual void CreateShips (SShipCreateCtx &Ctx);
		virtual Metric GetAverageLevelStrength (int iLevel);
		virtual ALERROR LoadFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);
		virtual ALERROR OnDesignLoadComplete (SDesignLoadCtx &Ctx);
		virtual ALERROR ValidateForRandomEncounter (void);

	private:
		struct SEntry
			{
			IShipGenerator *pEntry;
			int iChance;
			};

		DiceRange m_Count;

		int m_iTableCount;
		SEntry *m_Table;
	};

ALERROR IShipGenerator::CreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, IShipGenerator **retpGenerator)

//	CreateFromXML
//
//	Creates a new generator

	{
	ALERROR error;
	IShipGenerator *pGenerator = NULL;

	if (strEquals(pDesc->GetTag(), SHIP_TAG))
		pGenerator = new CSingleShip;
	else if (strEquals(pDesc->GetTag(), TABLE_TAG))
		pGenerator = new CTableOfShipGenerators;
	else if (strEquals(pDesc->GetTag(), LEVEL_TABLE_TAG))
		pGenerator = new CLevelTableOfShipGenerators;
	else if (strEquals(pDesc->GetTag(), GROUP_TAG) 
			|| strEquals(pDesc->GetTag(), SHIPS_TAG) 
			|| strEquals(pDesc->GetTag(), ESCORTS_TAG))
		pGenerator = new CGroupOfShipGenerators;
	else if (strEquals(pDesc->GetTag(), NULL_TAG))
		pGenerator = new CNullShip;
	else if (strEquals(pDesc->GetTag(), LOOKUP_TAG))
		pGenerator = new CLookupShipTable;
	else
		{
		Ctx.sError = strPatternSubst(CONSTLIT("Unknown ship generator: %s"), pDesc->GetTag());
		return ERR_FAIL;
		}

	if (error = pGenerator->LoadFromXML(Ctx, pDesc))
		{
		if (pGenerator)
			delete pGenerator;
		return error;
		}

	*retpGenerator = pGenerator;

	return NOERROR;
	}

ALERROR IShipGenerator::CreateFromXMLAsGroup (SDesignLoadCtx &Ctx, CXMLElement *pDesc, IShipGenerator **retpGenerator)

//	CreateFromXMLAsGroup
//
//	Creates a new generator

	{
	ALERROR error;

	IShipGenerator *pGenerator = new CGroupOfShipGenerators;
	if (error = pGenerator->LoadFromXML(Ctx, pDesc))
		{
		if (pGenerator)
			delete pGenerator;
		return error;
		}

	*retpGenerator = pGenerator;

	return NOERROR;
	}

//	CShipTable ----------------------------------------------------------------

CShipTable::CShipTable (void) : 
		m_pGenerator(NULL)

//	CShipTable constructor

	{
	}

CShipTable::~CShipTable (void)

//	CShipTable destructor

	{
	if (m_pGenerator)
		delete m_pGenerator;
	}

ALERROR CShipTable::OnBindDesign (SDesignLoadCtx &Ctx)

//	OnBindDesign
//
//	Bind design

	{
	ALERROR error;

	if (m_pGenerator)
		{
		if (error = m_pGenerator->OnDesignLoadComplete(Ctx))
			{
			Ctx.sError = strPatternSubst(CONSTLIT("ShipTable (%x): %s"), GetUNID(), Ctx.sError);
			return error;
			}
		}

	return NOERROR;
	}

ALERROR CShipTable::OnCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	OnCreateFromXML
//
//	Create from XML

	{
	ALERROR error;

	CXMLElement *pElement = pDesc->GetContentElement(0);
	if (pElement)
		{
		if (error = IShipGenerator::CreateFromXML(Ctx, pElement, &m_pGenerator))
			{
			Ctx.sError = strPatternSubst(CONSTLIT("ShipTable (%x): %s"), GetUNID(), Ctx.sError);
			return error;
			}
		}

	return NOERROR;
	}

//	CLevelTableOfShipGenerators ---------------------------------------------------------------

CLevelTableOfShipGenerators::~CLevelTableOfShipGenerators (void)

//	CLevelTableOfShipGenerators destructor

	{
	int i;

	for (i = 0; i < m_Table.GetCount(); i++)
		if (m_Table[i].pEntry)
			delete m_Table[i].pEntry;
	}

void CLevelTableOfShipGenerators::AddTypesUsed (TSortMap<DWORD, bool> *retTypesUsed)

//	AddTypesUsed
//
//	Adds types used by the table

	{
	int i;

	for (i = 0; i < m_Table.GetCount(); i++)
		if (m_Table[i].pEntry)
			m_Table[i].pEntry->AddTypesUsed(retTypesUsed);
	}

void CLevelTableOfShipGenerators::CreateShips (SShipCreateCtx &Ctx)

//	CreateShips
//
//	Create the ships

	{
	DEBUG_TRY

	int i, j;

	//	Compute probabilities

	int iLevel = Ctx.pSystem->GetLevel();
	if (iLevel != m_iComputedLevel)
		{
		m_iTotalChance = 0;
		for (i = 0; i < m_Table.GetCount(); i++)
			{
			m_Table[i].iChance = GetFrequencyByLevel(m_Table[i].sLevelFrequency, iLevel);
			m_iTotalChance += m_Table[i].iChance;
			}

		m_iComputedLevel = iLevel;
		}

	//	Generate

	if (m_iTotalChance)
		{
		int iCount = m_Count.Roll();

		for (i = 0; i < iCount; i++)
			{
			int iRoll = mathRandom(1, m_iTotalChance);

			for (j = 0; j < m_Table.GetCount(); j++)
				{
				iRoll -= m_Table[j].iChance;

				if (iRoll <= 0)
					{
					m_Table[j].pEntry->CreateShips(Ctx);
					break;
					}
				}
			}
		}

	DEBUG_CATCH
	}

Metric CLevelTableOfShipGenerators::GetAverageLevelStrength (int iLevel)

//	GetAverageLevelStrength
//
//	Returns average strength of ships created by generator (using iLevel as the
//	standard).

	{
	int i;

	Metric rCount = m_Count.GetAveValueFloat();

	//	Compute the table for this level.

	Metric rTotal = 0.0;
	int iTotalChance = 0;
	for (i = 0; i < m_Table.GetCount(); i++)
		{
		int iChance = GetFrequencyByLevel(m_Table[i].sLevelFrequency, iLevel);
		iTotalChance += iChance;
		}

	for (i = 0; i < m_Table.GetCount(); i++)
		{
		int iChance = GetFrequencyByLevel(m_Table[i].sLevelFrequency, iLevel);
		if (iChance > 0)
			rTotal += (rCount * m_Table[i].pEntry->GetAverageLevelStrength(iLevel) * (Metric)iChance / (Metric)iTotalChance);
		}

	//	Done

	return rTotal;
	}

ALERROR CLevelTableOfShipGenerators::LoadFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	LoadFromXML
//
//	Load descriptor

	{
	int i;
	ALERROR error;

	m_Count.LoadFromXML(pDesc->GetAttribute(COUNT_ATTRIB));
	if (m_Count.IsEmpty())
		m_Count.SetConstant(1);

	for (i = 0; i < pDesc->GetContentElementCount(); i++)
		{
		CXMLElement *pEntry = pDesc->GetContentElement(i);
		SEntry *pNewEntry = m_Table.Insert();

		pNewEntry->sLevelFrequency = pEntry->GetAttribute(LEVEL_FREQUENCY_ATTRIB);

		if (error = IShipGenerator::CreateFromXML(Ctx, pEntry, &pNewEntry->pEntry))
			return error;
		}

	m_iComputedLevel = -1;

	return NOERROR;
	}

ALERROR CLevelTableOfShipGenerators::OnDesignLoadComplete (SDesignLoadCtx &Ctx)

//	OnDesignLoadComplete
//
//	Design load done

	{
	ALERROR error;
	int i;

	m_iComputedLevel = -1;

	for (i = 0; i < m_Table.GetCount(); i++)
		{
		if (error = m_Table[i].pEntry->OnDesignLoadComplete(Ctx))
			return error;
		}

	return NOERROR;
	}

ALERROR CLevelTableOfShipGenerators::ValidateForRandomEncounter (void)

//	ValidateForRandomEncounter
//
//	Make sure the generator is valid for a ship table

	{
	ALERROR error;
	int i;

	for (i = 0; i < m_Table.GetCount(); i++)
		{
		if (error = m_Table[i].pEntry->ValidateForRandomEncounter())
			return error;
		}

	return NOERROR;
	}

//	CLookupShipTable ----------------------------------------------------------

void CLookupShipTable::AddTypesUsed (TSortMap<DWORD, bool> *retTypesUsed)

//	AddTypesUsed
//
//	Adds types used by the table

	{
	if (m_pTable)
		m_pTable->AddTypesUsed(retTypesUsed);
	}

void CLookupShipTable::CreateShips (SShipCreateCtx &Ctx)

//	CreateShip
//
//	Create ships

	{
	DEBUG_TRY

	int i;

	if (m_pTable)
		{
		int iCount = m_Count.Roll();

		for (i = 0; i < iCount; i++)
			m_pTable->CreateShips(Ctx);
		}

	DEBUG_CATCH
	}

ALERROR CLookupShipTable::LoadFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	LoadFromXML
//
//	Load from XML

	{
	ALERROR error;

	m_Count.LoadFromXML(pDesc->GetAttribute(COUNT_ATTRIB));
	if (m_Count.IsEmpty())
		m_Count.SetConstant(1);

	if (error = m_pTable.LoadUNID(Ctx, pDesc->GetAttribute(TABLE_ATTRIB)))
		return error;

	if (m_pTable.GetUNID() == 0)
		{
		Ctx.sError = strPatternSubst(CONSTLIT("<Lookup> element missing table attribute."));
		return ERR_FAIL;
		}

	return NOERROR;
	}

ALERROR CLookupShipTable::OnDesignLoadComplete (SDesignLoadCtx &Ctx)

//	OnDesignLoadComplete
//
//	Resolve references

	{
	ALERROR error;

	if (error = m_pTable.Bind(Ctx))
		return error;

	return NOERROR;
	}

//	CSingleShip ---------------------------------------------------------------

CSingleShip::CSingleShip (void) :
		m_pItems(NULL),
		m_pEscorts(NULL),
		m_pOnCreate(NULL)

//	CSingleShip constructor

	{
	}

CSingleShip::~CSingleShip (void)

//	CSingleShip destructor

	{
	if (m_pItems)
		delete m_pItems;

	if (m_pEscorts)
		delete m_pEscorts;

	if (m_pOnCreate)
		m_pOnCreate->Discard(&g_pUniverse->GetCC());
	}

void CSingleShip::AddTypesUsed (TSortMap<DWORD, bool> *retTypesUsed)

//	AddTypesUsed
//
//	Adds types used by the table

	{
	retTypesUsed->SetAt(m_pShipClass->GetUNID(), true);

	if (m_pItems)
		m_pItems->AddTypesUsed(retTypesUsed);

	if (m_pEscorts)
		m_pEscorts->AddTypesUsed(retTypesUsed);
	}

void CSingleShip::CreateShip (SShipCreateCtx &Ctx,
							  CSovereign *pSovereign,
							  const CVector &vPos,
							  CSpaceObject *pExitGate,
							  CShip **retpShip)

//	CreateShip
//
//	Creates a single ship

	{
	int i;

	ASSERT(m_pShipClass);
	ASSERT(pSovereign);

	DWORD dwClass = m_pShipClass->GetUNID();

	//	If we've got a maximum, then see if we've already got too many ships of this
	//	ship class.

	if (m_iMaxCountInSystem > 0)
		{
		int iMaxShips = m_iMaxCountInSystem;
		for (i = 0; i < Ctx.pSystem->GetObjectCount(); i++)
			{
			CSpaceObject *pObj = Ctx.pSystem->GetObject(i);
			if (pObj && pObj->GetClassUNID() == dwClass)
				{
				if (--iMaxShips == 0)
					{
					if (retpShip)
						*retpShip = NULL;
					return;
					}
				}
			}
		}

	//	Prepare the context block

	SShipGeneratorCtx GeneratorCtx;

	if (!m_sShipName.IsBlank())
		{
		GeneratorCtx.sName = ::GenerateRandomName(m_sShipName, NULL_STR);
		GeneratorCtx.dwNameFlags = m_dwShipNameFlags;
		}

	GeneratorCtx.pItems = m_pItems;
	GeneratorCtx.InitialData = m_InitialData;
	GeneratorCtx.pOnCreate = m_pOnCreate;
	GeneratorCtx.dwCreateFlags = Ctx.dwFlags;

	GeneratorCtx.iOrder = m_iOrder;
	GeneratorCtx.OrderData = m_OrderData;

	GeneratorCtx.pBase = Ctx.pBase;
	GeneratorCtx.pTarget = Ctx.pTarget;

	//	For orders that need a base, if we don't have a base, then use the exit
	//	gate instead. This is to handle case where we come here from sysCreateShip
	//	and we don't get a chance to set Ctx.pBase.

	if (Ctx.pBase == NULL && Ctx.pGate != NULL)
		{
		switch (m_iOrder)
			{
			case IShipController::orderGuard:
			case IShipController::orderMine:
			case IShipController::orderGateOnThreat:
			case IShipController::orderPatrol:
			case IShipController::orderEscort:
			case IShipController::orderFollow:
				GeneratorCtx.pBase = Ctx.pGate;
				break;
			}
		}

	//	Get the controller

	IShipController *pController = ::CreateShipController(m_sController);
	if (pController == NULL)
		{
		kernelDebugLogMessage("Cannot create ship %x; invalid controller: %s", dwClass, m_sController);
		if (retpShip)
			*retpShip = NULL;
		return;
		}

	//	Create the ship

	CShip *pShip;
	if (Ctx.pSystem->CreateShip(dwClass,
			pController,
			(m_pOverride ? m_pOverride : Ctx.pOverride),
			pSovereign,
			vPos,
			NullVector,
			mathRandom(0, 359),
			pExitGate,
			&GeneratorCtx,
			&pShip) != NOERROR)
		{
		kernelDebugLogMessage("Error creating ship: %x", dwClass);
		if (retpShip)
			*retpShip = NULL;
		return;
		}

	//	If this ship has escorts then create them as well

	if (m_pEscorts)
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
		ECtx.pSystem = Ctx.pSystem;
		ECtx.vPos = vPos;
		ECtx.pBase = pShip;
		ECtx.pTarget = Ctx.pTarget;
		ECtx.pGate = pEscortGate;

		m_pEscorts->CreateShips(ECtx);
		}

	//	Done

	if (retpShip)
		*retpShip = pShip;
	}

void CSingleShip::CreateShips (SShipCreateCtx &Ctx)

//	CreateShips
//
//	Create the ships

	{
	DEBUG_TRY

	int i;

	ASSERT(Ctx.pSystem);

	//	Figure out the sovereign

	CSovereign *pSovereign;
	if (m_pSovereign)
		pSovereign = m_pSovereign;
	else if (Ctx.pBase)
		pSovereign = Ctx.pBase->GetSovereign();
	else if (Ctx.pBaseSovereign)
		pSovereign = Ctx.pBaseSovereign;
	else
		{
		ASSERT(false);
		kernelDebugLogMessage("Unable to create ships: no sovereign specified");
		return;
		}

	//	Figure out override

	CDesignType *pOverride = Ctx.pOverride;
	if (m_pOverride)
		pOverride = m_pOverride;

	//	Figure out the creation position of the ship

	CSpaceObject *pGate;
	CVector vCenter;
	if (pOverride && pOverride->FireGetCreatePos(Ctx.pBase, Ctx.pTarget, &pGate, &vCenter))
		;
	else
		{
		if (pGate = Ctx.pGate)
			vCenter = pGate->GetPos();
		else
			vCenter = Ctx.vPos;
		}

	//	Create all the ships

	int iCount = m_Count.Roll();
	for (i = 0; i < iCount; i++)
		{
		CShip *pShip;

		//	Generate a position

		CVector vPos = vCenter + ::PolarToVector(mathRandom(0, 359), Ctx.PosSpread.Roll() * LIGHT_SECOND);

		//	Create the ship

		CreateShip(Ctx,
				pSovereign,
				vPos,
				pGate,
				&pShip);

		if (pShip)
			{
			//	Add to result list, if necessary

			if (Ctx.dwFlags & SShipCreateCtx::RETURN_RESULT)
				Ctx.Result.Add(pShip);

			//	Add encounter info, if necessary

			if (Ctx.pEncounterInfo)
				pShip->SetEncounterInfo(Ctx.pEncounterInfo);
			}
		}

	DEBUG_CATCH
	}

Metric CSingleShip::GetAverageLevelStrength (int iLevel)

//	GetAverageLevelStrength
//
//	Returns average strength of ships created by generator (using iLevel as the
//	standard).

	{
	//	Compute based on the level of the ship relative to the input level.

	Metric rTotal = m_Count.GetAveValueFloat() * ::CalcLevelDiffStrength(m_pShipClass->GetLevel() - iLevel);

	//	Add any escorts

	if (m_pEscorts)
		rTotal += m_pEscorts->GetAverageLevelStrength(iLevel);

	//	Done

	return rTotal;
	}

ALERROR CSingleShip::LoadFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	LoadFromXML
//
//	Load descriptor

	{
	ALERROR error;

	//	Load some data

	m_sController = pDesc->GetAttribute(CONTROLLER_ATTRIB);
	m_iMaxCountInSystem = pDesc->GetAttributeInteger(MAX_SHIPS_ATTRIB);
	m_Count.LoadFromXML(pDesc->GetAttribute(COUNT_ATTRIB));
	if (m_Count.IsEmpty())
		m_Count.SetConstant(1);

	//	Load name

	CXMLElement *pNames = pDesc->GetContentElementByTag(NAMES_TAG);
	if (pNames)
		{
		m_sShipName = pNames->GetContentText(0);
		m_dwShipNameFlags = LoadNameFlags(pNames);
		}
	else
		{
		m_sShipName = pDesc->GetAttribute(NAME_ATTRIB);
		m_dwShipNameFlags = 0;
		}

	//	Load references to other design elements

	if (error = m_pShipClass.LoadUNID(Ctx, pDesc->GetAttribute(CLASS_ATTRIB)))
		return error;

	if (error = m_pSovereign.LoadUNID(Ctx, pDesc->GetAttribute(SOVEREIGN_ATTRIB)))
		return error;

	if (error = m_pOverride.LoadUNID(Ctx, pDesc->GetAttribute(EVENT_HANDLER_ATTRIB)))
		return error;

	//	Load OnCreate code

	CXMLElement *pHandler = pDesc->GetContentElementByTag(ON_CREATE_TAG);
	if (pHandler)
		{
		m_pOnCreate = g_pUniverse->GetCC().Link(pHandler->GetContentText(0), 0, NULL);
		if (m_pOnCreate->IsError())
			{
			Ctx.sError = strPatternSubst("<OnCreate> in <Ship>: &s", m_pOnCreate->GetStringValue());
			return ERR_FAIL;
			}
		}

	//	Load orders

	if (!ParseOrderString(pDesc->GetAttribute(ORDERS_ATTRIB), &m_iOrder, &m_OrderData))
		{
		Ctx.sError = strPatternSubst("Invalid order: %s", pDesc->GetAttribute(ORDERS_ATTRIB));
		return ERR_FAIL;
		}

	//	If we have no orders, warn

#ifdef DEBUG
	if (g_pUniverse->InDebugMode()
			&& m_iOrder == IShipController::orderNone
			&& Ctx.pType
			&& Ctx.pType->GetType() != designShipTable
			&& m_pOverride.GetUNID() == 0
			&& m_pOnCreate == NULL
			&& m_sController.IsBlank())
		{
		::kernelDebugLogMessage("%s (%08x): Warning: Ship in table has no orders.", Ctx.pType->GetTypeName(), Ctx.pType->GetUNID());
		}
#endif

	//	For backwards compatibility, handle patrol distance

	switch (m_iOrder)
		{
		case IShipController::orderPatrol:
			if (m_OrderData.AsInteger() == 0)
				m_OrderData = IShipController::SData(pDesc->GetAttributeIntegerBounded(PATROL_DIST_ATTRIB, 1, -1, 1));
			break;
		}

	//	Load items

	DWORD dwItemTableUNID;
	CXMLElement *pItems = pDesc->GetContentElementByTag(ITEMS_TAG);
	if (pItems)
		{
		if (error = IItemGenerator::CreateFromXML(Ctx, pItems, &m_pItems))
			return error;
		}
	else if (pDesc->FindAttributeInteger(ITEM_TABLE_ATTRIB, (int *)&dwItemTableUNID))
		{
		if (dwItemTableUNID == 0)
			{
			Ctx.sError = strPatternSubst(CONSTLIT("Must specify a valid item table UNID: %s"), pDesc->GetAttribute(ITEM_TABLE_ATTRIB));
			return ERR_FAIL;
			}

		if (error = IItemGenerator::CreateLookupTable(Ctx, dwItemTableUNID, &m_pItems))
			return error;
		}
	else
		m_pItems = NULL;

	//	Load initial data

	CXMLElement *pInitialData = pDesc->GetContentElementByTag(INITIAL_DATA_TAG);
	if (pInitialData)
		m_InitialData.SetFromXML(pInitialData);

	//	Load escorts

	CXMLElement *pEscorts = pDesc->GetContentElementByTag(ESCORTS_TAG);
	if (pEscorts)
		{
		if (error = IShipGenerator::CreateFromXMLAsGroup(Ctx, pEscorts, &m_pEscorts))
			return error;
		}
	else
		m_pEscorts = NULL;

	return NOERROR;
	}

ALERROR CSingleShip::OnDesignLoadComplete (SDesignLoadCtx &Ctx)

//	OnDesignLoadComplete
//
//	Design load done

	{
	ALERROR error;

	if (error = m_pShipClass.Bind(Ctx))
		return error;

	if (error = m_pSovereign.Bind(Ctx))
		return error;

	if (error = m_pOverride.Bind(Ctx))
		return error;

	if (m_pItems)
		if (error = m_pItems->OnDesignLoadComplete(Ctx))
			return error;

	if (m_pEscorts)
		if (error = m_pEscorts->OnDesignLoadComplete(Ctx))
			return error;

	return NOERROR;
	}

ALERROR CSingleShip::ValidateForRandomEncounter (void)

//	ValidateForRandomEncounter
//
//	Make sure that this generator has enough data for a ship table (<EncounterTable>)

	{
	if (m_pSovereign == NULL)
		return ERR_FAIL;

	//	No need to recurse down to escorts because escorts take on the sovereign of
	//	their leader.

	return NOERROR;
	}

//	CTableOfShipGenerators ---------------------------------------------------------------

CTableOfShipGenerators::~CTableOfShipGenerators (void)

//	CTableOfShipGenerators destructor

	{
	int i;

	for (i = 0; i < m_Table.GetCount(); i++)
		if (m_Table[i].pEntry)
			delete m_Table[i].pEntry;
	}

void CTableOfShipGenerators::AddTypesUsed (TSortMap<DWORD, bool> *retTypesUsed)

//	AddTypesUsed
//
//	Adds types used by the table

	{
	int i;

	for (i = 0; i < m_Table.GetCount(); i++)
		if (m_Table[i].pEntry)
			m_Table[i].pEntry->AddTypesUsed(retTypesUsed);
	}

void CTableOfShipGenerators::CreateShips (SShipCreateCtx &Ctx)

//	CreateShips
//
//	Create the ships

	{
	DEBUG_TRY

	int i, j;

	int iCount = m_Count.Roll();
	for (j = 0; j < iCount; j++)
		{
		int iRoll = mathRandom(1, m_iTotalChance);

		for (i = 0; i < m_Table.GetCount(); i++)
			{
			iRoll -= m_Table[i].iChance;

			if (iRoll <= 0)
				{
				m_Table[i].pEntry->CreateShips(Ctx);
				break;
				}
			}
		}

	DEBUG_CATCH
	}

Metric CTableOfShipGenerators::GetAverageLevelStrength (int iLevel)

//	GetAverageLevelStrength
//
//	Returns average strength of ships created by generator (using iLevel as the
//	standard).

	{
	int i;

	if (m_iTotalChance == 0)
		return 0.0;

	Metric rCount = m_Count.GetAveValueFloat();

	//	Average value is proportional to chances.

	Metric rTotal = 0.0;
	for (i = 0; i < m_Table.GetCount(); i++)
		rTotal += (rCount * (Metric)m_Table[i].pEntry->GetAverageLevelStrength(iLevel) * (Metric)m_Table[i].iChance / (Metric)m_iTotalChance);

	return rTotal;
	}

ALERROR CTableOfShipGenerators::LoadFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	LoadFromXML
//
//	Load descriptor

	{
	int i;
	ALERROR error;

	m_Count.LoadFromXML(pDesc->GetAttribute(COUNT_ATTRIB));
	if (m_Count.IsEmpty())
		m_Count.SetConstant(1);

	m_iTotalChance = 0;
	int iCount = pDesc->GetContentElementCount();
	if (iCount > 0)
		{
		m_Table.InsertEmpty(iCount);

		for (i = 0; i < iCount; i++)
			{
			CXMLElement *pEntry = pDesc->GetContentElement(i);
			
			m_Table[i].iChance = pEntry->GetAttributeInteger(CHANCE_ATTRIB);
			m_iTotalChance += m_Table[i].iChance;

			if (error = IShipGenerator::CreateFromXML(Ctx, pEntry, &m_Table[i].pEntry))
				return error;
			}
		}

	return NOERROR;
	}

ALERROR CTableOfShipGenerators::OnDesignLoadComplete (SDesignLoadCtx &Ctx)

//	OnDesignLoadComplete
//
//	Design load done

	{
	ALERROR error;
	int i;

	for (i = 0; i < m_Table.GetCount(); i++)
		{
		if (error = m_Table[i].pEntry->OnDesignLoadComplete(Ctx))
			return error;
		}

	return NOERROR;
	}

ALERROR CTableOfShipGenerators::ValidateForRandomEncounter (void)

//	ValidateForRandomEncounter
//
//	Make sure the generator is valid for a ship table

	{
	ALERROR error;
	int i;

	for (i = 0; i < m_Table.GetCount(); i++)
		{
		if (error = m_Table[i].pEntry->ValidateForRandomEncounter())
			return error;
		}

	return NOERROR;
	}

//	CGroupOfShipGenerators ---------------------------------------------------------------

CGroupOfShipGenerators::~CGroupOfShipGenerators (void)

//	CGroupOfShipGenerators destructor

	{
	int i;

	if (m_Table)
		{
		for (i = 0; i < m_iTableCount; i++)
			if (m_Table[i].pEntry)
				delete m_Table[i].pEntry;

		delete [] m_Table;
		}
	}

void CGroupOfShipGenerators::AddTypesUsed (TSortMap<DWORD, bool> *retTypesUsed)

//	AddTypesUsed
//
//	Adds types used by the table

	{
	int i;

	for (i = 0; i < m_iTableCount; i++)
		if (m_Table[i].pEntry)
			m_Table[i].pEntry->AddTypesUsed(retTypesUsed);
	}

void CGroupOfShipGenerators::CreateShips (SShipCreateCtx &Ctx)

//	CreateShips
//
//	Create the ships

	{
	DEBUG_TRY

	int i, j;

	int iCount = m_Count.Roll();
	for (j = 0; j < iCount; j++)
		{
		for (i = 0; i < m_iTableCount; i++)
			{
			if (mathRandom(1, 100) <= m_Table[i].iChance)
				m_Table[i].pEntry->CreateShips(Ctx);
			}
		}

	DEBUG_CATCH
	}

Metric CGroupOfShipGenerators::GetAverageLevelStrength (int iLevel)

//	GetAverageLevelStrength
//
//	Returns average strength of ships created by generator (using iLevel as the
//	standard).

	{
	int i;

	Metric rCount = m_Count.GetAveValueFloat();

	//	Average value is proportional to chances.

	Metric rTotal = 0.0;
	for (i = 0; i < m_iTableCount; i++)
		{
		if (m_Table[i].iChance >= 100)
			rTotal += (rCount * m_Table[i].pEntry->GetAverageLevelStrength(iLevel) * (Metric)m_Table[i].iChance / 100.0);
		else
			rTotal += rCount * m_Table[i].pEntry->GetAverageLevelStrength(iLevel);
		}

	return rTotal;
	}

ALERROR CGroupOfShipGenerators::LoadFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	LoadFromXML
//
//	Load descriptor

	{
	ALERROR error;
	int i;

	m_Count.LoadFromXML(pDesc->GetAttribute(COUNT_ATTRIB));
	if (m_Count.IsEmpty())
		m_Count.SetConstant(1);

	m_iTableCount = pDesc->GetContentElementCount();
	if (m_iTableCount > 0)
		{
		m_Table = new SEntry [m_iTableCount];
		utlMemSet(m_Table, sizeof(SEntry) * m_iTableCount, 0);

		for (i = 0; i < m_iTableCount; i++)
			{
			CXMLElement *pEntry = pDesc->GetContentElement(i);
			
			m_Table[i].iChance = pEntry->GetAttributeInteger(CHANCE_ATTRIB);
			if (m_Table[i].iChance == 0)
				m_Table[i].iChance = 100;

			if (error = IShipGenerator::CreateFromXML(Ctx, pEntry, &m_Table[i].pEntry))
				return error;
			}
		}
	else
		m_Table = NULL;

	return NOERROR;
	}

ALERROR CGroupOfShipGenerators::OnDesignLoadComplete (SDesignLoadCtx &Ctx)

//	OnDesignLoadComplete
//
//	Design load done

	{
	ALERROR error;
	int i;

	for (i = 0; i < m_iTableCount; i++)
		{
		if (error = m_Table[i].pEntry->OnDesignLoadComplete(Ctx))
			return error;
		}

	return NOERROR;
	}

ALERROR CGroupOfShipGenerators::ValidateForRandomEncounter (void)

//	ValidateForRandomEncounter
//
//	Make sure the generator is valid for a ship table

	{
	ALERROR error;
	int i;

	for (i = 0; i < m_iTableCount; i++)
		{
		if (error = m_Table[i].pEntry->ValidateForRandomEncounter())
			return error;
		}

	return NOERROR;
	}
