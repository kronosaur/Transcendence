//	CLoadGameWithSignInTask.cpp
//
//	CLoadGameWithSignIn class
//	Copyright (c) 2012 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"
#include "Transcendence.h"

ALERROR CLoadGameWithSignInTask::OnExecute (ITaskProcessor *pProcessor, CString *retsResult)

//	OnExecute
//
//	Loads a game, signing in if necessary.
	
	{
	ALERROR error;

	//	Sign in first, if necessary

	if (m_Service.HasCapability(ICIService::autoLoginUser))
		{
		if (error = m_Service.SignInUser(pProcessor, NULL_STR, NULL_STR, true, retsResult))
			return error;
		}

	//	Load the game

	return m_Model.LoadGame(m_Service.GetUsername(), m_sFilespec, retsResult);
	}
