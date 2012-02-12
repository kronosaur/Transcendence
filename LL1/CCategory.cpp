//	CCategory.cpp
//
//	CCategory object

#include "PreComp.h"
#include "Tags.h"

CCategory::CCategory (void) : CObject(NULL),
		m_pCore(NULL)

//	CCategory constructor

	{
	}

CCategory::CCategory (CCore *pCore, CString sCategory) : CObject(NULL),
		m_pCore(pCore),
		m_sCategory(sCategory)

//	CCategory constructor

	{
	}

CCategory::~CCategory (void)

//	CCategory destructor

	{
	}

ALERROR CCategory::LoadFromEntry (CEntry *pEntry, CCategory **retpCategory)

//	LoadFromEntry
//
//	Creates a new category from the given entry

	{
	CField *pField;
	CString sCategory;
	CCategory *pCategory;

	//	This entry is the index for a category. First we figure out what
	//	category it is for.

	if (!pEntry->FindField(CONSTLIT("$Category"), &pField))
		return ERR_FAIL;

	sCategory = pField->GetValue(0);
	if (sCategory.IsBlank())
		return ERR_FAIL;

	//	Create the new object

	pCategory = new CCategory(pEntry->GetCore(), sCategory);
	if (pCategory == NULL)
		return ERR_FAIL;

	//	Since this entry is the index, set the index URL

	pCategory->m_sIndexURL = pCategory->m_pCore->MakeURLFromUNID(pEntry->GetUNID());

	//	Done

	*retpCategory = pCategory;

	return NOERROR;
	}
