//	SFXLightningStorm.cpp
//
//	CLightningStormEffectCreator class
//	Copyright (c) 2014 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"

#define INTENSITY_ATTRIB				CONSTLIT("intensity")
#define LIFETIME_ATTRIB					CONSTLIT("lifetime")
#define PRIMARY_COLOR_ATTRIB			CONSTLIT("primaryColor")
#define SECONDARY_COLOR_ATTRIB			CONSTLIT("secondaryColor")
#define STYLE_ATTRIB					CONSTLIT("style")

const Metric DEFAULT_ARC_COUNT =		5.0;
const int DEFAULT_INTENSITY =			50;
const int DEFAULT_OBJ_RADIUS =			24;

const int DEFAULT_ARC_HALF_LENGTH =		20;

//	To calculate the arc count from intensity:
//
//	count = pow(intensity, INTENSITY_POWER) * INTENSITY_FACTOR
//
//	This is scaled so that default intensity (50) has arc count = DEFAULT_ARC_COUNT (5).

const Metric INTENSITY_POWER =			2.0;
const Metric INTENSITY_FACTOR =			(1.0 / (pow(DEFAULT_INTENSITY, INTENSITY_POWER) / DEFAULT_ARC_COUNT));

class CLightningStormEffectPainter : public IEffectPainter
	{
	public:
		CLightningStormEffectPainter (CEffectCreator *pCreator);
		~CLightningStormEffectPainter (void);

		//	IEffectPainter virtuals

		virtual CEffectCreator *GetCreator (void) { return m_pCreator; }
		virtual int GetLifetime (void) { return m_iLifetime; }
		virtual void GetParam (const CString &sParam, CEffectParamDesc *retValue);
		virtual bool GetParamList (TArray<CString> *retList) const;
		virtual void GetRect (RECT *retRect) const;
		virtual void Paint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx);
		virtual bool PointInImage (int x, int y, int iTick, int iVariant = 0, int iRotation = 0) const;
		virtual void SetParam (CCreatePainterCtx &Ctx, const CString &sParam, const CEffectParamDesc &Value);

	private:
		enum EStyles
			{
			styleUnknown =			0,

			styleObjectArcs =		1,

			styleMax =				1,
			};

		struct SBoltDesc
			{
			CVector vPos;					//	Position of bolt. For styleObjectArcs,
											//		this is a polar position relative to
											//		the center of the object (not the effect)
											//		in pixels.
			};

		void CalcMetrics (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx);
		void Invalidate (void);
		void PaintObjectArcs (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx);

		CEffectCreator *m_pCreator;

		EStyles m_iStyle;
		int m_iIntensity;
		WORD m_wPrimaryColor;
		WORD m_wSecondaryColor;

		int m_iLifetime;

		//	Temporary variables based on shape/style/etc.

		bool m_bInitialized;				//	TRUE if values are valid
		int m_iComputedRadius;				//	Radius of object
		TArray<SBoltDesc> m_Bolts;			//	Bolt state
	};

static LPSTR STYLE_TABLE[] =
	{
	//	Must be same order as EStyles
		"",

		"objectArcs",

		NULL,
	};

CLightningStormEffectCreator::CLightningStormEffectCreator (void) :
			m_pSingleton(NULL)

//	CLightningStormEffectCreator constructor

	{
	}

CLightningStormEffectCreator::~CLightningStormEffectCreator (void)

//	COrbEffectCreator destructor

	{
	if (m_pSingleton)
		delete m_pSingleton;
	}

IEffectPainter *CLightningStormEffectCreator::CreatePainter (CCreatePainterCtx &Ctx)

//	CreatePainter
//
//	Creates a new painter
	
	{
	//	If we have a singleton, return that

	if (m_pSingleton)
		return m_pSingleton;

	//	Otherwise we need to create a painter with the actual
	//	parameters.

	IEffectPainter *pPainter = new CLightningStormEffectPainter(this);

	//	Initialize the painter parameters

	pPainter->SetParam(Ctx, STYLE_ATTRIB, m_Style);
	pPainter->SetParam(Ctx, INTENSITY_ATTRIB, m_Intensity);
	pPainter->SetParam(Ctx, PRIMARY_COLOR_ATTRIB, m_PrimaryColor);
	pPainter->SetParam(Ctx, SECONDARY_COLOR_ATTRIB, m_SecondaryColor);
	pPainter->SetParam(Ctx, LIFETIME_ATTRIB, m_Lifetime);

	//	Initialize via GetParameters, if necessary

	InitPainterParameters(Ctx, pPainter);

	//	If we are a singleton, then we only need to create this once.

	if (GetInstance() == instGame)
		{
		pPainter->SetSingleton(true);
		m_pSingleton = pPainter;
		}

	return pPainter;
	}

ALERROR CLightningStormEffectCreator::OnEffectCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, const CString &sUNID)

//	OnEffectCreateFromXML
//
//	Initialize type-specific data

	{
	ALERROR error;

	if (error = m_Style.InitIdentifierFromXML(Ctx, pDesc->GetAttribute(STYLE_ATTRIB), STYLE_TABLE))
		return error;

	if (error = m_Intensity.InitIntegerFromXML(Ctx, pDesc->GetAttribute(INTENSITY_ATTRIB)))
		return error;

	if (error = m_PrimaryColor.InitColorFromXML(Ctx, pDesc->GetAttribute(PRIMARY_COLOR_ATTRIB)))
		return error;

	if (error = m_SecondaryColor.InitColorFromXML(Ctx, pDesc->GetAttribute(SECONDARY_COLOR_ATTRIB)))
		return error;

	if (error = m_Lifetime.InitIntegerFromXML(Ctx, pDesc->GetAttribute(LIFETIME_ATTRIB)))
		return error;

	return NOERROR;
	}

ALERROR CLightningStormEffectCreator::OnEffectBindDesign (SDesignLoadCtx &Ctx)

//	OnEffectBindDesign
//
//	Resolve loading

	{
	//	Clean up, because we might want to recompute for next time.

	if (m_pSingleton)
		{
		delete m_pSingleton;
		m_pSingleton = NULL;
		}

	return NOERROR;
	}

//	CLightningStormEffectPainter -----------------------------------------------

CLightningStormEffectPainter::CLightningStormEffectPainter (CEffectCreator *pCreator) : 
		m_pCreator(pCreator),
		m_iStyle(styleObjectArcs),
		m_iIntensity(50),
		m_wPrimaryColor(CG16bitImage::RGBValue(255, 255, 255)),
		m_wSecondaryColor(CG16bitImage::RGBValue(128, 128, 128)),
		m_iLifetime(0),
		m_bInitialized(false)

//	COrbEffectCreator constructor

	{
	}

CLightningStormEffectPainter::~CLightningStormEffectPainter (void)

//	COrbEffectCreator destructor

	{
	//	Clean up temporaries

	Invalidate();
	}

void CLightningStormEffectPainter::CalcMetrics (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx)

//	CalcMetrics
//
//	Calculates metrics required for painting.

	{
	int i;

	//	Depends on the style

	switch (m_iStyle)
		{
		case styleObjectArcs:
			{
			//	Compute the object size (as a radius).

			if (Ctx.pObj)
				m_iComputedRadius = RectWidth(Ctx.pObj->GetImage().GetImageRect()) / 2;
			else
				m_iComputedRadius = DEFAULT_OBJ_RADIUS;

			//	Figure out how many arcs based on the intensity and object size

			Metric rRadiusAdj = Max(1.0, ((Metric)m_iComputedRadius / DEFAULT_OBJ_RADIUS));
			int iArcCount = (int)(pow(m_iIntensity, INTENSITY_POWER) * INTENSITY_FACTOR * rRadiusAdj);

			//	Create all the arcs starting at the center of the object.
			//	
			//	NOTE: For styleObjectArcs we expect a polar vector relative to 
			//	the center of the object.
			//
			//	LATER: We should offset based on the center of the effect.

			m_Bolts.DeleteAll();
			m_Bolts.InsertEmpty(iArcCount);

			for (i = 0; i < m_Bolts.GetCount(); i++)
				m_Bolts[i].vPos = CVector(0.0, 0.0);

			break;
			}
		}

	//	Done

	m_bInitialized = true;
	}

void CLightningStormEffectPainter::GetParam (const CString &sParam, CEffectParamDesc *retValue)

//	GetParam
//
//	Returns the parameter

	{
	if (strEquals(sParam, INTENSITY_ATTRIB))
		retValue->InitInteger(m_iIntensity);

	else if (strEquals(sParam, LIFETIME_ATTRIB))
		retValue->InitInteger(m_iLifetime);

	else if (strEquals(sParam, PRIMARY_COLOR_ATTRIB))
		retValue->InitColor(m_wPrimaryColor);

	else if (strEquals(sParam, SECONDARY_COLOR_ATTRIB))
		retValue->InitColor(m_wSecondaryColor);
	
	else if (strEquals(sParam, STYLE_ATTRIB))
		retValue->InitInteger(m_iStyle);

	else
		retValue->InitNull();
	}

bool CLightningStormEffectPainter::GetParamList (TArray<CString> *retList) const

//	GetParamList
//
//	Returns a list of value parameter names

	{
	retList->DeleteAll();
	retList->InsertEmpty(5);
	retList->GetAt(0) = INTENSITY_ATTRIB;
	retList->GetAt(1) = LIFETIME_ATTRIB;
	retList->GetAt(2) = PRIMARY_COLOR_ATTRIB;
	retList->GetAt(3) = SECONDARY_COLOR_ATTRIB;
	retList->GetAt(4) = STYLE_ATTRIB;

	return true;
	}

void CLightningStormEffectPainter::GetRect (RECT *retRect) const

//	GetRect
//
//	Returns the rect bounds of the image

	{
	int iSize = (m_bInitialized ? m_iComputedRadius : DEFAULT_OBJ_RADIUS);

	retRect->left = -iSize;
	retRect->right = iSize;
	retRect->top = -iSize;
	retRect->bottom = iSize;
	}

void CLightningStormEffectPainter::Invalidate (void)

//	Invalidate
//
//	Free up temporaries

	{
	m_bInitialized = false;
	m_Bolts.DeleteAll();
	}

void CLightningStormEffectPainter::Paint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx)

//	Paint
//
//	Paint the effect

	{
	//	Make sure we're initialized.

	if (!m_bInitialized)
		CalcMetrics(Dest, x, y, Ctx);

	//	Paint

	switch (m_iStyle)
		{
		case styleObjectArcs:
			PaintObjectArcs(Dest, x, y, Ctx);
			break;
		}
	}

void CLightningStormEffectPainter::PaintObjectArcs (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx)

//	PaintObjectArcs
//
//	Paints object arcs style. We assume that we've been initialized.

	{
	int i;

	ASSERT(m_bInitialized);

	//	Compute some object-specific variables

	int iRotation = (Ctx.pObj ? Ctx.pObj->GetRotation() : 0);

	int xCenter;
	int yCenter;
	if (Ctx.pObj)
		Ctx.XForm.Transform(Ctx.pObj->GetPos(), &xCenter, &yCenter);
	else
		{
		xCenter = x;
		yCenter = y;
		}

	const CObjectImageArray *pImage = (Ctx.pObj ? &Ctx.pObj->GetImage() : NULL);
	int iRotationFrame = ((pImage && pImage->GetRotationCount() > 0) ? Angle2Direction(0, pImage->GetRotationCount()) : 0);

	//	Paint each bolt

	for (i = 0; i < m_Bolts.GetCount(); i++)
		{
		//	Convert from polar to Cartessian, based on the rotation of the object.
		//	This is the starting position of the bolt relative to the object center.

		CVector vStart = PolarToVector((int)m_Bolts[i].vPos.GetX() + iRotation, m_Bolts[i].vPos.GetY());

		//	Pick a destination position for the bolt (again, relative to the object
		//	center).

		CVector vEndPolar;
		if (Ctx.pObj)
			{
			CVector vCenter = PolarToVector((int)m_Bolts[i].vPos.GetX(), m_Bolts[i].vPos.GetY());

			bool bFound = false;
			int iTriesLeft = 5;
			while (iTriesLeft-- > 0)
				{
				//	Randomly move to another point on the image

				CVector vNewPos = vCenter + CVector((Metric)mathRandom(-DEFAULT_ARC_HALF_LENGTH, DEFAULT_ARC_HALF_LENGTH), (Metric)mathRandom(-DEFAULT_ARC_HALF_LENGTH, DEFAULT_ARC_HALF_LENGTH));

				//	Convert to polar coordinates again

				Metric rRadius;
				int iAngle = VectorToPolar(vNewPos, &rRadius);
				vEndPolar = CVector((Metric)iAngle, rRadius);

				//	If we're on the image, then we're OK

				if (pImage == NULL || pImage->PointInImage((int)vNewPos.GetX(), -(int)vNewPos.GetY(), 0, iRotationFrame))
					{
					bFound = true;
					break;
					}
				}

			//	If we could not find a good point, then pick a random one inside the radius

			if (!bFound)
				vEndPolar = CVector((Metric)mathRandom(0, 359), (Metric)mathRandom(0, m_iComputedRadius));
			}
		else
			vEndPolar = CVector((Metric)mathRandom(0, 359), (Metric)mathRandom(0, m_iComputedRadius));

		CVector vEnd = PolarToVector((int)vEndPolar.GetX() + iRotation, vEndPolar.GetY());

		//	Paint it

		DrawLightning(Dest,
				xCenter + (int)vStart.GetX(),
				yCenter - (int)vStart.GetY(),
				xCenter + (int)vEnd.GetX(),
				yCenter - (int)vEnd.GetY(),
				m_wPrimaryColor,
				16,
				0.4);

		//	Remember where we left off

		m_Bolts[i].vPos = vEndPolar;
		}
	}

bool CLightningStormEffectPainter::PointInImage (int x, int y, int iTick, int iVariant, int iRotation) const

//	PointInImage
//
//	Returns TRUE if the given point is in the image

	{
	//	We only intersect if we are inside a box around the center of 1/2 the
	//	width (since a lot of the width is taken up by glow effects).

	int iSize = (m_bInitialized ? m_iComputedRadius : DEFAULT_OBJ_RADIUS) / 2;
	return (Absolute(x) <= iSize && Absolute(y) <= iSize);
	}

void CLightningStormEffectPainter::SetParam (CCreatePainterCtx &Ctx, const CString &sParam, const CEffectParamDesc &Value)

//	SetParam
//
//	Sets parameters

	{
	if (strEquals(sParam, INTENSITY_ATTRIB))
		m_iIntensity = Value.EvalIntegerBounded(Ctx, 0, 100, 50);

	else if (strEquals(sParam, LIFETIME_ATTRIB))
		m_iLifetime = Value.EvalIntegerBounded(Ctx, 0, -1, 0);

	else if (strEquals(sParam, PRIMARY_COLOR_ATTRIB))
		m_wPrimaryColor = Value.EvalColor(Ctx);

	else if (strEquals(sParam, SECONDARY_COLOR_ATTRIB))
		m_wSecondaryColor = Value.EvalColor(Ctx);
	
	else if (strEquals(sParam, STYLE_ATTRIB))
		m_iStyle = (EStyles)Value.EvalIdentifier(Ctx, STYLE_TABLE, styleMax, styleObjectArcs);
	}
