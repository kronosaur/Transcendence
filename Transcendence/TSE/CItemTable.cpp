//	CItemTable.cpp
//
//	CItemTable object
//
//	NOTES:
//
//	The count attribute is held in the collection objects. For example, in a table of items, the table
//	object keeps an item count for each item entry (instead of the item keeping its own count).
//	At some point, it might be better to switch this around.

#include "PreComp.h"

#define GROUP_TAG					CONSTLIT("Group")
#define TABLE_TAG					CONSTLIT("Table")
#define RANDOM_ITEM_TAG				CONSTLIT("RandomItem")
#define ITEM_TAG					CONSTLIT("Item")
#define ITEMS_TAG					CONSTLIT("Items")
#define LOOKUP_TAG					CONSTLIT("Lookup")
#define NULL_TAG					CONSTLIT("Null")
#define LEVEL_TABLE_TAG				CONSTLIT("LevelTable")

#define ATTRIBUTES_ATTRIB			CONSTLIT("attributes")
#define DEBUG_ONLY_ATTRIB			CONSTLIT("debugOnly")
#define UNID_ATTRIB					CONSTLIT("unid")
#define ITEM_ATTRIB					CONSTLIT("item")
#define COUNT_ATTRIB				CONSTLIT("count")
#define TABLE_ATTRIB				CONSTLIT("table")
#define CHANCE_ATTRIB				CONSTLIT("chance")
#define CRITERIA_ATTRIB				CONSTLIT("criteria")
#define LEVEL_ATTRIB				CONSTLIT("level")
#define LEVEL_CURVE_ATTRIB			CONSTLIT("levelCurve")
#define DAMAGED_ATTRIB				CONSTLIT("damaged")
#define CATEGORIES_ATTRIB			CONSTLIT("categories")
#define MODIFIERS_ATTRIB			CONSTLIT("modifiers")
#define LEVEL_FREQUENCY_ATTRIB		CONSTLIT("levelFrequency")
#define ENHANCED_ATTRIB				CONSTLIT("enhanced")
#define ENHANCEMENT_ATTRIB			CONSTLIT("enhancement")

#define STR_G_ITEM					CONSTLIT("gItem")

class CNullItem : public IItemGenerator
	{
	};

class CSingleItem : public IItemGenerator
	{
	public:
		virtual void AddItems (SItemAddCtx &Ctx);
		virtual CItemType *GetItemType (int iIndex) { return m_pItemType; }
		virtual int GetItemTypeCount (void) { return 1; }
		virtual ALERROR LoadFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);
		virtual ALERROR OnDesignLoadComplete (SDesignLoadCtx &Ctx);

	private:
		CItemTypeRef m_pItemType;
		int m_iDamaged;
		CRandomEnhancementGenerator m_Enhanced;
		bool m_bDebugOnly;
	};

class CLookup : public IItemGenerator
	{
	public:
		virtual void AddItems (SItemAddCtx &Ctx);
		virtual IItemGenerator *GetGenerator (int iIndex) { return m_pTable->GetGenerator(); }
		virtual int GetGeneratorCount (void) { return ((m_pTable && m_pTable->GetGenerator()) ? 1 : 0); }
		virtual ALERROR LoadFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);
		virtual ALERROR OnDesignLoadComplete (SDesignLoadCtx &Ctx);

	private:
		CItemTableRef m_pTable;
	};

class CTableOfGenerators : public IItemGenerator
	{
	public:
		virtual ~CTableOfGenerators (void);
		virtual void AddItems (SItemAddCtx &Ctx);
		virtual IItemGenerator *GetGenerator (int iIndex) { return m_Table[iIndex].pItem; }
		virtual int GetGeneratorCount (void) { return m_Table.GetCount(); }
		virtual ALERROR LoadFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);
		virtual ALERROR OnDesignLoadComplete (SDesignLoadCtx &Ctx);

	private:
		struct SEntry
			{
			IItemGenerator *pItem;
			int iChance;
			DiceRange Count;
			};

		TArray<SEntry> m_Table;
		int m_iTotalChance;
	};

class CLevelTableOfItemGenerators : public IItemGenerator
	{
	public:
		virtual ~CLevelTableOfItemGenerators (void);
		virtual void AddItems (SItemAddCtx &Ctx);
		virtual IItemGenerator *GetGenerator (int iIndex) { return m_Table[iIndex].pEntry; }
		virtual int GetGeneratorCount (void) { return m_Table.GetCount(); }
		virtual ALERROR LoadFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);
		virtual ALERROR OnDesignLoadComplete (SDesignLoadCtx &Ctx);

	private:
		struct SEntry
			{
			IItemGenerator *pEntry;
			CString sLevelFrequency;
			int iChance;
			DiceRange Count;
			};

		TArray<SEntry> m_Table;
		int m_iTotalChance;
		int m_iComputedLevel;
	};

class CGroupOfGenerators : public IItemGenerator
	{
	public:
		virtual ~CGroupOfGenerators (void);
		virtual void AddItems (SItemAddCtx &Ctx);
		virtual IItemGenerator *GetGenerator (int iIndex) { return m_Table[iIndex].pItem; }
		virtual int GetGeneratorCount (void) { return m_iCount; }
		virtual ALERROR LoadFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);
		virtual ALERROR OnDesignLoadComplete (SDesignLoadCtx &Ctx);

	private:
		struct SEntry
			{
			IItemGenerator *pItem;
			int iChance;
			DiceRange Count;
			};

		int m_iCount;
		SEntry *m_Table;
	};

class CRandomItems : public IItemGenerator
	{
	public:
		static ALERROR Create (const CItemCriteria &Crit, 
							   const CString &sLevelFrequency,
							   IItemGenerator **retpGenerator);

		virtual ~CRandomItems (void);
		virtual void AddItems (SItemAddCtx &Ctx);
		virtual CItemType *GetItemType (int iIndex) { return m_Table[iIndex].pType; }
		virtual int GetItemTypeCount (void) { return m_iCount; }
		virtual ALERROR LoadFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);
		virtual ALERROR OnDesignLoadComplete (SDesignLoadCtx &Ctx);

	private:
		struct SEntry
			{
			CItemType *pType;
			int iProbability;
			};

		void InitTable (void);

		int m_iCount;
		SEntry *m_Table;

		CItemCriteria m_Criteria;
		CString m_sLevelFrequency;
		int m_iLevel;
		int m_iLevelCurve;
		int m_iDamaged;
		CRandomEnhancementGenerator m_Enhanced;
	};

ALERROR IItemGenerator::CreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, IItemGenerator **retpGenerator)

//	CreateFromXML
//
//	Creates a new generator

	{
	ALERROR error;
	IItemGenerator *pGenerator = NULL;

	if (strEquals(pDesc->GetTag(), ITEM_TAG))
		pGenerator = new CSingleItem;
	else if (strEquals(pDesc->GetTag(), TABLE_TAG))
		pGenerator = new CTableOfGenerators;
	else if (strEquals(pDesc->GetTag(), RANDOM_ITEM_TAG))
		pGenerator = new CRandomItems;
	else if (strEquals(pDesc->GetTag(), GROUP_TAG) || strEquals(pDesc->GetTag(), ITEMS_TAG))
		pGenerator = new CGroupOfGenerators;
	else if (strEquals(pDesc->GetTag(), LOOKUP_TAG))
		pGenerator = new CLookup;
	else if (strEquals(pDesc->GetTag(), LEVEL_TABLE_TAG))
		pGenerator = new CLevelTableOfItemGenerators;
	else if (strEquals(pDesc->GetTag(), NULL_TAG))
		pGenerator = new CNullItem;
	else
		{
		Ctx.sError = strPatternSubst(CONSTLIT("Unknown item generator: %s"), pDesc->GetTag());
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

ALERROR IItemGenerator::CreateRandomItemTable (const CItemCriteria &Crit, 
											   const CString &sLevelFrequency,
											   IItemGenerator **retpGenerator)

//	CreateRandomItemTable
//
//	Creates a new random item table

	{
	return CRandomItems::Create(Crit, sLevelFrequency, retpGenerator);
	}

CItemTable::CItemTable (void) : 
		m_pGenerator(NULL)

//	CItemTable constructor

	{
	}

CItemTable::~CItemTable (void)

//	CItemTable destructor

	{
	if (m_pGenerator)
		delete m_pGenerator;
	}

ALERROR CItemTable::OnCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	OnCreateFromXML
//
//	Create from XML descriptor

	{
	ALERROR error;

	CXMLElement *pElement = pDesc->GetContentElement(0);
	if (pElement)
		{
		if (error = IItemGenerator::CreateFromXML(Ctx, pElement, &m_pGenerator))
			{
			Ctx.sError = strPatternSubst(CONSTLIT("ItemTable (%x): %s"), GetUNID(), Ctx.sError);
			return error;
			}
		}

	return NOERROR;
	}

ALERROR CItemTable::OnBindDesign (SDesignLoadCtx &Ctx)

//	OnBindDesign
//
//	Load design references

	{
	ALERROR error;

	if (m_pGenerator)
		{
		if (error = m_pGenerator->OnDesignLoadComplete(Ctx))
			{
			Ctx.sError = strPatternSubst(CONSTLIT("ItemTable (%x): %s"), GetUNID(), Ctx.sError);
			return error;
			}
		}

	return NOERROR;
	}

//	CSingleItem ---------------------------------------------------------------

void CSingleItem::AddItems (SItemAddCtx &Ctx)

//	AddItems
//
//	Add items

	{
	//	Ignore if not debugging

	if (m_bDebugOnly && !g_pUniverse->InDebugMode())
		return;

	//	Create item

	CItem NewItem(m_pItemType, 1);
	if (mathRandom(1, 100) <= m_iDamaged)
		NewItem.SetDamaged();
	else
		m_Enhanced.EnhanceItem(NewItem);

	Ctx.ItemList.AddItem(NewItem);
	}

ALERROR CSingleItem::LoadFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	LoadFromXML
//
//	Load from XML

	{
	ALERROR error;
	m_pItemType.LoadUNID(Ctx, pDesc->GetAttribute(ITEM_ATTRIB));
	m_iDamaged = pDesc->GetAttributeInteger(DAMAGED_ATTRIB);
	m_bDebugOnly = pDesc->GetAttributeBool(DEBUG_ONLY_ATTRIB);

	if (m_pItemType.GetUNID() == 0)
		{
		CString sUNID = pDesc->GetAttribute(ITEM_ATTRIB);
		if (sUNID.IsBlank())
			Ctx.sError = strPatternSubst(CONSTLIT("<Item> element missing item attribute."));
		else
			Ctx.sError = strPatternSubst(CONSTLIT("Invalid item UNID: %s"), sUNID);
		return ERR_FAIL;
		}

	if (error = m_Enhanced.InitFromXML(Ctx, pDesc))
		return error;

	return NOERROR;
	}

ALERROR CSingleItem::OnDesignLoadComplete (SDesignLoadCtx &Ctx)

//	OnDesignLoadComplete
//
//	Resolve references

	{
	ALERROR error;

	//	Ignore if not debugging. We don't bind because sometimes we have a
	//	reference to an item type that only exists in debug mode.

	if (m_bDebugOnly && !g_pUniverse->InDebugMode())
		return NOERROR;

	//	Bind

	if (error = m_pItemType.Bind(Ctx))
		return error;

	return NOERROR;
	}

//	CLevelTableOfItemGenerators -----------------------------------------------

CLevelTableOfItemGenerators::~CLevelTableOfItemGenerators (void)

//	CLevelTableOfItemGenerators destructor

	{
	int i;

	for (i = 0; i < m_Table.GetCount(); i++)
		if (m_Table[i].pEntry)
			delete m_Table[i].pEntry;
	}

void CLevelTableOfItemGenerators::AddItems (SItemAddCtx &Ctx)

//	AddItems
//
//	Adds items

	{
	int i, j;

	//	Compute probabilities, if necessary

	if (m_iComputedLevel != Ctx.iLevel)
		{
		//	Create a table of probabilities

		m_iTotalChance = 0;
		for (i = 0; i < m_Table.GetCount(); i++)
			{
			m_Table[i].iChance = GetFrequencyByLevel(m_Table[i].sLevelFrequency, Ctx.iLevel);
			m_iTotalChance += m_Table[i].iChance;
			}

		m_iComputedLevel = Ctx.iLevel;
		}

	//	Generate

	if (m_iTotalChance)
		{
		int iRoll = mathRandom(1, m_iTotalChance);
		for (i = 0; i < m_Table.GetCount(); i++)
			{
			iRoll -= m_Table[i].iChance;

			if (iRoll <= 0)
				{
				int iCount = m_Table[i].Count.Roll();

				for (j = 0; j < iCount; j++)
					m_Table[i].pEntry->AddItems(Ctx);

				break;
				}
			}
		}
	}

ALERROR CLevelTableOfItemGenerators::LoadFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	LoadFromXML
//
//	Load table from XML

	{
	int i;
	ALERROR error;

	for (i = 0; i < pDesc->GetContentElementCount(); i++)
		{
		CXMLElement *pEntry = pDesc->GetContentElement(i);
		SEntry *pNewEntry = m_Table.Insert();

		pNewEntry->sLevelFrequency = pEntry->GetAttribute(LEVEL_FREQUENCY_ATTRIB);

		pNewEntry->Count.LoadFromXML(pEntry->GetAttribute(COUNT_ATTRIB));
		if (pNewEntry->Count.IsEmpty())
			pNewEntry->Count.SetConstant(1);

		if (error = IItemGenerator::CreateFromXML(Ctx, pEntry, &pNewEntry->pEntry))
			return error;
		}

	m_iComputedLevel = -1;

	return NOERROR;
	}

ALERROR CLevelTableOfItemGenerators::OnDesignLoadComplete (SDesignLoadCtx &Ctx)

//	OnDesignLoadComplete
//
//	Bind design

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

//	CLookup -------------------------------------------------------------------

void CLookup::AddItems (SItemAddCtx &Ctx)

//	AddItems
//
//	Add items

	{
	m_pTable->AddItems(Ctx);
	}

ALERROR CLookup::LoadFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	LoadFromXML
//
//	Load from XML

	{
	m_pTable.LoadUNID(Ctx, pDesc->GetAttribute(TABLE_ATTRIB));
	if (m_pTable.GetUNID() == 0)
		{
		Ctx.sError = strPatternSubst(CONSTLIT("<Lookup> element missing table attribute."));
		return ERR_FAIL;
		}

	return NOERROR;
	}

ALERROR CLookup::OnDesignLoadComplete (SDesignLoadCtx &Ctx)

//	OnDesignLoadComplete
//
//	Resolve references

	{
	ALERROR error;

	if (error = m_pTable.Bind(Ctx))
		return error;

	return NOERROR;
	}

//	CTableOfGenerators --------------------------------------------------------

CTableOfGenerators::~CTableOfGenerators (void)

//	CTableOfGenerators destructor

	{
	int i;

	for (i = 0; i < m_Table.GetCount(); i++)
		if (m_Table[i].pItem)
			delete m_Table[i].pItem;
	}

void CTableOfGenerators::AddItems (SItemAddCtx &Ctx)

//	AddItems
//
//	Add items

	{
	int i, j;
	int iRoll = mathRandom(1, m_iTotalChance);

	for (i = 0; i < m_Table.GetCount(); i++)
		{
		iRoll -= m_Table[i].iChance;

		if (iRoll <= 0)
			{
			int iCount = m_Table[i].Count.Roll();

			for (j = 0; j < iCount; j++)
				m_Table[i].pItem->AddItems(Ctx);
			break;
			}
		}
	}

ALERROR CTableOfGenerators::LoadFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	LoadFromXML
//
//	Load from XML

	{
	int i;
	ALERROR error;

	m_iTotalChance = 0;
	int iCount = pDesc->GetContentElementCount();
	if (iCount > 0)
		{
		m_Table.InsertEmpty(iCount);

		//	Pre-initialize to NULL in case we exit with an error

		for (i = 0; i < iCount; i++)
			m_Table[i].pItem = NULL;

		//	Load

		for (i = 0; i < iCount; i++)
			{
			CXMLElement *pEntry = pDesc->GetContentElement(i);
			
			m_Table[i].iChance = pEntry->GetAttributeInteger(CHANCE_ATTRIB);
			m_iTotalChance += m_Table[i].iChance;

			CString sCount = pEntry->GetAttribute(COUNT_ATTRIB);
			if (sCount.IsBlank())
				m_Table[i].Count = DiceRange(0, 0, 1);
			else
				m_Table[i].Count.LoadFromXML(sCount);

			if (error = IItemGenerator::CreateFromXML(Ctx, pEntry, &m_Table[i].pItem))
				return error;
			}
		}

	return NOERROR;
	}

ALERROR CTableOfGenerators::OnDesignLoadComplete (SDesignLoadCtx &Ctx)

//	OnDesignLoadComplete
//
//	Resolve references

	{
	ALERROR error;
	int i;

	for (i = 0; i < m_Table.GetCount(); i++)
		{
		if (error = m_Table[i].pItem->OnDesignLoadComplete(Ctx))
			return error;
		}

	return NOERROR;
	}

//	CGroupOfGenerators --------------------------------------------------------

CGroupOfGenerators::~CGroupOfGenerators (void)

//	CGroupOfGenerators destructor

	{
	int i;

	if (m_Table)
		{
		for (i = 0; i < m_iCount; i++)
			if (m_Table[i].pItem)
				delete m_Table[i].pItem;

		delete [] m_Table;
		}
	}

void CGroupOfGenerators::AddItems (SItemAddCtx &Ctx)

//	AddItems
//
//	Add items

	{
	int i, j;

	for (i = 0; i < m_iCount; i++)
		{
		if (mathRandom(1, 100) <= m_Table[i].iChance)
			{
			int iCount = m_Table[i].Count.Roll();

			for (j = 0; j < iCount; j++)
				m_Table[i].pItem->AddItems(Ctx);
			}
		}
	}

ALERROR CGroupOfGenerators::LoadFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	LoadFromXML
//
//	Load from XML

	{
	int i;
	ALERROR error;

	m_iCount = pDesc->GetContentElementCount();
	if (m_iCount > 0)
		{
		m_Table = new SEntry [m_iCount];
		utlMemSet(m_Table, sizeof(SEntry) * m_iCount, 0);

		for (i = 0; i < m_iCount; i++)
			{
			CXMLElement *pEntry = pDesc->GetContentElement(i);
			
			m_Table[i].iChance = pEntry->GetAttributeInteger(CHANCE_ATTRIB);
			if (m_Table[i].iChance == 0)
				m_Table[i].iChance = 100;

			CString sCount = pEntry->GetAttribute(COUNT_ATTRIB);
			if (sCount.IsBlank())
				m_Table[i].Count = DiceRange(0, 0, 1);
			else
				m_Table[i].Count.LoadFromXML(sCount);

			if (error = IItemGenerator::CreateFromXML(Ctx, pEntry, &m_Table[i].pItem))
				return error;
			}
		}
	else
		m_Table = NULL;

	return NOERROR;
	}

ALERROR CGroupOfGenerators::OnDesignLoadComplete (SDesignLoadCtx &Ctx)

//	OnDesignLoadComplete
//
//	Resolve references

	{
	int i;
	ALERROR error;

	for (i = 0; i < m_iCount; i++)
		{
		if (error = m_Table[i].pItem->OnDesignLoadComplete(Ctx))
			return error;
		}

	return NOERROR;
	}

CRandomItems::~CRandomItems (void)

//	CRandomItems destructor

	{
	if (m_Table)
		delete [] m_Table;
	}

void CRandomItems::AddItems (SItemAddCtx &Ctx)

//	AddItems
//
//	Add items

	{
	int i, j;
	int iRoll = mathRandom(1, 1000);
	bool bAllAtOnce = (m_iDamaged == 0 && m_Enhanced.GetChance() == 0);

	for (i = 0; i < m_iCount; i++)
		{
		iRoll -= m_Table[i].iProbability;

		if (iRoll <= 0)
			{
			CItemType *pType = m_Table[i].pType;
			int iCount = pType->GetNumberAppearing().Roll();

			//	If we don't have a chance of enhancement or damage, just optimize the
			//	result by adding a group of items.

			if (bAllAtOnce)
				Ctx.ItemList.AddItem(CItem(m_Table[i].pType, iCount));

			//	If this is armor, then treat them as a block

			else if (pType->GetCategory() == itemcatArmor)
				{
				CItem Item(m_Table[i].pType, iCount);

				if (mathRandom(1, 100) <= m_iDamaged)
					Item.SetDamaged();
				else
					m_Enhanced.EnhanceItem(Item);

				Ctx.ItemList.AddItem(Item);
				}

			//	Otherwise, enhance/damage each item individually

			else
				{
				for (j = 0; j < iCount; j++)
					{
					CItem Item(m_Table[i].pType, 1);

					if (mathRandom(1, 100) <= m_iDamaged)
						Item.SetDamaged();
					else
						m_Enhanced.EnhanceItem(Item);

					Ctx.ItemList.AddItem(Item);
					}
				}
			break;
			}
		}
	}

ALERROR CRandomItems::Create (const CItemCriteria &Crit, 
							  const CString &sLevelFrequency,
							  IItemGenerator **retpGenerator)

//	Create
//
//	Creates the table from criteria and level frequency

	{
	CRandomItems *pGenerator = new CRandomItems;
	pGenerator->m_Criteria = Crit;
	pGenerator->m_sLevelFrequency = sLevelFrequency;
	pGenerator->m_iDamaged = 0;
	pGenerator->m_iLevel = 0;
	pGenerator->m_iLevelCurve = 0;

	pGenerator->m_iCount = 0;
	pGenerator->m_Table = NULL;

	pGenerator->InitTable();
	if (pGenerator->m_iCount == 0)
		return ERR_FAIL;

	//	Done

	*retpGenerator = pGenerator;

	return NOERROR;
	}

struct ItemEntryStruct
	{
	CItemType *pType;
	int iChance;
	int iRemainder;
	};

void CRandomItems::InitTable (void)

//	InitTable
//
//	Initializes the m_Table array.
//
//	We assume that m_Criteria, m_sLevelFrequency, m_iLevel, and m_iLevelCurve are properly initialized.

	{
	int i;

	//	Start by allocating an array large enough to hold
	//	all item types in the universe

	ItemEntryStruct *pTable = new ItemEntryStruct [g_pUniverse->GetItemTypeCount()];
	int iTableSize = 0;

	//	Figure out if we should use level curves or level frequency

	bool bUseLevelFrequency = !m_sLevelFrequency.IsBlank();

	//	Iterate over every item type and add it to the table if
	//	it matches the given criteria

	for (i = 0; i < g_pUniverse->GetItemTypeCount(); i++)
		{
		CItemType *pType = g_pUniverse->GetItemType(i);
		CItem Item(pType, 1);

		//	Skip if the item doesn't match the categories

		if (!Item.MatchesCriteria(m_Criteria))
			continue;

		//	Skip if this item is not found randomly

		if (pType->GetFrequency() == ftNotRandom)
			continue;

		//	Adjust score based on level, either the level curve
		//	or the level frequency string.

		int iScore;
		if (bUseLevelFrequency)
			{
			iScore = 1000 * GetFrequencyByLevel(m_sLevelFrequency, pType->GetLevel()) / ftCommon;
			}
		else
			{
			//	Skip if the item is not within the level curve

			if ((pType->GetLevel() < m_iLevel - m_iLevelCurve)
					|| (pType->GetLevel() > m_iLevel + m_iLevelCurve))
				continue;

			//	If we get this far then the item perfectly matches
			//	and we need to add it to our table. First, however, we need
			//	to calculate a score.
			//
			//	The score is number that represents how common the item
			//	is in the table. Later we normalize the score to be a probability

			int iLevelDelta = pType->GetLevel() - m_iLevel;
			switch (iLevelDelta)
				{
				case 0:
					iScore = 1000;
					break;

				case 1:
				case -1:
					iScore = 500;
					break;

				case 2:
				case -2:
					iScore = 200;
					break;

				default:
					iScore = 50;
				}
			}

		//	Adjust score based on item frequency

		iScore = iScore * pType->GetFrequency() * 10 / (ftCommon * 10);

		//	If we have a score of 0 then we skip this item

		if (iScore == 0)
			continue;

		//	Add the item to the table

		pTable[iTableSize].pType = pType;
		pTable[iTableSize].iChance = iScore;
		iTableSize++;
		}

	//	We must have items

	if (iTableSize == 0)
		{
		m_iCount = 0;
		m_Table = NULL;
		return;
		}

	//	Add up the total score of all items

	int iTotalScore = 0;
	for (i = 0; i < iTableSize; i++)
		iTotalScore += pTable[i].iChance;

	//	Compute the chance

	int iTotalChance = 0;
	for (i = 0; i < iTableSize; i++)
		{
		int iScore = pTable[i].iChance;
		pTable[i].iChance = (iScore * 1000) / iTotalScore;
		pTable[i].iRemainder = (iScore * 1000) % iTotalScore;

		iTotalChance += pTable[i].iChance;
		}

	//	Distribute the remaining chance points

	while (iTotalChance < 1000)
		{
		//	Look for the entry with the biggest remainder

		int iBestRemainder = 0;
		int iBestEntry = -1;

		for (i = 0; i < iTableSize; i++)
			if (pTable[i].iRemainder > iBestRemainder)
				{
				iBestRemainder = pTable[i].iRemainder;
				iBestEntry = i;
				}

		pTable[iBestEntry].iChance++;
		pTable[iBestEntry].iRemainder = 0;
		iTotalChance++;
		}

	//	Count the number of entries that we've got

	m_iCount = 0;
	for (i = 0; i < iTableSize; i++)
		if (pTable[i].iChance > 0)
			m_iCount++;

	//	Now loop over the entire table and add it to the 
	//	random entry generator

	m_Table = new SEntry [m_iCount];
	int j = 0;
	for (i = 0; i < iTableSize; i++)
		if (pTable[i].iChance > 0)
			{
			m_Table[j].pType = pTable[i].pType;
			m_Table[j].iProbability = pTable[i].iChance;
			j++;
			}

	//	Done

	delete [] pTable;
	}

ALERROR CRandomItems::LoadFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	LoadFromXML
//
//	Load from XML

	{
	ALERROR error;

	CString sCriteria = pDesc->GetAttribute(CRITERIA_ATTRIB);
	if (sCriteria.IsBlank())
		{
		CString sAttributes = pDesc->GetAttribute(ATTRIBUTES_ATTRIB);
		if (sAttributes.IsBlank())
			sAttributes = pDesc->GetAttribute(MODIFIERS_ATTRIB);

		sCriteria = strPatternSubst(CONSTLIT("%s %s"), pDesc->GetAttribute(CATEGORIES_ATTRIB), sAttributes);
		}

	CItem::ParseCriteria(sCriteria, &m_Criteria);
	m_sLevelFrequency = pDesc->GetAttribute(LEVEL_FREQUENCY_ATTRIB);
	m_iLevel = pDesc->GetAttributeInteger(LEVEL_ATTRIB);
	m_iLevelCurve = pDesc->GetAttributeInteger(LEVEL_CURVE_ATTRIB);
	m_iDamaged = pDesc->GetAttributeInteger(DAMAGED_ATTRIB);

	if (error = m_Enhanced.InitFromXML(Ctx, pDesc))
		return error;

	m_Table = NULL;
	m_iCount = 0;

	return NOERROR;
	}

ALERROR CRandomItems::OnDesignLoadComplete (SDesignLoadCtx &Ctx)

//	OnDesignLoadComplete
//
//	Resolve references

	{
	InitTable();

	return NOERROR;
	}

CRandomEnhancementGenerator::~CRandomEnhancementGenerator (void)

//	CRandomEnhancementGenerator destructor

	{
	if (m_pCode)
		m_pCode->Discard(&(g_pUniverse->GetCC()));
	}

ALERROR CRandomEnhancementGenerator::InitFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	InitFromXML
//
//	Loads the structure from XML

	{
	m_iChance = pDesc->GetAttributeInteger(ENHANCED_ATTRIB);
	CString sEnhancement = pDesc->GetAttribute(ENHANCEMENT_ATTRIB);

	//	If we have no enhancement desc, then we come up with a random
	//	one using our own internal algorithm.

	char *pPos = sEnhancement.GetASCIIZPointer();
	if (sEnhancement.IsBlank())
		{
		m_dwMods = 0;
		m_pCode = NULL;
		}

	//	If the enhancement desc is a script, then load it now

	else if (*pPos == '=')
		{
		CCodeChain &CC = g_pUniverse->GetCC();

		m_dwMods = 0;

		m_pCode = CC.Link(pPos, 1, NULL);
		if (m_pCode->IsError())
			{
			Ctx.sError = m_pCode->GetStringValue();
			m_pCode->Discard(&CC);
			return ERR_FAIL;
			}

		if (m_iChance == 0)
			m_iChance = 100;
		}

	//	Otherwise, see if this is a valid mod number

	else
		{
		m_dwMods = (DWORD)strToInt(sEnhancement, 0, NULL);
		m_pCode = NULL;

		if (m_dwMods != 0 && m_iChance == 0)
			m_iChance = 100;
		}

	return NOERROR;
	}

void CRandomEnhancementGenerator::EnhanceItem (CItem &Item)

//	EnhanceItem
//
//	Enhances the given item

	{
	//	See if this item is enhanced

	if (mathRandom(1, 100) > m_iChance)
		return;

	//	If we have code, execute the code to figure out the mod

	if (m_pCode)
		{
		CCodeChainCtx Ctx;

		//	Save the previous value of gItem

		Ctx.SaveItemVar();
		Ctx.DefineItem(Item);

		//	Execute the code

		ICCItem *pResult = Ctx.Run(m_pCode);

		//	If we have an error, report it

		DWORD dwMods;
		if (pResult->IsError())
			{
			CString sError = strPatternSubst(CONSTLIT("Generate Enhancement: %s"), pResult->GetStringValue());
			kernelDebugLogMessage(sError.GetASCIIZPointer());
			dwMods = 0;
			}

		//	Otherwise, the result code is the mods

		else
			dwMods = (DWORD)pResult->GetIntegerValue();

		//	Done with code

		Ctx.Discard(pResult);

		//	Enhance the item

		Item.AddEnhancement(dwMods);
		}

	//	Otherwise, if we have a constant mod, apply that

	else if (m_dwMods)
		{
		Item.AddEnhancement(m_dwMods);
		}

	//	Otherwise, we need to generate a random mod appropriate to the
	//	particular item

	else
		{
		DWORD dwMods;
		CItemType *pType = Item.GetType();
		int iRoll = mathRandom(1, 100);

		switch (pType->GetCategory())
			{
			case itemcatArmor:
			case itemcatShields:
				if (iRoll < 40)
					//	HP + 10%
					dwMods = 0x0101;
				else if (iRoll < 65)
					//	HP + 20%
					dwMods = 0x0102;
				else if (iRoll < 80)
					//	HP + 30%
					dwMods = 0x0103;
				else if (iRoll < 90)
					//	Damage Adj at 50% (and 75%)
					dwMods = 0x0A05 + (mathRandom(0, 3) << 4);
				else if (iRoll < 95)
					//	Immunity to special ion attacks
					dwMods = 0x0C00;
				else
					//	Regeneration
					dwMods = 0x0200;
				break;

			case itemcatWeapon:
				if (iRoll < 50)
					//	Damage + 10%
					dwMods = 0x0101;
				else if (iRoll < 80)
					//	Damage + 20%
					dwMods = 0x0102;
				else if (iRoll < 95)
					//	Damage + 30%
					dwMods = 0x0103;
				else
					//	Delay time at 70% of original
					dwMods = 0x1003;
				break;

			default:
				dwMods = 0;
			}

		Item.AddEnhancement(dwMods);
		}
	}
