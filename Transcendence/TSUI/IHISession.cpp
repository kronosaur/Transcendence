//	IHISession.cpp
//
//	IHISession class
//	Copyright (c) 2010 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

IHISession::IHISession (CHumanInterface &HI) : IHICommand(HI),
		m_bNoCursor(false),
		m_bTransparent(false)

//	IHISession constructor
	
	{
	}

void IHISession::DefaultOnAnimate (CG16bitImage &Screen, bool bTopMost)

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

	m_HI.EndSessionPaint(Screen, bTopMost);

	//	Update

	m_HI.BeginSessionUpdate();
	HIUpdate();
	m_HI.EndSessionUpdate(bTopMost);

	//	If we've got animations, then we need to invalidate
	//	the whole screen.

	if (bHasAnimation)
		HIInvalidate();
	}

void IHISession::HIChar (char chChar, DWORD dwKeyData)

//	HIChar
//
//	Handle character input
	
	{
	//	See if the animator will handle it

	if (m_Reanimator.HandleChar(chChar, dwKeyData))
		return;

	//	Otherwise, the subclass can handle it.

	OnChar(chChar, dwKeyData); 
	}

void IHISession::HIKeyDown (int iVirtKey, DWORD dwKeyData)

//	HIKeyDown
//
//	Handle character input
	
	{
	//	See if the animator will handle it

	if (m_Reanimator.HandleKeyDown(iVirtKey, dwKeyData))
		return;

	OnKeyDown(iVirtKey, dwKeyData);
	}

void IHISession::HIKeyUp (int iVirtKey, DWORD dwKeyData)

//	HIKeyUp
//
//	Handle character input

	{
	//	See if the animator will handle it

	if (m_Reanimator.HandleKeyUp(iVirtKey, dwKeyData))
		return;

	OnKeyUp(iVirtKey, dwKeyData);
	}
	
void IHISession::HILButtonDblClick (int x, int y, DWORD dwFlags)

//	HILButtonDblClick
//
//	Handle mouse input
	
	{
	//	See if the animator will handle it

	bool bCapture = false;
	if (m_Reanimator.HandleLButtonDblClick(x, y, dwFlags, &bCapture))
		{
		if (bCapture)
			::SetCapture(m_HI.GetHWND());
		return;
		}

	OnLButtonDblClick(x, y, dwFlags); 
	}

void IHISession::HILButtonDown (int x, int y, DWORD dwFlags)

//	HILButtonDown
//
//	Handle mouse input
	
	{
	//	See if the animator will handle it

	bool bCapture = false;
	if (m_Reanimator.HandleLButtonDown(x, y, dwFlags, &bCapture))
		{
		if (bCapture)
			::SetCapture(m_HI.GetHWND());
		return;
		}

	OnLButtonDown(x, y, dwFlags);
	}

void IHISession::HILButtonUp (int x, int y, DWORD dwFlags)

//	HILButtonUp
//
//	Handle mouse input
	
	{
	if (::GetCapture() == m_HI.GetHWND())
		::ReleaseCapture();

	//	See if the animator will handle it

	if (m_Reanimator.HandleLButtonUp(x, y, dwFlags))
		return;

	OnLButtonUp(x, y, dwFlags);
	}

void IHISession::HIMouseMove (int x, int y, DWORD dwFlags)

//	HIMouseMove
//
//	Handle mouse input
	
	{
	//	See if the animator will handle it

	if (m_Reanimator.HandleMouseMove(x, y, dwFlags))
		return;

	OnMouseMove(x, y, dwFlags);
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

void IHISession::OnAniCommand (const CString &sID, const CString &sEvent, const CString &sCmd, DWORD dwData)

//	OnAniCommand
//
//	Handle event from reanimator

	{
	SPerformanceEvent Ctx;
	Ctx.sID = sID;
	Ctx.sEvent = sEvent;

	HICommand(sCmd, &Ctx);
	}

void IHISession::RegisterPerformanceEvent (IAnimatron *pAni, const CString &sEvent, const CString &sCmd)

//	RegisterPerformanceEvent
//
//	Register to get events from an animatron control

	{
	pAni->AddListener(sEvent, this, sCmd);
	}

void IHISession::StartPerformance (IAnimatron *pAni, const CString &sID, DWORD dwFlags)

//	StartPerformance
//
//	Start an animation performance

	{
	DWORD dwPerf = m_Reanimator.AddPerformance(pAni, sID);
	m_Reanimator.StartPerformance(dwPerf, dwFlags);
	}
