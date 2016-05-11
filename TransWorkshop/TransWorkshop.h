//  TransWorkshop.h
//
//  CTransWorkshop class
//  Copyright (c) 2016 by Kronosaur Productions, LLC. All Rights Reserved.

#pragma once

class CSteamCtx;

class ITransCommand
    {
    public:
        virtual ~ITransCommand (void) { }

        virtual bool Execute (CSteamCtx &Ctx, CString *retsError) = 0;
        virtual bool Init (CXMLElement *pCmdLine, CString *retsError) { return true; }
        virtual void PrintHelp (void) { }
    };

class CTransWorkshop
    {
    public:
        void Run (CXMLElement *pCmdLine);

    private:
        bool ParseCommandLine (CXMLElement *pCmdLine, ITransCommand **retpCmd, CString *retsError = NULL) const;
        void PrintLogo (CXMLElement *pCmdLine) const;
    };