//	CAniVScroller.cpp
//
//	CAniVScroller class

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

const int INDEX_SCALE =						2;
#define PROP_SCALE							CONSTLIT("scale")

const int INDEX_ROTATION =					3;
#define PROP_ROTATION						CONSTLIT("rotation")

const int INDEX_OPACITY =					4;
#define PROP_OPACITY						CONSTLIT("opacity")

const int INDEX_VIEWPORT_HEIGHT =			5;
#define PROP_VIEWPORT_HEIGHT				CONSTLIT("viewportHeight")

const int INDEX_FADE_EDGE_HEIGHT =			6;
#define PROP_FADE_EDGE_HEIGHT				CONSTLIT("fadeEdgeHeight")

const int INDEX_SCROLL_POS =				7;
#define PROP_SCROLL_POS						CONSTLIT("scrollPos")

const int INDEX_MAX_SCROLL_POS =			8;
#define PROP_MAX_SCROLL_POS					CONSTLIT("maxScrollPos")

const int INDEX_PADDING_BOTTOM =			9;
#define PROP_PADDING_BOTTOM					CONSTLIT("paddingBottom")

#define PROP_COLOR							CONSTLIT("color")
#define PROP_FONT							CONSTLIT("font")
#define PROP_TEXT							CONSTLIT("text")

CAniVScroller::CAniVScroller (void) : 
		m_cyEnd(0.0)

//	CAniVScroller constructor
	
	{
	SetPropertyInteger(PROP_VISIBLE, 1);
	SetPropertyVector(PROP_POSITION, CVector());
	SetPropertyVector(PROP_SCALE, CVector(1.0, 1.0));
	SetPropertyInteger(PROP_ROTATION, 0);
	SetPropertyOpacity(PROP_OPACITY, 255);
	SetPropertyMetric(PROP_VIEWPORT_HEIGHT, 100.0);
	SetPropertyMetric(PROP_FADE_EDGE_HEIGHT, 20.0);
	SetPropertyMetric(PROP_SCROLL_POS, 0.0);
	SetPropertyMetric(PROP_MAX_SCROLL_POS, 0.0);
	SetPropertyMetric(PROP_PADDING_BOTTOM, 0.0);
	}

CAniVScroller::~CAniVScroller (void)

//	CAniVScroller destructor

	{
	int i;

	for (i = 0; i < m_List.GetCount(); i++)
		delete m_List[i].pAni;
	}

void CAniVScroller::AddLine (IAnimatron *pAni)

//	AddLine
//
//	Adds a new line to the bottom (and takes ownership of pAni)

	{
	//	Add it

	SLine *pLine = m_List.Insert();
	pLine->pAni = pAni;

	//	Initialize, in case this is being added after the animation has started.

	pAni->GoToStart();

	//	Cache some metrics

	RECT rcRect;
	pAni->GetSpacingRect(&rcRect);
	pLine->cyHeight = RectHeight(rcRect);

	//	Remember the max height of the list

	CVector vPos = pAni->GetPropertyVector(PROP_POSITION);
	Metric yEnd = vPos.GetY() + pLine->cyHeight;
	if (yEnd > m_cyEnd)
		m_cyEnd = yEnd;
	}

void CAniVScroller::AddTextLine (const CString &sText, const CG16bitFont *pFont, WORD wColor, DWORD dwFlags, int cyExtra)

//	AddTextLine
//
//	Adds a new line of text at the bottom

	{
	CAniText *pLine = new CAniText;
	pLine->SetPropertyVector(PROP_POSITION, CVector(0.0, m_cyEnd + cyExtra));
	pLine->SetPropertyString(PROP_TEXT, sText);
	pLine->SetPropertyColor(PROP_COLOR, wColor);
	pLine->SetPropertyFont(PROP_FONT, pFont);
	pLine->SetFontFlags(dwFlags);

	AddLine(pLine);
	}

void CAniVScroller::AnimateLinearScroll (Metric rRate)

//	AnimateLinearScroll
//
//	Sets an animation timeline to scroll through all lines

	{
	CLinearMetric *pScroller = new CLinearMetric;
	Metric cyViewport = m_Properties[INDEX_VIEWPORT_HEIGHT].GetMetric();
	pScroller->SetParams(-cyViewport, m_cyEnd, rRate);

	AnimateProperty(PROP_SCROLL_POS, pScroller);
	}

Metric CAniVScroller::CalcTotalHeight (void)

//	CalcTotalHeight
//
//	Returns the total height of the content

	{
	int i;

	//	Compute

	Metric rTotal = 0.0;
	for (i = 0; i < m_List.GetCount(); i++)
		{
		SLine *pList = &m_List[i];

		RECT rcLine;
		pList->pAni->GetSpacingRect(&rcLine);
		Metric yBottom = pList->pAni->GetPropertyVector(PROP_POSITION).GetY()
				+ (Metric)RectHeight(rcLine);

		if (yBottom > rTotal)
			rTotal = yBottom;
		}

	return rTotal + m_Properties[INDEX_PADDING_BOTTOM].GetMetric();
	}

bool CAniVScroller::DeleteElement (const CString &sID)

//	DeleteElement
//
//	Deletes the given element

	{
	int i;
	bool bDeleted = false;

	//	First see if this is one of our children.

	for (i = 0; i < m_List.GetCount(); i++)
		if (strEquals(sID, m_List[i].pAni->GetID()))
			{
			delete m_List[i].pAni;
			m_List.Delete(i);
			bDeleted = true;
			break;
			}

	//	Otherwise, recurse down.

	if (!bDeleted)
		{
		for (i = 0; i < m_List.GetCount(); i++)
			if (m_List[i].pAni->DeleteElement(sID))
				{
				bDeleted = true;
				break;
				}
		}

	//	If we deleted, then we need to recompute the size

	if (bDeleted)
		m_cyEnd = CalcTotalHeight();

	return bDeleted;
	}

bool CAniVScroller::FindDynamicPropertyMetric (const CString &sName, Metric *retrValue) const

//	FindDynamicPropertyMetric
//
//	Returns a dynamic property

	{
	if (strEquals(sName, PROP_MAX_SCROLL_POS))
		{
		*retrValue = Max((Metric)0.0, (Metric)(m_cyEnd + m_Properties[INDEX_PADDING_BOTTOM].GetMetric() - m_Properties[INDEX_VIEWPORT_HEIGHT].GetMetric()));
		return true;
		}
	else
		return false;
	}

bool CAniVScroller::FindElement (IAnimatron *pAni)

//	FindElement
//
//	Looks for the element with the given ID

	{
	int i;

	if (pAni == this)
		return true;

	for (i = 0; i < m_List.GetCount(); i++)
		if (m_List[i].pAni->FindElement(pAni))
			return true;

	return false;
	}

bool CAniVScroller::FindElement (const CString &sID, IAnimatron **retpAni)

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

	for (i = 0; i < m_List.GetCount(); i++)
		if (m_List[i].pAni->FindElement(sID, retpAni))
			return true;

	return false;
	}

int CAniVScroller::GetDuration (void)

//	GetDuration
//
//	Gets the total duration of the sequence

	{
	int i;
	int iTotalDuration = m_Properties.GetDuration();
	if (iTotalDuration == durationInfinite)
		return iTotalDuration;

	for (i = 0; i < m_List.GetCount(); i++)
		{
		int iDuration = m_List[i].pAni->GetDuration();
		if (iDuration == durationInfinite)
			return durationInfinite;
		else if (iDuration == durationUndefined)
			continue;

		if (iDuration > iTotalDuration)
			iTotalDuration = iDuration;
		}

	return iTotalDuration;
	}

void CAniVScroller::GetFocusElements (TArray<IAnimatron *> *retList)

//	GetFocusElements
//
//	Adds elements that take the focus

	{
	int i;

	for (i = 0; i < m_List.GetCount(); i++)
		m_List[i].pAni->GetFocusElements(retList);
	}

void CAniVScroller::GetSpacingRect (RECT *retrcRect)

//	GetSpacingRect
//
//	Returns the paint size

	{
	int i;

	int cxWidth = 0;
	for (i = 0; i < m_List.GetCount(); i++)
		{
		RECT rcRect;
		m_List[i].pAni->GetSpacingRect(&rcRect);
		int cxLine = RectWidth(rcRect);
		if (cxLine > cxWidth)
			cxWidth = cxLine;
		}

	retrcRect->left = 0;
	retrcRect->top = 0;
	retrcRect->right = cxWidth;
	retrcRect->bottom = (int)m_Properties[INDEX_VIEWPORT_HEIGHT].GetMetric();
	}

void CAniVScroller::GoToFrame (int iFrame)

//	GoToFrame
//
//	Go to the given frame

	{
	int i;

	m_Properties.GoToFrame(iFrame);

	for (i = 0; i < m_List.GetCount(); i++)
		m_List[i].pAni->GoToFrame(iFrame);
	}

void CAniVScroller::GoToNextFrame (SAniUpdateCtx &Ctx, int iFrame)

//	GoToNextFrame
//
//	Advances to the next frame

	{
	int i;

	m_Properties.GoToNextFrame(Ctx, iFrame);

	for (i = 0; i < m_List.GetCount(); i++)
		m_List[i].pAni->GoToNextFrame(Ctx, iFrame);
	}

void CAniVScroller::GoToStart (void)

//	GoToStart
//
//	Restarts the animation from the beginning

	{
	int i;

	m_Properties.GoToStart();

	for (i = 0; i < m_List.GetCount(); i++)
		m_List[i].pAni->GoToStart();
	}

IAnimatron *CAniVScroller::HitTest (const CXForm &ToDest, int x, int y)

//	HitTest
//
//	Hit test

	{
	int i;
	IAnimatron *pHit;

	//	Transform

	Metric yScrollPos = m_Properties[INDEX_SCROLL_POS].GetMetric();
	Metric cyViewport = m_Properties[INDEX_VIEWPORT_HEIGHT].GetMetric();
	CVector vPos = m_Properties[INDEX_POSITION].GetVector();
	CXForm LocalToDest = CXForm(xformTranslate, vPos - CVector(0.0, yScrollPos)) * ToDest;

	//	Loop over all lines

	for (i = 0; i < m_List.GetCount(); i++)
		{
		SLine *pList = &m_List[i];

		//	Are we in the viewport?

		RECT rcLine;
		pList->pAni->GetSpacingRect(&rcLine);
		Metric cyLine = (Metric)RectHeight(rcLine);

		Metric yLinePos = pList->pAni->GetPropertyVector(PROP_POSITION).GetY() - yScrollPos;
		if (yLinePos + cyLine < 0.0 || yLinePos >= cyViewport)
			continue;

		//	Hit test

		if (pHit = pList->pAni->HitTest(LocalToDest, x, y))
			return pHit;
		}

	return NULL;
	}

void CAniVScroller::Paint (SAniPaintCtx &Ctx)

//	Paint
//
//	Paint the lines

	{
	int i;

	//	Basic metrics

	Metric yScrollPos = m_Properties[INDEX_SCROLL_POS].GetMetric();
	Metric cyViewport = m_Properties[INDEX_VIEWPORT_HEIGHT].GetMetric();
	Metric cyFadeEdge = m_Properties[INDEX_FADE_EDGE_HEIGHT].GetMetric();
	DWORD dwBaseOpacity = m_Properties[INDEX_OPACITY].GetOpacity() * Ctx.dwOpacityToDest / 255;

	//	Transform

	CVector vPos = m_Properties[INDEX_POSITION].GetVector();
	CXForm LocalToDest = CXForm(xformTranslate, vPos - CVector(0.0, yScrollPos)) * Ctx.ToDest;

	//	Local paint ctx

	SAniPaintCtx LocalCtx(Ctx.Dest,
			LocalToDest,
			0,
			Ctx.iFrame);

	//	Clip

	RECT rcSavedClip = Ctx.Dest.GetClipRect();

	RECT rcClip = rcSavedClip;
	rcClip.top = (int)vPos.GetY();
	rcClip.bottom = (int)(vPos.GetY() + cyViewport);
	Ctx.Dest.SetClipRect(rcClip);

	//	Loop over all lines

	for (i = 0; i < m_List.GetCount(); i++)
		{
		SLine *pList = &m_List[i];

		//	Are we in the viewport?

		RECT rcLine;
		pList->pAni->GetSpacingRect(&rcLine);
		Metric cyLine = (Metric)RectHeight(rcLine);

		Metric yLinePos = pList->pAni->GetPropertyVector(PROP_POSITION).GetY() - yScrollPos;
		if (yLinePos + cyLine < 0.0 || yLinePos >= cyViewport)
			continue;

		//	Fade edges

		if (cyFadeEdge > 0.0)
			{
			if (yLinePos < 0.0)
				LocalCtx.dwOpacityToDest = 0;
			else if (yLinePos < cyFadeEdge)
				LocalCtx.dwOpacityToDest = (DWORD)((Metric)dwBaseOpacity * (yLinePos / cyFadeEdge));
			else if (yLinePos + cyFadeEdge > cyViewport)
				LocalCtx.dwOpacityToDest = (DWORD)((Metric)dwBaseOpacity * (cyViewport - yLinePos) / cyFadeEdge);
			else
				LocalCtx.dwOpacityToDest = dwBaseOpacity;
			}
		else
			LocalCtx.dwOpacityToDest = dwBaseOpacity;

		//	Paint

		pList->pAni->Paint(LocalCtx);
		}

	Ctx.Dest.SetClipRect(rcSavedClip);
	}

