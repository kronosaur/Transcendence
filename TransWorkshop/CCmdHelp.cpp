//	CCmdHelp.cpp
//
//	CCmdHelp class
//	Copyright (c) 2016 by Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

CCmdHelp::~CCmdHelp (void)

//  CCmdHelp destructor

    {
    if (m_pCmd)
        delete m_pCmd;
    }

bool CCmdHelp::Execute (CSteamCtx &Ctx, CString *retsError)

//  Execute
//
//  Execute the command

    {
    //  If we have a command, ask the command to print its own help

    if (m_pCmd)
        m_pCmd->PrintHelp();

    //  Otherwise, we list all available command

    else
        {
		printf("TransWorkshop [options]\n\n");
		printf("   /upload {TDB file}    Upload TDB and dependencies.\n");
		printf("   /?                    Show help.\n");
        }

    return true;
    }
