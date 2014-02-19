//	SFXParticleComet.cpp
//
//	Particle Comet SFX

#include "PreComp.h"

#define LENGTH_ATTRIB							(CONSTLIT("length"))
#define WIDTH_ATTRIB							(CONSTLIT("width"))
#define PRIMARY_COLOR_ATTRIB					(CONSTLIT("primaryColor"))
#define PARTICLE_COUNT_ATTRIB					(CONSTLIT("particleCount"))
#define SECONDARY_COLOR_ATTRIB					(CONSTLIT("secondaryColor"))

//	CParticleCometEffectCreator object

CParticleCometEffectCreator::CParticleCometEffectCreator (void) : 
		IEffectPainter(true),
		m_vPoints(NULL),
		m_Particles(NULL)

//	CParticleCometEffectCreator constructor

	{
	}

CParticleCometEffectCreator::~CParticleCometEffectCreator (void)

//	CParticleCometEffectCreator destructor

	{
	if (m_vPoints)
		delete [] m_vPoints;

	if (m_Particles)
		delete [] m_Particles;
	}

void CParticleCometEffectCreator::ComputeSplinePoints (void)

//	ComputeSplinePoints
//
//	Initializes m_vPoints
//
//	Spline Equation:
//
//	x(t) = N1x t^3 + 3 C1x t^2 (1-t) + 3 C2x t (1-t)^2 + N2x (1 - t)^3
//	y(t) = N1y t^3 + 3 C1y t^2 (1-t) + 3 C2y t (1-t)^2 + N2y (1 - t)^3
//
//	control point xy are absolute (not relative to node)
//	at t=0, we are at N2

	{
	int i;
	Metric rHeadWidth = 0.5;
	Metric rTailLength = 0.5;
	Metric rDeltaT = 1.0 / ((Metric)POINT_COUNT);

	//	Create the vectors

	CVector vNode[2];
	CVector vControl[2];

	vNode[0] = CVector(0.0, 0.0);
	vControl[0] = CVector(0.0, rHeadWidth);
	vNode[1] = CVector(1.0, 0.0);
	vControl[1] = CVector(1.0 - rTailLength, 0.0);

	//	Allocate the array

	ASSERT(m_vPoints == NULL);
	m_vPoints = new CVector [POINT_COUNT];

	//	Initialize array

	Metric rT = 1.0;
	for (i = 0; i < POINT_COUNT; i++)
		{
		Metric rT2 = (rT * rT);
		Metric rT3 = (rT2 * rT);
		Metric rOneMinusT = (1.0 - rT);
		Metric rOneMinusT2 = (rOneMinusT * rOneMinusT);
		Metric rOneMinusT3 = (rOneMinusT2 * rOneMinusT);
		Metric r3TOneMinusT2 = (3.0 * rT * rOneMinusT2);

#if 0
		m_vPoints[i] = CVector(
				rOneMinusT3 + (r3TOneMinusT2 * -rTailLength),
				(r3TOneMinusT2 * rHeadWidth)
				);
#else
		m_vPoints[i] = (vNode[0] * rT3)
				+ (vControl[0] * 3.0 * rT2 * rOneMinusT)
				+ (vControl[1] * 3.0 * rT * rOneMinusT2)
				+ (vNode[1] * rOneMinusT3);
#endif

		//	Next

		rT -= rDeltaT;
		}
	}

void CParticleCometEffectCreator::CreateParticles (void)

//	CreateParticles
//
//	Creates the particle array

	{
	int i;

	m_Particles = new SParticle [m_iParticleCount];
	for (i = 0; i < m_iParticleCount; i++)
		{
		m_Particles[i].iPos = mathRandom(0, POINT_COUNT-1);
		m_Particles[i].vScale = CVector(
				PolarToVector(mathRandom(0, 179), (Metric)m_iWidth).GetX(),
				m_iLength * mathRandom(80, 150) / 100.0
				);
		}
	}

CVector CParticleCometEffectCreator::GetParticlePos (int iParticle, int iTick, int iDirection, int *retiAge)

//	GetParticlePos
//
//	Returns the position of the given particle

	{
	int iAge = (iTick + m_Particles[iParticle].iPos) % POINT_COUNT;

	CVector vPos = m_vPoints[iAge];
	vPos = CVector(
			vPos.GetX() * m_Particles[iParticle].vScale.GetY(),
			vPos.GetY() * m_Particles[iParticle].vScale.GetX()
			);

	if (retiAge)
		*retiAge = iAge;

	return vPos.Rotate(iDirection + 180);
	}

void CParticleCometEffectCreator::GetRect (RECT *retRect) const

//	GetRect
//
//	Returns the RECT of the effect centered on 0,0

	{
	retRect->left = -100;
	retRect->top = -100;
	retRect->right = 100;
	retRect->bottom = 100;
	}

ALERROR CParticleCometEffectCreator::OnEffectCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, const CString &sUNID)

//	OnEffectCreateFromXML
//
//	Initializes from XML

	{
	m_iParticleCount = pDesc->GetAttributeInteger(PARTICLE_COUNT_ATTRIB);
	m_iLength = pDesc->GetAttributeInteger(LENGTH_ATTRIB);

	//	Note: We need to increase the width because the template is not full width
	m_iWidth = 18 * pDesc->GetAttributeInteger(WIDTH_ATTRIB) / 8;

	m_wPrimaryColor = ::LoadRGBColor(pDesc->GetAttribute(PRIMARY_COLOR_ATTRIB));

	CString sAttrib;
	if (pDesc->FindAttribute(SECONDARY_COLOR_ATTRIB, &sAttrib))
		m_wSecondaryColor = ::LoadRGBColor(sAttrib);
	else
		m_wSecondaryColor = m_wPrimaryColor;

	//	Compute arrays

	ComputeSplinePoints();
	CreateParticles();

	return NOERROR;
	}

void CParticleCometEffectCreator::Paint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx)

//	Paint
//
//	Paint

	{
	int i;

	int iParticleSize = 2;
	int iMaxAge = GetMaxAge();

	//	If we fade the color then we need a different loop

	if (m_wPrimaryColor != m_wSecondaryColor)
		{
		for (i = 0; i < GetParticleCount(); i++)
			{
			int iAge;
			CVector vPos = GetParticlePos(i, Ctx.iTick, Ctx.iRotation, &iAge);
			DWORD dwOpacity = 255 - (iAge * 255 / iMaxAge);
			WORD wColor = CG16bitImage::FadeColor(m_wPrimaryColor, m_wSecondaryColor, 100 * iAge / iMaxAge);

			DrawParticle(Dest,
					x + (int)vPos.GetX(),
					y - (int)vPos.GetY(),
					wColor,
					iParticleSize,
					dwOpacity);
			}
		}
	
	//	Otherwise paint all in one color

	else
		{
		for (i = 0; i < GetParticleCount(); i++)
			{
			int iAge;
			CVector vPos = GetParticlePos(i, Ctx.iTick, Ctx.iRotation, &iAge);
			DWORD dwOpacity = 255 - (iAge * 255 / iMaxAge);

			DrawParticle(Dest,
					x + (int)vPos.GetX(),
					y - (int)vPos.GetY(),
					m_wPrimaryColor,
					iParticleSize,
					dwOpacity);
			}
		}
	}
