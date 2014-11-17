//	CUIHelper.cpp
//
//	CUIHelper class
//	Copyright (c) 2012 by Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

const int TITLE_BAR_HEIGHT =					128;

const int ATTRIB_PADDING_X =					4;
const int ATTRIB_PADDING_Y =					1;
const int ATTRIB_SPACING_X =					2;
const int ATTRIB_SPACING_Y =					2;

const int BUTTON_HEIGHT =						96;
const int BUTTON_WIDTH =						96;

const int DAMAGE_ADJ_ICON_WIDTH =				16;
const int DAMAGE_ADJ_ICON_HEIGHT =				16;
const int DAMAGE_ADJ_ICON_SPACING_X =			2;
const int DAMAGE_ADJ_SPACING_X =				6;

const int DAMAGE_TYPE_ICON_WIDTH =				16;
const int DAMAGE_TYPE_ICON_HEIGHT =				16;

const int ICON_CORNER_RADIUS =					8;
const int ICON_HEIGHT =							96;
const int ICON_WIDTH =							96;

const int INPUT_ERROR_CORNER_RADIUS =			8;
const int INPUT_ERROR_PADDING_BOTTOM =			10;
const int INPUT_ERROR_PADDING_LEFT =			10;
const int INPUT_ERROR_PADDING_RIGHT =			10;
const int INPUT_ERROR_PADDING_TOP =				10;
const int INPUT_ERROR_TIME =					(30 * 10);

const int ITEM_TEXT_MARGIN_Y =					4;
const int ITEM_TEXT_MARGIN_X =					4;
const int ITEM_TEXT_MARGIN_BOTTOM =				10;
const int ITEM_DEFAULT_HEIGHT =					96;
const int ITEM_TITLE_EXTRA_MARGIN =				4;

const int PADDING_LEFT =						20;

const int RING_SIZE =							8;
const int RING_MIN_RADIUS =						10;
const int RING_COUNT =							4;

const WORD RGB_MODIFIER_NORMAL_BACKGROUND =		CG16bitImage::RGBValue(101,101,101);	//	H:0   S:0   B:40
const WORD RGB_MODIFIER_NORMAL_TEXT =			CG16bitImage::RGBValue(220,220,220);	//	H:0   S:0   B:86

#define ALIGN_RIGHT								CONSTLIT("right")

#define CMD_CLOSE_SESSION						CONSTLIT("cmdCloseSession")
#define CMD_OK_SESSION							CONSTLIT("cmdOKSession")

#define EVENT_ON_CLICK							CONSTLIT("onClick")

#define ID_DLG_INPUT_ERROR						CONSTLIT("uiHelperInputErrorMsg")

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

int CUIHelper::CalcItemEntryHeight (CSpaceObject *pSource, const CItem &Item, const RECT &rcRect, DWORD dwOptions) const

//	CalcItemEntryHeight
//
//	Computes the height necessary to paint the item entry.

	{
	const CVisualPalette &VI = m_HI.GetVisuals();
	const CG16bitFont &LargeBold = VI.GetFont(fontLargeBold);
	const CG16bitFont &Medium = VI.GetFont(fontMedium);

	bool bNoIcon = ((dwOptions & OPTION_NO_ICON) == OPTION_NO_ICON);
	bool bTitle = ((dwOptions & OPTION_TITLE) == OPTION_TITLE);

	//	Get the item

	CItemCtx Ctx(&Item, pSource);
	CItemType *pType = Item.GetType();
	if (pType == NULL)
		return ITEM_DEFAULT_HEIGHT;

	//	Compute the rect where the reference text will paint

	RECT rcDrawRect = rcRect;
	rcDrawRect.left += ITEM_TEXT_MARGIN_X;
	rcDrawRect.right -= ITEM_TEXT_MARGIN_X;
	if (!bNoIcon)
		rcDrawRect.left += ICON_WIDTH;

	int iLevel = pType->GetApparentLevel();

	//	Compute the height of the row

	int cyHeight = 0;

	//	Account for margin

	cyHeight += ITEM_TEXT_MARGIN_Y;

	//	Item title

	cyHeight += LargeBold.GetHeight();
	if (bTitle)
		cyHeight += ITEM_TITLE_EXTRA_MARGIN;

	//	Attributes

	TArray<SDisplayAttribute> Attribs;
	if (Item.GetDisplayAttributes(Ctx, &Attribs))
		{
		int cyAttribs;
		FormatDisplayAttributes(Attribs, rcDrawRect, &cyAttribs);
		cyHeight += cyAttribs + ATTRIB_SPACING_Y;
		}

	//	Reference

	CString sReference = pType->GetReference(Ctx);

	//	If this is a weapon, then add room for the weapon damage

	if (Item.GetReferenceDamageType(pSource, -1, 0, NULL, NULL))
		cyHeight += Medium.GetHeight();

	//	If this is armor or a shield, then add room for damage resistance

	else if (Item.GetReferenceDamageAdj(pSource, 0, NULL, NULL))
		cyHeight += Medium.GetHeight();

	//	Measure the reference text

	int iLines;
	if (!sReference.IsBlank())
		{
		iLines = Medium.BreakText(sReference, RectWidth(rcDrawRect), NULL, 0);
		cyHeight += iLines * Medium.GetHeight();
		}

	//	Measure the description

	CString sDesc = Item.GetDesc();
	iLines = Medium.BreakText(sDesc, RectWidth(rcDrawRect), NULL, 0);
	cyHeight += iLines * Medium.GetHeight();

	//	Margin

	cyHeight += ITEM_TEXT_MARGIN_BOTTOM;

	//	Done

	cyHeight = Max(ITEM_DEFAULT_HEIGHT, cyHeight);

	return cyHeight;
	}

void CUIHelper::CreateInputErrorMessage (IHISession *pSession, const RECT &rcRect, const CString &sTitle, CString &sDesc, IAnimatron **retpMsg) const

//	CreateInputErrorMessage
//
//	Creates an input error message box

	{
	const CVisualPalette &VI = m_HI.GetVisuals();
	const CG16bitFont &TitleFont = VI.GetFont(fontLargeBold);
	const CG16bitFont &DescFont = VI.GetFont(fontMedium);

	//	Start with a sequencer as a parent of everything

	CAniSequencer *pMsg;
	CAniSequencer::Create(CVector(rcRect.left, rcRect.top), &pMsg);

	//	Create a controller to handle dismissing the message

	CInputErrorMessageController *pController = new CInputErrorMessageController(pSession);
	pMsg->AddTrack(pController, 0);

	//	Add a button to handle a click

	CAniButton *pButton = new CAniButton;
	pButton->SetPropertyVector(PROP_POSITION, CVector(0, 0));
	pButton->SetPropertyVector(PROP_SCALE, CVector(RectWidth(rcRect), RectHeight(rcRect)));
	pButton->SetStyle(STYLE_DOWN, NULL);
	pButton->SetStyle(STYLE_HOVER, NULL);
	pButton->SetStyle(STYLE_NORMAL, NULL);
	pButton->SetStyle(STYLE_DISABLED, NULL);
	pButton->SetStyle(STYLE_TEXT, NULL);
	pButton->AddListener(EVENT_ON_CLICK, pController);

	pMsg->AddTrack(pButton, 0);

	//	Figure out where the text goes

	int x = INPUT_ERROR_PADDING_LEFT;
	int cxWidth = RectWidth(rcRect) - (INPUT_ERROR_PADDING_LEFT + INPUT_ERROR_PADDING_RIGHT);
	int y = INPUT_ERROR_PADDING_TOP;
	int yEnd = RectHeight(rcRect) - INPUT_ERROR_PADDING_BOTTOM;
	int cxText = 0;

	//	Title text

	IAnimatron *pTitle = new CAniText;
	pTitle->SetPropertyVector(PROP_POSITION, CVector(x, y));
	pTitle->SetPropertyVector(PROP_SCALE, CVector(cxWidth, TitleFont.GetHeight()));
	((CAniText *)pTitle)->SetPropertyFont(PROP_FONT, &TitleFont);
	pTitle->SetPropertyColor(PROP_COLOR, VI.GetColor(colorTextWarningMsg));
	pTitle->SetPropertyString(PROP_TEXT, sTitle);

	y += TitleFont.GetHeight();
	cxText += TitleFont.GetHeight();

	//	Description

	IAnimatron *pDesc = new CAniText;
	pDesc->SetPropertyVector(PROP_POSITION, CVector(x, y));
	pDesc->SetPropertyVector(PROP_SCALE, CVector(cxWidth, 1000));
	((CAniText *)pDesc)->SetPropertyFont(PROP_FONT, &DescFont);
	pDesc->SetPropertyColor(PROP_COLOR, VI.GetColor(colorTextWarningMsg));
	pDesc->SetPropertyString(PROP_TEXT, sDesc);

	RECT rcDesc;
	pDesc->GetSpacingRect(&rcDesc);
	cxText += RectHeight(rcDesc);

	//	Now that we know the height of the text, add a rectangle as a background

	int cxFrame = Max(RectHeight(rcRect), cxText + INPUT_ERROR_PADDING_TOP + INPUT_ERROR_PADDING_BOTTOM);

	IAnimatron *pRect = new CAniRoundedRect;
	pRect->SetPropertyVector(PROP_POSITION, CVector());
	pRect->SetPropertyVector(PROP_SCALE, CVector(RectWidth(rcRect), cxFrame));
	pRect->SetPropertyColor(PROP_COLOR, VI.GetColor(colorAreaWarningMsg));
	pRect->SetPropertyOpacity(PROP_OPACITY, 255);
	pRect->SetPropertyInteger(PROP_UL_RADIUS, INPUT_ERROR_CORNER_RADIUS);
	pRect->SetPropertyInteger(PROP_UR_RADIUS, INPUT_ERROR_CORNER_RADIUS);
	pRect->SetPropertyInteger(PROP_LL_RADIUS, INPUT_ERROR_CORNER_RADIUS);
	pRect->SetPropertyInteger(PROP_LR_RADIUS, INPUT_ERROR_CORNER_RADIUS);

	pMsg->AddTrack(pRect, 0);

	//	Add title and desc

	pMsg->AddTrack(pTitle, 0);
	pMsg->AddTrack(pDesc, 0);

	//	Fade after some time

	pMsg->AnimateLinearFade(INPUT_ERROR_TIME, 5, 30);

	//	If we already have an input error box, delete it

	pSession->StopPerformance(ID_DLG_INPUT_ERROR);

	//	Start a new one

	pSession->StartPerformance(pMsg, ID_DLG_INPUT_ERROR, CReanimator::SPR_FLAG_DELETE_WHEN_DONE);

	//	Done

	if (retpMsg)
		*retpMsg = pMsg;
	}

void CUIHelper::CreateSessionTitle (IHISession *pSession, 
									CCloudService &Service, 
									const CString &sTitle, 
									const TArray<SMenuEntry> *pMenu,
									DWORD dwOptions, 
									IAnimatron **retpControl) const

//	CreateSessionTitle
//
//	Creates a session title bar, including:
//
//	User icon
//	User name
//	Session title
//	Close button

	{
	int i;
	const CVisualPalette &VI = m_HI.GetVisuals();
	const CG16bitFont &SubTitleFont = VI.GetFont(fontSubTitle);

	RECT rcRect;
	VI.GetWidescreenRect(m_HI.GetScreen(), &rcRect);

	//	Create a sequencer to hold all the controls

	CAniSequencer *pRoot;
	CAniSequencer::Create(CVector(rcRect.left, rcRect.top - TITLE_BAR_HEIGHT), &pRoot);

	//	Add the header, unless excluded

	if (!(dwOptions & OPTION_SESSION_NO_HEADER))
		{
		//	The user icon is centered

		CAniRoundedRect *pIcon = new CAniRoundedRect;
		pIcon->SetPropertyVector(PROP_POSITION, CVector(0, (TITLE_BAR_HEIGHT - ICON_HEIGHT) / 2));
		pIcon->SetPropertyVector(PROP_SCALE, CVector(ICON_HEIGHT, ICON_WIDTH));
		pIcon->SetPropertyColor(PROP_COLOR, CG16bitImage::RGBValue(128, 128, 128));
		pIcon->SetPropertyOpacity(PROP_OPACITY, 255);
		pIcon->SetPropertyInteger(PROP_UL_RADIUS, ICON_CORNER_RADIUS);
		pIcon->SetPropertyInteger(PROP_UR_RADIUS, ICON_CORNER_RADIUS);
		pIcon->SetPropertyInteger(PROP_LL_RADIUS, ICON_CORNER_RADIUS);
		pIcon->SetPropertyInteger(PROP_LR_RADIUS, ICON_CORNER_RADIUS);

		pRoot->AddTrack(pIcon, 0);

		//	The user name baseline is centered.

		CString sUsername;
		WORD wUsernameColor;
		if (Service.HasCapability(ICIService::canGetUserProfile))
			{
			sUsername = Service.GetUsername();
			wUsernameColor = VI.GetColor(colorTextDialogInput);
			}
		else
			{
			sUsername = CONSTLIT("Offline");
			wUsernameColor = VI.GetColor(colorTextDialogLabel);
			}

		int y = (TITLE_BAR_HEIGHT / 2) - SubTitleFont.GetAscent();

		IAnimatron *pName = new CAniText;
		pName->SetPropertyVector(PROP_POSITION, CVector(ICON_WIDTH + PADDING_LEFT, y));
		pName->SetPropertyVector(PROP_SCALE, CVector(RectWidth(rcRect), RectHeight(rcRect)));
		pName->SetPropertyColor(PROP_COLOR, wUsernameColor);
		pName->SetPropertyFont(PROP_FONT, &SubTitleFont);
		pName->SetPropertyString(PROP_TEXT, sUsername);

		pRoot->AddTrack(pName, 0);
		y += SubTitleFont.GetHeight();

		//	Add the session title

		IAnimatron *pTitle = new CAniText;
		pTitle->SetPropertyVector(PROP_POSITION, CVector(ICON_WIDTH + PADDING_LEFT, y));
		pTitle->SetPropertyVector(PROP_SCALE, CVector(RectWidth(rcRect), RectHeight(rcRect)));
		pTitle->SetPropertyColor(PROP_COLOR, VI.GetColor(colorTextDialogTitle));
		pTitle->SetPropertyFont(PROP_FONT, &SubTitleFont);
		pTitle->SetPropertyString(PROP_TEXT, sTitle);

		pRoot->AddTrack(pTitle, 0);
		}

	//	Add command buttons at the bottom

	int yBottomBar = TITLE_BAR_HEIGHT + RectHeight(rcRect);

	//	Add a close button.

	if (!(dwOptions & OPTION_SESSION_NO_CANCEL_BUTTON))
		{
		//	If we have an OK button, then the label is Cancel.

		CString sCloseLabel = ((dwOptions & OPTION_SESSION_OK_BUTTON) ? CONSTLIT("Cancel") : CONSTLIT("Close"));
		const CG16bitImage &CloseIcon = VI.GetImage(imageCloseIcon);

		IAnimatron *pCloseButton;
		VI.CreateImageButton(pRoot, CMD_CLOSE_SESSION, RectWidth(rcRect) - BUTTON_WIDTH, yBottomBar + (TITLE_BAR_HEIGHT - BUTTON_HEIGHT) / 2, &CloseIcon, sCloseLabel, 0, &pCloseButton);

		pSession->RegisterPerformanceEvent(pCloseButton, EVENT_ON_CLICK, CMD_CLOSE_SESSION);
		}

	//	Add an OK button, if necessary

	if (dwOptions & OPTION_SESSION_OK_BUTTON)
		{
		const CG16bitImage &OKIcon = VI.GetImage(imageOKIcon);

		IAnimatron *pOKButton;
		VI.CreateImageButton(pRoot, CMD_OK_SESSION, (RectWidth(rcRect) - BUTTON_WIDTH) / 2, yBottomBar + (TITLE_BAR_HEIGHT - BUTTON_HEIGHT) / 2, &OKIcon, CONSTLIT("OK"), 0, &pOKButton);

		pSession->RegisterPerformanceEvent(pOKButton, EVENT_ON_CLICK, CMD_OK_SESSION);
		}

	//	Add extensions button

	else if (dwOptions & OPTION_SESSION_ADD_EXTENSION_BUTTON)
		{
		const CG16bitImage &OKIcon = VI.GetImage(imageSlotIcon);

		IAnimatron *pOKButton;
		VI.CreateImageButton(pRoot, CMD_OK_SESSION, (RectWidth(rcRect) - BUTTON_WIDTH) / 2, yBottomBar + (TITLE_BAR_HEIGHT - BUTTON_HEIGHT) / 2, &OKIcon, CONSTLIT("Add Extension"), 0, &pOKButton);

		pSession->RegisterPerformanceEvent(pOKButton, EVENT_ON_CLICK, CMD_OK_SESSION);
		}

	//	Add menu items, if necessary

	if (pMenu)
		{
		//	Menu is to the left.

		int xMenu = 0;
		int yMenu = yBottomBar + (TITLE_BAR_HEIGHT - BUTTON_HEIGHT) / 2;

		for (i = 0; i < pMenu->GetCount(); i++)
			{
			const SMenuEntry &Entry = pMenu->GetAt(i);

			IAnimatron *pButton;
			int cyHeight;
			VI.CreateLink(pRoot, Entry.sCommand, xMenu, yMenu, Entry.sLabel, 0, &pButton, NULL, &cyHeight);
			yMenu += cyHeight;

			pSession->RegisterPerformanceEvent(pButton, EVENT_ON_CLICK, Entry.sCommand);
			}
		}

	//	Done

	*retpControl = pRoot;
	}

void CUIHelper::CreateSessionWaitAnimation (const CString &sID, const CString &sText, IAnimatron **retpControl) const

//	CreateSessionWaitAnimation
//
//	Creates a small wait animation at the lower-left of a session screen.

	{
	const CVisualPalette &VI = m_HI.GetVisuals();

	//	Compute some metrics

	RECT rcRect;
	VI.GetWidescreenRect(m_HI.GetScreen(), &rcRect);

	//	Figure out the position of the ring animation

	int xCenter = rcRect.left + (TITLE_BAR_HEIGHT / 2);
	int yCenter = rcRect.bottom + (TITLE_BAR_HEIGHT / 2);

	//	Create a sequencer to hold all the animations

	CAniSequencer *pRoot;
	CAniSequencer::Create(CVector(xCenter, yCenter), &pRoot);

	//	Create rings of increasing diameter

	VI.CreateRingAnimation(pRoot, RING_COUNT, RING_MIN_RADIUS, RING_SIZE);

	//	Add some text

	if (!sText.IsBlank())
		{
		const CG16bitFont &SubTitleFont = VI.GetFont(fontSubTitle);

		int xText = 8 + (TITLE_BAR_HEIGHT / 2);
		int yText = -(SubTitleFont.GetHeight() / 2);

		IAnimatron *pName = new CAniText;
		pName->SetPropertyVector(PROP_POSITION, CVector(xText, yText));
		pName->SetPropertyVector(PROP_SCALE, CVector(RectWidth(rcRect), SubTitleFont.GetHeight()));
		pName->SetPropertyColor(PROP_COLOR, VI.GetColor(colorTextFade));
		pName->SetPropertyFont(PROP_FONT, &SubTitleFont);
		pName->SetPropertyString(PROP_TEXT, sText);

		pRoot->AddTrack(pName, 0);
		}

	//	Done

	if (retpControl)
		*retpControl = pRoot;
	}

void CUIHelper::FormatDisplayAttributes (TArray<SDisplayAttribute> &Attribs, const RECT &rcRect, int *retcyHeight) const

//	FormatDisplayAttributes
//
//	Initializes the rcRect structure in all attribute entries.

	{
	int i;
	const CG16bitFont &Medium = m_HI.GetVisuals().GetFont(fontMedium);

	int cxLeft = RectWidth(rcRect);
	int x = rcRect.left;
	int y = rcRect.top;

	for (i = 0; i < Attribs.GetCount(); i++)
		{
		int cxText = (ATTRIB_PADDING_X * 2) + Medium.MeasureText(Attribs[i].sText);
		int cyText = (ATTRIB_PADDING_Y * 2) + Medium.GetHeight();
		if (cxText > cxLeft && cxLeft != RectWidth(rcRect))
			{
			y += cyText + ATTRIB_SPACING_Y;
			cxLeft = RectWidth(rcRect);
			}

		Attribs[i].rcRect.left = x;
		Attribs[i].rcRect.top = y;
		Attribs[i].rcRect.right = x + cxText;
		Attribs[i].rcRect.bottom = y + cyText;

		x += cxText + ATTRIB_SPACING_X;
		cxLeft -= cxText + ATTRIB_SPACING_X;
		}

	y += (ATTRIB_PADDING_Y * 2) + Medium.GetHeight();

	if (retcyHeight)
		*retcyHeight = y - rcRect.top;
	}

void CUIHelper::GenerateDockScreenRTF (const CString &sText, CString *retsRTF) const

//	GenerateDockScreenRTF
//
//	Converts from plain text to RTF.

	{
	CMemoryWriteStream Output;
	if (Output.Create() != NOERROR)
		return;

	//	LATER: Get this from VI. We can't right now because it only stores
	//	16-bit colors.

	DWORD wQuoteColor = RGB(178, 217, 255);	//	H:210 S:30 B:100
	CString sCloseQuote = CONSTLIT("”}");

	//	Start with the RTF open

	Output.Write("{/rtf ", 6);

	//	Parse

	bool bInQuotes = false;
	char *pPos = sText.GetASCIIZPointer();
	while (*pPos != '\0')
		{
		if (*pPos == '\"')
			{
			if (bInQuotes)
				{
				Output.Write(sCloseQuote.GetPointer(), sCloseQuote.GetLength());
				bInQuotes = false;
				}
			else
				{
				CString sQuoteStart = strPatternSubst(CONSTLIT("{/c:0x%08x; “"), wQuoteColor);
				Output.Write(sQuoteStart.GetASCIIZPointer(), sQuoteStart.GetLength());
				bInQuotes = true;
				}
			}
		else if (*pPos == '\\')
			Output.Write("/\\", 2);
		else if (*pPos == '/')
			Output.Write("//", 2);
		else if (*pPos == '{')
			Output.Write("/{", 2);
		else if (*pPos == '}')
			Output.Write("/}", 2);
		else if (*pPos == '\n')
			{
			if (bInQuotes)
				{
				//	Look ahead and see if the next paragraph starts with a quote. If it does, then
				//	we close here.

				char *pScan = pPos + 1;
				while (*pScan != '\0' && (*pScan == '\n' || *pScan == ' '))
					pScan++;

				if (*pScan == '\"')
					{
					Output.Write("}", 1);
					bInQuotes = false;
					}
				}

			Output.Write("/n", 2);
			}
		else
			Output.Write(pPos, 1);

		pPos++;
		}

	//	Make sure we always match

	if (bInQuotes)
		Output.Write("}", 1);

	//	Close

	Output.Write("}", 1);

	//	Done

	*retsRTF = CString(Output.GetPointer(), Output.GetLength());
	}

void CUIHelper::PaintDisplayAttributes (CG16bitImage &Dest, TArray<SDisplayAttribute> &Attribs) const

//	PaintDisplayAttributes
//
//	Paints all display attributes. We assume that FormatDisplayAttributes
//	has already been called.

	{
	int i;
	const CVisualPalette &VI = m_HI.GetVisuals();
	const CG16bitFont &Medium = VI.GetFont(fontMedium);

	for (i = 0; i < Attribs.GetCount(); i++)
		{
		WORD wBackColor;
		WORD wTextColor;

		//	Figure out the colors

		switch (Attribs[i].iType)
			{
			case attribPositive:
				wBackColor = VI.GetColor(colorAreaAdvantage);
				wTextColor = VI.GetColor(colorTextAdvantage);
				break;

			case attribNegative:
				wBackColor = VI.GetColor(colorAreaDisadvantage);
				wTextColor = VI.GetColor(colorTextDisadvantage);
				break;

			default:
				wBackColor = RGB_MODIFIER_NORMAL_BACKGROUND;
				wTextColor = RGB_MODIFIER_NORMAL_TEXT;
				break;
			}

		//	Draw the background

		::DrawRoundedRect(Dest, 
				Attribs[i].rcRect.left, 
				Attribs[i].rcRect.top, 
				RectWidth(Attribs[i].rcRect), 
				RectHeight(Attribs[i].rcRect), 
				4, 
				wBackColor);

		//	Draw the text

		Medium.DrawText(Dest, 
				Attribs[i].rcRect.left + ATTRIB_PADDING_X, 
				Attribs[i].rcRect.top + ATTRIB_PADDING_Y, 
				wTextColor, 
				255, 
				Attribs[i].sText);
		}
	}

void CUIHelper::PaintItemEntry (CG16bitImage &Dest, CSpaceObject *pSource, const CItem &Item, const RECT &rcRect, DWORD dwOptions) const

//	PaintItemEntry
//
//	Paints an item entry suitable for an item list.

	{
	const CVisualPalette &VI = m_HI.GetVisuals();
	const CG16bitFont &LargeBold = VI.GetFont(fontLargeBold);
	const CG16bitFont &Medium = VI.GetFont(fontMedium);
	WORD wColorTitle = VI.GetColor(colorTextHighlight);
	WORD wColorRef = VI.GetColor(colorTextHighlight);
	WORD wColorDescSel = CG16bitImage::RGBValue(200,200,200);
	WORD wColorDesc = CG16bitImage::RGBValue(128,128,128);

	bool bSelected = ((dwOptions & OPTION_SELECTED) == OPTION_SELECTED);
	bool bNoIcon = ((dwOptions & OPTION_NO_ICON) == OPTION_NO_ICON);
	bool bTitle = ((dwOptions & OPTION_TITLE) == OPTION_TITLE);

	//	Item context

	CItemCtx Ctx(&Item, pSource);
	CItemType *pItemType = Item.GetType();

	//	Calc the rect where we will draw

	RECT rcDrawRect = rcRect;
	rcDrawRect.left += ITEM_TEXT_MARGIN_X;
	rcDrawRect.right -= ITEM_TEXT_MARGIN_X;
	rcDrawRect.top += ITEM_TEXT_MARGIN_Y;

	//	Paint the image

	if (!bNoIcon)
		{
		DrawItemTypeIcon(Dest, rcRect.left, rcRect.top, pItemType);
		rcDrawRect.left += ICON_WIDTH;
		}

	//	Paint the item name

	DWORD dwNounPhraseFlags = nounNoModifiers;
	if (bTitle)
		dwNounPhraseFlags |= nounTitleCapitalize | nounShort;
	else
		dwNounPhraseFlags |= nounCount;

	int cyHeight;
	RECT rcTitle = rcDrawRect;
	LargeBold.DrawText(Dest,
			rcTitle,
			wColorTitle,
			Item.GetNounPhrase(dwNounPhraseFlags),
			0,
			CG16bitFont::SmartQuotes | CG16bitFont::TruncateLine,
			&cyHeight);

	rcDrawRect.top += cyHeight;
	if (bTitle)
		rcDrawRect.top += ITEM_TITLE_EXTRA_MARGIN;

	//	Paint the display attributes

	TArray<SDisplayAttribute> Attribs;
	if (Item.GetDisplayAttributes(Ctx, &Attribs))
		{
		FormatDisplayAttributes(Attribs, rcDrawRect, &cyHeight);
		PaintDisplayAttributes(Dest, Attribs);
		rcDrawRect.top += cyHeight + ATTRIB_SPACING_Y;
		}

	//	Stats

	CString sStat;

	int iLevel = pItemType->GetApparentLevel();
	CString sReference = pItemType->GetReference(Ctx);
	DamageTypes iDamageType;
	CString sDamageRef;
	int iDamageAdj[damageCount];
	int iHP;

	if (Item.GetReferenceDamageType(pSource, -1, 0, &iDamageType, &sDamageRef))
		{
		//	Paint the damage type reference

		PaintReferenceDamageType(Dest,
				rcDrawRect.left + DAMAGE_ADJ_SPACING_X,
				rcDrawRect.top,
				iDamageType,
				sDamageRef);

		rcDrawRect.top += Medium.GetHeight();

		//	Paint additional reference in the line below

		if (!sReference.IsBlank())
			{
			Medium.DrawText(Dest, 
					rcDrawRect,
					wColorRef,
					sReference,
					0,
					0,
					&cyHeight);

			rcDrawRect.top += cyHeight;
			}
		}
	else if (Item.GetReferenceDamageAdj(pSource, 0, &iHP, iDamageAdj))
		{
		//	Paint the initial text

		sStat = strPatternSubst("hp: %d ", iHP);
		int cxWidth = Medium.MeasureText(sStat, &cyHeight);
		Medium.DrawText(Dest, 
				rcDrawRect,
				wColorRef,
				sStat,
				0,
				0,
				&cyHeight);

		//	Paint the damage type array

		PaintReferenceDamageAdj(Dest,
				rcDrawRect.left + cxWidth + DAMAGE_ADJ_SPACING_X,
				rcDrawRect.top,
				iLevel,
				iHP,
				iDamageAdj);

		rcDrawRect.top += cyHeight;

		//	Paint additional reference in the line below

		if (!sReference.IsBlank())
			{
			Medium.DrawText(Dest, 
					rcDrawRect,
					wColorRef,
					sReference,
					0,
					0,
					&cyHeight);

			rcDrawRect.top += cyHeight;
			}
		}
	else
		{
		Medium.DrawText(Dest, 
				rcDrawRect,
				wColorRef,
				sReference,
				0,
				0,
				&cyHeight);

		rcDrawRect.top += cyHeight;
		}

	//	Description

	CString sDesc = Item.GetDesc();
	Medium.DrawText(Dest,
			rcDrawRect,
			(bSelected ? wColorDescSel : wColorDesc),
			sDesc,
			0,
			CG16bitFont::SmartQuotes,
			&cyHeight);
	rcDrawRect.top += cyHeight;
	}

void CUIHelper::PaintReferenceDamageAdj (CG16bitImage &Dest, int x, int y, int iLevel, int iHP, const int *iDamageAdj) const

//	PaintReferenceDamageAdj
//
//	Takes an array of damage type adj values and displays them

	{
	int i;
	bool bSortByDamageType = true;
	bool bOptionShowDamageAdjAsHP = false;

	const CVisualPalette &VI = m_HI.GetVisuals();
	const CG16bitFont &Small = VI.GetFont(fontSmall);
	const CG16bitFont &Medium = VI.GetFont(fontMedium);
	WORD wColorRef = VI.GetColor(colorTextHighlight);

	//	Must have positive HP

	if (iHP == 0)
		return;

	//	Sort damage types from highest to lowest

	CSymbolTable Sorted;
	int iLengthEstimate = 0;
	int iImmuneCount = 0;
	for (i = 0; i < damageCount; i++)
		{
		//	Skip if this damage type is not appropriate to our level

		int iDamageLevel = GetDamageTypeLevel((DamageTypes)i);
		if (iDamageLevel < iLevel - 5 || iDamageLevel > iLevel + 3)
			continue;

		//	Skip if the damage adj is 100%

		if (iDamageAdj[i] == iHP)
			continue;

		//	Figure out the sort order

		CString sKey;
		if (bSortByDamageType)
			sKey = strPatternSubst(CONSTLIT("%02d"), i);
		else
			{
			DWORD dwHighToLow = (iDamageAdj[i] == -1 ? 0 : 1000000 - iDamageAdj[i]);
			sKey = strPatternSubst(CONSTLIT("%08x %02d"), dwHighToLow, i);
			}

		//	Add to list

		DWORD dwValue = MAKELONG((WORD)i, (WORD)(short)iDamageAdj[i]);

		Sorted.AddEntry(sKey, (CObject *)dwValue);

		//	Estimate how many entries we will have (so we can decide the font size)
		//	We assume that immune entries get collapsed.

		if (iDamageAdj[i] != -1)
			iLengthEstimate++;
		else
			iImmuneCount++;
		}

	//	If we have six or more icons, then we need to paint smaller

	iLengthEstimate += Min(2, iImmuneCount);
	const CG16bitFont &TheFont = (iLengthEstimate >= 6 ? Small : Medium);
	int cyOffset = (Medium.GetHeight() - TheFont.GetHeight()) / 2;
	
	//	Paint the icons

	for (i = 0; i < Sorted.GetCount(); i++)
		{
		DWORD dwValue = (DWORD)Sorted.GetValue(i);
		int iDamageType = LOWORD(dwValue);
		int iDamageAdj = (int)(short)HIWORD(dwValue);
		int iPercentAdj = (100 * (iDamageAdj - iHP) / iHP);

		//	Prettify the % by rounding to a number divisible by 5

		int iPrettyPercent = 5 * ((iPercentAdj + 2 * Sign(iPercentAdj)) / 5);

		//	Skip if prettify results in 0%

		if (bOptionShowDamageAdjAsHP && iPrettyPercent == 0)
			continue;

		//	Draw icon

		g_pHI->GetVisuals().DrawDamageTypeIcon(Dest, x, y, (DamageTypes)iDamageType);
		x += DAMAGE_TYPE_ICON_WIDTH + DAMAGE_ADJ_ICON_SPACING_X;

		//	If we have a bunch of entries with "immune", then compress them

		if (i < (Sorted.GetCount() - 1)
				&& iDamageAdj == -1
				&& (iDamageAdj == (int)(short)HIWORD((DWORD)Sorted.GetValue(i + 1))))
			continue;

		//	Figure out how to display damage adj

		CString sStat;
		if (iDamageAdj == -1)
			sStat = CONSTLIT("immune");
		else if (bOptionShowDamageAdjAsHP)
			sStat = strFromInt(iDamageAdj);
		else
			sStat = strPatternSubst(CONSTLIT("%s%d%%"), (iPrettyPercent > 0 ? CONSTLIT("+") : NULL_STR), iPrettyPercent);
		
		//	Draw

		Dest.DrawText(x,
				y + cyOffset,
				TheFont,
				wColorRef,
				sStat,
				0,
				&x);

		x += DAMAGE_ADJ_SPACING_X;
		}
	}

void CUIHelper::PaintReferenceDamageType (CG16bitImage &Dest, int x, int y, int iDamageType, const CString &sRef) const

//	PaintReferenceDamageType
//
//	Draws a single damage type icon (if not damageError) followed by reference text

	{
	const CVisualPalette &VI = m_HI.GetVisuals();
	const CG16bitFont &Medium = VI.GetFont(fontMedium);
	WORD wColorRef = VI.GetColor(colorTextHighlight);

	//	Paint the icon first

	if (iDamageType != damageError)
		{
		g_pHI->GetVisuals().DrawDamageTypeIcon(Dest, x, y, (DamageTypes)iDamageType);
		x += DAMAGE_TYPE_ICON_WIDTH + DAMAGE_ADJ_ICON_SPACING_X;
		}

	//	Paint the reference text

	Dest.DrawText(x,
			y,
			Medium,
			wColorRef,
			sRef,
			0);
	}

//	PaintItemEntry
void CInputErrorMessageController::OnAniCommand (const CString &sID, const CString &sEvent, const CString &sCmd, DWORD dwData)

//	OnAniCommand
//
//	Handles a command

	{
	m_pSession->StopPerformance(ID_DLG_INPUT_ERROR);
	}
