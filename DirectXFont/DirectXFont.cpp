//	DirectXFont
//
//	App to create a font bitmap

#include "stdafx.h"

#define NOARGS									CONSTLIT("noArgs")
#define NO_LOGO_SWITCH							CONSTLIT("nologo")
#define FONT_SWITCH								CONSTLIT("font")
#define INPUT_SWITCH							CONSTLIT("input")
#define OUTPUT_SWITCH							CONSTLIT("output")
#define SWITCH_HELP								CONSTLIT("help")

#define FONT_TAG								CONSTLIT("Font")
#define TYPEFACE_ATTRIB							CONSTLIT("typeface")
#define SIZE_ATTRIB								CONSTLIT("size")
#define BOLD_ATTRIB								CONSTLIT("bold")
#define ITALIC_ATTRIB							CONSTLIT("italic")
#define OUTPUT_ATTRIB							CONSTLIT("output")

void AlchemyMain (CXMLElement *pCmdLine);
ALERROR CreateFont (CXMLElement *pCmdLine, CG16bitFont *pFont);
void ProcessInputFile (const CString &sInputFile);

int main (int argc, char *argv[ ], char *envp[ ])
	{
	if (!kernelInit())
		{
		printf("ERROR: Unable to initialize Alchemy kernel.\n");
		return 1;
		}

	//	Do it

	{
	ALERROR error;
	CXMLElement *pCmdLine;
	if (error = CreateXMLElementFromCommandLine(argc, argv, &pCmdLine))
		{
		printf("ERROR: Unable to parse command line.\n");
		return 1;
		}

	AlchemyMain(pCmdLine);

	delete pCmdLine;
	}

	//	Done

	kernelCleanUp();
	return 0;
	}

void AlchemyMain (CXMLElement *pCmdLine)

//	AlchemyMain
//
//	Main entry-point after kernel initialization

	{
	ALERROR error;
	bool bLogo = !pCmdLine->GetAttributeBool(NO_LOGO_SWITCH);

	if (bLogo)
		{
		printf("DirectXFont v1.0\n");
		printf("Copyright (c) 2010 by George Moromisato. All Rights Reserved.\n\n");
		}

	if (pCmdLine->GetAttributeBool(NOARGS) || pCmdLine->GetAttributeBool(SWITCH_HELP))
		{
		printf("   /font:\"typeface font-size [bold] [italic]\"\n");
		printf("   /output:{output filespec}\n");
		printf("\n");
		printf("   OR\n\n");
		printf("   /input:{input filespec}\n");
		printf("\n");
		return;
		}

	//	If we've got an input file, then generate fonts from the input file

	CString sInputFilespec = pCmdLine->GetAttribute(INPUT_SWITCH);
	if (!sInputFilespec.IsBlank())
		{
		ProcessInputFile(sInputFilespec);
		return;
		}

	//	Create a font

	CG16bitFont TheFont;
	if (error = CreateFont(pCmdLine, &TheFont))
		return;

	//	Get the output filespec

	CString sOutputFilespec = pCmdLine->GetAttribute(OUTPUT_SWITCH);
	if (sOutputFilespec.IsBlank())
		sOutputFilespec = CONSTLIT("Font.dxfn");
	else
		sOutputFilespec = pathAddExtensionIfNecessary(sOutputFilespec, CONSTLIT(".dxfn"));

	//	Status

	if (bLogo)
		printf("Creating %s %d %s %s to file '%s'\n",
				TheFont.GetTypeface().GetASCIIZPointer(),
				TheFont.GetHeight(),
				(TheFont.IsBold() ? " bold" : ""),
				(TheFont.IsItalic() ? " italic" : ""),
				sOutputFilespec.GetASCIIZPointer());
	
	//	Create the output file

	CFileWriteStream Output(sOutputFilespec);
	if (error = Output.Create())
		{
		printf("DirectXFont: Unable to create file '%s'\n", sOutputFilespec.GetASCIIZPointer());
		return;
		}

	TheFont.WriteToStream(&Output);

	if (error = Output.Close())
		{
		printf("DirectXFont: Unable to close file '%s'\n", sOutputFilespec.GetASCIIZPointer());
		return;
		}
	}

ALERROR CreateFont (CXMLElement *pCmdLine, CG16bitFont *pFont)

//	CreateFont
//
//	Create a font based on command line parameters
//	The font parameter is of the form:
//
//	{typeface} {font-size} [bold] [italic]

	{
	ALERROR error;
	CString sFont = pCmdLine->GetAttribute(FONT_SWITCH);
	if (sFont.IsBlank())
		{
		printf("DirectXFont: Expected /font:{fontdesc} parameter.");
		return ERR_FAIL;
		}

	char *pPos = sFont.GetASCIIZPointer();
	CString sTypeface = CONSTLIT("Arial");
	int iSize = 10;
	bool bBold = false;
	bool bItalic = false;

	//	Skip whitespace

	while (*pPos == ' ' || *pPos == '\t')
		pPos++;

	//	Skip delimiters

	bool bSpaceDelimiter;
	if (*pPos == '\'' || *pPos == '\"')
		{
		bSpaceDelimiter = false;
		pPos++;
		}
	else
		bSpaceDelimiter = true;

	//	Look for typeface

	char *pStart = pPos;
	while (*pPos != '\'' && *pPos != '\"' && (!bSpaceDelimiter || *pPos != ' ') && *pPos != '\0')
		pPos++;

	sTypeface = CString(pStart, pPos - pStart);

	//	Look for size

	while ((*pPos < '0' || *pPos > '9') && *pPos != '\0')
		pPos++;

	if (*pPos == '\0')
		{
		printf("DirectXFont: Expected font size.");
		return ERR_FAIL;
		}

	iSize = strParseInt(pPos, -1, &pPos);
	if (iSize == -1)
		{
		printf("DirectXFont: Invalid font size.");
		return ERR_FAIL;
		}

	//	Skip whitespace

	while (*pPos == ' ' || *pPos == '\t')
		pPos++;

	//	Parse bold/italic

	pStart = pPos;
	bool bDone = (*pPos == '\0');
	while (!bDone)
		{
		if (*pPos == ' ' || *pPos == '\0')
			{
			CString sStyle = CString(pStart, pPos - pStart);
			if (strEquals(sStyle, CONSTLIT("bold")))
				bBold = true;
			else if (strEquals(sStyle, CONSTLIT("italic")))
				bItalic = true;

			//	Skip whitespace

			while (*pPos == ' ')
				pPos++;

			pStart = pPos;
			bDone = (*pPos == '\0');
			}
		else
			pPos++;
		}

	//	Create the font

	if (error = pFont->Create(sTypeface, iSize, bBold, bItalic))
		{
		printf("DirectXFont: Unable to create font.");
		return error;
		}

	return NOERROR;
	}

void ProcessInputFile (const CString &sInputFile)

//	ProcessInputFile
//
//	Generate fonts from input file

	{
	ALERROR error;
	int i;

	//	Parse the input file

	CFileReadBlock InputFile(sInputFile);
	if (error = InputFile.Open())
		{
		printf("DirectXFont: Unable to open file: '%s'\n", sInputFile.GetASCIIZPointer());
		return;
		}

	CXMLElement *pRoot;
	CString sError;
	if (error = CXMLElement::ParseXML(&InputFile, &pRoot, &sError))
		{
		printf("DirectXFont: %s\n", sError.GetASCIIZPointer());
		return;
		}

	//	Get the root path of the input file

	CString sPath = pathGetPath(sInputFile);

	//	Loop over all directives

	for (i = 0; i < pRoot->GetContentElementCount(); i++)
		{
		CXMLElement *pDesc = pRoot->GetContentElement(i);

		if (strEquals(pDesc->GetTag(), FONT_TAG))
			{
			CString sTypeface = pDesc->GetAttribute(TYPEFACE_ATTRIB);
			int iSize = pDesc->GetAttributeIntegerBounded(SIZE_ATTRIB, 0, -1, 0);
			bool bBold = pDesc->GetAttributeBool(BOLD_ATTRIB);
			bool bItalic = pDesc->GetAttributeBool(ITALIC_ATTRIB);
			CString sOutputFilespec = pDesc->GetAttribute(OUTPUT_ATTRIB);

			if (iSize == 0 || sTypeface.IsBlank() || sOutputFilespec.IsBlank())
				continue;
				
			//	Create the font

			CG16bitFont TheFont;
			if (error = TheFont.Create(sTypeface, iSize, bBold, bItalic))
				{
				printf("DirectXFont: Unable to create font %d\n", i);
				return;
				}

			//	Generate output filespec

			sOutputFilespec = pathAddComponent(sPath,
					pathAddExtensionIfNecessary(sOutputFilespec, CONSTLIT(".dxfn")));

			//	Log

			printf("%s: %s %dpx%s%s\n",
					sOutputFilespec.GetASCIIZPointer(),
					TheFont.GetTypeface().GetASCIIZPointer(),
					TheFont.GetHeight(),
					(TheFont.IsBold() ? " bold" : ""),
					(TheFont.IsItalic() ? " italic" : ""));

			//	Generate font file

			CFileWriteStream Output(sOutputFilespec);
			if (error = Output.Create())
				{
				printf("DirectXFont: Unable to create file '%s'\n", sOutputFilespec.GetASCIIZPointer());
				return;
				}

			TheFont.WriteToStream(&Output);

			if (error = Output.Close())
				{
				printf("DirectXFont: Unable to close file '%s'\n", sOutputFilespec.GetASCIIZPointer());
				return;
				}
			}
		}

	//	Done

	delete pRoot;
	}
