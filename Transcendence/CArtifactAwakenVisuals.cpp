//	CArtifactAwakenVisuals.cpp
//
//	CArtifactAwakenVisuals class
//	Copyright (c) 2016 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"
#include "Transcendence.h"

CG32bitPixel COLOR_TABLE[CArtifactAwakenVisuals::colorCount] =
	{
	CG32bitPixel(0x4d, 0x36, 0x4d),			//	DeployDaimonBack		H:300 S:30  B:30
	CG32bitPixel(0xff, 0xb2, 0xff),			//	DeployDaimonFore		H:300 S:30  B:100
	//CG32bitPixel(0x36, 0x4d, 0x36),			//	DeployDaimonBack		H:120 S:30  B:30
	//CG32bitPixel(0xb3, 0xff, 0xb3),			//	DeployDaimonFore		H:120 S:30  B:100
	CG32bitPixel(0xff, 0xd9, 0xff),			//	DeployDaimonTitle		H:300 S:15  B:100
	CG32bitPixel(229, 161, 229, 66),		//	DaimonLocusBack			H:300 S:30  B:90
	CG32bitPixel(255, 127, 127, 66),		//	CountermeasureLocusBack	H:0   S:50  B:100

	CG32bitPixel(0xff, 0x80, 0x80, 0x80),	//	AICoreBack				H:0   S:50  B:100
	CG32bitPixel(0xff, 0x80, 0x80),			//	AICoreFore				H:0   S:50  B:100
	};

CG32bitPixel CArtifactAwakenVisuals::GetColor (EColors iColor)

//	GetColor
//
//	Returns a color

	{
	return COLOR_TABLE[iColor];
	}
