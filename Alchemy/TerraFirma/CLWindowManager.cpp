//	CLWindowManager.cpp
//
//	Implements CLWindowManager object

#include "Alchemy.h"
#include "TerraFirma.h"
#include "WindowsExt.h"

CLWindowManager::CLWindowManager (CTerraFirma *pTF) :
		m_pTF(pTF),
		m_pCC(pTF->GetCC()),
		m_WindowList(FALSE)

//	CLWindowManager constructor

	{
	}

ICCItem *CLWindowManager::CreateLWindow (CLWindowOptions *pOptions, CLSizeOptions *pSize, ICCItem *pController)

//	CLWindowManager
//
//	Create a new window

	{
	CLWindow *pNewWindow;
	ICCItem *pError;

	pNewWindow = new CLWindow(this);
	if (pNewWindow == NULL)
		return m_pCC->CreateMemoryError();

	//	Initialize

	pError = pNewWindow->CreateItem(m_pCC, pOptions, pSize, pController);
	if (pError->IsError())
		return pError;

	pError->Discard(m_pCC);

	//	Add the window to the window manager's list

	m_WindowList.AppendObject(pNewWindow, NULL);

	return pNewWindow->Reference();
	}

void CLWindowManager::DestroyLWindow (CLWindow *pWindow)

//	DestroyWindow
//
//	Removes the window from the window manager's list. This method
//	should only be called by CLWindow to remove itself from the
//	list.

	{
	int iIndex = FindWindow(pWindow);

	ASSERT(iIndex != -1);
	m_WindowList.RemoveObject(iIndex);
	}

void CLWindowManager::GetRect (RECT *retrcRect)

//	GetRect
//
//	Returns the size of the main window

	{
	GetClientRect(GetTF()->GetHWND(), retrcRect);
	}

void CLWindowManager::WindowSize (RECT *pRect)

//	WindowSize
//
//	This method is called when the main window's size changes. This
//	code is responsible for resizing all child windows

	{
	}
