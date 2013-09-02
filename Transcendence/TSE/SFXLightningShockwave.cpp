//	SFXLightningShockwave.cpp
//
//	Lightning Shockwave SFX

#include "PreComp.h"


#if 0

#define IMAGE_TAG								CONSTLIT("Image")

#define SPEED_ATTRIB							CONSTLIT("speed")
#define LIFETIME_ATTRIB							CONSTLIT("lifetime")

class CLightningShockwavePainter : public IEffectPainter
	{
	public:
		CLightningShockwavePainter (CLightningShockwaveEffectCreator *pCreator);

		//	IEffectPainter virtuals
		virtual CEffectCreator *GetCreator (void) { return m_pCreator; }
		virtual void GetRect (RECT *retRect) const;
		virtual void OnUpdate (SEffectUpdateCtx &Ctx) { m_iRadius += m_pCreator->GetSpeed(); }
		virtual void Paint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx);

	protected:
		virtual void OnReadFromStream (SLoadCtx &Ctx);
		virtual void OnWriteToStream (IWriteStream *pStream);

	private:
		CLightningShockwaveEffectCreator *m_pCreator;
		int m_iRadius;
	};

//	CLightningShockwaveEffectCreator object

IEffectPainter *CLightningShockwaveEffectCreator::CreatePainter (CCreatePainterCtx &Ctx)

//	CreatePainter
//
//	Creates a new painter

	{
	return new CLightningShockwavePainter(this);
	}

ALERROR CLightningShockwaveEffectCreator::OnEffectCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, const CString &sUNID)

//	OnEffectCreateFromXML
//
//	Initializes from XML

	{
	ALERROR error;

	CXMLElement *pImage = pDesc->GetContentElementByTag(IMAGE_TAG);
	if (pImage)
		if (error = m_Image.InitFromXML(Ctx, pImage))
			return error;

	m_iSpeed = pDesc->GetAttributeInteger(SPEED_ATTRIB);
	m_iLifetime = pDesc->GetAttributeInteger(LIFETIME_ATTRIB);

	return NOERROR;
	}

//	CLightningShockwavePainter object

CLightningShockwavePainter::CLightningShockwavePainter (CLightningShockwaveEffectCreator *pCreator) : m_pCreator(pCreator),
		m_iRadius(1)

//	CLightningShockwavePainter constructor

	{
	}

void CLightningShockwavePainter::GetRect (RECT *retRect) const

//	GetRect
//
//	Returns the RECT of the effect centered on 0,0

	{
	retRect->left = -m_iRadius;
	retRect->top = -m_iRadius;
	retRect->right = m_iRadius + 1;
	retRect->bottom = m_iRadius + 1;
	}

void CLightningShockwavePainter::OnReadFromStream (SLoadCtx &Ctx)

//	OnReadFromStream
//
//	Load from stream

	{
	Ctx.pStream->Read((char *)&m_iRadius, sizeof(DWORD));
	}

void CLightningShockwavePainter::OnWriteToStream (IWriteStream *pStream)

//	OnWriteToStream
//
//	Write to stream

	{
	pStream->Write((char *)&m_iRadius, sizeof(DWORD));
	}

void CLightningShockwavePainter::Paint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx)

//	Paint
//
//	Paint

	{
	CG16bitImage &Image = m_pCreator->GetImage().GetImage();
	RECT rcImage = m_pCreator->GetImage().GetImageRect();

	DrawBltCircle(Dest, 
			x, 
			y, 
			m_iRadius, 
			Image, 
			rcImage.left, 
			rcImage.top, 
			RectWidth(rcImage), 
			RectHeight(rcImage),
			255);
	}

#endif