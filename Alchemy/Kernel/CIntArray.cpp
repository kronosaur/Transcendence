//	CIntArray.cpp
//
//	Implementation of dynamic array of integers

#include "Kernel.h"
#include "KernelObjID.h"

#define ALLOC_INCREMENT					128

static DATADESCSTRUCT g_DataDesc[] =
	{	{ DATADESC_OPCODE_ALLOC_SIZE32,	1,	0 },		//	m_iAllocSize
		{ DATADESC_OPCODE_ALLOC_MEMORY,	1,	0 },		//	m_pData
		{ DATADESC_OPCODE_INT,			1,	0 },		//	m_iLength
		{ DATADESC_OPCODE_STOP,	0,	0 } };
static CObjectClass<CIntArray>g_ClassData(OBJID_CINTARRAY, g_DataDesc);

CIntArray::CIntArray (void) :
		CObject(&g_ClassData),
		m_iAllocSize(0),
		m_pData(NULL),
		m_iLength(0)

//	CIntArray constructor

	{
	}

CIntArray::~CIntArray (void)

//	CIntArray destructor

	{
	if (m_pData)
		{
		MemFree(m_pData);
		m_pData = NULL;
		}
	}

CIntArray &CIntArray::operator= (const CIntArray &Obj)

//	CIntArray operator=

	{
	if (m_pData)
		MemFree(m_pData);

	if (Obj.m_pData)
		{
		m_iAllocSize = Obj.m_iAllocSize;
		m_iLength = Obj.m_iLength;
		m_pData = (int *)::MemAlloc(sizeof(int) * (Obj.m_iAllocSize));
		
		utlMemCopy((char *)Obj.m_pData, (char *)m_pData, sizeof(int) * m_iLength);
		}
	else
		{
		m_iAllocSize = 0;
		m_iLength = 0;
		m_pData = NULL;
		}

	return *this;
	}

ALERROR CIntArray::AppendElement (int iElement, int *retiIndex)

//	AppendElement
//
//	Appends element to the array

	{
	return InsertElement(iElement, m_iLength, retiIndex);
	}

ALERROR CIntArray::ExpandArray (int iPos, int iCount)

//	ExpandArray
//
//	Make room in the array at the given position for the
//	given number of elements. If iPos is -1, the array is
//	expanded at the end.

	{
	int i;

	if (iPos < 0 || iPos > m_iLength)
		iPos = m_iLength;

    //	Reallocate if necessary
    
    if (m_iLength + iCount > m_iAllocSize)
    	{
        int *pNewData;
		int iInc;

		iInc = AlignUp(iCount, ALLOC_INCREMENT);
        
        //	Allocate a bigger buffer
        
        pNewData = (int *)MemAlloc(sizeof(int) * (m_iAllocSize + iInc));
        if (pNewData == NULL)
        	return ERR_MEMORY;
        
        //	Copy the data
        
		if (m_pData)
			{
			for (i = 0; i < m_iLength; i++)
				pNewData[i] = m_pData[i];
	        MemFree(m_pData);
			}

        m_pData = pNewData;
        m_iAllocSize += iInc;
    	}
        
	//	Move the array up
    
    for (i = m_iLength-1; i >= iPos; i--)
    	m_pData[i+iCount] = m_pData[i];
    
	m_iLength += iCount;

	return NOERROR;
	}

int CIntArray::FindElement (int iElement) const

//	FindElement
//
//	Looks for the given element in the array. If it is found it returns
//	the position of the element; otherwise it returns -1.

	{
	int i;

	for (i = 0; i < m_iLength; i++)
		if (m_pData[i] == iElement)
			return i;

	return -1;
	}

int CIntArray::GetCount (void) const

//	GetCount
//
//	Returns the number of elements in the array

	{
	return m_iLength;
	}

int CIntArray::GetElement (int iIndex) const

//	GetElement
//
//	Returns the nth element in the array

	{
	ASSERT(iIndex >= 0 && iIndex < m_iLength);
	return m_pData[iIndex];
	}

ALERROR CIntArray::InsertElement (int iElement, int iPos, int *retiIndex)

//	InsertElement
//
//	Inserts data into the list. iPos is the position in the list to insert at.
//	If -1, the data is inserted at the end of the list.

	{
	ALERROR error;

	if (iPos < 0 || iPos > m_iLength)
		iPos = m_iLength;

	if (error = ExpandArray(iPos, 1))
		return error;
    
	m_pData[iPos] = iElement;
    if (retiIndex)
    	*retiIndex = iPos;
        
    return NOERROR;
	}

ALERROR CIntArray::InsertRange (CIntArray *pList, int iStart, int iEnd, int iPos)

//	InsertRange
//
//	Inserts a range of integers from another array

	{
	ALERROR error;
	int i, iCount;

	if (iPos < 0 || iPos > m_iLength)
		iPos = m_iLength;

	iCount = iEnd - iStart + 1;

	if (error = ExpandArray(iPos, iCount))
		return error;
    
	//	Copy the data

	for (i = 0; i < iCount; i++)
		m_pData[iPos + i] = pList->GetElement(iStart + i);

	return NOERROR;
	}

ALERROR CIntArray::MoveRange (int iStart, int iEnd, int iPos)

//	MoveRange
//
//	Move the set of elements to the given position

	{
	int iDestSize;
	int iDest;
	int i;

	ASSERT(iStart >= 0 && iStart < m_iLength);
	ASSERT(iEnd >= 0 && iStart < m_iLength);
	ASSERT(iEnd >= iStart);

	if (iPos < 0 || iPos > m_iLength)
		iPos = m_iLength;

	//	If we're moving to the same position, do nothing

	if (iPos >= iStart && iPos <= iEnd+1)
		return NOERROR;

	//	The algorithm below is a one-pass in-place move
	//	that slides elements around.

	if (iPos > iEnd+1)
		{
		iDestSize = iEnd - iStart + 1;
		iDest = 0;

		for (i = iPos - 1; i > iStart; i--)
			{
			int iTemp;

			iTemp = m_pData[iEnd - iDest];
			m_pData[iEnd - iDest] = m_pData[i];
			m_pData[i] = iTemp;

			iDest = (iDest + 1) % iDestSize;
			}
		}
	else if (iPos < iStart)
		{
		iDestSize = iEnd - iStart + 1;
		iDest = 0;

		for (i = iPos; i < iEnd; i++)
			{
			int iTemp;

			iTemp = m_pData[iStart + iDest];
			m_pData[iStart + iDest] = m_pData[i];
			m_pData[i] = iTemp;

			iDest = (iDest + 1) % iDestSize;
			}
		}

	return NOERROR;
	}

ALERROR CIntArray::Set (int iCount, int *pData)

//	Set
//
//	Sets the contents of the array

	{
	if (m_pData)
		MemFree(m_pData);

    //	Allocate the data
    
	m_iAllocSize = (1 + (iCount / ALLOC_INCREMENT)) * ALLOC_INCREMENT;
	m_pData = (int *)MemAlloc(sizeof(int) * m_iAllocSize);
	if (m_pData == NULL)
		return ERR_MEMORY;
    
    //	Copy any initial data
    
    if (pData)
        {
		int i;

    	m_iLength = iCount;
		for (i = 0; i < iCount; i++)
			m_pData[i] = pData[i];
    	}
    else
    	m_iLength = 0;

	return NOERROR;
	}

ALERROR CIntArray::RemoveAll (void)

//	RemoveAll
//
//	Remove all elements of the array

	{
	m_iLength = 0;
	return NOERROR;
	}

ALERROR CIntArray::RemoveRange (int iStart, int iEnd)

//	RemoveRange
//
//	Removes elements from iStart to iEnd

	{
    int i, iCount;

    ASSERT(iStart >= 0 && iStart < m_iLength);
    ASSERT(iEnd >= 0 && iEnd < m_iLength);
    ASSERT(iStart <= iEnd);

	iCount = (iEnd - iStart) + 1;
    
    for (i = iEnd+1; i < m_iLength; i++)
    	m_pData[i-iCount] = m_pData[i];
    
    m_iLength -= iCount;
    
    return NOERROR;
	}

void CIntArray::ReplaceElement (int iPos, int iElement)

//	ReplaceElement
//
//	Replace the given element

	{
    ASSERT(iPos >= 0 && iPos < m_iLength);
    m_pData[iPos] = iElement;
	}

void CIntArray::Shuffle (void)

//	Shuffle
//
//	Shuffles the values of the array
//	Uses Fisher-Yates algorithm as implemented by Durstenfeld.

	{
	if (m_iLength < 2)
		return;

	int i = m_iLength - 1;
	while (i > 0)
		{
		int x = mathRandom(0, i);

		int iValue = m_pData[x];
		m_pData[x] = m_pData[i];
		m_pData[i] = iValue;

		i--;
		}
	}
