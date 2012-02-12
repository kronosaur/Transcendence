//	MainWnd.cpp
//
//	Main window proc

#include "Alchemy.h"
#include "TerraFirma.h"

//	Forwards

LONG WMCommand (HWND hWnd, int iNotifyCode, int iID, HWND hCtrlWnd);
LONG WMCreate (HWND hWnd, LPCREATESTRUCT pCreate);
LONG WMDestroy (HWND hWnd);

//	Inlines

inline CTerraFirma *GetTF (HWND hWnd) { return (CTerraFirma *)GetWindowLong(hWnd, GWL_USERDATA); }
inline void StoreTF (HWND hWnd, CTerraFirma *pTF) { SetWindowLong(hWnd, GWL_USERDATA, (LONG)pTF); }

LONG APIENTRY MainWndProc (HWND hWnd, UINT message, UINT wParam, LONG lParam)

//	MainWndProc
//
//	Main window proc

	{
	switch (message)
		{
		case WM_COMMAND:
			return WMCommand(hWnd, HIWORD(wParam), LOWORD(wParam), (HWND)lParam);

		case WM_CREATE:
			return WMCreate(hWnd, (LPCREATESTRUCT)lParam);

		case WM_DESTROY:
			return WMDestroy(hWnd);

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}

LONG WMCommand (HWND hWnd, int iNotifyCode, int iID, HWND hCtrlWnd)

//	WMCommand
//
//	Handle WM_COMMAND

	{
	//	If this is a notification from an editor, let the editor
	//	handle it

	if (hCtrlWnd)
		return GetTF(hWnd)->HandleEditorNotification(iNotifyCode);

	return 0;
	}

LONG WMCreate (HWND hWnd, LPCREATESTRUCT pCreate)

//	WMCreate
//
//	Handle WM_CREATE

	{
	ALERROR error;
	CTerraFirma *pTF = NULL;

	//	Allocate the CTerraFirma object

	pTF = new CTerraFirma(hWnd);
	if (pTF == NULL)
		goto Fail;

	//	Store the pointer in the window structure so that
	//	we can get at it

	StoreTF(hWnd, pTF);

	//	Boot it up

	if (error = pTF->Boot(LITERAL("")))
		{
		pTF->DisplayError(error);
		goto Fail;
		}

	return 0;

Fail:

	if (pTF)
		{
		delete pTF;
		StoreTF(hWnd, NULL);
		}

	return -1;
	}

LONG WMDestroy (HWND hWnd)

//	WMDestroy
//
//	Handle WM_DESTROY

	{
	ALERROR error;
	CTerraFirma *pTF = GetTF(hWnd);

	//	Free the CTerraFirma object

	if (pTF)
		{
		if (error = pTF->Shutdown())
			pTF->DisplayError(error);

		delete pTF;
		StoreTF(hWnd, NULL);
		}

	//	Quit app

    PostQuitMessage(1);
	return 0;
	}
