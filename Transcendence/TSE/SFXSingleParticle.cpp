//	SFXSingleParticle.cpp
//
//	Single particle SFX

#include "PreComp.h"

#define STYLE_ATTRIB							CONSTLIT("style")
#define MIN_WIDTH_ATTRIB						CONSTLIT("minWidth")
#define MAX_WIDTH_ATTRIB						CONSTLIT("maxWidth")
#define PRIMARY_COLOR_ATTRIB					(CONSTLIT("primaryColor"))
#define SECONDARY_COLOR_ATTRIB					(CONSTLIT("secondaryColor"))

#define STYLE_PLAIN								CONSTLIT("plain")
#define STYLE_FLAME								CONSTLIT("flame")
#define STYLE_SMOKE								CONSTLIT("smoke")
#define STYLE_LINE								CONSTLIT("line")

const int DEFAULT_MIN_WIDTH =					1;
const int DEFAULT_MAX_WIDTH =					8;

class CSingleParticlePainter : public IEffectPainter
	{
	public:
		CSingleParticlePainter (CSingleParticleEffectCreator *pCreator);

		//	IEffectPainter virtuals
		virtual CEffectCreator *GetCreator (void) { return m_pCreator; }
		virtual bool GetParticlePaintDesc (SParticlePaintDesc *retDesc);
		virtual void Paint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx) { };

	protected:
		virtual void OnReadFromStream (SLoadCtx &Ctx);
		virtual void OnWriteToStream (IWriteStream *pStream);

	private:
		CSingleParticleEffectCreator *m_pCreator;
		int m_iMaxWidth;
		int m_iMinWidth;
	};

//	CSingleParticleEffectCreator object

IEffectPainter *CSingleParticleEffectCreator::CreatePainter (CCreatePainterCtx &Ctx)

//	CreatePainter
//
//	Creates a new painter

	{
	return new CSingleParticlePainter(this);
	}

ALERROR CSingleParticleEffectCreator::OnEffectCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, const CString &sUNID)

//	OnEffectCreateFromXML
//
//	Initializes from XML

	{
	ALERROR error;

	CString sStyle = pDesc->GetAttribute(STYLE_ATTRIB);
	if (sStyle.IsBlank() || strEquals(sStyle, STYLE_PLAIN))
		m_iStyle = paintPlain;
	else if (strEquals(sStyle, STYLE_FLAME))
		m_iStyle = paintFlame;
	else if (strEquals(sStyle, STYLE_SMOKE))
		m_iStyle = paintSmoke;
	else if (strEquals(sStyle, STYLE_LINE))
		m_iStyle = paintLine;
	else
		{
		Ctx.sError = CONSTLIT("Invalid SingleParticle style");
		return ERR_FAIL;
		}

	CString sRange;
	if (pDesc->FindAttribute(MAX_WIDTH_ATTRIB, &sRange))
		{
		if (error = m_MaxWidth.LoadFromXML(sRange))
			return error;
		}
	else
		m_MaxWidth = DiceRange(0, 0, DEFAULT_MAX_WIDTH);

	if (pDesc->FindAttribute(MIN_WIDTH_ATTRIB, &sRange))
		{
		if (error = m_MinWidth.LoadFromXML(sRange))
			return error;
		}
	else
		m_MinWidth = DiceRange(0, 0, DEFAULT_MIN_WIDTH);

	//	Colors

	m_wPrimaryColor = ::LoadRGBColor(pDesc->GetAttribute(PRIMARY_COLOR_ATTRIB));
	m_wSecondaryColor = ::LoadRGBColor(pDesc->GetAttribute(SECONDARY_COLOR_ATTRIB));

	return NOERROR;
	}

//	CSingleParticlePainter object

CSingleParticlePainter::CSingleParticlePainter (CSingleParticleEffectCreator *pCreator) : 
		m_pCreator(pCreator)

//	CSingleParticlePainter constructor

	{
	m_iMaxWidth = m_pCreator->GetMaxWidth();
	m_iMinWidth = m_pCreator->GetMinWidth();
	}

bool CSingleParticlePainter::GetParticlePaintDesc (SParticlePaintDesc *retDesc)

//	GetParticlePaintDesc
//
//	Returns a structure describing how particles should be painted

	{
	retDesc->iStyle = m_pCreator->GetStyle();
	retDesc->iMinWidth = m_iMinWidth;
	retDesc->iMaxWidth = m_iMaxWidth;
	retDesc->wPrimaryColor = m_pCreator->GetPrimaryColor();
	retDesc->wSecondaryColor = m_pCreator->GetSecondaryColor();

	return true;
	}

void CSingleParticlePainter::OnReadFromStream (SLoadCtx &Ctx)

//	OnReadFromStream
//
//	Load from stream

	{
	Ctx.pStream->Read((char *)&m_iMinWidth, sizeof(DWORD));
	Ctx.pStream->Read((char *)&m_iMaxWidth, sizeof(DWORD));
	}

void CSingleParticlePainter::OnWriteToStream (IWriteStream *pStream)

//	OnWriteToStream
//
//	Write to stream

	{
	pStream->Write((char *)&m_iMinWidth, sizeof(DWORD));
	pStream->Write((char *)&m_iMaxWidth, sizeof(DWORD));
	}

