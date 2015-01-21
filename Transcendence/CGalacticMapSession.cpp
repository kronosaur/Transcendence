//	CGalacticMapSession.cpp
//
//	CGalacticMapSession class

#include "PreComp.h"
#include "Transcendence.h"

const int MAX_HEIGHT =							1024;
const int BORDER_HEIGHT =						96;
const int SCROLL_STEP =							120;

#define STR_HELP_LINE1							CONSTLIT("[Arrows] to scroll")
#define STR_HELP_LINE2							CONSTLIT("[Esc] to exit")

static int SCALE_LEVELS[] =
	{	25, 50, 100, 200, 400 };

const int SCALE_LEVELS_COUNT = (sizeof(SCALE_LEVELS) / sizeof(SCALE_LEVELS[0]));

int CGalacticMapSession::GetScaleIndex (int iScale)

//	GetScaleIndex
//
//	Returns the scale index for the given scale

	{
	int i;

	for (i = 0; i < SCALE_LEVELS_COUNT; i++)
		if (iScale <= SCALE_LEVELS[i])
			return i;

	return SCALE_LEVELS[SCALE_LEVELS_COUNT - 1];
	}

int CGalacticMapSession::GetScale (int iScaleIndex)

//	GetScale
//
//	Returns the scale for the given index

	{
	return SCALE_LEVELS[iScaleIndex];
	}

void CGalacticMapSession::OnCleanUp (void)

//	OnCleanUp
//
//	Clean up

	{
	if (m_pPainter)
		{
		delete m_pPainter;
		m_pPainter = NULL;
		}
	}

ALERROR CGalacticMapSession::OnInit (CString *retsError)

//	OnInit
//
//	Initialize

	{
	const CVisualPalette &VI = m_HI.GetVisuals();

	//	Get the map

	CTopologyNode *pNode = g_pUniverse->GetCurrentTopologyNode();
	if (pNode == NULL)
		return NOERROR;

	m_pMap = pNode->GetDisplayPos(&m_xCenter, &m_yCenter);
	if (m_pMap == NULL)
		return NOERROR;

	//	Get basic map information

	m_sMapName = m_pMap->GetName();

	int iMinScale;
	int iMaxScale;
	m_pMap->GetScale(&m_iScale, &iMinScale, &iMaxScale);
	m_iScale = GetScale(GetScaleIndex(m_iScale));
	m_iMinScaleIndex = GetScaleIndex(iMinScale);
	m_iMaxScaleIndex = GetScaleIndex(iMaxScale);

	//	Create a painter

	m_pPainter = new CGalacticMapPainter(m_HI.GetVisuals(), m_pMap);

	//	Compute some rects

	RECT rcCenter;
	VI.GetWidescreenRect(m_HI.GetScreen(), &rcCenter);

	int cxWidth = m_HI.GetScreen().GetWidth();
	int cyTotalHeight = Min(MAX_HEIGHT + 2 * BORDER_HEIGHT, m_HI.GetScreen().GetHeight());
	int cyHeight = cyTotalHeight - (2 * BORDER_HEIGHT);
	m_rcView.left = 0;
	m_rcView.top = (m_HI.GetScreen().GetHeight() - cyHeight) / 2;
	m_rcView.right = m_rcView.left + cxWidth;
	m_rcView.bottom = m_rcView.top + cyHeight;

	m_rcHelp.left = rcCenter.left;
	m_rcHelp.top = m_rcView.bottom + VI.GetFont(fontMedium).GetHeight();

	//	Adjust the map position

	m_pPainter->AdjustCenter(m_rcView, m_xCenter, m_yCenter, m_iScale, &m_xCenter, &m_yCenter);

	//	Initialize animation

	m_iTargetScaleIndex = GetScaleIndex(m_iScale);
	m_xTargetCenter = m_xCenter;
	m_yTargetCenter = m_yCenter;

	return NOERROR;
	}

void CGalacticMapSession::OnKeyDown (int iVirtKey, DWORD dwKeyData)

//	OnKeyDown
//
//	KeyDown

	{
	switch (iVirtKey)
		{
		case 'C':
			if (uiIsControlDown() && m_pPainter)
				::CopyGalacticMapToClipboard(m_HI.GetHWND(), m_pPainter);
			break;

		case VK_ADD:
		case VK_OEM_PLUS:
			m_iTargetScaleIndex = Min(m_iTargetScaleIndex + 1, m_iMaxScaleIndex);
			m_pPainter->AdjustCenter(m_rcView, m_xTargetCenter, m_yTargetCenter, GetScale(m_iTargetScaleIndex), &m_xTargetCenter, &m_yTargetCenter);
			break;

		case VK_CONTROL:
			break;

		case VK_DOWN:
			m_pPainter->AdjustCenter(m_rcView, m_xCenter, m_yCenter - (100 * SCROLL_STEP / GetScale(m_iTargetScaleIndex)), GetScale(m_iTargetScaleIndex), &m_xTargetCenter, &m_yTargetCenter);
			break;

		case VK_HOME:
		case VK_END:
			{
			CTopologyNode *pNode = g_pUniverse->GetCurrentTopologyNode();
			if (pNode)
				{
				pNode->GetDisplayPos(&m_xTargetCenter, &m_yTargetCenter);
				m_pPainter->AdjustCenter(m_rcView, m_xTargetCenter, m_yTargetCenter, GetScale(m_iTargetScaleIndex), &m_xTargetCenter, &m_yTargetCenter);
				}
			break;
			}

		case VK_LEFT:
			m_pPainter->AdjustCenter(m_rcView, m_xCenter - (100 * SCROLL_STEP / GetScale(m_iTargetScaleIndex)), m_yCenter, GetScale(m_iTargetScaleIndex), &m_xTargetCenter, &m_yTargetCenter);
			break;

		case VK_NEXT:
			m_pPainter->AdjustCenter(m_rcView, m_xCenter, m_yCenter - (300 * SCROLL_STEP / GetScale(m_iTargetScaleIndex)), GetScale(m_iTargetScaleIndex), &m_xTargetCenter, &m_yTargetCenter);
			break;

		case VK_PRIOR:
			m_pPainter->AdjustCenter(m_rcView, m_xCenter, m_yCenter + (300 * SCROLL_STEP / GetScale(m_iTargetScaleIndex)), GetScale(m_iTargetScaleIndex), &m_xTargetCenter, &m_yTargetCenter);
			break;

		case VK_RIGHT:
			m_pPainter->AdjustCenter(m_rcView, m_xCenter + (100 * SCROLL_STEP / GetScale(m_iTargetScaleIndex)), m_yCenter, GetScale(m_iTargetScaleIndex), &m_xTargetCenter, &m_yTargetCenter);
			break;

		case VK_SUBTRACT:
		case VK_OEM_MINUS:
			m_iTargetScaleIndex = Max(m_iMinScaleIndex, m_iTargetScaleIndex - 1);
			m_pPainter->AdjustCenter(m_rcView, m_xTargetCenter, m_yTargetCenter, GetScale(m_iTargetScaleIndex), &m_xTargetCenter, &m_yTargetCenter);
			break;

		case VK_UP:
			m_pPainter->AdjustCenter(m_rcView, m_xCenter, m_yCenter + (100 * SCROLL_STEP / GetScale(m_iTargetScaleIndex)), GetScale(m_iTargetScaleIndex), &m_xTargetCenter, &m_yTargetCenter);
			break;

		//	Done

		default:
			m_HI.ClosePopupSession();
			break;
		}
	}

void CGalacticMapSession::OnLButtonDown (int x, int y, DWORD dwFlags)

//	OnLButtonDown
//
//	LButtonDown

	{
	//	Done

	m_HI.ClosePopupSession();
	}

void CGalacticMapSession::OnPaint (CG16bitImage &Screen, const RECT &rcInvalid)

//	OnPaint
//
//	Paint

	{
	int cxScreen = Screen.GetWidth();
	int cyScreen = Screen.GetHeight();

	const CVisualPalette &VI = m_HI.GetVisuals();
	WORD wBackgroundColor = VI.GetColor(colorAreaDeep);
	WORD wLineColor = VI.GetColor(colorLineFrame);
	const CG16bitFont &HeaderFont = VI.GetFont(fontHeader);
	const CG16bitFont &MediumFont = VI.GetFont(fontMedium);

	//	Paint the actual map

	if (m_pPainter)
		{
		m_pPainter->Paint(Screen, m_rcView, m_xCenter, m_yCenter, m_iScale);

		//	Paint the ship

		CSpaceObject *pPlayer = g_pUniverse->GetPlayerShip();
		if (pPlayer)
			{
			int xPos, yPos;
			g_pUniverse->GetCurrentSystem()->GetTopology()->GetDisplayPos(&xPos, &yPos);

			int xShip, yShip;
			m_pPainter->GetPos(xPos, yPos, m_rcView, m_xCenter, m_yCenter, m_iScale, &xShip, &yShip);

			pPlayer->PaintMap(CMapViewportCtx(), Screen, xShip, yShip);
			}
		}

	//	Paint frame

	Screen.Fill(0, 0, cxScreen, m_rcView.top, wBackgroundColor);
	Screen.Fill(0, m_rcView.bottom, cxScreen, cyScreen - m_rcView.bottom, wBackgroundColor);
	Screen.FillLine(0, m_rcView.top, cxScreen, wLineColor);
	Screen.FillLine(0, m_rcView.bottom, cxScreen, wLineColor);

	//	Paint some help text

	int x = m_rcHelp.left;
	int y = m_rcHelp.top;

	Screen.DrawText(x, y, HeaderFont, VI.GetColor(colorTextHighlight), m_sMapName);
	y += HeaderFont.GetHeight();
	
	Screen.DrawText(x, y, MediumFont, VI.GetColor(colorTextFade), STR_HELP_LINE1);
	y += MediumFont.GetHeight();

	Screen.DrawText(x, y, MediumFont, VI.GetColor(colorTextFade), STR_HELP_LINE2);
	y += MediumFont.GetHeight();
	}

void CGalacticMapSession::OnReportHardCrash (CString *retsMessage)

//	OnReportHardCrash
//
//	Describe current state

	{
	*retsMessage = CONSTLIT("session: CGalacticMapSession\r\n");
	}

void CGalacticMapSession::OnUpdate (bool bTopMost)

//	OnUpdate
//
//	Update

	{
	int iTargetScale = GetScale(m_iTargetScaleIndex);
	if (iTargetScale != m_iScale)
		{
		int iDiff = iTargetScale - m_iScale;
		m_iScale = (Absolute(iDiff) > 1 ? m_iScale + (iDiff / 2) : iTargetScale);
		HIInvalidate();
		}

	if (m_xCenter != m_xTargetCenter || m_yCenter != m_yTargetCenter)
		{
		int xDiff = m_xTargetCenter - m_xCenter;
		int yDiff = m_yTargetCenter - m_yCenter;
		
		m_xCenter = (Absolute(xDiff) > 1 ? m_xCenter + (xDiff / 2) : m_xTargetCenter);
		m_yCenter = (Absolute(yDiff) > 1 ? m_yCenter + (yDiff / 2) : m_yTargetCenter);

		HIInvalidate();
		}
	}

void CopyGalacticMapToClipboard (HWND hWnd, CGalacticMapPainter *pPainter)
	{
	//	Compute the size of the map

	RECT rcView;
	rcView.left = 0;
	rcView.top = 0;
	rcView.right = pPainter->GetWidth();
	rcView.bottom = pPainter->GetHeight();

	//	Create a bitmap of the appropriate size

	CG16bitImage FullMap;
	FullMap.CreateBlank(RectWidth(rcView), RectHeight(rcView), false);

	//	Paint

	pPainter->Paint(FullMap, rcView, 0, 0, 100);

	//	Copy to cliboard

	FullMap.CopyToClipboard();
	}
