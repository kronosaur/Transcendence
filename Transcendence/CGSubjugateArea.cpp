//	CGSubjugateArea.cpp
//
//	CGSubjugateArea class
//	Copyright (c) 2016 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"
#include "Transcendence.h"

#define STR_DEPLOY							CONSTLIT("Deploy")

const int COUNTERMEASURES_COUNT =			6;
const int COUNTERMEASURES_INNER_RADIUS =	94;
const int COUNTERMEASURES_ARC_ANGLE =		(360 / COUNTERMEASURES_COUNT);
const int COUNTERMEASURE_SPACING =			12;
const int COUNTERMEASURE_WIDTH =			80;

const int CURSOR_OFFSET_Y =					16;

const int DAIMON_BORDER_RADIUS =			6;
const int DAIMON_COUNT =					6;
const int DAIMON_HEIGHT =					96;
const int DAIMON_WIDTH =					96;
const int DAIMON_SPACING_X =				12;
const int DAIMON_SPACING_Y =				12;

const int DEFAULT_DELAY_INC =				90;

const int DMZ_WIDTH =						40;				//	Distance from outer edge of countermeasures to daimons

const int ICON_WIDTH =						64;
const int ICON_HEIGHT =						64;
const int ICON_LABEL_WIDTH =				120;
const int ICON_LABEL_HEIGHT =				64;

const int ITEM_INFO_PANE_WIDTH =			318;
const int INFO_PANE_WIDTH =					200;
const DWORD INFO_PANE_HOVER_TIME =			300;

const int MESSAGE_OFFSET_Y =				-24;

const int MOUSE_SCROLL_SENSITIVITY =		240;

const int DEPLOY_BUTTON_RADIUS =			40;

const int STAT_BOX_WIDTH =					90;
const int STAT_BOX_HEIGHT =					32;
const int STAT_BOX_SPACING_X =				2;

const int STAT_MESSAGE_OFFSET_Y =			32;

CGSubjugateArea::CGSubjugateArea (const CVisualPalette &VI, CDockScreenSubjugate &Controller, CArtifactAwakening &Artifact) : 
		m_VI(VI),
		m_Controller(Controller),
		m_Artifact(Artifact),
		m_InfoPane(VI),
		m_StatsPainter{ VI, VI, VI },
		m_DaimonListPainter(VI),
		m_Messages(VI),
		m_DeployBtn(VI)

//	CGSubjugateArea constructor

	{
	int i;

	//	Initialize stats

	RefreshStatsPainters();

	//	Initialize the countermeasure loci (these form a ring around the central
	//	core).

	int iAngle = 0;
	m_CountermeasureLoci.InsertEmpty(COUNTERMEASURES_COUNT);
	for (i = 0; i < COUNTERMEASURES_COUNT; i++)
		{
		SCountermeasureLocus &Locus = m_CountermeasureLoci[i];

		Locus.iIndex = i;
		Locus.iStartAngle = iAngle;
		Locus.iArc = COUNTERMEASURES_ARC_ANGLE;
		Locus.iInnerRadius = COUNTERMEASURES_INNER_RADIUS;
		Locus.iOuterRadius = COUNTERMEASURES_INNER_RADIUS + COUNTERMEASURE_WIDTH;

		iAngle += COUNTERMEASURES_ARC_ANGLE;
		}

	//	Compute the x-offset of the upper and lower daimon rows so that they 
	//	follow the curve of the central core.

	Metric rCentralRowDist = COUNTERMEASURES_INNER_RADIUS + COUNTERMEASURE_WIDTH + DMZ_WIDTH;
	Metric rRowHeight = DAIMON_HEIGHT + DAIMON_SPACING_Y;
	Metric rTopRowDist = sqrt(rCentralRowDist * rCentralRowDist - rRowHeight * rRowHeight);

	int xCentralRow = -(int)rCentralRowDist - DAIMON_WIDTH;
	int xTopRow = -(int)rTopRowDist - DAIMON_WIDTH;

	//	Initialize the daimon loci.

	m_DaimonLoci.InsertEmpty(DAIMON_COUNT);
	m_DaimonLoci[0].iIndex = 0;
	m_DaimonLoci[0].xPos = xCentralRow;
	m_DaimonLoci[0].yPos = -(DAIMON_HEIGHT / 2);
	m_DaimonLoci[0].cxWidth = DAIMON_WIDTH;
	m_DaimonLoci[0].cyHeight = DAIMON_HEIGHT;

	m_DaimonLoci[1].iIndex = 1;
	m_DaimonLoci[1].xPos = xTopRow;
	m_DaimonLoci[1].yPos = -(DAIMON_HEIGHT / 2) - (DAIMON_HEIGHT + DAIMON_SPACING_Y);
	m_DaimonLoci[1].cxWidth = DAIMON_WIDTH;
	m_DaimonLoci[1].cyHeight = DAIMON_HEIGHT;

	m_DaimonLoci[2].iIndex = 2;
	m_DaimonLoci[2].xPos = xTopRow;
	m_DaimonLoci[2].yPos = (DAIMON_HEIGHT / 2) + DAIMON_SPACING_Y;
	m_DaimonLoci[2].cxWidth = DAIMON_WIDTH;
	m_DaimonLoci[2].cyHeight = DAIMON_HEIGHT;

	m_DaimonLoci[3].iIndex = 3;
	m_DaimonLoci[3].xPos = xCentralRow - (DAIMON_WIDTH + DAIMON_SPACING_X);
	m_DaimonLoci[3].yPos = -(DAIMON_HEIGHT / 2);
	m_DaimonLoci[3].cxWidth = DAIMON_WIDTH;
	m_DaimonLoci[3].cyHeight = DAIMON_HEIGHT;

	m_DaimonLoci[4].iIndex = 4;
	m_DaimonLoci[4].xPos = xTopRow - (DAIMON_WIDTH + DAIMON_SPACING_X);
	m_DaimonLoci[4].yPos = -(DAIMON_HEIGHT / 2) - (DAIMON_HEIGHT + DAIMON_SPACING_Y);
	m_DaimonLoci[4].cxWidth = DAIMON_WIDTH;
	m_DaimonLoci[4].cyHeight = DAIMON_HEIGHT;

	m_DaimonLoci[5].iIndex = 5;
	m_DaimonLoci[5].xPos = xTopRow - (DAIMON_WIDTH + DAIMON_SPACING_X);
	m_DaimonLoci[5].yPos = (DAIMON_HEIGHT / 2) + DAIMON_SPACING_Y;
	m_DaimonLoci[5].cxWidth = DAIMON_WIDTH;
	m_DaimonLoci[5].cyHeight = DAIMON_HEIGHT;

	//	Add the daimons to the list of available daimons

	for (i = 0; i < m_Artifact.GetInitialDaimons().GetCount(); i++)
		m_DaimonList.Add(m_Artifact.GetInitialDaimons()[i]);

	//	Compute some colors

	m_rgbCountermeasureBack = CG32bitPixel(255, 127, 127, 66);		//	H:0   S:50 B:100
	m_rgbCountermeasureLabel = CG32bitPixel(255, 127, 127);			//	H:0   S:50 B:100
	m_rgbDaimonBack = CG32bitPixel(229, 161, 229, 66);				//	H:300 S:30 B:90
	m_rgbDaimonLabel = CG32bitPixel(229, 161, 229);					//	H:300 S:30 B:90
	}

CGSubjugateArea::~CGSubjugateArea (void)

//	CGSubjugateArea destructor

	{
	CleanUp();
	}

void CGSubjugateArea::AddEffect (CArtifactResultPainter *pEffect)

//	AddEffect
//
//	Adds an effect to our list of effect painters if we don't already have it.

	{
	int i;

	//	See if we already have this effect. If we do, then just mark the effect
	//	(to indicate that it is in use) and free the input.

	for (i = 0; i < m_Effects.GetCount(); i++)
		if (m_Effects[i]->IsEqualTo(*pEffect))
			{
			m_Effects[i]->Mark();
			delete pEffect;
			return;
			}

	//	Otherwise, we need to add it to the list (and mark it).

	m_Effects.Insert(pEffect);
	pEffect->Mark();
	}

bool CGSubjugateArea::AddEffect (const CArtifactAwakening::SEventDesc &Event, int iDelay)

//	AddEffect
//
//	Adds an appropriate effect. Returns TRUE if we handled this effect (such that we need
//	to increase the delay).

	{
	//	Calculate the position of the source program

	int xProgram, yProgram;
	if (Event.pSource)
		GetProgramPos(Event.pSource->GetType(), Event.pSource->GetLocusIndex(), &xProgram, &yProgram);

	//	Create the effect

	switch (Event.iEvent)
		{
		case CArtifactAwakening::eventActivated:
			if (!Event.bAlreadyReported)
				m_Messages.AddMessage(CONSTLIT("Activated"), CArtifactMessagePainter::styleInfo, xProgram, yProgram + MESSAGE_OFFSET_Y, iDelay);

			AddEffect(new CArtifactResultPainter(Event.pSource, xProgram, yProgram, Event.iEvent, NULL, 0, 0, iDelay));
			break;

		case CArtifactAwakening::eventDeployed:
			if (!Event.bAlreadyReported)
				m_Messages.AddMessage(CONSTLIT("Deployed"), CArtifactMessagePainter::styleInfo, xProgram, yProgram + MESSAGE_OFFSET_Y, iDelay);
			break;

		case CArtifactAwakening::eventEgoChanged:
			if (!Event.bAlreadyReported)
				{
				int xStat, yStat;
				m_StatsPainter[CArtifactStat::statEgo].GetPos(&xStat, &yStat);
				m_Messages.AddMessage(CONSTLIT("Ego attacked"), CArtifactMessagePainter::styleInfo, xStat, yStat + STAT_MESSAGE_OFFSET_Y, iDelay);
				}

			AddEffect(new CArtifactResultPainter(Event.pSource, xProgram, yProgram, Event.iEvent, NULL, m_xCenter, m_yCenter, iDelay));
			break;

		case CArtifactAwakening::eventIntelligenceChanged:
			if (!Event.bAlreadyReported)
				{
				int xStat, yStat;
				m_StatsPainter[CArtifactStat::statIntelligence].GetPos(&xStat, &yStat);
				m_Messages.AddMessage(CONSTLIT("Intelligence suppressed"), CArtifactMessagePainter::styleInfo, xStat, yStat + STAT_MESSAGE_OFFSET_Y, iDelay);
				}

			AddEffect(new CArtifactResultPainter(Event.pSource, xProgram, yProgram, Event.iEvent, NULL, m_xCenter, m_yCenter, iDelay));
			break;

		case CArtifactAwakening::eventWillpowerChanged:
			if (!Event.bAlreadyReported)
				{
				int xStat, yStat;
				m_StatsPainter[CArtifactStat::statWillpower].GetPos(&xStat, &yStat);
				m_Messages.AddMessage(CONSTLIT("Willpower sapped"), CArtifactMessagePainter::styleInfo, xStat, yStat + STAT_MESSAGE_OFFSET_Y, iDelay);
				}

			AddEffect(new CArtifactResultPainter(Event.pSource, xProgram, yProgram, Event.iEvent, NULL, m_xCenter, m_yCenter, iDelay));
			break;

		case CArtifactAwakening::eventHalted:
			{
			int xTarget, yTarget;
			GetProgramPos(Event.pTarget->GetType(), Event.pTarget->GetLocusIndex(), &xTarget, &yTarget);

			if (!Event.bAlreadyReported)
				m_Messages.AddMessage(CONSTLIT("Halted"), CArtifactMessagePainter::styleInfo, xTarget, yTarget + MESSAGE_OFFSET_Y, iDelay);
			AddEffect(new CArtifactResultPainter(Event.pSource, xProgram, yProgram, Event.iEvent, Event.pTarget, xTarget, yTarget, iDelay));
			break;
			}

		case CArtifactAwakening::eventDefenseChanged:
			{
			int xTarget, yTarget;
			GetProgramPos(Event.pTarget->GetType(), Event.pTarget->GetLocusIndex(), &xTarget, &yTarget);

			if (!Event.bAlreadyReported)
				m_Messages.AddMessage(CONSTLIT("Security boosted"), CArtifactMessagePainter::styleInfo, xTarget, yTarget + MESSAGE_OFFSET_Y, iDelay);
			AddEffect(new CArtifactResultPainter(Event.pSource, xProgram, yProgram, Event.iEvent, Event.pTarget, xTarget, yTarget, iDelay));
			break;
			}

		case CArtifactAwakening::eventStrengthChanged:
			{
			int xTarget, yTarget;
			GetProgramPos(Event.pTarget->GetType(), Event.pTarget->GetLocusIndex(), &xTarget, &yTarget);

			if (!Event.bAlreadyReported)
				m_Messages.AddMessage(CONSTLIT("computer power increased"), CArtifactMessagePainter::styleInfo, xTarget, yTarget + MESSAGE_OFFSET_Y, iDelay);
			AddEffect(new CArtifactResultPainter(Event.pSource, xProgram, yProgram, Event.iEvent, Event.pTarget, xTarget, yTarget, iDelay));
			break;
			}

		default:
			return false;
		}

	return true;
	}

void CGSubjugateArea::ArtifactSubdued (void)

//	ArtifactSubdued
//
//	This is called when the artifact is successfully subjugated

	{
	m_Controller.OnCompleted(true);
	}

void CGSubjugateArea::CleanUp (void)

//	CleanUp
//
//	Free up all resources

	{
	int i;

	for (i = 0; i < m_Effects.GetCount(); i++)
		delete m_Effects[i];
	}

void CGSubjugateArea::Command (ECommands iCommand, void *pData)

//	Command
//
//	Execute a command

	{
	if (!IsActive())
		return;

	switch (iCommand)
		{
		case cmdDeployDaimon:
			DeployDaimon();
			break;

		case cmdSelectNextDaimon:
			SelectDaimon(m_DaimonList.GetSelection() + 1);
			break;

		case cmdSelectPrevDaimon:
			SelectDaimon(m_DaimonList.GetSelection() - 1);
			break;
		}
	}

void CGSubjugateArea::DeployDaimon (void)

//	DeployDaimon
//
//	Deploy the currently selected daimon.

	{
	int iSelection = m_DaimonList.GetSelection();
	CItemType *pDaimon = m_DaimonList.GetDaimon(iSelection);
	if (pDaimon)
		{
		g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));

		//	Update turn

		TArray<CArtifactAwakening::SEventDesc> Results;
		CArtifactAwakening::EResultTypes iResult = m_Artifact.PlayTurn(pDaimon, m_DaimonList.GetCount(), Results);

		//	Can fail if we've already filled up all loci, but we should
		//	check for this elsewhere.

		if (iResult == CArtifactAwakening::resultError)
			return;

		//	Remove the daimon from our list

		int iNewSelection = m_DaimonList.DeleteSelectedDaimon();
		m_DaimonListPainter.OnSelectionDeleted(iSelection);

		//	Refresh state

		RefreshStatsPainters();
		RefreshEffects(Results);

		//	Repaint

		Invalidate();

		//	If the battle is over, handle it.

		if (iResult == CArtifactAwakening::resultArtifactSubdued)
			ArtifactSubdued();
		else if (iResult == CArtifactAwakening::resultPlayerFailed)
			PlayerFailed();
		}
	}

void CGSubjugateArea::GetProgramPos (CArtifactProgram::EProgramTypes iType, int iLocus, int *retx, int *rety) const

//	GetProgramPos
//
//	Returns the position of the given program locus (center).

	{
	if (iType == CArtifactProgram::typeCountermeasure)
		{
		const SCountermeasureLocus &Locus = m_CountermeasureLoci[iLocus];

		int iCenterAngle = Locus.iStartAngle + (Locus.iArc / 2);
		CVector vOffset = PolarToVector(iCenterAngle, Locus.iInnerRadius + (COUNTERMEASURE_WIDTH / 2));
		*retx = m_xCenter + (int)vOffset.GetX();
		*rety = m_yCenter - (int)vOffset.GetY();
		}
	else
		{
		const SDaimonLocus &Locus = m_DaimonLoci[iLocus];
		*retx = m_xCenter + Locus.xPos + (Locus.cxWidth / 2);
		*rety = m_yCenter + Locus.yPos + (Locus.cyHeight / 2);
		}
	}

void CGSubjugateArea::HideInfoPane (void)

//	HideInfoPane
//
//	Hides the info pane if it is showing

	{
	if (m_InfoPaneSel.iType != selectNone)
		{
		m_InfoPane.Hide();
		m_InfoPaneSel = SSelection();
		Invalidate();
		}
	}

bool CGSubjugateArea::HitTest (int x, int y, SSelection &Sel) const

//	HitTest
//
//	Returns TRUE if the point is over an active region.
//	NOTE: x,y are paint coordinates (i.e., screen relative).

	{
	//	Only works if we're active

	if (!IsActive())
		{
		Sel = SSelection();
		return false;
		}

	//	Check the list of daimons (NOTE: We could return iIndex of -1 if we
	//	are over the list area, but not on an actual item).

	else if (m_DaimonListPainter.HitTest(x, y, &Sel.iIndex))
		Sel.iType = selectDaimonList;

	//	Check deploy button

	else if (m_DeployBtn.HitTest(x, y))
		Sel.iType = selectDeployBtn;

	//	Check countermeasures

	else if (HitTestCountermeasureLoci(x, y, &Sel.iIndex))
		Sel.iType = selectCountermeasureLoci;

	//	Otherwise, nothing

	else
		{
		Sel = SSelection();
		return false;
		}

	//	Success!

	return true;
	}

bool CGSubjugateArea::HitTestCountermeasureLoci (int x, int y, int *retiIndex) const

//	HitTestCountermeasureLoci
//
//	Returns TRUE if the point is over a countermeasure loci.

	{
	int i;

	//	Convert mouse position to radial coordinates relative to the center of
	//	the core.

	Metric rRadius;
	int iAngle = VectorToPolar(CVector(x - m_xCenter, m_yCenter - y), &rRadius);
	int iRadius = (int)rRadius;

	//	See if we're inside any loci

	for (i = 0; i < m_CountermeasureLoci.GetCount(); i++)
		{
		const SCountermeasureLocus &Locus = m_CountermeasureLoci[i];

		if (iAngle >= Locus.iStartAngle && iAngle < (Locus.iStartAngle + Locus.iArc)
				&& iRadius >= Locus.iInnerRadius && iRadius < Locus.iOuterRadius)
			{
			if (retiIndex)
				*retiIndex = i;

			return true;
			}
		}

	return false;
	}

bool CGSubjugateArea::IsCommandValid (ECommands iCommand, void *pData) const

//	IsCommandValid
//
//	Returns TRUE if the command can be executed right now.

	{
	switch (iCommand)
		{
		case cmdSelectNextDaimon:
			return (m_DaimonList.GetSelection() < m_DaimonList.GetCount() - 1);

		case cmdSelectPrevDaimon:
			return (m_DaimonList.GetSelection() > 0);

		default:
			return false;
		}
	}

bool CGSubjugateArea::LButtonDoubleClick (int x, int y)

//	LButtonDoubleClick
//
//	Handle mouse

	{
	return LButtonDown(x, y);
	}

bool CGSubjugateArea::LButtonDown (int x, int y)

//	LButtonDown
//
//	Handle mouse

	{
	//	We store everything in paint coordinates, so we need to convert

	GetParent()->ConvertToPaintCoords(x, y);

	//	See what we clicked on.

	if (!HitTest(x, y, m_Clicked))
		return false;

	//	Handle the click (if necessary)

	switch (m_Clicked.iType)
		{
		case selectDaimonList:
			if (m_Clicked.iIndex != -1)
				SelectDaimon(m_Clicked.iIndex);
			break;
		}

	return true;
	}

void CGSubjugateArea::LButtonUp (int x, int y)

//	LButtonUp
//
//	Handle mouse

	{
	//	We store everything in paint coordinates, so we need to convert

	GetParent()->ConvertToPaintCoords(x, y);

	//	Remember where we clicked, but undo our internal state in case we
	//	need to return early.

	SSelection Clicked = m_Clicked;
	m_Clicked = SSelection();

	//	See where we are now

	SSelection Released;
	if (!HitTest(x, y, Released))
		return;

	//	Handle it if we released at an active point

	switch (Released.iType)
		{
		case selectDeployBtn:
			//	If we clicked down on the button and released over it, then we
			//	do it.

			if (Clicked.iType == selectDeployBtn)
				DeployDaimon();
			break;
		}
	}

void CGSubjugateArea::MouseEnter (void)

//	MouseEnter
//
//	Mouse has entered area

	{
	}

void CGSubjugateArea::MouseLeave (void)

//	MouseLeave
//
//	Mouse has left area

	{
	m_Hover = SSelection();
	HideInfoPane();
	}

void CGSubjugateArea::MouseMove (int x, int y)

//	MoveMose
//
//	Mouse has moved within the area.

	{
	//	We store everything in paint coordinates, so we need to convert

	GetParent()->ConvertToPaintCoords(x, y);

	//	Hit test (store the result in m_Hover).

	HitTest(x, y, m_Hover);

	//	Close the info pane, if necessary

	if (m_Hover != m_InfoPaneSel)
		HideInfoPane();
	}

void CGSubjugateArea::MouseWheel (int iDelta, int x, int y, DWORD dwFlags)

//	MouseWheel
//
//	Handle mouse scrolling

	{
	//	We store everything in paint coordinates, so we need to convert

	GetParent()->ConvertToPaintCoords(x, y);

	//	Hit test (store the result in m_Hover).

	SSelection Select;
	if (!HitTest(x, y, Select))
		return;

	//	Handle it

	switch (Select.iType)
		{
		case selectDaimonList:
			{
			int iChange = (-iDelta / MOUSE_SCROLL_SENSITIVITY);
			if (iChange == 0)
				iChange = -Sign(iDelta / (MOUSE_SCROLL_SENSITIVITY / 8));

			SelectDaimon(m_DaimonList.GetSelection() + iChange);
			break;
			}
		}
	}

void CGSubjugateArea::OnSetRect (void)

//	OnSetRect
//
//	The rectangle has been set, so we can compute some metrics

	{
	int i;
	RECT rcRect = GetParent()->GetPaintRect(GetRect());

	//	We split the rect in thirds

	int cxThird = RectWidth(rcRect) / 3;
	int xCol1 = rcRect.left;
	int xCol2 = xCol1 + cxThird;
	int xCol3 = rcRect.right - cxThird;

	//	Center line (vertically)

	int yRectCenter = rcRect.top + (RectHeight(rcRect) / 2);

	//	The core is at the center of the 3rd column

	m_xCenter = xCol3 + (cxThird / 2);
	m_yCenter = yRectCenter;

	//	The list of daimons in hand is in the first column

	m_rcHand.left = xCol1;
	m_rcHand.top = rcRect.top;
	m_rcHand.right = xCol1 + cxThird;
	m_rcHand.bottom = rcRect.bottom;

	m_DaimonListPainter.SetList(m_DaimonList);
	m_DaimonListPainter.SetRect(m_rcHand);

	//	Position the stats rects

	int cxAllStats = (CArtifactStat::statCount * STAT_BOX_WIDTH + (CArtifactStat::statCount - 1) * STAT_BOX_SPACING_X);
	int xBox = m_xCenter - (cxAllStats / 2);
	int yBox = rcRect.top;
	for (i = 0; i < CArtifactStat::statCount; i++)
		{
		RECT rcBox;
		rcBox.left = xBox;
		rcBox.right = rcBox.left + STAT_BOX_WIDTH;
		rcBox.top = yBox;
		rcBox.bottom = rcBox.top + STAT_BOX_HEIGHT;

		m_StatsPainter[i].SetRect(rcBox);

		xBox += STAT_BOX_WIDTH + STAT_BOX_SPACING_X;
		}

	m_StatsPainter[CArtifactStat::statEgo].SetLabel(CONSTLIT("ego"));
	m_StatsPainter[CArtifactStat::statIntelligence].SetLabel(CONSTLIT("intelligence"));
	m_StatsPainter[CArtifactStat::statWillpower].SetLabel(CONSTLIT("willpower"));

	//	Position the deploy button at the center of the boundary between 
	//	columns 1 and 2.

	m_DeployBtn.SetPos(xCol2 + DEPLOY_BUTTON_RADIUS, yRectCenter);
	m_DeployBtn.SetLabel(STR_DEPLOY);
	}

void CGSubjugateArea::Paint (CG32bitImage &Dest, const RECT &rcRect)

//	Paint
//
//	Paint

	{
	DEBUG_TRY

	int i;

//	Dest.Fill(rcRect.left, rcRect.top, RectWidth(rcRect), RectHeight(rcRect), CG32bitPixel(128, 128, 255, 0x40));

	//	Paint the central core animation

	m_AICorePainter.Paint(Dest, m_xCenter, m_yCenter);

	//	Paint the core stats

	for (i = 0; i < CArtifactStat::statCount; i++)
		m_StatsPainter[i].Paint(Dest);

	//	Paint the deployed countermeasures

	for (i = 0; i < m_CountermeasureLoci.GetCount(); i++)
		PaintCountermeasureLocus(Dest, m_CountermeasureLoci[i]);

	//	Paint the deployed daimons

	for (i = 0; i < m_DaimonLoci.GetCount(); i++)
		PaintDaimonLocus(Dest, m_DaimonLoci[i]);

	//	Paint the available daimons

	m_DaimonListPainter.Paint(Dest);

	//	Paint the deploy button

	CDaimonButtonPainter::EStates iDeployBtnState;
	if (!IsActive() || m_DaimonList.GetSelection() == -1)
		iDeployBtnState = CDaimonButtonPainter::stateDisabled;
	else if (g_pHI->IsLButtonDown() && m_Hover.iType == selectDeployBtn && m_Clicked.iType == selectDeployBtn)
		iDeployBtnState = CDaimonButtonPainter::stateDown;
	else if (!g_pHI->IsLButtonDown() && m_Hover.iType == selectDeployBtn)
		iDeployBtnState = CDaimonButtonPainter::stateHover;
	else
		iDeployBtnState = CDaimonButtonPainter::stateNormal;

	m_DeployBtn.Paint(Dest, iDeployBtnState);

	//	Paint effects

	for (i = 0; i < m_Effects.GetCount(); i++)
		m_Effects[i]->Paint(Dest);

	//	Messages

	m_Messages.Paint(Dest);

	//	Paint the info pane on top of everything

	m_InfoPane.Paint(Dest);

	DEBUG_CATCH
	}

void CGSubjugateArea::PaintCoreStats (CG32bitImage &Dest) const

//	PaintCoreStats
//
//	Paint the core stats

	{
	}

void CGSubjugateArea::PaintCountermeasureLocus (CG32bitImage &Dest, const SCountermeasureLocus &Locus) const

//	PaintCountermeasureLocus
//
//	Paints a deployed countermeasure.

	{
	CGDraw::Arc(Dest,
			m_xCenter,
			m_yCenter,
			Locus.iInnerRadius,
			Locus.iStartAngle,
			Locus.iStartAngle + Locus.iArc,
			COUNTERMEASURE_WIDTH,
			m_rgbCountermeasureBack,
			CGDraw::blendNormal,
			COUNTERMEASURE_SPACING / 2,
			CGDraw::ARC_INNER_RADIUS);

	CArtifactProgram *pProgram = m_Artifact.GetLocusProgram(CArtifactProgram::typeCountermeasure, Locus.iIndex);
	if (pProgram
			&& !Locus.bHidden)
		{
		int iCenterAngle = Locus.iStartAngle + (Locus.iArc / 2);
		CVector vOffset = PolarToVector(iCenterAngle, Locus.iInnerRadius + (COUNTERMEASURE_WIDTH / 2));
		int x = m_xCenter + (int)vOffset.GetX();
		int y = m_yCenter - (int)vOffset.GetY();

		PaintProgram(Dest, *pProgram, x, y, Locus.bPaintHalted);
		}
	}

void CGSubjugateArea::PaintDaimonLocus (CG32bitImage &Dest, const SDaimonLocus &Locus) const

//	PaintDaimonLocus
//
//	Paints a deployed daimon

	{
	CGDraw::RoundedRect(Dest,
			m_xCenter + Locus.xPos,
			m_yCenter + Locus.yPos,
			Locus.cxWidth,
			Locus.cyHeight,
			DAIMON_BORDER_RADIUS,
			m_rgbDaimonBack);

	CArtifactProgram *pProgram = m_Artifact.GetLocusProgram(CArtifactProgram::typeDaimon, Locus.iIndex);
	if (pProgram)
		{
		int x = m_xCenter + Locus.xPos + (Locus.cxWidth / 2);
		int y = m_yCenter + Locus.yPos + (Locus.cyHeight / 2);

		PaintProgram(Dest, *pProgram, x, y, Locus.bPaintHalted);
		}
	}

void CGSubjugateArea::PaintProgram (CG32bitImage &Dest, const CArtifactProgram &Program, int x, int y, bool bGrayed) const

//	PaintProgram
//
//	Paints a deployed program at the given coordinates.

	{
	CItemType *pType = Program.GetItemType();

	//	Choose some colors

	CG32bitPixel rgbLabel = (Program.GetType() == CArtifactProgram::typeCountermeasure ? m_rgbCountermeasureLabel : m_rgbDaimonLabel);

	//	Paint the icon (centered)

	DrawItemTypeIcon(Dest, x - (ICON_WIDTH / 2), y - (ICON_HEIGHT / 2), pType, ICON_WIDTH, ICON_HEIGHT, bGrayed);

	//	Paint the name

	RECT rcText;
	rcText.left = x - (ICON_LABEL_WIDTH / 2);
	rcText.right = rcText.left + ICON_LABEL_WIDTH;
	rcText.top = y + (ICON_HEIGHT / 2);
	rcText.bottom = rcText.top + ICON_LABEL_HEIGHT;

	DWORD dwNounFlags = nounNoModifiers | nounShort | nounTitleCapitalize;
	m_VI.GetFont(fontMedium).DrawText(Dest, rcText, rgbLabel, pType->GetNounPhrase(dwNounFlags), 0, CG16bitFont::AlignCenter);
	}

void CGSubjugateArea::PlayerFailed (void)

//	PlayerFailed
//
//	Player failed to subdue

	{
	m_Controller.OnCompleted(false);
	}

void CGSubjugateArea::RefreshEffects (const TArray<CArtifactAwakening::SEventDesc> &Events)

//	RefreshEffects
//
//	Loops over all effects in m_Artifact and makes sure they are reflected in 
//	our array.

	{
	int i;
	int iDelay = 0;

	//	Loop over all effects and clear their marks so we can tell which ones we
	//	need to delete.

	for (i = 0; i < m_Effects.GetCount(); i++)
		m_Effects[i]->Mark(false);

	//	Loop over all events and add effects and messages as appropriate.

	for (i = 0; i < Events.GetCount(); i++)
		{
		const CArtifactAwakening::SEventDesc &Event = Events[i];

		//	Add any effects and messages for this event.

		if (AddEffect(Event, iDelay))
			{
			//	If we added an effect, and if this event hasn't already been 
			//	reported, then we add a delay.

			if (!Event.bAlreadyReported)
				iDelay += DEFAULT_DELAY_INC;
			}
		}

	//	Delete any effects that we're no longer using

	for (i = 0; i < m_Effects.GetCount(); i++)
		{
		if (!m_Effects[i]->IsMarked())
			{
			delete m_Effects[i];
			m_Effects.Delete(i);
			i--;
			}
		}
	}

void CGSubjugateArea::RefreshStatsPainters (void)

//	RefreshStatsPainters
//
//	Updates painters from the artifact data.

	{
	int i;

	//	Initialize stats

	for (i = 0; i < CArtifactStat::statCount; i++)
		m_StatsPainter[i].SetStat(m_Artifact.GetCoreStat((CArtifactStat::ETypes)i));
	}

void CGSubjugateArea::SelectDaimon (int iNewSelection)

//	SelectDaimon
//
//	Selects the given daimon

	{
	int iOldSelection = m_DaimonList.GetSelection();
	iNewSelection = m_DaimonList.SetSelection(iNewSelection);

	if (iOldSelection != iNewSelection)
		{
		g_pUniverse->PlaySound(NULL, g_pUniverse->FindSound(UNID_DEFAULT_SELECT));
		m_DaimonListPainter.OnSelectionChanged(iOldSelection, iNewSelection);
		Invalidate();
		}
	}

void CGSubjugateArea::Update (void)

//	Update
//
//	Update

	{
	DEBUG_TRY

	int i;

	//	Update all our components

	if (m_DaimonListPainter.Update())
		Invalidate();

	m_DeployBtn.Update();
	m_AICorePainter.Update();
	m_Messages.Update();

	//	Reset all programs so they paint fully. We then check effects to see if 
	//	we need to paint a program as halted.

	for (i = 0; i < m_CountermeasureLoci.GetCount(); i++)
		{
		m_CountermeasureLoci[i].bHidden = true;
		m_CountermeasureLoci[i].bPaintHalted = false;
		}

	for (i = 0; i < m_DaimonLoci.GetCount(); i++)
		m_DaimonLoci[i].bPaintHalted = false;

	//	Update all effects

	for (i = 0; i < m_Effects.GetCount(); i++)
		{
		m_Effects[i]->Update();

		//	If this effect activates a program

		CArtifactProgram *pProgram;
		if (pProgram = m_Effects[i]->GetProgramActivated())
			{
			if (pProgram->GetType() == CArtifactProgram::typeCountermeasure)
				m_CountermeasureLoci[pProgram->GetLocusIndex()].bHidden = false;
			}

		//	See if this effect has halted a program

		if (pProgram = m_Effects[i]->GetProgramHalted())
			{
			if (pProgram->GetType() == CArtifactProgram::typeCountermeasure)
				m_CountermeasureLoci[pProgram->GetLocusIndex()].bPaintHalted = true;
			else
				m_DaimonLoci[pProgram->GetLocusIndex()].bPaintHalted = true;
			}
		}

	//	See if we need to show the info pane

	if (GetScreen()->GetTimeSinceMouseMove() >= INFO_PANE_HOVER_TIME
			&& m_InfoPaneSel != m_Hover)
		{
		POINT pt;
		GetScreen()->GetMousePos(&pt);
		int x = pt.x;
		int y = pt.y;
		GetParent()->ConvertToPaintCoords(x, y);

		switch (m_Hover.iType)
			{
			case selectCountermeasureLoci:
				{
				CArtifactProgram *pProgram = m_Artifact.GetLocusProgram(CArtifactProgram::typeCountermeasure, m_Hover.iIndex);
				if (pProgram)
					{
					m_InfoPane.SetItem(CItem(pProgram->GetItemType(), 1));
					m_InfoPane.Show(x, y + CURSOR_OFFSET_Y, ITEM_INFO_PANE_WIDTH, GetPaintRect());
					m_InfoPaneSel = m_Hover;
					Invalidate();
					}
				else
					m_InfoPane.Hide();

				m_InfoPaneSel = m_Hover;
				Invalidate();
				break;
				}
			}
		}

	DEBUG_CATCH
	}
