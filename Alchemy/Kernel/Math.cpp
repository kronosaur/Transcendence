//	Math.cpp
//
//	Integer math package

#include "Kernel.h"
#include <math.h>

DWORD g_Seed = 0;

DWORD mathGetSeed (void)

//	mathGetSeed
//
//	Returns the current seed value.

	{
	return g_Seed;
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

int mathPower (int x, int n)

//	mathPower
//
//	Returns x raised to the nth power

	{
	if (x == 1)
		return 1;
	else if (n > 0)
		{
		int iResult = 1;

		while (n)
			{
			if (n & 1)
				{
				iResult = iResult * x;
				n = n - 1;
				}
			else
				{
				x = x * x;
				n = n / 2;
				}
			}

		return iResult;
		}
	else if (n == 0)
		return 1;
	else
		return 0;
	}

DWORD mathRandom (void)

//	mathRandom
//
//	Returns a random 31-bit number: 0 to 2^31-1 (2147483647)
//
//	Based on code written by William S. England (Oct 1988) based
//	on:
//
//	Stephen K. Park and Keith W. Miller. RANDOM NUMBER GENERATORS:
//	GOOD ONES ARE HARD TO FIND. Communications of the ACM,
//	New York, NY.,October 1988 p.1192

	{
	const DWORD m = (DWORD)2147483647;
	const DWORD q = (DWORD)44488;

	const DWORD a = (DWORD)48271;
	const DWORD r = (DWORD)3399;

	//	Seed it

	if (g_Seed == 0)
		{
		g_Seed = MAKELONG(rand() % 0x10000, rand() % 0x10000);
		g_Seed *= ::GetTickCount();
		}

	//	Random

	int lo, hi, test;

	hi = g_Seed / q;
	lo = g_Seed % q;

	test = a * lo - r * hi;

	if (test > 0)
		g_Seed = test;
	else
		g_Seed = test + m;

	//	Done

	return g_Seed;
	}

int mathRandom (int iFrom, int iTo)

//	mathRandom
//
//	Returns a random number between iFrom and iTo (inclusive)

	{
	int iRandom;
	int iRange = Absolute(iTo - iFrom) + 1;

	//	mathRandom returns a value from 0 to 2,147,483,647. If our range is less
	//	than 1,000,000 then we just use a single value. Otherwise, we use
	//	DWORDLONGs

	if (iRange < 1000000)
		iRandom = mathRandom() % iRange;
	else
		{
		DWORDLONG dwValue = (mathRandom() << 31) | mathRandom();
		iRandom = (int)(dwValue % (DWORDLONG)iRange);
		}

	return iRandom + iFrom;
	}

double mathRandomMinusOneToOne (void)
	{
	DWORD dwValue = mathRandom();

	if (dwValue % 2)
		return ((dwValue >> 1) / 1073741824.0);
	else
		return ((dwValue >> 1) / -1073741824.0);
	}

double mathRandomGaussian (void)

//	mathRandomGaussian
//
//	Returns a random number with Gaussian distribution. The mean value is 0.0 
//	and the standard deviation is 1.0.
//
//	Uses the polar form of the Box-Muller transformation.
//
//	See: http://www.taygeta.com/random/gaussian.html

	{
	double x1, x2, w;

	do
		{
		x1 = mathRandomMinusOneToOne();
		x2 = mathRandomMinusOneToOne();
		w = x1 * x1 + x2 * x2;
		}
	while (w >= 1.0);

	w = sqrt((-2.0 * log(w)) / w);

	return x1 * w;
	}

int mathRound (double x)

//	mathRound
//
//	Round to the nearest integer.
//	Based on: http://ldesoras.free.fr/doc/articles/rounding_en.pdf

	{
	const float round_to_nearest = 0.5f;
	int i;

#ifndef __GNUC__
	__asm
		{
		fld x
		fadd st, st (0)
		fadd round_to_nearest
		fistp i
		sar i, 1
		}
#else
	//i = floor(x + round_to_nearest); //fallback alternative
	__asm__ __volatile__ (
		"fadd %%st\n\t"
		"fadd %%st(1)\n\t"
		"fistpl %0\n\t"
		"sarl $1, %0\n"
		: "=m"(i) : "u"(round_to_nearest), "t"(x) : "st"
        );
#endif
	return (i);
	}

int mathSeededRandom (int iSeed, int iFrom, int iTo)

//	mathSeededRandom
//
//	Returns a random number between iFrom and iTo (inclusive)
//	The same random number is returned for any given value of iSeed

	{
	int a = 1103515245;
	int c = 12345;

	iSeed = a * iSeed + c;
	int iRandom = iSeed ^ (iSeed >> 11) ^ (iSeed << 5) & (iSeed >> 7) ^ (iSeed << 3) ^ (iSeed >> 13);
	int iRange = Absolute(iTo - iFrom) + 1;

	return iFrom + (Absolute(iRandom) % iRange);
	}

void mathSetSeed (DWORD dwSeed)

//	mathSetSeed
//
//	Sets the seed for seeded random.

	{
	//	Must be a 31-bit number

	g_Seed = (dwSeed &= 0x7fffffff);
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
