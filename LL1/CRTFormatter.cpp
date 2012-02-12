//	CRTFormatter.cpp
//
//	CRTFormatter object

#include "PreComp.h"
#include "Tags.h"

#define TFLAG_BOLD							0x00000001
#define TFLAG_ITALIC						0x00000002
#define TFLAG_OPAQUE						0x00000004
#define TFLAG_ALLCAPS						0x00000008
#define TFLAG_SMALLCAPS						0x00000010

typedef struct
	{
	char *pszName;
	char *pszTypeface;
	float Height;
	COLORREF rgbText;
	COLORREF rgbBack;
	DWORD dwFlags;
	} TEXTFORMATINIT;

static TEXTFORMATINIT g_DefTextFormat[] =
	{
		{	"Default",		"Georgia", 10.0, RGB(0,0,0), RGB(255,255,255), 0 },
		{	"Body",			"Georgia", 10.0, RGB(0,0,0), RGB(255,255,255), 0 },
		{	"Title",		"Trebuchet MS", 16.0, RGB(96,96,96), RGB(255,255,255), TFLAG_BOLD },
		{	"Link",			"Georgia", 10.0, RGB(0,40,86), RGB(255,255,255), 0 },
		{	"Index",		"Georgia", 10.0, RGB(0,40,86), RGB(255,255,255), 0 },
		{	"Emphasis",		"Georgia", 10.0, RGB(0,0,0), RGB(255,255,255), TFLAG_ITALIC },
		{	"FieldData",	"Arial", 10.0, RGB(96,96,96), RGB(255,255,255), 0 },
		{	"Heading 1",	"Trebuchet MS", 12.0, RGB(0,0,0), RGB(255,255,255), TFLAG_BOLD },
		{	"Heading 2",	"Georgia", 10.0, RGB(0,0,0), RGB(255,255,255), TFLAG_SMALLCAPS },
		{	"Heading 3",	"Georgia", 10.0, RGB(0,0,0), RGB(255,255,255), TFLAG_ITALIC },
	};

#define DEFTEXTFORMAT_COUNT	(sizeof(g_DefTextFormat) / sizeof(g_DefTextFormat[0]))

#define PFLAG_NOINDENT_AFTER_HEADING		0x00000001
#define PFLAG_NUMBERED						0x00000002
#define PFLAG_DOTBULLET						0x00000004
#define PFLAG_BLOCK_SPACING_ONLY			0x00000008

typedef struct
	{
	char *pszName;
	float FirstLineLeftMargin;			//	All in inches
	float LeftMargin;
	float RightMargin;
	float SpaceBefore;
	float SpaceAfter;
	DWORD dwFlags;
	} PARAGRAPHFORMATINIT;

static PARAGRAPHFORMATINIT g_DefParagraphFormat[] =
	{
		{	"Default",		0.0,	0.0,	0.0,	0.0,	0.0,	0 },
		{	"Body",			0.333f,	0.0f,	0.0,	0.0,	0.0,	PFLAG_NOINDENT_AFTER_HEADING },
		{	"FieldData",	-1.0f,	1.0f,	0.0,	0.0,	0.0,	0 },
		{	"Heading 1",	0.0,	0.0,	0.0,	0.15f,	0.05f,	0 },
		{	"Heading 2",	0.0,	0.0f,	0.0,	0.09f,	0.03f,	0 },
		{	"Heading 3",	0.0,	0.0f,	0.0,	0.05f,	0.0,	0 },
		{	"Index",		-0.25f,	0.25f,	0.0,	0.1f,	0.1f,	PFLAG_BLOCK_SPACING_ONLY },
		{	"List 1",		-0.25f,	0.25,	0.5,	0.1f,	0.1f,	PFLAG_NUMBERED | PFLAG_BLOCK_SPACING_ONLY },
		{	"List 2",		-0.25f,	0.25f,	0.5,	0.1f,	0.1f,	PFLAG_DOTBULLET | PFLAG_BLOCK_SPACING_ONLY },
		{	"List 3",		-1.0f,	1.0f,	0.0,	0.1f,	0.1f,	PFLAG_BLOCK_SPACING_ONLY },
		{	"List 4",		-0.25f,	0.25f,	0.0,	0.1f,	0.1f,	PFLAG_BLOCK_SPACING_ONLY },
	};

#define DEFPARAGRAPHFORMAT_COUNT (sizeof(g_DefParagraphFormat) / sizeof(g_DefParagraphFormat[0]))

CRTFormatter::CRTFormatter (CCore *pCore) :
		m_pCore(pCore),
		m_TextFormat(TRUE, FALSE),
		m_ParagraphFormat(TRUE, FALSE),
		m_pDefTextFmt(NULL),
		m_pDefParagraphFmt(NULL),
		m_TwipsPerPixel(20.0)

//	CRTFormatter constructor

	{
	}

ALERROR CRTFormatter::AddTextFormat (CString sName, CRTTextFormat *pFormat)

//	AddTextFormat
//
//	Add a text format to our list

	{
	return m_TextFormat.AddEntry(sName, pFormat);
	}

ALERROR CRTFormatter::AddParagraphFormat (CString sName, CRTParagraphFormat *pFormat)

//	AddParagraphFormat
//
//	Add a frame format to our list

	{
	return m_ParagraphFormat.AddEntry(sName, pFormat);
	}

CRTParagraphFormat *CRTFormatter::ApplyParagraphFormat (CRTParagraphFormat *pOldFormat, CString sNewFormat)

//	ApplyParagraphFormat
//
//	Finds sNewFormat in the list of para formats and applies it

	{
	CRTParagraphFormat *pFmt = FindParagraphFormat(sNewFormat);

	if (pFmt == NULL)
		pFmt = m_pDefParagraphFmt;

	return pFmt;
	}

CRTTextFormat *CRTFormatter::ApplyTextFormat (CRTTextFormat *pOldFormat, CString sNewFormat)

//	ApplyTextFormat
//
//	Finds sNewFormat in the list of formats and applies it to pOldFormat
//	(if not NULL). Returns a pointer to the appropriate text format object

	{
	CRTTextFormat *pFmt = FindTextFormat(sNewFormat);

	if (pFmt == NULL)
		pFmt = m_pDefTextFmt;

	return pFmt;
	}

ALERROR CRTFormatter::Boot (void)

//	Boot
//
//	This must be called before the formatter can be used

	{
	ALERROR error;
	int i;

	//	Create some default text format

	for (i = 0; i < DEFTEXTFORMAT_COUNT; i++)
		{
		TEXTFORMATINIT *pInit = &g_DefTextFormat[i];
		CRTTextFormat *pTextFmt = new CRTTextFormat;
		if (pTextFmt == NULL)
			return ERR_MEMORY;

		//	Initialize

		pTextFmt->m_sTypeface = CString(pInit->pszTypeface);
		pTextFmt->m_Height = Points2Twips(pInit->Height);
		pTextFmt->m_rgbTextColor = pInit->rgbText;
		pTextFmt->m_rgbBackColor = pInit->rgbBack;
		pTextFmt->m_fBold = (pInit->dwFlags & TFLAG_BOLD ? TRUE : FALSE);
		pTextFmt->m_fItalic = (pInit->dwFlags & TFLAG_ITALIC ? TRUE : FALSE);
		pTextFmt->m_fOpaque = (pInit->dwFlags & TFLAG_OPAQUE ? TRUE : FALSE);
		pTextFmt->m_fAllCaps = (pInit->dwFlags & TFLAG_ALLCAPS ? TRUE : FALSE);
		pTextFmt->m_fSmallCaps = (pInit->dwFlags & TFLAG_SMALLCAPS ? TRUE : FALSE);

		if (error = AddTextFormat(CString(pInit->pszName), pTextFmt))
			return error;

		//	The first one is the default

		if (i == 0)
			m_pDefTextFmt = pTextFmt;
		}

	//	Create a default frame format

	for (i = 0; i < DEFPARAGRAPHFORMAT_COUNT; i++)
		{
		PARAGRAPHFORMATINIT *pInit = &g_DefParagraphFormat[i];
		int j;

		CRTParagraphFormat *pParagraphFmt = new CRTParagraphFormat;
		if (pParagraphFmt == NULL)
			return ERR_MEMORY;

		//	Initialize

		pParagraphFmt->m_FirstLineLeftMargin = Inches2Twips(pInit->FirstLineLeftMargin);
		pParagraphFmt->m_LeftMargin = Inches2Twips(pInit->LeftMargin);
		pParagraphFmt->m_RightMargin = Inches2Twips(pInit->RightMargin);
		pParagraphFmt->m_SpaceBefore = Inches2Twips(pInit->SpaceBefore);
		pParagraphFmt->m_SpaceAfter = Inches2Twips(pInit->SpaceAfter);

		pParagraphFmt->m_fNoIndentAfterHeading = (pInit->dwFlags & PFLAG_NOINDENT_AFTER_HEADING ? TRUE : FALSE);
		pParagraphFmt->m_fNumbered = (pInit->dwFlags & PFLAG_NUMBERED ? TRUE : FALSE);
		pParagraphFmt->m_fDotBullet = (pInit->dwFlags & PFLAG_DOTBULLET ? TRUE : FALSE);
		pParagraphFmt->m_fSpacingBeforeFirstOfBlock = (pInit->dwFlags & PFLAG_BLOCK_SPACING_ONLY ? TRUE : FALSE);
		pParagraphFmt->m_fSpacingAfterLastOfBlock = (pInit->dwFlags & PFLAG_BLOCK_SPACING_ONLY ? TRUE : FALSE);

		//	Add some tabs

		BOOL bNeedLeftMarginTab = TRUE;
		for (j = 0; j < 5; j++)
			{
			Twips Tab = (j + 1) * TWIPS_PER_INCH;

			//	We always have a tab at the left margin

			if (bNeedLeftMarginTab 
					&& pInit->LeftMargin != pInit->FirstLineLeftMargin
					&& Tab > pParagraphFmt->m_LeftMargin)
				{
				pParagraphFmt->AppendTab(pParagraphFmt->m_LeftMargin);
				bNeedLeftMarginTab = FALSE;
				}

			//	Do not add tabs that are before the left margin

			if (Tab > pParagraphFmt->m_LeftMargin)
				pParagraphFmt->AppendTab(Tab);
			}

		//	Add the paragraph format

		if (error = AddParagraphFormat(CString(pInit->pszName), pParagraphFmt))
			return error;

		//	The first one is the default

		if (i == 0)
			m_pDefParagraphFmt = pParagraphFmt;
		}

	return NOERROR;
	}

Twips CRTFormatter::CalcTwipsPerPixel (HDC hDC)

//	CalcTwipsPerPixel
//
//	Calculates the number of twips per pixel for the given device

	{
	int cxPixelsPerInch = GetDeviceCaps(hDC, LOGPIXELSX);
	if (cxPixelsPerInch > 0)
		return TWIPS_PER_INCH / cxPixelsPerInch;
	else
		return TWIPS_PER_INCH / 72;
	}

Twips CRTFormatter::CalcTwipsPerScreenPixel (void)

//	CalcTwipsPerScreenPixel
//
//	Calculates the number of twips per screen pixel

	{
	Twips TwipsPerPixel;

	HDC hDC = CreateCompatibleDC(NULL);
	TwipsPerPixel = CalcTwipsPerPixel(hDC);
	DeleteDC(hDC);

	return TwipsPerPixel;
	}

CRTParagraphFormat *CRTFormatter::FindParagraphFormat (CString sFmt)

//	FindParagraphFormat
//
//	Returns the given format by name (or NULL if not found)

	{
	CObject *pFmt;

	if (m_ParagraphFormat.Lookup(sFmt, &pFmt) != NOERROR)
		return NULL;

	return (CRTParagraphFormat *)pFmt;
	}

CRTTextFormat *CRTFormatter::FindTextFormat (CString sFmt)

//	FindTextFormat
//
//	Returns the given format by name (or NULL if not found)

	{
	CObject *pFmt;

	if (m_TextFormat.Lookup(sFmt, &pFmt) != NOERROR)
		return NULL;

	return (CRTTextFormat *)pFmt;
	}

ALERROR CRTFormatter::FormatFrame (RECT &rcRect, CString sData, CString sTextFmt, CString sParagraphFmt, CRTFrame **retpFrame)

//	FormatFrame
//
//	Formats the data using the current settings and returns a frame that
//	contains the formatted data. The width of the rect is used to compute
//	wrapping. The height is used to calculate the page size.

	{
	ALERROR error;
	CRTFrame *pFrame;
	int i;

	//	Create a new frame

	pFrame = new CRTFrame;
	if (pFrame == NULL)
		return ERR_MEMORY;

	//	Set the default formatting

	pFrame->SetCurrentTextFormat(ApplyTextFormat(NULL, sTextFmt));
	pFrame->SetCurrentParagraphFormat(ApplyParagraphFormat(NULL, sParagraphFmt));

	//	Parse

	CString sError;
	if (error = ParseTextBlock(sData, this, pFrame, &sError))
		{
		if (!sError.IsBlank())
			pFrame->AddText(strPatternSubst(LITERAL("[%s]"), sError));
		else
			return error;
		}

	//	HACK: Reset all fonts to use the new Twips per pixels value.

	for (i = 0; i < m_TextFormat.GetCount(); i++)
		{
		CRTTextFormat *pFmt = (CRTTextFormat *)m_TextFormat.GetValue(i);
		pFmt->ResetFont(m_TwipsPerPixel);
		}

	//	Done

	*retpFrame = pFrame;

	return NOERROR;
	}

void CRTTextFormat::ResetFont (Twips TwipsPerPixel)

//	ResetFont
//
//	Resets the current font to the given scale

	{
	m_Font.SetTypeface(m_sTypeface);
	m_Font.SetBold(m_fBold);
	m_Font.SetItalic(m_fItalic);

	//	Slightly smaller if small caps

	if (GetSmallCaps())
		m_Font.SetHeightInPixels(Twips2Pixels(m_Height * 5 / 6, TwipsPerPixel));
	else
		m_Font.SetHeightInPixels(Twips2Pixels(m_Height, TwipsPerPixel));
	}

CRTParagraphFormat::CRTParagraphFormat (void) : CObject(NULL)

//	CRTParagraphFormat constructor
	
	{
	for (int i = 0; i < MAX_TABS; i++)
		m_Tabs[i] = 0.0;
	}

void CRTParagraphFormat::AppendTab (Twips Tab)

//	AppendTab
//
//	Adds a tab. Tabs must be appended in ascending order

	{
	int i = 0;

	while (i < MAX_TABS && m_Tabs[i] > 0.0)
		i++;

	if (i < MAX_TABS)
		m_Tabs[i] = Tab;
	}

CString CRTParagraphFormat::GetBullet (int iNumber)

//	GetBullet
//
//	Returns the bullet to use

	{
	if (m_fNumbered)
		return strPatternSubst(LITERAL("%d."), iNumber);
	else if (m_fDotBullet)
		return CString("•");
	else
		return CString("•");
	}

Twips CRTParagraphFormat::GetNextTab (Twips Pos)

//	GetNextTab
//
//	Returns the next tab position after the given pos. If there
//	are no more tabs it returns 0.0

	{
	int i = 0;

	while (i < MAX_TABS && m_Tabs[i] > 0.0 && Pos >= m_Tabs[i])
		i++;

	if (i < MAX_TABS && m_Tabs[i] > 0.0)
		return m_Tabs[i];
	else
		return 0.0;
	}
