//	CImageEditor.cpp
//
//	Implements CImageEditor object.
//
//	Data Format:
//
//		(format regions image-data)
//
//		format: This is an ordered list of items, each item representing
//		a particular option for the node list. The list has the following
//		structure:
//
//			(width height mask)
//
//		The width is the width of the image in pixels; height is the height
//		in pixels. If mask is not Nil, then the transparency data represents
//		a solid mask (values are either 0 or 255, nothing in between). If Nil,
//		the transparency-data can have intermediate values.
//
//		regions: This is a list of regions within the image that can be
//		accessed separately. The list has the following format:
//
//			((x y width height) (x y width height) ...)
//
//		image-data: The actual image data consists of a two-dimensional
//		array of pixels. Each pixel is four sequential bytes, one each for
//		red, green, and blue, plus a transparency byte. The transparency
//		byte is used to paint the image on top of other images. A value of 
//		255 indicates that the background shows through. A value of 0 indicates
//		that the image is 100% solid. Intermediate values mix the image with the
//		background.

#include "Alchemy.h"
#include "TerraFirma.h"

CImageEditor::CImageEditor (void) :
		m_pNodeData(NULL)

//	CImageEditor constructor

	{
	}

CImageEditor::~CImageEditor (void)

//	CImageEditor destructor

	{
	CleanUp();
	}

void CImageEditor::CleanUp (void)

//	Destroy
//
//	Destroys the current node editor

	{
	if (m_pNodeData)
		{
		m_pNodeData->Discard(GetCC());
		m_pNodeData = NULL;
		}
	}

ICCItem *CImageEditor::GetNodeData (void)

//	GetNodeData
//
//	Returns the contents of the editor

	{
	if (m_pNodeData)
		return m_pNodeData->Reference();
	else
		return GetCC()->CreateNil();
	}

ALERROR CImageEditor::Initialize (HWND hParent, RECT rcRect)

//	Initialize
//
//	Creates a new node editor.

	{
	ALERROR error;

	//	Create the window

	if (error = m_Window.Create(this, hParent, &rcRect))
		return error;

	//	Set the focus

	m_Window.SetFocus();

	return NOERROR;
	}

LONG CImageEditor::Paint (HDC hDC, RECT *pUpdateRect)

//	Paint
//
//	Paints the given rect

	{
	return 0;
	}

ICCItem *CImageEditor::SetNodeData (ICCItem *pNodeData)

//	SetNodeData
//
//	Replaces the contents of the editor

	{
	//	Lose the old reference

	if (m_pNodeData)
		m_pNodeData->Discard(GetCC());

	//	Keep a reference to the data

	m_pNodeData = pNodeData->Reference();

	//	Done

	return GetCC()->CreateTrue();
	}
