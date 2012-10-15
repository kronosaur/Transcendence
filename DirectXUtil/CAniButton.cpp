//	CAniButton.cpp
//
//	CAniButton class
//	Copyright (c) 2011 by Kronosaur Productions, LLC. All Rights Reserved.

#include <windows.h>
#include <ddraw.h>
#include "Alchemy.h"
#include "DirectXUtil.h"
#include <math.h>
#include <stdio.h>

#include "Reanimator.h"

const int INDEX_VISIBLE =						0;
#define PROP_VISIBLE							CONSTLIT("visible")

const int INDEX_POSITION =						1;
#define PROP_POSITION							CONSTLIT("position")

const int INDEX_SCALE =							2;
#define PROP_SCALE								CONSTLIT("scale")

const int INDEX_TEXT =							3;
#define PROP_TEXT								CONSTLIT("text")

const int INDEX_ENABLED =						4;
#define PROP_ENABLED							CONSTLIT("enabled")

const int INDEX_CHECKED =						5;
#define PROP_CHECKED							CONSTLIT("checked")

#define EVENT_ON_CLICK							CONSTLIT("onClick")

#define PROP_COLOR								CONSTLIT("color")
#define PROP_OPACITY							CONSTLIT("opacity")

#define STYLE_DOWN								CONSTLIT("down")
#define STYLE_HOVER								CONSTLIT("hover")
#define STYLE_NORMAL							CONSTLIT("normal")
#define STYLE_TEXT								CONSTLIT("text")
#define STYLE_DISABLED							CONSTLIT("disabled")
#define STYLE_CHECK								CONSTLIT("check")
#define STYLE_IMAGE								CONSTLIT("image")

const int CHECKBOX_SPACING_X =					8;
const int LINK_SPACING_X =						4;

CAniButton::CAniButton (EButtonTypes iType) :
		m_iType(iType),
		m_bDown(false),
		m_bHover(false)

//	CAniButton constructor

	{
	m_Properties.SetInteger(PROP_VISIBLE, 1);
	m_Properties.SetVector(PROP_POSITION, CVector());
	m_Properties.SetVector(PROP_SCALE, CVector(1.0, 1.0));
	m_Properties.SetString(PROP_TEXT, NULL_STR);
	m_Properties.SetBool(PROP_ENABLED, true);
	m_Properties.SetBool(PROP_CHECKED, false);

	IAnimatron *pStyle = new CAniRect;
	pStyle->SetPropertyColor(PROP_COLOR, CG16bitImage::RGBValue(255, 255, 255));
	pStyle->SetPropertyOpacity(PROP_OPACITY, 255);
	SetStyle(styleDown, pStyle);

	pStyle = new CAniRect;
	pStyle->SetPropertyColor(PROP_COLOR, CG16bitImage::RGBValue(255, 255, 255));
	pStyle->SetPropertyOpacity(PROP_OPACITY, 255);
	SetStyle(styleHover, pStyle);

	pStyle = new CAniRect;
	pStyle->SetPropertyColor(PROP_COLOR, CG16bitImage::RGBValue(128, 128, 128));
	pStyle->SetPropertyOpacity(PROP_OPACITY, 255);
	SetStyle(styleNormal, pStyle);

	pStyle = new CAniRect;
	pStyle->SetPropertyColor(PROP_COLOR, CG16bitImage::RGBValue(128, 128, 128));
	pStyle->SetPropertyOpacity(PROP_OPACITY, 128);
	SetStyle(styleDisabled, pStyle);

	pStyle = new CAniText;
	pStyle->SetPropertyColor(PROP_COLOR, CG16bitImage::RGBValue(0, 0, 0));
	pStyle->SetPropertyString(CONSTLIT("textAlignHorz"), CONSTLIT("center"));
	pStyle->SetPropertyString(CONSTLIT("textAlignVert"), CONSTLIT("center"));
	SetStyle(styleText, pStyle);

	pStyle = new CAniText;
	pStyle->SetPropertyColor(PROP_COLOR, CG16bitImage::RGBValue(0, 0, 0));
	pStyle->SetPropertyString(CONSTLIT("textAlignHorz"), CONSTLIT("center"));
	pStyle->SetPropertyString(CONSTLIT("textAlignVert"), CONSTLIT("center"));
	SetStyle(styleCheck, pStyle);
	}

CAniButton::~CAniButton (void)

//	CAniButton destructor

	{
	}

void CAniButton::GetSpacingRect (RECT *retrcRect)

//	GetSpacingRect
//
//	Returns the size of the element

	{
	if (m_iType == typeCheckbox)
		{
		IAnimatron *pStyle = GetStyle(styleNormal);
		IAnimatron *pTextStyle = GetStyle(styleText);
		if (pStyle && pTextStyle)
			{
			RECT rcButton;
			pStyle->GetSpacingRect(&rcButton);

			CVector vSize = m_Properties[INDEX_SCALE].GetVector();
			pTextStyle->SetPropertyVector(PROP_SCALE, CVector(vSize.GetX() - (RectWidth(rcButton) + CHECKBOX_SPACING_X), RectHeight(rcButton)));
			pTextStyle->SetPropertyString(PROP_TEXT, m_Properties[INDEX_TEXT].GetString());

			RECT rcText;
			pTextStyle->GetSpacingRect(&rcText);

			retrcRect->left = 0;
			retrcRect->top = 0;
			retrcRect->right = RectWidth(rcButton) + CHECKBOX_SPACING_X + RectWidth(rcText);
			retrcRect->bottom = Max(RectHeight(rcButton), RectHeight(rcText));
			}
		}
	else
		{
		IAnimatron *pStyle = GetStyle(styleNormal);
		if (pStyle)
			pStyle->GetSpacingRect(retrcRect);
		}
	}

void CAniButton::HandleLButtonDblClick (int x, int y, DWORD dwFlags, bool *retbCapture, bool *retbFocus)

//	HandleLButtonDblClick
//
//	LButton double click

	{
	*retbFocus = false;
	m_bDown = m_Properties[INDEX_ENABLED].GetBool();
	*retbCapture = m_bDown;
	}

void CAniButton::HandleLButtonDown (int x, int y, DWORD dwFlags, bool *retbCapture, bool *retbFocus)

//	HandleLButtonDown
//
//	LButton down

	{
	*retbFocus = false;
	m_bDown = m_Properties[INDEX_ENABLED].GetBool();
	*retbCapture = m_bDown;
	}

void CAniButton::HandleLButtonUp (int x, int y, DWORD dwFlags)

//	HandleLButtonUp
//
//	LButton up

	{
	bool bRaiseEvent = (m_bDown 
			&& m_bHover 
			&& m_Properties[INDEX_ENABLED].GetBool());

	m_bDown = false;

	//	Fire event at the end because it may end up destroying the session

	if (bRaiseEvent)
		{
		if (m_iType == typeCheckbox)
			SetPropertyBool(PROP_CHECKED, !m_Properties[INDEX_CHECKED].GetBool());

		RaiseEvent(EVENT_ON_CLICK);
		}
	}

IAnimatron *CAniButton::HitTest (const CXForm &ToDest, int x, int y)

//	HitTest
//
//	Returns the element if x,y is in our bounds

	{
	//	Position and size

	CVector vPos = ToDest.Transform(m_Properties[INDEX_POSITION].GetVector());
	CVector vPos2 = ToDest.Transform(m_Properties[INDEX_POSITION].GetVector() + m_Properties[INDEX_SCALE].GetVector());
	CVector vSize = vPos2 - vPos;

	RECT rcRect;
	rcRect.left = (int)vPos.GetX();
	rcRect.top = (int)vPos.GetY();
	rcRect.right = (int)(vPos.GetX() + vSize.GetX());
	rcRect.bottom = (int)(vPos.GetY() + vSize.GetY());

	//	See if we're in the rect

	if (x >= rcRect.left && x < rcRect.right && y >= rcRect.top && y < rcRect.bottom)
		return this;
	else
		return NULL;
	}

int CAniButton::MapStyleName (const CString &sComponent) const

//	MapStyleName
//
//	Map from a style name to an index.

	{
	if (strEquals(sComponent, STYLE_DOWN))
		return styleDown;
	else if (strEquals(sComponent, STYLE_HOVER))
		return styleHover;
	else if (strEquals(sComponent, STYLE_NORMAL))
		return styleNormal;
	else if (strEquals(sComponent, STYLE_TEXT))
		return styleText;
	else if (strEquals(sComponent, STYLE_DISABLED))
		return styleDisabled;
	else if (strEquals(sComponent, STYLE_CHECK))
		return styleCheck;
	else if (strEquals(sComponent, STYLE_IMAGE))
		return styleImage;
	else
		return -1;
	}

void CAniButton::Paint (SAniPaintCtx &Ctx)

//	Paint
//
//	Paint the element

	{
	bool bEnabled = m_Properties[INDEX_ENABLED].GetBool();

	//	Get position and size

	CVector vPos = m_Properties[INDEX_POSITION].GetVector();
	CVector vSize = m_Properties[INDEX_SCALE].GetVector();

	//	Transform

	CXForm LocalToDest = CXForm(xformTranslate, vPos) * Ctx.ToDest;

	//	Create a context

	DWORD dwOpacity = (bEnabled ? 255 : 128);
	SAniPaintCtx LocalCtx(Ctx.Dest,
			LocalToDest,
			dwOpacity * Ctx.dwOpacityToDest / 255,
			0);

	//	Figure out how we're going to paint the button shape

	EStyleParts iStyle;
	if (!bEnabled)
		iStyle = styleDisabled;
	else if (m_bHover)
		iStyle = styleHover;
	else if (m_bDown)
		iStyle = styleDown;
	else
		iStyle = styleNormal;

	//	Paint the button shape

	IAnimatron *pFrameStyle = GetStyle(iStyle);
	RECT rcContent;
	if (pFrameStyle)
		{
		pFrameStyle->SetPropertyVector(PROP_POSITION, vPos);

		//	If this is a checkbox then we allow the style to control the size

		if (m_iType == typeCheckbox)
			{
			CVector vCheckbox = pFrameStyle->GetPropertyVector(PROP_SCALE);
			rcContent.left = (int)vPos.GetX();
			rcContent.top = (int)vPos.GetY();
			rcContent.right = rcContent.left + (int)vCheckbox.GetX();
			rcContent.bottom = rcContent.top + (int)vCheckbox.GetY();
			}

		//	Otherwise we set the size based on the control size

		else
			{
			pFrameStyle->SetPropertyVector(PROP_SCALE, vSize);
			pFrameStyle->GetContentRect(&rcContent);
			}

		//	Paint

		pFrameStyle->Paint(Ctx);
		}
	else
		{
		rcContent.left = 0;
		rcContent.right = 30;
		rcContent.top = 0;
		rcContent.bottom = 30;
		}

	//	Set the rect for the text

	IAnimatron *pTextStyle = GetStyle(styleText);
	if (pTextStyle)
		{
		//	Compute the position of the text

		int xText;
		int yText;
		int cxText;
		int cyText;
		if (m_iType == typeCheckbox)
			{
			xText = (int)vPos.GetX() + CHECKBOX_SPACING_X + RectWidth(rcContent);
			yText = (int)vPos.GetY();
			cxText = (int)vSize.GetX() - (CHECKBOX_SPACING_X + RectWidth(rcContent));
			cyText = (int)vSize.GetY();
			}
		else if (m_iType == typeLink)
			{
			xText = (int)vPos.GetX() + LINK_SPACING_X;
			yText = (int)vPos.GetY();
			cxText = (int)vSize.GetX();
			cyText = (int)vSize.GetY();
			}
		else
			{
			xText = (int)vPos.GetX() + rcContent.left;
			yText = (int)vPos.GetY() + rcContent.top;
			cxText = RectWidth(rcContent);
			cyText = RectHeight(rcContent);
			}

		//	Set up the style

		pTextStyle->SetPropertyVector(PROP_POSITION, CVector(xText, yText));
		pTextStyle->SetPropertyVector(PROP_SCALE, CVector(cxText, cyText));
		pTextStyle->SetPropertyOpacity(PROP_OPACITY, dwOpacity);

		//	Paint the text

		pTextStyle->SetPropertyString(PROP_TEXT, m_Properties[INDEX_TEXT].GetString());
		pTextStyle->Paint(Ctx);
		}

	//	Paint the image

	IAnimatron *pImageStyle = GetStyle(styleImage);
	if (pImageStyle)
		pImageStyle->Paint(LocalCtx);

	//	Draw checkmark

	if (m_iType == typeCheckbox)
		{
		bool bChecked = m_Properties[INDEX_CHECKED].GetBool();
		if (bChecked)
			{
			IAnimatron *pCheckStyle = GetStyle(styleCheck);
			if (pCheckStyle)
				{
				pCheckStyle->SetPropertyVector(PROP_POSITION, CVector(rcContent.left, rcContent.top));
				pCheckStyle->SetPropertyVector(PROP_SCALE, CVector(RectWidth(rcContent), RectHeight(rcContent)));
				pCheckStyle->Paint(Ctx);
				}
			}
		}
	}
