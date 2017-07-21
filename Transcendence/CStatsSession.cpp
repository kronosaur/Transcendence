//	CStatsSession.cpp
//
//	CStatsSession class

#include "PreComp.h"
#include "Transcendence.h"

#define ID_GAME_STAT_PERFORMANCE				CONSTLIT("gameStatPerformance")
#define ID_GAME_STAT_COPY_FLASH					CONSTLIT("gameStatCopyFlash")
#define ID_CTRL_TITLE							CONSTLIT("title")

#define CMD_OK_SESSION							CONSTLIT("cmdOKSession")
#define CMD_SESSION_STATS_DONE					CONSTLIT("sessionStatsDone")
#define CMD_UI_SHOW_HELP						CONSTLIT("uiShowHelp")

#define PROP_OPACITY							CONSTLIT("opacity")

#define STR_HELP_TITLE							CONSTLIT("Game Statistics")
#define STR_HELP_LINE1							CONSTLIT("[Arrows] to scroll")
#define STR_HELP_LINE2							CONSTLIT("[Ctrl+C] to copy to clipboard")

const CG32bitPixel RGB_IMAGE_BACKGROUND =		CG32bitPixel(15, 12, 10);

void CStatsSession::CreateCopyAnimation (const RECT &rcRect, int iDuration, IAnimatron **retpAni)

//	CreateCopyAnimation
//
//	Flashes a rect to indicate that the information was copied

	{
	const CVisualPalette &VI = m_HI.GetVisuals();

	IAnimatron *pAni;

	CAniRect::Create(CVector(rcRect.left, rcRect.top), 
			CVector(RectWidth(rcRect), RectHeight(rcRect)),
			VI.GetColor(colorTextFade),
			255,
			&pAni);

	//	Fade it

	CLinearFade *pFader = new CLinearFade;
	pFader->SetParams(iDuration, 0, iDuration, 128);
	pAni->AnimateProperty(PROP_OPACITY, pFader, 0);

	//	Done

	*retpAni = pAni;
	}

CString CStatsSession::GetTaskProgressText (void)

//	GetTaskProgressText
//
//	Returns text to paint on the task progress line

	{
	CString sText;

	m_HI.GetBackgroundProcessor().GetProgress(&sText);
	if (sText.IsBlank())
		m_HI.GetBackgroundProcessor().GetLastResult(&sText);

	return sText;
	}

void CStatsSession::OnCleanUp (void)

//	OnCleanup

	{
	}

ALERROR CStatsSession::OnCommand (const CString &sCmd, void *pData)

//	OnCommand
//
//	Handle a command

	{
	if (strEquals(sCmd, CMD_OK_SESSION))
		m_HI.HICommand(CMD_SESSION_STATS_DONE);

	return NOERROR;
	}

ALERROR CStatsSession::OnInit (CString *retsError)

//	OnInit

	{
	ALERROR error;
	const CVisualPalette &VI = m_HI.GetVisuals();

	//	Load a JPEG of the background image

	HBITMAP hDIB;
	if (error = JPEGLoadFromResource(NULL,
			MAKEINTRESOURCE(IDR_GAME_STATS_SCREEN),
			JPEG_LFR_DIB, 
			NULL,
			&hDIB))
		return error;

	bool bSuccess = m_BackgroundImage.CreateFromBitmap(hDIB);
	::DeleteObject(hDIB);
	if (!bSuccess)
		return ERR_FAIL;

	//	Figure out where the stats will go

	RECT rcCenter;
	RECT rcFull;
	VI.GetWidescreenRect(&rcCenter, &rcFull);
	const CG16bitFont &MediumFont = VI.GetFont(fontMedium);
	const CG16bitFont &HeaderFont = VI.GetFont(fontHeader);

	m_rcStats = rcCenter;
	m_rcStats.left = rcCenter.left + RectWidth(rcCenter) / 2;
	m_rcStats.right = rcFull.right - 4 * VI.GetFont(fontHeader).GetHeight();

	//	Create stats animation

	IAnimatron *pAni;
	CreateGameStatsAnimation(m_Stats, m_rcStats, rcCenter.left + 3 * RectWidth(rcCenter) / 4, &pAni);
	SelectGameStat(pAni, 1, RectWidth(m_rcStats));
	StartPerformance(pAni, ID_GAME_STAT_PERFORMANCE, CReanimator::SPR_FLAG_DELETE_WHEN_DONE);

	//	Figure out where task status will go

	m_rcTaskProgress.right = rcCenter.right;
	m_rcTaskProgress.left = m_rcTaskProgress.right - 80 * HeaderFont.GetAverageWidth();
	m_rcTaskProgress.top = rcCenter.bottom + MediumFont.GetHeight();
	m_rcTaskProgress.bottom = m_rcTaskProgress.top + HeaderFont.GetHeight();

	//	OK button

	CUIHelper Helper(m_HI);
	IAnimatron *pTitle;
	Helper.CreateSessionTitle(this, 
			m_Service, 
			NULL_STR, 
			NULL, 
			CUIHelper::OPTION_SESSION_NO_HEADER 
				| CUIHelper::OPTION_SESSION_NO_CANCEL_BUTTON
				| CUIHelper::OPTION_SESSION_OK_BUTTON, 
			&pTitle);
	StartPerformance(pTitle, ID_CTRL_TITLE, CReanimator::SPR_FLAG_DELETE_WHEN_DONE);

	return NOERROR;
	}

void CStatsSession::OnKeyDown (int iVirtKey, DWORD dwKeyData)

//	OnKeyDown

	{
	switch (iVirtKey)
		{
		case 'C':
			if (uiIsControlDown())
				{
				::CopyGameStatsToClipboard(m_HI.GetHWND(), m_Stats);

				IAnimatron *pAni;
				CreateCopyAnimation(m_rcStats, 2, &pAni);
				StartPerformance(pAni, ID_GAME_STAT_COPY_FLASH, CReanimator::SPR_FLAG_DELETE_WHEN_DONE);
				}
			break;

		case VK_CONTROL:
			break;

		case VK_F1:
			m_HI.HICommand(CMD_UI_SHOW_HELP);
			break;

		case VK_DOWN:
		case VK_RIGHT:
			SetSelection(GAME_STAT_POSITION_NEXT);
			break;

		case VK_END:
			SetSelection(GAME_STAT_POSITION_END);
			break;

		case VK_HOME:
			SetSelection(GAME_STAT_POSITION_HOME);
			break;

		case VK_LEFT:
		case VK_UP:
			SetSelection(GAME_STAT_POSITION_PREV);
			break;

		case VK_NEXT:
			SetSelection(GAME_STAT_POSITION_NEXT_PAGE);
			break;

		case VK_PRIOR:
			SetSelection(GAME_STAT_POSITION_PREV_PAGE);
			break;

		default:
			m_HI.HICommand(CMD_SESSION_STATS_DONE);
			break;
		}
	}

void CStatsSession::OnLButtonDown (int x, int y, DWORD dwFlags, bool *retbCapture)

//	OnLButtonDown

	{
	//	Done

	m_HI.HICommand(CMD_SESSION_STATS_DONE);
	}

void CStatsSession::OnMouseWheel (int iDelta, int x, int y, DWORD dwFlags)

//	OnMouseWheel
//
//	Handle mouse wheel

	{
	if (iDelta > 0)
		SetSelection(GAME_STAT_POSITION_PREV);
	else if (iDelta < 0)
		SetSelection(GAME_STAT_POSITION_NEXT);
	}

void CStatsSession::OnPaint (CG32bitImage &Screen, const RECT &rcInvalid)

//	OnPaint

	{
	const CVisualPalette &VI = m_HI.GetVisuals();
	const CG16bitFont &HeaderFont = VI.GetFont(fontHeader);
	const CG16bitFont &MediumFont = VI.GetFont(fontMedium);

	RECT rcCenter;
	VI.DrawSessionBackground(Screen, m_BackgroundImage, RGB_IMAGE_BACKGROUND, 0, &rcCenter);

	//	Paint some help text

	int x = rcCenter.left;
	int y = rcCenter.bottom + MediumFont.GetHeight();

	Screen.DrawText(x, y, HeaderFont, VI.GetColor(colorTextHighlight), STR_HELP_TITLE);
	y += HeaderFont.GetHeight();
	
	Screen.DrawText(x, y, MediumFont, VI.GetColor(colorTextFade), STR_HELP_LINE1);
	y += MediumFont.GetHeight();

	Screen.DrawText(x, y, MediumFont, VI.GetColor(colorTextFade), STR_HELP_LINE2);
	y += MediumFont.GetHeight();

	//	Paint task progress, if necessary

	if (m_dwFlags & SHOW_TASK_PROGRESS)
		{
		Screen.DrawText(m_rcTaskProgress.right,
				m_rcTaskProgress.top,
				HeaderFont,
				VI.GetColor(colorTextFade),
				GetTaskProgressText(),
				CG16bitFont::AlignRight);
		}
	}

void CStatsSession::OnReportHardCrash (CString *retsMessage)

//	OnReportHardCrash

	{
	*retsMessage = CONSTLIT("session: CStatsSession\r\n");
	}

void CStatsSession::OnUpdate (bool bTopMost)

//	OnUpdate

	{
	}

void CStatsSession::SetSelection (int iPos)

//	SetSelection
//
//	Sets the selection position

	{
	IAnimatron *pAni = GetPerformance(ID_GAME_STAT_PERFORMANCE);
	if (pAni == NULL)
		return;

	//	Set

	SelectGameStat(pAni, iPos, RectWidth(m_rcStats));

	//	Restart animation

	StartPerformance(ID_GAME_STAT_PERFORMANCE, CReanimator::SPR_FLAG_DELETE_WHEN_DONE);
	}
