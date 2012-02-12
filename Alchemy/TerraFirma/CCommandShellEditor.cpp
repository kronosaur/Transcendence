//	CCommandShellEditor.cpp
//
//	Implements CCommandShellEditor object.
//
//	Data Format:
//
//		string
//
//		The data format is a single string that represents the
//		contents of the entire buffer. Line breaks are denoted by
//		a /r/n sequence.

#include "Alchemy.h"
#include "TerraFirma.h"

CCommandShellEditor::CCommandShellEditor (void)

//	CCommandShellEditor constructor

	{
	}

CCommandShellEditor::~CCommandShellEditor (void)

//	CCommandShellEditor destructor

	{
	}

void CCommandShellEditor::CleanUp (void)

//	Destroy
//
//	Destroys the current node editor

	{
	m_CommandLineWnd.Destroy();
	}

ICCItem *CCommandShellEditor::GetNodeData (void)

//	GetNodeData
//
//	Returns the contents of the editor

	{
	CString sBuffer;

	sBuffer = m_CommandLineWnd.GetData();
	return GetCC()->CreateString(sBuffer);
	}

ALERROR CCommandShellEditor::Initialize (HWND hParent, RECT rcRect)

//	Initialize
//
//	Creates a new node editor.

	{
	ALERROR error;

	//	Create the command shell window

	if (error = m_CommandLineWnd.Create(GetTF(), hParent, rcRect))
		return error;

	//	Set the focus

	m_CommandLineWnd.SetFocus();

	return NOERROR;
	}

ICCItem *CCommandShellEditor::SetNodeData (ICCItem *pNodeData)

//	SetNodeData
//
//	Replaces the contents of the editor

	{
	CString sBuffer;

	sBuffer = pNodeData->GetStringValue();
	m_CommandLineWnd.SetData(sBuffer);
	return GetCC()->CreateTrue();
	}
