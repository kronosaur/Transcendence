//	EffectPerformanceTest.cpp
//
//	Test effect painting performance
//	Copyright (c) 2016 by Kronosaur Productions, LLC. All Rights Reserved.

#include <stdio.h>

#include <windows.h>
#include "Alchemy.h"
#include "XMLUtil.h"
#include "TransData.h"

#define COUNT_ATTRIB    					CONSTLIT("count")
#define UNID_ATTRIB	    					CONSTLIT("unid")

void DoEffectPerformanceTest (CUniverse &Universe, CXMLElement *pCmdLine)
    {
    DWORD dwEffectUNID = pCmdLine->GetAttributeInteger(UNID_ATTRIB);
    CDesignType *pType = Universe.FindDesignType(dwEffectUNID);
    if (pType == NULL)
        {
        printf("Unable to find type: %08x.", dwEffectUNID);
        return;
        }

    int iSeconds = pCmdLine->GetAttributeIntegerBounded(COUNT_ATTRIB, 1, -1, 5);
    DWORD dwTiming = iSeconds * 1000;

    CItemType *pItemType = CItemType::AsType(pType);
    CDeviceClass *pDeviceClass = (pItemType ? pItemType->GetDeviceClass() : NULL);
    CWeaponClass *pWeaponClass = (pDeviceClass ? pDeviceClass->AsWeaponClass() : NULL);

    //  We need an effect painter. If the UNID is a weapon, then get the 
    //  projectile effect.

    IEffectPainter *pPainter;
    if (pWeaponClass)
        pPainter = pWeaponClass->GetWeaponFireDesc(CItemCtx(CItem(pItemType, 1)))->CreateEffectPainter(SShotCreateCtx());
    else
        pPainter = NULL;

    //  If no painter, then we can't find the effect

    if (pPainter == NULL)
        {
        printf("Unable to find effect: %08x.", dwEffectUNID);
        return;
        }

    //  Prepare an output bitmap

    int cxWidth = 1024;
    int cyHeight = 1024;
    CG32bitImage Output;
    if (!Output.Create(cxWidth, cyHeight))
        {
        printf("Unable to create output image.");
        return;
        }

    //  Prepare paint context

    SViewportPaintCtx PaintCtx;
	PaintCtx.iTick = 0;
    PaintCtx.iVariant = 0;
    PaintCtx.iRotation = 90;
    PaintCtx.iDestiny = 0;
    PaintCtx.xCenter = cxWidth / 2;
    PaintCtx.yCenter = cyHeight / 2;
	PaintCtx.XForm = ViewportTransform(PaintCtx.vCenterPos, g_KlicksPerPixel, PaintCtx.xCenter, PaintCtx.yCenter);
	PaintCtx.XFormRel = PaintCtx.XForm;

    //  Paint to the output once and copy to clipboard so that callers can
    //  see what we're painting.

    pPainter->Paint(Output, PaintCtx.xCenter, PaintCtx.yCenter, PaintCtx);
    OutputImage(Output, NULL_STR);

    //  Count the number of times we blt in n seconds

    DWORD dwStart = ::GetTickCount();
    int iCount = 0;
    do
        {
        pPainter->Paint(Output, PaintCtx.xCenter, PaintCtx.yCenter, PaintCtx);
        iCount++;
        }
    while (sysGetTicksElapsed(dwStart) < dwTiming);

    //  Output timing

    DWORD dwElapsed = ::sysGetTicksElapsed(dwStart);
    Metric rPerMS = 1000.0 / ((Metric)dwElapsed / iCount);
    printf("%.2f paints per second.\n", rPerMS);
    }
