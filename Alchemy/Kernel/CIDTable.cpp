//	CSymboTable.cpp
//
//	Implementation of a symbol table

#include "Kernel.h"
#include "KernelObjID.h"

static DATADESCSTRUCT g_DataDesc[] =
	{	{ DATADESC_OPCODE_EMBED_OBJ,	1,	0 },		//	CDictionary

		{ DATADESC_OPCODE_INT,			2,	0 },		//	m_bOwned, m_bNoReference
		{ DATADESC_OPCODE_STOP,	0,	0 } };
static CObjectClass<CIDTable>g_ClassData(OBJID_CIDTABLE, g_DataDesc);

CIDTable::CIDTable (void) : CDictionary(&g_ClassData),
		m_bOwned(FALSE),
		m_bNoReference(TRUE)

//	CIDTable constructor

	{
	}

CIDTable::CIDTable (BOOL bOwned, BOOL bNoReference) : CDictionary(&g_ClassData),
		m_bOwned(bOwned),
		m_bNoReference(bNoReference)

//	CIDTable constructor

	{
	}

CIDTable::~CIDTable (void)

//	CIDTable destructor

	{
	if (m_bOwned)
		{
		int i;

		for (i = 0; i < CDictionary::GetCount(); i++)
			{
			int iKey;
			CObject *pValue;

			CDictionary::GetEntry(i, &iKey, (int *)&pValue);
			delete pValue;
			}
		}
	}

int CIDTable::Compare (int iKey1, int iKey2) const

//	Compare
//
//	Compares the two keys and returns which is greater. If 0, both
//	keys are equal. If 1, Key1 is greater. If -1, Key2 is greater

	{
	if (iKey1 == iKey2)
		return 0;
	else if ((DWORD)iKey1 > (DWORD)iKey2)
		return 1;
	else
		return -1;
	}

void CIDTable::CopyHandler (CObject *pOriginal)

//	CopyHandler
//
//	If we own the objects in the array, we need to make copies
//	of the objects also

	{
#ifndef LATER
	//	Not yet implemented
	ASSERT(FALSE);
#endif
	}

int CIDTable::GetKey (int iEntry) const

//	GetKey
//
//	Returns the key of the nth entry

	{
	int iKey, iValue;

	GetEntry(iEntry, &iKey, &iValue);

	return iKey;
	}

CObject *CIDTable::GetValue (int iEntry) const

//	GetValue
//
//	Returns the value of the nth entry

	{
	int iKey, iValue;

	GetEntry(iEntry, &iKey, &iValue);
	return (CObject *)iValue;
	}

ALERROR CIDTable::LoadHandler (CUnarchiver *pUnarchiver)

//	LoadHandler
//
//	Loads object

	{
	ALERROR error;
	DWORD dwCount;
	int i;

	//	Read in whether or not we own the objects

	if (error = pUnarchiver->ReadData((char *)&m_bOwned, sizeof(BOOL)))
		return error;

	//	Read in whether or not the value is a reference

	if (error = pUnarchiver->ReadData((char *)&m_bNoReference, sizeof(BOOL)))
		return error;

	//	Read in a count of objects

	if (error = pUnarchiver->ReadData((char *)&dwCount, sizeof(DWORD)))
		return error;

	//	Make sure that there's room for all the objects

	if (error = CDictionary::ExpandArray(0, dwCount))
		return error;

	//	Read in the objects themselves

	for (i = 0; i < (int)dwCount; i++)
		{
		CObject *pValue;
		int iKey;

		//	Read in the key

		if (error = pUnarchiver->ReadData((char *)&iKey, sizeof(DWORD)))
			return error;

		//	If we own the object, read in the object

		if (m_bOwned)
			{
			if (error = pUnarchiver->LoadObject(&pValue))
				return error;
			}
		else if (m_bNoReference)
			{
			if (error = pUnarchiver->ReadData((char *)&pValue, sizeof(DWORD)))
				return error;
			}
#ifndef LATER
		//	We need to handle references here.
		else
			ASSERT(FALSE);
#endif

		CDictionary::SetEntry(i, iKey, (int)pValue);
		}

	return NOERROR;
	}

ALERROR CIDTable::Lookup (int iKey, CObject **retpValue) const

//	Lookup
//
//	Do a look up. If not found, returns ERR_NOTFOUND

	{
	ALERROR error;
	int iValue;

	if (error = CDictionary::Find(iKey, &iValue))
		return error;

	*retpValue = (CObject *)iValue;
	return NOERROR;
	}

ALERROR CIDTable::LookupEx (int iKey, int *retiEntry) const

//	LookupEx
//
//	Do a look up and return the entry number

	{
	ALERROR error;

	if (error = CDictionary::FindEx(iKey, retiEntry))
		return error;

	return NOERROR;
	}

ALERROR CIDTable::RemoveAll (void)

//	RemoveAll
//
//	Removes all entries

	{
	//	Delete all objects

	if (m_bOwned)
		{
		for (int i = 0; i < GetCount(); i++)
			{
			CObject *pObj = GetValue(i);
			if (pObj)
				delete pObj;
			}
		}

	//	Done

	return CDictionary::RemoveAll();
	}

ALERROR CIDTable::RemoveEntry (int iKey, CObject **retpOldValue)

//	RemoveEntry
//
//	Removes the given entry

	{
	ALERROR error;
	int iOldValue;

	//	Let the dictionary do the removing

	if (error = CDictionary::RemoveEntry(iKey, &iOldValue))
		return error;

	CObject *pOldObj = (CObject *)iOldValue;

	//	If the caller wants us to return the old value, do it; otherwise,
	//	we delete it, if necessary

	if (retpOldValue)
		*retpOldValue = pOldObj;
	else if (m_bOwned && pOldObj)
		delete pOldObj;

	return NOERROR;
	}

ALERROR CIDTable::ReplaceEntry (int iKey, CObject *pValue, BOOL bAdd, CObject **retpOldValue)

//	ReplaceEntry
//
//	If the key is found in the symbol table, it is replaced
//	with the given value. Otherwise, it returns GAERR_NOTFOUND

	{
	ALERROR error;
	int iOldValue;
	CObject *pOldObj;
	BOOL bAdded;

	//	Let the dictionary code do the actual adding

	if (error = CDictionary::ReplaceEntry(iKey, (int)pValue, bAdd, &bAdded, &iOldValue))
		return error;

	//	If we added a new object, then there is no old value

	if (bAdded)
		pOldObj = NULL;
	else
		pOldObj = (CObject *)iOldValue;

	//	If the caller wants us to return the old value, do it; otherwise,
	//	we delete it, if necessary

	if (retpOldValue)
		*retpOldValue = pOldObj;
	else if (m_bOwned && pOldObj)
		delete pOldObj;

	return NOERROR;
	}

ALERROR CIDTable::SaveHandler (CArchiver *pArchiver)

//	SaveHandler
//
//	Save object

	{
	ALERROR error;
	DWORD dwCount;
	int i;

	//	Write out whether we are owned or not

	if (error = pArchiver->WriteData((char *)&m_bOwned, sizeof(BOOL)))
		return error;

	//	Write out whether we are referencing or not

	if (error = pArchiver->WriteData((char *)&m_bNoReference, sizeof(BOOL)))
		return error;

	//	Write out the number of entries that we've got

	dwCount = (DWORD)CDictionary::GetCount();
	if (error = pArchiver->WriteData((char *)&dwCount, sizeof(DWORD)))
		return error;

	//	Write out each object

	for (i = 0; i < CDictionary::GetCount(); i++)
		{
		int iKey, iValue;

		CDictionary::GetEntry(i, &iKey, &iValue);

		//	Write out the key

		if (error = pArchiver->WriteData((char *)&iKey, sizeof(int)))
			return error;

		//	If we're owned, write out the value. If we're not a reference
		//	just write out the value; otherwise, write out the reference

		if (m_bOwned)
			{
			CObject *pValue = (CObject *)iValue;

			if (error = pArchiver->SaveObject(pValue))
				return error;
			}
		else if (m_bNoReference)
			{
			if (error = pArchiver->WriteData((char *)&iValue, sizeof(int)))
				return error;
			}
		else
			{
			int iID;
			CObject *pValue = (CObject *)iValue;

			if (error = pArchiver->Reference2ID(pValue, &iID))
				return error;

			if (error = pArchiver->WriteData((char *)&iID, sizeof(int)))
				return error;
			}
		}

	return NOERROR;
	}

void CIDTable::SetValue (int iEntry, CObject *pValue, CObject **retpOldValue)

//	SetValue
//
//	Sets the value

	{
	int iKey, iValue;

	GetEntry(iEntry, &iKey, &iValue);

	if (retpOldValue)
		*retpOldValue = (CObject *)iValue;

	CDictionary::SetEntry(iEntry, iKey, (int)pValue);
	}
