//  CommandImpl.h
//
//  Implementation of commands
//  Copyright (c) 2016 by Kronosaur Productions, LLC. All Rights Reserved.

#pragma once

class CCmdHelp : public ITransCommand
    {
    public:
        CCmdHelp (ITransCommand *pCmd = NULL) :
                m_pCmd(pCmd)
            { }

        virtual ~CCmdHelp (void) override;

        virtual bool Execute (CSteamCtx &Ctx, CString *retsError) override;

    private:
        ITransCommand *m_pCmd;              //  Cmd to give help on (may be NULL)
    };

class CCmdUpload : public ITransCommand
    {
    public:
        virtual bool Execute (CSteamCtx &Ctx, CString *retsError) override;
        virtual bool Init (CXMLElement *pCmdLine, CString *retsError) override;

    private:
        CString m_sFilespec;
        CExtensionDirectory m_Extensions;
    };