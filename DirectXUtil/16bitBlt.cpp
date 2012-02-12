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
	(result) = CG16bitImage::RGBValue((WORD)red, (WORD)green, (WORD)blue);		\
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
						&& ySrcPos >= ySrc && ySrcPos + 1< ySrcEnd)
					{
					WORD *pSrcPos = Src.GetPixel(Src.GetRowStart(ySrcPos), xSrcPos);
					BYTE byAlpha = *Src.GetAlphaValue(xSrcPos, ySrcPos);

					if (byAlpha == 0)
						*pDestAlpha = 0;
					else if (byAlpha == 255)
						{
						INTERPOLATE(pSrcPos, iRowHeight, vSrcPos, xSrcPos, ySrcPos, *pDestPos);
						*pDestAlpha = 255;
						}
					else
						{
						INTERPOLATE(pSrcPos, iRowHeight, vSrcPos, xSrcPos, ySrcPos, *pDestPos);
						*pDestAlpha = byAlpha;
						}
					}

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
