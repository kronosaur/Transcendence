//	Effects.cpp
//
//	Generate basic stats about effects
//
//	An Effect Render File describes the set of effects to render. It has the
//	following format:
//
//	<TranscendenceRender>
//		<Render
//				width="..."			; defaults to 128
//				height="..."		; defaults to 128
//				frames="..."		; defaults to 1
//				rotation="..."		; defaults to 0
//				>
//			<Effect>
//				...
//			</Effect>
//		</Render>
//	</TranscendenceRender>

#include <stdio.h>

#include <windows.h>
#include "Alchemy.h"
#include "XMLUtil.h"
#include "TransData.h"

struct SEffectRender
	{
	SEffectRender (void) :
			pPainter(NULL),
			iLifetime(1)
		{ }

	~SEffectRender (void)
		{
		if (pPainter)
			pPainter->Delete();
		}

	CEffectCreatorRef pEffectCreator;
	IEffectPainter *pPainter;
	int iLifetime;
	};

void GenerateEffectImage (CUniverse &Universe, CXMLElement *pCmdLine)
	{
	CString sError;
	int i, j;

	//	Input file

	CString sInput = pCmdLine->GetAttribute(CONSTLIT("input"));
	if (sInput.IsBlank())
		{
		printf("Input file required.\n");
		return;
		}

	//	Output file (optional)

	CString sFilespec = pCmdLine->GetAttribute(CONSTLIT("output"));
	if (!sFilespec.IsBlank())
		sFilespec = pathAddExtensionIfNecessary(sFilespec, CONSTLIT(".bmp"));

	//	Load a resource file so that we can create a design load context

	CResourceDb Resources(CONSTLIT("Transcendence"));
	if (Resources.Open(DFOPEN_FLAG_READ_ONLY, &sError) != NOERROR)
		{
		printf("%s\n", (LPSTR)sError);
		return;
		}

	CExternalEntityTable *pEntities;
	if (Resources.LoadEntities(&sError, &pEntities) != NOERROR)
		{
		printf("%s\n", sError.GetASCIIZPointer());
		return;
		}

	//	Generate a list of effect render structures

	TArray<SEffectRender> Effects;

	//	Load the input file

	CFileReadBlock InputFile(sInput);
	if (InputFile.Open() != NOERROR)
		{
		printf("Unable to open input file: %s\n", sInput.GetASCIIZPointer());
		return;
		}

	//	Parse the file

	CXMLElement::SParseOptions Options;
	Options.pController = pEntities;

	CXMLElement *pRenderFile;
	if (CXMLElement::ParseXML(InputFile, Options, &pRenderFile, &sError) != NOERROR)
		{
		printf("%s\n", sError.GetASCIIZPointer());
		return;
		}

	//	Keep track of the max cell size and frame count

	int cxCellWidth = 0;
	int cyCellHeight = 0;
	int iCellsPerEffect = 1;

	//	Generate structures

	SDesignLoadCtx LoadCtx;
	LoadCtx.sResDb = Resources.GetFilespec();
	LoadCtx.pResDb = &Resources;

	for (i = 0; i < pRenderFile->GetContentElementCount(); i++)
		{
		CXMLElement *pRender = pRenderFile->GetContentElement(i);
		CXMLElement *pEffectDesc = pRender->GetContentElementByTag(CONSTLIT("Effect"));
		if (pEffectDesc == NULL)
			{
			printf("<Effect> tag required.\n");
			return;
			}

		SEffectRender *pEffect = Effects.Insert();

		//	Parse the effect

		if (pEffect->pEffectCreator.LoadEffect(LoadCtx, CONSTLIT("none"), pEffectDesc, NULL_STR) != NOERROR)
			{
			printf("%s\n", LoadCtx.sError.GetASCIIZPointer());
			return;
			}

		//	Bind

		if (pEffect->pEffectCreator.Bind(LoadCtx) != NOERROR)
			{
			printf("%s\n", LoadCtx.sError.GetASCIIZPointer());
			return;
			}

		//	Create a painter

		pEffect->pPainter = pEffect->pEffectCreator->CreatePainter(CCreatePainterCtx());
		if (pEffect->pPainter == NULL)
			{
			printf("Unable to create painter.\n");
			return;
			}

		//	Render specs

		int cyHeight = pRender->GetAttributeIntegerBounded(CONSTLIT("height"), 0, -1, 128);
		int cxWidth = pRender->GetAttributeIntegerBounded(CONSTLIT("width"), 0, -1, 128);

		cxCellWidth = Max(cxCellWidth, cxWidth);
		cyCellHeight = Max(cyCellHeight, cyHeight);

		//	Figure out how many animation cells

		pEffect->iLifetime = Max(1, pEffect->pEffectCreator->GetLifetime());
		iCellsPerEffect = Max(iCellsPerEffect, pEffect->iLifetime);
		}

	//	Create the resulting image

	CImageGrid Output;
	Output.Create(iCellsPerEffect * Effects.GetCount(), cxCellWidth, cyCellHeight);

	//	Paint

	for (i = 0; i < Effects.GetCount(); i++)
		{
		SEffectRender *pEffect = &Effects[i];

		SViewportPaintCtx Ctx;

		for (j = 0; j < pEffect->iLifetime; j++)
			{
			int x, y;
			Output.GetCellCenter(i * iCellsPerEffect + j, &x, &y);

			//	Create a context

			Ctx.iTick = j;

			//	Paint the effect

			pEffect->pPainter->Paint(Output.GetImage(), x, y, Ctx);

			//	Update

			pEffect->pPainter->OnUpdate();
			}
		}

	//	Output

	OutputImage(Output.GetImage(), sFilespec);
	}
