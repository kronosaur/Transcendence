//	CIntSet.cpp
//
//	Implementation of a set of (small) integers

#include "Kernel.h"
#include "KernelObjID.h"

#ifdef DEPRECATED

static DATADESCSTRUCT g_DataDesc[] =
	{	{ DATADESC_OPCODE_EMBED_OBJ,	1,	0 },		//	m_Set
		{ DATADESC_OPCODE_STOP,	0,	0 } };
static CObjectClass<CIntSet>g_ClassData(OBJID_CINTSET, g_DataDesc);

const int g_BitsPerBucket = 1024;
const int g_ElementsPerBucket = g_BitsPerBucket / (8 * sizeof(int));

CIntSet::CIntSet (void) : CObject(&g_ClassData),
		m_Set(TRUE, FALSE)

//	CIntSet constructor

	{
	}

ALERROR CIntSet::AddElement (int iElement)

//	AddElement
//
//	Adds an element to the set

	{
	ALERROR error;
	DWORD dwBucket = ((DWORD)iElement) / g_BitsPerBucket;

	//	Look for the bucket; if we don't find it, add a new one

	CIntArray *pBucket;
	if (error = m_Set.Lookup(dwBucket, (CObject **)&pBucket))
		{
		pBucket = new CIntArray;
		if (pBucket == NULL)
			return ERR_MEMORY;

		if (error = m_Set.AddEntry(dwBucket, pBucket))
			{
			delete pBucket;
			return error;
			}
		}

	//	Look for the bit in the bucket

	DWORD dwBucketBit = ((DWORD)iElement) % g_BitsPerBucket;
	DWORD dwOffset = dwBucketBit / 32;

	//	Make sure there is room in the bucket

	if ((int)dwOffset >= pBucket->GetCount())
		{
		int iOldCount = pBucket->GetCount();
		int iExpandBy = ((int)dwOffset + 1) - iOldCount;

		if (error = pBucket->ExpandArray(-1, iExpandBy))
			return error;

		for (int i = iOldCount; i < pBucket->GetCount(); i++)
			pBucket->ReplaceElement(i, 0);
		}

	//	OR-in the new element

	DWORD dwBit = dwBucketBit % 32;
	DWORD dwNewValue = ((DWORD)pBucket->GetElement(dwOffset)) | (1 << dwBit);
	pBucket->ReplaceElement(dwOffset, dwNewValue);

	return NOERROR;
	}

ALERROR CIntSet::EnumElements (CIntArray *pArray) const

//	EnumElements
//
//	Fills in the given array with an enumeration of all the
//	elements in the set.

	{
	ALERROR error;

	//	Loop over all buckets

	int i;
	for (i = 0; i < m_Set.GetCount(); i++)
		{
		DWORD dwBucket = (DWORD)m_Set.GetKey(i) * (DWORD)g_BitsPerBucket;
		CIntArray *pBucket = (CIntArray *)m_Set.GetValue(i);

		//	Now look over all elements in the bucket

		int j;
		for (j = 0; j < pBucket->GetCount(); j++)
			{
			DWORD dwElement = (DWORD)pBucket->GetElement(j);

			if (dwElement != 0)
				{
				//	Iterate over all bits

				int k;
				DWORD dwTest = 1;
				for (k = 0; k < 32; k++)
					{
					if (dwElement & dwTest)
						{
						if (error = pArray->AppendElement(dwBucket + (DWORD)(j * 32 + k), NULL))
							return error;
						}

					dwTest = dwTest << 1;
					}
				}
			}
		}

	return NOERROR;
	}

bool CIntSet::HasElement (int iElement) const

//	HasElement
//
//	Returns TRUE if the set has the given element

	{
	DWORD dwBucket = ((DWORD)iElement) / g_BitsPerBucket;

	//	Look for the bucket

	CIntArray *pBucket;
	if (m_Set.Lookup(dwBucket, (CObject **)&pBucket) != NOERROR)
		return false;

	DWORD dwBucketBit = ((DWORD)iElement) % g_BitsPerBucket;
	DWORD dwOffset = dwBucketBit / 32;

	if ((int)dwOffset >= pBucket->GetCount())
		return false;

	DWORD dwBit = dwBucketBit % 32;
	return (((DWORD)pBucket->GetElement(dwOffset)) & (1 << dwBit) ? true : false);
	}

void CIntSet::RemoveElement (int iElement)

//	RemoveElement
//
//	Removes the given element in the set

	{
	DWORD dwBucket = ((DWORD)iElement) / g_BitsPerBucket;

	//	Look for the bucket

	CIntArray *pBucket;
	if (m_Set.Lookup(dwBucket, (CObject **)&pBucket) != NOERROR)
		return;

	DWORD dwBucketBit = ((DWORD)iElement) % g_BitsPerBucket;
	DWORD dwOffset = dwBucketBit / 32;

	if ((int)dwOffset >= pBucket->GetCount())
		return;

	DWORD dwBit = dwBucketBit % 32;
	DWORD dwNewValue = ((DWORD)pBucket->GetElement(dwOffset)) & ~(1 << dwBit);
	pBucket->ReplaceElement(dwOffset, dwNewValue);
	}

#endif