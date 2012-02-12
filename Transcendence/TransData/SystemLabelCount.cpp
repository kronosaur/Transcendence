//	SystemLabelCount.cpp
//
//	Generate statistics about label frequency

#include <stdio.h>

#include <windows.h>
#include <ddraw.h>
#include "Alchemy.h"
#include "XMLUtil.h"
#include "TransData.h"

void GenerateSystemLabelCount (CUniverse &Universe, CXMLElement *pCmdLine)
	{
	ALERROR error;
	int i, j;
	CString sError;

	enum STypes
		{
		typeSystemLabels,
		typeNodeAttributes,
		typeNodeDebug,
		};

	//	Figure out what kind of output we want

	STypes iType;
	if (pCmdLine->GetAttributeBool(CONSTLIT("nodes")))
		iType = typeNodeAttributes;
	else if (pCmdLine->GetAttributeBool(CONSTLIT("nodeDebug")))
		iType = typeNodeDebug;
	else
		iType = typeSystemLabels;

	//	Samples

	int iSystemSample = pCmdLine->GetAttributeInteger(CONSTLIT("count"));
	if (iSystemSample == 0)
		iSystemSample = 1;

	//	Permutations

	bool bPermutations = pCmdLine->GetAttributeBool(CONSTLIT("permutations"));

	//	Generate systems for multiple games

	CSystemCreateStats Stats;
	for (i = 0; i < iSystemSample; i++)
		{
		printf("pass %d...\n", i+1);

		//	Initialize the universe

		if (error = Universe.InitGame(&sError))
			{
			printf("ERROR: %s", sError.GetASCIIZPointer());
			return;
			}

		//	Loop over all topology nodes

		for (j = 0; j < Universe.GetTopologyNodeCount(); j++)
			{
			CTopologyNode *pNode = Universe.GetTopologyNode(j);

			//	Different stats depending on type

			switch (iType)
				{
				case typeSystemLabels:
					{
					//	Create the system and generate stats

					CSystem *pSystem;
					if (error = Universe.CreateStarSystem(pNode, &pSystem, NULL, &Stats))
						{
						printf("ERROR: Unable to create star system.\n");
						return;
						}

					//	Done

					Universe.DestroySystem(pSystem);
					break;
					}

				case typeNodeAttributes:
					{
					CString sAttribs = pNode->GetAttributes();

					if (!sAttribs.IsBlank())
						Stats.AddLabel(sAttribs);
					else
						Stats.AddLabel(CONSTLIT("[no attributes]"));

					break;
					}

				case typeNodeDebug:
					{
					CString sAttribs = pNode->GetAttributes();
					printf("%s: %s\n", 
							pNode->GetID().GetASCIIZPointer(), 
							pNode->GetAttributes().GetASCIIZPointer());
					break;
					}
				}
			}
		}

	//	Titles

	int iTotalLabels100 = Stats.GetTotalLabelCount() * 100 / iSystemSample;
	int iTotalLabels = iTotalLabels100 / 100;
	int iTotalLabelsDecimal = iTotalLabels100 % 100;

	switch (iType)
		{
		case typeSystemLabels:
			printf("LABEL STATISTICS\n\n");
			printf("Average no. of labels per universe: %d.%02d\n\n", iTotalLabels, iTotalLabelsDecimal);
			break;

		case typeNodeAttributes:
			printf("NODE ATTRIBUTES\n\n");
			printf("Average no. of nodes per universe: %d.%02d\n\n", iTotalLabels, iTotalLabelsDecimal);
			break;
		}

	//	Nodes

	if (iType != typeNodeDebug)
		{
		for (i = 0; i < Stats.GetLabelAttributesCount(); i++)
			{
			CString sAttribs;
			int iCount;

			Stats.GetLabelAttributes(i, &sAttribs, &iCount);

			if (bPermutations || strFind(sAttribs, CONSTLIT(",")) == -1)
				{
				int iCount100 = iCount * 100 / iSystemSample;
				int iPercent100 = iCount * 10000 / Stats.GetTotalLabelCount();

				printf("%s: %d.%02d (%d.%02d%%)\n", 
						sAttribs.GetASCIIZPointer(), 
						iCount100 / 100,
						iCount100 % 100,
						iPercent100 / 100,
						iPercent100 % 100);
				}
			}
		}
	}
