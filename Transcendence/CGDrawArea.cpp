//	CGDrawArea.cpp
//
//	CGDrawArea class

#include "PreComp.h"
#include "Transcendence.h"

CGDrawArea::CGDrawArea (void) :
		m_wBackColor(0),
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
		m_Image.CreateBlank(RectWidth(GetRect()), RectHeight(GetRect()), false);
	}

void CGDrawArea::Paint (CG16bitImage &Dest, const RECT &rcRect)

//	Paint
//
//	Handle paint

	{
	if (m_bTransBackground)
		{
		Dest.ColorTransBlt(0,
				0,
				m_Image.GetWidth(),
				m_Image.GetHeight(),
				255,
				m_Image,
				rcRect.left,
				rcRect.top);
		}
	else
		{
		Dest.Blt(0,
				0,
				m_Image.GetWidth(),
				m_Image.GetHeight(),
				m_Image,
				rcRect.left,
				rcRect.top);
		}
	}
