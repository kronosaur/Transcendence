//	TextFormat.h
//
//	Text format representation

#ifndef INCL_TEXT_FORMAT
#define INCL_TEXT_FORMAT

//	Text Format Structures -----------------------------------------------------
//
//	These structures are used to represent a text format.

struct STextFormatDesc
	{
	STextFormatDesc (void) :
			sTypeface(CONSTLIT("Default")),
			iFontSize(10),
			bBold(false),
			bItalic(false),
			wColor(0),
			dwOpacity(255)
		{ }

	CString sTypeface;					//	Typeface (use IFontTable to look up)
	int iFontSize;						//	Font size in points
	bool bBold;							//	Bold
	bool bItalic;						//	Italic

	WORD wColor;						//	16-bit color
	DWORD dwOpacity;					//	8-bit alpha value
	};

struct SBlockFormatDesc
	{
	SBlockFormatDesc (void) :
			iHorzAlign(alignLeft),
			iVertAlign(alignTop),
			cxWidth(-1),
			cyHeight(-1)

		{ }

	AlignmentStyles iHorzAlign;
	AlignmentStyles iVertAlign;

	int cxWidth;						//	Width of block (in pixels). -1 = no fixed width.
	int cyHeight;						//	Height of block (in pixels). -1 = no fixed height.
	};

class IFontTable
	{
	public:
		virtual ~IFontTable (void) { }

		virtual const CG16bitFont *GetFont (const STextFormatDesc &Desc) const { return NULL; }
	};

//	Text Object Structures -----------------------------------------------------
//
//	These structures are used to represent formatted text objects (they are
//	designed for painting, not for representing abstract text formatting).

struct STextFormat
	{
	const CG16bitFont *pFont;
	WORD wColor;
	DWORD dwOpacity;
	};

struct SFormattedTextSpan
	{
	CString sText;
	STextFormat Format;

	int x;
	int y;
	int cx;
	int cy;
	};

class CTextBlock
	{
	public:
		CTextBlock (void) { }

		void AddTextSpan (const CString &sText, const STextFormat &Format, bool bEoP = false);
		void Format (const SBlockFormatDesc &BlockFormat);
		void GetBounds (RECT *retrcRect);
		inline const SFormattedTextSpan &GetFormattedSpan (int iIndex) const { return m_Formatted[iIndex]; }
		inline int GetFormattedSpanCount (void) const { return m_Formatted.GetCount(); }
		bool InitFromRTF (const CString &RTF, const IFontTable &FontTable, const SBlockFormatDesc &BlockFormat, CString *retsError = NULL);

		static CString Escape (const CString &sText);

	private:
		struct STextSpan
			{
			CString sText;
			STextFormat Format;
			bool bEoP;

			int cx;						//	Temp variable used while formatting
			};

		TArray<STextSpan> m_Text;
		TArray<SFormattedTextSpan> m_Formatted;
	};

#endif