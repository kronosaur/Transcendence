//	Noise.cpp
//
//	Noise code

#include <windows.h>
#include <ddraw.h>
#include "Alchemy.h"
#include "DirectXUtil.h"

#include <math.h>
#include <stdio.h>
#include "NoiseImpl.h"

const int RAND_MASK =					0x7fffffff;
const float PI =						3.1415926535f;

static bool g_bInit = false;
BYTE g_perm[PERM_TABLE_SIZE];
float g_gradient[PERM_TABLE_SIZE * 3];		//	Random 3D vectors

float Noise2D (int x, float fx0, float fx1, float wx, int y, float fy0, float fy1, float wy)

//	Noise2D
//
//	Returns noise at the given position:
//
//	x is integer position floor
//	fx0 is the fraction from x
//	fx1 is the fraction from x + 1
//	wx is the smoothstep
//
//	y is integer position floor
//	fy0 is the fraction from y
//	fy1 is the fraction from y + 1
//	wy is the smoothstep
//
//	BASED ON:
//
//	Texturing & Modeling, 2nd Edition
//	Ebert, David S. et al
//	pp.72-73

	{
	float vx0 = RANDVECTOR2D(x, y, fx0, fy0);
	float vx1 = RANDVECTOR2D(x + 1, y, fx1, fy0);
	float vy0 = LERP(wx, vx0, vx1);

	vx0 = RANDVECTOR2D(x, y + 1, fx0, fy1);
	vx1 = RANDVECTOR2D(x + 1, y + 1, fx1, fy1);
	float vy1 = LERP(wx, vx0, vx1);

	return LERP(wy, vy0, vy1);
	}

float Noise3D (int x, float fx0, float fx1, float wx, int y, float fy0, float fy1, float wy, int z, float fz0, float fz1, float wz)

//	Noise3D
//
//	Same as Noise2D but in 3-space

	{
	float vx0 = RANDVECTOR3D(x, y, z, fx0, fy0, fz0);
	float vx1 = RANDVECTOR3D(x + 1, y, z, fx1, fy0, fz0);
	float vy0 = LERP(wx, vx0, vx1);

	vx0 = RANDVECTOR3D(x, y + 1, z, fx0, fy1, fz0);
	vx1 = RANDVECTOR3D(x + 1, y + 1, z, fx1, fy1, fz0);
	float vy1 = LERP(wx, vx0, vx1);
	float vz0 = LERP(wy, vy0, vy1);

	vx0 = RANDVECTOR3D(x, y, z + 1, fx0, fy0, fz1);
	vx1 = RANDVECTOR3D(x + 1, y, z + 1, fx1, fy0, fz1);
	vy0 = LERP(wx, vx0, vx1);

	vx0 = RANDVECTOR3D(x, y + 1, z + 1, fx0, fy1, fz1);
	vx1 = RANDVECTOR3D(x + 1, y + 1, z + 1, fx1, fy1, fz1);
	vy1 = LERP(wx, vx0, vx1);
	float vz1 = LERP(wy, vy0, vy1);

	return LERP(wz, vz0, vz1);
	}

void NoiseInit (DWORD dwSeed)

//	NoiseInit
//
//	Initialize all the appropriate noise tables

	{
	if (!g_bInit)
		{
		int i;

		if (dwSeed != 0)
			::srand(dwSeed);

		//	Initialize permutation table

		for (i = 0; i < PERM_TABLE_SIZE; i++)
			g_perm[i] = i;

		//	Fisher-Yates shuffle algorithm

		i = PERM_TABLE_SIZE - 1;
		while (i > 0)
			{
			int x = mathRandom(0, i);

			BYTE Temp = g_perm[x];
			g_perm[x] = g_perm[i];
			g_perm[i] = Temp;

			i--;
			}

		//	Now initialize the gradient vector table

		float *pPos = g_gradient;
		float *pPosEnd = g_gradient + (PERM_TABLE_SIZE * 3);
		while (pPos < pPosEnd)
			{
			float z = 1.0f - 2.0f * RANDF();
			float r = ::sqrtf(1.0f - z * z);
			float theta = 2.0f * PI * RANDF();

			*pPos++ = r * cosf(theta);
			*pPos++ = r * sinf(theta);
			*pPos++ = z;
			}

		g_bInit = true;
		}
	}

void NoiseReinit (DWORD dwSeed)

//	Reinit
//
//	Reinitialize with a new random seed

	{
	g_bInit = false;
	NoiseInit(dwSeed);
	}

float NoiseSmoothStep (float x) { return SMOOTHSTEP(x); }