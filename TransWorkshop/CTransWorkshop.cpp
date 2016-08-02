//	CTransWorkshop.cpp
//
//	CTransWorkshop class
//	Copyright (c) 2016 by Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

#define NOARGS								CONSTLIT("noArgs")

#define H_SWITCH							CONSTLIT("h")
#define HELP_SWITCH							CONSTLIT("help")
#define QUESTION_MARK_SWITCH				CONSTLIT("?")
#define UPLOAD_SWITCH                       CONSTLIT("upload")

void CTransWorkshop::Run (CXMLElement *pCmdLine)

//  Run
//
//  Run the application

    {
    CString sError;

    PrintLogo(pCmdLine);

    //  Connect to Steam

    CSteamCtx Steam;
    if (!Steam.Connect(&sError))
        {
        printf("ERROR: %s\n", (LPSTR)sError);
        return;
        }

	printf("\n");

    //  Parse the command line

    ITransCommand *pCmd;
    if (!ParseCommandLine(pCmdLine, &pCmd, &sError))
        {
        printf("ERROR: %s\n", (LPSTR)sError);
        return;
        }

    //  Execute the command
    
    try
        {
        if (!pCmd->Execute(Steam, &sError))
            printf("ERROR: %s\n", (LPSTR)sError);
        }
    catch (...)
        {
        printf("ERROR: Crash executing command.\n");
        }

    //  Done

    delete pCmd;
    }

bool CTransWorkshop::ParseCommandLine (CXMLElement *pCmdLine, ITransCommand **retpCmd, CString *retsError) const

//  ParseCommandLine
//
//  Parses the command line and returns the command to execute. Returns FALSE if
//  parsing fails. If we return TRUE, the caller must free return resulting
//  command object.

    {
    //  Create the command based on the command line parameters

    ITransCommand *pCmd;
    if (pCmdLine->GetAttributeBool(NOARGS))
        pCmd = new CCmdHelp;
    else if (pCmdLine->FindAttribute(UPLOAD_SWITCH))
        pCmd = new CCmdUpload;
    else
        pCmd = NULL;

    //  If we have a help switch, then we apply it to the given command or just
    //  show a list of all commands.

    if (pCmd == NULL
			|| pCmdLine->GetAttributeBool(HELP_SWITCH))
        pCmd = new CCmdHelp(pCmd);

    //  Initialize

    if (!pCmd->Init(pCmdLine, retsError))
        {
        delete pCmd;
        return false;
        }

    //  Done

    *retpCmd = pCmd;
    return true;
    }

void CTransWorkshop::PrintLogo (CXMLElement *pCmdLine) const

//  PrintLogo
//
//  Prints the logo and version

    {
	SFileVersionInfo VersionInfo;
	if (fileGetVersionInfo(NULL_STR, &VersionInfo) != NOERROR)
		{
		printf("ERROR: Unable to get version info.\n");
		return;
		}

	printf("TransWorkshop %s\n", (LPSTR)VersionInfo.sProductVersion);
	printf("%s\n\n", (LPSTR)VersionInfo.sCopyright);
    }
