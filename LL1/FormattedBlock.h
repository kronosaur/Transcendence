//	FormattedBlock.h
//
//	Objects for formatted text

#ifndef INCL_FORMATTEDBLOCK
#define INCL_FORMATTEDBLOCK

typedef float Twips;

const Twips TWIPS_PER_POINT = 20.0;
const Twips TWIPS_PER_INCH = 1440.0;

class CRTTextFormat : public CObject
	{
	public:
		CRTTextFormat (void) : CObject(NULL) { }

		inline BOOL GetAllCaps (void) { return m_fAllCaps; }
		inline BOOL GetSmallCaps (void) { return m_fSmallCaps; }
		inline HFONT GetFont (void) { return m_Font.GetFont(); }
		inline COLORREF GetTextColor (void) { return m_rgbTextColor; }
		inline COLORREF GetBackColor (void) { return m_rgbBackColor; }
		inline BOOL GetOpaque (void) { return m_fOpaque; }
		void ResetFont (Twips TwipsPerPixel);

	public:
		CString m_sTypeface;						//	Typeface
		Twips m_Height;								//	Font size
		COLORREF m_rgbTextColor;					//	Text color
		COLORREF m_rgbBackColor;					//	Color of background
		DWORD m_fBold:1;							//	TRUE if bold
		DWORD m_fItalic:1;							//	TRUE if italic
		DWORD m_fOpaque:1;							//	TRUE if opaque background
		DWORD m_fAllCaps:1;							//	TRUE if all caps
		DWORD m_fSmallCaps:1;						//	TRUE if small capitals

	private:
		CGFont m_Font;								//	Current font
	};

class CRTParagraphFormat : public CObject
	{
	public:
		enum BulletType
			{
			Numbers,
			Letters,
			CapitalRomans,
			SmallRomans,
			Dot,
			Square,
			};

		CRTParagraphFormat (void);
		void AppendTab (Twips Tab);
		CString GetBullet (int iNumber);
		Twips GetNextTab (Twips Pos);
		inline BOOL HasBullet (void) { return m_fNumbered | m_fDotBullet; }
		inline BOOL NoIndentAfterHeading (void) { return m_fNoIndentAfterHeading; }
		inline BOOL SpacingBeforeFirstOfBlock (void) { return m_fSpacingBeforeFirstOfBlock; }
		inline BOOL SpacingAfterLastOfBlock (void) { return m_fSpacingAfterLastOfBlock; }

	public:
		Twips m_FirstLineLeftMargin;				//	Offset of first line relative to left margin
		Twips m_LeftMargin;							//	Left margin
		Twips m_RightMargin;						//	Right margin
		Twips m_SpaceBefore;						//	Vertical spacing before
		Twips m_SpaceAfter;							//	Vertical spacing after

		DWORD m_fNoIndentAfterHeading:1;			//	If TRUE, do not indent after a heading
		DWORD m_fNumbered:1;						//	If TRUE, number paragraphs
		DWORD m_fDotBullet:1;						//	If TRUE, has a dot bullet
		DWORD m_fSpacingBeforeFirstOfBlock:1;		//	If TRUE, spacing only before first of block
		DWORD m_fSpacingAfterLastOfBlock:1;			//	If TRUE, spacing only after last of block

	private:
		enum { MAX_TABS = 10 };

		Twips m_Tabs[MAX_TABS];						//	Tabs for this paragraph
	};

class CRTRun : public CObject
	{
	public:
		CRTRun (void);

		inline ALERROR AppendText (CString sText) { return m_sText.Append(sText); }
		inline int GetBaseline (void) { return m_iBaseline; }
		inline CRTTextFormat *GetFormat (void) { return m_pFmt; }
		inline CString GetLinkInfo (void) { return m_sLinkTo; }
		inline RECT &GetJustification (void) { return m_rcJustify; }
		inline BOOL HasTab (void) { return m_fTab; }
		inline BOOL HitTest (POINT &ptPoint) { return PtInRect(&m_rcJustify, ptPoint); }
		void Measure (HDC hDC,
				Twips TwipsPerPixel,
				int cxSpaceLeft,
				BOOL bStartOfLine,
				RECT *retRect,
				int *retiSplitAt,
				int *retiBaseline);
		void Paint (HDC hDC, int x, int y, RECT &rcUpdateRect);
		inline void SetBaseline (int iBaseline) { m_iBaseline = iBaseline; }
		inline void SetFormat (CRTTextFormat *pFmt) { m_pFmt = pFmt; }
		inline void SetJustification (RECT &rcRect) { m_rcJustify = rcRect; }
		inline void SetLinkInfo (CString sLinkTo) { m_sLinkTo = sLinkTo; }
		inline void SetTab (void) { m_fTab = TRUE; }
		inline void SetText (CString sText) { m_sText = sText; }
		ALERROR Split (int iSplitAt, CRTRun **retpRun);

	private:
		CString GetTextToPaint (void);

		CString m_sText;							//	Text to display
		CRTTextFormat *m_pFmt;						//	Text format

		CString m_sLinkTo;							//	Link target

		RECT m_rcJustify;							//	Current justification
		int m_iBaseline;							//	Offset from top of rect to baseline
		DWORD m_fSplit:1;							//	TRUE if the run has been split from
													//		Previous run
		DWORD m_fTab:1;								//	TRUE if this run is prepended by a tab
		DWORD m_fStartOfLine:1;						//	TRUE if this run is at the start of a line
	};

class CRTParagraph : public CObject
	{
	public:
		CRTParagraph (void);

		inline ALERROR AppendRun (CRTRun *pRun) { return m_RunList.AppendObject(pRun, NULL); }
		inline BOOL IsFirstOfSection (void) { return m_fFirstOfSection; }
		inline BOOL IsFirstOfBlock (void) { return m_fFirstOfBlock; }
		inline BOOL IsHeading (void) { return m_fHeading; }
		inline BOOL IsLastOfBlock (void) { return m_fLastOfBlock; }
		inline BOOL IsNumbered (void) { return m_pFmt->m_fNumbered; }
		CRTRun *HitTestRun (POINT &ptPoint);
		void Justify (HDC hDC, RECT *ioRect, Twips TwipsPerPixel);
		void Paint (HDC hDC, int x, int y, RECT &rcUpdateRect);
		inline void SetFirstOfSection (void) { m_fFirstOfSection = TRUE; }
		inline void SetFirstOfBlock (void) { m_fFirstOfBlock = TRUE; }
		inline void SetFormat (CRTParagraphFormat *pFmt) { m_pFmt = pFmt; }
		inline void SetHeading (void) { m_fHeading = TRUE; }
		inline void SetLastOfBlock (void) { m_fLastOfBlock = TRUE; }
		inline void SetNumber (int iNumber) { m_iNumber = iNumber; }

	private:
		CRTRun *GetRun (int iIndex) { return (CRTRun *)m_RunList.GetObject(iIndex); }

		CObjectArray m_RunList;						//	List of CRTRuns
		RECT m_rcRect;								//	Rect that contains paragraph
		CRTParagraphFormat *m_pFmt;					//	format

		RECT m_rcJustify;							//	Current justification
		int m_xBullet;								//	Coordinate of bullet
		int m_yBullet;
		int m_iNumber;								//	Number

		DWORD m_fHeading:1;							//	Paragraph is a heading
		DWORD m_fFirstOfSection:1;					//	First paragraph after a heading
		DWORD m_fFirstOfBlock:1;					//	First paragraph in a block
		DWORD m_fLastOfBlock:1;						//	Last paragraph in a block
	};

class CRTFrame : public CObject
	{
	public:
		CRTFrame (void);

		ALERROR AddTab (void);
		ALERROR AddText (CString sText);
		inline void ClearParagraphHeading (void) { m_fCurParagraphIsHeading = FALSE; }
		inline CRTTextFormat *GetCurrentTextFormat (void) { return m_pCurTextFmt; }
		inline CRTParagraphFormat *GetCurrentParagraphFormat (void) { return m_pCurFmt; }
		ALERROR NewParagraph (void);
		ALERROR SetCurrentLink (CString sLink);
		inline void SetCurrentParagraphFormat (CRTParagraphFormat *pFmt) { m_pCurFmt = pFmt; NewParagraph(); }
		inline void SetCurrentTextFormat (CRTTextFormat *pFmt) { m_pCurRun = NULL; m_pCurTextFmt = pFmt; }
		inline void SetParagraphHeading (void) { m_fCurParagraphIsHeading = TRUE; }
		inline void SetFirstOfBlock (void) { m_fCurParagraphIsFirstOfBlock = TRUE; }
		inline void SetLastOfBlock (void) { if (m_pPrevParagraph) m_pPrevParagraph->SetLastOfBlock(); }

		inline int GetHeight (void) { return RectHeight(&m_rcJustify); }
		void GetRect (RECT *retrcRect);				//	Returns rect of frame
		CRTRun *HitTestRun (int xOrigin, int yOrigin, POINT &ptPoint);
		void Justify (HDC hDC, RECT *ioRect, Twips TwipsPerPixel);
		void Paint (HDC hDC, int x, int y, RECT &rcUpdateRect);

	private:
		CRTParagraph *GetParagraph (int iIndex) { return (CRTParagraph *)m_ParagraphList.GetObject(iIndex); }

		CObjectArray m_ParagraphList;				//	List of CRTParagraphs

		int m_xScroll;								//	Scroll position
		int m_yScroll;

		CRTParagraphFormat *m_pCurFmt;				//	Current paragraph format
		CRTParagraph *m_pCurParagraph;				//	Current paragraph
		CRTParagraph *m_pPrevParagraph;				//	Previous paragraph

		CRTTextFormat *m_pCurTextFmt;				//	Current text format
		CRTRun *m_pCurRun;							//	Current run of text
		CString m_sCurLink;							//	Current link

		DWORD m_fAddTab:1;							//	TRUE if we prepend a tab to the next run
		DWORD m_fCurParagraphIsHeading:1;			//	TRUE if current paragraph is a heading
		DWORD m_fCurParagraphIsFirstOfBlock:1;		//	TRUE if current paragraph is first of block

		RECT m_rcJustify;							//	Current justification (0,0 origin)
	};

class CRTFormatter
	{
	public:
		CRTFormatter (CCore *pCore);

		ALERROR AddTextFormat (CString sName, CRTTextFormat *pFormat);
		ALERROR AddParagraphFormat (CString sName, CRTParagraphFormat *pFormat);
		CRTParagraphFormat *ApplyParagraphFormat (CRTParagraphFormat *pOldFormat, CString sNewFormat);
		CRTTextFormat *ApplyTextFormat (CRTTextFormat *pOldFormat, CString sNewFormat);
		ALERROR Boot (void);
		ALERROR FormatFrame (RECT &rcRect, CString sData, CString sTextFmt, CString sParagraphFmt, CRTFrame **retpFrame);
		inline CCore *GetCore (void) { return m_pCore; }
		inline void SetScale (Twips TwipsPerPixel) { m_TwipsPerPixel = TwipsPerPixel; }

		static Twips CalcTwipsPerPixel (HDC hDC);
		static Twips CalcTwipsPerScreenPixel (void);

	private:
		CRTParagraphFormat *FindParagraphFormat (CString sFmt);
		CRTTextFormat *FindTextFormat (CString sFmt);

		CCore *m_pCore;								//	To load links

		CSymbolTable m_TextFormat;					//	List of CRTTextFormats
		CSymbolTable m_ParagraphFormat;				//	List of CRTParagraphFormats

		CRTTextFormat *m_pDefTextFmt;				//	Default text format
		CRTParagraphFormat *m_pDefParagraphFmt;		//	Default frame format

		Twips m_TwipsPerPixel;						//	Scaling: Twips per pixel
	};

inline Twips Inches2Twips (float Inches) { return Inches * TWIPS_PER_INCH; }
inline Twips Points2Twips (float Points) { return Points * TWIPS_PER_POINT; }
inline int Twips2Pixels (Twips x, Twips TwipsPerPixel) { return (int)(x / TwipsPerPixel); }

//	XML functions

ALERROR XMLParseContent (char *pStart, CString *retsText, char **retpEnd, CString *retsError);
ALERROR XMLParseTag (char *pStart, CString *retsTag, BOOL *retbClose, BOOL *retbEmpty, CSymbolTable *retpAttributes, char **retpEnd);
ALERROR XMLValidate (CString sData, CString *retsError, int *retiErrorPos);

ALERROR ParseTextBlock (CString sText, CRTFormatter *pFormat, CRTFrame *pFrame, CString *retsError);

#endif