//	CLArea.cpp
//
//	Implements the CLArea object

#include "Alchemy.h"
#include "TerraFirma.h"
#include "WindowsExt.h"

static CObjectClass<CLArea>g_Class(OBJID_CLAREA, NULL);

CLArea::CLArea (void) : CObject(&g_Class)

//	CLArea constructor

	{
	}

CLArea::CLArea (IObjectClass *pClass, CLWindow *pWindow, int iID, ICCItem *pController) : CObject(pClass),
		m_pWindow(pWindow),
		m_iID(iID),
		m_pController(NULL),
		m_fHidden(FALSE),
		m_fDisabled(FALSE),
		m_fReadOnly(FALSE),
		m_fModified(FALSE),
		m_fFirstShow(FALSE)

//	CLArea constructor

	{
	if (pController)
		m_pController = pController->Reference();
	}

ICCItem *CLArea::Create (CLSizeOptions *pSize)

//	Create
//
//	Creates the new area and adds it to the window

	{
	RECT rcParent;
	ICCItem *pError;

	//	Figure out our size

	m_pWindow->GetRect(&rcParent);
	pSize->CalcSize(&rcParent, &m_rcRect);

	//	Let our subclass do some initialization

	pError = CreateNotify();
	if (pError->IsError())
		return pError;

	pError->Discard(GetCC());

	//	Add ourselves to the window list

	m_pWindow->AddArea(this);

	//	Done

	return GetCC()->CreateTrue();
	}

CCodeChain *CLArea::GetCC (void)

//	GetCC
//
//	Returns CodeChain object

	{
	return m_pWindow->GetWM()->GetCC();
	}

ICCItem *CLArea::GetData (void)

//	GetData
//
//	Return the object being displayed by this area

	{
	return GetCC()->CreateNil();
	}

CTerraFirma *CLArea::GetTF (void)

//	GetTF
//
//	Returns CTerraFirma object

	{
	return m_pWindow->GetWM()->GetTF();
	}

ICCItem *CLArea::SetData (ICCItem *pData)

//	SetData
//
//	Sets the object being displayed by this area.

	{
	return GetCC()->CreateNil();
	}

void CLArea::SetSize (CLSizeOptions *pOptions)

//	SetSize
//
//	Sets the size of the area with respect to its parent window

	{
	}

void CLArea::ShowArea (void)

//	ShowArea
//
//	Shows an area if it is currently hidden

	{
	}

void CLArea::WindowSize (RECT *pRect)

//	WindowSize
//
//	Adjusts the size of the area when the parent's size changes

	{
	}
