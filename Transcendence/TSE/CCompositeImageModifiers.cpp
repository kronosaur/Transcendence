//	CCompositeImageModifiers.cpp
//
//	CCompositeImageModifiers class
//	Copyright (c) 2013 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"

static IEffectPainter *g_pMediumDamage = NULL;
static IEffectPainter *g_pLargeDamage = NULL;

bool CCompositeImageModifiers::operator== (const CCompositeImageModifiers &Val) const

//	operator ==

	{
	return (m_wFadeColor == Val.m_wFadeColor
			&& m_wFadeOpacity == Val.m_wFadeOpacity
			&& m_fStationDamage == Val.m_fStationDamage);
	}

void CCompositeImageModifiers::Apply (CObjectImageArray *retImage) const

//	Apply
//
//	Apply modifiers to the image.

	{
	RECT rcNewImage;
	CG16bitImage *pNewDest = NULL;

	//	Station damage

	if (m_fStationDamage)
		{
		InitDamagePainters();

		//	Create a blank bitmap

		if (pNewDest == NULL)
			pNewDest = CreateCopy(retImage, &rcNewImage);

		//	Add some large damage

		int iCount = (pNewDest->GetWidth() / 32) * (pNewDest->GetHeight() / 32);
		PaintDamage(*pNewDest, rcNewImage, iCount, g_pLargeDamage);

		//	Add some medium damage

		iCount = (pNewDest->GetWidth() / 4) + (pNewDest->GetHeight() / 4);
		PaintDamage(*pNewDest, rcNewImage, iCount, g_pMediumDamage);
		}

	//	Apply wash on top

	if (m_wFadeOpacity != 0)
		{
		//	Create a blank bitmap

		if (pNewDest == NULL)
			pNewDest = CreateCopy(retImage, &rcNewImage);

		//	Fill with color via our own mask

		pNewDest->FillMask(0, 
				0,
				pNewDest->GetWidth(),
				pNewDest->GetHeight(),
				*pNewDest,
				m_wFadeColor,
				0,
				0,
				(BYTE)m_wFadeOpacity);
		}

	//	Replace the image (the result takes ownership of our image).

	if (pNewDest)
		{
		if (!retImage->HasAlpha())
			pNewDest->SetTransparentColor();

		retImage->Init(pNewDest, rcNewImage, 0, 0, true);
		}
	}

CG16bitImage *CCompositeImageModifiers::CreateCopy (CObjectImageArray *pImage, RECT *retrcNewImage) const

//	CreateCopy
//
//	Creates a copy of the given image. Caller is reponsible for freeing.

	{
	RECT rcImage = pImage->GetImageRect();
	int cxWidth = RectWidth(rcImage);
	int cyHeight = RectHeight(rcImage);
	CG16bitImage *pNewImage = new CG16bitImage;
	pNewImage->CreateBlank(cxWidth, cyHeight, true);

	rcImage.left = 0;
	rcImage.top = 0;
	rcImage.right = cxWidth;
	rcImage.bottom = cyHeight;

	//	Start with undamaged image

	pImage->CopyImage(*pNewImage,
			0,
			0,
			0,
			0);

	if (retrcNewImage)
		*retrcNewImage = rcImage;

	return pNewImage;
	}

void CCompositeImageModifiers::InitDamagePainters (void)

//	InitDamagePainters
//
//	Initializes station damage bitmaps

	{
	if (g_pMediumDamage == NULL)
		{
		CEffectCreator *pEffect = g_pUniverse->FindEffectType(MEDIUM_STATION_DAMAGE_UNID);
		if (pEffect)
			g_pMediumDamage = pEffect->CreatePainter(CCreatePainterCtx());
		}

	if (g_pLargeDamage == NULL)
		{
		CEffectCreator *pEffect = g_pUniverse->FindEffectType(LARGE_STATION_DAMAGE_UNID);
		if (pEffect)
			g_pLargeDamage = pEffect->CreatePainter(CCreatePainterCtx());
		}
	}

void CCompositeImageModifiers::PaintDamage (CG16bitImage &Dest, const RECT &rcDest, int iCount, IEffectPainter *pPainter)

//	PaintDamage
//
//	Applies damage to the image

	{
	if (pPainter == NULL)
		return;

	//	Get some data

	int iVariantCount = pPainter->GetVariants();
	if (iVariantCount == 0)
		return;

	int iLifetime = pPainter->GetLifetime();
	if (iLifetime == 0)
		return;

	//	Create context

	SViewportPaintCtx Ctx;

	//	Paint damage

	for (int i = 0; i < iCount; i++)
		{
		//	Pick a random position

		int x = rcDest.left + mathRandom(0, RectWidth(rcDest)-1);
		int y = rcDest.top + mathRandom(0, RectHeight(rcDest)-1);

		//	Set some parameters

		Ctx.iTick = mathRandom(0, iLifetime - 1);
		Ctx.iVariant = mathRandom(0, iVariantCount - 1);
		Ctx.iRotation = mathRandom(0, 359);
		Ctx.iDestiny = mathRandom(0, 359);

		//	Paint

		pPainter->Paint(Dest, x, y, Ctx);
		}
	}

void CCompositeImageModifiers::Reinit (void)

//	Reinit
//
//	Reinitialize global data

	{
	if (g_pMediumDamage)
		{
		g_pMediumDamage->Delete();
		g_pMediumDamage = NULL;
		}

	if (g_pLargeDamage)
		{
		g_pLargeDamage->Delete();
		g_pLargeDamage = NULL;
		}
	}

