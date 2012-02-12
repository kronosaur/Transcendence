//	CExternalEntityTable.cpp
//
//	CExternalEntityTable class

#include <windows.h>
#include "Alchemy.h"
#include "XMLUtil.h"

CExternalEntityTable::CExternalEntityTable (void) :
		m_Entities(TRUE, FALSE),
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
		{
		CString *pNewValue = new CString(*(CString *)Table.GetValue(i));
		m_Entities.AddEntry(Table.GetKey(i), pNewValue);
		}
	}

void CExternalEntityTable::GetEntity (int iIndex, CString *retsEntity, CString *retsValue)

//	GetEntity
//
//	Returns the entity by index

	{
	*retsEntity = m_Entities.GetKey(iIndex);
	*retsValue = *(CString *)m_Entities.GetValue(iIndex);
	}

CString CExternalEntityTable::ResolveExternalEntity (const CString &sName, bool *retbFound)

//	ResolveExternalEntity
//
//	Resolves the entity

	{
	CString *pValue;
	if (m_Entities.Lookup(sName, (CObject **)&pValue) == NOERROR)
		{
		if (retbFound) *retbFound = true;
		return *pValue;
		}

	//	If not found, then try the parent

	if (m_pParent)
		return m_pParent->ResolveExternalEntity(sName, retbFound);

	//	Otherwise, not found

	if (retbFound) *retbFound = false;
	return sName;
	}
