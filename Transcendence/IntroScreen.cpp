//	IntroScreen.cpp
//
//	Show intro screen

#include "PreComp.h"
#include "Transcendence.h"
#include "XMLUtil.h"

#include "Credits.h"

#define INTRO_DISPLAY_WIDTH						1024
#define INTRO_DISPLAY_HEIGHT					512

#define RGB_VERSION_COLOR						CG32bitPixel(128,128,128)
#define RGB_COPYRIGHT_COLOR						CG32bitPixel(80,80,80)
#define RGB_FRAME								CG32bitPixel(80,80,80)

#define MAX_TIME_WITH_ONE_SHIP					(g_TicksPerSecond * 90)
#define MAX_INTRO_SHIPS							500

#define DIALOG_WIDTH							500
#define DIALOG_HEIGHT							250
#define DIALOG_SPACING_X						8
#define DIALOG_SPACING_Y						8
#define DIALOG_BUTTON_HEIGHT					25
#define DIALOG_BUTTON_WIDTH						100
#define RGB_DIALOG_BACKGROUND					CG32bitPixel(48,48,64)
#define RGB_DIALOG_TEXT							CG32bitPixel(144,144,192)
#define RGB_DIALOG_BUTTON_BACKGROUND			CG32bitPixel(144,144,192)
#define RGB_DIALOG_BUTTON_TEXT					CG32bitPixel(0, 0, 0)

#define RGB_NEWS_PANE_BACKGROUND				CG32bitPixel(128, 213, 255)

const Metric GAME_STAT_SELECTION_WIDTH =		480.0;
const Metric GAME_STAT_SELECTION_X =			-20.0;
const DWORD GAME_STAT_SELECTION_OPACITY =		64;
const int GAME_STAT_DURATION =					300;

const int BUTTON_HEIGHT =						96;
const int BUTTON_WIDTH =						96;
const int ICON_CORNER_RADIUS =					8;
const int ICON_HEIGHT =							96;
const int ICON_WIDTH =							96;
const int PADDING_LEFT =						20;
const int SMALL_LINK_SPACING =					20;
const int TITLE_BAR_HEIGHT =					128;

const int NEWS_PANE_WIDTH =						400;
const int NEWS_PANE_PADDING_X =					20;
const int NEWS_PANE_PADDING_Y =					8;
const int NEWS_PANE_CORNER_RADIUS =				8;
const int NEWS_PANE_INNER_SPACING_Y =			8;

#define ALIGN_CENTER							CONSTLIT("center")

#define CMD_ACCOUNT								CONSTLIT("cmdAccount")
#define CMD_ACCOUNT_EDIT						CONSTLIT("cmdAccountEdit")
#define CMD_CHANGE_PASSWORD						CONSTLIT("cmdChangePassword")
#define CMD_OPEN_NEWS							CONSTLIT("cmdOpenNews")
#define CMD_SHOW_MOD_EXCHANGE					CONSTLIT("cmdShowModExchange")
#define CMD_SHOW_PROFILE						CONSTLIT("cmdShowProfile")
#define CMD_SHOW_SETTINGS						CONSTLIT("cmdShowSettings")
#define CMD_SIGN_OUT							CONSTLIT("cmdSignOut")
#define CMD_TOGGLE_DEBUG						CONSTLIT("cmdToggleDebug")
#define CMD_TOGGLE_MUSIC						CONSTLIT("cmdToggleMusic")

#define CMD_SERVICE_NEWS_LOADED					CONSTLIT("serviceNewsLoaded")

//	These are commands that we send to the Controller

#define EVENT_ON_CLICK							CONSTLIT("onClick")

#define ID_GAME_STAT_PERFORMANCE				CONSTLIT("gameStatPerformance")
#define ID_GAME_STAT_SCROLLER					CONSTLIT("gsScroller")
#define ID_GAME_STAT_FADER						CONSTLIT("gsFader")
#define ID_GAME_STAT_SELECT_RECT				CONSTLIT("gsSelRect")
#define ID_ACCOUNT_CONTROLS						CONSTLIT("idAccount")
#define ID_CREDITS_PERFORMANCE					CONSTLIT("idCredits")
#define ID_END_GAME_PERFORMANCE					CONSTLIT("idEndGame")
#define ID_PLAYER_BAR_PERFORMANCE				CONSTLIT("idPlayerBar")
#define ID_SHIP_DESC_PERFORMANCE				CONSTLIT("idShipDescPerformance")
#define ID_TITLES_PERFORMANCE					CONSTLIT("idTitles")
#define ID_NEWS_PERFORMANCE						CONSTLIT("idNews")
#define ID_SOUNDTRACK_TITLE_PERFORMANCE			CONSTLIT("idSoundtrackTitle")

#define PROP_COLOR								CONSTLIT("color")
#define PROP_FONT								CONSTLIT("font")
#define PROP_LL_RADIUS							CONSTLIT("llRadius")
#define PROP_LR_RADIUS							CONSTLIT("lrRadius")
#define PROP_OPACITY							CONSTLIT("opacity")
#define PROP_POSITION							CONSTLIT("position")
#define PROP_SCALE								CONSTLIT("scale")
#define PROP_SCROLL_POS							CONSTLIT("scrollPos")
#define PROP_TEXT								CONSTLIT("text")
#define PROP_TEXT_ALIGN_HORZ					CONSTLIT("textAlignHorz")
#define PROP_UL_RADIUS							CONSTLIT("ulRadius")
#define PROP_UR_RADIUS							CONSTLIT("urRadius")
#define PROP_VIEWPORT_HEIGHT					CONSTLIT("viewportHeight")

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


//Not sure where to put this
const int INTRO_HELP_TEXT_COUNT = 19;
const char *INTRO_HELP_TEXT[INTRO_HELP_TEXT_COUNT] =
{
	"[Space]: Cancel current display",
	"[F11]: Hide intro menu",
	"[!]: Enter a ~ command or spawn ship by name",
	"[C], [c]: Credits",
	"[D], [d]: Duplicate current ship",
	"[H], [h]: Show high scores",
	"[I], [i]: Show this display",
	"[K[: Destroy all ships",
	"[k]: Destroy current ship class",
	"[L], [l]: Load Game",
	"[N]: Spawn previous ship class",
	"[n]: Spawn next ship class",
	"[O], [o]: View opposing ship",
	"[P]: View next ship",
	"[p]: View previous ship",
	"[Q], [q]: Quit game",
	"[S]: Toggle intro ship sounds",
	"[s]: View ship stats",
	"[V], [v]: Title",
};

void CTranscendenceWnd::CreateCreditsAnimation (IAnimatron **retpAnimatron)

//	CreateCreditsAnimation
//
//	Creates full end credits

	{
	int i;
	CAniSequencer *pSeq = new CAniSequencer;
	int iTime = 0;

	//	Figure out the position

	int xMidCenter = m_rcIntroMain.left + RectWidth(m_rcIntroMain) / 2;
	int yMidCenter = m_rcIntroMain.bottom - RectHeight(m_rcIntroMain) / 3;

	IAnimatron *pAnimation;

	//	Create credits

	TArray<CString> Names;
	Names.Insert(CONSTLIT("George Moromisato"));
	m_UIRes.CreateMediumCredit(CONSTLIT("designed & created by"), 
			Names,
			xMidCenter,
			yMidCenter,
			150,
			&pAnimation);
	pSeq->AddTrack(pAnimation, iTime);
	iTime += 150;

	Names.DeleteAll();
	Names.Insert(CONSTLIT("Michael Tangent"));
	m_UIRes.CreateMediumCredit(CONSTLIT("music by"), 
			Names,
			xMidCenter,
			yMidCenter,
			150,
			&pAnimation);
	pSeq->AddTrack(pAnimation, iTime);
	iTime += 150;

	//	More programming

	Names.DeleteAll();
	for (i = 0; i < ADDITIONAL_PROGRAMMING_COUNT; i++)
		Names.Insert(CString(ADDITIONAL_PROGRAMMING[i]));

	m_UIRes.CreateMediumCredit(CONSTLIT("additional programming by"),
			Names,
			xMidCenter,
			yMidCenter,
			150,
			&pAnimation);
	pSeq->AddTrack(pAnimation, iTime);
	iTime += ADDITIONAL_PROGRAMMING_COUNT * 150;

	//	Special thanks

	Names.DeleteAll();
	for (i = 0; i < SPECIAL_THANKS_COUNT; i++)
		Names.Insert(CString(SPECIAL_THANKS[i]));

	m_UIRes.CreateMediumCredit(CONSTLIT("special thanks to"),
			Names,
			xMidCenter,
			yMidCenter,
			150,
			&pAnimation);
	pSeq->AddTrack(pAnimation, iTime);
	iTime += SPECIAL_THANKS_COUNT * 150;

	//	Thanks to

	int yStart = m_rcIntroMain.top;
	int yEnd = m_rcIntroMain.bottom - m_Fonts.Header.GetHeight();
	CreateLongCreditsAnimation(xMidCenter + RectWidth(m_rcIntroMain) / 6,
			yStart,
			yEnd - yStart, 
			&pAnimation);
	pSeq->AddTrack(pAnimation, iTime);
	iTime += pAnimation->GetDuration();

	//	Done

	*retpAnimatron = pSeq;
	}

void CTranscendenceWnd::CreateIntroHelpAnimation(IAnimatron **retpAnimatron)

//	CreateHelpAnimation
//
//	Creates an animation showing help text for the intro screen keys

{
	int iDuration = 300;
	int x = m_rcIntroMain.left + (RectWidth(m_rcIntroMain) / 2) + (RectWidth(m_rcIntroMain) / 6);
	int y = m_rcIntroMain.bottom - RectHeight(m_rcIntroMain) / 2 - RectHeight(m_rcIntroMain) / 4;

	//	Create sequencer to hold everything

	CAniSequencer *pSeq = new CAniSequencer;
	IAnimatron *pText;
	CAniText::Create(CONSTLIT("Intro Key Commands"),
		CVector((Metric)x, (Metric)y),
		&m_Fonts.SubTitle,
		0,
		m_Fonts.rgbLightTitleColor,
		&pText);
	pText->AnimateLinearFade(iDuration, 15, 30);
	pSeq->AddTrack(pText, 5);

	y += m_Fonts.SubTitle.GetHeight() + (m_Fonts.SubTitle.GetHeight() / 6);

	for(int i = 0; i < INTRO_HELP_TEXT_COUNT; i++)
		{
		CAniText::Create(INTRO_HELP_TEXT[i],
			CVector((Metric)x, (Metric)y),
			&m_Fonts.Header,
			0,
			m_Fonts.rgbTitleColor,
			&pText);
		pText->AnimateLinearFade(iDuration, 15, 30);
		pSeq->AddTrack(pText, 5);

		y += m_Fonts.Header.GetHeight() + (m_Fonts.Header.GetHeight() / 6);
		}

	*retpAnimatron = pSeq;
}

void CTranscendenceWnd::CreateLongCreditsAnimation (int x, int y, int cyHeight, IAnimatron **retpAnimatron)

//	CreateLongCreditsAnimation
//
//	Creates scrolling credits

	{
	int i, j;
	CAniVScroller *pAni = new CAniVScroller;
	pAni->SetPropertyVector(CONSTLIT("position"), CVector((Metric)x, (Metric)y));
	pAni->SetPropertyMetric(CONSTLIT("viewportHeight"), (Metric)cyHeight);
	pAni->SetPropertyMetric(CONSTLIT("fadeEdgeHeight"), (Metric)(cyHeight / 8));

	//	Inspiration, Ideas, and Testing

	pAni->AddTextLine(CONSTLIT("ideas & testing"), &m_Fonts.SubTitle, m_Fonts.rgbLightTitleColor, CG16bitFont::AlignCenter);

	//	Add names

	for (i = 0; i < FEEDBACK_COUNT; i++)
		pAni->AddTextLine(CString(FEEDBACK[i]), 
				&m_Fonts.Header, 
				m_Fonts.rgbTitleColor, 
				CG16bitFont::AlignCenter,
				(i == 0 ? m_Fonts.Header.GetHeight() : 0));

	//	Software

	pAni->AddTextLine(CONSTLIT("created using"), &m_Fonts.SubTitle, m_Fonts.rgbLightTitleColor, CG16bitFont::AlignCenter, m_Fonts.Title.GetHeight());
	for (i = 0; i < SOFTWARE_COUNT; i++)
		pAni->AddTextLine(CString(SOFTWARE[i]), 
				&m_Fonts.Header, 
				m_Fonts.rgbTitleColor, 
				CG16bitFont::AlignCenter,
				(i == 0 ? m_Fonts.Header.GetHeight() : 0));

	//	Inspiration

	pAni->AddTextLine(CONSTLIT("with inspiration from"), &m_Fonts.SubTitle, m_Fonts.rgbLightTitleColor, CG16bitFont::AlignCenter, m_Fonts.Title.GetHeight());
	for (i = 0; i < INSPIRATION_COUNT; i++)
		pAni->AddTextLine(CString(INSPIRATION[i]), 
				&m_Fonts.Header, 
				m_Fonts.rgbTitleColor, 
				CG16bitFont::AlignCenter,
				(i == 0 ? m_Fonts.Header.GetHeight() : 0));

	//	Extensions credit. Build a list of all extensions 
	//	that need to be credited.

	struct SExtCredit
		{
		TArray<CString> Extensions;
		TArray<CString> Credits;
		};

	TArray<SExtCredit> ExtCredit;

	for (i = 0; i < g_pUniverse->GetExtensionDescCount(); i++)
		{
		const CExtension *pExt = g_pUniverse->GetExtensionDesc(i);
		if (!pExt->GetName().IsBlank() && pExt->GetCredits().GetCount() > 0)
			{
			//	See if we already have an extension with the same credits

			SExtCredit *pFound = NULL;
			for (j = 0; j < ExtCredit.GetCount() && pFound == NULL; j++)
				{
				if (ExtCredit[j].Credits.GetCount() == pExt->GetCredits().GetCount())
					{
					int k;

					bool bSame = true;
					for (k = 0; k < pExt->GetCredits().GetCount() && bSame; k++)
						if (!strEquals(ExtCredit[j].Credits[k], pExt->GetCredits()[k]))
							bSame = false;

					if (bSame)
						pFound = &ExtCredit[j];
					}
				}

			//	If we found a duplicate, then add the extension
			//	to the struct

			if (pFound)
				{
				//	Make sure we don't already have an extension of the same name listed

				bool bDuplicate = false;
				for (j = 0; j < pFound->Extensions.GetCount() && !bDuplicate; j++)
					if (strEquals(pFound->Extensions[j], pExt->GetName()))
						bDuplicate = true;

				if (!bDuplicate)
					pFound->Extensions.Insert(strToLower(pExt->GetName()));
				}

			//	Otherwise, add the new entry

			else
				{
				SExtCredit *pNew = ExtCredit.Insert();
				pNew->Extensions.Insert(strToLower(pExt->GetName()));
				pNew->Credits = pExt->GetCredits();
				}
			}
		}

	//	Add all the credits

	for (i = 0; i < ExtCredit.GetCount(); i++)
		{
		SExtCredit *pExt = &ExtCredit[i];

		//	Add the names of the extensions that are credited
		//	to the given people

		for (j = 0; j < pExt->Extensions.GetCount(); j++)
			if (j == pExt->Extensions.GetCount() - 1)
				{
				CString sCredit = (j == 0 ?
						pExt->Extensions[j]
						: strPatternSubst(CONSTLIT("and %s"), pExt->Extensions[j]));

				pAni->AddTextLine(sCredit,
						&m_Fonts.SubTitle, 
						m_Fonts.rgbLightTitleColor, 
						CG16bitFont::AlignCenter,
						(j == 0 ? m_Fonts.Title.GetHeight() : 0));
				}
			else
				{
				pAni->AddTextLine(strPatternSubst(CONSTLIT("%s,"), pExt->Extensions[j]),
						&m_Fonts.SubTitle, 
						m_Fonts.rgbLightTitleColor, 
						CG16bitFont::AlignCenter,
						(j == 0 ? m_Fonts.Title.GetHeight() : 0));
				}

		//	Add the people names

		for (j = 0; j < pExt->Credits.GetCount(); j++)
			pAni->AddTextLine(pExt->Credits[j], 
					&m_Fonts.Header, 
					m_Fonts.rgbTitleColor, 
					CG16bitFont::AlignCenter,
					(j == 0 ? m_Fonts.Header.GetHeight() : 0));
		}

	//	Copyright

	pAni->AddTextLine(m_sVersion, &m_Fonts.SubTitle, m_Fonts.rgbTitleColor, CG16bitFont::AlignCenter, m_Fonts.Title.GetHeight());
	pAni->AddTextLine(m_sCopyright, &m_Fonts.Medium, m_Fonts.rgbTextColor, CG16bitFont::AlignCenter);
	pAni->AddTextLine(CONSTLIT("Transcendence is a registered trademark"), &m_Fonts.Medium, m_Fonts.rgbTextColor, CG16bitFont::AlignCenter);
	pAni->AddTextLine(CONSTLIT("http://transcendence-game.com"), &m_Fonts.Medium, m_Fonts.rgbTextColor, CG16bitFont::AlignCenter);

	//	Animate

	pAni->AnimateLinearScroll(1.0);

	//	Done

	*retpAnimatron = pAni;
	}

void CTranscendenceWnd::CreateNewsAnimation (CMultiverseNewsEntry *pEntry, IAnimatron **retpAnimatron)

//	CreateNewsAnimation
//
//	Creates animation of a Multiverse news entry.

	{
	int iDuration = 600;
	int iInitialFade = 30;
	int iEndFade = 30;

	//	Compute some metrics for the pane based on the entry information

	int cxInnerPane = NEWS_PANE_WIDTH - (2 * NEWS_PANE_PADDING_X);

	CG32bitImage *pImage = pEntry->LoadImageHandoff();
	int cyImage = (pImage ? pImage->GetHeight() : 0);

	TArray<CString> TitleLines;
	m_Fonts.SubTitle.BreakText(pEntry->GetTitle(), cxInnerPane, &TitleLines);
	int cyTitle = m_Fonts.SubTitle.GetHeight() * TitleLines.GetCount();

	TArray<CString> BodyLines;
	m_Fonts.Medium.BreakText(pEntry->GetBody(), cxInnerPane, &BodyLines);
	int cyBody = m_Fonts.Medium.GetHeight() * BodyLines.GetCount();

	TArray<CString> FooterLines;
	m_Fonts.MediumHeavyBold.BreakText(pEntry->GetCallToActionText(), cxInnerPane, &FooterLines);
	int cyFooter = m_Fonts.MediumHeavyBold.GetHeight() * FooterLines.GetCount();

	int cyPane = cyImage
			+ cyTitle
			+ NEWS_PANE_INNER_SPACING_Y
			+ cyBody
			+ NEWS_PANE_INNER_SPACING_Y
			+ cyFooter
			+ NEWS_PANE_PADDING_Y;

	int xPane = m_rcIntroMain.left + (RectWidth(m_rcIntroMain) / 2) + (RectWidth(m_rcIntroMain) / 6);
	int yPane = m_rcIntroMain.top + ((RectHeight(m_rcIntroMain) - cyPane) / 2);

	//	Create sequencer to hold everything The origin of the sequencer is
	//	at the top-center of the pane.

	CAniSequencer *pSeq = new CAniSequencer;
	pSeq->SetPropertyVector(PROP_POSITION, CVector(xPane, yPane));

	int xLeft = -NEWS_PANE_WIDTH / 2;

	//	Create a button that will respond to clicks on the news pane

	CAniButton *pButton = new CAniButton;
	pButton->SetPropertyVector(PROP_POSITION, CVector(xLeft, 0));
	pButton->SetPropertyVector(PROP_SCALE, CVector(NEWS_PANE_WIDTH, cyPane));
	pButton->SetStyle(STYLE_DOWN, NULL);
	pButton->SetStyle(STYLE_HOVER, NULL);
	pButton->SetStyle(STYLE_NORMAL, NULL);
	pButton->SetStyle(STYLE_DISABLED, NULL);
	pButton->SetStyle(STYLE_TEXT, NULL);
	pButton->AddListener(EVENT_ON_CLICK, m_pIntroSession, CMD_OPEN_NEWS);

	pSeq->AddTrack(pButton, 0);

	//	Create the background

	CAniRoundedRect *pPane = new CAniRoundedRect;
	pPane->SetPropertyVector(PROP_POSITION, CVector(xLeft, 0));
	pPane->SetPropertyVector(PROP_SCALE, CVector(NEWS_PANE_WIDTH, cyPane));
	pPane->SetPropertyColor(PROP_COLOR, RGB_NEWS_PANE_BACKGROUND);
	pPane->SetPropertyOpacity(PROP_OPACITY, 64);
	pPane->SetPropertyInteger(PROP_UL_RADIUS, NEWS_PANE_CORNER_RADIUS);
	pPane->SetPropertyInteger(PROP_UR_RADIUS, NEWS_PANE_CORNER_RADIUS);
	pPane->SetPropertyInteger(PROP_LL_RADIUS, NEWS_PANE_CORNER_RADIUS);
	pPane->SetPropertyInteger(PROP_LR_RADIUS, NEWS_PANE_CORNER_RADIUS);
	pPane->AnimateLinearFade(iDuration, iInitialFade, iEndFade, 64);

	pSeq->AddTrack(pPane, 0);

	//	Add the content

	int yPos = 0;
	int xInnerLeft = -(cxInnerPane / 2);

	//	Create the image

	if (pImage)
		{
		//	If the image is wide enough to hit the rounded corners, then we
		//	need to mask it out.

		if (pImage->GetWidth() > (NEWS_PANE_WIDTH - 2 * NEWS_PANE_CORNER_RADIUS))
			{
			//	Create a mask the size of the pane and apply it to the image
			//	(We own the image so we can modify it).

			CG8bitImage PaneMask;
			PaneMask.CreateRoundedRect(NEWS_PANE_WIDTH, cyPane, NEWS_PANE_CORNER_RADIUS);
			pImage->IntersectMask(0, 
					0, 
					PaneMask.GetWidth(), 
					PaneMask.GetHeight(), 
					PaneMask,
					(pImage->GetWidth() - PaneMask.GetWidth()) / 2,
					0);
			}

		//	Create an animatron

		CAniRect *pRect = new CAniRect;
		pRect->SetPropertyVector(PROP_POSITION, CVector(-pImage->GetWidth() / 2, yPos));
		pRect->SetPropertyVector(PROP_SCALE, CVector(pImage->GetWidth(), pImage->GetHeight()));
		pRect->SetFillMethod(new CAniImageFill(pImage, true));
		pRect->AnimateLinearFade(iDuration, iInitialFade, iEndFade);

		pSeq->AddTrack(pRect, 0);

		yPos += cyImage;
		}

	//	Create the title

	IAnimatron *pText;
	CAniText::Create(pEntry->GetTitle(),
			CVector(xInnerLeft, yPos),
			&m_Fonts.SubTitle,
			CG16bitFont::AlignCenter,
			m_Fonts.rgbTitleColor,
			&pText);
	pText->SetPropertyVector(PROP_SCALE, CVector(cxInnerPane, cyPane));
	pText->AnimateLinearFade(iDuration, iInitialFade, iEndFade);
	pSeq->AddTrack(pText, 0);

	yPos += cyTitle + NEWS_PANE_INNER_SPACING_Y;

	//	Create the text

	CAniText::Create(pEntry->GetBody(),
			CVector(xInnerLeft, yPos),
			&m_Fonts.Medium,
			CG16bitFont::AlignCenter,
			m_Fonts.rgbTitleColor,
			&pText);
	pText->SetPropertyVector(PROP_SCALE, CVector(cxInnerPane, cyPane));
	pText->AnimateLinearFade(iDuration, iInitialFade, iEndFade);
	pSeq->AddTrack(pText, 0);

	yPos += cyBody + NEWS_PANE_INNER_SPACING_Y;

	//	Create the call to action

	CAniText::Create(pEntry->GetCallToActionText(),
			CVector(xInnerLeft, yPos),
			&m_Fonts.MediumHeavyBold,
			CG16bitFont::AlignCenter,
			m_Fonts.rgbTitleColor,
			&pText);
	pText->SetPropertyVector(PROP_SCALE, CVector(cxInnerPane, cyPane));
	pText->AnimateLinearFade(iDuration, iInitialFade, iEndFade);
	pSeq->AddTrack(pText, 0);

	//	Remember the URL to open when the user clicks

	m_sNewsURL = pEntry->GetCallToActionURL();

	//	Done

	*retpAnimatron = pSeq;
	}

void CTranscendenceWnd::CreatePlayerBarAnimation (IAnimatron **retpAni)

//	CreatePlayerBarAnimation
//
//	Creates an animation for the top bar.

	{
	const CVisualPalette &VI = g_pHI->GetVisuals();
	const CG16bitFont &SubTitleFont = VI.GetFont(fontSubTitle);
	CCloudService &Service = m_pTC->GetService();

	RECT rcRect;
	VI.GetWidescreenRect(&rcRect);

	//	Create a sequencer to hold all the controls

	CAniSequencer *pRoot;
	CAniSequencer::Create(CVector(rcRect.left, rcRect.top - TITLE_BAR_HEIGHT), &pRoot);

	//	Create buttons (from right to left)

	int x = RectWidth(rcRect) - BUTTON_WIDTH;
	IAnimatron *pButton;

	//	Debug icon

	if (m_pTC->GetOptionBoolean(CGameSettings::debugMode))
		{
		VI.CreateImageButton(pRoot, CMD_TOGGLE_DEBUG, x, (TITLE_BAR_HEIGHT - BUTTON_HEIGHT) / 2, &VI.GetImage(imageDebugIcon), CONSTLIT("Debug"), 0, &pButton);
		pButton->AddListener(EVENT_ON_CLICK, m_pIntroSession, CMD_TOGGLE_DEBUG);

		x -= (BUTTON_WIDTH + PADDING_LEFT);
		}

    //  Keyboard settings

	VI.CreateImageButton(pRoot, CMD_SHOW_SETTINGS, x, (TITLE_BAR_HEIGHT - BUTTON_HEIGHT) / 2, &VI.GetImage(imageSettingsIcon), CONSTLIT("Keyboard Settings"), 0, &pButton);
	pButton->AddListener(EVENT_ON_CLICK, m_pIntroSession, CMD_SHOW_SETTINGS);

	x -= (BUTTON_WIDTH + PADDING_LEFT);

	//	Music toggle

	VI.CreateImageButton(pRoot, CMD_TOGGLE_MUSIC, x, (TITLE_BAR_HEIGHT - BUTTON_HEIGHT) / 2, &VI.GetImage(imageMusicIconOn), CONSTLIT("Music On"), 0, &pButton);
	pButton->AddListener(EVENT_ON_CLICK, m_pIntroSession, CMD_TOGGLE_MUSIC);

	x -= (BUTTON_WIDTH + PADDING_LEFT);

	//	Profile

	if (Service.HasCapability(ICIService::userProfile))
		{
		VI.CreateImageButton(pRoot, CMD_SHOW_PROFILE, x, (TITLE_BAR_HEIGHT - BUTTON_HEIGHT) / 2, &VI.GetImage(imageProfileIcon), CONSTLIT("Records"), 0, &pButton);
		pButton->AddListener(EVENT_ON_CLICK, m_pIntroSession, CMD_SHOW_PROFILE);

		x -= (BUTTON_WIDTH + PADDING_LEFT);
		}

	//	Mod Exchange

	if (Service.HasCapability(ICIService::modExchange))
		{
		VI.CreateImageButton(pRoot, CMD_SHOW_MOD_EXCHANGE, x, (TITLE_BAR_HEIGHT - BUTTON_HEIGHT) / 2, &VI.GetImage(imageModExchangeIcon), CONSTLIT("Mod Collection"), 0, &pButton);
		pButton->AddListener(EVENT_ON_CLICK, m_pIntroSession, CMD_SHOW_MOD_EXCHANGE);
		}

	//	Done

	*retpAni = pRoot;
	}

void CTranscendenceWnd::CreateScoreAnimation (const CGameRecord &Stats, IAnimatron **retpAnimatron)

//	CreateScoreAnimation
//
//	Creates an animation of the given score

	{
	int i;
	int iDuration = 300;
	int x = m_rcIntroMain.left + RectWidth(m_rcIntroMain) / 2;
	int y = m_rcIntroMain.bottom - RectHeight(m_rcIntroMain) / 3;

	//	Create sequencer to hold everything

	CAniSequencer *pSeq = new CAniSequencer;

	//	Create the score

	CAniText *pCredit = new CAniText;
	pCredit->SetPropertyVector(CONSTLIT("position"), CVector((Metric)x, (Metric)y));
	pCredit->SetPropertyColor(CONSTLIT("color"), m_Fonts.rgbTitleColor);
	pCredit->SetPropertyString(CONSTLIT("text"), strFromInt(Stats.GetScore()));

	pCredit->SetPropertyFont(CONSTLIT("font"), &m_Fonts.Title);
	pCredit->SetFontFlags(CG16bitFont::AlignCenter);

	pCredit->AnimateLinearFade(iDuration, 30, 30);

	pSeq->AddTrack(pCredit, 0);
	y += m_Fonts.Title.GetHeight();

	//	Player name

	CAniText *pName = new CAniText;
	pName->SetPropertyVector(CONSTLIT("position"), CVector((Metric)x, (Metric)y));
	pName->SetPropertyColor(CONSTLIT("color"), m_Fonts.rgbTextColor);
	pName->SetPropertyString(CONSTLIT("text"), Stats.GetPlayerName());

	pName->SetPropertyFont(CONSTLIT("font"), &m_Fonts.SubTitle);
	pName->SetFontFlags(CG16bitFont::AlignCenter);

	pName->AnimateLinearFade(iDuration, 30, 30);

	pSeq->AddTrack(pName, 5);
	y += m_Fonts.SubTitle.GetHeight();

	//	Epitaph

	TArray<CString> EpitaphLines;
	m_Fonts.Header.BreakText(strCapitalize(Stats.GetEndGameEpitaph()), 512, &EpitaphLines);

	for (i = 0; i < EpitaphLines.GetCount(); i++)
		{
		CAniText *pLine = new CAniText;
		pLine->SetPropertyVector(CONSTLIT("position"), CVector((Metric)x, (Metric)y));
		pLine->SetPropertyColor(CONSTLIT("color"), m_Fonts.rgbTextColor);
		pLine->SetPropertyString(CONSTLIT("text"), EpitaphLines[i]);

		pLine->SetPropertyFont(CONSTLIT("font"), &m_Fonts.Header);
		pLine->SetFontFlags(CG16bitFont::AlignCenter);

		pLine->AnimateLinearFade(iDuration, 30, 30);

		pSeq->AddTrack(pLine, 5);
		y += m_Fonts.Header.GetHeight();
		}

	//	Done

	*retpAnimatron = pSeq;
	}

void CTranscendenceWnd::CreateShipDescAnimation (CShip *pShip, IAnimatron **retpAnimatron)

//	CreateShipDescAnimation
//
//	Creates animation describing the given ship

	{
	int i, j;
	int iDuration = 600;
	int iInterLineDelay = 1;
	int iDelay = 0;
	int x = m_rcIntroMain.left + (RectWidth(m_rcIntroMain) / 2) + (RectWidth(m_rcIntroMain) / 6);
	int y = m_rcIntroMain.bottom - RectHeight(m_rcIntroMain) / 3;

	//	Create sequencer to hold everything.

	CAniSequencer *pSeq = new CAniSequencer;

	//	Show the ship class

	CString sClassName = strToLower(pShip->GetNounPhrase());
	int cyClassName;
	int cxClassName = m_Fonts.SubTitle.MeasureText(sClassName, &cyClassName);
	int cySectionSpacing = cyClassName / 6;

	IAnimatron *pText;
	CAniText::Create(sClassName,
			CVector((Metric)x, (Metric)y),
			&m_Fonts.SubTitle,
			CG16bitFont::AlignCenter,
			m_Fonts.rgbTitleColor,
			&pText);
	pText->AnimateLinearFade(iDuration, 0, 30);
	pSeq->AddTrack(pText, 0);

	y += cyClassName + cySectionSpacing;
	iDelay += iInterLineDelay * 3;

	//	Weapons label

	CAniText::Create(CONSTLIT("WEAPONS:"),
			CVector((Metric)x - cyClassName / 4, (Metric)(y + m_Fonts.Medium.GetAscent() - m_Fonts.Small.GetAscent())),
			&m_Fonts.Small,
			CG16bitFont::AlignRight,
			m_Fonts.rgbLightTitleColor,
			&pText);
	pText->AnimateLinearFade(iDuration - iDelay, 15, 30);
	pSeq->AddTrack(pText, iDelay);

	//	Collect duplicate weapons

	struct SWeaponDesc
		{
		CString sWeaponName;
		int iCount;
		};

	TArray<SWeaponDesc> WeaponList;

	for (i = 0; i < pShip->GetDeviceCount(); i++)
		{
		CInstalledDevice *pDevice = pShip->GetDevice(i);
		if (pDevice->IsEmpty())
			continue;

		if (pDevice->GetCategory() == itemcatWeapon || pDevice->GetCategory() == itemcatLauncher)
			{
			CString sName = pDevice->GetClass()->GetItemType()->GetNounPhrase(nounActual | nounCapitalize);

			//	Look for the weapon in the list

			bool bFound = false;
			for (j = 0; j < WeaponList.GetCount() && !bFound; j++)
				if (strEquals(WeaponList[j].sWeaponName, sName))
					{
					WeaponList[j].iCount++;
					bFound = true;
					}

			//	Add if necessary

			if (!bFound)
				{
				SWeaponDesc *pWeapon = WeaponList.Insert();
				pWeapon->sWeaponName = sName;
				pWeapon->iCount = 1;
				}
			}
		}

	//	Output weapon list

	if (WeaponList.GetCount() == 0)
		{
		CAniText::Create(CONSTLIT("None"),
				CVector((Metric)x + cyClassName / 4, (Metric)y),
				&m_Fonts.Medium,
				0,
				m_Fonts.rgbTitleColor,
				&pText);
		pText->AnimateLinearFade(iDuration - iDelay, 15, 30);
		pSeq->AddTrack(pText, iDelay);

		y += m_Fonts.Medium.GetHeight();
		iDelay += iInterLineDelay;
		}
	else
		{
		for (i = 0; i < WeaponList.GetCount() && i < 6; i++)
			{
			CAniText::Create((WeaponList[i].iCount == 1 ? WeaponList[i].sWeaponName
						: strPatternSubst(CONSTLIT("%s (x%d)"), WeaponList[i].sWeaponName, WeaponList[i].iCount)),
					CVector((Metric)x + cyClassName / 4, (Metric)y),
					&m_Fonts.Medium,
					0,
					m_Fonts.rgbTitleColor,
					&pText);
			pText->AnimateLinearFade(iDuration - iDelay, 15, 30);
			pSeq->AddTrack(pText, iDelay);

			y += m_Fonts.Medium.GetHeight();
			iDelay += iInterLineDelay;
			}
		}

	//	Shields

	y += cySectionSpacing;

	CAniText::Create(CONSTLIT("SHIELDS:"),
			CVector((Metric)x - cyClassName / 4, (Metric)(y + m_Fonts.Medium.GetAscent() - m_Fonts.Small.GetAscent())),
			&m_Fonts.Small,
			CG16bitFont::AlignRight,
			m_Fonts.rgbLightTitleColor,
			&pText);
	pText->AnimateLinearFade(iDuration - iDelay, 15, 30);
	pSeq->AddTrack(pText, iDelay);

	CInstalledDevice *pShields = pShip->GetNamedDevice(devShields);

	CAniText::Create((pShields ? pShields->GetClass()->GetItemType()->GetNounPhrase(nounActual | nounCapitalize) : CONSTLIT("None")),
			CVector((Metric)x + cyClassName / 4, (Metric)y),
			&m_Fonts.Medium,
			0,
			m_Fonts.rgbTitleColor,
			&pText);
	pText->AnimateLinearFade(iDuration - iDelay, 15, 30);
	pSeq->AddTrack(pText, iDelay);

	y += m_Fonts.Medium.GetHeight();
	iDelay += iInterLineDelay;

	//	Armor

	y += cySectionSpacing;

	CAniText::Create(CONSTLIT("ARMOR:"),
			CVector((Metric)x - cyClassName / 4, (Metric)(y + m_Fonts.Medium.GetAscent() - m_Fonts.Small.GetAscent())),
			&m_Fonts.Small,
			CG16bitFont::AlignRight,
			m_Fonts.rgbLightTitleColor,
			&pText);
	pText->AnimateLinearFade(iDuration - iDelay, 15, 30);
	pSeq->AddTrack(pText, iDelay);

	int iCount = pShip->GetArmorSectionCount();
	CInstalledArmor *pArmor = (iCount > 0 ? pShip->GetArmorSection(0) : NULL);
	CString sArmor = (pArmor ? pArmor->GetClass()->GetItemType()->GetNounPhrase(nounActual | nounCapitalize | nounShort) : NULL_STR);

	CAniText::Create((pArmor ? strPatternSubst(CONSTLIT("%s (x%d)"), sArmor, iCount) : CONSTLIT("None")),
			CVector((Metric)x + cyClassName / 4, (Metric)y),
			&m_Fonts.Medium,
			0,
			m_Fonts.rgbTitleColor,
			&pText);
	pText->AnimateLinearFade(iDuration - iDelay, 15, 30);
	pSeq->AddTrack(pText, iDelay);

	y += m_Fonts.Medium.GetHeight();
	iDelay += iInterLineDelay;

	//	Done

	*retpAnimatron = pSeq;
	}

void CTranscendenceWnd::CreateTitleAnimation (IAnimatron **retpAnimatron)

//	CreateTitleAnimation
//
//	Creates opening titles animation

	{
	CAniSequencer *pSeq = new CAniSequencer;
	int iTime = 0;

	//	Figure out the position

	int xMidCenter = m_rcIntroMain.left + RectWidth(m_rcIntroMain) / 2;
	int yMidCenter = m_rcIntroMain.bottom - RectHeight(m_rcIntroMain) / 3;
	IAnimatron *pAnimation;

	//	Create Transcendence title text

	m_UIRes.CreateTitleAnimation(xMidCenter, yMidCenter, 150, &pAnimation);
	pSeq->AddTrack(pAnimation, iTime);

	//	Create version

	int y = m_rcIntroMain.bottom - (m_Fonts.MediumHeavyBold.GetHeight() + 2 * m_Fonts.Medium.GetHeight());
	CAniText::Create(m_sVersion,
			CVector(xMidCenter, y),
			&m_Fonts.MediumHeavyBold,
			CG16bitFont::AlignCenter,
			RGB_VERSION_COLOR,
			&pAnimation);
	pAnimation->AnimateLinearFade(150, 30, 30);
	pSeq->AddTrack(pAnimation, iTime);

	y += m_Fonts.MediumHeavyBold.GetHeight();

	//	Copyright

	CAniText::Create(m_sCopyright,
			CVector(xMidCenter, y),
			&m_Fonts.Medium,
			CG16bitFont::AlignCenter,
			RGB_COPYRIGHT_COLOR,
			&pAnimation);
	pAnimation->AnimateLinearFade(150, 30, 30);
	pSeq->AddTrack(pAnimation, iTime);

	iTime += 150;

	//	Done

	*retpAnimatron = pSeq;
	}

void CTranscendenceWnd::DestroyAllIntroShips(void)

//	DestroyAllIntroShips
//
//	Destroys all ships in the intro screen

{
	int i;

	CShip *pShip = g_pUniverse->GetPOV()->AsShip();
	if (pShip == NULL)
		return;

	CSystem *pSystem = pShip->GetSystem();
	for (i = 0; i < pSystem->GetObjectCount(); i++)
	{
		CSpaceObject *pObj = pSystem->GetObject(i);
		CShip *pShip;
		if (pObj
			&& pObj->CanAttack()
			&& (pShip = pObj->AsShip()))
		{
			pShip->Destroy(removedFromSystem, CDamageSource());
		}
	}
}

void CTranscendenceWnd::DestroyPOVIntroShips (void)

//	DestroyPOVIntroShips
//
//	Destroys all ships of the same class as the POV

	{
	int i;

	CShip *pShip = g_pUniverse->GetPOV()->AsShip();
	if (pShip == NULL)
		return;

	//	Destroy all ships of the current class

	CSystem *pSystem = pShip->GetSystem();
	CShipClass *pClassToDestroy = pShip->GetClass();
	TArray<CSpaceObject *> ShipsToDestroy;
	CSpaceObject *pOtherShip = NULL;
	for (i = 0; i < pSystem->GetObjectCount(); i++)
		{
		CSpaceObject *pObj = pSystem->GetObject(i);
		CShip *pShip;
		if (pObj 
				&& pObj->CanAttack()
				&& (pShip = pObj->AsShip()))
			{
			if (pShip->GetClass() == pClassToDestroy)
				ShipsToDestroy.Insert(pObj);
			else if (pOtherShip == NULL)
				pOtherShip = pObj;
			}
		}

	//	Destroy ships

	for (i = 0; i < ShipsToDestroy.GetCount(); i++)
		ShipsToDestroy[i]->Destroy(removedFromSystem, CDamageSource());
	}

void CTranscendenceWnd::OnAccountChanged (const CMultiverseModel &Multiverse)

//	OnAccountChanged
//
//	The user account has changed

	{
	switch (m_State)
		{
		case gsIntro:
			SetAccountControls(Multiverse);
			break;
		}
	}

void CTranscendenceWnd::OnCommandIntro (const CString &sCmd, void *pData)

//	OnCommandIntro
//
//	Handle commands from Reanimator, etc.

	{
	if (strEquals(sCmd, CMD_TOGGLE_MUSIC))
		{
		m_pTC->SetOptionBoolean(CGameSettings::noMusic, !m_pTC->GetOptionBoolean(CGameSettings::noMusic));
		SetMusicOption();
		}

	else if (strEquals(sCmd, CMD_TOGGLE_DEBUG))
		{
		m_pTC->GetModel().SetDebugMode(!m_pTC->GetModel().GetDebugMode());
		SetDebugOption();
		}

	else if (strEquals(sCmd, CMD_OPEN_NEWS))
		{
		if (!m_sNewsURL.IsBlank())
			sysOpenURL(m_sNewsURL);
		}
	}

void CTranscendenceWnd::OnDblClickIntro (int x, int y, DWORD dwFlags)

//	OnDblClickIntro
//
//	Handle WM_LBUTTONDBLCLK

	{
	//	See if the animator will handle it

	bool bCapture = false;
	if (m_Reanimator.HandleLButtonDblClick(x, y, dwFlags, &bCapture))
		{
		if (bCapture)
			::SetCapture(g_pHI->GetHWND());
		return;
		}

	if (m_ButtonBarDisplay.OnLButtonDoubleClick(x, y))
		return;
	}

void CTranscendenceWnd::OnLButtonDownIntro (int x, int y, DWORD dwFlags)

//	OnLButtonDownIntro
//
//	Handle WM_LBUTTONDOWN

	{
	//	See if the animator will handle it

	bool bCapture = false;
	if (m_Reanimator.HandleLButtonDown(x, y, dwFlags, &bCapture))
		{
		if (bCapture)
			::SetCapture(g_pHI->GetHWND());
		return;
		}

	if (m_ButtonBarDisplay.OnLButtonDown(x, y))
		return;
	}

void CTranscendenceWnd::OnLButtonUpIntro (int x, int y, DWORD dwFlags)

//	OnLButtonUpIntro
//
//	Handle WM_LBUTTONUP

	{
	if (::GetCapture() == g_pHI->GetHWND())
		::ReleaseCapture();

	//	See if the animator will handle it

	if (m_Reanimator.HandleLButtonUp(x, y, dwFlags))
		return;
	}

void CTranscendenceWnd::OnMouseMoveIntro (int x, int y, DWORD dwFlags)

//	OnMouseMoveIntro
//
//	Handle WM_MOUSEMOVE

	{
	//	See if the animator will handle it

	if (m_Reanimator.HandleMouseMove(x, y, dwFlags))
		return;

	m_ButtonBarDisplay.OnMouseMove(x, y);
	}

void CTranscendenceWnd::PaintDlgButton (const RECT &rcRect, const CString &sText)

//	PaintDlgButton
//
//	Paint button

	{
	CG32bitImage &TheScreen = g_pHI->GetScreen();

	TheScreen.Fill(rcRect.left, 
			rcRect.top,
			RectWidth(rcRect),
			RectHeight(rcRect),
			RGB_DIALOG_BUTTON_BACKGROUND);

	int cy;
	int cx = m_Fonts.MediumHeavyBold.MeasureText(sText, &cy);

	TheScreen.DrawText(rcRect.left + (RectWidth(rcRect) - cx) / 2,
			rcRect.top + (RectHeight(rcRect) - cy) / 2,
			m_Fonts.MediumHeavyBold,
			RGB_DIALOG_BUTTON_TEXT,
			sText);
	}

void CTranscendenceWnd::SetAccountControls (const CMultiverseModel &Multiverse)

//	SetAccountControls
//
//	Sets the user account controls

	{
	const CVisualPalette &VI = g_pHI->GetVisuals();
	const CG16bitFont &MediumFont = VI.GetFont(fontMedium);
	const CG16bitFont &SubTitleFont = VI.GetFont(fontSubTitle);

	//	Get the account state

	CString sUsername;
	CString sStatus;
	CMultiverseModel::EOnlineStates iState = Multiverse.GetOnlineState(&sUsername, &sStatus);
	CG32bitPixel rgbUsernameColor = VI.GetColor(colorTextDialogLabel);

	//	Metrics

	int cxText = Max(SubTitleFont.MeasureText(sUsername), MediumFont.MeasureText(sStatus));

	//	Compute metrics

	RECT rcRect;
	VI.GetWidescreenRect(&rcRect);

	//	Delete any existing controls

	m_Reanimator.DeleteElement(ID_ACCOUNT_CONTROLS);

	//	Create a sequencer to hold all the controls. We will be a child of the
	//	player bar animation, so we are relative to that.

	CAniSequencer *pRoot;
	CAniSequencer::Create(CVector(0, 0), &pRoot);
	pRoot->SetID(ID_ACCOUNT_CONTROLS);

	//	The user icon is centered

	CAniRoundedRect *pIcon = new CAniRoundedRect;
	pIcon->SetPropertyVector(PROP_POSITION, CVector(0, (TITLE_BAR_HEIGHT - ICON_HEIGHT) / 2));
	pIcon->SetPropertyVector(PROP_SCALE, CVector(ICON_HEIGHT, ICON_WIDTH));
	pIcon->SetPropertyColor(PROP_COLOR, CG32bitPixel(128, 128, 128));
	pIcon->SetPropertyOpacity(PROP_OPACITY, 255);
	pIcon->SetPropertyInteger(PROP_UL_RADIUS, ICON_CORNER_RADIUS);
	pIcon->SetPropertyInteger(PROP_UR_RADIUS, ICON_CORNER_RADIUS);
	pIcon->SetPropertyInteger(PROP_LL_RADIUS, ICON_CORNER_RADIUS);
	pIcon->SetPropertyInteger(PROP_LR_RADIUS, ICON_CORNER_RADIUS);

	pRoot->AddTrack(pIcon, 0);

	//	The user name baseline is centered.

	int y = (TITLE_BAR_HEIGHT / 2) - SubTitleFont.GetAscent();

	//	Create a hot spot over the entire text region (so that the user can 
	//	click on the username to sign in).

	if (iState == CMultiverseModel::stateNoUser || iState == CMultiverseModel::stateOffline)
		{
		IAnimatron *pButton;
		VI.CreateHiddenButton(pRoot, CMD_ACCOUNT,
				ICON_WIDTH + (PADDING_LEFT / 2),
				y - (PADDING_LEFT / 2),
				cxText + PADDING_LEFT,
				SubTitleFont.GetHeight() + 2 * MediumFont.GetHeight() + PADDING_LEFT,
				0,
				&pButton);
		pButton->AddListener(EVENT_ON_CLICK, m_pIntroSession, CMD_ACCOUNT);
		}

	//	Username

	IAnimatron *pName = new CAniText;
	pName->SetPropertyVector(PROP_POSITION, CVector(ICON_WIDTH + PADDING_LEFT, y));
	pName->SetPropertyVector(PROP_SCALE, CVector(RectWidth(rcRect), RectHeight(rcRect)));
	pName->SetPropertyColor(PROP_COLOR, rgbUsernameColor);
	pName->SetPropertyFont(PROP_FONT, &SubTitleFont);
	pName->SetPropertyString(PROP_TEXT, sUsername);

	pRoot->AddTrack(pName, 0);
	y += SubTitleFont.GetHeight();

	//	Status

	IAnimatron *pStatus = new CAniText;
	pStatus->SetPropertyVector(PROP_POSITION, CVector(ICON_WIDTH + PADDING_LEFT, y));
	pStatus->SetPropertyVector(PROP_SCALE, CVector(RectWidth(rcRect), RectHeight(rcRect)));
	pStatus->SetPropertyColor(PROP_COLOR, VI.GetColor(colorTextDialogLabel));
	pStatus->SetPropertyFont(PROP_FONT, &MediumFont);
	pStatus->SetPropertyString(PROP_TEXT, sStatus);

	pRoot->AddTrack(pStatus, 0);
	y += MediumFont.GetHeight();

	//	If the user is signed in, add buttons to edit account and sign out.

#ifndef STEAM_BUILD
	if (iState == CMultiverseModel::stateOnline)
		{
		int x = ICON_WIDTH + PADDING_LEFT;

		//	Edit account

		IAnimatron *pButton;
		int cxLink;
#ifdef EDIT_ACCOUNT
		VI.CreateLink(pRoot, CMD_ACCOUNT_EDIT, x, y, CONSTLIT("edit account"), CVisualPalette::OPTION_LINK_MEDIUM_FONT, &pButton, &cxLink);
		pButton->AddListener(EVENT_ON_CLICK, m_pIntroSession, CMD_ACCOUNT_EDIT);
#else
		VI.CreateLink(pRoot, CMD_CHANGE_PASSWORD, x, y, CONSTLIT("change password"), CVisualPalette::OPTION_LINK_MEDIUM_FONT, &pButton, &cxLink);
		pButton->AddListener(EVENT_ON_CLICK, m_pIntroSession, CMD_CHANGE_PASSWORD);
#endif

		x += cxLink;

		//	Separator

		IAnimatron *pSep = new CAniText;
		pSep->SetPropertyVector(PROP_POSITION, CVector(x, y));
		pSep->SetPropertyVector(PROP_SCALE, CVector(SMALL_LINK_SPACING, RectHeight(rcRect)));
		pSep->SetPropertyColor(PROP_COLOR, VI.GetColor(colorTextDialogLabel));
		pSep->SetPropertyFont(PROP_FONT, &MediumFont);
		pSep->SetPropertyString(PROP_TEXT, CONSTLIT("•"));
		pSep->SetPropertyString(PROP_TEXT_ALIGN_HORZ, ALIGN_CENTER);

		pRoot->AddTrack(pSep, 0);
		x += SMALL_LINK_SPACING;

		//	Sign out

		VI.CreateLink(pRoot, CMD_SIGN_OUT, x, y, CONSTLIT("sign out"), CVisualPalette::OPTION_LINK_MEDIUM_FONT, &pButton, &cxLink);
		pButton->AddListener(EVENT_ON_CLICK, m_pIntroSession, CMD_SIGN_OUT);

		x += cxLink;
		}
#endif

	//	Add it to the existing sequencer

	IAnimatron *pPlayerBar = m_Reanimator.GetPerformance(ID_PLAYER_BAR_PERFORMANCE);
	if (pPlayerBar == NULL)
		return;

	CAniSequencer *pSeq = (CAniSequencer *)pPlayerBar;
	pSeq->AddTrack(pRoot, 0);
	}

void CTranscendenceWnd::SetDebugOption (void)

//	SetDebugOption
//
//	Sets the current state of debug mode

	{
	const CVisualPalette &VI = g_pHI->GetVisuals();

	IAnimatron *pButton = m_Reanimator.GetElement(CMD_TOGGLE_DEBUG);
	if (pButton == NULL)
		return;

	IAnimatron *pStyle = pButton->GetStyle(STYLE_IMAGE);
	if (pStyle == NULL)
		return;

	bool bDebug = m_pTC->GetModel().GetDebugMode();
	if (bDebug)
		{
		pStyle->SetFillMethod(new CAniImageFill(&VI.GetImage(imageDebugIcon), false));
		pButton->SetPropertyString(PROP_TEXT, CONSTLIT("Debug"));
		}
	else
		{
		pStyle->SetFillMethod(new CAniImageFill(&VI.GetImage(imagePlayIcon), false));
		pButton->SetPropertyString(PROP_TEXT, CONSTLIT("Play"));
		}
	}

void CTranscendenceWnd::SetMusicOption (void)

//	SetMusicOption
//
//	Sets the current state of the music toggle icon

	{
	const CVisualPalette &VI = g_pHI->GetVisuals();

	IAnimatron *pButton = m_Reanimator.GetElement(CMD_TOGGLE_MUSIC);
	if (pButton == NULL)
		return;

	IAnimatron *pStyle = pButton->GetStyle(STYLE_IMAGE);
	if (pStyle == NULL)
		return;

	bool bMusicOn = !m_pTC->GetOptionBoolean(CGameSettings::noMusic);
	if (bMusicOn)
		{
		pStyle->SetFillMethod(new CAniImageFill(&VI.GetImage(imageMusicIconOn), false));
		pButton->SetPropertyString(PROP_TEXT, CONSTLIT("Music On"));
		}
	else
		{
		pStyle->SetFillMethod(new CAniImageFill(&VI.GetImage(imageMusicIconOff), false));
		pButton->SetPropertyString(PROP_TEXT, CONSTLIT("Music Off"));
		}
	}

ALERROR CTranscendenceWnd::StartIntro (CIntroSession *pThis)

//	StartIntro
//
//	Start introduction

	{
	//	Temporarily set this member variable because some of our functions are
	//	implemented in CTranscendenceWnd. Over time, all those functions should
	//	be moved to CIntroSession.

	m_pIntroSession = pThis;

	//	Init

	ClearDebugLines();

	//	Use widescreen topology

	int cyBarHeight = Max(128, (g_cyScreen - INTRO_DISPLAY_HEIGHT) / 2);
	m_rcIntroTop.top = 0;
	m_rcIntroTop.left = 0;
	m_rcIntroTop.bottom = cyBarHeight;
	m_rcIntroTop.right = g_cxScreen;

	m_rcIntroMain.top = cyBarHeight;
	m_rcIntroMain.left = 0;
	m_rcIntroMain.bottom = g_cyScreen - cyBarHeight;
	m_rcIntroMain.right = g_cxScreen;

	m_rcIntroBottom.top = g_cyScreen - cyBarHeight;
	m_rcIntroBottom.left = 0;
	m_rcIntroBottom.bottom = g_cyScreen;
	m_rcIntroBottom.right = g_cxScreen;

	//	Create the buttons

	m_ButtonBar.Init();

	m_bSavedGame = false;
	m_ButtonBar.AddButton(CMD_CONTINUE_OLD_GAME,
			CONSTLIT("Load Game"),
			CONSTLIT("Continue a Previous Game"),
			CONSTLIT("C"),
			0,
			CButtonBarData::alignLeft);

	m_ButtonBar.AddButton(CMD_START_NEW_GAME,
			CONSTLIT("New Game"),
			CONSTLIT("Begin a New Game"),
			CONSTLIT("N"),
			1,
			CButtonBarData::alignCenter);

	m_ButtonBar.AddButton(CMD_QUIT_GAME,
			CONSTLIT("Quit"),
			CONSTLIT("Exit Transcendence"),
			CONSTLIT("Q"),
			2,
			CButtonBarData::alignRight);

	m_ButtonBarDisplay.SetFontTable(&m_Fonts);
	m_ButtonBarDisplay.Init(this, &m_ButtonBar, m_rcIntroBottom);

	//	Create the credits performance

	IAnimatron *pAnimation;
	CreateCreditsAnimation(&pAnimation);
	m_dwCreditsPerformance = m_Reanimator.AddPerformance(pAnimation, ID_CREDITS_PERFORMANCE);

	//	Create the titles performance

	CreateTitleAnimation(&pAnimation);
	m_dwTitlesPerformance = m_Reanimator.AddPerformance(pAnimation, ID_TITLES_PERFORMANCE);

	//	Create the top bar

	CreatePlayerBarAnimation(&pAnimation);
	m_dwPlayerBarPerformance = m_Reanimator.AddPerformance(pAnimation, ID_PLAYER_BAR_PERFORMANCE);
	m_Reanimator.StartPerformance(m_dwPlayerBarPerformance);

	SetMusicOption();
	SetDebugOption();
	SetAccountControls(m_pTC->GetMultiverse());

	//	Start

	m_State = gsIntro;

	return NOERROR;
	}

void CTranscendenceWnd::StopIntro (void)

//	StopIntro
//
//	Stop introduction screen

	{
	ASSERT(m_State == gsIntro);

	m_ButtonBarDisplay.CleanUp();
	m_ButtonBar.CleanUp();

	//	Clean up reanimator (so that we don't keep memory allocated)

	if (m_dwCreditsPerformance)
		m_Reanimator.DeletePerformance(m_dwCreditsPerformance);

	if (m_dwTitlesPerformance)
		m_Reanimator.DeletePerformance(m_dwTitlesPerformance);

	if (m_dwPlayerBarPerformance)
		m_Reanimator.DeletePerformance(m_dwPlayerBarPerformance);

	//	Destroy system

	g_pUniverse->DestroySystem(m_pIntroSystem);
	m_pIntroSystem = NULL;

	//	Enable sound

	g_pUniverse->SetSound(true);

	//	Hide cursor

	ShowCursor(false);
	m_pIntroSession = NULL;
	}
