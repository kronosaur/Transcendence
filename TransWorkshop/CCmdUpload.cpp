//	CCmdUpload.cpp
//
//	CCmdUpload class
//	Copyright (c) 2016 by Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

#define FILE_TYPE_TDB							CONSTLIT("tdb")

bool CCmdUpload::Execute (CSteamCtx &Ctx, CString *retsError)

//  Execute
//
//  Update the file

    {
    int i;
    CString sRoot = pathGetExecutablePath(NULL);

    //  Load all extensions at the location

    if (!m_Extensions.Init(sRoot, m_sFilespec, retsError))
        return false;

    //  Get the UNID.

    DWORD dwUNID;
    if (!m_Extensions.FindByFilespec(m_sFilespec, &dwUNID))
        {
        if (retsError) *retsError = strPatternSubst(CONSTLIT("Unable to find %s."), m_sFilespec);
        return false;
        }

    //  Figure out which files we need to upload

    TArray<CString> Files;
    if (!m_Extensions.CalcRequiredFiles(dwUNID, Files))
        {
        if (retsError) *retsError = CONSTLIT("Unable to calc required files.");
        return false;
        }

#ifdef DEBUG
    for (i = 0; i < Files.GetCount(); i++)
        printf("DEBUG: Upload %s\n", (LPSTR)Files[i]);
#endif

    return true;
    //  Crack open the TDB file

    CResourceDb RootTDB(m_sFilespec, true);

    PublishedFileId_t FileId;
    bool bMustSign;
    if (!Ctx.CreateItem(&FileId, &bMustSign, retsError))
        return false;

    printf("Created %x\n", (DWORD)FileId);
    if (bMustSign)
        printf("Must sign agreement.\n");

    return true;
    }

bool CCmdUpload::Init (CXMLElement *pCmdLine, CString *retsError)

//  Init
//
//  Initialize from command line.

    {
    m_sFilespec = pCmdLine->GetContentText(0);
    if (m_sFilespec.IsBlank())
        {
        *retsError = CONSTLIT("Expected TDB filespec to upload.");
        return false;
        }

    //  Path must exist

    if (!pathExists(m_sFilespec))
        {
        *retsError = strPatternSubst(CONSTLIT("File not found: %s."), m_sFilespec);
        return false;
        }

    //  Must be a TDB file

    if (!strEquals(pathGetExtension(m_sFilespec), FILE_TYPE_TDB))
        {
        *retsError = CONSTLIT("Sorry, only .tdb files are supported.\n");
        return false;
        }

    return true;
    }
