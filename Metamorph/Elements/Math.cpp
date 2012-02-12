//	Math.cpp
//
//	Miscellaneous Math Routines
//	Copyright (c) 2000 by NeuroHack, Inc. All Rights Reserved.

#include "Elements.h"

#include <math.h>

int intNumberOfBitsNeeded (int iPowerOf2)
	{
	int i = 0;
	while (iPowerOf2 != 1)
		{
		iPowerOf2 = iPowerOf2 >> 1;
		i++;
		}

	return i;
	}

int intReverseBits (int iIndex, int iNumBits)
	{
    int i, rev;

    for (i = rev = 0; i < iNumBits; i++)
		{
        rev = (rev << 1) | (iIndex & 1);
        iIndex >>= 1;
		}

    return rev;
	}

void mathFFT (int iSamples,
			  double *pRealIn,
			  double *pImagIn,
			  double *pRealOut,
			  double *pImagOut,
			  bool bInverse)

//	mathFFT
//
//	iSamples must be a power of 2.
//	pImagIn may be NULL; all other arrays must be allocated to at least iSamples size
//
//	Based on code by Don Cross <dcross@intersrv.com>
//	http://www.intersrv.com/~dcross/fft.html

	{
	ASSERT(iSamples == mathNearestPowerOf2(iSamples));

	int i, j, k, n;
	int iNumBits;			//	Number of bits needed to store indeces
	int iBlockSize, iBlockEnd;

	//	Deal with NULL pImagOut

	bool bFreeImagOut = false;
	if (pImagOut == NULL)
		{
		pImagOut = new double [iSamples];
		bFreeImagOut = true;
		}

	//	Setup

	double rAngleNumerator = 2.0 * g_Pi;
    double tr, ti;			//	temp real, temp imaginary

    if (bInverse)
        rAngleNumerator = -rAngleNumerator;

	//	Do simultaneous data copy and bit-reversal ordering into outputs...

    iNumBits = intNumberOfBitsNeeded(iSamples);
    for (i = 0; i < iSamples; i++)
		{
        j = intReverseBits(i, iNumBits);
		pRealOut[j] = pRealIn[i];
		pImagOut[j] = (pImagIn == NULL) ? 0.0 : pImagIn[i];
		}

	//	Do the FFT itself...

	iBlockEnd = 1;
	for (iBlockSize = 2; iBlockSize <= iSamples; iBlockSize <<= 1 )
	    {
        double rDeltaAngle = rAngleNumerator / (double)iBlockSize;
        double sm2 = sin(-2 * rDeltaAngle);
        double sm1 = sin(-rDeltaAngle);
        double cm2 = cos(-2 * rDeltaAngle);
        double cm1 = cos(-rDeltaAngle);
        double w = 2 * cm1;
        double ar[3], ai[3];

        for (i = 0; i < iSamples; i += iBlockSize)
			{
            ar[2] = cm2;
            ar[1] = cm1;

            ai[2] = sm2;
            ai[1] = sm1;

            for (j = i, n = 0; n < iBlockEnd; j++, n++)
				{
                ar[0] = w * ar[1] - ar[2];
                ar[2] = ar[1];
                ar[1] = ar[0];

                ai[0] = w * ai[1] - ai[2];
                ai[2] = ai[1];
                ai[1] = ai[0];

                k = j + iBlockEnd;
                tr = ar[0] * pRealOut[k] - ai[0] * pImagOut[k];
                ti = ar[0] * pImagOut[k] + ai[0] * pRealOut[k];

                pRealOut[k] = pRealOut[j] - tr;
                pImagOut[k] = pImagOut[j] - ti;

                pRealOut[j] += tr;
                pImagOut[j] += ti;
				}
			}

        iBlockEnd = iBlockSize;
		}

    //	Need to normalize if inverse transform...

    if (bInverse)
		{
        double denom = (double)iSamples;

        for (i = 0; i < iSamples; i++)
			{
            pRealOut[i] /= denom;
            pImagOut[i] /= denom;
			}
		}

	//	Done

	if (bFreeImagOut)
		delete [] pImagOut;
	}

int mathNearestPowerOf2 (int x)

//	mathNearestPowerOf2
//
//	Returns the largest power of 2 that is <= x
//	NOTE: This doesn't work for very large integers because we overflow
//	iResult.

	{
	int iResult;

	iResult = 1;
	while (x > 0)
		{
		x = x >> 1;
		iResult = iResult << 1;
		}

	return iResult >> 1;
	}

int mathSqrt (int x)

//	mathSqrt
//
//	Returns the square root of x

	{
	//	For now we use a floating point method
#ifndef LATER
	return (int)sqrt((double)x);
#endif
	}

int mathRandom (int iFrom, int iTo)

//	mathRandom
//
//	Returns a random number between iFrom and iTo (inclusive)

	{
	int iRandom;
	int iRange = Absolute(iTo - iFrom) + 1;

	if (iRange > RAND_MAX)
		iRandom = ((1000 * rand()) + (rand() % 1000)) % iRange;
	else
		iRandom = rand() % iRange;

	return iRandom + iFrom;
	}

