//	CBoundaryMarker.cpp
//
//	CBoundaryMarker class

#include "PreComp.h"

static CObjectClass<CBoundaryMarker>g_BoundaryMarkerClass(OBJID_CBOUNDARYMARKER, NULL);

CBoundaryMarker::CBoundaryMarker (void) : CSpaceObject(&g_BoundaryMarkerClass)

//	CBoundaryMarker constructor

	{
	}

void CBoundaryMarker::AddSegment (const CVector &vEndpoint)

//	AddSegment
//
//	Adds a segment

	{
	m_Path.Insert(vEndpoint);
	}

void CBoundaryMarker::CloseBoundary (void)

//	CloseBoundary
//
//	Closes the path

	{
	}

ALERROR CBoundaryMarker::Create (CSystem *pSystem,
								 const CVector &vStartPos,
								 CBoundaryMarker **retpMarker)

//	Create
//
//	Create a boundary marker

	{
	ALERROR error;
	CBoundaryMarker *pMarker;

	pMarker = new CBoundaryMarker;
	if (pMarker == NULL)
		return ERR_MEMORY;

	pMarker->Place(vStartPos);
	pMarker->SetCannotMove();
	pMarker->SetCannotBeHit();

	//	Initialize

	pMarker->m_Path.Insert(vStartPos);

	//	Add to system

	if (error = pMarker->AddToSystem(pSystem))
		{
		delete pMarker;
		return error;
		}

	//	Done

	if (retpMarker)
		*retpMarker = pMarker;

	return NOERROR;
	}

bool CBoundaryMarker::FindIntersectSegment (const CVector &vStart, const CVector &vEnd, CVector *retvSegInt, CVector *retvSegEnd, int *retiSeg)

//	FindIntersectSegment
//
//	Sees if the given line intersects any of the segments in the boundary.

	{
	int i;

	int iCount = m_Path.GetCount();
	if (iCount < 2)
		return false;

	//	Find the line with the closest intersection

	Metric rNearestIntersect = 1.0;
	int iNearestIntersect = -1;
	CVector vStartD = m_Path[0];
	for (i = 0; i < m_Path.GetCount(); i++)
		{
		CVector vEndD = m_Path[(i + 1) % iCount];

		Metric rPos;
		if (IntersectLine(vStart, vEnd, vStartD, vEndD, NULL, &rPos)
				&& rPos < rNearestIntersect)
			{
			iNearestIntersect = i;
			rNearestIntersect = rPos;
			}
		}

	//	Return

	if (iNearestIntersect != -1)
		{
		if (retvSegInt)
			*retvSegInt = vStart + (rNearestIntersect * (vEnd - vStart));

		if (retvSegEnd)
			*retvSegEnd = m_Path[(iNearestIntersect + 1) % iCount];

		if (retiSeg)
			*retiSeg = iNearestIntersect;

		return true;
		}
	else
		return false;
	}

void CBoundaryMarker::GetSegment (int iSeg, CVector *retvStart, CVector *retvEnd)

//	GetSegment
//
//	Returns the given segment

	{
	int iCount = m_Path.GetCount();
	if (iCount == 0)
		return;

	if (retvStart)
		*retvStart = m_Path[iSeg % iCount];

	if (retvEnd)
		*retvEnd = m_Path[(iSeg + 1) % iCount];
	}

void CBoundaryMarker::OnReadFromStream (SLoadCtx &Ctx)

//	OnReadFromStream
//
//	Read the object's data from a stream
//
//	DWORD		Number of vectors
//	CVector		path

	{
	int i;

	DWORD dwLoad;
	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));

	for (i = 0; i < (int)dwLoad; i++)
		{
		CVector vPos;
		Ctx.pStream->Read((char *)&vPos, sizeof(CVector));
		m_Path.Insert(vPos);
		}
	}

void CBoundaryMarker::OnWriteToStream (IWriteStream *pStream)

//	OnWriteToStream
//
//	Write the object's data to a stream
//
//	DWORD		Number of vectors
//	CVector		path

	{
	int i;

	DWORD dwSave = m_Path.GetCount();
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	for (i = 0; i < m_Path.GetCount(); i++)
		pStream->Write((char *)&m_Path[i], sizeof(CVector));
	}

