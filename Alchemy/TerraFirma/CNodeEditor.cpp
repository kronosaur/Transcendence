//	CNodeEditor.cpp
//
//	Implements CNodeEditor object. This object is an abstract object
//	that defines the interaction of an editor with the main program.

#include "Alchemy.h"
#include "TerraFirma.h"

CNodeEditor::CNodeEditor (void) :
		m_hParent(NULL),
		m_pTF(NULL)

//	CNodeEditor constructor

	{
	}

CNodeEditor::~CNodeEditor (void)

//	CNodeEditor destructor

	{
	}

ALERROR CNodeEditor::Create (CTerraFirma *pTF, HWND hParent, RECT rcRect)

//	Create
//
//	Creates a new node editor.

	{
	ALERROR error;

	//	Remember our parent

	m_pTF = pTF;
	m_hParent = hParent;
	m_rcRect = rcRect;

	//	Let our subclass initialize itself

	if (error = Initialize(hParent, rcRect))
		return error;

	return NOERROR;
	}

void CNodeEditor::Destroy (void)

//	Destroy
//
//	Destroys the current node editor

	{
	//	Let our subclass clean up

	CleanUp();
	}

CCodeChain *CNodeEditor::GetCC (void)

//	GetCC
//
//	Return pointer to CCodeChain object

	{ return m_pTF->GetCC(); }

CLibraryFile *CNodeEditor::GetLibrary (void)

//	GetLibrary
//
//	Return pointer to CLibrary object

	{ return m_pTF->GetLibrary(); }

CTerraFirma *CNodeEditor::GetTF (void)

//	GetTF
//
//	Return pointer to CTerraFirma object

	{ return m_pTF; }
