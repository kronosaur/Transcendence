//	CSymboTable.cpp
//
//	Implementation of a symbol table

#include "Kernel.h"
#include "KernelObjID.h"

static DATADESCSTRUCT g_DataDesc[] =
	{	{ DATADESC_OPCODE_EMBED_OBJ,	1,	0 },		//	CDictionary

		{ DATADESC_OPCODE_INT,			2,	0 },		//	m_bOwned, m_bNoReference
		{ DATADESC_OPCODE_STOP,	0,	0 } };
static CObjectClass<CSymbolTable>g_ClassData(OBJID_CSYMBOLTABLE, g_DataDesc);

#define VERSION2HACK				0xffffffff

CSymbolTable::CSymbolTable (void) : CDictionary(&g_ClassData),
		m_bOwned(FALSE),
		m_bNoReference(TRUE)

//	CSymbolTable constructor
//	HACK: We need to default to these settings so that the CAtomTable
//	object can allocate an array of CSymbolTables

	{
	}

CSymbolTable::CSymbolTable (BOOL bOwned, BOOL bNoReference) : CDictionary(&g_ClassData),
		m_bOwned(bOwned),
		m_bNoReference(bNoReference)

//	CSymbolTable constructor

	{
	}

CSymbolTable::~CSymbolTable (void)

//	CSymbolTable destructor

	{
	int i;

	for (i = 0; i < CDictionary::GetCount(); i++)
		{
		int iKey, iValue;
		CString *pKey;

		CDictionary::GetEntry(i, &iKey, &iValue);
		pKey = (CString *)iKey;

		delete pKey;
		if (m_bOwned)
			{
			CObject *pValue = (CObject *)iValue;
			delete pValue;
			}
		}
	}

CSymbolTable &CSymbolTable::operator= (const CSymbolTable &Obj)

//	CSymbolTable operator=

	{
	m_bOwned = Obj.m_bOwned;
	m_bNoReference = Obj.m_bNoReference;
	m_Array = Obj.m_Array;

	if (m_bOwned)
		CopyHandler((CSymbolTable *)&Obj);

	return *this;
	}

ALERROR CSymbolTable::AddEntry (const CString &sKey, CObject *pValue)

//	AddEntry
//
//	Add an entry to the symbol table

	{
	ALERROR error;
	CString *psKey;

	psKey = new CString(sKey);
	if (psKey == NULL)
		return ERR_MEMORY;

	//	Add key and value

	if (error = CDictionary::AddEntry((int)psKey, (int)pValue))
		{
		delete psKey;
		return error;
		}

	return NOERROR;
	}

int CSymbolTable::Compare (int iKey1, int iKey2) const

//	Compare
//
//	Compares two keys

	{
	CString *pKey1 = (CString *)iKey1;
	CString *pKey2 = (CString *)iKey2;

	return strCompareAbsolute(*pKey1, *pKey2);
	}

void CSymbolTable::CopyHandler (CObject *pOriginal)

//	CopyHandler
//
//	If we own the objects in the array, we need to make copies
//	of the objects also

	{
	int i;

	for (i = 0; i < GetCount(); i++)
		{
		//	Get the key and value

		int iKey, iValue;
		GetEntry(i, &iKey, &iValue);

		//	Convert to the appropriate thing

		CString *pKey = (CString *)iKey;
		CObject *pValue = (CObject *)iValue;

		//	Bump the ref-count on the string

		CString *pNewKey = (CString *)pKey->Copy();

		//	If we own the object then make a copy too

		CObject *pNewValue;
		if (m_bOwned)
			pNewValue = pValue->Copy();
		else
			pNewValue = pValue;

		//	Stuff the new values (we don't need to free the previous
		//	values since they are kept by the original).

		SetEntry(i, (int)pNewKey, (int)pNewValue);
		}
	}

CString CSymbolTable::GetKey (int iEntry) const

//	GetKey
//
//	Returns the key of the nth entry

	{
	int iKey, iValue;
	CString *pKey;

	GetEntry(iEntry, &iKey, &iValue);
	pKey = (CString *)iKey;

	return *pKey;
	}

CObject *CSymbolTable::GetValue (int iEntry) const

//	GetValue
//
//	Returns the value of the nth entry

	{
	int iKey, iValue;

	GetEntry(iEntry, &iKey, &iValue);
	return (CObject *)iValue;
	}

ALERROR CSymbolTable::LoadHandler (CUnarchiver *pUnarchiver)

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

	//	If count is -1, then this is a new version of the symbol table

	if (dwCount == VERSION2HACK)
		{
		//	Read in a count of objects

		if (error = pUnarchiver->ReadData((char *)&dwCount, sizeof(DWORD)))
			return error;

		//	Make sure that there's room for all the objects

		if (error = CDictionary::ExpandArray(0, dwCount))
			return error;

		//	Read in the objects themselves

		for (i = 0; i < (int)dwCount; i++)
			{
			CObject *pObject;
			CObject *pValue;
			CString *pKey;

			//	Read in the key

			if (error = pUnarchiver->LoadObject(&pObject))
				return error;

			pKey = dynamic_cast<CString *>(pObject);
			if (pKey == NULL)
				{
				delete pObject;
				return ERR_FAIL;
				}

			//	If we own the object, read in the object

			if (m_bOwned)
				{
				if (error = pUnarchiver->LoadObject(&pValue))
					{
					delete pKey;
					return error;
					}
				}
			else if (m_bNoReference)
				{
				if (error = pUnarchiver->ReadData((char *)&pValue, sizeof(DWORD)))
					{
					delete pKey;
					return error;
					}
				}
#ifndef LATER
			//	We need to handle references here.
			else
				ASSERT(FALSE);
#endif

			CDictionary::SetEntry(i, (int)pKey, (int)pValue);
			}
		}

	//	Otherwise, continue loading the old version in compatibility mode

	else
		{
		//	Read in the objects themselves

		for (i = 0; i < (int)dwCount; i++)
			{
			CObject *pObject;
			CObject *pValue;
			CString *pKey;

			//	Read in the key

			if (error = pUnarchiver->LoadObject(&pObject))
				return error;

			pKey = dynamic_cast<CString *>(pObject);
			if (pKey == NULL)
				{
				delete pObject;
				return ERR_FAIL;
				}

			//	If we own the object, read in the object

			if (m_bOwned)
				{
				if (error = pUnarchiver->LoadObject(&pValue))
					{
					delete pKey;
					return error;
					}
				}
			else if (m_bNoReference)
				{
				if (error = pUnarchiver->ReadData((char *)&pValue, sizeof(DWORD)))
					{
					delete pKey;
					return error;
					}
				}
#ifndef LATER
			//	We need to handle references here.
			else
				{
				ASSERT(FALSE);
				pValue = NULL;
				}
#endif

			//	For previous version we insert the objects in the table
			//	although this is less efficient, it is required because the
			//	sort order changed from version 1 to 2.

			if (error = CDictionary::AddEntry((int)pKey, (int)pValue))
				{
				delete pKey;
				delete pValue;
				return error;
				}
			}
		}

	return NOERROR;
	}

ALERROR CSymbolTable::Lookup (const CString &sKey, CObject **retpValue) const

//	Lookup
//
//	Do a look up. If not found, returns ERR_NOTFOUND

	{
	ALERROR error;
	int iValue;
	CString sKeyToFind(sKey);

	if (error = CDictionary::Find((int)&sKeyToFind, &iValue))
		return error;

	if (retpValue)
		*retpValue = (CObject *)iValue;

	return NOERROR;
	}

ALERROR CSymbolTable::LookupEx (const CString &sKey, int *retiEntry) const

//	LookupEx
//
//	Do a look up and return the entry number

	{
	ALERROR error;
	CString sKeyToFind(sKey);

	if (error = CDictionary::FindEx((int)&sKeyToFind, retiEntry))
		return error;

	return NOERROR;
	}

ALERROR CSymbolTable::RemoveAll (void)

//	RemoveAll
//
//	Remove all entries in symbol table

	{
	int i;

	for (i = 0; i < CDictionary::GetCount(); i++)
		{
		int iKey, iValue;
		CString *pKey;

		CDictionary::GetEntry(i, &iKey, &iValue);
		pKey = (CString *)iKey;

		delete pKey;
		if (m_bOwned)
			{
			CObject *pValue = (CObject *)iValue;
			delete pValue;
			}
		}

	return CDictionary::RemoveAll();
	}

ALERROR CSymbolTable::RemoveEntry (int iEntry, CObject **retpOldValue)

//	RemoveEntry
//
//	Removes the given entry

	{
	ALERROR error;
	int iOldValue;

	//	Let the dictionary do the removing

	if (error = CDictionary::RemoveEntryByOrdinal(iEntry, &iOldValue))
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

ALERROR CSymbolTable::RemoveEntry (const CString &sKey, CObject **retpOldValue)

//	RemoveEntry
//
//	Removes the given entry

	{
	ALERROR error;
	int iOldValue;

	//	Let the dictionary do the removing

	if (error = CDictionary::RemoveEntry((int)&sKey, &iOldValue))
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

ALERROR CSymbolTable::ReplaceEntry (const CString &sKey, CObject *pValue, BOOL bAdd, CObject **retpOldValue)

//	ReplaceEntry
//
//	If the key is found in the symbol table, it is replaced
//	with the given value. Otherwise, it returns ERR_NOTFOUND

	{
	ALERROR error;
	int iOldValue;
	CObject *pOldObj;
	CString *pKey;
	BOOL bAdded;

	//	We need to allocate a new string in case the key is added

	if (bAdd)
		{
		pKey = new CString(sKey);
		if (pKey == NULL)
			return ERR_MEMORY;
		}
	else
		pKey = const_cast<CString *>(&sKey);

	//	Let the dictionary code do the actual adding

	if (error = CDictionary::ReplaceEntry((int)pKey, (int)pValue, bAdd, &bAdded, &iOldValue))
		return error;

	//	If we didn't actually add a key, then we free the string
	//	that we allocated

	if (bAdd && !bAdded)
		delete pKey;

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

ALERROR CSymbolTable::SaveHandler (CArchiver *pArchiver)

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

	//	Write out -1 to indicate that this is the new version

	dwCount = VERSION2HACK;
	if (error = pArchiver->WriteData((char *)&dwCount, sizeof(DWORD)))
		return error;

	//	Write out the number of entries that we've got

	dwCount = (DWORD)CDictionary::GetCount();
	if (error = pArchiver->WriteData((char *)&dwCount, sizeof(DWORD)))
		return error;

	//	Write out each object

	for (i = 0; i < CDictionary::GetCount(); i++)
		{
		int iKey, iValue;
		CString *pKey;

		CDictionary::GetEntry(i, &iKey, &iValue);
		pKey = (CString *)iKey;

		//	Write out the key

		if (error = pArchiver->SaveObject(pKey))
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

void CSymbolTable::SetValue (int iEntry, CObject *pValue, CObject **retpOldValue)

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
