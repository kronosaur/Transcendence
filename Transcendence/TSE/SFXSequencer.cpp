//	SFXSequencer.cpp
//
//	Effect sequencer

#include "PreComp.h"

CEffectSequencerCreator::~CEffectSequencerCreator (void)

//	CEffectSequencerCreator destructor

	{
	for (int i = 0; i < m_Timeline.GetCount(); i++)
		delete m_Timeline[i].pCreator;
	}

ALERROR CEffectSequencerCreator::CreateEffect (CSystem *pSystem,
											   CSpaceObject *pAnchor,
											   const CVector &vPos,
											   const CVector &vVel,
											   int iRotation,
											   int iVariant,
											   CSpaceObject **retpEffect)

//	CreateEffect
//
//	Creates the effect

	{
	ALERROR error;

	//	Create the effect

	CSequencerEffect *pObj;

	if (error = CSequencerEffect::Create(pSystem,
			this,
			pAnchor,
			vPos,
			vVel,
			&pObj))
		return error;

	//	Play Sound

	PlaySound(pObj);

	//	Done

	if (retpEffect)
		*retpEffect = pObj;

	return NOERROR;
	}

int CEffectSequencerCreator::GetLifetime (void)

//	GetLifetime
//
//	Returns the total lifetime

	{
	int iTotalLifetime = 0;
	for (int i = 0; i < m_Timeline.GetCount(); i++)
		{
		SEntry *pEntry = &m_Timeline[i];

		int iLifetime = pEntry->pCreator->GetLifetime();
		if (iLifetime == -1)
			return -1;

		int iEnd = pEntry->iTime + iLifetime;
		if (iEnd > iTotalLifetime)
			iTotalLifetime = iEnd;
		}

	return iTotalLifetime;
	}

CEffectSequencerCreator::TimeTypes CEffectSequencerCreator::GetTime (int iIndex, int iStart, int iEnd)

//	GetTime
//
//	Returns when the given event should happen: either "past" (already happened),
//	"now" (inside iStart and iEnd) or "future" (after iEnd)

	{
	int iEventTime = m_Timeline[iIndex].iTime;

	if (iEnd == -1)
		iEnd = iStart;

	if (iEventTime < iStart)
		return timePast;
	else if (iEventTime > iEnd)
		return timeFuture;
	else
		return timeNow;
	}

ALERROR CEffectSequencerCreator::OnEffectCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, const CString &sUNID)

//	OnEffectCreateFromXML
//
//	Creates from XML

	{
	ALERROR error;
	int i;

	int iTime = 0;
	for (i = 0; i < pDesc->GetContentElementCount(); i++)
		{
		CXMLElement *pCreatorDesc = pDesc->GetContentElement(i);
		CString sSubUNID = strPatternSubst(CONSTLIT("%s/%d"), sUNID, i);

		//	Load the creator

		SEntry *pEntry = m_Timeline.Insert();
		if (error = CEffectCreator::CreateSimpleFromXML(Ctx, pCreatorDesc, sSubUNID, &pEntry->pCreator))
			return error;

		//	Set the time

		pEntry->iTime = iTime;
		if (pEntry->pCreator)
			iTime += pEntry->pCreator->GetLifetime();
		}

	return NOERROR;
	}

ALERROR CEffectSequencerCreator::OnEffectBindDesign (SDesignLoadCtx &Ctx)

//	OnEffectBindDesign
//
//	Resolve loading

	{
	ALERROR error;

	for (int i = 0; i < m_Timeline.GetCount(); i++)
		if (error = m_Timeline[i].pCreator->BindDesign(Ctx))
			return error;

	return NOERROR;
	}

void CEffectSequencerCreator::OnMarkImages (void)

//	MarkImages
//
//	Mark images used by this effect

	{
	for (int i = 0; i < m_Timeline.GetCount(); i++)
		m_Timeline[i].pCreator->MarkImages();
	}

