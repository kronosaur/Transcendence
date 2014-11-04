//	NoiseImpl.h
//
//	Some internal definitions common across image files.
//	Copyright (c) 2014 by Kronosaur Productions, LLC. All Rights Reserved.

//	Noise ----------------------------------------------------------------------

const int PERM_TABLE_SIZE =					256;	//	Must be power of 2
const int PERM_TABLE_MASK =					(PERM_TABLE_SIZE - 1);

extern BYTE g_perm[PERM_TABLE_SIZE];
extern float g_gradient[PERM_TABLE_SIZE * 3];

inline float RANDF (void) { return (float)::rand() / (float)RAND_MAX; }
inline float LERP (float t, float x0, float x1) { return (x0 + t * (x1 - x0)); }
inline float SMOOTHSTEP (float x) { return x * x * (3.0f - 2.0f * x); }

inline BYTE PERM (int x) { return g_perm[x & PERM_TABLE_MASK]; }
inline BYTE INDEX2D (int x, int y) { return PERM(x + PERM(y)); }
inline BYTE INDEX3D (int x, int y, int z) { return PERM(x + PERM(y + PERM(z))); }
inline float RANDVECTOR2D (int x, int y, float fx, float fy)
	{
	float *pVec = &g_gradient[INDEX2D(x, y) * 3];
	return pVec[0] * fx + pVec[1] * fy;
	}
inline float RANDVECTOR3D (int x, int y, int z, float fx, float fy, float fz)
	{
	float *pVec = &g_gradient[INDEX3D(x, y, z) * 3];
	return pVec[0] * fx + pVec[1] * fy + pVec[2] * fz;
	}

//	Stochastic Opacity ---------------------------------------------------------

const int STOCHASTIC_DIM =							16;
const int STOCHASTIC_SIZE =							STOCHASTIC_DIM * STOCHASTIC_DIM;
extern BYTE STOCHASTIC_OPACITY[STOCHASTIC_SIZE][256];

void InitStochasticTable (void);
