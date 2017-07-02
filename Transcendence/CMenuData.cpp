//	CMenuData.cpp
//
//	CMenuData class

#include "PreComp.h"
#include "Transcendence.h"

CMenuData::CMenuData (void) : m_iCount(0)

//	CMenuData constructor

	{
	}

void CMenuData::AddMenuItem (const CString &sKey,
							 const CString &sLabel,
							 const CObjectImageArray *pImage,
							 const CString &sExtra,
							 DWORD dwFlags,
							 DWORD dwData,
							 DWORD dwData2)

//	AddMenuItem
//
//	Add an item

	{
	ASSERT(m_iCount < MAX_MENU_ITEMS);
	if (m_iCount == MAX_MENU_ITEMS)
		return;

	//	If we have a key, sort by key. Otherwise, we
	//	add it at the end.

	int iPos;
	if (sKey.IsBlank() || !(dwFlags & FLAG_SORT_BY_KEY))
		iPos = m_iCount;
	else
		{
		iPos = 0;
		while (iPos < m_iCount 
				&& !m_List[iPos].sKey.IsBlank()
				&& strCompareAbsolute(sKey, m_List[iPos].sKey) > 0)
			iPos++;

		//	Move other items up

		for (int i = m_iCount - 1; i >= iPos; i--)
			m_List[i+1] = m_List[i];
		}

	//	Add item

	m_List[iPos].sKey = sKey;
	m_List[iPos].sLabel = sLabel;
	m_List[iPos].dwData = dwData;
	m_List[iPos].dwData2 = dwData2;
	m_List[iPos].sExtra = sExtra;
	m_List[iPos].dwFlags = dwFlags;
	m_List[iPos].pImage = pImage;

	m_iCount++;
	}

int CMenuData::FindItemByKey (const CString &sKey)

//	FindItemByKey
//
//	Returns the index of the menu item with the given key

	{
	for (int i = 0; i < m_iCount; i++)
		if (strEquals(sKey, m_List[i].sKey))
			return i;

	return -1;
	}

bool CMenuData::FindItemData (const CString &sKey, DWORD *retdwData, DWORD *retdwData2)

//	FindItemData
//
//	Returns the data associated with the menu item that
//	has the given key

	{
	int iIndex = FindItemByKey(sKey);
	if (iIndex != -1)
		{
		if (retdwData)
			*retdwData = m_List[iIndex].dwData;

		if (retdwData2)
			*retdwData2 = m_List[iIndex].dwData2;

		return true;
		}

	return false;
	}
