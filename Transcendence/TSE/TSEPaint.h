//	TSEPaint.h
//
//	Transcendence Paint Utilities
//	Copyright (c) 2013 by Kronosaur Productions, LLC. All Rights Reserved.

#ifndef INCL_TSE_PAINT
#define INCL_TSE_PAINT

class CImageArranger
	{
	public:
		struct SArrangeDesc
			{
			int cxDesiredWidth;
			int cxSpacing;
			int cxExtraMargin;
			CG16bitFont *pHeader;
			};

		struct SCellDesc
			{
			int cxWidth;
			int cyHeight;
			CString sText;
			};

		CImageArranger (void);

		void AddMargin (int cx, int cy);
		void ArrangeByRow (const SArrangeDesc &Desc, const TArray<SCellDesc> &Cells);
		inline int GetHeight (void) const { return m_cyMaxHeight; }
		inline int GetHeight (int iIndex) const { return m_Map[iIndex].cyHeight; }
		inline int GetTextHeight (int iIndex) const { return m_Map[iIndex].cyText; }
		inline int GetTextWidth (int iIndex) const { return m_Map[iIndex].cxText; }
		inline int GetTextX (int iIndex) const { return m_Map[iIndex].xText; }
		inline int GetTextY (int iIndex) const { return m_Map[iIndex].yText; }
		inline int GetWidth (void) const { return m_cxMaxWidth; }
		inline int GetWidth (int iIndex) const { return m_Map[iIndex].cxWidth; }
		inline int GetX (int iIndex) const { return m_Map[iIndex].x; }
		inline int GetY (int iIndex) const { return m_Map[iIndex].y; }
		void Place (int iIndex, int x, int y, int cxWidth, int cyHeight);
		void PlaceText (int iIndex, int x, int y, int cxWidth, int cyHeight);

	private:
		struct SEntry
			{
			int x;
			int y;
			int cxWidth;
			int cyHeight;

			int xText;
			int yText;
			int cxText;
			int cyText;
			};

		void Init (int iCells);

		TArray<SEntry> m_Map;
		int m_cxMaxWidth;
		int m_cyMaxHeight;
	};

class CPaintHelper
	{
	public:
		static void PaintArrow (CG16bitImage &Dest, int x, int y, int iDirection, WORD wColor);
		static void PaintStatusBar (CG16bitImage &Dest, int x, int y, int iTick, WORD wColor, const CString &sLabel, int iPos, int iMaxPos = 100, int *retcyHeight = NULL);
		static void PaintTargetHighlight (CG16bitImage &Dest, int x, int y, int iTick, int iRadius, int iRingSpacing, int iDelay, WORD wColor);
	};

#endif