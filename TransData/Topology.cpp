//	Topology.cpp
//
//	Generate statistics about system topology

#include <stdio.h>

#include <windows.h>
#include "Alchemy.h"
#include "XMLUtil.h"
#include "TransData.h"

#define COUNT_SWITCH						CONSTLIT("count")

struct SStargateInfo
	{
	CString sName;
	CString sDest;
	};

struct SNodeInfo
	{
	CString sID;
	CString sName;
	DWORD dwSystemUNID;
	CString sAttribs;
	TArray<SStargateInfo> Stargates;
	};

struct SStat
	{
	SStat (void) : 
			m_iMin(INT_MAX),
			m_iMax(0),
			m_iTotal(0),
			m_iCount(0) { }

	inline int GetAverage (int iCount) { return m_iTotal / iCount; }

	inline void Insert (int iValue)
		{
		if (iValue < m_iMin)
			m_iMin = iValue;
		if (iValue > m_iMax)
			m_iMax = iValue;

		m_iTotal += iValue;
		m_iCount++;
		}

	inline void Print (const CString &sStat, int iCount, int iTotal)
		{
		int iAverage = m_iTotal / iCount;
		int iPercent = (int)((100.0 * (Metric)iAverage / (Metric)iTotal) + 0.5f);

		if (m_iMin == m_iMax)
			printf("%s:\t%d\t%d%%\n", sStat.GetASCIIZPointer(), m_iMin, iPercent);
		else
			printf("%s:\t%d-%d (%d)\t%d%%\n",
					sStat.GetASCIIZPointer(),
					m_iMin, 
					m_iMax, 
					iAverage,
					iPercent);
		}

	inline void SetMin (int iValue) { m_iMin = iValue; }

	int m_iMin;
	int m_iMax;
	int m_iTotal;
	int m_iCount;
	};

struct STopologyStats
	{
	SStat NodeCount;					//	Node count in topology
	TSortMap<CString, SStat> Attribs;	//	Node count for each attribute
	};

void OutputTopologyNode (SNodeInfo *pNode);

void GenerateTopology (CUniverse &Universe, CXMLElement *pCmdLine)
	{
	int i, j, k;

	int iCount = pCmdLine->GetAttributeIntegerBounded(COUNT_SWITCH, 1, -1, 1);

	STopologyStats Stats;

	for (k = 0; k < iCount; k++)
		{
		if (iCount > 1)
			printf("sample %d", k+1);

		TSortMap<CString, SNodeInfo> NodeData;
		TSortMap<CString, int> AttribCount;

		//	Initialize the topology

		CString sError;
		Universe.GetFirstTopologyNode();

		//	Loop over all nodes

		for (i = 0; i < Universe.GetTopologyNodeCount(); i++)
			{
			CTopologyNode *pNode = Universe.GetTopologyNode(i);

			SNodeInfo *pNewNode = NodeData.Insert(pNode->GetID());
			pNewNode->sID = pNode->GetID();
			pNewNode->sName = pNode->GetSystemName();
			pNewNode->dwSystemUNID = pNode->GetSystemTypeUNID();
			pNewNode->sAttribs = pNode->GetAttributes();

			//	Add the attributes in this node to the list of 
			//	attributes for this try.

			TArray<CString> Attribs;
			ParseAttributes(pNewNode->sAttribs, &Attribs);
			for (j = 0; j < Attribs.GetCount(); j++)
				{
				int *pCount = AttribCount.GetAt(Attribs[j]);
				if (pCount == NULL)
					{
					pCount = AttribCount.Insert(Attribs[j]);
					*pCount = 0;
					}

				*pCount = (*pCount) + 1;
				}
			}

		//	Compute topology stats
		//	Add the node count for this try

		Stats.NodeCount.Insert(NodeData.GetCount());

		//	Loop over all attributes that we know about. If one doesn't
		//	exist in this try, then we set its min to 0

		if (k > 0)
			{
			for (i = 0; i < Stats.Attribs.GetCount(); i++)
				{
				if (AttribCount.GetAt(Stats.Attribs.GetKey(i)) == NULL)
					Stats.Attribs[i].SetMin(0);
				}
			}

		//	Loop over all attributes in this try and add them to the stats

		SStat *pAttribStat;
		for (i = 0; i < AttribCount.GetCount(); i++)
			{
			//	If we have some attributes that no other try had, then
			//	that means that the minimum value is 0.

			if (pAttribStat = Stats.Attribs.GetAt(AttribCount.GetKey(i)))
				pAttribStat->Insert(AttribCount[i]);
			else
				{
				pAttribStat = Stats.Attribs.Insert(AttribCount.GetKey(i));
				pAttribStat->Insert(AttribCount[i]);
				if (k > 0)
					pAttribStat->SetMin(0);
				}
			}

		//	Output all the nodes

		if (iCount == 1)
			{
			printf("Node\tSystemType\tName\tStargates\tAttributes\n");
			for (i = 0; i < NodeData.GetCount(); i++)
				{
				SNodeInfo *pNode = &NodeData.GetValue(i);
				printf("%s\t%08x\t%s\t%d\t%s\n",
						pNode->sID.GetASCIIZPointer(),
						pNode->dwSystemUNID,
						pNode->sName.GetASCIIZPointer(),
						pNode->Stargates.GetCount(),
						pNode->sAttribs.GetASCIIZPointer());
				}
			}
		else
			{
			Universe.Reinit();
			printf("\n");
			}
		}

	//	Output stats

	if (iCount > 0)
		{
		printf("\n");
		printf("STATS\n\n");
		Stats.NodeCount.Print(CONSTLIT("Nodes"), iCount, Stats.NodeCount.GetAverage(iCount));
		printf("\n");

		printf("ATTRIBS\n\n");
		for (i = 0; i < Stats.Attribs.GetCount(); i++)
			Stats.Attribs[i].Print(Stats.Attribs.GetKey(i), iCount, Stats.NodeCount.GetAverage(iCount));
		printf("\n");
		}
	}

void OutputTopologyNode (SNodeInfo *pNode)
	{
	}