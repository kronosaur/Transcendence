//	CTopologyNodeList.cpp
//
//	CTopologyNodeList class

#include "PreComp.h"

void CTopologyNodeList::Delete (CTopologyNode *pNode)

//	Delete
//
//	Delete the given node

	{
	int iIndex;
	if (FindNode(pNode, &iIndex))
		m_List.Delete(iIndex);
	}

ALERROR CTopologyNodeList::Filter (CTopologyNode::SCriteriaCtx &Ctx, CXMLElement *pCriteria, CTopologyNodeList *retList, CString *retsError)

//	Filter
//
//	Filters the list based on the criteria and returns a new list

	{
	ALERROR error;

	//	Parse the filter

	CTopologyNode::SCriteria Crit;
	if (error = CTopologyNode::ParseCriteria(pCriteria, &Crit, retsError))
		return error;

	//	Filter

	if (error = Filter(Ctx, Crit, retList))
		{
		*retsError = CONSTLIT("Error filtering nodes.");
		return error;
		}

	return NOERROR;
	}

ALERROR CTopologyNodeList::Filter (CTopologyNode::SCriteriaCtx &Ctx, CTopologyNode::SCriteria &Crit, CTopologyNodeList *ioList)

//	Filter
//
//	Filters the list based on the criteria and returns a new list

	{
	int i;

	//	Loop over all nodes and generate a filtered list

	for (i = 0; i < m_List.GetCount(); i++)
		{
		if (m_List[i]->MatchesCriteria(Ctx, Crit))
			ioList->Insert(m_List[i]);
		}

	return NOERROR;
	}

bool CTopologyNodeList::FindNode (CTopologyNode *pNode, int *retiIndex)

//	FindNode
//
//	Finds the given node

	{
	int i;

	for (i = 0; i < m_List.GetCount(); i++)
		if (pNode == m_List[i])
			{
			if (retiIndex)
				*retiIndex = i;
			return true;
			}

	return false;
	}

bool CTopologyNodeList::FindNode (const CString &sID, int *retiIndex)

//	FindNode
//
//	Finds the given node

	{
	int i;

	for (i = 0; i < m_List.GetCount(); i++)
		if (strEquals(sID, m_List[i]->GetID()))
			{
			if (retiIndex)
				*retiIndex = i;
			return true;
			}

	return false;
	}

bool CTopologyNodeList::IsNodeInRangeOf (CTopologyNode *pNode, int iMin, int iMax, const TArray<CString> &AttribsRequired, const TArray<CString> &AttribsNotAllowed, CTopologyNodeList &Checked)

//	IsNodeInRangeOf
//
//	Returns TRUE if pNode is >= iMin and <= iMax of a node that has AttribsRequired
//	and doesn't have AttribsNotAllowed, and is not in Checked.

	{
	int i;

	//	Add ourselves to the Checked list

	Checked.Insert(pNode);

	//	If iMin is 0, then check the current node

	if (iMin <= 0)
		{
		const CString &sAttribs = pNode->GetAttributes();

		//	Check required attributes

		bool bMatches = true;
		for (i = 0; i < AttribsRequired.GetCount(); i++)
			if (!::HasModifier(sAttribs, AttribsRequired[i]))
				{
				bMatches = false;
				break;
				}

		//	Check disallowed attributes

		if (bMatches)
			{
			for (i = 0; i < AttribsNotAllowed.GetCount(); i++)
				if (::HasModifier(sAttribs, AttribsNotAllowed[i]))
					{
					bMatches = false;
					break;
					}
			}

		//	If we match, then we're OK

		if (bMatches)
			return true;
		}

	//	If any adjacent nodes are in range, then we are also in range

	if (iMax > 0)
		{
		for (i = 0; i < pNode->GetStargateCount(); i++)
			{
			CTopologyNode *pDest = pNode->GetStargateDest(i);

			//	Skip if we've already checked this node

			if (Checked.FindNode(pDest))
				continue;

			//	If this node is in range, then we're done

			if (IsNodeInRangeOf(pDest, iMin - 1, iMax - 1, AttribsRequired, AttribsNotAllowed, Checked))
				return true;
			}
		}

	//	Otherwise, we're not in range

	return false;
	}

void CTopologyNodeList::RestoreMarks (TArray<bool> &Saved)

//	RestoreMarks
//
//	Restore node marks from array

	{
	int i;
	int iCount = Min(Saved.GetCount(), m_List.GetCount());

	for (i = 0; i < iCount; i++)
		m_List[i]->SetMarked(Saved[i]);
	}

void CTopologyNodeList::SaveAndSetMarks (bool bMark, TArray<bool> *retSaved)

//	SaveAndSetMarks
//
//	Saves the marks and sets them all to the new value

	{
	int i;

	retSaved->DeleteAll();
	retSaved->InsertEmpty(m_List.GetCount());

	for (i = 0; i < m_List.GetCount(); i++)
		{
		retSaved->GetAt(i) = m_List[i]->IsMarked();
		m_List[i]->SetMarked(bMark);
		}
	}
