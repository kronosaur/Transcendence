//	CButtonBarData.cpp
//
//	CButtonBarData class

#include "PreComp.h"
#include "Transcendence.h"

const int BUTTON_IMAGE_WIDTH =							128;
const int BUTTON_IMAGE_HEIGHT =							64;

const int MEDIUM_BUTTON_IMAGE_WIDTH =					64;
const int MEDIUM_BUTTON_IMAGE_HEIGHT =					64;

CButtonBarData::CButtonBarData (void) : m_iCount(0)

//	CButtonBarData constructor

	{
	}

CButtonBarData::~CButtonBarData (void)

//	CButtonBarData destructor

	{
	CleanUp();
	}

void CButtonBarData::AddButton (DWORD dwCmdID,
								const CString &sLabel,
								const CString &sDescription,
								const CString &sKey,
								int iImageIndex,
								AlignmentTypes iAlign,
								StyleTypes iStyle)

//	AddButton
//
//	Add a button

	{
	if (m_iCount < MAX_BUTTONS)
		{
		m_Buttons[m_iCount].dwCmdID = dwCmdID;
		m_Buttons[m_iCount].sLabel = sLabel;
		m_Buttons[m_iCount].sDescription = sDescription;
		m_Buttons[m_iCount].sKey = sKey;
		m_Buttons[m_iCount].iImageIndex = iImageIndex;
		m_Buttons[m_iCount].iAlign = iAlign;
		m_Buttons[m_iCount].iStyle = iStyle;

		m_Buttons[m_iCount].dwFlags = FLAG_VISIBLE;

		m_iCount++;
		}
	}

void CButtonBarData::CleanUp (void)

//	CleanUp
//
//	Clean up the data

	{
	m_Images.CleanUp();
	m_iCount = 0;
	}

int CButtonBarData::FindButton (const CString &sKey)

//	FindButton
//
//	Find a button by key and return the index (-1 if not found)

	{
	return -1;
	}

void CButtonBarData::GetImageSize (int iIndex, RECT *retrcRect)

//	GetImageSize
//
//	Returns the size of the image

	{
	switch (m_Buttons[iIndex].iStyle)
		{
		case styleMedium:
			retrcRect->left = 0;
			retrcRect->top = 0;
			retrcRect->right = MEDIUM_BUTTON_IMAGE_WIDTH;
			retrcRect->bottom = MEDIUM_BUTTON_IMAGE_HEIGHT;
			break;

		default:
			retrcRect->left = 0;
			retrcRect->top = 0;
			retrcRect->right = BUTTON_IMAGE_WIDTH;
			retrcRect->bottom = BUTTON_IMAGE_HEIGHT;
		}
	}

int CButtonBarData::HitTest (int x, int y)

//	HitTest
//
//	Find the button at the given coordinates (-1 if none)

	{
	return -1;
	}

ALERROR CButtonBarData::Init (void)

//	Init
//
//	Initialize button bar object (this loads the default images)

	{
	ALERROR error;

	//	Load images

	HBITMAP hDIB;
	if (error = JPEGLoadFromResource(NULL,
			MAKEINTRESOURCE(IDR_GAME_BUTTONS_IMAGE),
			JPEG_LFR_DIB, 
			NULL, 
			&hDIB))
		return error;

	bool bSuccess = m_Images.CreateFromBitmap(hDIB);
	::DeleteObject(hDIB);
	if (!bSuccess)
		return ERR_FAIL;

	return NOERROR;
	}

