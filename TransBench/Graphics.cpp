//	Graphics.cpp
//
//	Tests various graphics algorithms
//	Copyright (c) 2007 by George Moromisato. All Rights Reserved.

#include <stdio.h>

#include <windows.h>
#include <ddraw.h>
#include "Alchemy.h"
#include "XMLUtil.h"
#include "TransBench.h"

typedef WORD (*BLENDPROC)(WORD pxDest, WORD pxSource, DWORD byTrans);

#define clipByte(x)		((x) > 255 ? (BYTE)255 : (BYTE)(x))
#define clip5bit(x)		((x) > 32 ? (BYTE)32 : (BYTE)(x))
#define clip6bit(x)		((x) > 64 ? (BYTE)64 : (BYTE)(x))

WORD FloatBlendPixel (WORD pxDest, WORD pxSource, DWORD byTrans);
WORD GPMBlendPixel (WORD pxDest, WORD pxSource, DWORD byTrans);
WORD LookupBlendPixel (WORD pxDest, WORD pxSource, DWORD byTrans);
WORD NaiveBlendPixel (WORD pxDest, WORD pxSource, DWORD byTrans);

void InitLookup (void);
void PrintBlend (WORD wBackground, WORD wSource, DWORD dwTrans);
void TestBlendGrayscale (BLENDPROC pfBlend);
void TestParticleBlt (CG16bitImage &Surface, int iRadius);
void TestRun (BLENDPROC pfBlend);

typedef BYTE AlphaArray5 [32];
typedef BYTE AlphaArray6 [64];
static AlphaArray5 Alpha5 [256];
static AlphaArray6 Alpha6 [256];
static bool bInit = false;

void TestParticleBltComparison (void)
	{
	CG16bitImage Surface;
	Surface.CreateBlank(100, 100, false);

	printf("\n4 pixel circle\n");
	TestParticleBlt(Surface, 4);

	printf("\n6 pixel circle\n");
	TestParticleBlt(Surface, 6);

	printf("\n10 pixel circle\n");
	TestParticleBlt(Surface, 10);

	printf("\n20 pixel circle\n");
	TestParticleBlt(Surface, 20);
	}

void TestPixelBlendComparison (void)
	{
	InitLookup();

	printf("\nNaive BlendPixel\n\n");
	TestRun(NaiveBlendPixel);

	printf("\nOriginal BlendPixel\n\n");
	TestRun(GPMBlendPixel);

	printf("\nFloating-Point BlendPixel\n\n");
	TestRun(FloatBlendPixel);
	//TestBlendGrayscale(FloatBlendPixel);

	printf("\nLookup BlendPixel\n\n");
	TestRun(LookupBlendPixel);
	}

void TestParticleBlt (CG16bitImage &Surface, int iRadius)
	{
	int i;
	int iCount = 500000;

	DWORD dwStartTimer = ::GetTickCount();
	for (i = 0; i < iCount; i++)
		DrawFilledCircleTrans(Surface, 50, 50, iRadius, CG16bitImage::RGBValue(128,128,128), 128);
	DWORD dwTime = ::GetTickCount() - dwStartTimer;

	printf("%d iterations: %d ms (%d ms per 1000)\n", iCount, dwTime, dwTime / (iCount / 1000));
	}

void PrintBlend (WORD wBackground, WORD wSource, DWORD dwTrans, BLENDPROC pfBlend)
	{
	WORD wResult = (*pfBlend)(wBackground, wSource, dwTrans);

	printf("Background: RGB(%d,%d,%d) + Source: RGB(%d,%d,%d) @ Opacity: %d = RGB(%d,%d,%d)\n", 
			CG16bitImage::RedValue(wBackground),
			CG16bitImage::GreenValue(wBackground),
			CG16bitImage::BlueValue(wBackground),
			CG16bitImage::RedValue(wSource),
			CG16bitImage::GreenValue(wSource),
			CG16bitImage::BlueValue(wSource),
			dwTrans,
			CG16bitImage::RedValue(wResult),
			CG16bitImage::GreenValue(wResult),
			CG16bitImage::BlueValue(wResult)
			);
	}

void TestRun (BLENDPROC pfBlend)
	{
	WORD wBackground = CG16bitImage::RGBValue(0,0,8);
	WORD wSource = CG16bitImage::RGBValue(50,50,50);

	PrintBlend(wBackground, wSource, 0, pfBlend);
	PrintBlend(wBackground, wSource, 50, pfBlend);
	PrintBlend(wBackground, wSource, 100, pfBlend);
	PrintBlend(wBackground, wSource, 150, pfBlend);
	PrintBlend(wBackground, wSource, 200, pfBlend);
	PrintBlend(wBackground, wSource, 250, pfBlend);
	PrintBlend(wBackground, wSource, 255, pfBlend);

	//	Run

	int i;
	int iCount = 50000000;
	DWORD dwStartTimer = ::GetTickCount();
	WORD wResult = 0;
	for (i = 0; i < iCount; i++)
		{
		wResult += (*pfBlend)(112, i % 256, 56);
		}
	DWORD dwTime = ::GetTickCount() - dwStartTimer;

	printf("%d iterations: %d ms\n", iCount, dwTime);
	}

void TestBlendGrayscale (BLENDPROC pfBlend)
	{
	int i;

	for (i = 0; i < 256; i++)
		{
		PrintBlend(0, CG16bitImage::RGBValue(56, 56, 56), i, pfBlend);
		}
	}

WORD FloatBlendPixel (WORD pxDest, WORD pxSource, DWORD byTrans)

//	BlendPixel
//
//	Blends the dest and the source according to byTrans

	{
	float rOpacity = byTrans / 255.0f;
	float rInvOpacity = 1.0f - rOpacity;

	float rRedDest = CG16bitImage::RedValue(pxDest) * rInvOpacity;
	float rGreenDest = CG16bitImage::GreenValue(pxDest) * rInvOpacity;
	float rBlueDest = CG16bitImage::BlueValue(pxDest) * rInvOpacity;

	float rRedSource = CG16bitImage::RedValue(pxSource) * rOpacity;
	float rGreenSource = CG16bitImage::GreenValue(pxSource) * rOpacity;
	float rBlueSource = CG16bitImage::BlueValue(pxSource) * rOpacity;

	DWORD dwRedResult = (DWORD)(rRedDest + rRedSource + 0.5f);
	DWORD dwGreenResult = (DWORD)(rGreenDest + rGreenSource + 0.5f);
	DWORD dwBlueResult = (DWORD)(rBlueDest + rBlueSource + 0.5f);

	return CG16bitImage::RGBValue((WORD)dwRedResult, (WORD)dwGreenResult, (WORD)dwBlueResult);
	}

void InitLookup (void)
	{
	int i, j, k;

	if (!bInit)
		{
		for (i = 0; i < 32; i++)
			for (j = 0; j < 256; j++)
				Alpha5[j][i] = (BYTE)((DWORD)(((i << 3) * (j / 255.0f)) + 4.0f) >> 3);

		for (i = 0; i < 64; i++)
			for (j = 0; j < 256; j++)
				Alpha6[j][i] = (BYTE)((DWORD)(((i << 2) * (j / 255.0f)) + 2.0f) >> 2);

		//	Test

		for (i = 0; i < 256; i++)
			for (j = 0; j < 32; j++)
				for (k = 0; k < 32; k++)
					{
					DWORD dwResult = Alpha5[i][j] + Alpha5[255 - i][k];
					if (dwResult >= 32)
						printf("OVERFLOW: %d @ %d + %d @ %d = %d\n",
								j, i,
								k, 255 - i,
								dwResult);
					}

		for (i = 0; i < 256; i++)
			for (j = 0; j < 64; j++)
				for (k = 0; k < 64; k++)
					{
					DWORD dwResult = Alpha6[i][j] + Alpha6[255 - i][k];
					if (dwResult >= 64)
						printf("OVERFLOW: %d @ %d + %d @ %d = %d\n",
								j, i,
								k, 255 - i,
								dwResult);
					}

		bInit = true;
		}
	}

WORD LookupBlendPixel (WORD pxDest, WORD pxSource, DWORD byTrans)

//	BlendPixel
//
//	Blends the dest and the source according to byTrans

	{
	BYTE *pAlpha5 = Alpha5[byTrans];
	BYTE *pAlpha6 = Alpha6[byTrans];
	BYTE *pAlpha5Inv = Alpha5[255 - byTrans];
	BYTE *pAlpha6Inv = Alpha6[255 - byTrans];

	DWORD dTemp = pxDest;
	DWORD sTemp = pxSource;

	DWORD dwRedResult = pAlpha5Inv[(dTemp & 0xf800) >> 11] + pAlpha5[(sTemp & 0xf800) >> 11];
	DWORD dwGreenResult = pAlpha6Inv[(dTemp & 0x7e0) >> 5] + pAlpha6[(sTemp & 0x7e0) >> 5];
	DWORD dwBlueResult = pAlpha5Inv[(dTemp & 0x1f)] + pAlpha5[(sTemp & 0x1f)];

	return (WORD)((dwRedResult << 11) | (dwGreenResult << 5) | (dwBlueResult));
	}

WORD NaiveBlendPixel (WORD pxDest, WORD pxSource, DWORD byTrans)

//	BlendPixel
//
//	Blends the dest and the source according to byTrans

	{
	DWORD dwInvTrans = 255 - byTrans;

	DWORD dwRedDest = CG16bitImage::RedValue(pxDest) * dwInvTrans / 255;
	DWORD dwGreenDest = CG16bitImage::GreenValue(pxDest) * dwInvTrans / 255;
	DWORD dwBlueDest = CG16bitImage::BlueValue(pxDest) * dwInvTrans / 255;

	DWORD dwRedSource = CG16bitImage::RedValue(pxSource) * byTrans / 255;
	DWORD dwGreenSource = CG16bitImage::GreenValue(pxSource) * byTrans / 255;
	DWORD dwBlueSource = CG16bitImage::BlueValue(pxSource) * byTrans / 255;

	DWORD dwRedResult = dwRedDest + dwRedSource;
	DWORD dwGreenResult = dwGreenDest + dwGreenSource;
	DWORD dwBlueResult = dwBlueDest + dwBlueSource;

	return CG16bitImage::RGBValue((WORD)dwRedResult, (WORD)dwGreenResult, (WORD)dwBlueResult);
	}

WORD GPMBlendPixel (WORD pxDest, WORD pxSource, DWORD byTrans)

//	BlendPixel
//
//	Blends the dest and the source according to byTrans

	{
	DWORD sTemp = pxSource;
	DWORD dTemp = pxDest;

	DWORD sb = sTemp & 0x1f;
	DWORD db = dTemp & 0x1f;
	DWORD sg = (sTemp >> 5) & 0x3f;
	DWORD dg = (dTemp >> 5) & 0x3f;
	DWORD sr = (sTemp >> 11) & 0x1f;
	DWORD dr = (dTemp >> 11) & 0x1f;

	return (WORD)((byTrans * (sb - db) >> 8) + db |
			((byTrans * (sg - dg) >> 8) + dg) << 5 |
			((byTrans * (sr - dr) >> 8) + dr) << 11);
	}

