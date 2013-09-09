//	SFXFlare.cpp
//
//	Shockwave SFX

#include "PreComp.h"

#define LIFETIME_ATTRIB							CONSTLIT("lifetime")
#define PRIMARY_COLOR_ATTRIB					CONSTLIT("primaryColor")
#define RADIUS_ATTRIB							CONSTLIT("radius")
#define SECONDARY_COLOR_ATTRIB					CONSTLIT("secondaryColor")
#define STYLE_ATTRIB							CONSTLIT("style")

#define STYLE_FADING_BLAST						CONSTLIT("fadingBlast")
#define STYLE_FLICKER							CONSTLIT("flicker")
#define STYLE_PLAIN								CONSTLIT("plain")

const int MAIN_SPIKE_COUNT =					3;
const int MAIN_SPIKE_WIDTH_RATIO =				32;
const DWORD MAIN_SPIKE_OPACITY =				128;

class CFlarePainter : public IEffectPainter
	{
	public:
		CFlarePainter (CFlareEffectCreator *pCreator);

		//	IEffectPainter virtuals
		virtual CEffectCreator *GetCreator (void) { return m_pCreator; }
		virtual void GetRect (RECT *retRect) const;
		virtual void OnUpdate (SEffectUpdateCtx &Ctx) { m_iTick++; }
		virtual void Paint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx);

	protected:
		virtual void OnReadFromStream (SLoadCtx &Ctx);
		virtual void OnWriteToStream (IWriteStream *pStream);

	private:
		void PaintFlare (CG16bitImage &Dest, int x, int y, int iRadius, WORD wColor, bool bSpikes, SViewportPaintCtx &Ctx);

		CFlareEffectCreator *m_pCreator;
		int m_iTick;
	};

//	CFlareEffectCreator object

void CFlareEffectCreator::CreateFlareSpike (int iAngle, int iLength, int iWidth, SPoint *Poly)

//	CreateFlareSpike
//
//	Creates a thin diamond spike centered at the origin (i.e., spikes out of each side of the origin)

	{
	CVector vPos;

	//	Start with one tip

	vPos = PolarToVector(iAngle, iLength);
	Poly[0].x = (int)vPos.GetX();
	Poly[0].y = -(int)vPos.GetY();

	//	Clockwise to the right short vertext

	vPos = PolarToVector(iAngle + 270, iWidth);
	Poly[1].x = (int)vPos.GetX();
	Poly[1].y = -(int)vPos.GetY();

	//	Now the opposite tip

	vPos = PolarToVector(iAngle + 180, iLength);
	Poly[2].x = (int)vPos.GetX();
	Poly[2].y = -(int)vPos.GetY();

	//	And finally the other short vertex

	vPos = PolarToVector(iAngle + 90, iWidth);
	Poly[3].x = (int)vPos.GetX();
	Poly[3].y = -(int)vPos.GetY();
	}

IEffectPainter *CFlareEffectCreator::CreatePainter (CCreatePainterCtx &Ctx)

//	CreatePainter
//
//	Creates a new painter

	{
	return new CFlarePainter(this);
	}

ALERROR CFlareEffectCreator::OnEffectCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, const CString &sUNID)

//	OnEffectCreateFromXML
//
//	Initializes from XML

	{
	CString sStyle = pDesc->GetAttribute(STYLE_ATTRIB);
	if (strEquals(sStyle, STYLE_FADING_BLAST))
		m_iStyle = styleFadingBlast;
	else if (strEquals(sStyle, STYLE_FLICKER))
		m_iStyle = styleFlicker;
	else if (sStyle.IsBlank() || strEquals(sStyle, STYLE_PLAIN))
		m_iStyle = stylePlain;
	else
		{
		Ctx.sError = strPatternSubst(CONSTLIT("Invalid Flare style: %s"), sStyle);
		return ERR_FAIL;
		}

	m_iRadius = pDesc->GetAttributeIntegerBounded(RADIUS_ATTRIB, 0, -1, 100);
	m_iLifetime = pDesc->GetAttributeIntegerBounded(LIFETIME_ATTRIB, 0, -1, 1);
	m_wPrimaryColor = ::LoadRGBColor(pDesc->GetAttribute(PRIMARY_COLOR_ATTRIB));

	CString sAttrib;
	if (pDesc->FindAttribute(SECONDARY_COLOR_ATTRIB, &sAttrib))
		m_wSecondaryColor = ::LoadRGBColor(sAttrib);
	else
		m_wSecondaryColor = m_wPrimaryColor;

	return NOERROR;
	}

//	CFlarePainter object

CFlarePainter::CFlarePainter (CFlareEffectCreator *pCreator) : m_pCreator(pCreator),
		m_iTick(0)

//	CFlarePainter constructor

	{
	}

void CFlarePainter::GetRect (RECT *retRect) const

//	GetRect
//
//	Returns the RECT of the effect centered on 0,0

	{
	int iRadius = m_pCreator->GetRadius();

	retRect->left = -iRadius;
	retRect->top = -iRadius;
	retRect->right = iRadius + 1;
	retRect->bottom = iRadius + 1;
	}

void CFlarePainter::OnReadFromStream (SLoadCtx &Ctx)

//	OnReadFromStream
//
//	Load from stream

	{
	Ctx.pStream->Read((char *)&m_iTick, sizeof(DWORD));
	}

void CFlarePainter::OnWriteToStream (IWriteStream *pStream)

//	OnWriteToStream
//
//	Write to stream

	{
	pStream->Write((char *)&m_iTick, sizeof(DWORD));
	}

void CFlarePainter::Paint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx)

//	Paint
//
//	Paint

	{
	int iTotalLifetime = m_pCreator->GetLifetime();

	switch (m_pCreator->GetStyle())
		{
		case CFlareEffectCreator::styleFlicker:
			if ((m_iTick % 2) == 0)
				PaintFlare(Dest, x, y, m_pCreator->GetRadius(), m_pCreator->GetPrimaryColor(), true, Ctx);
			break;

		case CFlareEffectCreator::stylePlain:
			PaintFlare(Dest, x, y, m_pCreator->GetRadius(), m_pCreator->GetPrimaryColor(), false, Ctx);
			break;

		case CFlareEffectCreator::styleFadingBlast:
			{
			//	Safety checks

			if (iTotalLifetime <= 0)
				return;

			//	Radius shrinks proportionally each tick

			int iRadius;
			if (m_iTick < m_pCreator->GetLifetime())
				iRadius = (int)(m_pCreator->GetRadius() * ((Metric)(m_pCreator->GetLifetime() - m_iTick) / (Metric)iTotalLifetime));
			else
				iRadius = 0;

			//	Color moves from primary to secondary

			WORD wColor;
			if (m_pCreator->GetPrimaryColor() != m_pCreator->GetSecondaryColor())
				wColor = CG16bitImage::FadeColor(m_pCreator->GetPrimaryColor(), m_pCreator->GetSecondaryColor(), 100 * m_iTick / iTotalLifetime);
			else
				wColor = m_pCreator->GetPrimaryColor();

			//	Paint

			PaintFlare(Dest, x, y, iRadius, wColor, true, Ctx);

			break;
			}
		}
	}

void CFlarePainter::PaintFlare (CG16bitImage &Dest, int x, int y, int iRadius, WORD wColor, bool bSpikes, SViewportPaintCtx &Ctx)

//	PaintFlare
//
//	Paints a flare

	{
	int i;

	if (iRadius > 0)
		{
		//	Paint each of the spikes first

		if (bSpikes)
			{
			int iAngle = 360 / MAIN_SPIKE_COUNT;
			for (i = 0; i < MAIN_SPIKE_COUNT; i++)
				{
				SPoint Spike[4];
				m_pCreator->CreateFlareSpike(i * iAngle, iRadius, iRadius / MAIN_SPIKE_WIDTH_RATIO, Spike);

				CG16bitBinaryRegion Region;
				Region.CreateFromSimplePolygon(4, Spike);
				Region.FillTrans(Dest, x, y, wColor, MAIN_SPIKE_OPACITY);
				}
			}

		//	Paint the extended glow

		DrawAlphaGradientCircle(Dest,
				x,
				y,
				iRadius,
				wColor);
		}
	}
