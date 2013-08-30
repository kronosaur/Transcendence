//	SFXShape.cpp
//
//	Shape SFX

#include "PreComp.h"

#define POINT_TAG								(CONSTLIT("Point"))

#define SCALE_WIDTH_ATTRIB						(CONSTLIT("scaleWidth"))
#define SCALE_LENGTH_ATTRIB						(CONSTLIT("scaleLength"))
#define COLOR_ATTRIB							(CONSTLIT("color"))
#define OPACITY_ATTRIB							(CONSTLIT("opacity"))
#define DIRECTIONAL_ATTRIB						(CONSTLIT("directional"))
#define X_ATTRIB								(CONSTLIT("x"))
#define Y_ATTRIB								(CONSTLIT("y"))
#define SCALE_WIDTH_INC_ATTRIB					(CONSTLIT("scaleWidthInc"))
#define SCALE_LENGTH_INC_ATTRIB					(CONSTLIT("scaleLengthInc"))

class CShapeEffectPainter : public IEffectPainter
	{
	public:
		CShapeEffectPainter (CShapeEffectCreator *pCreator);

		//	IEffectPainter virtuals
		virtual CEffectCreator *GetCreator (void) { return m_pCreator; }
		virtual void GetRect (RECT *retRect) const;
		virtual void Paint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx);
		virtual bool PointInImage (int x, int y, int iTick, int iVariant = 0, int iRotation = 0) const;

	protected:
		virtual void OnReadFromStream (SLoadCtx &Ctx);
		virtual void OnWriteToStream (IWriteStream *pStream);

	private:
		void CacheShapeRegion (int iAngle, int iLength, int iWidth);

		CShapeEffectCreator *m_pCreator;

		int m_iCachedWidth;
		int m_iCachedLength;
		int m_iCachedRotation;
		CG16bitBinaryRegion m_CachedRegion;
	};

//	CShapeEffectCreator

CShapeEffectCreator::~CShapeEffectCreator (void)

//	CShapeEffectCreator destructor

	{
	if (m_Points)
		delete [] m_Points;

	if (m_TransBuffer)
		delete [] m_TransBuffer;
	}

IEffectPainter *CShapeEffectCreator::CreatePainter (CCreatePainterCtx &Ctx)

//	CreatePainter
//
//	Creates a new painter

	{
	return new CShapeEffectPainter(this);
	}

ALERROR CShapeEffectCreator::OnEffectCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, const CString &sUNID)

//	OnEffectCreateFromXML
//
//	Load from XML

	{
	int i;
	CString sAttrib;

	m_iWidth = pDesc->GetAttributeInteger(SCALE_WIDTH_ATTRIB);
	m_iLength = pDesc->GetAttributeInteger(SCALE_LENGTH_ATTRIB);
	m_bDirectional = pDesc->GetAttributeBool(DIRECTIONAL_ATTRIB);
	m_iWidthInc = pDesc->GetAttributeInteger(SCALE_WIDTH_INC_ATTRIB);
	m_iLengthInc = pDesc->GetAttributeInteger(SCALE_LENGTH_INC_ATTRIB);
	m_wColor = ::LoadRGBColor(pDesc->GetAttribute(COLOR_ATTRIB));

	if (pDesc->FindAttribute(OPACITY_ATTRIB, &sAttrib))
		m_byOpacity = strToInt(sAttrib, 255);
	else
		m_byOpacity = 255;

	//	Initialize the points structure

	m_iPointCount = pDesc->GetContentElementCount();
	if (m_iPointCount > 0)
		{
		m_Points = new SPoint [m_iPointCount];
		m_TransBuffer = new SPoint [m_iPointCount];

		for (i = 0; i < m_iPointCount; i++)
			{
			CXMLElement *pPointDesc = pDesc->GetContentElement(i);
			if (!strEquals(pPointDesc->GetTag(), POINT_TAG))
				{
				Ctx.sError = CONSTLIT("<Point> element expected");
				return ERR_FAIL;
				}

			m_Points[i].x = pPointDesc->GetAttributeInteger(X_ATTRIB);
			m_Points[i].y = pPointDesc->GetAttributeInteger(Y_ATTRIB);
			}

		//	Convex polygons (which remain convex no matter the rotation) can be
		//	computed faster with an optimized algorithm, so we keep track here.

		m_bConvexPolygon = IsConvexPolygon(m_iPointCount, m_Points);
		}
	else
		{
		m_Points = NULL;
		m_TransBuffer = NULL;
		m_bConvexPolygon = true;
		}

	return NOERROR;
	}

void CShapeEffectCreator::CreateShapeRegion (int iAngle, int iLength, int iWidth, CG16bitBinaryRegion *pRegion)

//	CreateShapeRegion
//
//	Creates a transformed polygon from m_Points and the given parameters

	{
	//	Define a transformation for this shape

	CXForm Trans(xformScale, ((Metric)iLength)/100.0, ((Metric)iWidth)/100.0);
	Trans = Trans * CXForm(xformRotate, iAngle);

	//	Transform the points

	for (int i = 0; i < m_iPointCount; i++)
		{
		Metric x, y;
		Trans.Transform(m_Points[i].x, m_Points[i].y, &x, &y);
		m_TransBuffer[i].x = (int)(x + 0.5);
		m_TransBuffer[i].y = -(int)(y + 0.5);
		}

	//	Create the region

	if (m_bConvexPolygon)
		pRegion->CreateFromConvexPolygon(m_iPointCount, m_TransBuffer);
	else
		pRegion->CreateFromPolygon(m_iPointCount, m_TransBuffer);
	}

//	CShapeEffectPainter

CShapeEffectPainter::CShapeEffectPainter (CShapeEffectCreator *pCreator) :
		m_pCreator(pCreator),
		m_iCachedLength(-1),
		m_iCachedWidth(-1),
		m_iCachedRotation(-1)

//	CShapeEffectPainter constructor

	{
	}

void CShapeEffectPainter::CacheShapeRegion (int iAngle, int iLength, int iWidth)

//	CacheShapeRegion
//
//	Ensures that the cache has the proper region

	{
	if (iAngle != m_iCachedRotation
			|| iLength != m_iCachedLength
			|| iWidth != m_iCachedWidth)
		{
		m_pCreator->CreateShapeRegion(iAngle, iLength, iWidth, &m_CachedRegion);

		m_iCachedRotation = iAngle;
		m_iCachedLength = iLength;
		m_iCachedWidth = iWidth;
		}
	}

void CShapeEffectPainter::GetRect (RECT *retRect) const

//	GetRect
//
//	Returns the RECT for the effect

	{
	retRect->left = -256;
	retRect->top = -256;
	retRect->right = 256;
	retRect->bottom = 256;
	}

void CShapeEffectPainter::OnReadFromStream (SLoadCtx &Ctx)

//	OnReadFromStream
//
//	Load from stream

	{
	m_iCachedRotation = -1;
	m_iCachedLength = -1;
	m_iCachedWidth = -1;
	}

void CShapeEffectPainter::OnWriteToStream (IWriteStream *pStream)

//	OnWriteToStream
//
//	Write to stream

	{
	}

void CShapeEffectPainter::Paint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx)

//	Paint
//
//	Paint effect

	{
	//	Compute the desired rotation, length, and width

	int iLength = m_pCreator->GetLength() + Ctx.iTick * m_pCreator->GetLengthInc();
	int iWidth = m_pCreator->GetWidth() + Ctx.iTick * m_pCreator->GetWidthInc();
	int iRotation = (m_pCreator->IsDirectional() ? Ctx.iRotation : 0);

	//	Generate the shape

	CacheShapeRegion(iRotation, iLength, iWidth);

	//	Paint

	DWORD byOpacity = m_pCreator->GetOpacity();
	WORD wColor = m_pCreator->GetColor();

	if (byOpacity == 255)
		m_CachedRegion.Fill(Dest, x, y, wColor);
	else
		m_CachedRegion.FillTrans(Dest, x, y, wColor, byOpacity);
	}

bool CShapeEffectPainter::PointInImage (int x, int y, int iTick, int iVariant, int iRotation) const

//	PointInImage
//
//	Return TRUE if the given point is in the effect

	{
	RECT rcRect;
	GetRect(&rcRect);

	int iWidth = m_pCreator->GetWidth();

	return (x >= -(iWidth / 2) && x < (iWidth / 2) && y >= -(iWidth / 2) && y < (iWidth / 2));
	}

