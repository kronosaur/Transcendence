//	ShipImage.cpp
//
//	Generate basic stats about ship classes

#include <stdio.h>

#include <windows.h>
#include <ddraw.h>
#include "Alchemy.h"
#include "XMLUtil.h"
#include "TransData.h"

const int X_MARGIN = 50;
const int Y_MARGIN = 50;

void GenerateShipImage (CUniverse &Universe, CXMLElement *pCmdLine)
	{
	int i, j;

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
	bool bWeaponPos = pCmdLine->GetAttributeBool(CONSTLIT("weaponpos"));

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

	//	Paint

	ViewportTransform Trans;
	for (i = 0; i < iRotationCount; i++)
		{
		int x, y;

		//	Paint the ship

		Output.GetCellCenter(i, &x, &y);
		pClass->Paint(Output.GetImage(), 
				x, 
				y, 
				Trans, 
				i, 
				0,
				bDriveImages,
				false);

		//	Paint weapon positions

		if (bWeaponPos)
			{
			int iScale = RectWidth(pClass->GetImage().GetImageRect());
			int iRotation = Direction2Angle(i, iRotationCount);

			CDeviceDescList Devices;
			pClass->GenerateDevices(pClass->GetLevel(), Devices);

			for (j = 0; j < Devices.GetCount(); j++)
				{
				const SDeviceDesc &Desc = Devices.GetDeviceDesc(j);

				switch (Desc.Item.GetType()->GetCategory())
					{
					case itemcatWeapon:
					case itemcatLauncher:
						{
						int xPos;
						int yPos;
						if (Desc.b3DPosition)
							C3DConversion::CalcCoord(iScale, iRotation + Desc.iPosAngle, Desc.iPosRadius, Desc.iPosZ, &xPos, &yPos);
						else
							C3DConversion::CalcCoordCompatible(iRotation + Desc.iPosAngle, Desc.iPosRadius, &xPos, &yPos);

						Output.GetImage().DrawDot(x + xPos, y + yPos, CG16bitImage::RGBValue(255, 255, 0), CG16bitImage::markerMediumCross);
						break;
						}
					}
				}
			}
		}

	//	Output

	OutputImage(Output.GetImage(), sFilespec);
	}

