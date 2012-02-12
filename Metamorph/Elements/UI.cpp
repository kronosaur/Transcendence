//	UI.cpp
//
//	Miscellaneous UI Routines
//	Copyright (c) 2001 by NeuroHack, Inc. All Rights Reserved.

#include "Elements.h"

CString FormatFilterString (const CString &sExtensions, int *retiFirstExtPos = NULL);

bool uiChooseColor (HWND hParent,
					COLORREF *ioColor)

//	uiChooseColor
//
//	Puts up a color chooser dialog box. Returns TRUE if the user
//	accepted; FALSE if the user cancels

	{
	CHOOSECOLOR cc;
	static COLORREF acrCustClr[16];

	::ZeroMemory(&cc, sizeof(cc));
	cc.lStructSize = sizeof(cc);
	cc.hwndOwner = hParent;
	cc.lpCustColors = (LPDWORD)acrCustClr;
	cc.rgbResult = *ioColor;
	cc.Flags = CC_FULLOPEN | CC_RGBINIT;

	if (::ChooseColor(&cc))
		{
		*ioColor = cc.rgbResult;
		return true;
		}
	else
		return false;
	}

bool uiGetOpenFilename (HWND hParent,
						const CString &sExtensions,
						const CString &sFolder,
						bool bAllowMultipleFiles,
						CString *retsFilenames)

//	uiGetOpenFilename
//
//	Puts up a file open dialog box. Returns TRUE if the user
//	accepted the file; FALSE if the user cancels.

	{
	OPENFILENAME of;
	char szBuffer[8192];

	CString sFilter = FormatFilterString(sExtensions);

	::ZeroMemory(&of, sizeof(of));
	of.lStructSize = sizeof(of);
	of.hwndOwner = hParent;
	of.lpstrFilter = sFilter.GetASCIIZ();
	of.lpstrFile = szBuffer;
	szBuffer[0] = '\0';
	of.nMaxFile = sizeof(szBuffer)-1;
	of.lpstrInitialDir = sFolder.GetASCIIZ();
	of.Flags = 0;
	if (bAllowMultipleFiles)
		of.Flags |= OFN_ALLOWMULTISELECT;
	of.Flags |= OFN_EXPLORER;
	of.Flags |= OFN_FILEMUSTEXIST;
	of.Flags |= OFN_HIDEREADONLY;

	//	Done

	if (::GetOpenFileName(&of))
		{
		*retsFilenames = CString(of.lpstrFile);
		return true;
		}
	else
		return false;
	}

bool uiGetSaveFilename (HWND hParent,
						const CString &sExtensions,
						const CString &sFolder,
						CString *retsFilename)

//	uiGetSaveFilename
//
//	Puts up a Save As dialog box. Returns FALSE if the user cancels

	{
	OPENFILENAME of;
	char szBuffer[8192];

	int iFirstExtPos;
	CString sFilter = FormatFilterString(sExtensions, &iFirstExtPos);

	::ZeroMemory(&of, sizeof(of));
	of.lStructSize = sizeof(of);
	of.hwndOwner = hParent;
	of.lpstrFilter = sFilter.GetASCIIZ();
	of.lpstrFile = szBuffer;
	szBuffer[0] = '\0';
	of.nMaxFile = sizeof(szBuffer)-1;
	of.lpstrInitialDir = sFolder.GetASCIIZ();
	of.Flags = 0;
	of.Flags |= OFN_EXPLORER;
	of.Flags |= OFN_HIDEREADONLY;
	of.Flags |= OFN_OVERWRITEPROMPT;
	of.Flags |= OFN_PATHMUSTEXIST;
	of.lpstrDefExt = sFilter.GetASCIIZ() + iFirstExtPos;

	//	Done

	if (::GetSaveFileName(&of))
		{
		*retsFilename = CString(of.lpstrFile);
		return true;
		}
	else
		return false;
	}

CString FormatFilterString (const CString &sExtensions, int *retiFirstExtPos)

//	FormatFilterString
//
//	Takes a string of the form:
//
//		Name1;Ext1;Name2;Ext2...
//
//	And returns a string suitable for using in an OPENFILENAME struct

	{
	CString sResult;
	char *pSrc = sExtensions.GetASCIIZ();
	char *pDest = sResult.GetWritePointer(sExtensions.GetLength() + 1);
	int iFirstExtPos = -1;

	while (*pSrc)
		{
		if (*pSrc == ';')
			{
			*pDest = '\0';

			if (iFirstExtPos == -1)
				iFirstExtPos = (pSrc + 1 - sExtensions.GetASCIIZ());
			}
		else
			*pDest = *pSrc;

		pDest++;
		pSrc++;
		}

	*pDest = '\0';

	if (retiFirstExtPos)
		*retiFirstExtPos = iFirstExtPos + 2;

	return sResult;
	}
