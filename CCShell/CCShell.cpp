//	CCShell.cpp
//
//	CodeChain command shell
//	Copyright (c) 2010 by George Moromisato

#include "stdafx.h"

#define NO_LOGO_SWITCH						CONSTLIT("nologo")

#define ERR_INVALID_INPUT					CONSTLIT("Unable to parse input.")
#define ERR_UNABLE_TO_INIT_CODECHAIN		CONSTLIT("Unable to initialize CodeChain.")
#define ERR_UNABLE_TO_PARSE_COMMAND_LINE	CONSTLIT("Unable to parse command line.")
#define ERR_UNABLE_TO_REGISTER_PRIMITIVES	CONSTLIT("Unable to register primitives.")

void PrintError (const CString &sError);

bool g_bQuitSignal = false;

int AlchemyMain (CXMLElement *pCmdLine)

//	AlchemyMain
//
//	Main entry-point after kernel initialization

	{
	ALERROR error;
	bool bLogo = !pCmdLine->GetAttributeBool(NO_LOGO_SWITCH);

	if (bLogo)
		{
		printf("CCShell v1.0\n");
		printf("Copyright (c) 2010 by George Moromisato. All Rights Reserved.\n\n");
		}

	//	Initialize CodeChain environment

	CCodeChain CC;
	if (error = CC.Boot())
		{
		PrintError(ERR_UNABLE_TO_INIT_CODECHAIN);
		return 1;
		}

	//	Load some functions

	if (error = RegisterShellPrimitives(CC))
		{
		PrintError(ERR_UNABLE_TO_REGISTER_PRIMITIVES);
		return 1;
		}

	//	Prepare context

	SExecuteCtx Ctx;
	Ctx.bQuit = false;

	//	Interpreter loop

	while (!Ctx.bQuit)
		{
		//	Prompt

		printf(": ");

		//	Read a line

		CString sInput;
		char *pDest = sInput.GetWritePointer(65536);
		if (fgets(pDest, 65536, stdin) == NULL)
			{
			PrintError(ERR_INVALID_INPUT);
			continue;
			}
		sInput.Truncate(strlen(sInput.GetASCIIZPointer()));

		//	Parse the line

		ICCItem *pCode = CC.Link(sInput, 0, NULL);
		if (pCode->IsError())
			{
			printf("codechain: %s\n", pCode->Print(&CC).GetASCIIZPointer());
			pCode->Discard(&CC);
			continue;
			}

		//	Execute

		ICCItem *pResult = CC.TopLevel(pCode, &Ctx);

		//	Output

		if (pResult->IsError())
			printf("codechain: %s\n", pResult->Print(&CC).GetASCIIZPointer());
		else
			printf("%s\n", pResult->Print(&CC).GetASCIIZPointer());

		//	Done

		pCode->Discard(&CC);
		pResult->Discard(&CC);
		}

	return 0;
	}

int main (int argc, char *argv[ ], char *envp[ ])

//	main
//
//	main entry-point

	{
	int iResult;

	if (!kernelInit())
		{
		printf("ccshell: Unable to initialize Alchemy kernel.\n");
		return 1;
		}

	//	Do it

	{
	ALERROR error;
	CXMLElement *pCmdLine;
	if (error = CreateXMLElementFromCommandLine(argc, argv, &pCmdLine))
		{
		PrintError(ERR_UNABLE_TO_PARSE_COMMAND_LINE);
		return 1;
		}

	iResult = AlchemyMain(pCmdLine);

	delete pCmdLine;
	}

	//	Done

	kernelCleanUp();
	return iResult;
	}

void PrintError (const CString &sError)
	{
	printf("ccshell: %s\n", sError.GetASCIIZPointer());
	}

