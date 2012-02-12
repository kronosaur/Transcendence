//	CLargeSet.cpp
//
//	CLargeSet class

#include "Kernel.h"
#include "KernelObjID.h"

const int BITS_PER_DWORD =						32;

CLargeSet::CLargeSet (int iSize)

//	CLargeSet constructor

	{
	if (iSize != -1)
		{
		int iCount = AlignUp(iSize, BITS_PER_DWORD) / BITS_PER_DWORD;
		m_Set.InsertEmpty(iCount);
		for (int i = 0; i < iCount; i++)
			m_Set[i] = 0;
		}
	}

void CLargeSet::Clear (DWORD dwValue)

//	Clear
//
//	Remove the value from the set

	{
	DWORD dwPos = dwValue / BITS_PER_DWORD;
	DWORD dwBit = dwValue % BITS_PER_DWORD;

	if (dwPos >= (DWORD)m_Set.GetCount())
		return;

	m_Set[dwPos] = (m_Set[dwPos] & ~(1 << dwBit));
	}

void CLargeSet::ClearAll (void)

//	ClearAll
//
//	Removes all bits

	{
	m_Set.DeleteAll();
	}

bool CLargeSet::IsEmpty (void) const

//	IsEmpty
//
//	Returns TRUE if empty

	{
	int i;

	for (i = 0; i < m_Set.GetCount(); i++)
		if (m_Set[i])
			return false;

	return true;
	}

bool CLargeSet::IsSet (DWORD dwValue) const

//	IsSet
//
//	Returns TRUE if the value is in the set

	{
	DWORD dwPos = dwValue / BITS_PER_DWORD;
	DWORD dwBit = dwValue % BITS_PER_DWORD;

	if (dwPos >= (DWORD)m_Set.GetCount())
		return false;

	return ((m_Set[dwPos] & (1 << dwBit)) ? true : false);
	}

void CLargeSet::Set (DWORD dwValue)

//	Set
//
//	Add the value to the set

	{
	DWORD dwPos = dwValue / BITS_PER_DWORD;
	DWORD dwBit = dwValue % BITS_PER_DWORD;
	
	int iOldCount = m_Set.GetCount();
	if (dwPos >= (DWORD)iOldCount)
		{
		m_Set.InsertEmpty(dwPos - iOldCount + 1);
		for (int i = iOldCount; i < m_Set.GetCount(); i++)
			m_Set[i] = 0;
		}

	m_Set[dwPos] = (m_Set[dwPos] | (1 << dwBit));
	}
