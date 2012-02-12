//	CRandomPointsProc.cpp
//
//	CRandomPointsProc class

#include "PreComp.h"

#define AREA_TAG							CONSTLIT("Area")
#define CRITERIA_TAG						CONSTLIT("Criteria")
#define LABEL_EFFECT_TAG					CONSTLIT("LabelEffect")
#define MAP_EFFECT_TAG						CONSTLIT("MapEffect")
#define NODES_TAG							CONSTLIT("Nodes")
#define POINT_TAG							CONSTLIT("Point")
#define ROTATION_TAG						CONSTLIT("Rotation")

#define ANGLE_ATTRIB						CONSTLIT("angle")
#define CENTER_ATTRIB						CONSTLIT("center")
#define CHANCE_ATTRIB						CONSTLIT("chance")
#define COUNT_ATTRIB						CONSTLIT("count")
#define LABEL_EFFECT_ATTRIB					CONSTLIT("labelEffect")
#define MAP_EFFECT_ATTRIB					CONSTLIT("mapEffect")
#define MAX_DIST_ATTRIB						CONSTLIT("maxDist")
#define MIN_DIST_ATTRIB						CONSTLIT("minDist")
#define MIN_SEPARATION_ATTRIB				CONSTLIT("minSeparation")
#define TANGENT_ATTRIB						CONSTLIT("tangent")

const int DEFAULT_MIN_SEPARATION = 40;

CRandomPointsProc::~CRandomPointsProc (void)

//	CRandomPointsProc destructor

	{
	if (m_pAreaDef)
		delete m_pAreaDef;
	}

int CRandomPointsProc::GenerateRotation (int x, int y)

//	GenerateRotation
//
//	Generate appropriate rotation based on rotation desc

	{
	switch (m_iRotationType)
		{
		case rotationConstant:
			return m_xRotation;

		case rotationCenter:
			{
			//	Point to center

			int xVec = x - m_xRotation;
			int yVec = y - m_yRotation;

			return IntVectorToPolar(xVec, yVec);
			}

		case rotationTangent:
			{
			//	Point to center

			int xVec = x - m_xRotation;
			int yVec = y - m_yRotation;

			return ((IntVectorToPolar(xVec, yVec) + 90) % 360);
			}

		default:
			return 0;
		}
	}

ALERROR CRandomPointsProc::OnBindDesign (SDesignLoadCtx &Ctx)

//	OnBindDesign
//
//	Bind design

	{
	ALERROR error;
	int i;

	for (i = 0; i < m_PointProcs.GetCount(); i++)
		{
		if (m_PointProcs[i].pLabelEffect)
			if (error = m_PointProcs[i].pLabelEffect->BindDesign(Ctx))
				return error;

		if (m_PointProcs[i].pMapEffect)
			if (error = m_PointProcs[i].pMapEffect->BindDesign(Ctx))
				return error;

		if (m_PointProcs[i].pProc)
			if (error = m_PointProcs[i].pProc->BindDesign(Ctx))
				return error;
		}

	return NOERROR;
	}

CEffectCreator *CRandomPointsProc::OnFindEffectCreator (const CString &sUNID)

//	OnFindEffectCreator
//
//	Finds the effect creator
//
//	{unid}:p{index}/{index}:l
//	{unid}:p{index}/{index}:m
//	{unid}:p{index}/{index}/...
//		           ^

	{
	char *pPos = sUNID.GetASCIIZPointer();

	//	If we've got a slash, then recurse down

	if (*pPos == '/')
		{
		pPos++;

		//	Get the processor index

		int iIndex = ::strParseInt(pPos, -1, &pPos);
		if (iIndex < 0 || iIndex >= m_PointProcs.GetCount())
			return NULL;

		//	If we have a colon, then this one of our effects

		if (*pPos == ':')
			{
			pPos++;

			if (*pPos == 'l')
				return m_PointProcs[iIndex].pLabelEffect;
			else if (*pPos == 'm')
				return m_PointProcs[iIndex].pMapEffect;
			else
				return NULL;
			}

		//	Otherwise let the processor handle it

		else
			return m_PointProcs[iIndex].pProc->FindEffectCreator(CString(pPos));
		}

	//	Otherwise we have no clue

	else
		return NULL;
	}

ALERROR CRandomPointsProc::OnInitFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, const CString &sUNID)

//	OnInitFromXML
//
//	Initialize

	{
	ALERROR error;
	int i;

	//	Basic attributes

	if (error = m_Points.LoadFromXML(pDesc->GetAttribute(COUNT_ATTRIB)))
		{
		Ctx.sError = CONSTLIT("Invalid count in <RandomPoints>");
		return error;
		}

	m_iMinSeparation = pDesc->GetAttributeIntegerBounded(MIN_SEPARATION_ATTRIB, 1, -1, DEFAULT_MIN_SEPARATION);

	//	Initialize

	m_pAreaDef = NULL;
	m_iRotationType = rotationNone;
	m_iTotalChance = 0;
	CTopologyNode::ParseCriteria(NULL, &m_Criteria);

	//	Loop over content elements

	for (i = 0; i < pDesc->GetContentElementCount(); i++)
		{
		CXMLElement *pItem = pDesc->GetContentElement(i);

		if (strEquals(pItem->GetTag(), POINT_TAG))
			{
			CString sNewUNID = strPatternSubst(CONSTLIT("%s/%d"), sUNID, m_PointProcs.GetCount());
			SPointProc *pPointProc = m_PointProcs.Insert();

			//	Basic attributes

			pPointProc->iChance = pItem->GetAttributeIntegerBounded(CHANCE_ATTRIB, 0, -1, 0);
			m_iTotalChance += pPointProc->iChance;

			//	Node procs

			CXMLElement *pNodes = pItem->GetContentElementByTag(NODES_TAG);
			if (pNodes)
				{
				pPointProc->iMinNodeDist = pNodes->GetAttributeIntegerBounded(MIN_DIST_ATTRIB, 0, -1, -1);
				pPointProc->iMaxNodeDist = pNodes->GetAttributeIntegerBounded(MAX_DIST_ATTRIB, 0, -1, -1);

				if (error = ITopologyProcessor::CreateFromXMLAsGroup(Ctx, pNodes, sNewUNID, &pPointProc->pProc))
					return error;
				}
			else
				pPointProc->pProc = NULL;

			//	Label effect

			if (error = pPointProc->pLabelEffect.LoadEffect(Ctx,
					strPatternSubst(CONSTLIT("%s:l"), sNewUNID),
					pItem->GetContentElementByTag(LABEL_EFFECT_TAG),
					pItem->GetAttribute(LABEL_EFFECT_ATTRIB)))
				return error;

			if (error = pPointProc->pMapEffect.LoadEffect(Ctx,
					strPatternSubst(CONSTLIT("%s:m"), sNewUNID),
					pItem->GetContentElementByTag(MAP_EFFECT_TAG),
					pItem->GetAttribute(MAP_EFFECT_ATTRIB)))
				return error;
			}
		else if (strEquals(pItem->GetTag(), AREA_TAG))
			{
			m_pAreaDef = pItem->OrphanCopy();
			}
		else if (strEquals(pItem->GetTag(), ROTATION_TAG))
			{
			CString sValue;

			if (pItem->FindAttribute(ANGLE_ATTRIB, &sValue))
				{
				m_iRotationType = rotationConstant;
				m_xRotation = pItem->GetAttributeIntegerBounded(ANGLE_ATTRIB, 0, 359, 0);
				}
			else if (pItem->FindAttribute(CENTER_ATTRIB, &sValue))
				{
				m_iRotationType = rotationCenter;
				if (error = CTopologyNode::ParsePosition(sValue, &m_xRotation, &m_yRotation))
					{
					Ctx.sError = strPatternSubst(CONSTLIT("Invalid rotation center: %s"), sValue);
					return ERR_FAIL;
					}
				}
			else if (pItem->FindAttribute(TANGENT_ATTRIB, &sValue))
				{
				m_iRotationType = rotationTangent;
				if (error = CTopologyNode::ParsePosition(sValue, &m_xRotation, &m_yRotation))
					{
					Ctx.sError = strPatternSubst(CONSTLIT("Invalid rotation center: %s"), sValue);
					return ERR_FAIL;
					}
				}
			else
				{
				Ctx.sError = CONSTLIT("Invalid <Rotation> specifier");
				return ERR_FAIL;
				}
			}
		else if (strEquals(pItem->GetTag(), CRITERIA_TAG))
			{
			if (error = CTopologyNode::ParseCriteria(pItem, &m_Criteria, &Ctx.sError))
				return error;
			}
		else
			{
			Ctx.sError = strPatternSubst(CONSTLIT("Invalid <RandomPoints> element: %s"), pItem->GetTag());
			return ERR_FAIL;
			}
		}

	return NOERROR;
	}

ALERROR CRandomPointsProc::OnProcess (CSystemMap *pMap, CTopology &Topology, CTopologyNodeList &NodeList, CString *retsError)

//	OnProcess
//
//	Process the topology

	{
	ALERROR error;
	int i, j;

	//	Figure out how many points to generate

	int iCount = m_Points.Roll();
	if (iCount <= 0 || m_iTotalChance <= 0)
		return NOERROR;

	//	Compute an area definition

	CComplexArea ValidArea;
	if (m_pAreaDef)
		{
		if (error = Topology.InitComplexArea(m_pAreaDef, m_iMinSeparation, &ValidArea))
			{
			*retsError = CONSTLIT("Invalid area definition");
			return error;
			}
		}
	else
		{
		//	If we have no area defined, use the entire map

		int cxWidth, cyHeight;
		pMap->GetBackgroundImageSize(&cxWidth, &cyHeight);
		ValidArea.IncludeRect(-cxWidth / 2, cyHeight / 2, cxWidth, cyHeight);
		}

	//	Generate some random points in the valid area

	CIntGraph RegionGraph;
	ValidArea.GeneratePointsInArea(iCount, m_iMinSeparation, &RegionGraph);
	ASSERT(RegionGraph.GetNodeCount() == iCount);

	//	If we have a criteria, the filter the nodes

	CTopologyNodeList FilteredNodeList;
	CTopologyNodeList *pNodeList = FilterNodes(Topology, m_Criteria, NodeList, FilteredNodeList);
	if (pNodeList == NULL)
		{
		*retsError = CONSTLIT("Error filtering nodes");
		return ERR_FAIL;
		}

	//	We mark nodes that are available for being placed
	//	(So we first save the marks)

	TArray<bool> SavedMarks;
	SaveAndMarkNodes(Topology, *pNodeList, &SavedMarks);

	//	Loop over each point

	for (i = 0; i < iCount; i++)
		{
		//	Pick a random processor

		int iProc = 0;
		int iRoll = mathRandom(1, m_iTotalChance);
		while (iRoll > m_PointProcs[iProc].iChance)
			iRoll -= m_PointProcs[iProc++].iChance;

		SPointProc *pPointProc = &m_PointProcs[iProc];

		//	Position

		int xPos, yPos;
		RegionGraph.GetNodePos(RegionGraph.GetNodeID(i), &xPos, &yPos);

		//	Add any effects

		if (pPointProc->pMapEffect)
			pMap->AddAnnotation(pPointProc->pMapEffect, xPos, yPos, GenerateRotation(xPos, yPos));

		//	Process any nodes near the point

		if (pPointProc->pProc)
			{
			//	Compute min and max criteria

			int iMin2 = (pPointProc->iMinNodeDist > 0 ? pPointProc->iMinNodeDist * pPointProc->iMinNodeDist : 0);
			int iMax2 = (pPointProc->iMaxNodeDist > 0 ? pPointProc->iMaxNodeDist * pPointProc->iMaxNodeDist : INT_MAX);

			//	Generate a node list of all the nodes that match the distance
			//	requirements.

			CTopologyNodeList NodesNearPoint;
			for (j = 0; j < pNodeList->GetCount(); j++)
				{
				CTopologyNode *pNode = pNodeList->GetAt(j);
				if (pNode->IsMarked())
					{
					int xNode, yNode;
					pNode->GetDisplayPos(&xNode, &yNode);

					//	Compute distance to point pos

					int xDist = xNode - xPos;
					int yDist = yNode - yPos;
					int iDist2 = (xDist * xDist) + (yDist * yDist);

					//	See if we are in range

					if (iDist2 >= iMin2 && iDist2 <= iMax2)
						{
						//	Add to list

						NodesNearPoint.Insert(pNode);

						//	Unmark so it won't be found again

						pNode->SetMarked(false);
						}
					}
				}

			//	Process all the nodes

			if (NodesNearPoint.GetCount() > 0)
				if (error = pPointProc->pProc->Process(pMap, Topology, NodesNearPoint, retsError))
					return error;
			}
		}

	//	Any nodes that are unmarked should be removed from the list
	//	(because we processed them)

	if (pNodeList == &NodeList)
		{
		for (i = 0; i < pNodeList->GetCount(); i++)
			{
			CTopologyNode *pNode = pNodeList->GetAt(i);
			if (!pNode->IsMarked())
				{
				pNodeList->Delete(i);
				i--;
				}
			}
		}
	else
		{
		for (i = 0; i < pNodeList->GetCount(); i++)
			{
			CTopologyNode *pNode = pNodeList->GetAt(i);
			if (!pNode->IsMarked())
				NodeList.Delete(pNode);
			}
		}

	//	Done

	RestoreMarks(Topology, SavedMarks);

	return NOERROR;
	}
