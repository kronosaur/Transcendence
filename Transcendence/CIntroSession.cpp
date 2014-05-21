//	CIntroSession.cpp
//
//	CIntroSession class
//	Copyright (c) 2014 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"
#include "Transcendence.h"

void CIntroSession::OnAnimate (CG16bitImage &Screen, bool bTopMost)

//	OnAnimate
//
//	Animate the session

	{
	bool bFailed = false;

	//	Update context

	SSystemUpdateCtx UpdateCtx;

	try
		{
		//	Paint

		SetProgramState(psAnimating);
		g_pTrans->AnimateIntro(bTopMost);

		//	Flip

		if (bTopMost)
			m_HI.GetScreenMgr().Flip();

		SetProgramState(psUnknown);
		}
	catch (...)
		{
		bFailed = true;
		}

	//	Deal with errors/crashes

	if (bFailed)
		{
		m_HI.GetScreenMgr().StopDX();
		g_pTrans->ReportCrash();
		}
	}
