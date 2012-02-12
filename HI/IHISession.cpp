//	IHISession.cpp
//
//	IHISession class
//	Copyright (c) 2010 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

void IHISession::DefaultOnAnimate (CG16bitImage &Screen)

//	DefaultOnAnimate
//
//	Only if session does not override OnAnimate

	{
	bool bHasAnimation = false;

	//	Let the session paint the screen

	m_HI.BeginSessionPaint(Screen);

	//	Session itself paints

	HIPaint(Screen);

	//	Paint animations on top

	if (m_Reanimator.PaintFrame(m_HI.GetScreen()))
		bHasAnimation = true;

	//	Done

	m_HI.EndSessionPaint(Screen);

	//	Update

	m_HI.BeginSessionUpdate();
	HIUpdate();
	m_HI.EndSessionUpdate();

	//	If we've got animations, then we need to invalidate
	//	the whole screen.

	if (bHasAnimation)
		HIInvalidate();
	}

void IHISession::HIPaint (CG16bitImage &Screen)

//	HIPaint
//
//	Paint the session screen
	
	{
	RECT rcInvalid;
	if (m_HI.GetScreenMgr().GetInvalidRect(&rcInvalid))
		{
		//	Prepare

		Screen.SetClipRect(rcInvalid);

		//	Paint

		OnPaint(Screen, rcInvalid);

		//	Done

		m_HI.GetScreenMgr().Validate();
		Screen.ResetClipRect();
		}
	}

void IHISession::StartPerformance (IAnimatron *pAni, const CString &sID, DWORD dwFlags)

//	StartPerformance
//
//	Start an animation performance

	{
	DWORD dwPerf = m_Reanimator.AddPerformance(pAni, sID);
	m_Reanimator.StartPerformance(dwPerf, dwFlags);
	}
