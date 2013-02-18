//	Dialogs.cpp
//
//	Dialogs
//	Copyright (c) 2012 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

const int BUTTON_CORNER_RADIUS =				8;
const int BUTTON_BORDER_WIDTH =					3;

const int CHECKBOX_BORDER_WIDTH =				1;
const int CHECKBOX_HEIGHT =						18;
const int CHECKBOX_LINE_PADDING =				3;
const int CHECKBOX_WIDTH =						18;

const int DLG_APPEAR_TIME =						5;
const int DLG_BORDER_WIDTH =					3;
const int DLG_CORNER_RADIUS =					20;
const int DLG_PADDING_BOTTOM =					20;
const int DLG_PADDING_LEFT =					20;
const int DLG_PADDING_RIGHT =					20;
const int DLG_PADDING_TOP =						20;
const int DLG_TITLE_HEIGHT =					42;

const int IMAGE_BUTTON_BORDER_WIDTH =			1;
const int IMAGE_BUTTON_IMAGE_PADDING_TOP =		4;
const int IMAGE_BUTTON_HEIGHT =					96;
const int IMAGE_BUTTON_WIDTH =					96;

const int LINK_HEIGHT =							20;
const int LINK_SPACING_X =						4;
const int LINK_CORNER_RADIUS =					3;

const int MSG_BORDER_WIDTH =					1;
const int MSG_CORNER_RADIUS =					20;
const int MSG_PADDING_LEFT =					20;
const int MSG_PADDING_RIGHT =					20;
const int MSG_PADDING_TOP =						20;
const int MSG_PANE_HEIGHT =						200;
const int MSG_PANE_WIDTH =						400;

const int RING_SPACING =						2;
const int RING_SIZE =							10;
const int RING_SIZE_INC =						2;
const int RING_MIN_RATE =						2;
const int RING_MAX_RATE =						6;
const int RING_MIN_RADIUS =						20;
const int RING_COUNT =							4;

const int SMALL_BUTTON_CORNER_RADIUS =			4;

const int SMALL_IMAGE_BUTTON_BORDER_WIDTH =		1;
const int SMALL_IMAGE_BUTTON_HEIGHT =			48;
const int SMALL_IMAGE_BUTTON_WIDTH =			48;

#define PROP_COLOR								CONSTLIT("color")
#define PROP_FILL_TYPE							CONSTLIT("fillType")
#define PROP_FONT								CONSTLIT("font")
#define PROP_LINE_COLOR							CONSTLIT("lineColor")
#define PROP_LINE_PADDING						CONSTLIT("linePadding")
#define PROP_LINE_TYPE							CONSTLIT("lineType")
#define PROP_LINE_WIDTH							CONSTLIT("lineWidth")
#define PROP_LL_RADIUS							CONSTLIT("llRadius")
#define PROP_LR_RADIUS							CONSTLIT("lrRadius")
#define PROP_OPACITY							CONSTLIT("opacity")
#define PROP_POSITION							CONSTLIT("position")
#define PROP_SCALE								CONSTLIT("scale")
#define PROP_TEXT								CONSTLIT("text")
#define PROP_TEXT_ALIGN_HORZ					CONSTLIT("textAlignHorz")
#define PROP_TEXT_ALIGN_VERT					CONSTLIT("textAlignVert")
#define PROP_UL_RADIUS							CONSTLIT("ulRadius")
#define PROP_UR_RADIUS							CONSTLIT("urRadius")

#define ALIGN_CENTER							CONSTLIT("center")

#define STYLE_CHECK								CONSTLIT("check")
#define STYLE_DISABLED							CONSTLIT("disabled")
#define STYLE_DOWN								CONSTLIT("down")
#define STYLE_FRAME								CONSTLIT("frame")
#define STYLE_FRAME_FOCUS						CONSTLIT("frameFocus")
#define STYLE_FRAME_DISABLED					CONSTLIT("frameDisabled")
#define STYLE_HOVER								CONSTLIT("hover")
#define STYLE_IMAGE								CONSTLIT("image")
#define STYLE_NORMAL							CONSTLIT("normal")
#define STYLE_TEXT								CONSTLIT("text")

#define FILL_TYPE_NONE							CONSTLIT("none")
#define LINE_TYPE_SOLID							CONSTLIT("solid")

void CVisualPalette::CreateButton (CAniSequencer *pContainer,
								   const CString &sID,
								   int x,
								   int y,
								   int cxWidth,
								   int cyHeight,
								   DWORD dwOptions,
								   const CString &sLabel,
								   IAnimatron **retpControl) const

//	CreateButton
//
//	Creates a button control

	{
	bool bDefaultButton = ((dwOptions & OPTION_BUTTON_DEFAULT) ? true : false);

	CAniButton *pButton = new CAniButton;
	pButton->SetID(sID);
	pButton->SetPropertyVector(CONSTLIT("position"), CVector(x, y));
	pButton->SetPropertyVector(CONSTLIT("scale"), CVector(cxWidth, cyHeight));
	pButton->SetPropertyString(CONSTLIT("text"), sLabel);

	//	Set styles

	IAnimatron *pStyle = new CAniRoundedRect;
	pStyle->SetPropertyColor(PROP_COLOR, GetColor(colorAreaDialogTitle));
	pStyle->SetPropertyOpacity(PROP_OPACITY, 255);
	pStyle->SetPropertyString(PROP_LINE_TYPE, LINE_TYPE_SOLID);
	pStyle->SetPropertyColor(PROP_LINE_COLOR, GetColor(colorLineDialogFrame));
	pStyle->SetPropertyInteger(PROP_LINE_WIDTH, BUTTON_BORDER_WIDTH);
	pStyle->SetPropertyInteger(PROP_UL_RADIUS, BUTTON_CORNER_RADIUS);
	pStyle->SetPropertyInteger(PROP_UR_RADIUS, BUTTON_CORNER_RADIUS);
	pStyle->SetPropertyInteger(PROP_LL_RADIUS, BUTTON_CORNER_RADIUS);
	pStyle->SetPropertyInteger(PROP_LR_RADIUS, BUTTON_CORNER_RADIUS);
	pButton->SetStyle(STYLE_DOWN, pStyle);

	pStyle = new CAniRoundedRect;
	pStyle->SetPropertyColor(PROP_COLOR, GetColor(colorAreaDialogTitle));
	pStyle->SetPropertyOpacity(PROP_OPACITY, 255);
	pStyle->SetPropertyString(PROP_LINE_TYPE, LINE_TYPE_SOLID);
	pStyle->SetPropertyColor(PROP_LINE_COLOR, GetColor(colorLineDialogFrame));
	pStyle->SetPropertyInteger(PROP_LINE_WIDTH, BUTTON_BORDER_WIDTH);
	pStyle->SetPropertyInteger(PROP_UL_RADIUS, BUTTON_CORNER_RADIUS);
	pStyle->SetPropertyInteger(PROP_UR_RADIUS, BUTTON_CORNER_RADIUS);
	pStyle->SetPropertyInteger(PROP_LL_RADIUS, BUTTON_CORNER_RADIUS);
	pStyle->SetPropertyInteger(PROP_LR_RADIUS, BUTTON_CORNER_RADIUS);
	pButton->SetStyle(STYLE_HOVER, pStyle);

	pStyle = new CAniRoundedRect;
	if (bDefaultButton)
		pStyle->SetPropertyColor(PROP_COLOR, GetColor(colorAreaDialogTitle));
	else
		pStyle->SetPropertyColor(PROP_COLOR, GetColor(colorAreaDialogInput));
	pStyle->SetPropertyOpacity(PROP_OPACITY, 255);
	pStyle->SetPropertyString(PROP_LINE_TYPE, LINE_TYPE_SOLID);
	pStyle->SetPropertyColor(PROP_LINE_COLOR, GetColor(colorTextDialogLabel));
	pStyle->SetPropertyInteger(PROP_LINE_WIDTH, BUTTON_BORDER_WIDTH);
	pStyle->SetPropertyInteger(PROP_UL_RADIUS, BUTTON_CORNER_RADIUS);
	pStyle->SetPropertyInteger(PROP_UR_RADIUS, BUTTON_CORNER_RADIUS);
	pStyle->SetPropertyInteger(PROP_LL_RADIUS, BUTTON_CORNER_RADIUS);
	pStyle->SetPropertyInteger(PROP_LR_RADIUS, BUTTON_CORNER_RADIUS);
	pButton->SetStyle(STYLE_NORMAL, pStyle);

	pStyle = new CAniRoundedRect;
	pStyle->SetPropertyColor(PROP_COLOR, GetColor(colorAreaDialogInput));
	pStyle->SetPropertyOpacity(PROP_OPACITY, 128);
	pStyle->SetPropertyString(PROP_LINE_TYPE, LINE_TYPE_SOLID);
	pStyle->SetPropertyColor(PROP_LINE_COLOR, GetColor(colorTextDialogLabel));
	pStyle->SetPropertyInteger(PROP_LINE_WIDTH, BUTTON_BORDER_WIDTH);
	pStyle->SetPropertyInteger(PROP_UL_RADIUS, BUTTON_CORNER_RADIUS);
	pStyle->SetPropertyInteger(PROP_UR_RADIUS, BUTTON_CORNER_RADIUS);
	pStyle->SetPropertyInteger(PROP_LL_RADIUS, BUTTON_CORNER_RADIUS);
	pStyle->SetPropertyInteger(PROP_LR_RADIUS, BUTTON_CORNER_RADIUS);
	pButton->SetStyle(STYLE_DISABLED, pStyle);

	pStyle = new CAniText;
	pStyle->SetPropertyColor(PROP_COLOR, GetColor(colorTextDialogInput));
	if (bDefaultButton)
		pStyle->SetPropertyFont(PROP_FONT, &GetFont(fontLargeBold));
	else
		pStyle->SetPropertyFont(PROP_FONT, &GetFont(fontLarge));
	pStyle->SetPropertyString(PROP_TEXT_ALIGN_HORZ, CONSTLIT("center"));
	pStyle->SetPropertyString(PROP_TEXT_ALIGN_VERT, CONSTLIT("center"));
	pButton->SetStyle(STYLE_TEXT, pStyle);

	//	Add

	if (pContainer)
		pContainer->AddTrack(pButton, 0);

	//	Done

	if (retpControl)
		*retpControl = pButton;
	}

void CVisualPalette::CreateCheckbox (CAniSequencer *pContainer,
									 const CString &sID,
									 int x,
									 int y,
									 int cxWidth,
									 DWORD dwOptions,
									 const CString &sLabel,
									 IAnimatron **retpControl,
									 int *retcyHeight) const

//	CreateCheckbox
//
//	Creates a checkbox control

	{
	const CG16bitFont *pLabelFont;
	WORD wLabelColor;
	if (dwOptions & OPTION_CHECKBOX_LARGE_FONT)
		{
		pLabelFont = &GetFont(fontLarge);
		wLabelColor = GetColor(colorTextDialogInput);
		}
	else
		{
		pLabelFont = &GetFont(fontMedium);
		wLabelColor = GetColor(colorTextDialogLabel);
		}

	CAniButton *pButton = new CAniButton(CAniButton::typeCheckbox);
	pButton->SetID(sID);
	pButton->SetPropertyVector(CONSTLIT("position"), CVector(x, y));
	pButton->SetPropertyVector(CONSTLIT("scale"), CVector(cxWidth, CHECKBOX_HEIGHT));
	pButton->SetPropertyString(CONSTLIT("text"), sLabel);

	//	Set styles

	IAnimatron *pStyle = new CAniRect;
	pStyle->SetPropertyVector(PROP_SCALE, CVector(CHECKBOX_WIDTH, CHECKBOX_HEIGHT));
	pStyle->SetPropertyColor(PROP_COLOR, GetColor(colorAreaDialogTitle));
	pStyle->SetPropertyOpacity(PROP_OPACITY, 255);
	pStyle->SetPropertyString(PROP_LINE_TYPE, LINE_TYPE_SOLID);
	pStyle->SetPropertyColor(PROP_LINE_COLOR, GetColor(colorTextDialogLabel));
	pStyle->SetPropertyInteger(PROP_LINE_PADDING, CHECKBOX_LINE_PADDING);
	pStyle->SetPropertyInteger(PROP_LINE_WIDTH, CHECKBOX_BORDER_WIDTH);
	pButton->SetStyle(STYLE_DOWN, pStyle);

	pStyle = new CAniRect;
	pStyle->SetPropertyVector(PROP_SCALE, CVector(CHECKBOX_WIDTH, CHECKBOX_HEIGHT));
	pStyle->SetPropertyColor(PROP_COLOR, GetColor(colorAreaDialogTitle));
	pStyle->SetPropertyOpacity(PROP_OPACITY, 255);
	pStyle->SetPropertyString(PROP_LINE_TYPE, LINE_TYPE_SOLID);
	pStyle->SetPropertyColor(PROP_LINE_COLOR, GetColor(colorTextDialogLabel));
	pStyle->SetPropertyInteger(PROP_LINE_PADDING, CHECKBOX_LINE_PADDING);
	pStyle->SetPropertyInteger(PROP_LINE_WIDTH, CHECKBOX_BORDER_WIDTH);
	pButton->SetStyle(STYLE_HOVER, pStyle);

	pStyle = new CAniRect;
	pStyle->SetPropertyVector(PROP_SCALE, CVector(CHECKBOX_WIDTH, CHECKBOX_HEIGHT));
	pStyle->SetPropertyColor(PROP_COLOR, GetColor(colorAreaDialogInput));
	pStyle->SetPropertyOpacity(PROP_OPACITY, 255);
	pStyle->SetPropertyString(PROP_LINE_TYPE, LINE_TYPE_SOLID);
	pStyle->SetPropertyColor(PROP_LINE_COLOR, GetColor(colorTextDialogLabel));
	pStyle->SetPropertyInteger(PROP_LINE_PADDING, CHECKBOX_LINE_PADDING);
	pStyle->SetPropertyInteger(PROP_LINE_WIDTH, CHECKBOX_BORDER_WIDTH);
	pButton->SetStyle(STYLE_NORMAL, pStyle);

	pStyle = new CAniRect;
	pStyle->SetPropertyVector(PROP_SCALE, CVector(CHECKBOX_WIDTH, CHECKBOX_HEIGHT));
	pStyle->SetPropertyColor(PROP_COLOR, GetColor(colorAreaDialogInput));
	pStyle->SetPropertyOpacity(PROP_OPACITY, 128);
	pStyle->SetPropertyString(PROP_LINE_TYPE, LINE_TYPE_SOLID);
	pStyle->SetPropertyColor(PROP_LINE_COLOR, GetColor(colorTextDialogLabel));
	pStyle->SetPropertyInteger(PROP_LINE_PADDING, CHECKBOX_LINE_PADDING);
	pStyle->SetPropertyInteger(PROP_LINE_WIDTH, CHECKBOX_BORDER_WIDTH);
	pButton->SetStyle(STYLE_DISABLED, pStyle);

	pStyle = new CAniText;
	pStyle->SetPropertyColor(PROP_COLOR, wLabelColor);
	pStyle->SetPropertyFont(PROP_FONT, pLabelFont);
	pStyle->SetPropertyString(PROP_TEXT_ALIGN_VERT, CONSTLIT("center"));
	pButton->SetStyle(STYLE_TEXT, pStyle);

	pStyle = new CAniText;
	pStyle->SetPropertyColor(PROP_COLOR, GetColor(colorTextDialogInput));
	pStyle->SetPropertyFont(PROP_FONT, &GetFont(fontHeaderBold));
	pStyle->SetPropertyString(PROP_TEXT, CONSTLIT("×"));
	pStyle->SetPropertyString(PROP_TEXT_ALIGN_HORZ, CONSTLIT("center"));
	pStyle->SetPropertyString(PROP_TEXT_ALIGN_VERT, CONSTLIT("center"));
	pButton->SetStyle(STYLE_CHECK, pStyle);

	//	Get the height of the button

	int cyHeight = pButton->GetSpacingHeight();
	if (cyHeight > CHECKBOX_HEIGHT)
		pButton->SetPropertyVector(CONSTLIT("position"), CVector(x, y + ((cyHeight - CHECKBOX_HEIGHT) / 2)));

	//	Add

	if (pContainer)
		pContainer->AddTrack(pButton, 0);

	//	Height

	if (retcyHeight)
		*retcyHeight = cyHeight;

	//	Done

	if (retpControl)
		*retpControl = pButton;
	}

void CVisualPalette::CreateEditControl (CAniSequencer *pContainer, 
										const CString &sID, 
										int x, 
										int y,
										int cxWidth,
										DWORD dwOptions,
										const CString &sLabel, 
										IAnimatron **retpControl, 
										int *retcyHeight) const

//	CreateEditControl
//
//	Creates an edit control

	{
	RECT rcRect;
	rcRect.left = x;
	rcRect.top = y;
	rcRect.right = x + cxWidth;
	rcRect.bottom = y + 100;

	IAnimatron *pInput;
	CAniTextInput::Create(rcRect,
			&GetFont(fontHeader),
			((dwOptions & OPTION_EDIT_PASSWORD) ? CAniTextInput::OPTION_PASSWORD : 0),
			&pInput);
	pInput->SetID(sID);
	pInput->SetPropertyColor(CONSTLIT("color"), GetColor(colorTextDialogInput));
	pInput->SetPropertyColor(CONSTLIT("focusBorderColor"), GetColor(colorAreaDialogHighlight));
	pInput->SetPropertyColor(CONSTLIT("labelColor"), GetColor(colorTextDialogLabel));
	pInput->SetPropertyFont(CONSTLIT("labelFont"), &GetFont(fontMedium));
	pInput->SetPropertyString(CONSTLIT("labelText"), sLabel);

	//	Set the styles

	IAnimatron *pStyle = new CAniRect;
	pStyle->SetPropertyColor(PROP_COLOR, GetColor(colorAreaDialogInput));
	pStyle->SetPropertyString(PROP_LINE_TYPE, LINE_TYPE_SOLID);
	pStyle->SetPropertyColor(PROP_LINE_COLOR, GetColor(colorTextDialogLabel));
	pStyle->SetPropertyInteger(PROP_LINE_WIDTH, 1);
	pStyle->SetPropertyOpacity(PROP_OPACITY, 255);
	pInput->SetStyle(STYLE_FRAME, pStyle);

	pStyle = new CAniRect;
	pStyle->SetPropertyColor(PROP_COLOR, GetColor(colorAreaDialogInputFocus));
	pStyle->SetPropertyString(PROP_LINE_TYPE, LINE_TYPE_SOLID);
	pStyle->SetPropertyColor(PROP_LINE_COLOR, GetColor(colorTextDialogLabel));
	pStyle->SetPropertyInteger(PROP_LINE_WIDTH, 1);
	pStyle->SetPropertyOpacity(PROP_OPACITY, 255);
	pInput->SetStyle(STYLE_FRAME_FOCUS, pStyle);

	pStyle = new CAniRect;
	pStyle->SetPropertyColor(PROP_COLOR, GetColor(colorAreaDialogInput));
	pStyle->SetPropertyString(PROP_LINE_TYPE, LINE_TYPE_SOLID);
	pStyle->SetPropertyColor(PROP_LINE_COLOR, GetColor(colorTextDialogLabel));
	pStyle->SetPropertyInteger(PROP_LINE_WIDTH, 1);
	pStyle->SetPropertyOpacity(PROP_OPACITY, 128);
	pInput->SetStyle(STYLE_FRAME_DISABLED, pStyle);

	//	Add

	if (pContainer)
		pContainer->AddTrack(pInput, 0);

	//	Compute height

	if (retcyHeight)
		{
		pInput->GetSpacingRect(&rcRect);
		*retcyHeight = RectHeight(rcRect);
		}

	//	Done

	if (retpControl)
		*retpControl = pInput;
	}

void CVisualPalette::CreateHiddenButton (CAniSequencer *pContainer,
										 const CString &sID,
										 int x,
										 int y,
										 int cxWidth,
										 int cyHeight,
										 DWORD dwOptions,
										 IAnimatron **retpControl) const

//	CreateHiddenButton
//
//	Creates a button control

	{
	CAniButton *pButton = new CAniButton;
	pButton->SetID(sID);
	pButton->SetPropertyVector(CONSTLIT("position"), CVector(x, y));
	pButton->SetPropertyVector(CONSTLIT("scale"), CVector(cxWidth, cyHeight));

	//	Set styles

	IAnimatron *pStyle = new CAniRoundedRect;
	pStyle->SetPropertyColor(PROP_COLOR, GetColor(colorAreaDialogTitle));
	pStyle->SetPropertyOpacity(PROP_OPACITY, 255);
	pStyle->SetPropertyInteger(PROP_UL_RADIUS, BUTTON_CORNER_RADIUS);
	pStyle->SetPropertyInteger(PROP_UR_RADIUS, BUTTON_CORNER_RADIUS);
	pStyle->SetPropertyInteger(PROP_LL_RADIUS, BUTTON_CORNER_RADIUS);
	pStyle->SetPropertyInteger(PROP_LR_RADIUS, BUTTON_CORNER_RADIUS);
	pButton->SetStyle(STYLE_DOWN, pStyle);

	pStyle = new CAniRoundedRect;
	pStyle->SetPropertyColor(PROP_COLOR, GetColor(colorAreaDialogTitle));
	pStyle->SetPropertyOpacity(PROP_OPACITY, 255);
	pStyle->SetPropertyInteger(PROP_UL_RADIUS, BUTTON_CORNER_RADIUS);
	pStyle->SetPropertyInteger(PROP_UR_RADIUS, BUTTON_CORNER_RADIUS);
	pStyle->SetPropertyInteger(PROP_LL_RADIUS, BUTTON_CORNER_RADIUS);
	pStyle->SetPropertyInteger(PROP_LR_RADIUS, BUTTON_CORNER_RADIUS);
	pButton->SetStyle(STYLE_HOVER, pStyle);

	pButton->SetStyle(STYLE_NORMAL, NULL);
	pButton->SetStyle(STYLE_DISABLED, NULL);
	pButton->SetStyle(STYLE_TEXT, NULL);

	//	Add

	if (pContainer)
		pContainer->AddTrack(pButton, 0);

	//	Done

	if (retpControl)
		*retpControl = pButton;
	}

void CVisualPalette::CreateImageButton (CAniSequencer *pContainer,
										const CString &sID,
										int x,
										int y,
										const CG16bitImage *pImage,
										const CString &sLabel,
										DWORD dwOptions,
										IAnimatron **retpControl) const

//	CreateImageButton
//
//	Creates a standard image button

	{
	CAniButton *pButton = new CAniButton;
	pButton->SetID(sID);
	pButton->SetPropertyVector(CONSTLIT("position"), CVector(x, y));
	pButton->SetPropertyVector(CONSTLIT("scale"), CVector(IMAGE_BUTTON_WIDTH, IMAGE_BUTTON_HEIGHT));
	pButton->SetPropertyString(CONSTLIT("text"), sLabel);

	//	Set styles

	IAnimatron *pStyle = new CAniRoundedRect;
	pStyle->SetPropertyColor(PROP_COLOR, GetColor(colorAreaDialogTitle));
	pStyle->SetPropertyOpacity(PROP_OPACITY, 255);
	pStyle->SetPropertyString(PROP_LINE_TYPE, LINE_TYPE_SOLID);
	pStyle->SetPropertyColor(PROP_LINE_COLOR, GetColor(colorLineDialogFrame));
	pStyle->SetPropertyInteger(PROP_LINE_WIDTH, IMAGE_BUTTON_BORDER_WIDTH);
	pStyle->SetPropertyInteger(PROP_UL_RADIUS, BUTTON_CORNER_RADIUS);
	pStyle->SetPropertyInteger(PROP_UR_RADIUS, BUTTON_CORNER_RADIUS);
	pStyle->SetPropertyInteger(PROP_LL_RADIUS, BUTTON_CORNER_RADIUS);
	pStyle->SetPropertyInteger(PROP_LR_RADIUS, BUTTON_CORNER_RADIUS);
	pButton->SetStyle(STYLE_DOWN, pStyle);

	pStyle = new CAniRoundedRect;
	pStyle->SetPropertyColor(PROP_COLOR, GetColor(colorAreaDialogTitle));
	pStyle->SetPropertyOpacity(PROP_OPACITY, 255);
	pStyle->SetPropertyString(PROP_LINE_TYPE, LINE_TYPE_SOLID);
	pStyle->SetPropertyColor(PROP_LINE_COLOR, GetColor(colorLineDialogFrame));
	pStyle->SetPropertyInteger(PROP_LINE_WIDTH, IMAGE_BUTTON_BORDER_WIDTH);
	pStyle->SetPropertyInteger(PROP_UL_RADIUS, BUTTON_CORNER_RADIUS);
	pStyle->SetPropertyInteger(PROP_UR_RADIUS, BUTTON_CORNER_RADIUS);
	pStyle->SetPropertyInteger(PROP_LL_RADIUS, BUTTON_CORNER_RADIUS);
	pStyle->SetPropertyInteger(PROP_LR_RADIUS, BUTTON_CORNER_RADIUS);
	pButton->SetStyle(STYLE_HOVER, pStyle);

	pStyle = new CAniRoundedRect;
	pStyle->SetPropertyColor(PROP_COLOR, GetColor(colorAreaDialogInput));
	pStyle->SetPropertyOpacity(PROP_OPACITY, 255);
	pStyle->SetPropertyString(PROP_LINE_TYPE, LINE_TYPE_SOLID);
	pStyle->SetPropertyColor(PROP_LINE_COLOR, GetColor(colorTextDialogLabel));
	pStyle->SetPropertyInteger(PROP_LINE_WIDTH, IMAGE_BUTTON_BORDER_WIDTH);
	pStyle->SetPropertyInteger(PROP_UL_RADIUS, BUTTON_CORNER_RADIUS);
	pStyle->SetPropertyInteger(PROP_UR_RADIUS, BUTTON_CORNER_RADIUS);
	pStyle->SetPropertyInteger(PROP_LL_RADIUS, BUTTON_CORNER_RADIUS);
	pStyle->SetPropertyInteger(PROP_LR_RADIUS, BUTTON_CORNER_RADIUS);
	pButton->SetStyle(STYLE_NORMAL, pStyle);

	pStyle = new CAniRoundedRect;
	pStyle->SetPropertyColor(PROP_COLOR, GetColor(colorAreaDialogInput));
	pStyle->SetPropertyOpacity(PROP_OPACITY, 128);
	pStyle->SetPropertyString(PROP_LINE_TYPE, LINE_TYPE_SOLID);
	pStyle->SetPropertyColor(PROP_LINE_COLOR, GetColor(colorTextDialogLabel));
	pStyle->SetPropertyInteger(PROP_LINE_WIDTH, IMAGE_BUTTON_BORDER_WIDTH);
	pStyle->SetPropertyInteger(PROP_UL_RADIUS, BUTTON_CORNER_RADIUS);
	pStyle->SetPropertyInteger(PROP_UR_RADIUS, BUTTON_CORNER_RADIUS);
	pStyle->SetPropertyInteger(PROP_LL_RADIUS, BUTTON_CORNER_RADIUS);
	pStyle->SetPropertyInteger(PROP_LR_RADIUS, BUTTON_CORNER_RADIUS);
	pButton->SetStyle(STYLE_DISABLED, pStyle);

	pStyle = new CAniText;
	pStyle->SetPropertyColor(PROP_COLOR, GetColor(colorTextDialogInput));
	pStyle->SetPropertyFont(PROP_FONT, &GetFont(fontMedium));
	pStyle->SetPropertyString(PROP_TEXT_ALIGN_HORZ, CONSTLIT("center"));
	pStyle->SetPropertyString(PROP_TEXT_ALIGN_VERT, CONSTLIT("bottom"));
	pButton->SetStyle(STYLE_TEXT, pStyle);

	pStyle = new CAniRect;
	pStyle->SetPropertyVector(PROP_POSITION, CVector((IMAGE_BUTTON_WIDTH - pImage->GetWidth()) / 2, IMAGE_BUTTON_IMAGE_PADDING_TOP));
	pStyle->SetPropertyVector(PROP_SCALE, CVector(pImage->GetWidth(), pImage->GetHeight()));
	pStyle->SetFillMethod(new CAniImageFill(pImage, false));
	pButton->SetStyle(STYLE_IMAGE, pStyle);

	//	Add

	if (pContainer)
		pContainer->AddTrack(pButton, 0);

	//	Done

	if (retpControl)
		*retpControl = pButton;
	}

void CVisualPalette::CreateImageButtonSmall (CAniSequencer *pContainer,
											 const CString &sID,
											 int x,
											 int y,
											 const CG16bitImage *pImage,
											 DWORD dwOptions,
											 IAnimatron **retpControl) const

//	CreateImageButtonSmall
//
//	Creates a standard image button

	{
	CAniButton *pButton = new CAniButton;
	pButton->SetID(sID);
	pButton->SetPropertyVector(CONSTLIT("position"), CVector(x, y));
	pButton->SetPropertyVector(CONSTLIT("scale"), CVector(SMALL_IMAGE_BUTTON_WIDTH, SMALL_IMAGE_BUTTON_HEIGHT));

	//	Set styles

	IAnimatron *pStyle = new CAniRoundedRect;
	pStyle->SetPropertyColor(PROP_COLOR, GetColor(colorAreaDialogTitle));
	pStyle->SetPropertyOpacity(PROP_OPACITY, 255);
	pStyle->SetPropertyString(PROP_LINE_TYPE, LINE_TYPE_SOLID);
	pStyle->SetPropertyColor(PROP_LINE_COLOR, GetColor(colorLineDialogFrame));
	pStyle->SetPropertyInteger(PROP_LINE_WIDTH, SMALL_IMAGE_BUTTON_BORDER_WIDTH);
	pStyle->SetPropertyInteger(PROP_UL_RADIUS, SMALL_BUTTON_CORNER_RADIUS);
	pStyle->SetPropertyInteger(PROP_UR_RADIUS, SMALL_BUTTON_CORNER_RADIUS);
	pStyle->SetPropertyInteger(PROP_LL_RADIUS, SMALL_BUTTON_CORNER_RADIUS);
	pStyle->SetPropertyInteger(PROP_LR_RADIUS, SMALL_BUTTON_CORNER_RADIUS);
	pButton->SetStyle(STYLE_DOWN, pStyle);

	pStyle = new CAniRoundedRect;
	pStyle->SetPropertyColor(PROP_COLOR, GetColor(colorAreaDialogTitle));
	pStyle->SetPropertyOpacity(PROP_OPACITY, 255);
	pStyle->SetPropertyString(PROP_LINE_TYPE, LINE_TYPE_SOLID);
	pStyle->SetPropertyColor(PROP_LINE_COLOR, GetColor(colorLineDialogFrame));
	pStyle->SetPropertyInteger(PROP_LINE_WIDTH, SMALL_IMAGE_BUTTON_BORDER_WIDTH);
	pStyle->SetPropertyInteger(PROP_UL_RADIUS, SMALL_BUTTON_CORNER_RADIUS);
	pStyle->SetPropertyInteger(PROP_UR_RADIUS, SMALL_BUTTON_CORNER_RADIUS);
	pStyle->SetPropertyInteger(PROP_LL_RADIUS, SMALL_BUTTON_CORNER_RADIUS);
	pStyle->SetPropertyInteger(PROP_LR_RADIUS, SMALL_BUTTON_CORNER_RADIUS);
	pButton->SetStyle(STYLE_HOVER, pStyle);

	pStyle = new CAniRoundedRect;
	pStyle->SetPropertyColor(PROP_COLOR, GetColor(colorAreaDialogInput));
	pStyle->SetPropertyOpacity(PROP_OPACITY, 255);
	pStyle->SetPropertyString(PROP_LINE_TYPE, LINE_TYPE_SOLID);
	pStyle->SetPropertyColor(PROP_LINE_COLOR, GetColor(colorTextDialogLabel));
	pStyle->SetPropertyInteger(PROP_LINE_WIDTH, SMALL_IMAGE_BUTTON_BORDER_WIDTH);
	pStyle->SetPropertyInteger(PROP_UL_RADIUS, SMALL_BUTTON_CORNER_RADIUS);
	pStyle->SetPropertyInteger(PROP_UR_RADIUS, SMALL_BUTTON_CORNER_RADIUS);
	pStyle->SetPropertyInteger(PROP_LL_RADIUS, SMALL_BUTTON_CORNER_RADIUS);
	pStyle->SetPropertyInteger(PROP_LR_RADIUS, SMALL_BUTTON_CORNER_RADIUS);
	pButton->SetStyle(STYLE_NORMAL, pStyle);

	pStyle = new CAniRoundedRect;
	pStyle->SetPropertyColor(PROP_COLOR, GetColor(colorAreaDialogInput));
	pStyle->SetPropertyOpacity(PROP_OPACITY, 128);
	pStyle->SetPropertyString(PROP_LINE_TYPE, LINE_TYPE_SOLID);
	pStyle->SetPropertyColor(PROP_LINE_COLOR, GetColor(colorTextDialogLabel));
	pStyle->SetPropertyInteger(PROP_LINE_WIDTH, SMALL_IMAGE_BUTTON_BORDER_WIDTH);
	pStyle->SetPropertyInteger(PROP_UL_RADIUS, SMALL_BUTTON_CORNER_RADIUS);
	pStyle->SetPropertyInteger(PROP_UR_RADIUS, SMALL_BUTTON_CORNER_RADIUS);
	pStyle->SetPropertyInteger(PROP_LL_RADIUS, SMALL_BUTTON_CORNER_RADIUS);
	pStyle->SetPropertyInteger(PROP_LR_RADIUS, SMALL_BUTTON_CORNER_RADIUS);
	pButton->SetStyle(STYLE_DISABLED, pStyle);

	pStyle = new CAniRect;
	pStyle->SetPropertyVector(PROP_POSITION, CVector((SMALL_IMAGE_BUTTON_WIDTH - pImage->GetWidth()) / 2, (SMALL_IMAGE_BUTTON_HEIGHT - pImage->GetHeight()) / 2));
	pStyle->SetPropertyVector(PROP_SCALE, CVector(pImage->GetWidth(), pImage->GetHeight()));
	pStyle->SetFillMethod(new CAniImageFill(pImage, false));
	pButton->SetStyle(STYLE_IMAGE, pStyle);

	//	Add

	if (pContainer)
		pContainer->AddTrack(pButton, 0);

	//	Done

	if (retpControl)
		*retpControl = pButton;
	}

void CVisualPalette::CreateLink (CAniSequencer *pContainer,
								 const CString &sID,
								 int x,
								 int y,
								 const CString &sLabel,
								 DWORD dwOptions,
								 IAnimatron **retpControl,
								 int *retcxWidth,
								 int *retcyHeight) const

//	CreateLink
//
//	Create link control

	{
	//	Pick the font

	const CG16bitFont *pFont;
	if (dwOptions & OPTION_LINK_MEDIUM_FONT)
		pFont = &GetFont(fontMedium);
	else if (dwOptions & OPTION_LINK_SUB_TITLE_FONT)
		pFont = &GetFont(fontSubTitle);
	else
		pFont = &GetFont(fontLarge);

	int cyHeight;
	int cxText = pFont->MeasureText(sLabel, &cyHeight);
	int cxWidth = cxText + 2 * LINK_SPACING_X;

	CAniButton *pButton = new CAniButton(CAniButton::typeLink);
	pButton->SetID(sID);
	pButton->SetPropertyVector(CONSTLIT("position"), CVector(x - LINK_SPACING_X, y));
	pButton->SetPropertyVector(CONSTLIT("scale"), CVector(cxWidth, cyHeight));
	pButton->SetPropertyString(CONSTLIT("text"), sLabel);

	//	Set styles

	pButton->SetStyle(STYLE_NORMAL, NULL);
	pButton->SetStyle(STYLE_DISABLED, NULL);

	IAnimatron *pStyle = new CAniRoundedRect;
	pStyle->SetPropertyColor(PROP_COLOR, GetColor(colorAreaDialogInputFocus));
	pStyle->SetPropertyOpacity(PROP_OPACITY, 255);
	pStyle->SetPropertyInteger(PROP_UL_RADIUS, LINK_CORNER_RADIUS);
	pStyle->SetPropertyInteger(PROP_UR_RADIUS, LINK_CORNER_RADIUS);
	pStyle->SetPropertyInteger(PROP_LL_RADIUS, LINK_CORNER_RADIUS);
	pStyle->SetPropertyInteger(PROP_LR_RADIUS, LINK_CORNER_RADIUS);
	pButton->SetStyle(STYLE_DOWN, pStyle);

	pStyle = new CAniRoundedRect;
	pStyle->SetPropertyColor(PROP_COLOR, GetColor(colorAreaDialogInputFocus));
	pStyle->SetPropertyOpacity(PROP_OPACITY, 255);
	pStyle->SetPropertyInteger(PROP_UL_RADIUS, LINK_CORNER_RADIUS);
	pStyle->SetPropertyInteger(PROP_UR_RADIUS, LINK_CORNER_RADIUS);
	pStyle->SetPropertyInteger(PROP_LL_RADIUS, LINK_CORNER_RADIUS);
	pStyle->SetPropertyInteger(PROP_LR_RADIUS, LINK_CORNER_RADIUS);
	pButton->SetStyle(STYLE_HOVER, pStyle);

	pStyle = new CAniText;
	pStyle->SetPropertyColor(PROP_COLOR, GetColor(colorTextDialogLink));
	pStyle->SetPropertyFont(PROP_FONT, pFont);
	pStyle->SetPropertyString(PROP_TEXT_ALIGN_VERT, CONSTLIT("center"));
	pButton->SetStyle(STYLE_TEXT, pStyle);

	//	Add

	if (pContainer)
		pContainer->AddTrack(pButton, 0);

	//	Compute height and width

	if (retcxWidth)
		*retcxWidth = cxText;

	if (retcyHeight)
		*retcyHeight = cyHeight;

	//	Done

	if (retpControl)
		*retpControl = pButton;
	}

void CVisualPalette::CreateMessagePane (CAniSequencer *pContainer, 
										const CString &sID, 
										const CString &sTitle,
										const CString &sDesc,
										const RECT &rcRect,
										DWORD dwOptions,
										IAnimatron **retpControl) const

//	CreateMessagePane
//
//	Creates a simple pane displaying a message

	{
	const CG16bitFont &MediumFont = GetFont(fontMedium);
	const CG16bitFont &SubTitleFont = GetFont(fontSubTitle);

	//	Figure out the position of the pane within the rect

	int xPane = rcRect.left + (RectWidth(rcRect) - MSG_PANE_WIDTH) / 2;
	int yPane = rcRect.top + (RectHeight(rcRect) - MSG_PANE_HEIGHT) / 3;

	//	Create a sequencer to hold all the animations

	CAniSequencer *pRoot;
	CAniSequencer::Create(CVector(xPane, yPane), &pRoot);

	//	Create the background rect

	CAniRoundedRect *pFrame = new CAniRoundedRect;
	pFrame->SetPropertyVector(PROP_POSITION, CVector(0, 0));
	pFrame->SetPropertyVector(PROP_SCALE, CVector(MSG_PANE_WIDTH, MSG_PANE_HEIGHT));
	pFrame->SetPropertyColor(PROP_COLOR, GetColor(colorAreaInfoMsg));
	pFrame->SetPropertyOpacity(PROP_OPACITY, 255);
	pFrame->SetPropertyInteger(PROP_UL_RADIUS, MSG_CORNER_RADIUS);
	pFrame->SetPropertyInteger(PROP_UR_RADIUS, MSG_CORNER_RADIUS);
	pFrame->SetPropertyInteger(PROP_LL_RADIUS, MSG_CORNER_RADIUS);
	pFrame->SetPropertyInteger(PROP_LR_RADIUS, MSG_CORNER_RADIUS);

	pFrame->SetPropertyString(PROP_LINE_TYPE, LINE_TYPE_SOLID);
	pFrame->SetPropertyColor(PROP_LINE_COLOR, GetColor(colorLineDialogFrame));
	pFrame->SetPropertyInteger(PROP_LINE_WIDTH, MSG_BORDER_WIDTH);

	pRoot->AddTrack(pFrame, 0);

	//	Add the title of the message

	int y = MSG_PADDING_TOP;

	IAnimatron *pTitle = new CAniText;
	pTitle->SetPropertyVector(PROP_POSITION, CVector(MSG_PADDING_LEFT, y));
	pTitle->SetPropertyVector(PROP_SCALE, CVector(MSG_PANE_WIDTH - (MSG_PADDING_LEFT + MSG_PADDING_TOP), MSG_PANE_HEIGHT));
	pTitle->SetPropertyColor(PROP_COLOR, GetColor(colorTextDialogInput));
	pTitle->SetPropertyFont(PROP_FONT, &SubTitleFont);
	pTitle->SetPropertyString(PROP_TEXT_ALIGN_HORZ, ALIGN_CENTER);
	pTitle->SetPropertyString(PROP_TEXT, sTitle);

	pRoot->AddTrack(pTitle, 0);
	y += SubTitleFont.GetHeight() + MSG_PADDING_TOP;

	//	Add desc

	IAnimatron *pText = new CAniText;
	pText->SetPropertyVector(PROP_POSITION, CVector(MSG_PADDING_LEFT, y));
	pText->SetPropertyVector(PROP_SCALE, CVector(MSG_PANE_WIDTH - (MSG_PADDING_LEFT + MSG_PADDING_TOP), MSG_PANE_HEIGHT));
	pText->SetPropertyColor(PROP_COLOR, GetColor(colorTextDialogLabel));
	pText->SetPropertyFont(PROP_FONT, &MediumFont);
	pText->SetPropertyString(PROP_TEXT_ALIGN_HORZ, ALIGN_CENTER);
	pText->SetPropertyString(PROP_TEXT, sDesc);

	pRoot->AddTrack(pText, 0);

	//	Done

	if (retpControl)
		*retpControl = pRoot;
	}

void CVisualPalette::CreateRingAnimation (CAniSequencer *pContainer, int iCount, int iMinRadius, int iInc) const

//	CreateRingAnimation
//
//	Creates spinning rings.

	{
	int i;

	int iRadius = iMinRadius;

	for (i = 0; i < iCount; i++)
		{
		int iInner = iRadius;
		int iOuter = iInner + iInc - RING_SPACING;
		int iArc = mathRandom(90, 270);
		int iStart = mathRandom(0, 359);

		//	Create the ring

		TArray<CVector> Points;
		CreateArcPolygon(iInner, iOuter, iArc, &Points);

		IAnimatron *pRing = new CAniPolygon(Points);
		pRing->SetPropertyColor(PROP_COLOR, GetColor(colorAreaDialogHighlight));

		//	Animate rotation

		Metric rRate = mathRandom(RING_MIN_RATE, RING_MAX_RATE);
		if (mathRandom(1, 100) <= 50)
			rRate = -rRate;

		pRing->AnimateLinearRotation(iStart, rRate, durationInfinite);

		//	Add it

		pContainer->AddTrack(pRing, 0);

		//	Next

		iRadius += iInc;
		iInc += RING_SIZE_INC;
		}
	}

void CVisualPalette::CreateStdDialog (const RECT &rcRect, const CString &sTitle, IAnimatron **retpDlg, CAniSequencer **retpContainer) const

//	CreateStdDialog
//
//	Creates a new dialog object of the given size. We return the dialog object,
//	which should be added to the timeline, plus a container object which should
//	be the parent of any dialog controls.
//
//	Note that rcRect is the rectangle of the desired content region; the actual
//	dialog frame will be larger.

	{
	//	Convert from the container rect to the outer dialog rect

	RECT rcDlg;
	rcDlg.left = rcRect.left - DLG_PADDING_LEFT;
	rcDlg.right = rcRect.right + DLG_PADDING_RIGHT;
	rcDlg.top = rcRect.top - DLG_TITLE_HEIGHT - DLG_PADDING_TOP;
	rcDlg.bottom = rcRect.bottom + DLG_PADDING_BOTTOM;

	//	Start with a sequencer as a parent of everything

	CAniSequencer *pDlg;
	CAniSequencer::Create(CVector(rcDlg.left, rcDlg.top), &pDlg);

	//	Animate the dialog appearing

	int yDlgStartPos = (RectHeight(rcDlg) - (2 * DLG_TITLE_HEIGHT)) / 2;

	//	Add a title rectangle

	IAnimatron *pTitleFrame;
	CAniRoundedRect::Create(CVector(), 
			CVector(RectWidth(rcDlg), DLG_TITLE_HEIGHT),
			GetColor(colorAreaDialogTitle),
			255,
			&pTitleFrame);
	pTitleFrame->AnimatePropertyLinear(NULL_STR, PROP_POSITION, CVector(0, yDlgStartPos), CVector(), DLG_APPEAR_TIME);
	pTitleFrame->SetPropertyInteger(PROP_UL_RADIUS, DLG_CORNER_RADIUS);
	pTitleFrame->SetPropertyInteger(PROP_UR_RADIUS, DLG_CORNER_RADIUS);
	pTitleFrame->SetPropertyInteger(PROP_LL_RADIUS, 0);
	pTitleFrame->SetPropertyInteger(PROP_LR_RADIUS, 0);

	pDlg->AddTrack(pTitleFrame, 0);

	//	Add the title text

	const CG16bitFont &TitleFont = GetFont(fontSubTitle);
	IAnimatron *pTitleText;
	int xTitle = DLG_PADDING_LEFT;
	int yTitle = (DLG_TITLE_HEIGHT - TitleFont.GetHeight()) / 2;
	CAniText::Create(sTitle,
			CVector(xTitle, yTitle),
			&TitleFont,
			0,
			GetColor(colorTextDialogTitle),
			&pTitleText);

	pTitleText->AnimatePropertyLinear(NULL_STR, PROP_POSITION, CVector(xTitle, yTitle + yDlgStartPos), CVector(xTitle, yTitle), DLG_APPEAR_TIME);

	pDlg->AddTrack(pTitleText, 0);

	//	Add a rectangle as a background

	IAnimatron *pFrame;
	CAniRect::Create(CVector(0, DLG_TITLE_HEIGHT), 
			CVector(RectWidth(rcDlg), RectHeight(rcDlg) - DLG_TITLE_HEIGHT),
			GetColor(colorAreaDialog),
			255,
			&pFrame);

	pFrame->AnimatePropertyLinear(NULL_STR, PROP_POSITION, CVector(0, DLG_TITLE_HEIGHT + yDlgStartPos), CVector(0, DLG_TITLE_HEIGHT), DLG_APPEAR_TIME);
	pFrame->AnimatePropertyLinear(NULL_STR, PROP_SCALE, CVector(RectWidth(rcDlg), DLG_TITLE_HEIGHT), CVector(RectWidth(rcDlg), RectHeight(rcDlg) - DLG_TITLE_HEIGHT), DLG_APPEAR_TIME);
	pDlg->AddTrack(pFrame, 0);

	//	Create a border around the dialog box

	IAnimatron *pBorder;
	CAniRoundedRect::Create(CVector(),
			CVector(RectWidth(rcDlg), RectHeight(rcDlg)),
			0,
			255,
			&pBorder);
	pBorder->AnimatePropertyLinear(NULL_STR, PROP_POSITION, CVector(0, yDlgStartPos), CVector(), DLG_APPEAR_TIME);
	pBorder->AnimatePropertyLinear(NULL_STR, PROP_SCALE, CVector(RectWidth(rcDlg), 2 * DLG_TITLE_HEIGHT), CVector(RectWidth(rcDlg), RectHeight(rcDlg)), DLG_APPEAR_TIME);
	pBorder->SetPropertyString(PROP_FILL_TYPE, FILL_TYPE_NONE);
	pBorder->SetPropertyString(PROP_LINE_TYPE, LINE_TYPE_SOLID);
	pBorder->SetPropertyColor(PROP_LINE_COLOR, GetColor(colorLineDialogFrame));
	pBorder->SetPropertyInteger(PROP_LINE_WIDTH, DLG_BORDER_WIDTH);
	pBorder->SetPropertyInteger(PROP_UL_RADIUS, DLG_CORNER_RADIUS);
	pBorder->SetPropertyInteger(PROP_UR_RADIUS, DLG_CORNER_RADIUS);
	pBorder->SetPropertyInteger(PROP_LL_RADIUS, 0);
	pBorder->SetPropertyInteger(PROP_LR_RADIUS, 0);

	pDlg->AddTrack(pBorder, 0);

	//	Add a sequencer as a container for all dialog controls

	CAniSequencer *pContainer;
	CAniSequencer::Create(CVector(DLG_PADDING_LEFT, DLG_TITLE_HEIGHT + DLG_PADDING_TOP), &pContainer);

	pContainer->AnimateLinearFade(durationInfinite, 3, 0);
	pDlg->AddTrack(pContainer, DLG_APPEAR_TIME);

	//	Done

	*retpDlg = pDlg;
	*retpContainer = pContainer;
	}

void CVisualPalette::CreateWaitAnimation (CAniSequencer *pContainer, const CString &sID, const RECT &rcRect, IAnimatron **retpControl) const

//	CreateWaitAnimation
//
//	Shows an animation while we wait for something.

	{
	//	Figure out the position of the ring animation

	int xCenter = rcRect.left + (RectWidth(rcRect) / 2);
	int yCenter = rcRect.top + (RectHeight(rcRect) / 2);

	//	Create a sequencer to hold all the animations

	CAniSequencer *pRoot;
	CAniSequencer::Create(CVector(xCenter, yCenter), &pRoot);

	//	Create rings of increasing diameter

	CreateRingAnimation(pRoot, RING_COUNT, RING_MIN_RADIUS, RING_SIZE);

	//	Done

	if (retpControl)
		*retpControl = pRoot;
	}
