//	CG16bitPixel.cpp
//
//	Implementation of raw 16-bit pixel
//	Copyright (c) 2013 by Kronosaur Productions, LLC. All Rights Reserved.

#include <windows.h>
#include <ddraw.h>
#include "Alchemy.h"
#include "DirectXUtil.h"

COLORREF CG16bitPixel::Blend (COLORREF rgbFrom, COLORREF rgbTo, double rUnitRange)

//	Blend
//
//	Returns a blended pixel from rgbFrom to rgbTo, where rUnitRange is from
//	0 to 1.0

	{
	double rUnitRangeInv = 1.0 - rUnitRange;

	return RGB(
		(DWORD)(GetRValue(rgbFrom) * rUnitRangeInv + GetRValue(rgbTo) * rUnitRange),
		(DWORD)(GetGValue(rgbFrom) * rUnitRangeInv + GetGValue(rgbTo) * rUnitRange),
		(DWORD)(GetBValue(rgbFrom) * rUnitRangeInv + GetBValue(rgbTo) * rUnitRange)
		);
	}

COLORREF CG16bitPixel::Blend (COLORREF rgbNegative, COLORREF rgbCenter, COLORREF rgbPositive, double rDoubleRange)

//	Blend
//
//	Returns a blended pixel from rgbNegative to rgbPositive, where rDoubleRange
//	is from -1.0 to 1.0.

	{
	if (rDoubleRange < 0.0)
		return Blend(rgbNegative, rgbCenter, rDoubleRange + 1.0);
	else if (rDoubleRange > 0.0)
		return Blend(rgbCenter, rgbPositive, rDoubleRange);
	else
		return rgbCenter;
	}

COLORREF CG16bitPixel::Desaturate (COLORREF rgbColor)

//	Desaturate
//
//	Desaturate a pixel.

	{
	DWORD dwMax = Max(Max(GetRValue(rgbColor), GetGValue(rgbColor)), GetBValue(rgbColor));
	DWORD dwMin = Min(Min(GetRValue(rgbColor), GetGValue(rgbColor)), GetBValue(rgbColor));
	DWORD dwDesaturate = (dwMax + dwMin) / 2;
	return RGB(dwDesaturate, dwDesaturate, dwDesaturate);
	}

SColorRGB CG16bitPixel::HSBToRGB (const SColorHSB &hsbColor)

//	HSBToRGB
//
//	Convert from HSB to RGB colorspace
//	From: Computer Graphics, Foley & van Dam, p.593

	{
	if (hsbColor.rSaturation == 0.0)
		return SColorRGB(hsbColor.rBrightness, hsbColor.rBrightness, hsbColor.rBrightness);
	else
		{
		double rH = (hsbColor.rHue == 360.0 ? 0.0 : hsbColor.rHue) / 60.0;
		double rI = floor(rH);
		double rF = rH - rI;
		double rP = hsbColor.rBrightness * (1.0 - hsbColor.rSaturation);
		double rQ = hsbColor.rBrightness * (1.0 - hsbColor.rSaturation * rF);
		double rT = hsbColor.rBrightness * (1.0 - hsbColor.rSaturation * (1.0 - rF));

		switch ((int)rI)
			{
			case 0:
				return SColorRGB(hsbColor.rBrightness, rT, rP);

			case 1:
				return SColorRGB(rQ, hsbColor.rBrightness, rP);

			case 2:
				return SColorRGB(rP, hsbColor.rBrightness, rT);

			case 3:
				return SColorRGB(rP, rQ, hsbColor.rBrightness);

			case 4:
				return SColorRGB(rT, rP, hsbColor.rBrightness);

			case 5:
				return SColorRGB(hsbColor.rBrightness, rP, rQ);

			default:
				return SColorRGB();
			}
		}
	}

SColorHSB CG16bitPixel::RGBToHSB (const SColorRGB &rgbColor)

//	RGBToHSB
//
//	Convert from RGB to HSB colorspace.
//	From: Computer Graphics, Foley & van Dam, p.592

	{
	enum EColors
		{
		colorRed = 0,
		colorGreen = 1,
		colorBlue = 2,
		};

	//	Figure out which is the primary (brightest) color

	int iPrimary;
	double rMax;
	double rMin;

	//	R G B
	//	R B G
	//	B R G

	if (rgbColor.rRed >= rgbColor.rGreen)
		{
		//	R G B
		//	R B G

		if (rgbColor.rRed >= rgbColor.rBlue)
			{
			iPrimary = colorRed;
			rMax = rgbColor.rRed;

			if (rgbColor.rGreen >= rgbColor.rBlue)
				rMin = rgbColor.rBlue;
			else
				rMin = rgbColor.rGreen;
			}

		//	B R G

		else
			{
			iPrimary = colorBlue;
			rMax = rgbColor.rBlue;
			rMin = rgbColor.rGreen;
			}
		}

	//	G R B
	//	G B R
	//	B G R

	else
		{
		//	G R B
		//	G B R

		if (rgbColor.rGreen >= rgbColor.rBlue)
			{
			iPrimary = colorGreen;
			rMax = rgbColor.rGreen;

			if (rgbColor.rRed > rgbColor.rBlue)
				rMin = rgbColor.rBlue;
			else
				rMin = rgbColor.rRed;
			}

		//	B G R

		else
			{
			iPrimary = colorBlue;
			rMax = rgbColor.rBlue;
			rMin = rgbColor.rRed;
			}
		}
	
	//	Brightness

	double rBrightness = rMax;

	//	Saturation

	double rDelta = rMax - rMin;
	double rSaturation = (rMax != 0.0 ? (rDelta / rMax) : 0.0);

	//	Hue

	double rHue;
	if (rSaturation == 0.0)
		rHue = 0.0;	//	Undefined, but we just set it to 0
	else
		{
		switch (iPrimary)
			{
			case colorRed:
				rHue = (rgbColor.rGreen - rgbColor.rBlue) / rDelta;
				break;

			case colorGreen:
				rHue = 2.0 + (rgbColor.rBlue - rgbColor.rRed) / rDelta;
				break;

			case colorBlue:
				rHue = 4.0 + (rgbColor.rRed - rgbColor.rGreen) / rDelta;
				break;
			}

		rHue *= 60.0;
		if (rHue < 0.0)
			rHue += 360.0;
		}

	return SColorHSB(
		rHue,
		rSaturation,
		rBrightness
		);
	}

COLORREF CG16bitPixel::RGBRealToRGB (const SColorRGB &rgbColor)

//	RGBRealToRGB
//
//	Converts from real-valued RGB

	{
	return RGB(
		(DWORD)(rgbColor.rRed * 255.0),
		(DWORD)(rgbColor.rGreen * 255.0),
		(DWORD)(rgbColor.rBlue * 255.0));
	}

SColorRGB CG16bitPixel::RGBToRGBReal (COLORREF rgbColor)

//	RGBToRGBReal
//
//	Converts to real-valued RGB.

	{
	return SColorRGB(
		(double)GetRValue(rgbColor) / 255.0,
		(double)GetGValue(rgbColor) / 255.0,
		(double)GetBValue(rgbColor) / 255.0
		);
	}

