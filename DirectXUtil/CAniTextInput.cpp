//	CAniTextInput.cpp
//
//	CAniTextInput class
//	Copyright (c) 2011 by Kronosaur Productions, LLC. All Rights Reserved.

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

const int INDEX_COLOR =						4;
#define PROP_COLOR							CONSTLIT("color")

const int INDEX_OPACITY =					5;
#define PROP_OPACITY						CONSTLIT("opacity")

const int INDEX_TEXT =						6;
#define PROP_TEXT 							CONSTLIT("text")

const int INDEX_FOCUS_BORDER_COLOR =		7;
#define PROP_FOCUS_BORDER_COLOR				CONSTLIT("focusBorderColor")

const int INDEX_ENABLED =					8;
#define PROP_ENABLED						CONSTLIT("enabled")

const int INDEX_FONT =						9;
#define PROP_FONT							CONSTLIT("font")

const int INDEX_LABEL_FONT =				10;
#define PROP_LABEL_FONT						CONSTLIT("labelFont")

const int INDEX_LABEL_TEXT =				11;
#define PROP_LABEL_TEXT						CONSTLIT("labelText")

const int INDEX_LABEL_COLOR =				12;
#define PROP_LABEL_COLOR					CONSTLIT("labelColor")

#define STYLE_FRAME							CONSTLIT("frame")
#define STYLE_FRAME_FOCUS					CONSTLIT("frameFocus")
#define STYLE_FRAME_DISABLED				CONSTLIT("frameDisabled")

const int PADDING_BOTTOM =					2;
const int PADDING_LEFT =					4;
const int PADDING_TOP =						2;

const int CURSOR_BLINK_RATE =				30;
const int CURSOR_WIDTH =					2;

const int FOCUS_BRACKET_THICKNESS =			2;
const int FOCUS_BRACKET_WIDTH =				12;

const int LABEL_PADDING =					2;

CAniTextInput::CAniTextInput (void) :
		m_bFocus(false),
		m_bPassword(false)

//	CAniTextInput constructor

	{
	m_Properties.SetInteger(PROP_VISIBLE, 1);
	m_Properties.SetVector(PROP_POSITION, CVector());
	m_Properties.SetVector(PROP_SCALE, CVector(1.0, 1.0));
	m_Properties.SetInteger(PROP_ROTATION, 0);
	m_Properties.SetColor(PROP_COLOR, 0xffff);
	m_Properties.SetOpacity(PROP_OPACITY, 255);
	m_Properties.SetString(PROP_TEXT, NULL_STR);
	m_Properties.SetColor(PROP_FOCUS_BORDER_COLOR, 0xffff);
	m_Properties.SetBool(PROP_ENABLED, true);
	m_Properties.SetFont(PROP_FONT, NULL);
	m_Properties.SetFont(PROP_LABEL_FONT, NULL);
	m_Properties.SetString(PROP_LABEL_TEXT, NULL_STR);
	m_Properties.SetColor(PROP_LABEL_COLOR, 0xffff);

	IAnimatron *pStyle = new CAniRect;
	pStyle->SetPropertyColor(PROP_COLOR, CG16bitImage::RGBValue(64, 64, 64));
	pStyle->SetPropertyOpacity(PROP_OPACITY, 255);
	SetStyle(styleFrame, pStyle);

	pStyle = new CAniRect;
	pStyle->SetPropertyColor(PROP_COLOR, CG16bitImage::RGBValue(128, 128, 128));
	pStyle->SetPropertyOpacity(PROP_OPACITY, 255);
	SetStyle(styleFrameFocus, pStyle);

	pStyle = new CAniRect;
	pStyle->SetPropertyColor(PROP_COLOR, CG16bitImage::RGBValue(64, 64, 64));
	pStyle->SetPropertyOpacity(PROP_OPACITY, 128);
	SetStyle(styleFrameDisabled, pStyle);
	}

CAniTextInput::~CAniTextInput (void)

//	CAniButton destructor

	{
	}

void CAniTextInput::Create (const RECT &rcRect,
							const CG16bitFont *pFont,
							DWORD dwOptions,
							IAnimatron **retpAni)

//	Create
//
//	Creates text with basic attributes

	{
	//	Create

	CAniTextInput *pText = new CAniTextInput;
	pText->SetPropertyVector(PROP_POSITION, CVector(rcRect.left, rcRect.top));
	pText->SetPropertyFont(PROP_FONT, pFont);

	//	Make sure size is big enough to fit font

	if (pFont)
		{
		int cyMin = pFont->GetHeight() + PADDING_TOP + PADDING_BOTTOM;
		if (cyMin > RectHeight(rcRect))
			pText->SetPropertyVector(PROP_SCALE, CVector(RectWidth(rcRect), cyMin));
		else
			pText->SetPropertyVector(PROP_SCALE, CVector(RectWidth(rcRect), RectHeight(rcRect)));
		}

	//	Options

	if (dwOptions & OPTION_PASSWORD)
		pText->m_bPassword = true;

	*retpAni = pText;
	}

int CAniTextInput::GetDefaultHeight (const CG16bitFont &Font)

//	GetDefaultHeight
//
//	Returns the default height of the input field with the given font

	{
	return Font.GetHeight() + PADDING_TOP + PADDING_BOTTOM;
	}

void CAniTextInput::GetFocusElements (TArray<IAnimatron *> *retList)

//	GetFocusElements
//
//	Adds elements capable of receiving the focus
	
	{
	if (!m_Properties[INDEX_ENABLED].GetBool())
		return;

	retList->Insert(this); 
	}

void CAniTextInput::GetSpacingRect (RECT *retrcRect)

//	GetSpacingRect
//
//	Returns the spacing rect

	{
	CVector vScale = m_Properties[INDEX_SCALE].GetVector();
	const CG16bitFont *pFont = m_Properties[INDEX_FONT].GetFont();
	const CG16bitFont *pLabelFont = m_Properties[INDEX_LABEL_FONT].GetFont();

	retrcRect->left = 0;
	retrcRect->top = 0;
	retrcRect->right = (int)vScale.GetX();
	retrcRect->bottom = (pLabelFont ? pLabelFont->GetHeight() + LABEL_PADDING : 0)
			+ PADDING_TOP
			+ (pFont ? pFont->GetHeight() : 0)
			+ PADDING_BOTTOM;
	}

bool CAniTextInput::HandleChar (char chChar, DWORD dwKeyData)

//	HandleChar
//
//	Handle the character

	{
	if (!m_Properties[INDEX_ENABLED].GetBool())
		return false;

	//	If this is a backspace, delete a character

	else if (chChar == VK_BACK)
		{
		CString sText = m_Properties[INDEX_TEXT].GetString();
		sText = strSubString(sText, 0, sText.GetLength() - 1);
		m_Properties.SetString(PROP_TEXT, sText);

		return true;
		}

	//	If this is a text character, then add it to the input

	else if (strIsAlphaNumeric(&chChar) || chChar == ' ' || strIsASCIISymbol(&chChar))
		{
		CString sText = m_Properties[INDEX_TEXT].GetString();
		sText.Append(CString(&chChar, 1));
		m_Properties.SetString(PROP_TEXT, sText);

		return true;
		}
	else
		return false;
	}

void CAniTextInput::HandleLButtonDblClick (int x, int y, DWORD dwFlags, bool *retbCapture, bool *retbFocus)

//	HandleLButtonDblClick
//
//	LButton double click

	{
	*retbCapture = false;
	*retbFocus = m_Properties[INDEX_ENABLED].GetBool();
	}

void CAniTextInput::HandleLButtonDown (int x, int y, DWORD dwFlags, bool *retbCapture, bool *retbFocus)

//	HandleLButtonDown
//
//	LButton down

	{
	*retbCapture = false;
	*retbFocus = m_Properties[INDEX_ENABLED].GetBool();
	}

IAnimatron *CAniTextInput::HitTest (const CXForm &ToDest, int x, int y)

//	HitTest
//
//	Returns the element if x,y is in our bounds

	{
	const CG16bitFont *pFont = m_Properties[INDEX_FONT].GetFont();
	const CG16bitFont *pLabelFont = m_Properties[INDEX_LABEL_FONT].GetFont();

	//	Position and size

	CVector vPos = ToDest.Transform(m_Properties[INDEX_POSITION].GetVector());
	CVector vPos2 = ToDest.Transform(m_Properties[INDEX_POSITION].GetVector() + m_Properties[INDEX_SCALE].GetVector());
	CVector vSize = vPos2 - vPos;

	//	Figure out the position of the field

	int xField = (int)vPos.GetX();
	int yField = (int)vPos.GetY() + LABEL_PADDING + (pLabelFont ? pLabelFont->GetHeight() : 0);

	//	Now compute the size of the input field frame

	int cxField = (int)vSize.GetX();
	int cyField = (pFont ? pFont->GetHeight() : 0) + PADDING_TOP + PADDING_BOTTOM;

	RECT rcRect;
	rcRect.left = xField;
	rcRect.top = yField;
	rcRect.right = xField + cxField;
	rcRect.bottom = yField + cyField;

	//	See if we're in the rect

	if (x >= rcRect.left && x < rcRect.right && y >= rcRect.top && y < rcRect.bottom)
		return this;
	else
		return NULL;
	}

int CAniTextInput::MapStyleName (const CString &sComponent) const

//	MapStyleName
//
//	Maps from a style name to an index

	{
	if (strEquals(sComponent, STYLE_FRAME))
		return styleFrame;
	else if (strEquals(sComponent, STYLE_FRAME_FOCUS))
		return styleFrameFocus;
	else if (strEquals(sComponent, STYLE_FRAME_DISABLED))
		return styleFrameDisabled;
	else
		return -1;
	}

void CAniTextInput::Paint (SAniPaintCtx &Ctx)

//	Paint
//
//	Paints

	{
	//	Get the font for the input

	const CG16bitFont *pFont = m_Properties[INDEX_FONT].GetFont();
	if (pFont == NULL)
		return;

	//	Get the font for the label

	const CG16bitFont *pLabelFont = m_Properties[INDEX_LABEL_FONT].GetFont();

	//	Enabled?

	bool bEnabled = m_Properties[INDEX_ENABLED].GetBool();

	//	Get position and size

	CVector vPos = m_Properties[INDEX_POSITION].GetVector();
	CVector vSize = m_Properties[INDEX_SCALE].GetVector();
	CVector vPosTrans = Ctx.ToDest.Transform(vPos);
	CVector vPosTrans2 = Ctx.ToDest.Transform(vPos + vSize);
	CVector vSizeTrans = vPosTrans2 - vPosTrans;

	//	The field (including the label starts here)

	int x = (int)vPosTrans.GetX();
	int y = (int)vPosTrans.GetY();

	//	Figure out the position of the label

	int xLabel = x;
	int yLabel = y;

	//	Figure out the position of the input field frame

	int xField = x;
	int yField = yLabel + LABEL_PADDING + (pLabelFont ? pLabelFont->GetHeight() : 0);

	//	Figure out the position of the text within the input field

	int xText = xField + PADDING_LEFT;
	int yText = yField + PADDING_TOP;

	int cxText = 0;
	int cyText = pFont->GetHeight();

	//	Now compute the size of the input field frame

	int cxField = (int)vSizeTrans.GetX();
	int cyField = cyText + PADDING_TOP + PADDING_BOTTOM;

	//	Get color & opacity

	WORD wColor = m_Properties[INDEX_COLOR].GetColor();
	DWORD dwOpacity = m_Properties[INDEX_OPACITY].GetOpacity() * Ctx.dwOpacityToDest / 255;
	if (!bEnabled)
		dwOpacity = dwOpacity / 2;

	//	Paint the label

	CString sLabel = m_Properties[INDEX_LABEL_TEXT].GetString();
	if (pLabelFont && !sLabel.IsBlank())
		{
		pLabelFont->DrawText(Ctx.Dest, 
				xLabel, 
				yLabel,
				m_Properties[INDEX_LABEL_COLOR].GetColor(), 
				dwOpacity, 
				sLabel, 
				0);
		}

	//	Paint the background

	EStyleParts iStyle;
	if (!bEnabled)
		iStyle = styleFrameDisabled;
	else if (m_bFocus)
		iStyle = styleFrameFocus;
	else
		iStyle = styleFrame;

	//	Paint the button shape

	IAnimatron *pFrameStyle = GetStyle(iStyle);
	if (pFrameStyle)
		{
		pFrameStyle->SetPropertyVector(PROP_POSITION, CVector(vPos.GetX(), vPos.GetY() + yField - y));
		pFrameStyle->SetPropertyVector(PROP_SCALE, CVector(cxField, cyField));
		pFrameStyle->Paint(Ctx);
		}

	//	Get the text

	CString sText;
	if (m_bPassword)
		sText = strRepeat(CONSTLIT("•"), m_Properties[INDEX_TEXT].GetString().GetLength());
	else
		sText = m_Properties[INDEX_TEXT].GetString();

	//	Draw

	int xPos;
	pFont->DrawText(Ctx.Dest, 
			xText, 
			yText,
			wColor, 
			dwOpacity, 
			sText, 
			0,
			&xPos);

	//	Draw state if we have the focus

	if (m_bFocus && bEnabled)
		{
		WORD wFocusColor = m_Properties[INDEX_FOCUS_BORDER_COLOR].GetColor();
		Ctx.Dest.Fill(xField, yField, FOCUS_BRACKET_WIDTH, FOCUS_BRACKET_THICKNESS, wFocusColor);
		Ctx.Dest.Fill(xField, yField, FOCUS_BRACKET_THICKNESS, cyField, wFocusColor);
		Ctx.Dest.Fill(xField, yField + cyField - FOCUS_BRACKET_THICKNESS, FOCUS_BRACKET_WIDTH, FOCUS_BRACKET_THICKNESS, wFocusColor);

		Ctx.Dest.Fill(xField + cxField - FOCUS_BRACKET_WIDTH, yField, FOCUS_BRACKET_WIDTH, FOCUS_BRACKET_THICKNESS, wFocusColor);
		Ctx.Dest.Fill(xField + cxField - FOCUS_BRACKET_THICKNESS, yField, FOCUS_BRACKET_THICKNESS, cyField, wFocusColor);
		Ctx.Dest.Fill(xField + cxField - FOCUS_BRACKET_WIDTH, yField + cyField - FOCUS_BRACKET_THICKNESS, FOCUS_BRACKET_WIDTH, FOCUS_BRACKET_THICKNESS, wFocusColor);

#if 0
		DrawRectDotted(Ctx.Dest, 
				xField, 
				yField, 
				cxField, 
				cyField, 
				m_Properties[INDEX_FOCUS_BORDER_COLOR].GetColor());
#endif

		//	Paint cursor

		if ((Ctx.iFrame / CURSOR_BLINK_RATE) % 2)
			Ctx.Dest.Fill(xPos, yText, CURSOR_WIDTH, cyText, wColor);
		}
	}
