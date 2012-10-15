//	8bitNoise.cpp
//
//	Drawing routines for noise and turbulence

#include <windows.h>
#include <ddraw.h>
#include "Alchemy.h"
#include "DirectXUtil.h"
#include <math.h>
#include <stdio.h>

void DrawNebulosity8bit (CG16bitImage &Dest, int x, int y, int cxWidth, int cyHeight, int iScale, BYTE byMin, BYTE byMax)

//	DrawNebulosity8bit
//
//	Draws 8-bit clouds.

	{
	int i;

	ASSERT(iScale > 0);

	NoiseInit();

	//	Make sure we're in bounds

	if (!Dest.HasAlpha() || !Dest.AdjustCoords(NULL, NULL, 0, 0, 
			&x, &y, 
			&cxWidth, &cyHeight))
		return;

	//	We need noise generators for each frequency

	struct SFreq
		{
		int iFreq;
		float Amplitude;
		CNoiseGenerator *pNoise;
		SNoisePos x;
		SNoisePos y;
		};

	TArray<SFreq> FreqList;

	//	At iMinFreq = 1 we create detail all the way to the pixel level.

	int iMinFreq = 1;//Max(iScale / 256, 1);

	//	In theory, the noise function returns values from -1.0 to 1.0, but
	//	in practice the values are clutered between -0.5 to 0.5. We multiply
	//	the result by this amplitude adjustment so that we get a greater
	//	dynamic range.

	float rAmplitudeAdj = 2.0f;

	//	As we get to smaller and smaller detail, the amplitude decreases
	//	(by half for each frequency). But if there are too many levels of
	//	detail, the amplitude decreases so much that it is invisible.
	//	Thus we have a minimum amplitude.

	float rMinAmplitude = (float)iScale / 32.0f;

	//	Create noise generators are each frequency.

	float rMaxValue = 0.0;
	int iFreq = iScale;
	while (iFreq >= iMinFreq)
		{
		SFreq *pFreq = FreqList.Insert();

		pFreq->iFreq = iFreq;
		pFreq->pNoise = new CNoiseGenerator(iFreq);
		pFreq->Amplitude = rAmplitudeAdj * Max((float)iFreq, rMinAmplitude);

		rMaxValue += (pFreq->Amplitude / rAmplitudeAdj);

		iFreq = iFreq / 2;
		};

	//	Compute the factor to adjust the pixel value

	float rRange = (float)(byMax - byMin) + 0.99999f;
	float rFactor = rRange / (2.0f * rMaxValue);

	int iFreqCount = FreqList.GetCount();

	//	Fill

	for (i = 0; i < iFreqCount; i++)
		FreqList[i].pNoise->Reset(FreqList[i].y, y);

	BYTE *pRow = Dest.GetAlphaRow(y);
	BYTE *pRowEnd = Dest.GetAlphaRow(y + cyHeight);
	while (pRow < pRowEnd)
		{
		for (i = 0; i < iFreqCount; i++)
			FreqList[i].pNoise->Reset(FreqList[i].x, x);

		BYTE *pPos = pRow;
		BYTE *pPosEnd = pRow + cxWidth;
		while (pPos < pPosEnd)
			{
			float rValue = rMaxValue;
			for (i = 0; i < iFreqCount; i++)
				{
				SFreq *pFreq = &FreqList[i];
				rValue += pFreq->pNoise->GetAt(pFreq->x, pFreq->y) * pFreq->Amplitude;

				pFreq->pNoise->Next(pFreq->x);
				}

			*pPos = (BYTE)Clamp((DWORD)byMin + (DWORD)(rFactor * rValue), (DWORD)byMin, (DWORD)byMax);

			//	Next pixel

			pPos++;
			}

		//	Next row

		pRow = Dest.NextAlphaRow(pRow);
		for (i = 0; i < iFreqCount; i++)
			FreqList[i].pNoise->Next(FreqList[i].y);
		}

	//	Free the noise generators

	for (i = 0; i < iFreqCount; i++)
		delete FreqList[i].pNoise;
	}

void DrawNoise8bit (CG16bitImage &Dest, int x, int y, int cxWidth, int cyHeight, int iScale, BYTE byMin, BYTE byMax)

//	DrawNoise8bit
//
//	Draws 8-bit Perlin noise

	{
	ASSERT(iScale > 0);

	NoiseInit();

	//	Make sure we're in bounds

	if (!Dest.HasAlpha() || !Dest.AdjustCoords(NULL, NULL, 0, 0, 
			&x, &y, 
			&cxWidth, &cyHeight))
		return;

	//	Noise returns numbers from -1.0 to 1.0, so we need to scale to appropriate values
	//	Note: We add 0.99999 because we will later truncate to integers and we want
	//	255.0 - 255.99999 to equal 255. We can't add more 9s because floating-point precision
	//	will turn it into 256.0.

	float rRange = (float)(byMax - byMin) + 0.99999f;
	float rFactor = rRange / 2.0f;

	//	Prepare

	CNoiseGenerator Noise(iScale);
	SNoisePos xP, yP;

	//	Fill

	Noise.Reset(yP, y);

	BYTE *pRow = Dest.GetAlphaRow(y);
	BYTE *pRowEnd = Dest.GetAlphaRow(y + cyHeight);
	while (pRow < pRowEnd)
		{
		Noise.Reset(xP, x);

		BYTE *pPos = pRow;
		BYTE *pPosEnd = pRow + cxWidth;
		while (pPos < pPosEnd)
			{
			float rNoise = Noise.GetAt(xP, yP);
			*pPos = byMin + (BYTE)(DWORD)(rFactor * (1.0f + rNoise));

			//	Next pixel

			pPos++;
			Noise.Next(xP);
			}

		//	Next row

		pRow = Dest.NextAlphaRow(pRow);
		Noise.Next(yP);
		}
	}


