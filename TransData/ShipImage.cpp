//	ShipImage.cpp
//
//	Generate basic stats about ship classes

#include <stdio.h>

#include <windows.h>
#include "Alchemy.h"
#include "XMLUtil.h"
#include "TransData.h"

const int X_MARGIN = 50;
const int Y_MARGIN = 50;

void GenerateShipImage (CUniverse &Universe, CXMLElement *pCmdLine)
	{
	int i;

	//	Ship to output

	CString sUNID = pCmdLine->GetAttribute(CONSTLIT("unid"));
	CShipClass *pClass = g_pUniverse->FindShipClass(strToInt(sUNID, 0));
	if (pClass == NULL)
		{
		printf("Unknown ship class: %s\n", sUNID.GetASCIIZPointer());
		return;
		}

	//	Output file

	CString sFilespec = pCmdLine->GetAttribute(CONSTLIT("output"));
	if (!sFilespec.IsBlank())
		sFilespec = pathAddExtensionIfNecessary(sFilespec, CONSTLIT(".bmp"));

	//	Options

	bool bGrid = pCmdLine->GetAttributeBool(CONSTLIT("grid"));
	bool bDriveImages = pCmdLine->GetAttributeBool(CONSTLIT("driveimages"));
	bool bPortPos = pCmdLine->GetAttributeBool(CONSTLIT("portPos"));
	bool bWeaponPos = pCmdLine->GetAttributeBool(CONSTLIT("weaponpos"));
	bool bInterior = pCmdLine->GetAttributeBool(CONSTLIT("interior"));
	bool bAngles = pCmdLine->GetAttributeBool(CONSTLIT("angles"));

	if (pCmdLine->GetAttributeBool(CONSTLIT("debug")))
		{
		bDriveImages = true;
		bPortPos = true;
		bWeaponPos = true;
		bInterior = true;
		}

	//	Font for text

	CString sTypeface;
	int iSize;
	bool bBold;
	bool bItalic;

	if (!CG16bitFont::ParseFontDesc(pCmdLine->GetAttribute(CONSTLIT("font")),
			&sTypeface,
			&iSize,
			&bBold,
			&bItalic))
		{
		sTypeface = CONSTLIT("Arial");
		iSize = 10;
		bBold = false;
		bItalic = false;
		}

	CG16bitFont TextFont;
	TextFont.Create(sTypeface, -PointsToPixels(iSize), bBold, bItalic);
	CG32bitPixel rgbNameColor = CG32bitPixel(255, 255, 255);

	//	How many rotations do we need to output?

	int iStartRotation = 0;
	int iRotationCount = pClass->GetImage().GetRotationCount();

	//	Compute the size of each cell

	RECT rcRect = pClass->GetImage().GetImageRect();
	int cxWidth = RectWidth(rcRect) + X_MARGIN;
	int cyHeight = RectHeight(rcRect) + Y_MARGIN;

	//	Create the resulting image

	CImageGrid Output;
	Output.Create(iRotationCount, cxWidth, cyHeight);

	//	Create a rotation structure (to track ship thrusters)

	CIntegralRotation Rotation;
	Rotation.Init(pClass->GetIntegralRotationDesc());

	//	Paint

	SViewportPaintCtx Ctx;
	for (i = 0; i < iRotationCount; i++)
		{
		CViewportPaintCtxSmartSave Save(Ctx);

		int x, y;

		Output.GetCellCenter(i, &x, &y);

		//	Set the ship rotation

		Rotation.SetRotationAngle(pClass->GetIntegralRotationDesc(), pClass->GetIntegralRotationDesc().GetRotationAngle(i));

		//	Set the paint context (we need to do this after the rotation setting)

		Ctx.iTick = 0;
		Ctx.iVariant = Rotation.GetFrameIndex();
		Ctx.iRotation = Rotation.GetRotationAngle(pClass->GetIntegralRotationDesc());
		Ctx.iDestiny = 0;
		Ctx.iMaxLength = -1;

		//	Since we don't have an object, we use the viewport center to indicate
		//	the center of the object.

		Ctx.xCenter = x;
		Ctx.yCenter = y;

		//	Create an effects structure

		CObjectEffectList Effects;
		pClass->InitEffects(NULL, &Effects);

		//	Paint thrusters behind ship

		Ctx.bInFront = false;
		if (bDriveImages)
			Effects.PaintAll(Ctx, pClass->GetEffectsDesc(), Output.GetImage(), x, y);

		//	Paint the ship

		pClass->Paint(Output.GetImage(), 
				x, 
				y, 
				Ctx.XForm,
				i, 
				0,
				bDriveImages);

		//	Paint thrusters in front of ship

		Ctx.bInFront = true;
		if (bDriveImages)
			Effects.PaintAll(Ctx, pClass->GetEffectsDesc(), Output.GetImage(), x, y);

		//	Paint the center

		Output.GetImage().DrawDot(x, y, CG32bitPixel(0, 255, 255), markerMediumCross);

		//	Internal compartments

		if (bInterior)
			{
			pClass->PaintInteriorCompartments(Output.GetImage(), x, y, Ctx.iRotation);
			}

		//	Paint weapon positions

		if (bWeaponPos)
			{
			CDeviceDescList Devices;
			pClass->GenerateDevices(pClass->GetLevel(), Devices);

			pClass->PaintDevicePositions(Output.GetImage(), x, y, Devices, Ctx.iRotation);
			}

		//	Docking ports

		if (bPortPos)
			{
			pClass->PaintDockPortPositions(Output.GetImage(), x, y, Ctx.iRotation);
			}

		//	If requested, paint a polar grid

		if (bGrid)
			{
			int iAngle;
			int iRadius;
			int iScale = pClass->GetImageViewportSize();

			int iMaxRadius = cxWidth / 2;
			int iRadiusInc = 20;
			for (iRadius = 0; iRadius <= iMaxRadius; iRadius += 20)
				{
				int xLastPos;
				int yLastPos;

				C3DConversion::CalcCoord(iScale, 0, iRadius, 0, &xLastPos, &yLastPos);

				for (iAngle = 1; iAngle <= 360; iAngle++)
					{
					int xPos;
					int yPos;
					C3DConversion::CalcCoord(iScale, iAngle, iRadius, 0, &xPos, &yPos);

					Output.GetImage().DrawLine(x + xLastPos, y + yLastPos, x + xPos, y + yPos, 1, CG32bitPixel(255, 255, 0));

					xLastPos = xPos;
					yLastPos = yPos;
					}
				}

			for (iAngle = 0; iAngle < 360; iAngle += 18)
				{
				int xPos;
				int yPos;
				C3DConversion::CalcCoord(iScale, iAngle, iMaxRadius, 0, &xPos, &yPos);

				Output.GetImage().DrawLine(x, y, x + xPos, y + yPos, 1, CG32bitPixel(255, 255, 0));
				}
			}

		//	If we're painting angles do that now

		if (bAngles)
			{
			Metric rHalfFacingArc = 180.0 / (Metric)iRotationCount;
			int iMid = Ctx.iRotation;
			int iHigh = iMid + (int)floor(rHalfFacingArc);
			int iLow = iHigh - (int)floor(rHalfFacingArc * 2.0) + 1;

			TextFont.DrawText(Output.GetImage(),
					x,
					y + (cyHeight / 2) - TextFont.GetHeight(),
					rgbNameColor,
					strPatternSubst(CONSTLIT("%d [%d - %d]"), iMid, iLow, iHigh),
					CG16bitFont::AlignCenter);
			}
		}

	//	Output

	OutputImage(Output.GetImage(), sFilespec);
	}

