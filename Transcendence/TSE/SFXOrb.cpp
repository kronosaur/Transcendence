//	SFXOrb.cpp
//
//	COrbEffectCreator class
//	Copyright (c) 2014 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"

#define ANIMATE_ATTRIB					CONSTLIT("animate")
#define INTENSITY_ATTRIB				CONSTLIT("intensity")
#define LIFETIME_ATTRIB					CONSTLIT("lifetime")
#define PRIMARY_COLOR_ATTRIB			CONSTLIT("primaryColor")
#define RADIUS_ATTRIB					CONSTLIT("radius")
#define SECONDARY_COLOR_ATTRIB			CONSTLIT("secondaryColor")
#define STYLE_ATTRIB					CONSTLIT("style")

class COrbEffectPainter : public IEffectPainter
	{
	public:
		COrbEffectPainter (CEffectCreator *pCreator);
		~COrbEffectPainter (void);

		//	IEffectPainter virtuals

		virtual bool CanPaintComposite (void) { return true; }
		virtual CEffectCreator *GetCreator (void) { return m_pCreator; }
		virtual int GetLifetime (void) { return m_iLifetime; }
		virtual void GetParam (const CString &sParam, CEffectParamDesc *retValue);
		virtual bool GetParamList (TArray<CString> *retList) const;
		virtual void GetRect (RECT *retRect) const;
		virtual void Paint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx);
		virtual void PaintComposite (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx);
		virtual bool PointInImage (int x, int y, int iTick, int iVariant = 0, int iRotation = 0) const;
		virtual void SetParam (CCreatePainterCtx &Ctx, const CString &sParam, const CEffectParamDesc &Value);

	private:
		enum EAnimationTypes
			{
			animateNone =			0,

			animateFade =			1,
			};

		enum EOrbStyles
			{
			styleUnknown =			0,

			styleSmooth =			1,

			styleMax =				1,
			};

		void CalcSphericalTables (void);
		void Invalidate (void);

		CEffectCreator *m_pCreator;

		int m_iRadius;
		EOrbStyles m_iStyle;
		int m_iIntensity;
		WORD m_wPrimaryColor;
		WORD m_wSecondaryColor;

		int m_iLifetime;
		EAnimationTypes m_iAnimation;

		//	Temporary variables based on shape/style/etc.

		bool m_bInitialized;				//	TRUE if values are valid

		WORD *m_pColorTable;
		BYTE *m_pOpacityTable;
	};

static LPSTR ANIMATION_TABLE[] =
	{
	//	Must be same order as EAnimationTypes
		"",

		"fade",

		NULL
	};

static LPSTR STYLE_TABLE[] =
	{
	//	Must be same order as ERayStyles
		"",

		"smooth",

		NULL,
	};

COrbEffectCreator::COrbEffectCreator (void) :
			m_pSingleton(NULL)

//	COrbEffectCreator constructor

	{
	}

COrbEffectCreator::~COrbEffectCreator (void)

//	COrbEffectCreator destructor

	{
	if (m_pSingleton)
		delete m_pSingleton;
	}

IEffectPainter *COrbEffectCreator::CreatePainter (CCreatePainterCtx &Ctx)

//	CreatePainter
//
//	Creates a new painter
	
	{
	//	If we have a singleton, return that

	if (m_pSingleton)
		return m_pSingleton;

	//	Otherwise we need to create a painter with the actual
	//	parameters.

	IEffectPainter *pPainter = new COrbEffectPainter(this);

	//	Initialize the painter parameters

	pPainter->SetParam(Ctx, ANIMATE_ATTRIB, m_Animate);
	pPainter->SetParam(Ctx, RADIUS_ATTRIB, m_Radius);
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

ALERROR COrbEffectCreator::OnEffectCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, const CString &sUNID)

//	OnEffectCreateFromXML
//
//	Initialize type-specific data

	{
	ALERROR error;

	if (error = m_Animate.InitIdentifierFromXML(Ctx, pDesc->GetAttribute(ANIMATE_ATTRIB), ANIMATION_TABLE))
		return error;

	if (error = m_Radius.InitIntegerFromXML(Ctx, pDesc->GetAttribute(RADIUS_ATTRIB)))
		return error;

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

ALERROR COrbEffectCreator::OnEffectBindDesign (SDesignLoadCtx &Ctx)

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

//	COrbEffectPainter ----------------------------------------------------------

COrbEffectPainter::COrbEffectPainter (CEffectCreator *pCreator) : 
		m_pCreator(pCreator),
		m_iRadius((int)(STD_SECONDS_PER_UPDATE * LIGHT_SECOND / KLICKS_PER_PIXEL)),
		m_iStyle(styleSmooth),
		m_iIntensity(50),
		m_wPrimaryColor(CG16bitImage::RGBValue(255, 255, 255)),
		m_wSecondaryColor(CG16bitImage::RGBValue(128, 128, 128)),
		m_iLifetime(0),
		m_iAnimation(animateNone),
		m_bInitialized(false),
		m_pColorTable(NULL),
		m_pOpacityTable(NULL)

//	COrbEffectCreator constructor

	{
	}

COrbEffectPainter::~COrbEffectPainter (void)

//	COrbEffectCreator destructor

	{
	//	Clean up temporaries

	Invalidate();
	}

void COrbEffectPainter::CalcSphericalTables (void)

//	CalcSphericalTables
//
//	Computes m_pColorTable and m_pOpacityTable

	{
	int i;

	if (!m_bInitialized)
		{
		Invalidate();
		if (m_iRadius <= 0)
			return;

		//	Allocate tables

		m_pColorTable = new WORD [m_iRadius];
		m_pOpacityTable = new BYTE [m_iRadius];

		//	Compute some temporaries

		int iFringeMaxRadius = m_iRadius * m_iIntensity / 120;
		int iFringeWidth = iFringeMaxRadius / 8;
		int iBlownRadius = iFringeMaxRadius - iFringeWidth;
		int iFadeWidth = m_iRadius - iFringeMaxRadius;

		//	Initialize table

		for (i = 0; i < m_iRadius; i++)
			{
			if (i < iBlownRadius)
				{
				m_pColorTable[i] = CG16bitImage::RGBValue(255, 255, 255);
				m_pOpacityTable[i] = 255;
				}
			else if (i < iFringeMaxRadius && iFringeWidth > 0)
				{
				int iStep = (i - iBlownRadius);
				DWORD dwOpacity = iStep * 255 / iFringeWidth;
				m_pColorTable[i] = CG16bitImage::BlendPixel(CG16bitImage::RGBValue(255, 255, 255), m_wPrimaryColor, dwOpacity);
				m_pOpacityTable[i] = 255;
				}
			else if (iFadeWidth > 0)
				{
				int iStep = (i - iFringeMaxRadius);
				Metric rOpacity = 255.0 - (iStep * 255.0 / iFadeWidth);
				rOpacity = (rOpacity * rOpacity) / 255.0;
				m_pColorTable[i] = m_wSecondaryColor;
				m_pOpacityTable[i] = (BYTE)(DWORD)rOpacity;
				}
			else
				{
				m_pColorTable[i] = 0;
				m_pOpacityTable[i] = 0;
				}
			}

		m_bInitialized = true;
		}
	}

void COrbEffectPainter::GetParam (const CString &sParam, CEffectParamDesc *retValue)

//	GetParam
//
//	Returns the parameter

	{
	if (strEquals(sParam, ANIMATE_ATTRIB))
		retValue->InitInteger(m_iAnimation);

	else if (strEquals(sParam, INTENSITY_ATTRIB))
		retValue->InitInteger(m_iIntensity);

	else if (strEquals(sParam, LIFETIME_ATTRIB))
		retValue->InitInteger(m_iLifetime);

	else if (strEquals(sParam, PRIMARY_COLOR_ATTRIB))
		retValue->InitColor(m_wPrimaryColor);

	else if (strEquals(sParam, RADIUS_ATTRIB))
		retValue->InitInteger(m_iRadius);

	else if (strEquals(sParam, SECONDARY_COLOR_ATTRIB))
		retValue->InitColor(m_wSecondaryColor);
	
	else if (strEquals(sParam, STYLE_ATTRIB))
		retValue->InitInteger(m_iStyle);

	else
		retValue->InitNull();
	}

bool COrbEffectPainter::GetParamList (TArray<CString> *retList) const

//	GetParamList
//
//	Returns a list of value parameter names

	{
	retList->DeleteAll();
	retList->InsertEmpty(7);
	retList->GetAt(0) = ANIMATE_ATTRIB;
	retList->GetAt(1) = INTENSITY_ATTRIB;
	retList->GetAt(2) = LIFETIME_ATTRIB;
	retList->GetAt(3) = PRIMARY_COLOR_ATTRIB;
	retList->GetAt(4) = RADIUS_ATTRIB;
	retList->GetAt(5) = SECONDARY_COLOR_ATTRIB;
	retList->GetAt(6) = STYLE_ATTRIB;

	return true;
	}

void COrbEffectPainter::GetRect (RECT *retRect) const

//	GetRect
//
//	Returns the rect bounds of the image

	{
	int iSize = m_iRadius;

	retRect->left = -iSize;
	retRect->right = iSize;
	retRect->top = -iSize;
	retRect->bottom = iSize;
	}

void COrbEffectPainter::Invalidate (void)

//	Invalidate
//
//	Free up temporaries

	{
	if (m_pColorTable)
		{
		delete [] m_pColorTable;
		m_pColorTable = NULL;
		}

	if (m_pOpacityTable)
		{
		delete [] m_pOpacityTable;
		m_pOpacityTable = NULL;
		}

	m_bInitialized = false;
	}

void COrbEffectPainter::Paint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx)

//	Paint
//
//	Paint the effect

	{
	CalcSphericalTables();
	DrawFilledCircle(Dest, x, y, m_iRadius, m_pColorTable, m_pOpacityTable);
	}

void COrbEffectPainter::PaintComposite (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx)

//	PaintComposite
//
//	Paint the effect

	{
	CalcSphericalTables();
	CompositeFilledCircle(Dest, x, y, m_iRadius, m_pColorTable, m_pOpacityTable, true);
	}

bool COrbEffectPainter::PointInImage (int x, int y, int iTick, int iVariant, int iRotation) const

//	PointInImage
//
//	Returns TRUE if the given point is in the image

	{
	//	We only intersect if we are inside a box around the center of 1/2 the
	//	width (since a lot of the width is taken up by glow effects).

	int iSize = (m_iRadius / 2);
	return (Absolute(x) <= iSize && Absolute(y) <= iSize);
	}

void COrbEffectPainter::SetParam (CCreatePainterCtx &Ctx, const CString &sParam, const CEffectParamDesc &Value)

//	SetParam
//
//	Sets parameters

	{
	if (strEquals(sParam, ANIMATE_ATTRIB))
		m_iAnimation = (EAnimationTypes)Value.EvalIntegerBounded(Ctx, 0, -1, animateNone);

	else if (strEquals(sParam, INTENSITY_ATTRIB))
		m_iIntensity = Value.EvalIntegerBounded(Ctx, 0, 100, 50);

	else if (strEquals(sParam, LIFETIME_ATTRIB))
		m_iLifetime = Value.EvalIntegerBounded(Ctx, 0, -1, 0);

	else if (strEquals(sParam, PRIMARY_COLOR_ATTRIB))
		m_wPrimaryColor = Value.EvalColor(Ctx);

	else if (strEquals(sParam, RADIUS_ATTRIB))
		m_iRadius = Value.EvalIntegerBounded(Ctx, 1, -1, (int)(STD_SECONDS_PER_UPDATE * LIGHT_SECOND / KLICKS_PER_PIXEL));

	else if (strEquals(sParam, SECONDARY_COLOR_ATTRIB))
		m_wSecondaryColor = Value.EvalColor(Ctx);
	
	else if (strEquals(sParam, STYLE_ATTRIB))
		m_iStyle = (EOrbStyles)Value.EvalIdentifier(Ctx, STYLE_TABLE, styleMax, styleSmooth);
	}
