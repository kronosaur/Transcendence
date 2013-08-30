//	SFXMoltenBolt.cpp
//
//	Molten Bolt SFX

#include "PreComp.h"


#define WIDTH_ATTRIB							(CONSTLIT("width"))
#define LENGTH_ATTRIB							(CONSTLIT("length"))
#define GROWTH_ATTRIB							(CONSTLIT("growth"))
#define PRIMARY_COLOR_ATTRIB					(CONSTLIT("primaryColor"))
#define SECONDARY_COLOR_ATTRIB					(CONSTLIT("secondaryColor"))

static SPoint g_Shape[12] = 
	{
		{    0,    0	},
		{    -5,   10	},
		{   -10,   15	},
		{   -20,   17	},
		{   -30,   16	},
		{   -40,   15	},
		{  -100,    0	},
		{   -40,   -15	},
		{   -30,   -16	},
		{   -20,   -17	},
		{   -10,   -15	},
		{    -5,   -10	},
	};

#define SHAPE_COUNT								(sizeof(g_Shape) / sizeof(g_Shape[0]))

void CMoltenBoltEffectCreator::CreateBoltShape (int iAngle, int iLength, int iWidth, SPoint *Poly)

//	CreateBoltShape
//
//	Creates a bolt-shaped polygon

	{
	//	Define a transformation for this shape

	CXForm Trans(xformScale, ((Metric)iLength)/100.0, ((Metric)iWidth)/34.0);
	Trans = Trans * CXForm(xformRotate, iAngle);

	//	Generate the points

	for (int i = 0; i < SHAPE_COUNT; i++)
		{
		Metric x, y;
		Trans.Transform(g_Shape[i].x, g_Shape[i].y, &x, &y);
		Poly[i].x = (int)(x + 0.5);
		Poly[i].y = -(int)(y + 0.5);
		}
	}

void CMoltenBoltEffectCreator::GetRect (RECT *retRect) const

//	GetRect
//
//	Returns the RECT for the effect

	{
	retRect->left = -256;
	retRect->top = -256;
	retRect->right = 256;
	retRect->bottom = 256;
	}

ALERROR CMoltenBoltEffectCreator::OnEffectCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, const CString &sUNID)

//	OnEffectCreateFromXML
//
//	Load from XML

	{
	m_iWidth = pDesc->GetAttributeInteger(WIDTH_ATTRIB);
	m_iLength = pDesc->GetAttributeInteger(LENGTH_ATTRIB);
	m_iGrowth = pDesc->GetAttributeInteger(GROWTH_ATTRIB);
	m_wPrimaryColor = ::LoadRGBColor(pDesc->GetAttribute(PRIMARY_COLOR_ATTRIB));
	m_wSecondaryColor = ::LoadRGBColor(pDesc->GetAttribute(SECONDARY_COLOR_ATTRIB));

	if (m_iLength == 0)
		m_iLength = m_iGrowth;

	return NOERROR;
	}

void CMoltenBoltEffectCreator::Paint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx)

//	Paint
//
//	Paint effect

	{
	//	Compute the length of the bolt based on the time

	int iLength = m_iLength + (m_iGrowth * (Min(8, Ctx.iTick)));

	//	Create the outer shape

	SPoint Poly[SHAPE_COUNT];
	CreateBoltShape(Ctx.iRotation, iLength, m_iWidth, Poly);

	//	Paint

	CG16bitBinaryRegion Region;
	WORD wColor = CG16bitImage::BlendPixel(Ctx.wSpaceColor, m_wSecondaryColor, 200);
	Region.CreateFromConvexPolygon(SHAPE_COUNT, Poly);
	Region.Fill(Dest, x, y, wColor);

	//	Create the inner shape

	CreateBoltShape(Ctx.iRotation, Max(16, iLength * 7 / 8), Max(2, m_iWidth * 7 / 8), Poly);

	//	Paint

	Region.CreateFromConvexPolygon(SHAPE_COUNT, Poly);
	Region.Fill(Dest, x, y, m_wPrimaryColor);
	}

bool CMoltenBoltEffectCreator::PointInImage (int x, int y, int iTick, int iVariant, int iRotation) const

//	PointInImage
//
//	Return TRUE if the given point is in the effect

	{
	RECT rcRect;
	GetRect(&rcRect);
	return (x >= -(m_iWidth / 2) && x < (m_iWidth / 2) && y >= -(m_iWidth / 2) && y < (m_iWidth / 2));
	}
