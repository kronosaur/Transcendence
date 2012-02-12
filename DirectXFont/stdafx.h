// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						

#include <stdio.h>
#include <tchar.h>

#include <windows.h>
#include <ddraw.h>

#include "Alchemy.h"
#include "XMLUtil.h"
#include "DirectXUtil.h"

//	Don't care about warning of depecrated functions (e.g., sprintf)
#pragma warning(disable: 4996)

// TODO: reference additional headers your program requires here
