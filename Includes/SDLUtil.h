//	SDLUtil.h
//
//	SDL Wrapper
//
//	We require:
//
//		Kernel.h

#pragma once

#ifndef INCL_KERNEL
#include "Kernel.h"
#endif

bool sdlInit (void);
void sdlCleanUp (void);
