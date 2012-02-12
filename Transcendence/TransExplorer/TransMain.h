//	TransMain.h
//
//	Main application entry-point for Transcendence UI applications
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#pragma once

#include "Alchemy.h"
#include "..\TSUI\TSUI.h"

//	Application definitions

#define TRANS_APP_TITLE					"Transcendence Explorer"
#define TRANS_APP_WINDOW_CLASS			"TransExplorer_class"

//	Constants

const int TICKS_PER_SECOND = 30;

//	Globals

extern HINSTANCE g_hInst;

//	Include application header

#include "TransExplorer.h"
