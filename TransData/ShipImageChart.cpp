//	ShipTable.cpp
//
//	Generate basic stats about ship classes

#include <stdio.h>

#include <windows.h>
#include "Alchemy.h"
#include "XMLUtil.h"
#include "TransData.h"

class CPaintMap
	{
	public:
		CPaintMap (int iCount)
			{
			m_Map.InsertEmpty(iCount);
			for (int i = 0; i < iCount; i++)
				m_Map[i].x = -1;

			m_cxMaxWidth = 0;
			m_cyMaxHeight = 0;
			}

		void AddMargin (int cx, int cy)
			{
			m_cxMaxWidth += cx;
			m_cyMaxHeight += cy;
			}

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

		void Place (int iIndex, int x, int y, int cxWidth, int cyHeight)
			{
			SEntry *pEntry = &m_Map[iIndex];
			pEntry->x = x;
			pEntry->y = y;
			pEntry->cxWidth = cxWidth;
			pEntry->cyHeight = cyHeight;

			pEntry->xText = -1;

			m_cxMaxWidth = Max(m_cxMaxWidth, x + cxWidth);
			m_cyMaxHeight = Max(m_cyMaxHeight, y + cyHeight);
			}

		void PlaceText (int iIndex, int x, int y, int cxWidth, int cyHeight)
			{
			SEntry *pEntry = &m_Map[iIndex];
			pEntry->xText = x;
			pEntry->yText = y;
			pEntry->cxText = cxWidth;
			pEntry->cyText = cyHeight;

			m_cxMaxWidth = Max(m_cxMaxWidth, x + cxWidth);
			m_cyMaxHeight = Max(m_cyMaxHeight, y + cyHeight);
			}

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

		TArray<SEntry> m_Map;
		int m_cxMaxWidth;
		int m_cyMaxHeight;
	};

struct SArrangeDesc
	{
	int cxDesiredWidth;
	int cxSpacing;
	int cxExtraMargin;
	bool bNoLabel;
	CG16bitFont *pHeader;
	};

void ArrangeByCell (CSymbolTable &Table, int cxDesiredWidth, CPaintMap &Map);
void ArrangeByRow (CSymbolTable &Table, SArrangeDesc &Desc, CPaintMap &Map);

void GenerateShipImageChart (CUniverse &Universe, CXMLElement *pCmdLine)
	{
	int i;

	enum OrderTypes
		{
		orderSmallest = 1,
		orderLargest = 2,
		orderName = 3,
		};

	//	Get the criteria from the command line. Always append 's' because we
	//	want ship classes.

	CString sCriteria = strPatternSubst(CONSTLIT("%s s"), pCmdLine->GetAttribute(CONSTLIT("criteria")));
	CDesignTypeCriteria Criteria;
	if (CDesignTypeCriteria::ParseCriteria(sCriteria, &Criteria) != NOERROR)
		{
		printf("ERROR: Unable to parse criteria.\n");
		return;
		}

	//	Options

	bool bAllClasses = pCmdLine->GetAttributeBool(CONSTLIT("all"));
	bool bTextBoxesOnly = pCmdLine->GetAttributeBool(CONSTLIT("textBoxesOnly"));
	bool bNoLabel = pCmdLine->GetAttributeBool(CONSTLIT("noLabel"));

	//	Figure out what order we want

	CString sOrder = pCmdLine->GetAttribute(CONSTLIT("sort"));
	int iOrder;
	if (strEquals(sOrder, CONSTLIT("smallest")))
		iOrder = orderSmallest;
	else if (strEquals(sOrder, CONSTLIT("largest")))
		iOrder = orderLargest;
	else
		iOrder = orderName;

	//	Image size

	int cxDesiredWidth;
	if (pCmdLine->FindAttributeInteger(CONSTLIT("width"), &cxDesiredWidth))
		cxDesiredWidth = Max(512, cxDesiredWidth);
	else
		cxDesiredWidth = 1280;

	//	Spacing

	int cxSpacing = pCmdLine->GetAttributeInteger(CONSTLIT("xSpacing"));
	int cxExtraMargin = pCmdLine->GetAttributeInteger(CONSTLIT("xMargin"));

	//	Rotation

	int iRotation = pCmdLine->GetAttributeInteger(CONSTLIT("rotation"));

	//	Font for text

	CString sTypeface;
	int iSize;
	bool bBold;
	bool bItalic;

	if (!CG16bitFont::ParseFontDesc(pCmdLine->GetAttribute(CONSTLIT("font")),
			&sTypeface,
			&iSize,
			&bBold,
			&bItalic))
		{
		sTypeface = CONSTLIT("Arial");
		iSize = 10;
		bBold = false;
		bItalic = false;
		}

	CG16bitFont NameFont;
	NameFont.Create(sTypeface, -PointsToPixels(iSize), bBold, bItalic);
	CG32bitPixel rgbNameColor = CG32bitPixel(255, 255, 255);

	//	Output file

	CString sFilespec = pCmdLine->GetAttribute(CONSTLIT("output"));
	if (!sFilespec.IsBlank())
		sFilespec = pathAddExtensionIfNecessary(sFilespec, CONSTLIT(".bmp"));

	//	Generate a table of ships

	CSymbolTable Table(FALSE, TRUE);
	for (i = 0; i < Universe.GetShipClassCount(); i++)
		{
		CShipClass *pClass = Universe.GetShipClass(i);

		//	Skip if we don't match criteria

		if (!pClass->MatchesCriteria(Criteria))
			continue;

		//	Skip non-generic classes

		if (!bAllClasses && !pClass->HasLiteralAttribute(CONSTLIT("genericClass")))
			continue;

		//	Compute the sort key

		char szBuffer[1024];
		switch (iOrder)
			{
			case orderLargest:
				wsprintf(szBuffer, "%04d%s%x",
						2048 - RectWidth(pClass->GetImage().GetImageRect()),
						pClass->GetNounPhrase(nounGeneric).GetASCIIZPointer(),
						pClass);
				break;

			case orderSmallest:
				wsprintf(szBuffer, "%04d%s%x",
						RectWidth(pClass->GetImage().GetImageRect()),
						pClass->GetNounPhrase(nounGeneric).GetASCIIZPointer(),
						pClass);
				break;

			default:
				wsprintf(szBuffer, "%s%x", pClass->GetNounPhrase(nounGeneric).GetASCIIZPointer(), pClass);
				break;
			}

		//	Add to list

		Table.AddEntry(CString(szBuffer), (CObject *)pClass);
		}

	//	Allocate a map that tracks where to paint each ship

	CPaintMap Map(Table.GetCount());

	//	Arrange the ships

	SArrangeDesc Desc;
	Desc.cxDesiredWidth = Max(512, cxDesiredWidth - (2 * (cxSpacing + cxExtraMargin)));
	Desc.cxSpacing = cxSpacing;
	Desc.cxExtraMargin = cxExtraMargin;
	Desc.bNoLabel = bNoLabel;
	Desc.pHeader = &NameFont;

	ArrangeByRow(Table, Desc, Map);
	//ArrangeByCell(Table, cxDesiredWidth, Map);

	//	Create a large image

	CG32bitImage Output;
	int cxWidth = Max(cxDesiredWidth, Map.GetWidth());
	int cyHeight = Map.GetHeight();
	Output.Create(cxWidth, cyHeight);
	printf("Creating %dx%d image.\n", cxWidth, cyHeight);

	//	Paint the images

	for (i = 0; i < Table.GetCount(); i++)
		{
		CShipClass *pClass = (CShipClass *)Table.GetValue(i);

		int x = Map.GetX(i);
		int y = Map.GetY(i);
		if (x != -1)
			{
			if (!bTextBoxesOnly)
				pClass->GetImage().PaintImageUL(Output,
						x,
						y,
						0,
						pClass->Angle2Direction(iRotation));

			//	Paint name

			if (!bNoLabel)
				{
				int xText = Map.GetTextX(i);
				int yText = Map.GetTextY(i);
				if (xText != -1)
					{
					if (bTextBoxesOnly)
						Output.Fill(xText, yText, Map.GetTextWidth(i), Map.GetTextHeight(i), 0xffff);

					if (!bTextBoxesOnly)
						{
						Output.FillColumn(x + (Map.GetWidth(i) / 2),
								y + Map.GetHeight(i),
								yText - (y + Map.GetHeight(i)),
								rgbNameColor);

						NameFont.DrawText(Output,
								xText,
								yText,
								rgbNameColor,
								pClass->GetNounPhrase(nounGeneric));
						}
					}
				}
			}
		}

	//	Write to file or clipboard

	OutputImage(Output, sFilespec);
	}

class CCellMap
	{
	public:
		CCellMap (int iCols, int iRows)
			{
			m_pCellMap = new bool [iCols * iRows];
			m_iCols = iCols;
			m_iRows = iRows;

			for (int i = 0; i < iCols * iRows; i++)
				m_pCellMap[i] = false;
			}

		~CCellMap (void)
			{
			delete [] m_pCellMap;
			}

		inline int GetCols (void) const { return m_iCols; }
		inline int GetRows (void) const { return m_iRows; }

		bool GetSpace (int iCellsNeeded, int *retx, int *rety)
			{
			int x = 0;
			int y = 0;
			while (!SpaceAvailable(x, y, iCellsNeeded))
				{
				x++;
				if (x >= m_iCols)
					{
					x = 0;
					y++;
					if (y >= m_iRows)
						{
						int i;

						//	If no room, then increase to a larger
						//	number of rows

						int iNewRows = m_iRows + iCellsNeeded;
						bool *pNewCellMap = new bool [m_iCols * iNewRows];
						int iOldCount = m_iCols * m_iRows;
						int iNewCount = m_iCols * iNewRows;
						for (i = 0; i < iOldCount; i++)
							pNewCellMap[i] = m_pCellMap[i];
						for (i = iOldCount; i < iNewCount; i++)
							pNewCellMap[i] = false;

						//	Swap

						delete [] m_pCellMap;
						m_pCellMap = pNewCellMap;
						m_iRows = iNewRows;

						//	Restart a little ways up

						y = Max(0, y - iCellsNeeded);
						}
					}
				}

			for (int y1 = 0; y1 < iCellsNeeded; y1++)
				for (int x1 = 0; x1 < iCellsNeeded; x1++)
					MarkUsed(x + x1, y + y1);

			*retx = x;
			*rety = y;

			return true;
			}

		bool InUse (int x, int y)
			{
			return m_pCellMap[y * m_iCols + x];
			}

		void MarkUsed (int x, int y)
			{
			m_pCellMap[y * m_iCols + x] = true;
			}

		bool SpaceAvailable (int x, int y, int iCellsNeeded)
			{
			if (x + iCellsNeeded > m_iCols)
				return false;
			else if (y + iCellsNeeded > m_iRows)
				return false;
			else
				{
				for (int y1 = 0; y1 < iCellsNeeded; y1++)
					for (int x1 = 0; x1 < iCellsNeeded; x1++)
						if (InUse(x + x1, y + y1))
							return false;

				return true;
				}
			}

	private:
		bool *m_pCellMap;
		int m_iCols;
		int m_iRows;
	};

void ArrangeByCell (CSymbolTable &Table, int cxDesiredWidth, CPaintMap &Map)
	{
	int i;

	//	Compute the size of each image cell

	int cxCell = 32;
	int cyCell = 32;

	//	Compute the number of rows & columns

	int iInitCols = (cxDesiredWidth / cxCell) + ((cxDesiredWidth % cxCell) ? 1 : 0);
	int iInitRows = AlignUp(20 * Table.GetCount(), iInitCols) / iInitCols;

	//	Create an array that keeps track of which cells we've used up

	CCellMap CellMap(iInitCols, iInitRows);

	//	Figure out where to place all the ships

	for (i = 0; i < Table.GetCount(); i++)
		{
		CShipClass *pClass = (CShipClass *)Table.GetValue(i);

		//	Figure out how many cells we need

		int cxSize = RectWidth(pClass->GetImage().GetImageRect());
		int iCellsNeeded = AlignUp(cxSize, cxCell) / cxCell;
		int cxAreaSize = iCellsNeeded * cxCell;

		//	Position the ship

		int x, y;
		if (CellMap.GetSpace(iCellsNeeded, &x, &y))
			Map.Place(i,
				x * cxCell + (cxAreaSize - cxSize) / 2,
				y * cyCell + (cxAreaSize - cxSize) / 2,
				cxSize,
				cxSize);
		}
	}

void ArrangeByRow (CSymbolTable &Table, SArrangeDesc &Desc, CPaintMap &Map)
	{
	int iNext = 0;
	int y = 0;
	int cyHeader = Desc.pHeader->GetHeight();

	int cxInternalSpacing = 8;
	int cyInternalSpacing = 2 * Desc.pHeader->GetHeight();
	int cyNameSpacing = Desc.pHeader->GetHeight() / 2;

	while (iNext < Table.GetCount())
		{
		int i;
		int cxWidthLeft = Desc.cxDesiredWidth;
		int cyRowHeight = 0;
		int iStart = iNext;

		//	First figure out how many ships will fit
		
		while (iNext < Table.GetCount())
			{
			CShipClass *pClass = (CShipClass *)Table.GetValue(iNext);
			int cxSize = RectWidth(pClass->GetImage().GetImageRect());
			if (cxSize > cxWidthLeft && iStart != iNext)
				break;

			int cxCell = Max(cxSize + cxInternalSpacing, Desc.cxSpacing);

			cxWidthLeft -= cxCell;
			if (cxSize > cyRowHeight)
				cyRowHeight = cxSize;

			iNext++;
			}

		//	Compute the total width

		int cxRowWidth = Min(Desc.cxDesiredWidth - cxWidthLeft, Desc.cxDesiredWidth);
		int xOffset = (Desc.cxDesiredWidth - cxRowWidth) / 2;

		//	See if any of the ships overlap the text from the previous ships
		//	If so, we increase y a little bit

		int x = Desc.cxSpacing + Desc.cxExtraMargin;
		if (!Desc.bNoLabel)
			{
			for (i = iStart; i < iNext; i++)
				{
				CShipClass *pClass = (CShipClass *)Table.GetValue(i);
				int cxSize = RectWidth(pClass->GetImage().GetImageRect());
				int yOffset = (cyRowHeight - cxSize) / 2;

				int xPoint = x + xOffset + cxSize / 2;
				int yPoint = y + yOffset;

				for (int j = 0; j < iStart; j++)
					{
					int xText = Map.GetTextX(j);
					int yText = Map.GetTextY(j);
					int cxText = Map.GetTextWidth(j);
					int cyText = Map.GetTextHeight(j) + cyInternalSpacing;

					if (xPoint >= xText && xPoint < xText + cxText && yPoint < yText + cyText)
						y = yText + cyText + cyInternalSpacing;
					}

				int cxCell = Max(cxSize + cxInternalSpacing, Desc.cxSpacing);
				x += cxCell;
				}
			}

		//	Place the ships

		x = Desc.cxSpacing + Desc.cxExtraMargin;
		int yOverlapOffset = 0;
		for (i = iStart; i < iNext; i++)
			{
			CShipClass *pClass = (CShipClass *)Table.GetValue(i);
			int cxSize = RectWidth(pClass->GetImage().GetImageRect());
			int cxCell = Max(cxSize + cxInternalSpacing, Desc.cxSpacing);

			//	Center vertically

			int yOffset = (cyRowHeight - cxSize) / 2;

			//	Place

			Map.Place(i, x + xOffset, y + yOffset, cxSize, cxSize);

			//	Figure out the position of the text

			int cyName;
			int cxName = Desc.pHeader->MeasureText(pClass->GetNounPhrase(nounGeneric), &cyName);
			if (cxName <= cxSize)
				{
				int yText = y + yOffset + cxSize + cyNameSpacing;
				Map.PlaceText(i,
						x + xOffset + (cxSize - cxName) / 2,
						yText,
						cxName,
						cyName);

				yOverlapOffset = yText + cyName + cyNameSpacing;
				}
			else
				{
				//	See if the name fits above the previous name

				int yText;
				if (yOverlapOffset > 0 
						&& i > iStart 
						&& (x + xOffset + cxSize - (Map.GetX(i - 1) + Map.GetWidth(i - 1) / 2)) > cxName + 8)
					{
					yText = y + yOffset + cxSize + cyNameSpacing;
					if (yText < Map.GetY(i - 1) + Map.GetHeight(i - 1))
						yText = Map.GetY(i - 1) + Map.GetHeight(i - 1) + cyNameSpacing;

					if (yText > Map.GetTextY(i - 1) - (cyName + cyNameSpacing))
						yText = yOverlapOffset;
					}

				//	Otherwise, use the overlap, if it exits

				else if (yOverlapOffset == 0)
					yText = y + yOffset + cxSize + cyNameSpacing;
				else
					yText = yOverlapOffset;

				Map.PlaceText(i,
						x + xOffset + cxSize - cxName,
						yText,
						cxName,
						cyName);

				yOverlapOffset = yText + cyName + cyNameSpacing;
				}

			x += cxCell;
			}

		y += cyRowHeight;
		}

	//	Add a margin to the right and bottom

	Map.AddMargin(Desc.cxSpacing + Desc.cxExtraMargin, cyInternalSpacing);
	}

bool OutputImage (CG32bitImage &Image, const CString &sFilespec)
	{
	if (!sFilespec.IsBlank())
		{
		if (Image.SaveAsWindowsBMP(sFilespec) != NOERROR)
			{
			printf("Unable to save to '%s'\n", sFilespec.GetASCIIZPointer());
			return false;
			}

		printf("%s\n", sFilespec.GetASCIIZPointer());
		}

	//	Otherwise, clipboard

	else
		{
		if (!Image.CopyToClipboard())
			{
			printf("Unable to copy to clipboard.\n");
			return false;
			}

		printf("Image copied to clipboard.\n");
		}

	//	Done

	return true;
	}
