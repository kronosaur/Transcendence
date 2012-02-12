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
		virtual void CreateShips (SShipCreateCtx &Ctx);
		virtual ALERROR LoadFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);
		virtual ALERROR OnDesignLoadComplete (SDesignLoadCtx &Ctx);
		virtual ALERROR ValidateForRandomEncounter (void);

	private:
		struct SEntry
			{
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
		virtual void CreateShips (SShipCreateCtx &Ctx);
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
		virtual void CreateShips (SShipCreateCtx &Ctx);
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
		DWORD m_dwOrderData;						//	Order data
	};

class CTableOfShipGenerators : public IShipGenerator
	{
	public:
		CTableOfShipGenerators (void) : m_iTotalChance(0) { }
		virtual ~CTableOfShipGenerators (void);
		virtual void CreateShips (SShipCreateCtx &Ctx);
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
		TArray<SEntry> m_Table;
		int m_iTotalChance;
	};

class CGroupOfShipGenerators : public IShipGenerator
	{
	public:
		CGroupOfShipGenerators (void) : m_Table(NULL) { }
		virtual ~CGroupOfShipGenerators (void);
		virtual void CreateShips (SShipCreateCtx &Ctx);
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

void CLevelTableOfShipGenerators::CreateShips (SShipCreateCtx &Ctx)

//	CreateShips
//
//	Create the ships

	{
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

void CLookupShipTable::CreateShips (SShipCreateCtx &Ctx)

//	CreateShip
//
//	Create ships

	{
	int i;

	if (m_pTable)
		{
		int iCount = m_Count.Roll();

		for (i = 0; i < iCount; i++)
			m_pTable->CreateShips(Ctx);
		}
	}

ALERROR CLookupShipTable::LoadFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	LoadFromXML
//
//	Load from XML

	{
	m_Count.LoadFromXML(pDesc->GetAttribute(COUNT_ATTRIB));
	if (m_Count.IsEmpty())
		m_Count.SetConstant(1);

	m_pTable.LoadUNID(Ctx, pDesc->GetAttribute(TABLE_ATTRIB));
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
	GeneratorCtx.dwOrderData = m_dwOrderData;

	GeneratorCtx.pBase = Ctx.pBase;
	GeneratorCtx.pTarget = Ctx.pTarget;

	//	Get the controller

	IShipController *pController = ::CreateShipController(m_sController);
	if (pController == NULL)
		{
		kernelDebugLogMessage("Cannot create ship %x; invalid controller: %s", dwClass, m_sController.GetASCIIZPointer());
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

	m_pShipClass.LoadUNID(Ctx, pDesc->GetAttribute(CLASS_ATTRIB));
	m_pSovereign.LoadUNID(Ctx, pDesc->GetAttribute(SOVEREIGN_ATTRIB));
	m_pOverride.LoadUNID(Ctx, pDesc->GetAttribute(EVENT_HANDLER_ATTRIB));

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
	//	For compatibility, some orders have alternate strings

	CString sOrders = pDesc->GetAttribute(ORDERS_ATTRIB);
	if (strEquals(sOrders, CONSTLIT("trade route")))
		sOrders = CONSTLIT("tradeRoute");

	m_iOrder = GetOrderType(sOrders);
	switch (m_iOrder)
		{
		case IShipController::orderPatrol:
			m_dwOrderData = Max(1, pDesc->GetAttributeInteger(PATROL_DIST_ATTRIB));
			break;

		default:
			m_dwOrderData = 0;
		}

	//	Load items

	CXMLElement *pItems = pDesc->GetContentElementByTag(ITEMS_TAG);
	if (pItems)
		{
		if (error = IItemGenerator::CreateFromXML(Ctx, pItems, &m_pItems))
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

void CTableOfShipGenerators::CreateShips (SShipCreateCtx &Ctx)

//	CreateShips
//
//	Create the ships

	{
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

void CGroupOfShipGenerators::CreateShips (SShipCreateCtx &Ctx)

//	CreateShips
//
//	Create the ships

	{
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
