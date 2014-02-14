//	SDLUtil.cpp
//
//	SDLUtil Library
//	Copyright (c) 2014 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

bool sdlInit (void)

//	sdlInit
//
//	Initialize. This must be called once per process. Return TRUE if we 
//	succeeded.

	{
	if (SDL_Init(SDL_INIT_AUDIO) != 0)
		{
		::kernelDebugLogMessage("ERROR: Unable to initialize SDL: %s", CString(SDL_GetError()));
		return false;
		}

	return true;
	}

void sdlCleanUp (void)

//	sdlCleanUp
//
//	Clean up SDL. This must be called before terminating.

	{
	SDL_Quit();
	}
