//	CIntGraph.cpp
//
//	Implements CIntGraph class
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "Kernel.h"
#include "KernelObjID.h"

#include "Euclid.h"

const int MAX_DIST2 = 2147483647;

CIntGraph::CIntGraph (void) :
		m_iFirstFreeNode(-1),
		m_iFirstFreeConnection(-1),
		m_bNodeIndexValid(false)

//	CIntGraph constructor

	{
	}

void CIntGraph::AddGraph (CIntGraph &Source)

//	AddGraph
//
//	Adds the source graph to this graph

	{
	int i;

	//	Keep a map between source ID and dest ID

	TArray<DWORD> SourceToDest;
	SourceToDest.InsertEmpty(Source.m_Nodes.GetCount());
	for (i = 0; i < Source.m_Nodes.GetCount(); i++)
		{
		SNode *pNode = Source.GetNode(i);
		if (!NodeIsFree(pNode))
			{
			//	Add node to destination

			DWORD dwNewID;
			AddNode(pNode->x, pNode->y, &dwNewID);

			//	Add a mapping

			SourceToDest[i] = dwNewID;
			}
		}

	//	Now add the connections between nodes

	for (i = 0; i < Source.m_Nodes.GetCount(); i++)
		{
		SNode *pNode = Source.GetNode(i);
		if (!NodeIsFree(pNode))
			{
			SConnection *pConnection = Source.GetForwardConnection(pNode);
			while (pConnection)
				{
				Connect(SourceToDest[i], SourceToDest[pConnection->iTo]);

				pConnection = Source.GetNextConnection(pConnection);
				}
			}
		}
	}

void CIntGraph::AddNode (int x, int y, DWORD *retdwID)

//	AddNode
//
//	Adds a new node

	{
	int iNode = AllocNode();
	SNode *pNode = GetNode(iNode);

	pNode->x = x;
	pNode->y = y;

	if (retdwID)
		*retdwID = (DWORD)iNode;
	}

int CIntGraph::AllocConnection (void)

//	AllocConnection
//
//	Allocates a new connection

	{
	int iConnection;
	SConnection *pConnection;

	if (m_iFirstFreeConnection == -1)
		{
		//	If we have no more free connections, allocate a new one

		iConnection = m_Connections.GetCount();
		pConnection = m_Connections.Insert();
		}
	else
		{
		ASSERT(m_iFirstFreeConnection >= 0 && m_iFirstFreeConnection < m_Connections.GetCount());

		iConnection = m_iFirstFreeConnection;
		pConnection = &m_Connections[iConnection];

		m_iFirstFreeConnection = pConnection->iNext;
		}

	pConnection->iNext = -1;

	return iConnection;
	}

int CIntGraph::AllocNode (void)

//	AllocNode
//
//	Allocate a node and return its index. Note: Pointers to nodes may change after this call.

	{
	int iNodeIndex;
	SNode *pNode;

	if (m_iFirstFreeNode == -1)
		{
		//	If we have no more free nodes, allocate a new one and return it

		iNodeIndex = m_Nodes.GetCount();
		pNode = m_Nodes.Insert();
		}

	//	Otherwise, return the free node

	else
		{
		ASSERT(m_iFirstFreeNode >= 0 && m_iFirstFreeNode < m_Nodes.GetCount());

		iNodeIndex = m_iFirstFreeNode;
		pNode = &m_Nodes[iNodeIndex];
		ASSERT(NodeIsFree(pNode));

		m_iFirstFreeNode = GetNextFreeNode(pNode);
		}

	pNode->iFirstBackward = -1;
	pNode->iFirstForward = -1;
	m_bNodeIndexValid = false;

	return iNodeIndex;
	}

void CIntGraph::Connect (DWORD dwFromID, DWORD dwToID)

//	Connect
//
//	Connects the two nodes

	{
	SNode *pFrom = GetNode((int)dwFromID);
	SNode *pTo = GetNode((int)dwToID);

	//	Add the forward connection

	int iConnection = AllocConnection();
	SConnection *pConnection = GetConnection(iConnection);
	pConnection->iFrom = (int)dwFromID;
	pConnection->iTo = (int)dwToID;
	pConnection->iNext = pFrom->iFirstForward;
	pFrom->iFirstForward = iConnection;

	//	Add the backwards connection

	iConnection = AllocConnection();
	pConnection = GetConnection(iConnection);
	pConnection->iFrom = (int)dwToID;
	pConnection->iTo = (int)dwFromID;
	pConnection->iNext = pTo->iFirstBackward;
	pTo->iFirstBackward = iConnection;
	}

void CIntGraph::CreateNodeIndex (void)

//	CreateNodeIndex
//
//	Creates an index of allocated nodes

	{
	int i;

	if (!m_bNodeIndexValid)
		{
		m_NodeIndex.DeleteAll();

		for (i = 0; i < m_Nodes.GetCount(); i++)
			{
			if (!NodeIsFree(&m_Nodes[i]))
				m_NodeIndex.Insert(i);
			}

		m_bNodeIndexValid = true;
		}
	}

void CIntGraph::DeleteAll (void)

//	DeleteAll
//
//	Delete all nodes and connections

	{
	m_Nodes.DeleteAll();
	m_Connections.DeleteAll();
	m_iFirstFreeNode = -1;
	m_iFirstFreeConnection = -1;
	m_bNodeIndexValid = false;
	}

bool CIntGraph::FindNearestNode (int x, int y, DWORD *retdwID)

//	FindNearestNode
//
//	Finds the nearest node

	{
	int i;

	int iBestNode = -1;
	int iBestDist2 = MAX_DIST2;
	for (i = 0; i < m_Nodes.GetCount(); i++)
		{
		SNode *pNode = GetNode(i);
		if (!NodeIsFree(pNode))
			{
			int xDiff = pNode->x - x;
			int yDiff = pNode->y - y;
			int iDist2 = (xDiff * xDiff) + (yDiff * yDiff);
			if (iDist2 < iBestDist2)
				{
				iBestNode = i;
				iBestDist2 = iDist2;
				}
			}
		}

	//	None found

	if (iBestNode == -1)
		return false;

	//	Done

	if (retdwID)
		*retdwID = (DWORD)iBestNode;
	return true;
	}

void CIntGraph::FreeConnection (int iConnection)

//	FreeConnection
//
//	Frees the given connection

	{
	SConnection *pConnection = &m_Connections[iConnection];
	ASSERT(pConnection->iFrom != -1);

	pConnection->iFrom = -1;
	pConnection->iNext = m_iFirstFreeConnection;
	m_iFirstFreeConnection = iConnection;
	}

void CIntGraph::FreeNode (int iNode)

//	FreeNode
//
//	Adds the node to the free list

	{
	SNode *pNode = &m_Nodes[iNode];
	ASSERT(!NodeIsFree(pNode));

	MakeNodeFree(pNode, m_iFirstFreeNode);
	m_iFirstFreeNode = iNode;
	m_bNodeIndexValid = false;
	}

void CIntGraph::GenerateRandomConnections (DWORD dwStartNode, int iMinConnections, int iMaxConnections)

//	GenerateRandomConnections
//
//	Generate random connection across all nodes.

	{
	int i, j;

	//	We start by making sure every node is connected with one other node.
	//	We keep track of the nodes that are connected and those that are not.

	TArray<int> Connected;
	TArray<int> NotConnected;

	//	All the nodes are part of the not-connected group
	//	(except for the start node)

	for (i = 0; i < m_Nodes.GetCount(); i++)
		if (i != dwStartNode && !NodeIsFree(GetNode(i)))
			NotConnected.Insert(i);

	Connected.Insert((int)dwStartNode);

	//	Loop until all nodes are connected

	while (NotConnected.GetCount() > 0)
		{
		//	Look for the shortest, non-overlapping distance
		//	between a node in the connected list and a node in the
		//	not-connected list.

		int iBestDist2 = MAX_DIST2;
		int iBestFrom = -1;
		int iBestTo = -1;

		for (i = 0; i < Connected.GetCount(); i++)
			{
			int iFrom = i;
			SNode *pFrom = GetNode(Connected[iFrom]);

			for (j = 0; j < NotConnected.GetCount(); j++)
				{
				int iTo = j;
				SNode *pTo = GetNode(NotConnected[iTo]);

				int xDist = pTo->x - pFrom->x;
				int yDist = pTo->y - pFrom->y;
				int iDist2 = xDist * xDist + yDist * yDist;
				if (iDist2 < iBestDist2
						&& !IsCrossingConnection(Connected[iFrom], NotConnected[iTo]))
					{
					iBestDist2 = iDist2;
					iBestFrom = iFrom;
					iBestTo = iTo;
					}
				}
			}

		//	If we found a best distance, connect the two nodes

		if (iBestFrom != -1)
			{
			Connect(Connected[iBestFrom], NotConnected[iBestTo]);
			Connected.Insert(NotConnected[iBestTo]);
			NotConnected.Delete(iBestTo);
			}

		//	If we did not find the best distance, then it means that we could not
		//	connect without overlapping. In that case, just connect all the unconnected

		else
			{
			for (i = 0; i < NotConnected.GetCount(); i++)
				Connect(Connected[0], NotConnected[i]);

			NotConnected.DeleteAll();
			}
		}
	}

int CIntGraph::GetNodeCount (void)

//	GetNodeCount
//
//	Returns the number of nodes

	{
	if (m_iFirstFreeNode == -1)
		return m_Nodes.GetCount();
	else
		{
		CreateNodeIndex();
		return m_NodeIndex.GetCount();
		}
	}

int CIntGraph::GetNodeForwardConnections (DWORD dwID, TArray<int> *retConnections)

//	GetNodeForwardConnections
//
//	Fills in a list of forward connections from the given node.

	{
	ASSERT(dwID >= 0 && dwID < (DWORD)m_Nodes.GetCount());

	SNode *pNode = GetNode(dwID);
	SConnection *pConnection = GetForwardConnection(pNode);
	while (pConnection)
		{
		retConnections->Insert(pConnection->iTo);
		pConnection = GetNextConnection(pConnection);
		}

	return retConnections->GetCount();
	}

DWORD CIntGraph::GetNodeID (int iIndex)

//	GetNodeID
//
//	Returns the node ID of the given node index

	{
	if (m_iFirstFreeNode == -1)
		return (DWORD)iIndex;
	else
		{
		CreateNodeIndex();
		return (DWORD)m_NodeIndex[iIndex];
		}
	}

int CIntGraph::GetNodeIndex (DWORD dwID)

//	GetNodeIndex
//
//	Returns the node index give an ID

	{
	int i;

	if (m_iFirstFreeNode == -1)
		return (int)dwID;
	else
		{
		for (i = 0; i < m_NodeIndex.GetCount(); i++)
			if ((DWORD)m_NodeIndex[i] == dwID)
				return i;

		return -1;
		}
	}

void CIntGraph::GetNodePos (DWORD dwID, int *retx, int *rety)

//	GetNodePos
//
//	Get the node position

	{
	ASSERT(dwID >= 0 && dwID < (DWORD)m_Nodes.GetCount());

	*retx = m_Nodes[dwID].x;
	*rety = m_Nodes[dwID].y;
	}

bool CIntGraph::IsCrossingConnection (DWORD dwFromID, DWORD dwToID)

//	IsCrossingConnection
//
//	Returns TRUE if a connection between the two nodes would cross an already
//	existing connection.

	{
	int i;

	//	Get the coordinates of the proposed connection

	SNode *pFrom = GetNode((int)dwFromID);
	SNode *pTo = GetNode((int)dwToID);
	int A1x = pFrom->x;
	int A1y = pFrom->y;
	int A2x = pTo->x;
	int A2y = pTo->y;

	//	Walk all nodes

	for (i = 0; i < m_Nodes.GetCount(); i++)
		{
		SNode *pNode = GetNode(i);
		if (!NodeIsFree(pNode) && pNode != pFrom && pNode != pTo)
			{
			//	Walk all connections for this node
			//	(We only walk forward connections)

			int iConnection = pNode->iFirstForward;
			while (iConnection != -1)
				{
				SConnection *pConnection = GetConnection(iConnection);
				SNode *pFrom2 = GetNode(pConnection->iFrom);
				SNode *pTo2 = GetNode(pConnection->iTo);

				if (pTo2 != pFrom && pTo2 != pTo)
					{
					int B1x = pFrom2->x;
					int B1y = pFrom2->y;
					int B2x = pTo2->x;
					int B2y = pTo2->y;

					if (IntSegmentsIntersect(A1x, A1y, A2x, A2y, B1x, B1y, B2x, B2y))
						return true;
					}

				iConnection = pConnection->iNext;
				}
			}
		}

	return false;
	}
