//	CObjectArray.cpp
//
//	Implementation of dynamic array of objects

#include "Kernel.h"
#include "KernelObjID.h"

static DATADESCSTRUCT g_DataDesc[] =
	{	{ DATADESC_OPCODE_INT,			1,	0 },		//	m_bOwned
		{ DATADESC_OPCODE_EMBED_OBJ,	1,	0 },		//	m_Array
		{ DATADESC_OPCODE_STOP,	0,	0 } };
static CObjectClass<CObjectArray>g_ClassData(OBJID_COBJECTARRAY,g_DataDesc);

CObjectArray::CObjectArray (void) :
		CObject(&g_ClassData),
		m_bOwned(FALSE)

//	CObjectArray constructor

	{
	}

CObjectArray::CObjectArray (BOOL bOwned) :
		CObject(&g_ClassData),
		m_bOwned(bOwned)

//	CObjectArray constructor

	{
	}

CObjectArray::~CObjectArray (void)

//	CObjectArray destructor

	{
	//	If we own the objects in the array, delete them now

	RemoveAll();
	}

void CObjectArray::CopyHandler (CObject *pOriginal)

//	CopyHandler
//
//	If we own the objects in the array, we need to make copies
//	of the objects also

	{
	if (m_bOwned)
		{
		int i;

		for (i = 0; i < GetCount(); i++)
			{
			CObject *pObj = GetObject(i);

			//	Since the m_Array is just a copy of the original, we
			//	cannot free the object that is in the array before
			//	we stuff the new copy.

			m_Array.ReplaceElement(i, (int)pObj->Copy());
			}
		}
	}

CObject *CObjectArray::DetachObject (int iIndex)

//	DetachObject
//
//	This removes the object from the array and returns it.

	{
	CObject *pObject = GetObject(iIndex);

	//	We remove the object whether we are owned or not

	m_Array.RemoveElement(iIndex);
	return pObject;
	}

int CObjectArray::FindObject (CObject *pObj)

//	FindObject
//
//	Returns the index of the given object (or -1 if not found)

	{
	int i;

	for (i = 0; i < GetCount(); i++)
		if (pObj == GetObject(i))
			return i;

	return -1;
	}

ALERROR CObjectArray::LoadDoneHandler (void)

//	LoadDoneHandler
//
//	Tell our objects that we're done loading

	{
	ALERROR error;

	if (m_bOwned)
		{
		int i;

		for (i = 0; i < GetCount(); i++)
			{
			CObject *pObj = GetObject(i);

			if (error = pObj->LoadDone())
				return error;
			}
		}

	return NOERROR;
	}

ALERROR CObjectArray::LoadHandler (CUnarchiver *pUnarchiver)

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

	//	Read in a count of objects

	if (error = pUnarchiver->ReadData((char *)&dwCount, sizeof(DWORD)))
		return error;

	//	Read in the objects themselves

	for (i = 0; i < (int)dwCount; i++)
		{
		if (m_bOwned)
			{
			CObject *pObj;

			if (error = pUnarchiver->LoadObject(&pObj))
				return error;

			//	Insert the object into our list

			m_Array.AppendElement((int)pObj, NULL);
			}
		else
			ASSERT(FALSE);
		}

	return NOERROR;
	}

void CObjectArray::RemoveAll (void)

//	RemoveAll
//
//	Remove all objects from the array
//	If the objects are owned by us, we delete them before removing them

	{
	if (m_bOwned)
		{
		int i;

		for (i = 0; i < GetCount(); i++)
			delete GetObject(i);
		}

	m_Array.RemoveAll();
	}

void CObjectArray::RemoveObject (int iPos)

//	RemoveObject
//
//	Removes an object from the array

	{
	if (m_bOwned)
		delete GetObject(iPos);

	m_Array.RemoveElement(iPos);
	}

void CObjectArray::ReplaceObject (int iPos, CObject *pObj, bool bDelete)

//	ReplaceObject
//
//	Replaces one object with another

	{
	if (m_bOwned && bDelete)
		delete GetObject(iPos);

	m_Array.ReplaceElement(iPos, (int)pObj);
	}

ALERROR CObjectArray::SaveHandler (CArchiver *pArchiver)

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

	//	Write out the number of objects that we've got

	dwCount = (DWORD)GetCount();
	if (error = pArchiver->WriteData((char *)&dwCount, sizeof(DWORD)))
		return error;

	//	Write out each object

	for (i = 0; i < GetCount(); i++)
		{
		//	If we own the object, write out the entire object; otherwise
		//	we just write out a reference.

		if (m_bOwned)
			{
			CObject *pObj = GetObject(i);

			if (error = pArchiver->SaveObject(pObj))
				return error;
			}
		else
			ASSERT(FALSE);
		}

	return NOERROR;
	}

