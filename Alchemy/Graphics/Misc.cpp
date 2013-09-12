//	Misc.cpp
//
//	Implements miscellaneous functions

#include <windows.h>
#include <MMSystem.h>

#include "Kernel.h"
#include "KernelObjID.h"
#include "Graphics.h"

COLORREF strToCOLORREF(const CString &sColor)

//	strToCOLORREF
//
//	Converts a string of the 24-bit hex-representation "rrggbb" into an RGB COLORREF

	{
	char *pPos = sColor.GetASCIIZPointer();

	//	Skip '#', if there

	if (*pPos == '#')
		pPos++;

	//	Parse value as hex

	DWORD dwValue = (DWORD)strParseIntOfBase(pPos,
			16,
			0xFFFFFF,
			NULL,
			NULL);

	//	Modify format appropriately

	return (COLORREF)(((dwValue & 0x00FF0000) >> 16) | ((dwValue & 0x0000FF00)) | ((dwValue & 0x000000FF) << 16));
	}
