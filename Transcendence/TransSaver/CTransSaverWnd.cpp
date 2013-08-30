//	CTransSaverWnd.cpp
//
//	CTransSaverWnd class
//	Copyright (c) 2010 by George Moromisato. All Rights Reserved.

#include "PreComp.h"

CTransSaverWnd::CTransSaverWnd (void)

//	CTransSaverWnd constructor

	{
	}

void CTransSaverWnd::OnAnimate (void)

//	OnAnimate
//
//	Paint an animation frame

	{
	CG16bitImage &Screen = m_UI.GetScreen();

	Screen.Fill(0, 0, 300, 300, CG16bitImage::RGBValue(0, 0, 128));
	m_UI.BltScreen();
	m_UI.FlipScreen();
	}

LONG CTransSaverWnd::WMCreate (HWND hWnd, LPCREATESTRUCT pCreate, CString *retsError)

//	WMCreate
//
//	Handle WM_CREATE

	{
	ALERROR error;

	SHIOptions Options;
	Options.m_hWnd = hWnd;
	if (error = m_UI.Init(Options, retsError))
		return -1;

	return 0;
	}

LONG CTransSaverWnd::WMDestroy (void)

//	WMDestroy
//
//	Handle WM_DESTROY

	{
	m_UI.CleanUp();

	return 0;
	}
