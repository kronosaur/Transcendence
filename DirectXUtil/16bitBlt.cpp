//	16bitBlt.cpp
//
//	Blt routines

#include <windows.h>
#include <ddraw.h>
#include "Alchemy.h"
#include "DirectXUtil.h"

#include <math.h>
#include <stdio.h>
#include "Euclid.h"
#include "NoiseImpl.h"

#define INTERPOLATE(pos,lineInc,XYFloat,xInt,yInt,result)		\
	{															\
	WORD a = (*(pos));											\
	WORD b = (*((pos) + (lineInc)));							\
	WORD c = (*((pos) + 1));									\
	WORD d = (*((pos) + (lineInc) + 1));						\
	Metric xf = (XYFloat).GetX() - (Metric)(xInt);				\
	Metric yf = (XYFloat).GetY() - (Metric)(yInt);				\
																\
	Metric ka = (1.0 - xf) * (1.0 - yf);						\
	Metric kb = (1.0 - xf) * yf;								\
	Metric kc = xf * (1.0 - yf);								\
	Metric kd = xf * yf;										\
																\
	int iBackCount = 0;											\
	if (a == DEFAULT_TRANSPARENT_COLOR)	{ a = 0; iBackCount++; }	\
	if (b == DEFAULT_TRANSPARENT_COLOR)	{ b = 0; iBackCount++; }	\
	if (c == DEFAULT_TRANSPARENT_COLOR)	{ c = 0; iBackCount++; }	\
	if (d == DEFAULT_TRANSPARENT_COLOR)	{ d = 0; iBackCount++; }	\
																\
	if (iBackCount != 4)										\
		{														\
		DWORD red = (DWORD)(ka * CG16bitImage::RedValue(a)		\
				+ kb * CG16bitImage::RedValue(b)				\
				+ kc * CG16bitImage::RedValue(c)				\
				+ kd * CG16bitImage::RedValue(d));				\
																\
		DWORD green = (DWORD)(ka * CG16bitImage::GreenValue(a)	\
				+ kb * CG16bitImage::GreenValue(b)				\
				+ kc * CG16bitImage::GreenValue(c)				\
				+ kd * CG16bitImage::GreenValue(d));			\
																\
		DWORD blue = (DWORD)(ka * CG16bitImage::BlueValue(a)	\
				+ kb * CG16bitImage::BlueValue(b)				\
				+ kc * CG16bitImage::BlueValue(c)				\
				+ kd * CG16bitImage::BlueValue(d));				\
																\
		(result) = CG16bitImage::RGBValue((WORD)red, (WORD)green, (WORD)blue);		\
		}														\
	}

#define INTERPOLATE_ALPHA(pos,alphaInv,lineInc,XYFloat,xInt,yInt,result)		\
	{															\
	WORD a = (*(pos));											\
	WORD b = (*((pos) + (lineInc)));							\
	WORD c = (*((pos) + 1));									\
	WORD d = (*((pos) + (lineInc) + 1));						\
	Metric xf = (XYFloat).GetX() - (Metric)(xInt);				\
	Metric yf = (XYFloat).GetY() - (Metric)(yInt);				\
																\
	Metric ka = (1.0 - xf) * (1.0 - yf);						\
	Metric kb = (1.0 - xf) * yf;								\
	Metric kc = xf * (1.0 - yf);								\
	Metric kd = xf * yf;										\
																\
	DWORD red = (DWORD)(ka * CG16bitImage::RedValue(a)			\
			+ kb * CG16bitImage::RedValue(b)					\
			+ kc * CG16bitImage::RedValue(c)					\
			+ kd * CG16bitImage::RedValue(d));					\
																\
	DWORD green = (DWORD)(ka * CG16bitImage::GreenValue(a)		\
			+ kb * CG16bitImage::GreenValue(b)					\
			+ kc * CG16bitImage::GreenValue(c)					\
			+ kd * CG16bitImage::GreenValue(d));				\
																\
	DWORD blue = (DWORD)(ka * CG16bitImage::BlueValue(a)		\
			+ kb * CG16bitImage::BlueValue(b)					\
			+ kc * CG16bitImage::BlueValue(c)					\
			+ kd * CG16bitImage::BlueValue(d));					\
																\
	red = red + (DWORD)(alphaInv * CG16bitImage::RedValue((result)));	\
	green = green + (DWORD)(alphaInv * CG16bitImage::GreenValue((result)));	\
	blue = blue + (DWORD)(alphaInv * CG16bitImage::BlueValue((result)));	\
	(result) = CG16bitImage::RGBValue((WORD)Min(red, (DWORD)255), (WORD)Min(green, (DWORD)255), (WORD)Min(blue, (DWORD)255));		\
	}

#define INTERPOLATE_ALPHA_GRAY(pos,alphaInv,lineInc,XYFloat,xInt,yInt,result)		\
	{															\
	WORD a = (*(pos));											\
	WORD b = (*((pos) + (lineInc)));							\
	WORD c = (*((pos) + 1));									\
	WORD d = (*((pos) + (lineInc) + 1));						\
	WORD ag = CG16bitImage::DesaturateValue(a);						\
	WORD bg = CG16bitImage::DesaturateValue(b);		\
	WORD cg = CG16bitImage::DesaturateValue(c);				\
	WORD dg = CG16bitImage::DesaturateValue(d);	\
	Metric xf = (XYFloat).GetX() - (Metric)(xInt);				\
	Metric yf = (XYFloat).GetY() - (Metric)(yInt);				\
																\
	Metric ka = (1.0 - xf) * (1.0 - yf);						\
	Metric kb = (1.0 - xf) * yf;								\
	Metric kc = xf * (1.0 - yf);								\
	Metric kd = xf * yf;										\
																\
	DWORD gray = (DWORD)(ka * ag			\
			+ kb * bg					\
			+ kc * cg					\
			+ kd * dg);					\
																\
	DWORD red = gray + (DWORD)(alphaInv * CG16bitImage::RedValue((result)));	\
	DWORD green = gray + (DWORD)(alphaInv * CG16bitImage::GreenValue((result)));	\
	DWORD blue = gray + (DWORD)(alphaInv * CG16bitImage::BlueValue((result)));	\
	(result) = CG16bitImage::RGBValue((WORD)Min(red, (DWORD)255), (WORD)Min(green, (DWORD)255), (WORD)Min(blue, (DWORD)255));		\
	}

#define INTERPOLATE_COPY(pos,lineInc,XYFloat,xInt,yInt,result)	\
	{															\
	WORD a = (*(pos));											\
	WORD b = (*((pos) + (lineInc)));							\
	WORD c = (*((pos) + 1));									\
	WORD d = (*((pos) + (lineInc) + 1));						\
	Metric xf = (XYFloat).GetX() - (Metric)(xInt);				\
	Metric yf = (XYFloat).GetY() - (Metric)(yInt);				\
																\
	Metric ka = (1.0 - xf) * (1.0 - yf);						\
	Metric kb = (1.0 - xf) * yf;								\
	Metric kc = xf * (1.0 - yf);								\
	Metric kd = xf * yf;										\
																\
	int iBackCount = 0;											\
	if (a == DEFAULT_TRANSPARENT_COLOR)	{ a = 0; iBackCount++; }	\
	if (b == DEFAULT_TRANSPARENT_COLOR)	{ b = 0; iBackCount++; }	\
	if (c == DEFAULT_TRANSPARENT_COLOR)	{ c = 0; iBackCount++; }	\
	if (d == DEFAULT_TRANSPARENT_COLOR)	{ d = 0; iBackCount++; }	\
																\
	if (iBackCount == 4)										\
		(result) = DEFAULT_TRANSPARENT_COLOR;					\
	else														\
		{														\
		DWORD red = (DWORD)(ka * CG16bitImage::RedValue(a)		\
				+ kb * CG16bitImage::RedValue(b)				\
				+ kc * CG16bitImage::RedValue(c)				\
				+ kd * CG16bitImage::RedValue(d));				\
																\
		DWORD green = (DWORD)(ka * CG16bitImage::GreenValue(a)	\
				+ kb * CG16bitImage::GreenValue(b)				\
				+ kc * CG16bitImage::GreenValue(c)				\
				+ kd * CG16bitImage::GreenValue(d));			\
																\
		DWORD blue = (DWORD)(ka * CG16bitImage::BlueValue(a)	\
				+ kb * CG16bitImage::BlueValue(b)				\
				+ kc * CG16bitImage::BlueValue(c)				\
				+ kd * CG16bitImage::BlueValue(d));				\
																\
		(result) = CG16bitImage::RGBValue((WORD)red, (WORD)green, (WORD)blue);		\
		}														\
	}

#define INTERPOLATE_GRAY(pos,lineInc,XYFloat,xInt,yInt,result)	\
	{															\
	WORD a = (*(pos));						\
	WORD b = (*((pos) + (lineInc)));		\
	WORD c = (*((pos) + 1));				\
	WORD d = (*((pos) + (lineInc) + 1));	\
	WORD ag = CG16bitImage::DesaturateValue(a);						\
	WORD bg = CG16bitImage::DesaturateValue(b);		\
	WORD cg = CG16bitImage::DesaturateValue(c);				\
	WORD dg = CG16bitImage::DesaturateValue(d);	\
	Metric xf = (XYFloat).GetX() - (Metric)(xInt);				\
	Metric yf = (XYFloat).GetY() - (Metric)(yInt);				\
																\
	Metric ka = (1.0 - xf) * (1.0 - yf);						\
	Metric kb = (1.0 - xf) * yf;								\
	Metric kc = xf * (1.0 - yf);								\
	Metric kd = xf * yf;										\
																\
	int iBackCount = 0;											\
	if (a == DEFAULT_TRANSPARENT_COLOR)	{ ag = 0; iBackCount++; }	\
	if (b == DEFAULT_TRANSPARENT_COLOR)	{ bg = 0; iBackCount++; }	\
	if (c == DEFAULT_TRANSPARENT_COLOR)	{ cg = 0; iBackCount++; }	\
	if (d == DEFAULT_TRANSPARENT_COLOR)	{ dg = 0; iBackCount++; }	\
																\
	if (iBackCount != 4)										\
		{														\
		DWORD gray = (DWORD)(ka * ag		\
				+ kb * bg				\
				+ kc * cg				\
				+ kd * dg);				\
																\
		(result) = CG16bitImage::RGBValue((WORD)gray, (WORD)gray, (WORD)gray);		\
		}														\
	}

bool CalcBltTransform (Metric rX,
					   Metric rY,
					   Metric rScaleX,
					   Metric rScaleY,
					   Metric rRotation,
					   int xSrc,
					   int ySrc,
					   int cxSrc,
					   int cySrc,
					   CXForm *retSrcToDest, 
					   CXForm *retDestToSrc, 
					   RECT *retrcDest)
	{
	//	Eliminate some simple edge conditions

	if (cxSrc <= 0 || cySrc <= 0 || rScaleX <= 0.0 || rScaleY <= 0.0)
		return false;

	//	Compute the center of the source in source coordinates

	int xSrcCenter = xSrc + (cxSrc / 2);
	int ySrcCenter = ySrc + (cySrc / 2);

	//	Create a transform from source coords to destination

	*retSrcToDest 
			//	First move the source origin to the center
			= CXForm(xformTranslate, -xSrcCenter, -ySrcCenter)

			//	Next, scale up the source
			* CXForm(xformScale, rScaleX, rScaleY)

			//	Then rotate
			* CXForm(xformRotate, -(int)rRotation)

			//	Now move to the proper location
			* CXForm(xformTranslate, rX, rY);

	//	Now create the inverse transfor (from dest to source)

	*retDestToSrc = CXForm(xformTranslate, -rX, -rY)
			* CXForm(xformRotate, (int)rRotation)
			* CXForm(xformScale, 1.0 / rScaleX, 1.0 / rScaleY)
			* CXForm(xformTranslate, xSrcCenter, ySrcCenter);

	//	Transform the four corners of the source to see where they
	//	end up on the destination

	CVector vSrcUL = retSrcToDest->Transform(CVector(xSrc, ySrc));
	CVector vSrcUR = retSrcToDest->Transform(CVector(xSrc + cxSrc, ySrc));
	CVector vSrcLL = retSrcToDest->Transform(CVector(xSrc, ySrc + cySrc));
	CVector vSrcLR = retSrcToDest->Transform(CVector(xSrc + cxSrc, ySrc + cySrc));

	//	Now figure out the axis-aligned box in which the source will
	//	be in destination coordinates

	Metric rLeft = Min(Min(vSrcUL.GetX(), vSrcUR.GetX()), Min(vSrcLL.GetX(), vSrcLR.GetX()));
	Metric rRight = Max(Max(vSrcUL.GetX(), vSrcUR.GetX()), Max(vSrcLL.GetX(), vSrcLR.GetX()));
	Metric rTop = Min(Min(vSrcUL.GetY(), vSrcUR.GetY()), Min(vSrcLL.GetY(), vSrcLR.GetY()));
	Metric rBottom = Max(Max(vSrcUL.GetY(), vSrcUR.GetY()), Max(vSrcLL.GetY(), vSrcLR.GetY()));

	//	Generate the rect

	retrcDest->left = (int)rLeft;
	retrcDest->right = retrcDest->left + (int)(rRight - rLeft);
	retrcDest->top = (int)rTop;
	retrcDest->bottom = retrcDest->top + (int)(rBottom - rTop);

	//	Done

	return (RectWidth(*retrcDest) > 0 ) && (RectHeight(*retrcDest) > 0);
	}

void CopyBltColorize (CG16bitImage &Dest,
					  int xDest,
					  int yDest,
					  int cxDest,
					  int cyDest,
					  const CG16bitImage &Src,
					  int xSrc,
					  int ySrc,
					  DWORD dwHue,
					  DWORD dwSaturation)

//	CopyBltColorize
//
//	Blts the source to the destination and colorizes to the given color.
//	NOTE: We copy the source mask over, if any.
//
//	See: Computer Graphics, Foley & van Dam, p.593.

	{
	//	Compute the color

	COLORREF rgbBlack = RGB(0, 0, 0);
	COLORREF rgbWhite = RGB(255, 255, 255);
	COLORREF rgbHue = CG16bitPixel::RGBRealToRGB(CG16bitPixel::HSBToRGB(SColorHSB(dwHue, 1.0, 1.0)));
	COLORREF rgbColor = CG16bitPixel::Blend(RGB(128, 128, 128), rgbHue, dwSaturation / 100.0);

	//	Different code paths depending on whether we have alpha values or not

	if (Src.HasAlpha() && Dest.HasAlpha())
		{
		//	Loop over every pixel in the destination

		WORD *pSrcRow = Src.GetPixel(Src.GetRowStart(ySrc), xSrc);
		WORD *pDestRow = Dest.GetPixel(Dest.GetRowStart(yDest), xDest);
		WORD *pDestRowEnd = Dest.GetPixel(Dest.GetRowStart(yDest + cyDest), xDest);
		BYTE *pSrcRowAlpha = Src.GetAlphaValue(xSrc, ySrc);
		BYTE *pDestRowAlpha = Dest.GetAlphaValue(xDest, yDest);
		while (pDestRow < pDestRowEnd)
			{
			WORD *pSrcPos = pSrcRow;
			WORD *pDestPos = pDestRow;
			WORD *pDestPosEnd = pDestRow + cxDest;
			BYTE *pSrcAlpha = pSrcRowAlpha;
			BYTE *pDestAlpha = pDestRowAlpha;
			while (pDestPos < pDestPosEnd)
				{
				//	Convert color
				
				WORD wSrc = *pSrcPos;
				DWORD srcBrightness = GetRValue(CG16bitPixel::Desaturate(CG16bitPixel::PixelToRGB(wSrc)));
				double rB = srcBrightness / 255.0;

				COLORREF rgbResult = CG16bitPixel::Blend(rgbBlack, rgbColor, rgbWhite, 2.0 * (rB - 1.0) + 1.0);
				*pDestPos = CG16bitPixel::RGBToPixel(rgbResult);

				//	Copy alpha value

				*pDestAlpha++ = *pSrcAlpha++;

				//	Next

				pSrcPos++;
				pDestPos++;
				}

			pSrcRow = Src.NextRow(pSrcRow);
			pDestRow = Dest.NextRow(pDestRow);
			pSrcRowAlpha = Src.NextAlphaRow(pSrcRowAlpha);
			pDestRowAlpha = Dest.NextAlphaRow(pDestRowAlpha);
			}
		}
	//	LATER: Handle case of alpha blt on non-alpha blt
	else
		{
		WORD wSrcBackColor = Src.GetBackColor();
		WORD wDestBackColor = Dest.GetBackColor();

		//	Loop over every pixel in the destination

		WORD *pSrcRow = Src.GetPixel(Src.GetRowStart(ySrc), xSrc);
		WORD *pDestRow = Dest.GetPixel(Dest.GetRowStart(yDest), xDest);
		WORD *pDestRowEnd = Dest.GetPixel(Dest.GetRowStart(yDest + cyDest), xDest);
		while (pDestRow < pDestRowEnd)
			{
			WORD *pSrcPos = pSrcRow;
			WORD *pDestPos = pDestRow;
			WORD *pDestPosEnd = pDestRow + cxDest;
			while (pDestPos < pDestPosEnd)
				{
				WORD wSrc = *pSrcPos;

				if (wSrc == wSrcBackColor)
					*pDestPos = wDestBackColor;
				else
					{
					DWORD srcBrightness = GetRValue(CG16bitPixel::Desaturate(CG16bitPixel::PixelToRGB(wSrc)));
					double rB = srcBrightness / 255.0;

					COLORREF rgbResult = CG16bitPixel::Blend(rgbBlack, rgbColor, rgbWhite, 2.0 * (rB - 1.0) + 1.0);
					*pDestPos = CG16bitPixel::RGBToPixel(rgbResult);
					}

				//	Next

				pSrcPos++;
				pDestPos++;
				}

			pSrcRow = Src.NextRow(pSrcRow);
			pDestRow = Dest.NextRow(pDestRow);
			}
		}
	}

void CopyBltTransformed (CG16bitImage &Dest, 
						 const RECT &rcDest,
						 const CG16bitImage &Src, 
						 const RECT &rcSrc,
						 const CXForm &SrcToDest,
						 const CXForm &DestToSrc,
						 const RECT &rcDestXForm)

//	CopyBltTransformed
//
//	Blts the source to the destination. All transformations are from the
//	center of the source.

	{
	int xDest = rcDest.left;
	int yDest = rcDest.top;
	int cxDest = RectWidth(rcDest);
	int cyDest = RectHeight(rcDest);

	int xSrc = rcSrc.left;
	int ySrc = rcSrc.top;
	int cxSrc = RectWidth(rcSrc);
	int cySrc = RectHeight(rcSrc);
	int xSrcEnd = xSrc + cxSrc;
	int ySrcEnd = ySrc + cySrc;

	//	Compute vectors that move us by 1 pixel

	CVector vOrigin = DestToSrc.Transform(CVector(0.0, 0.0));
	CVector vIncX = DestToSrc.Transform(CVector(1.0, 0.0)) - vOrigin;
	CVector vIncY = DestToSrc.Transform(CVector(0.0, 1.0)) - vOrigin;

	int iRowHeight = Src.GetRowStart(1) - Src.GetRowStart(0);

	//	Different code paths depending on whether we have alpha values or not

	if (Src.HasAlpha() && Dest.HasAlpha())
		{
		//	Loop over every pixel in the destination

		CVector vSrcRow = DestToSrc.Transform(CVector(rcDestXForm.left, rcDestXForm.top));
		WORD *pDestRow = Dest.GetPixel(Dest.GetRowStart(yDest), xDest);
		WORD *pDestRowEnd = Dest.GetPixel(Dest.GetRowStart(yDest + cyDest), xDest);
		BYTE *pDestRowAlpha = Dest.GetAlphaValue(xDest, yDest);
		while (pDestRow < pDestRowEnd)
			{
			CVector vSrcPos = vSrcRow;
			WORD *pDestPos = pDestRow;
			WORD *pDestPosEnd = pDestRow + cxDest;
			BYTE *pDestAlpha = pDestRowAlpha;
			while (pDestPos < pDestPosEnd)
				{
				int xSrcPos = (int)vSrcPos.GetX();
				int ySrcPos = (int)vSrcPos.GetY();

				if (xSrcPos >= xSrc && xSrcPos + 1 < xSrcEnd
						&& ySrcPos >= ySrc && ySrcPos + 1 < ySrcEnd)
					{
					WORD *pSrcPos = Src.GetPixel(Src.GetRowStart(ySrcPos), xSrcPos);
					BYTE byAlpha = *Src.GetAlphaValue(xSrcPos, ySrcPos);

					if (byAlpha == 0)
						*pDestAlpha = 0;
					else if (byAlpha == 255)
						{
						INTERPOLATE_COPY(pSrcPos, iRowHeight, vSrcPos, xSrcPos, ySrcPos, *pDestPos);
						*pDestAlpha = 255;
						}
					else
						{
						INTERPOLATE_COPY(pSrcPos, iRowHeight, vSrcPos, xSrcPos, ySrcPos, *pDestPos);
						*pDestAlpha = byAlpha;
						}
					}
				else
					*pDestAlpha = 0;

				//	Next

				vSrcPos = vSrcPos + vIncX;
				pDestPos++;
				pDestAlpha++;
				}

			//	Next row

			vSrcRow = vSrcRow + vIncY;
			pDestRow = Dest.NextRow(pDestRow);
			pDestRowAlpha = Dest.NextAlphaRow(pDestRowAlpha);
			}
		}
	else
		{
		//	Loop over every pixel in the destination

		CVector vSrcRow = DestToSrc.Transform(CVector(rcDestXForm.left, rcDestXForm.top));
		WORD *pDestRow = Dest.GetPixel(Dest.GetRowStart(yDest), xDest);
		WORD *pDestRowEnd = Dest.GetPixel(Dest.GetRowStart(yDest + cyDest), xDest);
		while (pDestRow < pDestRowEnd)
			{
			CVector vSrcPos = vSrcRow;
			WORD *pDestPos = pDestRow;
			WORD *pDestPosEnd = pDestRow + cxDest;
			while (pDestPos < pDestPosEnd)
				{
				int xSrcPos = (int)vSrcPos.GetX();
				int ySrcPos = (int)vSrcPos.GetY();

				if (xSrcPos >= xSrc && xSrcPos + 1 < xSrcEnd
						&& ySrcPos >= ySrc && ySrcPos + 1 < ySrcEnd)
					{
					WORD *pSrcPos = Src.GetPixel(Src.GetRowStart(ySrcPos), xSrcPos);
					INTERPOLATE_COPY(pSrcPos, iRowHeight, vSrcPos, xSrcPos, ySrcPos, *pDestPos);
					}

				//	Next

				vSrcPos = vSrcPos + vIncX;
				pDestPos++;
				}

			//	Next row

			vSrcRow = vSrcRow + vIncY;
			pDestRow = Dest.NextRow(pDestRow);
			}
		}
	}

void DrawBltRotated (CG16bitImage &Dest,
					 int xDest,
					 int yDest,
					 int iRotation,
					 const CG16bitImage &Src,
					 int xSrc,
					 int ySrc,
					 int cxSrc,
					 int cySrc)

//	DrawBltRotated
//
//	Draws the source image with its center at xDest, yDest and rotated
//	by iRotation angle (in degrees)

	{
	DrawBltTransformed(Dest, xDest, yDest, 1.0, 1.0, iRotation, Src, xSrc, ySrc, cxSrc, cySrc);
	}

void DrawBltScaledFast (CG16bitImage &Dest,
						int xDest,
						int yDest,
						int cxDest,
						int cyDest,
						const CG16bitImage &Src,
						int xSrc,
						int ySrc,
						int cxSrc,
						int cySrc)

//	DrawBltScaledFast
//
//	Scales the source image to fit in the destination

	{
	if (cxDest <= 0 || cyDest <= 0 || cxSrc <= 0 || cySrc <= 0)
		return;

	//	Compute the increment on the source to cover the entire destination

	Metric xSrcInc = (Metric)cxSrc / (Metric)cxDest;
	Metric ySrcInc = (Metric)cySrc / (Metric)cyDest;

	//	Make sure we're in bounds

	Metric xSrcStart = (Metric)xSrc;
	Metric ySrcStart = (Metric)ySrc;
	if (!Dest.AdjustScaledCoords(&xSrcStart, &ySrcStart, Src.GetWidth(), Src.GetHeight(), 
			xSrcInc, ySrcInc,
			&xDest, &yDest,
			&cxDest, &cyDest))
		return;

	//	Do the blt

	WORD *pDestRow = Dest.GetPixel(Dest.GetRowStart(yDest), xDest);
	WORD *pDestRowEnd = Dest.GetPixel(Dest.GetRowStart(yDest + cyDest), xDest);

	Metric y = ySrcStart;
	while (pDestRow < pDestRowEnd)
		{
		WORD *pDestPos = pDestRow;
		WORD *pDestPosEnd = pDestPos + cxDest;

		WORD *pSrcRow = Src.GetPixel(Src.GetRowStart((int)y), (int)xSrcStart);
		Metric xOffset = 0.0;

		while (pDestPos < pDestPosEnd)
			{
			*pDestPos++ = *(pSrcRow + (int)xOffset);
			xOffset += xSrcInc;
			}

		y += ySrcInc;
		pDestRow = Dest.NextRow(pDestRow);
		}
	}

void DrawBltShimmer (CG16bitImage &Dest,
					 int xDest,
					 int yDest,
					 int cxDest,
					 int cyDest,
					 const CG16bitImage &Src,
					 int xSrc,
					 int ySrc,
					 DWORD byIntensity,
					 DWORD dwSeed)

//	DrawBltShimmer
//
//	Draws shimmer

	{
	const DWORD LARGE_PRIME1 = 433494437;
	const DWORD LARGE_PRIME2 = 3010349;

	//	Make sure we're in bounds

	if (!Dest.AdjustCoords(&xSrc, &ySrc, Src.GetWidth(), Src.GetHeight(), 
			&xDest, &yDest,
			&cxDest, &cyDest))
		return;

	//	We need noise

	NoiseInit();

	//	Initialize

	WORD *pSrcRow = Src.GetPixel(Src.GetRowStart(ySrc), xSrc);
	WORD *pSrcRowEnd = Src.GetPixel(Src.GetRowStart(ySrc + cyDest), xSrc);
	WORD *pDestRow = Dest.GetPixel(Dest.GetRowStart(yDest), xDest);

	//	If we've got an alpha mask then blt using the transparency
	//	information.

	if (Src.HasAlpha())
		{
		DWORD dwRnd = LARGE_PRIME2 * dwSeed;
		BYTE *pAlphaSrcRow = Src.GetAlphaValue(xSrc, ySrc);

		while (pSrcRow < pSrcRowEnd)
			{
			BYTE *pAlphaPos = pAlphaSrcRow;
			WORD *pSrcPos = pSrcRow;
			WORD *pSrcPosEnd = pSrcRow + cxDest;
			WORD *pDestPos = pDestRow;

			while (pSrcPos < pSrcPosEnd)
				{
				if (*pAlphaPos == 0)
					{
					}
				else if (*pAlphaPos == 255 || *pDestPos == 0)
					{
					if (PERM((DWORD)pDestPos * LARGE_PRIME2 + dwRnd) < byIntensity)
						*pDestPos = *pSrcPos;
					}
				else
					{
					if (PERM((DWORD)pDestPos * LARGE_PRIME2 + dwRnd) < byIntensity)
						{
						DWORD pxSource = *pSrcPos;
						DWORD pxDest = *pDestPos;

						//	x ^ 0xff is the same as 255 - x
						//	| 0x07 so that we round-up
						//	+ 1 because below we divide by 256 instead of 255.
						//	LATER: Use a table lookup

						DWORD dwInvTrans = ((((DWORD)(*pAlphaPos)) ^ 0xff) | 0x07) + 1;

						WORD wRedSrc = (*pSrcPos >> 11) & 0x1f;
						WORD wGreenSrc = (*pSrcPos >> 5) & 0x3f;
						WORD wBlueSrc = (*pSrcPos) & 0x1f;

						WORD wRedDest = ((WORD)dwInvTrans * ((*pDestPos >> 11) & 0x1f)) >> 8;
						WORD wGreenDest = ((WORD)dwInvTrans * ((*pDestPos >> 5) & 0x3f)) >> 8;
						WORD wBlueDest = ((WORD)dwInvTrans * ((*pDestPos) & 0x1f)) >> 8;

						WORD wRedResult = Min((WORD)0x1f, (WORD)(wRedSrc + wRedDest));
						WORD wGreenResult = Min((WORD)0x3f, (WORD)(wGreenSrc + wGreenDest));
						WORD wBlueResult = Min((WORD)0x1f, (WORD)(wBlueSrc + wBlueDest));

						*pDestPos = (wRedResult << 11) | (wGreenResult << 5) | wBlueResult;
						}
					}

				pDestPos++;
				pSrcPos++;
				pAlphaPos++;
				}

			pSrcRow = Src.NextRow(pSrcRow);
			pDestRow = Dest.NextRow(pDestRow);
			pAlphaSrcRow = Src.NextAlphaRow(pAlphaSrcRow);
			}
		}

	//	If we've got constant transparency then use the alpha tables

	else if (Src.IsTransparent())
		{
#ifdef LATER
		while (pSrcRow < pSrcRowEnd)
			{
			WORD *pSrcPos = pSrcRow;
			WORD *pSrcPosEnd = pSrcRow + cxWidth;
			WORD *pDestPos = pDestRow;

			while (pSrcPos < pSrcPosEnd)
				if (*pSrcPos == m_wBackColor)
					{
					pDestPos++;
					pSrcPos++;
					}
				else
					{
					//	Blend the pixel using the appropriate tables

					WORD rgbRed = Source.m_pRedAlphaTable[((*pSrcPos & 0xf800) >> 6) | ((*pDestPos & 0xf800) >> 11)];
					WORD rgbGreen = Source.m_pGreenAlphaTable[((*pSrcPos & 0x7e0) << 1) | ((*pDestPos & 0x7e0) >> 5)];
					WORD rgbBlue = Source.m_pBlueAlphaTable[(*pSrcPos & 0x1f) << 5 | (*pDestPos & 0x1f)];

					*pDestPos++ = rgbRed | rgbGreen | rgbBlue;
					pSrcPos++;
					}

			pSrcRow = Source.NextRow(pSrcRow);
			pDestRow = NextRow(pDestRow);
			}
#endif
		}

	//	Otherwise just blt

	else
		{
		DWORD dwRnd = LARGE_PRIME2 * dwSeed;
		WORD wSrcBackColor = Src.GetBackColor();

		while (pSrcRow < pSrcRowEnd)
			{
			WORD *pSrcPos = pSrcRow;
			WORD *pSrcPosEnd = pSrcRow + cxDest;
			WORD *pDestPos = pDestRow;

			while (pSrcPos < pSrcPosEnd)
				{
				if ((*pSrcPos != wSrcBackColor)
						&& (PERM((DWORD)pDestPos * LARGE_PRIME2 + dwRnd) < byIntensity))
					*pDestPos = *pSrcPos;

				pDestPos++;
				pSrcPos++;
				}

			dwRnd *= LARGE_PRIME1;
			pSrcRow = Src.NextRow(pSrcRow);
			pDestRow = Dest.NextRow(pDestRow);
			}
		}
	}

void DrawBltTransformed (CG16bitImage &Dest, 
						 Metric rX, 
						 Metric rY, 
						 Metric rScaleX, 
						 Metric rScaleY, 
						 Metric rRotation, 
						 const CG16bitImage &Src, 
						 int xSrc, 
						 int ySrc, 
						 int cxSrc, 
						 int cySrc)

//	DrawBltTransformed
//
//	Blts the source to the destination. All transformations are from the
//	center of the source.

	{
	CXForm SrcToDest;
	CXForm DestToSrc;
	RECT rcDest;
	if (!CalcBltTransform(rX, rY, rScaleX, rScaleY, rRotation, xSrc, ySrc, cxSrc, cySrc, &SrcToDest, &DestToSrc, &rcDest))
		return;

	//	Bounds check on the destination

	int cxDest = RectWidth(rcDest);
	int cyDest = RectHeight(rcDest);

	int xDest = rcDest.left;
	if (xDest < 0)
		{
		cxDest += xDest;
		xDest = 0;
		}

	int yDest = rcDest.top;
	if (yDest < 0)
		{
		cyDest += yDest;
		yDest = 0;
		}

	if (xDest + cxDest > Dest.GetWidth())
		cxDest = Dest.GetWidth() - xDest;

	if (yDest + cyDest > Dest.GetHeight())
		cyDest = Dest.GetHeight() - yDest;

	if (cxDest <= 0 || cyDest <= 0)
		return;

	int xSrcEnd = xSrc + cxSrc;
	int ySrcEnd = ySrc + cySrc;

	//	Compute vectors that move us by 1 pixel

	CVector vOrigin = DestToSrc.Transform(CVector(0.0, 0.0));
	CVector vIncX = DestToSrc.Transform(CVector(1.0, 0.0)) - vOrigin;
	CVector vIncY = DestToSrc.Transform(CVector(0.0, 1.0)) - vOrigin;

	int iRowHeight = Src.GetRowStart(1) - Src.GetRowStart(0);

	//	Different code paths depending on whether we have alpha values or not

	if (Src.HasAlpha())
		{
		//	Loop over every pixel in the destination

		CVector vSrcRow = DestToSrc.Transform(CVector(xDest, yDest));
		WORD *pDestRow = Dest.GetPixel(Dest.GetRowStart(yDest), xDest);
		WORD *pDestRowEnd = Dest.GetPixel(Dest.GetRowStart(yDest + cyDest), xDest);
		while (pDestRow < pDestRowEnd)
			{
			CVector vSrcPos = vSrcRow;
			WORD *pDestPos = pDestRow;
			WORD *pDestPosEnd = pDestRow + cxDest;
			while (pDestPos < pDestPosEnd)
				{
				int xSrcPos = (int)vSrcPos.GetX();
				int ySrcPos = (int)vSrcPos.GetY();

				if (xSrcPos >= xSrc && xSrcPos + 1 < xSrcEnd
						&& ySrcPos >= ySrc && ySrcPos + 1< ySrcEnd)
					{
					WORD *pSrcPos = Src.GetPixel(Src.GetRowStart(ySrcPos), xSrcPos);
					BYTE byAlpha = *Src.GetAlphaValue(xSrcPos, ySrcPos);

					if (byAlpha == 0)
						NULL;
					else if (byAlpha == 255)
						{
						INTERPOLATE(pSrcPos, iRowHeight, vSrcPos, xSrcPos, ySrcPos, *pDestPos);
						}
					else
						{
						Metric rAlphaInv = (Metric)(255 - byAlpha) / 255.0;
						INTERPOLATE_ALPHA(pSrcPos, rAlphaInv, iRowHeight, vSrcPos, xSrcPos, ySrcPos, *pDestPos);
						}
					}

				//	Next

				vSrcPos = vSrcPos + vIncX;
				pDestPos++;
				}

			//	Next row

			vSrcRow = vSrcRow + vIncY;
			pDestRow = Dest.NextRow(pDestRow);
			}
		}
	else
		{
		//	Loop over every pixel in the destination

		CVector vSrcRow = DestToSrc.Transform(CVector(xDest, yDest));
		WORD *pDestRow = Dest.GetPixel(Dest.GetRowStart(yDest), xDest);
		WORD *pDestRowEnd = Dest.GetPixel(Dest.GetRowStart(yDest + cyDest), xDest);
		while (pDestRow < pDestRowEnd)
			{
			CVector vSrcPos = vSrcRow;
			WORD *pDestPos = pDestRow;
			WORD *pDestPosEnd = pDestRow + cxDest;
			while (pDestPos < pDestPosEnd)
				{
				int xSrcPos = (int)vSrcPos.GetX();
				int ySrcPos = (int)vSrcPos.GetY();

				if (xSrcPos >= xSrc && xSrcPos + 1 < xSrcEnd
						&& ySrcPos >= ySrc && ySrcPos + 1< ySrcEnd)
					{
					WORD *pSrcPos = Src.GetPixel(Src.GetRowStart(ySrcPos), xSrcPos);
					INTERPOLATE(pSrcPos, iRowHeight, vSrcPos, xSrcPos, ySrcPos, *pDestPos);
					}

				//	Next

				vSrcPos = vSrcPos + vIncX;
				pDestPos++;
				}

			//	Next row

			vSrcRow = vSrcRow + vIncY;
			pDestRow = Dest.NextRow(pDestRow);
			}
		}
	}

void DrawBltTransformedGray (CG16bitImage &Dest, 
							 Metric rX, 
							 Metric rY, 
							 Metric rScaleX, 
							 Metric rScaleY, 
							 Metric rRotation, 
							 const CG16bitImage &Src, 
							 int xSrc, 
							 int ySrc, 
							 int cxSrc, 
							 int cySrc)

//	DrawBltTransformedGray
//
//	Blts the source to the destination. All transformations are from the
//	center of the source.

	{
	CXForm SrcToDest;
	CXForm DestToSrc;
	RECT rcDest;
	if (!CalcBltTransform(rX, rY, rScaleX, rScaleY, rRotation, xSrc, ySrc, cxSrc, cySrc, &SrcToDest, &DestToSrc, &rcDest))
		return;

	//	Bounds check on the destination

	int cxDest = RectWidth(rcDest);
	int cyDest = RectHeight(rcDest);

	int xDest = rcDest.left;
	if (xDest < 0)
		{
		cxDest += xDest;
		xDest = 0;
		}

	int yDest = rcDest.top;
	if (yDest < 0)
		{
		cyDest += yDest;
		yDest = 0;
		}

	if (xDest + cxDest > Dest.GetWidth())
		cxDest = Dest.GetWidth() - xDest;

	if (yDest + cyDest > Dest.GetHeight())
		cyDest = Dest.GetHeight() - yDest;

	if (cxDest <= 0 || cyDest <= 0)
		return;

	int xSrcEnd = xSrc + cxSrc;
	int ySrcEnd = ySrc + cySrc;

	//	Compute vectors that move us by 1 pixel

	CVector vOrigin = DestToSrc.Transform(CVector(0.0, 0.0));
	CVector vIncX = DestToSrc.Transform(CVector(1.0, 0.0)) - vOrigin;
	CVector vIncY = DestToSrc.Transform(CVector(0.0, 1.0)) - vOrigin;

	int iRowHeight = Src.GetRowStart(1) - Src.GetRowStart(0);

	//	Different code paths depending on whether we have alpha values or not

	if (Src.HasAlpha())
		{
		//	Loop over every pixel in the destination

		CVector vSrcRow = DestToSrc.Transform(CVector(xDest, yDest));
		WORD *pDestRow = Dest.GetPixel(Dest.GetRowStart(yDest), xDest);
		WORD *pDestRowEnd = Dest.GetPixel(Dest.GetRowStart(yDest + cyDest), xDest);
		while (pDestRow < pDestRowEnd)
			{
			CVector vSrcPos = vSrcRow;
			WORD *pDestPos = pDestRow;
			WORD *pDestPosEnd = pDestRow + cxDest;
			while (pDestPos < pDestPosEnd)
				{
				int xSrcPos = (int)vSrcPos.GetX();
				int ySrcPos = (int)vSrcPos.GetY();

				if (xSrcPos >= xSrc && xSrcPos + 1 < xSrcEnd
						&& ySrcPos >= ySrc && ySrcPos + 1< ySrcEnd)
					{
					WORD *pSrcPos = Src.GetPixel(Src.GetRowStart(ySrcPos), xSrcPos);
					BYTE byAlpha = *Src.GetAlphaValue(xSrcPos, ySrcPos);

					if (byAlpha == 0)
						NULL;
					else if (byAlpha == 255)
						{
						INTERPOLATE_GRAY(pSrcPos, iRowHeight, vSrcPos, xSrcPos, ySrcPos, *pDestPos);
						}
					else
						{
						Metric rAlphaInv = (Metric)(255 - byAlpha) / 255.0;
						INTERPOLATE_ALPHA_GRAY(pSrcPos, rAlphaInv, iRowHeight, vSrcPos, xSrcPos, ySrcPos, *pDestPos);
						}
					}

				//	Next

				vSrcPos = vSrcPos + vIncX;
				pDestPos++;
				}

			//	Next row

			vSrcRow = vSrcRow + vIncY;
			pDestRow = Dest.NextRow(pDestRow);
			}
		}
	else
		{
		//	Loop over every pixel in the destination

		CVector vSrcRow = DestToSrc.Transform(CVector(xDest, yDest));
		WORD *pDestRow = Dest.GetPixel(Dest.GetRowStart(yDest), xDest);
		WORD *pDestRowEnd = Dest.GetPixel(Dest.GetRowStart(yDest + cyDest), xDest);
		while (pDestRow < pDestRowEnd)
			{
			CVector vSrcPos = vSrcRow;
			WORD *pDestPos = pDestRow;
			WORD *pDestPosEnd = pDestRow + cxDest;
			while (pDestPos < pDestPosEnd)
				{
				int xSrcPos = (int)vSrcPos.GetX();
				int ySrcPos = (int)vSrcPos.GetY();

				if (xSrcPos >= xSrc && xSrcPos + 1 < xSrcEnd
						&& ySrcPos >= ySrc && ySrcPos + 1< ySrcEnd)
					{
					WORD *pSrcPos = Src.GetPixel(Src.GetRowStart(ySrcPos), xSrcPos);
					INTERPOLATE_GRAY(pSrcPos, iRowHeight, vSrcPos, xSrcPos, ySrcPos, *pDestPos);
					}

				//	Next

				vSrcPos = vSrcPos + vIncX;
				pDestPos++;
				}

			//	Next row

			vSrcRow = vSrcRow + vIncY;
			pDestRow = Dest.NextRow(pDestRow);
			}
		}
	}
