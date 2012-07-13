//	CEntityResolverList.cpp
//
//	CEntityResolverList class

#include <windows.h>
#include "Alchemy.h"
#include "XMLUtil.h"

CString CEntityResolverList::ResolveExternalEntity (const CString &sName, bool *retbFound)

//	ResolveExternalEntity
//
//	Resolve entity

	{
	int i;

	for (i = 0; i < m_Resolvers.GetCount(); i++)
		{
		bool bFound;
		CString sResult = m_Resolvers[i]->ResolveExternalEntity(sName, &bFound);
		if (bFound)
			{
			if (retbFound)
				*retbFound = true;
			return sResult;
			}
		}

	//	Not found

	if (retbFound)
		*retbFound = false;

	return NULL_STR;
	}
