//	CRandomEntryGenerator.cpp
//
//	CRandomEntryGenerator class

#include "PreComp.h"


#define GROUP_TAG					CONSTLIT("Group")
#define NULL_TAG					CONSTLIT("Null")
#define TABLE_TAG					CONSTLIT("Table")
#define RANDOM_ITEM_TAG				CONSTLIT("RandomItem")

#define PROBABILITY_ATTRIB			CONSTLIT("probability")
#define CHANCE_ATTRIB				CONSTLIT("chance")
#define COUNT_ATTRIB				CONSTLIT("count")
#define CATEGORIES_ATTRIB			CONSTLIT("categories")
#define CRITERIA_ATTRIB				CONSTLIT("criteria")
#define LEVEL_ATTRIB				CONSTLIT("level")
#define LEVEL_CURVE_ATTRIB			CONSTLIT("levelCurve")
#define WEIGHT_ATTRIB				CONSTLIT("weight")
#define DAMAGED_ATTRIB				CONSTLIT("damaged")

typedef CRandomEntryGenerator *CRandomEntryGeneratorPtr;

class CGroup : public CRandomEntryGenerator
	{
	public:
		virtual ~CGroup (void);
		static ALERROR LoadFromXML (CXMLElement *pElement, CRandomEntryGenerator **retpGenerator);

		virtual void Generate (CRandomEntryResults &Results);

	private:
		CGroup (CXMLElement *pElement);

		int m_iItemCount;
		CRandomEntryGeneratorPtr *m_pItems;
	};

class CNull : public CRandomEntryGenerator
	{
	public:
		virtual ~CNull (void) { }
		static ALERROR LoadFromXML (CXMLElement *pElement, CRandomEntryGenerator **retpGenerator) { *retpGenerator = new CNull(pElement); return NOERROR; }

		virtual void Generate (CRandomEntryResults &Results) { }

	private:
		CNull (CXMLElement *pElement) : CRandomEntryGenerator(pElement) { }
	};

class CTable : public CRandomEntryGenerator
	{
	public:
		virtual ~CTable (void);
		static ALERROR LoadFromXML (CXMLElement *pElement, CRandomEntryGenerator **retpGenerator);

		virtual void Generate (CRandomEntryResults &Results);

	private:
		CTable (CXMLElement *pElement);

		int m_iItemCount;
		int m_iTotalPercent;
		CRandomEntryGeneratorPtr *m_pItems;
	};

class CNode : public CRandomEntryGenerator
	{
	public:
		virtual ~CNode (void);
		static ALERROR LoadFromXML (CXMLElement *pElement, CRandomEntryGenerator **retpGenerator);

		virtual void Generate (CRandomEntryResults &Results);

	private:
		CNode (CXMLElement *pElement);
	};

//	CRandomEntryGenerator -----------------------------------------------------

CRandomEntryGenerator::CRandomEntryGenerator (void) :
		m_pElement(NULL),
		m_Count(0,0,1),
		m_iPercent(0)

//	CRandomEntryGenerator constructor

	{
	}

CRandomEntryGenerator::CRandomEntryGenerator (CXMLElement *pElement) : 
		m_pElement(pElement),
		m_Count(0,0,1)

//	CRandomEntryGenerator constructor

	{
	if (pElement)
		{
		//	Load percent attrib

		m_iPercent = pElement->GetAttributeInteger(PROBABILITY_ATTRIB);
		if (m_iPercent == 0)
			m_iPercent = pElement->GetAttributeInteger(CHANCE_ATTRIB);

		//	Load count attrib (defaults to 1)

		CString sCount = pElement->GetAttribute(COUNT_ATTRIB);
		if (!sCount.IsBlank())
			m_Count.LoadFromXML(sCount);
		}
	else
		m_iPercent = 0;
	}

CRandomEntryGenerator::~CRandomEntryGenerator (void)

//	CRandomEntryGenerator destructor

	{
	}

ALERROR CRandomEntryGenerator::Generate (CXMLElement *pElement, CRandomEntryResults &Results)

//	Generate
//
//	Generate a list of elements given a generator table

	{
	ALERROR error;

	//	Load the table

	CRandomEntryGenerator *pTable;
	if (error = CRandomEntryGenerator::LoadFromXML(pElement, &pTable))
		return error;

	//	Generate the entries

	pTable->Generate(Results);
	delete pTable;

	return NOERROR;
	}

ALERROR CRandomEntryGenerator::GenerateAsGroup (CXMLElement *pElement, CRandomEntryResults &Results)

//	GenerateAsGroup
//
//	Generate a list of elements given a group generator table

	{
	ALERROR error;

	//	Load the table

	CRandomEntryGenerator *pTable;
	if (error = CRandomEntryGenerator::LoadFromXMLAsGroup(pElement, &pTable))
		return error;

	//	Generate the entries

	pTable->Generate(Results);
	delete pTable;

	return NOERROR;
	}

ALERROR CRandomEntryGenerator::GenerateAsTable (CXMLElement *pElement, CRandomEntryResults &Results)

//	GenerateAsTable
//
//	Generate a list of elements given a group generator table

	{
	ALERROR error;

	//	Load the table

	CRandomEntryGenerator *pTable;
	if (error = CTable::LoadFromXML(pElement, &pTable))
		return error;

	//	Generate the entries

	pTable->Generate(Results);
	delete pTable;

	return NOERROR;
	}

ALERROR CRandomEntryGenerator::LoadFromXML (CXMLElement *pElement, CRandomEntryGenerator **retpGenerator)

//	LoadFromXML
//
//	Load the generator from XML

	{
	CString sTag = pElement->GetTag();
	if (strEquals(sTag, GROUP_TAG))
		return CGroup::LoadFromXML(pElement, retpGenerator);
	else if (strEquals(sTag, TABLE_TAG))
		return CTable::LoadFromXML(pElement, retpGenerator);
	else if (strEquals(sTag, NULL_TAG))
		return CNull::LoadFromXML(pElement, retpGenerator);
	else if (strEquals(sTag, RANDOM_ITEM_TAG))
		{
		DebugBreak();
		ASSERT(false);
		return ERR_FAIL;
		}
	else
		return CNode::LoadFromXML(pElement, retpGenerator);
	}

ALERROR CRandomEntryGenerator::LoadFromXMLAsGroup (CXMLElement *pElement, CRandomEntryGenerator **retpGenerator)

//	LoadFromXMLAsGroup
//
//	Loads a generator from XML, assuming that the element is a group

	{
	return CGroup::LoadFromXML(pElement, retpGenerator);
	}

//	CGroup --------------------------------------------------------------------

CGroup::CGroup (CXMLElement *pElement) : 
		CRandomEntryGenerator(pElement), 
		m_pItems(NULL)

//	CGroup constructor

	{
	}

CGroup::~CGroup (void)

//	CGroup destructor

	{
	//	Delete all items in our group

	if (m_pItems)
		{
		for (int i = 0; i < m_iItemCount; i++)
			delete m_pItems[i];

		delete [] m_pItems;
		}
	}

ALERROR CGroup::LoadFromXML (CXMLElement *pElement, CRandomEntryGenerator **retpGenerator)

//	LoadFromXML
//
//	Load a group element

	{
	ALERROR error;

	//	Create the group

	CGroup *pGroup = new CGroup(pElement);
	if (pGroup == NULL)
		return ERR_MEMORY;

	//	Load each of the entries in the group

	pGroup->m_iItemCount = pElement->GetContentElementCount();
	if (pGroup->m_iItemCount > 0)
		{
		pGroup->m_pItems = new CRandomEntryGeneratorPtr[pGroup->m_iItemCount];

		for (int i = 0; i < pGroup->m_iItemCount; i++)
			{
			CXMLElement *pItem = pElement->GetContentElement(i);
			if (error = CRandomEntryGenerator::LoadFromXML(pItem, &pGroup->m_pItems[i]))
				return error;
			}
		}

	//	Done

	*retpGenerator = pGroup;

	return NOERROR;
	}

void CGroup::Generate (CRandomEntryResults &Results)

//	Generate
//
//	Generate all the items in the group

	{
	int iCount = GetCount();

	//	Generate all in order

	for (int j = 0; j < iCount; j++)
		for (int i = 0; i < m_iItemCount; i++)
			{
			if (m_pItems[i]->GetPercent() == 0
					|| mathRandom(1, 100) <= m_pItems[i]->GetPercent())
				m_pItems[i]->Generate(Results);
			}
	}

//	CTable --------------------------------------------------------------------


CTable::CTable (CXMLElement *pElement) : 
		CRandomEntryGenerator(pElement), 
		m_pItems(NULL)

//	CTable constructor

	{
	}

CTable::~CTable (void)

//	CTable destructor

	{
	//	Delete all items in our group

	if (m_pItems)
		{
		for (int i = 0; i < m_iItemCount; i++)
			delete m_pItems[i];

		delete [] m_pItems;
		}
	}

ALERROR CTable::LoadFromXML (CXMLElement *pElement, CRandomEntryGenerator **retpGenerator)

//	LoadFromXML
//
//	Load a group element

	{
	ALERROR error;
	int i;

	//	Create the table

	CTable *pTable = new CTable(pElement);
	if (pTable == NULL)
		return ERR_MEMORY;

	//	Load each of the entries in the group

	pTable->m_iItemCount = pElement->GetContentElementCount();
	if (pTable->m_iItemCount > 0)
		{
		//	Allocate table of items

		pTable->m_pItems = new CRandomEntryGeneratorPtr[pTable->m_iItemCount];

		//	Loop over all items

		for (i = 0; i < pTable->m_iItemCount; i++)
			{
			CXMLElement *pItem = pElement->GetContentElement(i);
			if (error = CRandomEntryGenerator::LoadFromXML(pItem, &pTable->m_pItems[i]))
				return error;

			//	If this item has a weight then we treat it as a percentage.
			//	[This is for legacy code, before we decided to treat 
			//	chance= and probability= as a weight.]

			int iWeight;
			if (pItem->FindAttributeInteger(WEIGHT_ATTRIB, &iWeight))
				pTable->m_pItems[i]->SetPercent(iWeight);
			}
		}

	//	Add up the total percentages

	pTable->m_iTotalPercent = 0;
	for (i = 0; i < pTable->m_iItemCount; i++)
		pTable->m_iTotalPercent += pTable->m_pItems[i]->GetPercent();

	//	Done

	*retpGenerator = pTable;

	return NOERROR;
	}

void CTable::Generate (CRandomEntryResults &Results)

//	Generate
//
//	Generate all the items in the group

	{
	int iCount = GetCount();
	if (iCount == 0 || m_iTotalPercent <= 0)
		return;

	for (int j = 0; j < iCount; j++)
		{
		int iRoll = mathRandom(1, m_iTotalPercent);

		//	Pick a random item in the table

		int i = 0;
		while (iRoll > m_pItems[i]->GetPercent())
			iRoll -= m_pItems[i++]->GetPercent();

		m_pItems[i]->Generate(Results);
		}
	}

//	CNode ---------------------------------------------------------------------

CNode::CNode (CXMLElement *pElement) : CRandomEntryGenerator(pElement)

//	CNode constructor

	{
	}

CNode::~CNode (void)

//	CNode destructor

	{
	}

ALERROR CNode::LoadFromXML (CXMLElement *pElement, CRandomEntryGenerator **retpGenerator)

//	LoadFromXML
//
//	Load terminal node from XML

	{
	//	Create the node element

	CNode *pNode = new CNode(pElement);
	if (pNode == NULL)
		return ERR_MEMORY;

	//	Done

	*retpGenerator = pNode;

	return NOERROR;
	}

void CNode::Generate (CRandomEntryResults &Results)

//	Generate
//
//	Generate the node

	{
	Results.AddResult(GetElement(), GetCount());
	}

//	CRandomEntryResults -------------------------------------------------------

struct ResultEntry
	{
	int iCount;
	CXMLElement *pElement;
	};

CRandomEntryResults::CRandomEntryResults (void) : m_Results(sizeof(ResultEntry), 5)

//	CRandomEntryResults constructor

	{
	}

CRandomEntryResults::~CRandomEntryResults (void)

//	CRandomEntryResults destructor

	{
	}

void CRandomEntryResults::AddResult (CXMLElement *pElement, int iCount)

//	AddResult
//
//	Adds a result

	{
	ResultEntry Entry;

	Entry.iCount = iCount;
	Entry.pElement = pElement;

	m_Results.AppendStruct(&Entry, NULL);
	}

CXMLElement *CRandomEntryResults::GetResult (int iIndex)

//	GetResult
//
//	Returns a result

	{
	ResultEntry *pEntry = (ResultEntry *)m_Results.GetStruct(iIndex);
	return pEntry->pElement;
	}

int CRandomEntryResults::GetResultCount (int iIndex)

//	GetResultCount
//
//	Returns the count for a result

	{
	ResultEntry *pEntry = (ResultEntry *)m_Results.GetStruct(iIndex);
	return pEntry->iCount;
	}
