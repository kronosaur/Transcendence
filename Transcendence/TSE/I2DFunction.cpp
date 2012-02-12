//	I2DFunction.cpp
//
//	I2DFunction class and descendants

#include "PreComp.h"

#define NOISE_TAG							CONSTLIT("Noise")

#define SCALE_ATTRIB						CONSTLIT("scale")

//	I2DFunction ----------------------------------------------------------------

ALERROR I2DFunction::CreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, I2DFunction **retpFunc)

//	CreateFromXML
//
//	Creates the appropriate function object based on the XML

	{
	ALERROR error;

	I2DFunction *pFunc;
	if (strEquals(pDesc->GetTag(), NOISE_TAG))
		pFunc = new CNoise2DFunc;
	else
		{
		Ctx.sError = strPatternSubst(CONSTLIT("Unknown <MapFunction> element: <%s>"), pDesc->GetTag());
		return ERR_FAIL;
		}

	//	Load

	if (error = pFunc->InitFromXML(Ctx, pDesc))
		return error;

	//	Done

	*retpFunc = pFunc;

	return NOERROR;
	}

//	CNoise2DFunc ---------------------------------------------------------------

float CNoise2DFunc::OnEval (float x, float y)

//	OnEval
//
//	Evaluates function at (x, y) and returns value from 0.0 to 1.0

	{
	//	Apply scale

	x = (x / (float)m_iScale);
	y = (y / (float)m_iScale);

	if ((m_iScale % 2) == 1)
		{
		x += (1.0f / (float)(2 * m_iScale));
		y += (1.0f / (float)(2 * m_iScale));
		}

	//	Compute

	float ix = floorf(x);
	float fx0 = x - ix;
	float fx1 = fx0 - 1.0f;
	float wx = NoiseSmoothStep(fx0);

	float iy = floorf(y);
	float fy0 = y - iy;
	float fy1 = fy0 - 1.0f;
	float wy = NoiseSmoothStep(fy0);

	float rNoise = Noise2D((int)ix, fx0, fx1, wx, (int)iy, fy0, fy1, wy);

	//	Map to 0.0 to 1.0

	return (rNoise + 1.0f) / 2.0f;
	}

ALERROR CNoise2DFunc::OnInitFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	OnInitFromXML
//
//	Initialize from XML

	{
	//	Initialize noise

	NoiseInit();

	//	Load scale

	m_iScale = pDesc->GetAttributeIntegerBounded(SCALE_ATTRIB, 1, -1, 1);
	ASSERT(m_iScale > 0);

	return NOERROR;
	}
