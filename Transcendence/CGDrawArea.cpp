//	CGDrawArea.cpp
//
//	CGDrawArea class

#include "PreComp.h"
#include "Transcendence.h"

CGDrawArea::CGDrawArea (void) :
		m_rgbBackColor(0, 0, 0),
		m_bTransBackground(true)

//	CGDrawArea constructor

	{
	}

void CGDrawArea::CreateImage (void)

//	CreateImage
//
//	Create the offscreen image

	{
	if (m_Image.IsEmpty())
		m_Image.Create(RectWidth(GetRect()), RectHeight(GetRect()), CG32bitImage::alpha8);
	}

void CGDrawArea::Paint (CG32bitImage &Dest, const RECT &rcRect)

//	Paint
//
//	Handle paint

	{
	DEBUG_TRY

	Dest.Blt(0,
			0,
			m_Image.GetWidth(),
			m_Image.GetHeight(),
			255,
			m_Image,
			rcRect.left,
			rcRect.top);

	DEBUG_CATCH
	}
