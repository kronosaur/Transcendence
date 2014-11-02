//	CAniSequencer.cpp
//
//	CAniSequencer class

#include <windows.h>
#include <ddraw.h>
#include "Alchemy.h"
#include "DirectXUtil.h"
#include <math.h>
#include <stdio.h>

#include "Reanimator.h"

const int INDEX_VISIBLE =					0;
#define PROP_VISIBLE						CONSTLIT("visible")

const int INDEX_POSITION =					1;
#define PROP_POSITION						CONSTLIT("position")

const int INDEX_OPACITY =					2;
#define PROP_OPACITY						CONSTLIT("opacity")

CAniSequencer::CAniSequencer (void)

//	CAniSequencer constructor

	{
	m_Properties.SetInteger(PROP_VISIBLE, 1);
	m_Properties.SetVector(PROP_POSITION, CVector());
	m_Properties.SetOpacity(PROP_OPACITY, 255);
	}

CAniSequencer::~CAniSequencer (void)

//	CAniSequencer destructor

	{
	int i;

	for (i = 0; i < m_Timeline.GetCount(); i++)
		delete m_Timeline[i].pAni;
	}

void CAniSequencer::AddTrack (IAnimatron *pAni, int iStartFrame)

//	AddTrack
//
//	Adds a track

	{
	STrack *pTrack = m_Timeline.Insert();

	pAni->GoToStart();

	pTrack->pAni = pAni;
	pTrack->iStartFrame = iStartFrame;
	pTrack->iFrame = (iStartFrame > 0 ? -1 : 0);
	pTrack->iDuration = pAni->GetDuration();
	}

void CAniSequencer::Create (const CVector &vPos, CAniSequencer **retpAni)

//	Create
//
//	Create a new sequencer

	{
	CAniSequencer *pAni = new CAniSequencer;
	pAni->SetPropertyVector(PROP_POSITION, vPos);

	*retpAni = pAni;
	}

bool CAniSequencer::DeleteElement (const CString &sID)

//	DeleteElement
//
//	Deletes the given element

	{
	int i;

	//	First see if this is one of our children.

	for (i = 0; i < m_Timeline.GetCount(); i++)
		if (strEquals(sID, m_Timeline[i].pAni->GetID()))
			{
			delete m_Timeline[i].pAni;
			m_Timeline.Delete(i);
			return true;
			}

	//	Otherwise, recurse down.

	for (i = 0; i < m_Timeline.GetCount(); i++)
		if (m_Timeline[i].pAni->DeleteElement(sID))
			return true;

	return false;
	}

bool CAniSequencer::FindElement (IAnimatron *pAni)

//	FindElement
//
//	Looks for the element with the given ID

	{
	int i;

	if (pAni == this)
		return true;

	for (i = 0; i < m_Timeline.GetCount(); i++)
		if (m_Timeline[i].pAni->FindElement(pAni))
			return true;

	return false;
	}

bool CAniSequencer::FindElement (const CString &sID, IAnimatron **retpAni)

//	FindElement
//
//	Looks for the element with the given ID

	{
	int i;

	if (strEquals(sID, m_sID))
		{
		if (retpAni)
			*retpAni = this;
		return true;
		}

	for (i = 0; i < m_Timeline.GetCount(); i++)
		if (m_Timeline[i].pAni->FindElement(sID, retpAni))
			return true;

	return false;
	}

int CAniSequencer::GetDuration (void)

//	GetDuration
//
//	Gets the total duration of the sequence

	{
	int i;
	int iTotalDuration = m_Properties.GetDuration();
	if (iTotalDuration == durationInfinite)
		return iTotalDuration;

	for (i = 0; i < m_Timeline.GetCount(); i++)
		{
		int iDuration = m_Timeline[i].iDuration;
		if (iDuration == durationInfinite)
			return -1;
		else if (iDuration == durationUndefined)
			continue;

		iDuration += m_Timeline[i].iStartFrame;
		if (iDuration > iTotalDuration)
			iTotalDuration = iDuration;
		}

	return iTotalDuration;
	}

void CAniSequencer::GetFocusElements (TArray<IAnimatron *> *retList)

//	GetFocusElements
//
//	Adds elements that take the focus

	{
	int i;

	for (i = 0; i < m_Timeline.GetCount(); i++)
		m_Timeline[i].pAni->GetFocusElements(retList);
	}

void CAniSequencer::GetSpacingRect (RECT *retrcRect)

//	GetSpacingRect
//
//	Returns the paint size

	{
	int i;

	retrcRect->left = 0;
	retrcRect->right = 0;
	retrcRect->top = 0;
	retrcRect->bottom = 0;

	for (i = 0; i < m_Timeline.GetCount(); i++)
		{
		CVector vPos = m_Timeline[i].pAni->GetPropertyVector(PROP_POSITION);
		int x = (int)vPos.GetX();
		int y = (int)vPos.GetY();

		RECT rcSize;
		m_Timeline[i].pAni->GetSpacingRect(&rcSize);

		if (x + rcSize.left < retrcRect->left)
			retrcRect->left = x + rcSize.left;
		if (x + rcSize.right > retrcRect->right)
			retrcRect->right = x + rcSize.right;
		if (y + rcSize.top < retrcRect->top)
			retrcRect->top = y + rcSize.top;
		if (y + rcSize.bottom > retrcRect->bottom)
			retrcRect->bottom = y + rcSize.bottom;
		}
	}

void CAniSequencer::GoToFrame (int iFrame)

//	GoToFrame
//
//	Go to the given frame

	{
	int i;

	m_Properties.GoToFrame(iFrame);

	for (i = 0; i < m_Timeline.GetCount(); i++)
		{
		STrack *pTrack = &m_Timeline[i];

		//	If this track has not yet started, then it's stopped

		if (pTrack->iStartFrame > iFrame)
			{
			pTrack->pAni->GoToStart();
			pTrack->iFrame = -1;
			}

		//	If this track has already ended, then it's stopped

		else if (pTrack->iDuration >= 0 && iFrame >= pTrack->iStartFrame + pTrack->iDuration)
			{
			pTrack->iFrame = -1;
			}

		//	Otherwise, set at the proper frame

		else
			{
			pTrack->iFrame = iFrame - pTrack->iStartFrame;
			pTrack->pAni->GoToFrame(pTrack->iFrame);
			}
		}
	}

void CAniSequencer::GoToNextFrame (SAniUpdateCtx &Ctx, int iFrame)

//	GoToNextFrame
//
//	Advances to the next frame

	{
	int i;

	m_Properties.GoToNextFrame(Ctx, iFrame);

	for (i = 0; i < m_Timeline.GetCount(); i++)
		{
		STrack *pTrack = &m_Timeline[i];

		//	Deal with running tracks

		if (pTrack->iFrame != -1)
			{
			pTrack->iFrame++;
			pTrack->pAni->GoToNextFrame(Ctx, pTrack->iFrame);

			if (pTrack->iDuration >= 0 && pTrack->iFrame >= pTrack->iDuration)
				pTrack->iFrame = -1;
			}

		//	See if it is time to start a new track

		else if (pTrack->iStartFrame == iFrame)
			{
			pTrack->iFrame = 0;
			}
		}
	}

void CAniSequencer::GoToStart (void)

//	GoToStart
//
//	Restarts the animation from the beginning

	{
	int i;

	m_Properties.GoToStart();

	for (i = 0; i < m_Timeline.GetCount(); i++)
		{
		m_Timeline[i].pAni->GoToStart();
		m_Timeline[i].iDuration = m_Timeline[i].pAni->GetDuration();

		if (m_Timeline[i].iStartFrame == 0)
			m_Timeline[i].iFrame = 0;
		else
			m_Timeline[i].iFrame = -1;
		}
	}

IAnimatron *CAniSequencer::HitTest (const CXForm &ToDest, int x, int y)

//	HitTest
//
//	Hit test

	{
	int i;
	IAnimatron *pHit;

	//	Transform

	CVector vPos = m_Properties[INDEX_POSITION].GetVector();
	CXForm LocalToDest = CXForm(xformTranslate, vPos) * ToDest;

	//	Loop over all tracks

	for (i = 0; i < m_Timeline.GetCount(); i++)
		{
		STrack *pTrack = &m_Timeline[i];

		if (pTrack->iFrame != -1)
			{
			if (pHit = pTrack->pAni->HitTest(LocalToDest, x, y))
				return pHit;
			}
		}

	return NULL;
	}

void CAniSequencer::Paint (SAniPaintCtx &Ctx)

//	Paint
//
//	Paints a frame

	{
	int i;

	//	Transform

	CVector vPos = m_Properties[INDEX_POSITION].GetVector();
	CXForm LocalToDest = CXForm(xformTranslate, vPos) * Ctx.ToDest;

	//	Create a context

	SAniPaintCtx LocalCtx(Ctx.Dest,
			LocalToDest,
			m_Properties[INDEX_OPACITY].GetOpacity() * Ctx.dwOpacityToDest / 255,
			0);

	//	Loop over all tracks

	for (i = 0; i < m_Timeline.GetCount(); i++)
		{
		STrack *pTrack = &m_Timeline[i];

		if (pTrack->iFrame != -1)
			{
			LocalCtx.iFrame = pTrack->iFrame;

			pTrack->pAni->Paint(LocalCtx);
			}
		}
	}