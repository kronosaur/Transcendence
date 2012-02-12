//	WikiData
//
//	Software for converting from WikiMedia format to other formats.
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

#define INPUT_SWITCH							CONSTLIT("input")
#define NOARGS									CONSTLIT("noArgs")
#define NO_LOGO_SWITCH							CONSTLIT("nologo")
#define OUTPUT_SWITCH							CONSTLIT("output")
#define SWITCH_HELP								CONSTLIT("help")

void AlchemyMain (CXMLElement *pCmdLine);

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
	{
	ALERROR error;
	bool bLogo = !pCmdLine->GetAttributeBool(NO_LOGO_SWITCH);

	if (bLogo)
		{
		printf("WikiData v1.0\n");
		printf("Copyright (c) 2011 by George Moromisato. All Rights Reserved.\n\n");
		}

	if (pCmdLine->GetAttributeBool(NOARGS) || pCmdLine->GetAttributeBool(SWITCH_HELP))
		{
		printf("   /input:{input filespec}\n");
		printf("   /output:{output filespec}\n");
		printf("\n");
		return;
		}

	//	Get the input file

	CString sInputFile = pCmdLine->GetAttribute(INPUT_SWITCH);
	if (sInputFile.IsBlank())
		{
		printf("WikiData: Input filespec expected.\n");
		return;
		}

	//	Parse the input file

	CFileReadBlock InputFile(sInputFile);
	if (error = InputFile.Open())
		{
		printf("WikiData: Unable to open file: '%s'\n", sInputFile.GetASCIIZPointer());
		return;
		}

	CXMLElement *pRoot;
	CString sError;
	if (error = CXMLElement::ParseXML(&InputFile, &pRoot, &sError))
		{
		printf("WikiData: %s\n", sError.GetASCIIZPointer());
		return;
		}

	//	Initialize wiki

	CWiki Wiki;
	if (error = Wiki.InitFromXML(pRoot, &sError))
		{
		printf("WikiData: %s\n", sError.GetASCIIZPointer());
		return;
		}

	//	Get the output file

	CString sOutputFile = pCmdLine->GetAttribute(OUTPUT_SWITCH);
	
	//	Figure out what to do

	if (sOutputFile.IsBlank())
		OutputPageList(Wiki, pCmdLine);
	else
		{
		}

	//	Done

	delete pRoot;
	}
