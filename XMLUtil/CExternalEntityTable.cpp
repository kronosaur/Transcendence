//	CExternalEntityTable.cpp
//
//	CExternalEntityTable class

#include <windows.h>
#include "Alchemy.h"
#include "XMLUtil.h"

CExternalEntityTable::CExternalEntityTable (void) :
		m_pParent(NULL)

//	CExternalEntityTable constructor

	{
	}

void CExternalEntityTable::AddTable (CSymbolTable &Table)

//	AddTable
//
//	Adds all entities in the given table

	{
	int i;

	for (i = 0; i < Table.GetCount(); i++)
		m_Entities.Insert(Table.GetKey(i), *(CString *)Table.GetValue(i));
	}

void CExternalEntityTable::GetEntity (int iIndex, CString *retsEntity, CString *retsValue)

//	GetEntity
//
//	Returns the entity by index

	{
	*retsEntity = m_Entities.GetKey(iIndex);
	*retsValue = m_Entities[iIndex];
	}

CString CExternalEntityTable::ResolveExternalEntity (const CString &sName, bool *retbFound)

//	ResolveExternalEntity
//
//	Resolves the entity

	{
	CString sValue;

	if (m_Entities.Find(sName, &sValue))
		{
		if (retbFound) *retbFound = true;
		return sValue;
		}

	//	If not found, then try the parent

	if (m_pParent)
		return m_pParent->ResolveExternalEntity(sName, retbFound);

	//	Otherwise, not found

	if (retbFound) *retbFound = false;

	return sName;
	}
