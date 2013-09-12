//	CCVector.cpp
//
//	Implements CCVector class

#include <windows.h>
#include "Kernel.h"
#include "KernelObjID.h"
#include "CodeChain.h"

static CObjectClass<CCVector>g_Class(OBJID_CCVECTOR, NULL);

CCVector::CCVector (void) : ICCList(&g_Class),
		m_pCC(NULL),
		m_iCount(0),
		m_pData(NULL)

//	CCVector constructor

	{
	}

CCVector::CCVector (CCodeChain *pCC) : ICCList(&g_Class),
		m_pCC(pCC),
		m_iCount(0),
		m_pData(NULL)

//	CCVector constructor

	{
	}

CCVector::~CCVector (void)

//	CCVector destructor

	{
	if (m_pData)
		MemFree(m_pData);
	}

ICCItem *CCVector::Clone (CCodeChain *pCC)

//	Clone
//
//	Clone this item

	{
	CCVector *pNewVector;
	ICCItem *pError;

	//	Create new vector

	pNewVector = new CCVector(pCC);
	if (pNewVector == NULL)
		return pCC->CreateMemoryError();

	//	Initialize

	pError = pNewVector->SetSize(pCC, m_iCount);
	if (pError->IsError())
		{
		delete pNewVector;
		return pError;
		}

	pError->Discard(pCC);

	//	Copy the vector

	if (m_pData)
		{
		int *pSource = m_pData;
		int *pDest = pNewVector->m_pData;
		int *pEnd = pDest + m_iCount;

		//	Copy

		while (pDest < pEnd)
			*pDest++ = *pSource++;
		}

	//	Done

	return pNewVector;
	}

void CCVector::DestroyItem (CCodeChain *pCC)

//	DestroyItem
//
//	Destroy this item

	{
	//	Free the vector

	if (m_pData)
		{
		MemFree(m_pData);
		m_pData = NULL;
		}

	//	Delete ourselves

	pCC->DestroyVector(this);
	}

ICCItem *CCVector::Enum (CEvalContext *pCtx, ICCItem *pCode)

//	Enum
//
//	

	{
	return pCtx->pCC->CreateNil();
	}

int *CCVector::GetArray (void)

//	GetArray
//
//	

	{
	return m_pData;
	}

ICCItem *CCVector::GetElement (int iIndex)

//	GetElement
//
//	Return the nth element in the vector (0-based)

	{
	ASSERT(m_pCC);

	if (iIndex < 0 || iIndex >= m_iCount)
		return m_pCC->CreateNil();

	return m_pCC->CreateInteger(m_pData[iIndex]);
	}

CString CCVector::Print (CCodeChain *pCC, DWORD dwFlags)

//	Print
//
//	Print a user-visible message

	{
	return strPatternSubst(LITERAL("[vector of %d elements]"), m_iCount);
	}

void CCVector::Reset (void)

//	Reset
//
//	Reset

	{
	}

BOOL CCVector::SetElement (int iIndex, int iElement)

//	SetElement
//
//	Sets the nth element in the vector (0-based)

	{
	if (iIndex < 0 || iIndex >= m_iCount)
		return FALSE;

	m_pData[iIndex] = iElement;
	return TRUE;
	}

ICCItem *CCVector::SetSize (CCodeChain *pCC, int iNewSize)

//	SetSize
//
//	Sets the size of the vector, preserving any previous data

	{
	int *pNewData = NULL;

	//	Allocate a new array

	if (iNewSize > 0)
		{
		int iInitialized;
		int i;

		pNewData = (int *)MemAlloc(iNewSize * sizeof(DWORD));
		if (pNewData == NULL)
			return pCC->CreateMemoryError();

		//	Copy the old array

		if (m_pData)
			{
			int *pSource = m_pData;
			int *pDest = pNewData;
			int *pEnd;

			//	Figure out how much to copy

			iInitialized = min(m_iCount, iNewSize);
			pEnd = pDest + iInitialized;

			//	Copy

			while (pDest < pEnd)
				*pDest++ = *pSource++;
			}
		else
			iInitialized = 0;

		//	Initialize the rest of the array

		for (i = iInitialized; i < iNewSize; i++)
			pNewData[i] = 0;
		}

	//	Free the original

	if (m_pData)
		MemFree(m_pData);

	//	Done

	m_pData = pNewData;
	m_iCount = iNewSize;

	return pCC->CreateTrue();
	}

ICCItem *CCVector::StreamItem (CCodeChain *pCC, IWriteStream *pStream)

//	StreamItem
//
//	Streams the vector to a stream

	{
	ALERROR error;

	//	Write out the count

	if (error = pStream->Write((char *)&m_iCount, sizeof(m_iCount), NULL))
		return pCC->CreateSystemError(error);

	//	Write out the data

	if (m_pData)
		{
		if (error = pStream->Write((char *)m_pData, m_iCount * sizeof(DWORD), NULL))
			return pCC->CreateSystemError(error);
		}

	//	Done

	return pCC->CreateTrue();
	}

ICCItem *CCVector::Tail (CCodeChain *pCC)

//	Tail
//
//	Returns the tail of the vector

	{
	return pCC->CreateNil();
	}

ICCItem *CCVector::UnstreamItem (CCodeChain *pCC, IReadStream *pStream)

//	UnstreamItem
//
//	Reads the vector from a stream

	{
	ALERROR error;
	int iCount;
	ICCItem *pError;

	//	Read the count

	if (error = pStream->Read((char *)&iCount, sizeof(iCount), NULL))
		return pCC->CreateSystemError(error);

	//	Resize the vector

	pError = SetSize(pCC, iCount);
	if (pError->IsError())
		return pError;

	pError->Discard(pCC);

	//	Read the data

	if (m_pData)
		{
		if (error = pStream->Read((char *)m_pData, iCount * sizeof(DWORD), NULL))
			return pCC->CreateSystemError(error);
		}

	//	Done

	m_iCount = iCount;

	return pCC->CreateTrue();
	}
