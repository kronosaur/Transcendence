//	CLImageArea.cpp
//
//	Implements the CLImageArea object

#include "Alchemy.h"
#include "TerraFirma.h"
#include "WindowsExt.h"

static CObjectClass<CLImageArea>g_Class(OBJID_CLIMAGEAREA, NULL);

CLImageArea::CLImageArea (void) : CLArea(&g_Class, NULL, 0, NULL)

//	CLArea constructor

	{
	}

CLImageArea::CLImageArea (CLWindow *pWindow, int iID, ICCItem *pController) 
		: CLArea(&g_Class, pWindow, iID, pController),
		m_pData(NULL),
		m_hImage(NULL)

//	CLImageArea constructor

	{
	}

CLImageArea::~CLImageArea (void)

//	CLImageArea destructor

	{
	if (m_pData)
		m_pData->Discard(GetCC());

	if (m_hImage)
		DeleteObject(m_hImage);
	}

ICCItem *CLImageArea::GetData (void)

//	GetData
//
//	Returns the data being displayed

	{
	return GetCC()->CreateNil();
	}

void CLImageArea::PaintNeeded (HDC hDC, RECT *pUpdate)

//	PaintNeeded
//
//	Paint the area

	{
	if (m_hImage)
		{
		RECT rcDest;
		RECT rcRect;

		//	Center the image in the area

		rcDest.left = m_rcRect.left + (RectWidth(&m_rcRect) - RectWidth(&m_rcImage)) / 2;
		rcDest.top = m_rcRect.top + (RectHeight(&m_rcRect) - RectHeight(&m_rcImage)) / 2;
		rcDest.right = rcDest.left + RectWidth(&m_rcImage);
		rcDest.bottom = rcDest.top + RectHeight(&m_rcImage);

		//	Figure out the actual rect being painted. If we intersect
		//	the update area, then blt

		if (IntersectRect(&rcRect, &rcDest, pUpdate))
			{
			HBITMAP hOldBitmap = SelectObject(m_pWindow->GetTempDC(), m_hImage);
			BitBlt(hDC,
					rcRect.left,
					rcRect.top,
					RectWidth(&rcRect),
					RectHeight(&rcRect),
					m_pWindow->GetTempDC(),
					rcDest.left - rcRect.left,
					rcDest.top - rcRect.top,
					SRCCOPY);
			SelectObject(m_pWindow->GetTempDC(), hOldBitmap);
			}
		}
	}

ICCItem *CLImageArea::SetData (ICCItem *pData)

//	SetData
//
//	Sets the data being displayed

	{
	ALERROR error;
	CCodeChain *pCC = GetCC();
	ICCItem *pItem;
	CCVector *pBits;
	HBITMAP hBitmap;
	BOOL bOk;
	RECT rcImage;

	//	Get the size

	pItem = GetTF()->ObjMethod(pData, METHOD_IGetImageRect, NULL, NULL, NULL);
	if (pItem->IsError())
		return pItem;

	bOk = CLSizeOptions::ConvertItemToRect(pItem, &rcImage);
	pItem->Discard(pCC);
	if (!bOk)
		return pCC->CreateError(LITERAL("Invalid image size"), NULL);

	//	Get the actual bits

	pItem = GetTF()->ObjMethod(pData, METHOD_IGetRawImage, NULL, NULL, NULL);
	if (pItem->IsError())
		return pItem;

	//	Convert to a vector

	pBits = dynamic_cast<CCVector *>(pItem);
	if (pBits == NULL)
		{
		pItem->Discard(pCC);
		return pCC->CreateError(LITERAL("Invalid raw image"), NULL);
		}

	//	Convert to a DDB

	if (error = rawConvertToDDB(RectWidth(&rcImage),
			RectHeight(&rcImage),
			(RAWPIXEL *)pBits->GetArray(),
			0,
			NULL,
			&hBitmap))
		{
		pBits->Discard(pCC);
		return pCC->CreateSystemError(error);
		}

	//	Free any old bitmaps

	if (m_hImage)
		DeleteObject(m_hImage);

	//	Remember the new data

	m_hImage = hBitmap;
	m_rcImage = rcImage;

	//	Repaint

	m_pWindow->UpdateRect(&m_rcRect);

	//	Done
	
	return pCC->CreateTrue();
	}
