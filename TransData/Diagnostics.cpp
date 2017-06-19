//	Diagnostics.cpp
//
//	Generate game diagnostics.
//	Copyright (c) 2017 Kronosaur Productions, LLC. All Rights Reserved.

#include <stdio.h>

#include <windows.h>
#include "Alchemy.h"
#include "XMLUtil.h"
#include "TransData.h"

void GenerateDiagnostics (CUniverse &Universe, CXMLElement *pCmdLine)
	{
	int i;
	ALERROR error;
	CString sError;
	CSovereign *pPlayer = Universe.FindSovereign(g_PlayerSovereignUNID);

	//	Create all systems.

	TSortMap<CString, CSystem *> AllSystems;
	for (i = 0; i < Universe.GetTopologyNodeCount(); i++)
		{
		CTopologyNode *pNode = Universe.GetTopologyNode(i);

		//	Do not try to create end game nodes

		if (pNode->IsEndGame())
			continue;

		//	Create this system

		CSystem *pNewSystem;
		if (error = Universe.CreateStarSystem(pNode, &pNewSystem, &sError))
			{
			printf("Error creating system %s: %s\n", (LPSTR)pNode->GetID(), (LPSTR)sError);
			return;
			}

		AllSystems.SetAt(pNode->GetID(), pNewSystem);

		printf("Created %s\n", (LPSTR)pNode->GetSystemName());
		}

	//	Now loop over all systems are invoke OnSystemDiagnostics

	for (i = 0; i < AllSystems.GetCount(); i++)
		{
		Universe.SetCurrentSystem(AllSystems[i]);

		//	Start diagnostics

		if (i == 0)
			Universe.GetDesignCollection().FireOnGlobalStartDiagnostics();

		//	System diagnostics

		Universe.GetDesignCollection().FireOnGlobalSystemDiagnostics();
		}

	//	Fire OnEndDiagnostics

	Universe.GetDesignCollection().FireOnGlobalEndDiagnostics();
	}
