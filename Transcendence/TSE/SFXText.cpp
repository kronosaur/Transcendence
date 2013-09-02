//	SFXText.cpp
//
//	Polyflash SFX

#include "PreComp.h"

#define ALIGN_ATTRIB							CONSTLIT("align")
#define FONT_ATTRIB								CONSTLIT("font")
#define OPACITY_ATTRIB							CONSTLIT("opacity")
#define PRIMARY_COLOR_ATTRIB					CONSTLIT("primaryColor")
#define TEXT_ATTRIB								CONSTLIT("text")

#define ALIGN_CENTER							CONSTLIT("center")
#define ALIGN_RIGHT								CONSTLIT("right")
#define ALIGN_LEFT								CONSTLIT("left")

class CTextPainter : public IEffectPainter
	{
	public:
		CTextPainter (CTextEffectCreator *pCreator);

		void SetText (const CString &sText);

		//	IEffectPainter virtuals
		virtual CEffectCreator *GetCreator (void) { return m_pCreator; }
		virtual void GetRect (RECT *retRect) const;
		virtual void Paint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx);
		virtual bool PointInImage (int x, int y, int iTick, int iVariant = 0, int iRotation = 0) const;
		virtual bool SetParamString (const CString &sParam, const CString &sValue);
		virtual bool SetProperty (const CString &sProperty, ICCItem *pValue);

	protected:
		virtual void OnReadFromStream (SLoadCtx &Ctx);
		virtual void OnWriteToStream (IWriteStream *pStream);

	private:
		void MeasureText (void);

		CTextEffectCreator *m_pCreator;
		CString m_sText;

		//	Cached
		int m_xText;
		int m_yText;
		int m_cxText;
		int m_cyText;
	};

//	CTextEffectCreator object

IEffectPainter *CTextEffectCreator::CreatePainter (CCreatePainterCtx &Ctx)

//	CreatePainter
//
//	Creates a new painter

	{
	CTextPainter *pPainter = new CTextPainter(this);
	pPainter->SetText(m_sDefaultText);
	return pPainter;
	}

ALERROR CTextEffectCreator::OnEffectCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, const CString &sUNID)

//	OnEffectCreateFromXML
//
//	Loads the effect from XML

	{
	m_sDefaultText = strCEscapeCodes(pDesc->GetAttribute(TEXT_ATTRIB));

	m_pFont = g_pUniverse->GetFont(pDesc->GetAttribute(FONT_ATTRIB));
	if (m_pFont == NULL)
		{
		Ctx.sError = strPatternSubst(CONSTLIT("Unknown font: %s"), pDesc->GetAttribute(FONT_ATTRIB));
		return ERR_FAIL;
		}

	m_wPrimaryColor = ::LoadRGBColor(pDesc->GetAttribute(PRIMARY_COLOR_ATTRIB));
	m_byOpacity = pDesc->GetAttributeIntegerBounded(OPACITY_ATTRIB, 0, 255, 255);

	//	Alignment

	CString sAlign;
	if (pDesc->FindAttribute(ALIGN_ATTRIB, &sAlign))
		{
		if (strEquals(sAlign, ALIGN_LEFT))
			m_dwAlignment = 0;
		else if (strEquals(sAlign, ALIGN_RIGHT))
			m_dwAlignment = CG16bitFont::AlignRight;
		else
			m_dwAlignment = CG16bitFont::AlignCenter;
		}
	else
		m_dwAlignment = CG16bitFont::AlignCenter;

	return NOERROR;
	}

//	CTextPainter object

CTextPainter::CTextPainter (CTextEffectCreator *pCreator) : m_pCreator(pCreator),
		m_xText(0),
		m_yText(0),
		m_cxText(0),
		m_cyText(0)

//	CTextPainter constructor

	{
	}

void CTextPainter::GetRect (RECT *retRect) const

//	GetRect
//
//	Returns the RECT

	{
	retRect->left = m_xText;
	retRect->top = m_yText;
	retRect->right = retRect->left + m_cxText;
	retRect->bottom = retRect->top + m_cyText;
	}

void CTextPainter::MeasureText (void)

//	MeasureText
//
//	Initializes cached measurements

	{
	//	Get the font

	const CG16bitFont *pFont = (m_pCreator ? m_pCreator->GetFont() : NULL);
	if (pFont == NULL)
		{
		m_xText = 0;
		m_yText = 0;
		m_cxText = 0;
		m_cyText = 0;
		return;
		}

	//	Measure it

	m_cxText = pFont->MeasureText(m_sText, &m_cyText);

	//	Position it

	m_xText = -(m_cxText / 2);
	m_yText = 0;
	}

void CTextPainter::OnReadFromStream (SLoadCtx &Ctx)

//	OnReadFromStream
//
//	Reads from a stream
//
//	CString			m_sText

	{
	m_sText.ReadFromStream(Ctx.pStream);
	MeasureText();
	}

void CTextPainter::OnWriteToStream (IWriteStream *pStream)

//	OnWriteToStream
//
//	Writes to a stream
//
//	CString			m_sText

	{
	m_sText.WriteToStream(pStream);
	}

void CTextPainter::Paint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx)

//	Paint
//
//	Paints the text

	{
	//	Get the font

	const CG16bitFont *pFont = (m_pCreator ? m_pCreator->GetFont() : NULL);
	if (pFont == NULL)
		return;

	WORD wColor = (m_pCreator ? m_pCreator->GetPrimaryColor() : 0xFFFF);
	DWORD dwFlags = (m_pCreator ? m_pCreator->GetFontFlags() : 0);
	DWORD byOpacity = (m_pCreator ? m_pCreator->GetOpacity() : 255);

	//	Compute the rect to paint

	RECT rcRect;
	rcRect.top = y;
	rcRect.bottom = Dest.GetHeight();

	if (dwFlags & CG16bitFont::AlignCenter)
		{
		int cxHalfWidth = Min(x, Dest.GetWidth() - x);
		rcRect.left = x - cxHalfWidth;
		rcRect.right = x + cxHalfWidth;
		}
	else if (dwFlags & CG16bitFont::AlignRight)
		{
		rcRect.left = 0;
		rcRect.right = x;
		}
	else
		{
		rcRect.left = x;
		rcRect.right = Dest.GetWidth();
		}

	//	Paint

	pFont->DrawText(Dest, rcRect, wColor, byOpacity, m_sText, 0, dwFlags);
	}

bool CTextPainter::PointInImage (int x, int y, int iTick, int iVariant, int iRotation) const

//	PointInImage
//
//	Returns TRUE if the point is in the image

	{
	RECT rcRect;
	GetRect(&rcRect);

	return (x >= rcRect.left && x < rcRect.right && y >= rcRect.top && y < rcRect.bottom);
	}

bool CTextPainter::SetParamString (const CString &sParam, const CString &sValue)

//	SetParamString
//
//	Sets parameters

	{
	if (strEquals(sParam, TEXT_ATTRIB))
		SetText(sValue);
	else
		return false;

	return true;
	}

bool CTextPainter::SetProperty (const CString &sProperty, ICCItem *pValue)

//	SetProperty
//
//	Sets a property

	{
	if (strEquals(sProperty, TEXT_ATTRIB))
		SetText(pValue->GetStringValue());
	else
		return false;

	return true;
	}

void CTextPainter::SetText (const CString &sText)

//	SetText
//
//	Sets the text

	{
	m_sText = sText;
	MeasureText();
	}
