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

#define AVERAGE_VALUE_TAG						CONSTLIT("AverageValue")
#define GROUP_TAG								CONSTLIT("Group")
#define ITEM_TAG								CONSTLIT("Item")
#define ITEMS_TAG								CONSTLIT("Items")
#define LEVEL_TABLE_TAG							CONSTLIT("LevelTable")
#define LOCATION_CRITERIA_TABLE_TAG				CONSTLIT("LocationCriteriaTable")
#define LOOKUP_TAG								CONSTLIT("Lookup")
#define NULL_TAG								CONSTLIT("Null")
#define RANDOM_ITEM_TAG							CONSTLIT("RandomItem")
#define TABLE_TAG								CONSTLIT("Table")

#define ATTRIBUTES_ATTRIB						CONSTLIT("attributes")
#define CATEGORIES_ATTRIB						CONSTLIT("categories")
#define CHANCE_ATTRIB							CONSTLIT("chance")
#define COUNT_ATTRIB							CONSTLIT("count")
#define CRITERIA_ATTRIB							CONSTLIT("criteria")
#define DAMAGED_ATTRIB							CONSTLIT("damaged")
#define DEBUG_ONLY_ATTRIB						CONSTLIT("debugOnly")
#define ENHANCED_ATTRIB							CONSTLIT("enhanced")
#define ENHANCEMENT_ATTRIB						CONSTLIT("enhancement")
#define ITEM_ATTRIB								CONSTLIT("item")
#define LEVEL_ATTRIB							CONSTLIT("level")
#define LEVEL_CURVE_ATTRIB						CONSTLIT("levelCurve")
#define LEVEL_FREQUENCY_ATTRIB					CONSTLIT("levelFrequency")
#define LEVEL_VALUE_ATTRIB						CONSTLIT("levelValue")
#define MODIFIERS_ATTRIB						CONSTLIT("modifiers")
#define TABLE_ATTRIB							CONSTLIT("table")
#define UNID_ATTRIB								CONSTLIT("unid")
#define VALUE_ATTRIB							CONSTLIT("value")

#define FIELD_TREASURE_VALUE					CONSTLIT("treasureValue")

#define STR_G_ITEM								CONSTLIT("gItem")

class CGroupOfGenerators : public IItemGenerator
	{
	public:
		virtual ~CGroupOfGenerators (void);
		virtual void AddItems (SItemAddCtx &Ctx);
		virtual void AddTypesUsed (TSortMap<DWORD, bool> *retTypesUsed);
		virtual CurrencyValue GetAverageValue (int iLevel);
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

		void AddItemsInt (SItemAddCtx &Ctx);
		Metric GetCountAdj (int iLevel);
		inline bool SetsAverageValue (void) const { return m_AverageValue.GetCount() > 0; }

		TArray<SEntry> m_Table;
		TArray<int> m_AverageValue;
		TArray<Metric> m_CountAdj;
	};

class CLevelTableOfItemGenerators : public IItemGenerator
	{
	public:
		virtual ~CLevelTableOfItemGenerators (void);
		virtual void AddItems (SItemAddCtx &Ctx);
		virtual void AddTypesUsed (TSortMap<DWORD, bool> *retTypesUsed);
		virtual CurrencyValue GetAverageValue (int iLevel);
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

class CLocationCriteriaTableOfItemGenerators : public IItemGenerator
	{
	public:
		virtual ~CLocationCriteriaTableOfItemGenerators (void);
		virtual void AddItems (SItemAddCtx &Ctx);
		virtual void AddTypesUsed (TSortMap<DWORD, bool> *retTypesUsed);
		virtual CurrencyValue GetAverageValue (int iLevel);
		virtual IItemGenerator *GetGenerator (int iIndex) { return m_Table[iIndex].pEntry; }
		virtual int GetGeneratorCount (void) { return m_Table.GetCount(); }
		virtual ALERROR LoadFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);
		virtual ALERROR OnDesignLoadComplete (SDesignLoadCtx &Ctx);

	private:
		struct SEntry
			{
			IItemGenerator *pEntry;
			CAttributeCriteria Criteria;
			int iChance;
			DiceRange Count;
			};

		TArray<SEntry> m_Table;
		int m_iTotalChance;
		CSystem *m_pComputedSystem;
		CString m_sComputedAttribs;
	};

class CLookup : public IItemGenerator
	{
	public:
		static ALERROR Create (DWORD dwUNID, IItemGenerator **retpGenerator, CString *retsError = NULL);

		virtual void AddItems (SItemAddCtx &Ctx);
		virtual void AddTypesUsed (TSortMap<DWORD, bool> *retTypesUsed);
		virtual CurrencyValue GetAverageValue (int iLevel) { return m_pTable->GetAverageValue(iLevel); }
		virtual IItemGenerator *GetGenerator (int iIndex) { return m_pTable->GetGenerator(); }
		virtual int GetGeneratorCount (void) { return ((m_pTable && m_pTable->GetGenerator()) ? 1 : 0); }
		virtual ALERROR LoadFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);
		virtual ALERROR OnDesignLoadComplete (SDesignLoadCtx &Ctx);

	private:
		CItemTableRef m_pTable;
	};

class CNullItem : public IItemGenerator
	{
	};

class CRandomItems : public IItemGenerator
	{
	public:
		CRandomItems (void) : m_Table(NULL) { }
		static ALERROR Create (const CItemCriteria &Crit, 
							   const CString &sLevelFrequency,
							   IItemGenerator **retpGenerator);

		virtual ~CRandomItems (void);
		virtual void AddItems (SItemAddCtx &Ctx);
		virtual void AddTypesUsed (TSortMap<DWORD, bool> *retTypesUsed);
		virtual CurrencyValue GetAverageValue (int iLevel);
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

		void InitTable (const CString &sLevelFrequency);

		int m_iCount;
		SEntry *m_Table;

		CItemCriteria m_Criteria;
		CString m_sLevelFrequency;
		int m_iLevel;
		int m_iLevelCurve;
		int m_iDamaged;
		CRandomEnhancementGenerator m_Enhanced;
		bool m_bDynamicLevelFrequency;		//	If TRUE, level frequency depends on system level
		int m_iDynamicLevel;
	};

class CSingleItem : public IItemGenerator
	{
	public:
		virtual void AddItems (SItemAddCtx &Ctx);
		virtual void AddTypesUsed (TSortMap<DWORD, bool> *retTypesUsed);
		virtual CurrencyValue GetAverageValue (int iLevel);
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

class CTableOfGenerators : public IItemGenerator
	{
	public:
		virtual ~CTableOfGenerators (void);
		virtual void AddItems (SItemAddCtx &Ctx);
		virtual void AddTypesUsed (TSortMap<DWORD, bool> *retTypesUsed);
		virtual CurrencyValue GetAverageValue (int iLevel);
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

//	IItemGenerator -------------------------------------------------------------

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
	else if (strEquals(pDesc->GetTag(), GROUP_TAG) 
			|| strEquals(pDesc->GetTag(), ITEMS_TAG)
			|| strEquals(pDesc->GetTag(), AVERAGE_VALUE_TAG))
		pGenerator = new CGroupOfGenerators;
	else if (strEquals(pDesc->GetTag(), LOOKUP_TAG))
		pGenerator = new CLookup;
	else if (strEquals(pDesc->GetTag(), LEVEL_TABLE_TAG))
		pGenerator = new CLevelTableOfItemGenerators;
	else if (strEquals(pDesc->GetTag(), LOCATION_CRITERIA_TABLE_TAG))
		pGenerator = new CLocationCriteriaTableOfItemGenerators;
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

ALERROR IItemGenerator::CreateLookupTable (SDesignLoadCtx &Ctx, DWORD dwUNID, IItemGenerator **retpGenerator)

//	CreateLookupTable
//
//	Creates a new lookup item table

	{
	return CLookup::Create(dwUNID, retpGenerator, &Ctx.sError);
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

bool CItemTable::FindDataField (const CString &sField, CString *retsValue)

//	FindDataField
//
//	Returns meta-data

	{
	//	Deal with the meta-data that we know about

	if (strEquals(sField, FIELD_TREASURE_VALUE))
		*retsValue = strFromInt((int)GetAverageValue(1));
	else
		return CDesignType::FindDataField(sField, retsValue);

	return true;
	}

void CItemTable::OnAddTypesUsed (TSortMap<DWORD, bool> *retTypesUsed)

//	OnAddTypesUsed
//
//	Adds types used by this table

	{
	if (m_pGenerator)
		m_pGenerator->AddTypesUsed(retTypesUsed);
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

//	CGroupOfGenerators --------------------------------------------------------

CGroupOfGenerators::~CGroupOfGenerators (void)

//	CGroupOfGenerators destructor

	{
	int i;

	for (i = 0; i < m_Table.GetCount(); i++)
		if (m_Table[i].pItem)
			delete m_Table[i].pItem;
	}

void CGroupOfGenerators::AddItems (SItemAddCtx &Ctx)

//	AddItems
//
//	Add items

	{
	int i, j;

	//	If we need to adjust counts, then do a separate algorithm

	if (SetsAverageValue())
		{
		//	Get the count adjustment.

		Metric rCountAdj = GetCountAdj(Ctx.iLevel);
		Metric rLoops = floor(rCountAdj);
		Metric rLastLoopAdj = rCountAdj - rLoops;

		//	Loop if we have extra items

		int iFullLoops = (int)rLoops;
		for (i = 0; i < iFullLoops + 1; i++)
			{
			//	For a full loop we just add the items

			if (i < iFullLoops)
				AddItemsInt(Ctx);

			//	Otherwise we need to add partial items

			else
				{
				//	Add the items to a private list.

				CItemList LocalList;
				CItemListManipulator ItemList(LocalList);
				SItemAddCtx LocalCtx(ItemList);
				LocalCtx.iLevel = Ctx.iLevel;
				AddItemsInt(LocalCtx);

				//	Now loop over the items and adjust the count appropriately.

				for (j = 0; j < LocalList.GetCount(); j++)
					{
					const CItem &Item = LocalList.GetItem(j);
					int iOriginalCount = Item.GetCount();

					//	Adjust the count

					Metric rNewCount = iOriginalCount * rLastLoopAdj;
					Metric rNewCountInt = floor(rNewCount);
					int iNewCount = (int)rNewCountInt;

					Metric rExtra = rNewCount - rNewCountInt;
					int iExtraChance = (int)(100000.0 * rExtra);
					if (mathRandom(0, 100000) < iExtraChance)
						iNewCount++;

					//	Add the item with the new count

					if (iNewCount > 0)
						{
						if (iNewCount == iOriginalCount)
							Ctx.ItemList.AddItem(Item);
						else
							{
							CItem NewItem(Item);
							NewItem.SetCount(iNewCount);
							Ctx.ItemList.AddItem(NewItem);
							}
						}
					}
				}
			}
		}
	else
		AddItemsInt(Ctx);
	}

void CGroupOfGenerators::AddItemsInt (SItemAddCtx &Ctx)

//	AddItemsInt
//
//	Adds items to the context.

	{
	int i, j;

	for (i = 0; i < m_Table.GetCount(); i++)
		{
		if (mathRandom(1, 100) <= m_Table[i].iChance)
			{
			int iCount = m_Table[i].Count.Roll();

			for (j = 0; j < iCount; j++)
				m_Table[i].pItem->AddItems(Ctx);
			}
		}
	}

void CGroupOfGenerators::AddTypesUsed (TSortMap<DWORD, bool> *retTypesUsed)

//	AddTypesUsed
//
//	Adds types used by this generator

	{
	int i;

	for (i = 0; i < m_Table.GetCount(); i++)
		m_Table[i].pItem->AddTypesUsed(retTypesUsed);
	}

CurrencyValue CGroupOfGenerators::GetAverageValue (int iLevel)

//	GetAverageValue
//
//	Returns the average value.

	{
	int i;

	if (SetsAverageValue())
		{
		if (iLevel >= 0 && iLevel < m_AverageValue.GetCount())
			return m_AverageValue[iLevel];
		else
			return 0;
		}
	else
		{
		//	Average value is proportional to chances.

		Metric rTotal = 0.0;
		for (i = 0; i < m_Table.GetCount(); i++)
			{
			if (m_Table[i].iChance < 100)
				rTotal += (m_Table[i].Count.GetAveValueFloat() * (Metric)m_Table[i].pItem->GetAverageValue(iLevel) * (Metric)m_Table[i].iChance / 100.0);
			else
				rTotal += m_Table[i].Count.GetAveValueFloat() * m_Table[i].pItem->GetAverageValue(iLevel);
			}

		return (CurrencyValue)(rTotal + 0.5);
		}
	}

Metric CGroupOfGenerators::GetCountAdj (int iLevel)

//	GetCountAdj
//
//	Returns the count adjusment for the given level.

	{
	int i;

	if (iLevel >= 0 && iLevel < m_CountAdj.GetCount())
		{
		Metric rCountAdj = m_CountAdj[iLevel];
		if (rCountAdj < 0.0)
			{
			//	Loop over all our children and compute the average value.

			Metric rTotal = 0.0;
			for (i = 0; i < m_Table.GetCount(); i++)
				rTotal += (Metric)m_Table[i].pItem->GetAverageValue(iLevel);

			//	Compute the factor that we have to multiply the total to get to
			//	the desired value.

			rCountAdj = (rTotal > 0.0 ? (Metric)m_AverageValue[iLevel] / rTotal : 0.0);

			//	Remember so we don't have to compute it again.

			m_CountAdj[iLevel] = rCountAdj;
			}

		return rCountAdj;
		}
	else
		return 0.0;
	}

ALERROR CGroupOfGenerators::LoadFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	LoadFromXML
//
//	Load from XML

	{
	int i;
	ALERROR error;

	//	Load content elements

	m_Table.InsertEmpty(pDesc->GetContentElementCount());
	for (i = 0; i < m_Table.GetCount(); i++)
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

	//	See if we force an average value

	CString sAttrib;
	if (pDesc->FindAttribute(LEVEL_VALUE_ATTRIB, &sAttrib))
		{
		TArray<int> Values;
		ParseIntegerList(sAttrib, 0, &Values);

		m_AverageValue.InsertEmpty(MAX_ITEM_LEVEL + 1);
		m_AverageValue[0] = 0;
		for (i = 0; i < Values.GetCount(); i++)
			m_AverageValue[i + 1] = Values[i];

		for (i = Values.GetCount() + 1; i <= MAX_ITEM_LEVEL; i++)
			m_AverageValue[i] = 0;
		}
	else if (pDesc->FindAttribute(VALUE_ATTRIB, &sAttrib))
		{
		int iValue = strToInt(sAttrib, 0);

		m_AverageValue.InsertEmpty(MAX_ITEM_LEVEL + 1);
		m_AverageValue[0] = 0;
		for (i = 1; i <= MAX_ITEM_LEVEL; i++)
			m_AverageValue[i] = iValue;
		}

	return NOERROR;
	}

ALERROR CGroupOfGenerators::OnDesignLoadComplete (SDesignLoadCtx &Ctx)

//	OnDesignLoadComplete
//
//	Resolve references

	{
	int i;
	ALERROR error;

	for (i = 0; i < m_Table.GetCount(); i++)
		{
		if (error = m_Table[i].pItem->OnDesignLoadComplete(Ctx))
			return error;
		}

	//	Initialize count adjustment

	m_CountAdj.InsertEmpty(m_AverageValue.GetCount());
	for (i = 0; i < m_AverageValue.GetCount(); i++)
		m_CountAdj[i] = -1.0;

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

void CSingleItem::AddTypesUsed (TSortMap<DWORD, bool> *retTypesUsed)

//	AddTypesUsed
//
//	Adds types used by this generator

	{
	retTypesUsed->SetAt(m_pItemType.GetUNID(), true);
	}

CurrencyValue CSingleItem::GetAverageValue (int iLevel)

//	GetAverageValue
//
//	Returns the average value (in credits)

	{
	return CEconomyType::ExchangeToCredits(m_pItemType->GetCurrencyType(), m_pItemType->GetValue(CItemCtx(), true));
	}

ALERROR CSingleItem::LoadFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	LoadFromXML
//
//	Load from XML

	{
	ALERROR error;

	if (error = m_pItemType.LoadUNID(Ctx, pDesc->GetAttribute(ITEM_ATTRIB)))
		return error;

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

void CLevelTableOfItemGenerators::AddTypesUsed (TSortMap<DWORD, bool> *retTypesUsed)

//	AddTypesUsed
//
//	Adds types used by this generator

	{
	int i;

	for (i = 0; i < m_Table.GetCount(); i++)
		m_Table[i].pEntry->AddTypesUsed(retTypesUsed);
	}

CurrencyValue CLevelTableOfItemGenerators::GetAverageValue (int iLevel)

//	GetAverageValue
//
//	Returns the average value.

	{
	int i;

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
			rTotal += (m_Table[i].Count.GetAveValueFloat() * (Metric)m_Table[i].pEntry->GetAverageValue(iLevel) * (Metric)iChance / (Metric)iTotalChance);
		}

	return (CurrencyValue)(rTotal + 0.5);
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

	m_iComputedLevel = -1;

	for (i = 0; i < m_Table.GetCount(); i++)
		{
		if (error = m_Table[i].pEntry->OnDesignLoadComplete(Ctx))
			return error;
		}

	return NOERROR;
	}

//	CLocationCriteriaTableOfItemGenerators -------------------------------------

CLocationCriteriaTableOfItemGenerators::~CLocationCriteriaTableOfItemGenerators (void)

//	CLevelTableOfItemGenerators destructor

	{
	int i;

	for (i = 0; i < m_Table.GetCount(); i++)
		if (m_Table[i].pEntry)
			delete m_Table[i].pEntry;
	}

void CLocationCriteriaTableOfItemGenerators::AddItems (SItemAddCtx &Ctx)

//	AddItems
//
//	Adds items

	{
	int i, j;

	//	Cache probabilities, if necessary

	CString sAttribsAtPos = (Ctx.pSystem ? Ctx.pSystem->GetAttribsAtPos(Ctx.vPos) : NULL_STR);
	if (m_pComputedSystem != Ctx.pSystem 
			|| !strEquals(m_sComputedAttribs, sAttribsAtPos))
		{
		//	Create a table of probabilities

		m_iTotalChance = 0;
		for (i = 0; i < m_Table.GetCount(); i++)
			{
			m_Table[i].iChance = m_Table[i].Criteria.CalcLocationWeight(Ctx.pSystem, NULL_STR, Ctx.vPos);
			m_iTotalChance += m_Table[i].iChance;
			}

		m_pComputedSystem = Ctx.pSystem;
		m_sComputedAttribs = sAttribsAtPos;
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

void CLocationCriteriaTableOfItemGenerators::AddTypesUsed (TSortMap<DWORD, bool> *retTypesUsed)

//	AddTypesUsed
//
//	Adds types used by this generator

	{
	int i;

	for (i = 0; i < m_Table.GetCount(); i++)
		m_Table[i].pEntry->AddTypesUsed(retTypesUsed);
	}

CurrencyValue CLocationCriteriaTableOfItemGenerators::GetAverageValue (int iLevel)

//	GetAverageValue
//
//	Returns the average value.

	{
	int i;

	//	Equal probability for each entry

	Metric rTotal = 0.0;
	int iTotalChance = 0;
	for (i = 0; i < m_Table.GetCount(); i++)
		iTotalChance += 1;

	for (i = 0; i < m_Table.GetCount(); i++)
		rTotal += (m_Table[i].Count.GetAveValueFloat() * (Metric)m_Table[i].pEntry->GetAverageValue(iLevel) / (Metric)iTotalChance);

	return (CurrencyValue)(rTotal + 0.5);
	}

ALERROR CLocationCriteriaTableOfItemGenerators::LoadFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

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

		CString sCriteria = pEntry->GetAttribute(CRITERIA_ATTRIB);
		if (error = pNewEntry->Criteria.Parse(sCriteria, 0, &Ctx.sError))
			return error;

		pNewEntry->Count.LoadFromXML(pEntry->GetAttribute(COUNT_ATTRIB));
		if (pNewEntry->Count.IsEmpty())
			pNewEntry->Count.SetConstant(1);

		if (error = IItemGenerator::CreateFromXML(Ctx, pEntry, &pNewEntry->pEntry))
			return error;
		}

	m_pComputedSystem = NULL;
	m_sComputedAttribs = NULL_STR;

	return NOERROR;
	}

ALERROR CLocationCriteriaTableOfItemGenerators::OnDesignLoadComplete (SDesignLoadCtx &Ctx)

//	OnDesignLoadComplete
//
//	Bind design

	{
	ALERROR error;
	int i;

	m_pComputedSystem = NULL;
	m_sComputedAttribs = NULL_STR;

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

void CLookup::AddTypesUsed (TSortMap<DWORD, bool> *retTypesUsed)

//	AddTypesUsed
//
//	Adds types used by this generator

	{
	m_pTable->AddTypesUsed(retTypesUsed);
	}

ALERROR CLookup::Create (DWORD dwUNID, IItemGenerator **retpGenerator, CString *retsError)

//	Create
//
//	Creates a table for the given item table

	{
	CLookup *pGenerator = new CLookup;
	pGenerator->m_pTable.SetUNID(dwUNID);

	*retpGenerator = pGenerator;

	return NOERROR;
	}

ALERROR CLookup::LoadFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	LoadFromXML
//
//	Load from XML

	{
	ALERROR error;

	if (error = m_pTable.LoadUNID(Ctx, pDesc->GetAttribute(TABLE_ATTRIB)))
		return error;

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

void CTableOfGenerators::AddTypesUsed (TSortMap<DWORD, bool> *retTypesUsed)

//	AddTypesUsed
//
//	Adds types used by this generator

	{
	int i;

	for (i = 0; i < m_Table.GetCount(); i++)
		m_Table[i].pItem->AddTypesUsed(retTypesUsed);
	}

CurrencyValue CTableOfGenerators::GetAverageValue (int iLevel)

//	GetAverageValue
//
//	Returns the average value.

	{
	int i;

	if (m_iTotalChance == 0)
		return 0;

	//	Average value is proportional to chances.

	Metric rTotal = 0.0;
	for (i = 0; i < m_Table.GetCount(); i++)
		rTotal += (m_Table[i].Count.GetAveValueFloat() * (Metric)m_Table[i].pItem->GetAverageValue(iLevel) * (Metric)m_Table[i].iChance / (Metric)m_iTotalChance);

	return (CurrencyValue)(rTotal + 0.5);
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

//	CRandomItems ---------------------------------------------------------------

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

	//	If this is a dynamic table, then we need to generate it now

	if (m_bDynamicLevelFrequency && m_iDynamicLevel != Ctx.iLevel)
		{
		InitTable(GenerateLevelFrequency(m_sLevelFrequency, Ctx.iLevel));
		m_iDynamicLevel = Ctx.iLevel;
		}

	//	Roll

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

void CRandomItems::AddTypesUsed (TSortMap<DWORD, bool> *retTypesUsed)

//	AddTypesUsed
//
//	Adds types used by this generator

	{
	//	Since random items tables don't explicitly refer to a type
	//	(they do it by criteria) we don't need to add them.
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
	pGenerator->m_bDynamicLevelFrequency = false;
	pGenerator->m_iDynamicLevel = 0;
	pGenerator->m_iDamaged = 0;
	pGenerator->m_iLevel = 0;
	pGenerator->m_iLevelCurve = 0;

	pGenerator->m_iCount = 0;
	pGenerator->m_Table = NULL;

	pGenerator->InitTable(sLevelFrequency);
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

CurrencyValue CRandomItems::GetAverageValue (int iLevel)

//	GetAverageValue
//
//	Returns the average value.

	{
	int i;

	//	If this is a dynamic table we need to compute all levels

	if (m_bDynamicLevelFrequency)
		{
		Metric rTotal = 0.0;
		InitTable(GenerateLevelFrequency(m_sLevelFrequency, iLevel));
		m_iDynamicLevel = iLevel;

		for (i = 0; i < m_iCount; i++)
			{
			CItemType *pType = m_Table[i].pType;
			CurrencyValue ItemValue = CEconomyType::ExchangeToCredits(pType->GetCurrencyType(), pType->GetValue(CItemCtx(), true));
			rTotal += (pType->GetNumberAppearing().GetAveValueFloat() * (Metric)ItemValue * (Metric)m_Table[i].iProbability / 1000.0);
			}

		return (CurrencyValue)(rTotal + 0.5);
		}

	//	Otherwise the table is already initialized.

	else
		{
		//	Average value is proportional to chances.

		Metric rTotal = 0.0;
		for (i = 0; i < m_iCount; i++)
			{
			CItemType *pType = m_Table[i].pType;
			CurrencyValue ItemValue = CEconomyType::ExchangeToCredits(pType->GetCurrencyType(), pType->GetValue(CItemCtx(), true));
			rTotal += (pType->GetNumberAppearing().GetAveValueFloat() * (Metric)ItemValue * (Metric)m_Table[i].iProbability / 1000.0);
			}

		return (CurrencyValue)(rTotal + 0.5);
		}
	}

void CRandomItems::InitTable (const CString &sLevelFrequency)

//	InitTable
//
//	Initializes the m_Table array.
//
//	We assume that m_Criteria, m_iLevel, and m_iLevelCurve are properly initialized.

	{
	int i;

	//	Free original, if necessary

	if (m_Table)
		delete [] m_Table;

	//	Start by allocating an array large enough to hold
	//	all item types in the universe

	ItemEntryStruct *pTable = new ItemEntryStruct [g_pUniverse->GetItemTypeCount()];
	int iTableSize = 0;

	//	Figure out if we should use level curves or level frequency

	bool bUseLevelFrequency = !sLevelFrequency.IsBlank();

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
			iScore = 1000 * GetFrequencyByLevel(sLevelFrequency, pType->GetLevel()) / ftCommon;
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
	m_bDynamicLevelFrequency = (strFind(m_sLevelFrequency, CONSTLIT(":")) != -1);
	m_iDynamicLevel = 0;

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
	if (!m_bDynamicLevelFrequency)
		InitTable(m_sLevelFrequency);

	//	Reset

	m_iDynamicLevel = 0;

	return NOERROR;
	}

CRandomEnhancementGenerator::~CRandomEnhancementGenerator (void)

//	CRandomEnhancementGenerator destructor

	{
	if (m_pCode)
		m_pCode->Discard(&(g_pUniverse->GetCC()));
	}

CRandomEnhancementGenerator &CRandomEnhancementGenerator::operator= (const CRandomEnhancementGenerator &Src)

//	CRandomEnhancementGenerator operator =

	{
	m_iChance = Src.m_iChance;
	m_dwMods = Src.m_dwMods;
	m_pCode = (Src.m_pCode ? Src.m_pCode->Reference() : NULL);

	return *this;
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

		Ctx.SaveAndDefineItemVar(Item);

		//	Execute the code

		ICCItem *pResult = Ctx.Run(m_pCode);	//	LATER:Event

		//	If we have an error, report it

		DWORD dwMods;
		if (pResult->IsError())
			{
			CString sError = strPatternSubst(CONSTLIT("Generate Enhancement: %s"), pResult->GetStringValue());
			kernelDebugLogMessage(sError);
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
