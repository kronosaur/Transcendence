//	CStringArray.cpp
//
//	Implementation of dynamic array of string objects

#include <windows.h>
#include "Kernel.h"
#include "KernelObjID.h"

#define ALLOC_QUANTUM					(8 * sizeof(CString))

static DATADESCSTRUCT g_DataDesc[] =
	{	{ DATADESC_OPCODE_ZERO,			2,	0 },		//	CINTDynamicArray
		{ DATADESC_OPCODE_INT,			1,	0 },		//	CINTDynamicArray
		{ DATADESC_OPCODE_ZERO,			1,	0 },		//	CINTDynamicArray
		{ DATADESC_OPCODE_STOP,	0,	0 } };
static CObjectClass<CStringArray>g_ClassData(OBJID_CSTRINGARRAY, g_DataDesc);

CStringArray::CStringArray (void) : CObject(&g_ClassData)

//	CStringArray constructor

	{
	}

CStringArray::~CStringArray (void)

//	CStringArray destructor

	{
	//	If we own the objects in the array, delete them now

	RemoveAll();
	}

CStringArray::CStringArray (const CStringArray &Source) : CObject(&g_ClassData)

//	copy constructor

	{
	int i;

	//	Copy all the strings

	for (i = 0; i < Source.GetCount(); i++)
		AppendString(Source.GetStringValue(i), NULL);
	}

CStringArray &CStringArray::operator= (const CStringArray &Source)

//	operator =
//
//	Overrides the assignment operator

	{
	int i;

	RemoveAll();

	//	Copy all the strings

	for (i = 0; i < Source.GetCount(); i++)
		AppendString(Source.GetStringValue(i), NULL);

	return *this;
	}

void CStringArray::CopyHandler (CObject *pOriginal)

//	CopyHandler
//
//	If we own the objects in the array, we need to make copies
//	of the objects also

	{
	CStringArray *pSource = (CStringArray *)pOriginal;
	int i;

	//	Copy the data

	m_Array.Append(pSource->m_Array.GetPointer(0),
			pSource->m_Array.GetLength(),
			ALLOC_QUANTUM);

	//	Bump the refcount for all strings

	for (i = 0; i < GetCount(); i++)
		{
		CString *pString = GetString(i);
		pString->IncRefCount();
		}
	}

int CStringArray::FindString (const CString &sString)

//	FindString
//
//	Returns the position of the given string or -1 if not found

	{
	int i;

	for (i = 0; i < GetCount(); i++)
		{
		CString *pString = GetString(i);

		if (strCompare(sString, *pString) == 0)
			return i;
		}

	return -1;
	}

int CStringArray::GetCount (void) const

//	GetCount
//
//	Returns the number of strings in the array

	{
	ASSERT(m_Array.GetLength() % sizeof(CString) == 0);
	return m_Array.GetLength() / sizeof(CString);
	}

CString *CStringArray::GetString (int iIndex) const

//	GetString
//
//	Returns a pointer to the given string

	{
	ASSERT(iIndex >= 0 && iIndex < GetCount());
	return (CString *)m_Array.GetPointer(iIndex * sizeof(CString));
	}

ALERROR CStringArray::InsertString (const CString &sString, int iPos, int *retiIndex)

//	InsertObject
//
//	Inserts the string

	{
	ALERROR error;

	if (iPos == -1)
		iPos = GetCount();

	//	Increment ref count

	((CString &)sString).IncRefCount();

	//	Append

	if (error = m_Array.Insert(iPos * sizeof(CString), (BYTE *)&sString, sizeof(CString), ALLOC_QUANTUM))
		return error;

	//	Done

	if (retiIndex)
		*retiIndex = iPos;

	return NOERROR;
	}

ALERROR CStringArray::LoadDoneHandler (void)

//	LoadDoneHandler
//
//	Tell our objects that we're done loading

	{
	ALERROR error;
	int i;

	//	Tell the strings

	for (i = 0; i < GetCount(); i++)
		{
		CString *pString = GetString(i);
		if (error = pString->LoadDone())
			return error;
		}

	return NOERROR;
	}

ALERROR CStringArray::LoadHandler (CUnarchiver *pUnarchiver)

//	LoadHandler
//
//	Loads object

	{
	ALERROR error;
	DWORD dwCount;
	int i;

	//	Read in a count of objects

	if (error = pUnarchiver->ReadData((char *)&dwCount, sizeof(DWORD)))
		return error;

	//	Read in the objects themselves

	for (i = 0; i < (int)dwCount; i++)
		{
		CObject *pObj;

		if (error = pUnarchiver->LoadObject(&pObj))
			return error;

		if (pObj->GetClass()->GetObjID() != OBJID_CSTRING)
			return ERR_FAIL;

		if (error = AppendString(*((CString *)pObj), NULL))
			return error;
		}

	return NOERROR;
	}

ALERROR CStringArray::RemoveString (int iPos)

//	RemoveString
//
//	Removes the string at the given position

	{
	//	Decrement the refcount

	CString *pString = GetString(iPos);
	pString->DecRefCount();

	//	Close up the array

	return m_Array.Delete(iPos * sizeof(CString), sizeof(CString));
	}

void CStringArray::RemoveAll (void)

//	RemoveAll
//
//	Remove all objects from the array
//	If the objects are owned by us, we delete them before removing them

	{
	int i;

	for (i = 0; i < GetCount(); i++)
		{
		CString *pString = GetString(i);
		pString->DecRefCount();
		}

	m_Array.DeleteAll();
	}

ALERROR CStringArray::SaveHandler (CArchiver *pArchiver)

//	SaveHandler
//
//	Save object

	{
	ALERROR error;
	DWORD dwCount;
	int i;

	//	Write out the number of objects that we've got

	dwCount = (DWORD)GetCount();
	if (error = pArchiver->WriteData((char *)&dwCount, sizeof(DWORD)))
		return error;

	//	Write out each object

	for (i = 0; i < GetCount(); i++)
		{
		CString *pString = GetString(i);

		if (error = pArchiver->SaveObject(pString))
			return error;
		}

	return NOERROR;
	}

