//	TLisp.cpp
//
//	TLisp interpreter.
//	Copyright (c) Kronosaur Productions, LLC. All Rights Reserved.

#include <stdio.h>

#include <windows.h>
#include "Alchemy.h"
#include "XMLUtil.h"
#include "TransData.h"

#define NO_LOGO_SWITCH						CONSTLIT("nologo")
#define RUN_SWITCH							CONSTLIT("run")
#define RUN_FILE_SWITCH						CONSTLIT("runFile")

void Run (CUniverse &Universe, CXMLElement *pCmdLine)
	{
	ALERROR error;
	int i;
	CCodeChain &CC = g_pUniverse->GetCC();
	bool bNoLogo = pCmdLine->GetAttributeBool(NO_LOGO_SWITCH);

	//	Prepare the universe

	CTopologyNode *pNode = g_pUniverse->GetFirstTopologyNode();
	if (pNode == NULL)
		{
		printf("ERROR: No topology node found.\n");
		return;
		}

	CSystem *pSystem;
	if (error = g_pUniverse->CreateStarSystem(pNode, &pSystem))
		{
		printf("ERROR: Unable to create star system.\n");
		return;
		}

	//	Set the POV

	CSpaceObject *pPOV = pSystem->GetObject(0);
	g_pUniverse->SetPOV(pPOV);
	pSystem->SetPOVLRS(pPOV);

	//	Prepare system

	g_pUniverse->UpdateExtended();
	g_pUniverse->GarbageCollectLibraryBitmaps();

	CString sCommand = pCmdLine->GetAttribute(RUN_SWITCH);
	CString sRunFile = pCmdLine->GetAttribute(RUN_FILE_SWITCH);

	//	If this is a run file, then we parse it and run it

	if (!sRunFile.IsBlank() && !strEquals(sRunFile, CONSTLIT("true")))
		{
		TArray<CString> Files;
		ParseStringList(sRunFile, PSL_FLAG_ALLOW_WHITESPACE, &Files);
		for (i = 0; i < Files.GetCount(); i++)
			RunFile(Files[i], bNoLogo);
		}

	//	If we have a command, invoke it

	else if (!sCommand.IsBlank() && !strEquals(sCommand, CONSTLIT("True")))
		{
		CCodeChainCtx Ctx;
		ICCItemPtr pCode = Ctx.LinkCode(sCommand);
		ICCItemPtr pResult = Ctx.RunCode(pCode);

		CString sOutput;
		if (pResult->IsIdentifier())
			sOutput = pResult->Print(&CC, PRFLAG_NO_QUOTES | PRFLAG_ENCODE_FOR_DISPLAY);
		else
			sOutput = CC.Unlink(pResult);

		//	Output result

		printf("%s\n", sOutput.GetASCIIZPointer());
		}

	//	Otherwise, we enter a command loop

	else
		{
		//	Welcome

		if (!bNoLogo)
			{
			printf("(help) for function help.\n");
			printf("\\q to quit.\n\n");
			}

		//	Loop

		while (true)
			{
			char szBuffer[1024];
			if (!bNoLogo)
				printf(": ");
			gets_s(szBuffer, sizeof(szBuffer)-1);
			CString sCommand(szBuffer);

			//	Escape codes

			if (*sCommand.GetASCIIZPointer() == '\\')
				{
				//	Quit command

				if (strStartsWith(sCommand, CONSTLIT("\\q")))
					break;
				else if (strStartsWith(sCommand, CONSTLIT("\\?"))
						|| strStartsWith(sCommand, CONSTLIT("\\h")))
					{
					printf("TLisp Shell\n\n");
					printf("\\h      Show this help\n");
					printf("\\q      Quit\n");

					printf("\n(help) for function help.\n");
					}
				}

			//	Null command

			else if (sCommand.IsBlank())
				NULL;

			//	Command

			else
				{
				CCodeChainCtx Ctx;
				ICCItemPtr pCode = Ctx.LinkCode(sCommand);
				ICCItemPtr pResult = Ctx.RunCode(pCode);

				CString sOutput;
				if (pResult->IsIdentifier())
					sOutput = pResult->Print(&CC, PRFLAG_NO_QUOTES | PRFLAG_ENCODE_FOR_DISPLAY);
				else
					sOutput = CC.Unlink(pResult);

				//	Output result

				printf("%s\n", sOutput.GetASCIIZPointer());
				}
			}
		}
	}

void RunFile (const CString &sFilespec, bool bNoLogo)
	{
	ALERROR error;
	CCodeChain &CC = g_pUniverse->GetCC();
	CCodeChainCtx Ctx;

	//	Verify the file

	CString sRunFile = sFilespec;
	if (!strEndsWith(sRunFile, CONSTLIT("."))
			&& pathGetExtension(sRunFile).IsBlank())
		sRunFile.Append(CONSTLIT(".tlisp"));

	//	Open the file

	CFileReadBlock InputFile(sRunFile);
	if (error = InputFile.Open())
		{
		printf("error : Unable to open file '%s'.\n", sRunFile.GetASCIIZPointer());
		return;
		}

	if (!bNoLogo)
		printf("%s\n", sRunFile.GetASCIIZPointer());

	//	Parse

	CString sInputFile(InputFile.GetPointer(0), InputFile.GetLength(), TRUE);
	CString sOutput;

	CCodeChain::SLinkOptions Options;

	while (true)
		{
		ICCItemPtr pCode = Ctx.LinkCode(sInputFile, Options);
		if (pCode->IsNil())
			break;
		else if (pCode->IsError())
			{
			printf("error : %s\n", pCode->GetStringValue().GetASCIIZPointer());
			return;
			}

		Options.iOffset += Options.iLinked;

		//	Execute

		ICCItemPtr pResult = Ctx.RunCode(pCode);

		//	Compose output

		if (pResult->IsIdentifier())
			sOutput = pResult->Print(&CC, PRFLAG_NO_QUOTES | PRFLAG_ENCODE_FOR_DISPLAY);
		else
			sOutput = CC.Unlink(pResult);
		}

	//	Output result

	printf("%s\n", sOutput.GetASCIIZPointer());
	}