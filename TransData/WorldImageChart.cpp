//	WorldImageChart.cpp
//
//	Generate a poster of asteroid and world images.
//	Copyright (c) 2013 by Kronosaur Productions, LLC. All Rights Reserved.

#include <stdio.h>

#include <windows.h>
#include "Alchemy.h"
#include "XMLUtil.h"
#include "TransData.h"

void GenerateWorldImageChart (CUniverse &Universe, CXMLElement *pCmdLine)
	{
	int i;

	enum OrderTypes
		{
		orderSmallest = 1,
		orderLargest = 2,
		orderName = 3,
		};

	//	Options

	bool bTextBoxesOnly = pCmdLine->GetAttributeBool(CONSTLIT("textBoxesOnly"));

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

	//	Generate a sorted table of worlds

	TSortMap<CString, CStationType *> Table;
	for (i = 0; i < Universe.GetStationTypeCount(); i++)
		{
		CStationType *pType = Universe.GetStationType(i);

		//	Skip non-generic classes and non-world objects

		if (pType->GetScale() != scaleWorld
				|| !pType->HasLiteralAttribute(CONSTLIT("generic")))
			continue;

		//	Compute the sort key

		char szBuffer[1024];
		switch (iOrder)
			{
			case orderLargest:
				wsprintf(szBuffer, "%09d%s%x",
						1000000 - pType->GetSize(),
						pType->GetNounPhrase(0).GetASCIIZPointer(),
						pType->GetUNID());
				break;

			case orderSmallest:
				wsprintf(szBuffer, "%09d%s%x",
						pType->GetSize(),
						pType->GetNounPhrase(0).GetASCIIZPointer(),
						pType->GetUNID());
				break;

			default:
				wsprintf(szBuffer, "%s%x", pType->GetNounPhrase(0).GetASCIIZPointer(), pType->GetUNID());
				break;
			}

		//	Add to list

		Table.Insert(CString(szBuffer), pType);
		}

	//	Allocate an arranger that tracks where to paint each world.

	CImageArranger Arranger;

	//	Settings for the overall arrangement

	CImageArranger::SArrangeDesc Desc;
	Desc.cxDesiredWidth = Max(512, cxDesiredWidth - (2 * (cxSpacing + cxExtraMargin)));
	Desc.cxSpacing = cxSpacing;
	Desc.cxExtraMargin = cxExtraMargin;
	Desc.pHeader = &NameFont;

	//	Generate a table of cells for the arranger

	TArray<CCompositeImageSelector> Selectors;
	Selectors.InsertEmpty(Table.GetCount());

	TArray<CImageArranger::SCellDesc> Cells;
	for (i = 0; i < Table.GetCount(); i++)
		{
		CStationType *pType = Table[i];

		SSelectorInitCtx InitCtx;
		pType->SetImageSelector(InitCtx, &Selectors[i]);
		const CObjectImageArray &Image = pType->GetImage(Selectors[i], CCompositeImageModifiers());

		CImageArranger::SCellDesc *pNewCell = Cells.Insert();
		pNewCell->cxWidth = RectWidth(Image.GetImageRect());
		pNewCell->cyHeight = RectHeight(Image.GetImageRect());
		pNewCell->sText = pType->GetNounPhrase(0);
		}

	//	Arrange

	Arranger.ArrangeByRow(Desc, Cells);

	//	Create a large image

	CG32bitImage Output;
	int cxWidth = Max(cxDesiredWidth, Arranger.GetWidth());
	int cyHeight = Arranger.GetHeight();
	Output.Create(cxWidth, cyHeight);
	printf("Creating %dx%d image.\n", cxWidth, cyHeight);

	//	Paint the images

	for (i = 0; i < Table.GetCount(); i++)
		{
		CStationType *pType = Table[i];

		int x = Arranger.GetX(i);
		int y = Arranger.GetY(i);
		if (x != -1)
			{
			if (!bTextBoxesOnly)
				{
				const CObjectImageArray &Image = pType->GetImage(Selectors[i], CCompositeImageModifiers());
				Image.PaintImageUL(Output,
						x,
						y,
						0,
						0);
				}

			//	Paint name

			int xText = Arranger.GetTextX(i);
			int yText = Arranger.GetTextY(i);
			if (xText != -1)
				{
				if (bTextBoxesOnly)
					Output.Fill(xText, yText, Arranger.GetTextWidth(i), Arranger.GetTextHeight(i), 0xffff);

				if (!bTextBoxesOnly)
					{
					Output.FillColumn(x + (Arranger.GetWidth(i) / 2),
							y + Arranger.GetHeight(i),
							yText - (y + Arranger.GetHeight(i)),
							rgbNameColor);

					NameFont.DrawText(Output,
							xText,
							yText,
							rgbNameColor,
							pType->GetNounPhrase(0));
					}
				}
			}
		}

	//	Write to file or clipboard

	OutputImage(Output, sFilespec);
	}

