//	CStructArray.cpp
//
//	Implementation of dynamic array of simple structs

#include "Kernel.h"
#include "KernelObjID.h"

#define ALLOC_QUANTUM					(16)

static DATADESCSTRUCT g_DataDesc[] =
	{	{ DATADESC_OPCODE_ZERO,			4,	0 },		//	CINTDynamicArray
		{ DATADESC_OPCODE_INT,			2,	0 },		//	m_iElementSize, m_iInitSize
		{ DATADESC_OPCODE_STOP,	0,	0 } };
static CObjectClass<CStructArray>g_ClassData(OBJID_CSTRUCTARRAY, g_DataDesc);

CStructArray::CStructArray (void) : CObject(&g_ClassData)

//	CStructArray constructor

	{
	}

CStructArray::CStructArray (int iElementSize, int iInitSize) : CObject(&g_ClassData),
		m_iElementSize(iElementSize),
		m_iInitSize(iInitSize)

//	CStructArray constructor

	{
	}

void CStructArray::CopyHandler (CObject *pOriginal)

//	CopyHandler
//
//	If we own the objects in the array, we need to make copies
//	of the objects also

	{
	ASSERT(FALSE);
	}

ALERROR CStructArray::ExpandArray (int iPos, int iCount)

//	ExpandArray
//
//	Expands the structure array

	{
	ALERROR error;
	int iAllocSize;

	if (iPos == -1)
		iPos = GetCount();

	if (GetCount() == 0)
		iAllocSize = m_iInitSize * m_iElementSize;
	else
		iAllocSize = ALLOC_QUANTUM * m_iElementSize;

	if (error = m_Array.Insert(iPos * m_iElementSize,
			NULL,
			iCount * m_iElementSize,
			iAllocSize))
		return error;

	return NOERROR;
	}

int CStructArray::GetCount (void) const

//	GetCount
//
//	Returns the number of strings in the array

	{
	ASSERT(m_Array.GetLength() % m_iElementSize == 0);
	return m_Array.GetLength() / m_iElementSize;
	}

void *CStructArray::GetStruct (int iIndex) const

//	GetStruct
//
//	Returns a pointer to the given struct

	{
	ASSERT(iIndex >= 0 && iIndex < GetCount());
	return m_Array.GetPointer(iIndex * m_iElementSize);
	}

ALERROR CStructArray::InsertStruct (void *pData, int iPos, int *retiIndex)

//	InsertStruct
//
//	Inserts the struct

	{
	ALERROR error;
	int iAllocSize;

	if (iPos == -1)
		iPos = GetCount();

	//	Figure out the allocation granularity

	if (GetCount() == 0)
		iAllocSize = m_iInitSize * m_iElementSize;
	else
		iAllocSize = ALLOC_QUANTUM * m_iElementSize;

	//	Append

	if (error = m_Array.Insert(iPos * m_iElementSize,
			(BYTE *)pData,
			m_iElementSize,
			iAllocSize))
		return error;

	//	Done

	if (retiIndex)
		*retiIndex = iPos;

	return NOERROR;
	}

ALERROR CStructArray::LoadHandler (CUnarchiver *pUnarchiver)

//	LoadHandler
//
//	Loads object

	{
	ALERROR error;
	DWORD dwCount;

	//	Read in the element size

	if (error = pUnarchiver->ReadData((char *)&m_iElementSize, sizeof(DWORD)))
		return error;

	//	Read in the init size

	if (error = pUnarchiver->ReadData((char *)&m_iInitSize, sizeof(DWORD)))
		return error;

	//	Read in a count of objects

	if (error = pUnarchiver->ReadData((char *)&dwCount, sizeof(DWORD)))
		return error;

	//	Read in the objects themselves

	if (error = m_Array.Resize(dwCount * m_iElementSize, FALSE, 0))
		return error;

	if (error = pUnarchiver->ReadData((char *)m_Array.GetPointer(0),
			dwCount * m_iElementSize))
		return error;

	return NOERROR;
	}

void CStructArray::Remove (int iIndex)

//	Remove
//
//	Removes the given structure

	{
	m_Array.Delete(iIndex * m_iElementSize, m_iElementSize);
	}

void CStructArray::RemoveAll (void)

//	RemoveAll
//
//	Remove all objects from the array
//	If the objects are owned by us, we delete them before removing them

	{
	m_Array.DeleteAll();
	}

ALERROR CStructArray::SaveHandler (CArchiver *pArchiver)

//	SaveHandler
//
//	Save object

	{
	ALERROR error;
	DWORD dwCount;

	//	Write out the element size

	if (error = pArchiver->WriteData((char *)&m_iElementSize, sizeof(DWORD)))
		return error;

	//	And the init size

	if (error = pArchiver->WriteData((char *)&m_iInitSize, sizeof(DWORD)))
		return error;

	//	Write out the number of objects that we've got

	dwCount = (DWORD)GetCount();
	if (error = pArchiver->WriteData((char *)&dwCount, sizeof(DWORD)))
		return error;

	//	Write the entire table

	if (error = pArchiver->WriteData((char *)m_Array.GetPointer(0),
			dwCount * m_iElementSize))
		return error;

	return NOERROR;
	}

void CStructArray::SetStruct (int iIndex, void *pData)

//	SetStruct
//
//	Sets the given structure

	{
	ASSERT(iIndex >= 0 && iIndex < GetCount());
	utlMemCopy((char *)pData,
			(char *)m_Array.GetPointer(iIndex * m_iElementSize),
			m_iElementSize);
	}
