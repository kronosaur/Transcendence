//	SFXImage.cpp
//
//	Paints a simple image

#include "PreComp.h"

#define RANDOM_START_FRAME_ATTRIB				CONSTLIT("randomStartFrame")
#define ROTATE_IMAGE_ATTRIB						CONSTLIT("rotateImage")
#define VARIANTS_ATTRIB							CONSTLIT("imageVariants")
#define ROTATION_COUNT_ATTRIB					CONSTLIT("rotationCount")

class CImagePainter : public IEffectPainter
	{
	public:
		CImagePainter (CImageEffectCreator *pCreator);

		//	IEffectPainter virtuals
		virtual CEffectCreator *GetCreator (void) { return m_pCreator; }
		virtual bool GetParticlePaintDesc (SParticlePaintDesc *retDesc);
		virtual void GetRect (RECT *retRect) const;
		virtual int GetVariants (void) const;
		virtual void Paint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx);
		virtual bool PointInImage (int x, int y, int iTick, int iVariant = 0, int iRotation = 0) const;

	protected:
		virtual void OnReadFromStream (SLoadCtx &Ctx);
		virtual void OnWriteToStream (IWriteStream *pStream);

	private:
		CImageEffectCreator *m_pCreator;
		CCompositeImageSelector m_Sel;
	};

IEffectPainter *CImageEffectCreator::CreatePainter (CCreatePainterCtx &Ctx)

//	CreatePainter
//
//	Returns a painter
	
	{
	if (m_Image.IsConstant())
		return this;
	else
		return new CImagePainter(this);
	}

bool CImageEffectCreator::GetParticlePaintDesc (SParticlePaintDesc *retDesc)

//	GetParticlePaintDesc
//
//	Returns particle painting descriptor for optimized painting

	{
	retDesc->iStyle = paintImage;
	retDesc->pImage = &m_Image.GetImage(CCompositeImageSelector());
	retDesc->iVariants = m_iVariants;
	retDesc->bDirectional = m_bDirectional;
	retDesc->bRandomStartFrame = m_bRandomStartFrame;

	return true;
	}

void CImageEffectCreator::GetRect (RECT *retRect) const

//	GetRect
//
//	Returns the image rect
	
	{
	CObjectImageArray &Image = m_Image.GetImage(CCompositeImageSelector());
	*retRect = Image.GetImageRect();
	}

ALERROR CImageEffectCreator::OnEffectCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, const CString &sUNID)

//	OnEffectCreateFromXML
//
//	Initialize type-specific data

	{
	ALERROR error;

	if (error = m_Image.InitFromXML(Ctx, pDesc))
		return error;

	m_iLifetime = m_Image.GetMaxLifetime();
	if (m_iLifetime <= 0)
		m_iLifetime = 1;

	m_bRandomStartFrame = pDesc->GetAttributeBool(RANDOM_START_FRAME_ATTRIB);

	//	Variants & Rotation

	m_bRotateImage = pDesc->GetAttributeBool(ROTATE_IMAGE_ATTRIB);
	if (m_bRotateImage)
		{
		m_iVariants = 1;
		m_bDirectional = false;
		}
	else
		{
		m_iVariants = pDesc->GetAttributeInteger(ROTATION_COUNT_ATTRIB);
		m_bDirectional = (m_iVariants > 1);

		if (m_iVariants == 0)
			m_iVariants = pDesc->GetAttributeInteger(VARIANTS_ATTRIB);

		if (m_iVariants <= 0)
			m_iVariants = 1;
		}

	return NOERROR;
	}

ALERROR CImageEffectCreator::OnEffectBindDesign (SDesignLoadCtx &Ctx)

//	OnEffectBindDesign
//
//	Resolve loading

	{
	ALERROR error;

	if (error = m_Image.OnDesignLoadComplete(Ctx))
		return error;

	return NOERROR;
	}

void CImageEffectCreator::Paint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx)

//	Paint
//
//	Paint the effect

	{
	CObjectImageArray &Image = m_Image.GetImage(CCompositeImageSelector());

	int iTick = Ctx.iTick;
	if (m_bRandomStartFrame)
		iTick += Ctx.iDestiny;

	if (m_bRotateImage)
		Image.PaintRotatedImage(Dest, x, y, iTick, Ctx.iRotation);
	else
		{
		int iFrame = (m_bDirectional ? Angle2Direction(Ctx.iRotation, m_iVariants) : (Ctx.iVariant % m_iVariants));

		Image.PaintImage(Dest, x, y, iTick, iFrame);
		}
	}

void CImageEffectCreator::PaintComposite (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx)

//	Paint
//
//	Paint the effect

	{
	CObjectImageArray &Image = m_Image.GetImage(CCompositeImageSelector());

	int iTick = Ctx.iTick;
	if (m_bRandomStartFrame)
		iTick += Ctx.iDestiny;

	if (m_bRotateImage)
		Image.PaintRotatedImage(Dest, x, y, iTick, Ctx.iRotation, true);
	else
		{
		int iFrame = (m_bDirectional ? Angle2Direction(Ctx.iRotation, m_iVariants) : (Ctx.iVariant % m_iVariants));

		Image.PaintImage(Dest, x, y, iTick, iFrame, true);
		}
	}

bool CImageEffectCreator::PointInImage (int x, int y, int iTick, int iVariant, int iRotation) const

//	PointInImage
//
//	Returns TRUE if the given point is in the image

	{
	CObjectImageArray &Image = m_Image.GetImage(CCompositeImageSelector());

	return Image.PointInImage(x, y, iTick, (iVariant % m_iVariants));
	}

void CImageEffectCreator::SetVariants (int iVariants)

//	SetVariants
//
//	Sets the number of variants
//	This function is only called because directionally used to be set at the
//	object level. We only honor this if we are not already directional

	{
	if (!m_bDirectional)
		{
		m_bDirectional = true;
		m_iVariants = iVariants;
		}
	}

//	CImagePainter --------------------------------------------------------------

CImagePainter::CImagePainter (CImageEffectCreator *pCreator) : m_pCreator(pCreator)

//	CImagePainter constructor
	
	{
	SSelectorInitCtx InitCtx;

	m_pCreator->GetImage().InitSelector(InitCtx, &m_Sel);
	}

bool CImagePainter::GetParticlePaintDesc (SParticlePaintDesc *retDesc)

//	GetParticlePaintDesc
//
//	Returns the particle paint descriptor

	{
	CObjectImageArray &Image = m_pCreator->GetImage().GetImage(m_Sel);

	retDesc->iStyle = paintImage;
	retDesc->pImage = &Image;
	retDesc->iVariants = m_pCreator->GetVariants();
	retDesc->bDirectional = m_pCreator->IsDirectional();
	retDesc->bRandomStartFrame = m_pCreator->HasRandomStartFrame();

	return true;
	}

void CImagePainter::GetRect (RECT *retRect) const

//	GetRect
//
//	Returns the rect of the image

	{
	CObjectImageArray &Image = m_pCreator->GetImage().GetImage(m_Sel);
	*retRect = Image.GetImageRect();
	}

int CImagePainter::GetVariants (void) const

//	GetVariants
//
//	Returns the number of variants

	{
	return m_pCreator->GetVariants();
	}

void CImagePainter::OnReadFromStream (SLoadCtx &Ctx)

//	OnReadFromStream
//
//	Load from stream

	{
	//	The constructor initializes this to a default value; we need to clear
	//	it out because ReadFromStream ASSERTS that it is empty.

	m_Sel.DeleteAll();

	m_Sel.ReadFromStream(Ctx);
	}

void CImagePainter::OnWriteToStream (IWriteStream *pStream)

//	OnWriteToStream
//
//	Write to stream

	{
	m_Sel.WriteToStream(pStream);
	}

void CImagePainter::Paint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx)

//	Paint
//
//	Paint image

	{
	CObjectImageArray &Image = m_pCreator->GetImage().GetImage(m_Sel);

	int iTick = Ctx.iTick;
	if (m_pCreator->HasRandomStartFrame())
		iTick += Ctx.iDestiny;

	if (m_pCreator->ImageRotationNeeded())
		Image.PaintRotatedImage(Dest, x, y, iTick, Ctx.iRotation);
	else
		{
		int iVariants = m_pCreator->GetVariants();
		int iFrame = (m_pCreator->IsDirectional() ? Angle2Direction(Ctx.iRotation, iVariants) : (Ctx.iVariant % iVariants));

		Image.PaintImage(Dest, x, y, iTick, iFrame);
		}
	}

bool CImagePainter::PointInImage (int x, int y, int iTick, int iVariant, int iRotation) const

//	PointInImage
//
//	Returns TRUE if point is in the image

	{
	CObjectImageArray &Image = m_pCreator->GetImage().GetImage(m_Sel);
	int iVariants = m_pCreator->GetVariants();
	return Image.PointInImage(x, y, iTick, (iVariant % iVariants));
	}

