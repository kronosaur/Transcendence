//	CArtifactAICorePainter.cpp
//
//	CArtifactAICorePainter class
//	Copyright (c) 2016 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"
#include "Transcendence.h"

const int MAX_RADIUS =						80;

#define STYLECOLOR(x)						(CArtifactAwakenVisuals::GetColor(CArtifactAwakenVisuals::##x))

CArtifactAICorePainter::CArtifactAICorePainter (void)

//	CArtifactAICorePainter constructor

	{
	}

void CArtifactAICorePainter::Paint (CG32bitImage &Dest, int x, int y)

//	Paint
//
//	Paints a frame

	{
	CGDraw::Circle(Dest, x, y, MAX_RADIUS, STYLECOLOR(colorAICoreBack));
	}
