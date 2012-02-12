//	CSpaceObjectTable.cpp
//
//	CSpaceObjectTable class

#include "PreComp.h"


CSpaceObjectTable::CSpaceObjectTable (void) : 
		m_Table(FALSE, TRUE)

//	CSpaceObjectTable constructor

	{
	}

void CSpaceObjectTable::Add (const CString &sKey, CSpaceObject *pObj)

//	Add
//
//	Add an object

	{
	m_Table.AddEntry(sKey, pObj);
	}

bool CSpaceObjectTable::Find (CSpaceObject *pObj, int *retiIndex)

//	Find
//
//	Find the object in the table

	{
	int i;

	for (i = 0; i < m_Table.GetCount(); i++)
		{
		if (m_Table.GetValue(i) == pObj)
			{
			if (retiIndex)
				*retiIndex = i;
			return true;
			}
		}

	return false;
	}

bool CSpaceObjectTable::Remove (CSpaceObject *pObj)

//	Remove
//
//	Removes the given object (if it exists). Returns if the object was
//	deleted or not.

	{
	int iIndex;

	if (Find(pObj, &iIndex))
		{
		m_Table.RemoveEntry(m_Table.GetKey(iIndex), NULL);
		return true;
		}
	else
		return false;
	}
