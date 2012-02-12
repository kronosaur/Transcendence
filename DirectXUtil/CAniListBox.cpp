//	CAniListBox.cpp
//
//	CAniListBox class
//	Copyright (c) 2012 by Kronosaur Productions, LLC. All Rights Reserved.

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

const int INDEX_ENABLED =					10;
#define PROP_ENABLED						CONSTLIT("enabled")

const int INDEX_COUNT =						11;
#define PROP_COUNT							CONSTLIT("count")

const int INDEX_SELECTION =					12;
#define PROP_SELECTION						CONSTLIT("selection")

#define PROP_SELECTION_ID					CONSTLIT("selectionID")

#define STYLE_SELECTION						CONSTLIT("selection")
#define STYLE_SELECTION_FOCUS				CONSTLIT("selectionFocus")

#define ID_SCROLL_ANIMATOR					CONSTLIT("idScrollAnimator")

CAniListBox::CAniListBox (void)

//	CAniListBox constructor

	{
	m_Properties.SetInteger(PROP_VISIBLE, 1);
	m_Properties.SetVector(PROP_POSITION, CVector());
	m_Properties.SetVector(PROP_SCALE, CVector(1.0, 1.0));
	m_Properties.SetInteger(PROP_ROTATION, 0);
	m_Properties.SetOpacity(PROP_OPACITY, 255);
	m_Properties.SetMetric(PROP_VIEWPORT_HEIGHT, 100.0);
	m_Properties.SetMetric(PROP_FADE_EDGE_HEIGHT, 20.0);
	m_Properties.SetMetric(PROP_SCROLL_POS, 0.0);
	m_Properties.SetMetric(PROP_MAX_SCROLL_POS, 0.0);
	m_Properties.SetMetric(PROP_PADDING_BOTTOM, 0.0);
	m_Properties.SetBool(PROP_ENABLED, true);
	m_Properties.SetInteger(PROP_COUNT, 0);
	m_Properties.SetInteger(PROP_SELECTION, 0);

	m_pScroller = new CAniVScroller;
	}

CAniListBox::~CAniListBox (void)

//	CAniListBox destructor

	{
	delete m_pScroller;
	}

void CAniListBox::AddEntry (const CString &sID, IAnimatron *pEntry)

//	AddEntry
//
//	Adds an entry to the list

	{
	SEntry *pEntryDesc = m_Entries.Insert();
	pEntryDesc->pAni = pEntry;
	pEntry->GetSpacingRect(&pEntryDesc->rcRect);
	pEntry->SetID(sID);

	m_pScroller->AddLine(pEntry);
	}

bool CAniListBox::DeleteElement (const CString &sID)

//	DeleteElement
//
//	Deletes the element by ID

	{
	int i;

	for (i = 0; i < m_Entries.GetCount(); i++)
		if (strEquals(m_Entries[i].pAni->GetID(), sID))
			{
			m_Entries.Delete(i);
			break;
			}

	return m_pScroller->DeleteElement(sID);
	}

bool CAniListBox::FindDynamicPropertyInteger (const CString &sName, int *retiValue) const

//	FindDynamicPropertyInteger
//
//	Returns a dynamic property

	{
	if (strEquals(sName, PROP_COUNT))
		{
		*retiValue = m_Entries.GetCount();
		return true;
		}

	return false;
	}
	
bool CAniListBox::FindDynamicPropertyMetric (const CString &sName, Metric *retrValue) const

//	FindDynamicPropertyMetric
//
//	Returns a dynamic property

	{
	if (strEquals(sName, PROP_MAX_SCROLL_POS))
		{
		*retrValue = m_pScroller->GetPropertyMetric(sName);
		return true;
		}

	return false;
	}
	
bool CAniListBox::FindDynamicPropertyString (const CString &sName, CString *retsValue) const

//	FindDynamicPropertyString
//
//	Returns a dynamic property

	{
	if (strEquals(sName, PROP_SELECTION_ID))
		{
		int iSelection = m_Properties[INDEX_SELECTION].GetInteger();
		if (iSelection < m_Entries.GetCount() && iSelection >= 0)
			*retsValue = m_Entries[iSelection].pAni->GetID();
		else
			*retsValue = NULL_STR;

		return true;
		}

	return false;
	}
	
void CAniListBox::GetFocusElements (TArray<IAnimatron *> *retList)

//	GetFocusElements
//
//	Returns the focus element

	{
	if (!m_Properties[INDEX_ENABLED].GetBool())
		return;

	retList->Insert(this); 
	}

bool CAniListBox::HandleChar (char chChar, DWORD dwKeyData)

//	HandleChar
//
//	Handle character input

	{
	if (!m_Properties[INDEX_ENABLED].GetBool())
		return false;

	int iSelection = m_Properties[INDEX_SELECTION].GetInteger();
	int iCount = m_Properties[INDEX_COUNT].GetInteger();

	return false;
	}

bool CAniListBox::HandleKeyDown (int iVirtKey, DWORD dwKeyData)

//	HandleKeyDown
//
//	Handle character input

	{
	if (!m_Properties[INDEX_ENABLED].GetBool())
		return false;

	int iSelection = m_Properties[INDEX_SELECTION].GetInteger();

	//	PROP_COUNT is dynamic, so we have to do the full call.
	int iCount = GetPropertyInteger(PROP_COUNT);

	switch (iVirtKey)
		{
		case VK_UP:
			if (iSelection > 0)
				Select(iSelection - 1);
			return true;

		case VK_DOWN:
			if (iSelection + 1 < iCount)
				Select(iSelection + 1);
			return true;

		default:
			return false;
		}
	}

int CAniListBox::MapStyleName (const CString &sComponent) const

//	MapStyleName
//
//	Maps a style name

	{
	if (strEquals(sComponent, STYLE_SELECTION))
		return styleSelection;
	else if (strEquals(sComponent, STYLE_SELECTION_FOCUS))
		return styleSelectionFocus;
	else
		return -1;
	}

void CAniListBox::OnPropertyChanged (const CString &sName)

//	OnPropertyChanged
//
//	Property has changed

	{
	int iIndex = m_Properties.FindProperty(sName);
	if (iIndex == -1)
		return;

	m_pScroller->SetProperty(sName, m_Properties[iIndex]);
	}

void CAniListBox::Paint (SAniPaintCtx &Ctx)

//	Paint
//
//	Paints the control

	{
	CVector vPos = m_Properties[INDEX_POSITION].GetVector();
	CVector vSize = m_Properties[INDEX_SCALE].GetVector();
	bool bEnabled = m_Properties[INDEX_ENABLED].GetBool();
	int cyViewport = (int)m_pScroller->GetPropertyMetric(PROP_VIEWPORT_HEIGHT);
	int cxMinWidth = (int)vSize.GetX();

	//	Paint the contents

	int iSelection = m_Properties[INDEX_SELECTION].GetInteger();
	if (iSelection >= 0 && iSelection < m_Entries.GetCount())
		{
		//	Figure out where the selection should paint

		RECT rcSelection = m_Entries[iSelection].rcRect;
		int cxSel = RectWidth(rcSelection);
		int cySel = RectHeight(rcSelection);
		CVector vEntryPos = m_Entries[iSelection].pAni->GetPropertyVector(PROP_POSITION);

		cxSel = Max(cxSel, cxMinWidth);

		int yOffset = (int)m_pScroller->GetPropertyMetric(PROP_SCROLL_POS);
		int xPadding = 8;
		int yPadding = 8;

		rcSelection.left = (int)vEntryPos.GetX() - xPadding;
		rcSelection.right = rcSelection.left + cxSel + 2 * xPadding;
		rcSelection.top = (int)vEntryPos.GetY() - yOffset - yPadding;
		rcSelection.bottom = rcSelection.top + cySel + 2 * yPadding;

		//	Create a transform for the selection

		CXForm SelToDest = CXForm(xformTranslate, vPos + CVector((Metric)rcSelection.left, (Metric)rcSelection.top)) * Ctx.ToDest;

		//	Create a context

		DWORD dwOpacity = (bEnabled ? 255 : 128);
		SAniPaintCtx SelCtx(Ctx.Dest,
				SelToDest,
				dwOpacity * Ctx.dwOpacityToDest / 255,
				0);

		//	Paint the selection

		IAnimatron *pSelStyle = (m_bFocus ? GetStyle(styleSelectionFocus) : GetStyle(styleSelection));
		if (pSelStyle)
			{
			//	Clip to the viewport

			RECT rcSavedClip = Ctx.Dest.GetClipRect();

			RECT rcClip = rcSavedClip;
			rcClip.top = (int)vPos.GetY();
			rcClip.bottom = (int)(vPos.GetY() + cyViewport);
			Ctx.Dest.SetClipRect(rcClip);

			//	Paint

			pSelStyle->SetPropertyVector(PROP_POSITION, CVector());
			pSelStyle->SetPropertyVector(PROP_SCALE, CVector(RectWidth(rcSelection), RectHeight(rcSelection)));
			pSelStyle->Paint(SelCtx);

			//	Restore

			Ctx.Dest.SetClipRect(rcSavedClip);
			}

		//	Paint all rows

		m_pScroller->Paint(Ctx);
		}
	}

void CAniListBox::Select (int iEntry)

//	Select
//
//	Selects the given entry

	{
	SetPropertyInteger(PROP_SELECTION, iEntry);

	//	Scroll the viewport so that the selection is centered

	int iMaxScrollPos = (int)m_pScroller->GetPropertyMetric(PROP_MAX_SCROLL_POS);
	int cyViewport = (int)m_pScroller->GetPropertyMetric(PROP_VIEWPORT_HEIGHT);
	int iScrollPos = (int)m_pScroller->GetPropertyMetric(PROP_SCROLL_POS);

	//	Compute the new scroll position

	CVector vEntryPos = m_Entries[iEntry].pAni->GetPropertyVector(PROP_POSITION);
	int yEntry = (int)vEntryPos.GetY();
	int cyEntry = RectHeight(m_Entries[iEntry].rcRect);

	int iNewScrollPos = Max(0, Min(yEntry - ((cyViewport - cyEntry) / 2), iMaxScrollPos));

	if (iNewScrollPos != iScrollPos)
		{
		m_pScroller->RemoveAnimation(ID_SCROLL_ANIMATOR);
		m_pScroller->AnimatePropertyLinear(ID_SCROLL_ANIMATOR, PROP_SCROLL_POS, CAniProperty((Metric)iScrollPos), CAniProperty((Metric)iNewScrollPos), 5, true);
		}
	}
