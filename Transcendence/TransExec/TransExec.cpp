//	TransExec.cpp
//
//	TransExec is a utility for running TransLISP programs.
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

#define NO_LOGO_SWITCH						CONSTLIT("nologo")
#define RUN_SWITCH							CONSTLIT("run")

void AlchemyMain (CXMLElement *pCmdLine);
ALERROR InitCC (void);

CCodeChain g_CC;

int main (int argc, char *argv[ ], char *envp[ ])

//	main
//
//	main entry-point

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

	//	Load some command line options

	bool bLogo = !pCmdLine->GetAttributeBool(NO_LOGO_SWITCH);
	CString sFilespec = pCmdLine->GetAttribute(RUN_SWITCH);

	//	If no input file, just output help.

	if (sFilespec.IsBlank())
		{
		printf("TransExec v1.0\n");
		printf("Copyright (c) 2011 by George Moromisato. All Rights Reserved.\n\n");
		printf("TransExec /run:{filespec}\n");
		return;
		}

	//	Initialize CodeChain

	if (error = InitCC())
		return;

	//	Open the file

	CFileReadBlock InputFile(sFilespec);
	if (error = InputFile.Open())
		{
		printf("error : Unable to open file '%s'.\n", sFilespec.GetASCIIZPointer());
		g_CC.CleanUp();
		return;
		}

	//	Parse

	CString sInputFile(InputFile.GetPointer(0), InputFile.GetLength(), TRUE);
	ICCItem *pCode = g_CC.Link(sInputFile, 0);
	if (pCode->IsError())
		{
		printf("error : %s\n", pCode->GetStringValue().GetASCIIZPointer());
		pCode->Discard(&g_CC);
		g_CC.CleanUp();
		return;
		}

	if (bLogo)
		printf("%s\n", sFilespec.GetASCIIZPointer());

	//	Execute

	ICCItem *pReturn = g_CC.TopLevel(pCode, NULL);

	//	Output result

	printf("%s\n", g_CC.Unlink(pReturn).GetASCIIZPointer());
	
	//	Done

	pReturn->Discard(&g_CC);
	pCode->Discard(&g_CC);
	g_CC.CleanUp();
	}

ALERROR InitCC (void)

//	InitCC
//
//	Initialize CodeChain

	{
	ALERROR error;

	//	Boot

	if (error = g_CC.Boot())
		{
		printf("error : Unable to initialize CodeChain.\n");
		return error;
		}

	//	Load some libraries

	if (error = dbgLibraryInit(g_CC))
		{
		printf("error : Unable to initialize debug library.\n");
		return error;
		}

	if (error = pageLibraryInit(g_CC))
		{
		printf("error : Unable to initialize page library.\n");
		return error;
		}

	return NOERROR;
	}
