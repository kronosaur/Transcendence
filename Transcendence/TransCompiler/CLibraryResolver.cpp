//	CLibraryResolver.cpp
//
//	CLibraryResolver Class
//	Copyright (c) 2013 by Kronosaur Productions, LLC. All Rights Reserved.

#include <stdio.h>

#include <windows.h>
#include <ddraw.h>
#include "Alchemy.h"
#include "XMLUtil.h"
#include "TDB.h"

CLibraryResolver::~CLibraryResolver (void)

//	CLibraryResolver destructor

	{
	int i;

	for (i = 0; i < m_Tables.GetCount(); i++)
		delete m_Tables[i];
	}

CString CLibraryResolver::ResolveExternalEntity (const CString &sName, bool *retbFound)

//	ResolveExternalEntity
//
//	Resolve the external entity

	{
	int i;

	for (i = 0; i < m_Tables.GetCount(); i++)
		{
		bool bFound;
		CString sResult = m_Tables[i]->ResolveExternalEntity(sName, &bFound);
		if (bFound)
			{
			if (retbFound)
				*retbFound = true;
			return sResult;
			}
		}

	if (retbFound)
		*retbFound = false;

	return NULL_STR;
	}
