//	CCmdUpload.cpp
//
//	CCmdUpload class
//	Copyright (c) 2016 by Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

#define FILE_TYPE_TDB							CONSTLIT("tdb")

#define UPLOAD_SWITCH							CONSTLIT("upload")

#define STR_TRUE								CONSTLIT("true")

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

    //  We only support certain UNIDs

    DWORD dwNamespace = (dwUNID & 0xf0000000);
    if (dwNamespace != 0xA0000000
            && dwNamespace != 0xB0000000
            && dwNamespace != 0xC0000000
            && dwNamespace != 0xD0000000
            && dwNamespace != 0xE0000000)
        {
        if (retsError) *retsError = strPatternSubst(CONSTLIT("Invalid UNID range for mod: %08x"), dwUNID);
        return false;
        }

    //  Get the FileId to upload to

    PublishedFileId_t FileId;
    bool bCreated;
    if (!Ctx.GetOrCreateItem(dwUNID, &FileId, &bCreated, retsError))
        return false;

    if (bCreated)
        printf("Created new Steam item for %08x. FileId = %lld\n", dwUNID, FileId);
    else
        printf("Found existing Steam item for %08x. FileId = %lld\n", dwUNID, FileId);

    //  Get information about the extension

    CExtensionDirectory::SExtensionInfo ExtensionInfo;
    if (!m_Extensions.GetExtensionInfo(dwUNID, ExtensionInfo))
        {
        if (retsError) *retsError = CONSTLIT("Unable to obtain info about extension.");
        return false;
        }

    //  Create a temporary directory to hold files. They need to be in a 
    //  directory to upload.

    CString sTempRoot;
    do
        {
        sTempRoot = pathAddComponent(pathGetTempPath(), strPatternSubst(CONSTLIT("%x_%x"), GetTickCount(), mathRandom(10000, 99999)));
        }
    while (pathExists(sTempRoot));

    CString sTempFiles = pathAddComponent(sTempRoot, CONSTLIT("files"));
    if (!pathCreate(sTempFiles))
        {
        if (retsError) *retsError = strPatternSubst(CONSTLIT("Unable to create temp path: %s."), sTempRoot);
        return false;
        }

    //  Copy the required files to the temp directory.

    for (i = 0; i < ExtensionInfo.Files.GetCount(); i++)
        {
        if (!fileCopy(ExtensionInfo.Files[i], pathAddComponent(sTempFiles, pathGetFilename(ExtensionInfo.Files[i]))))
            {
            pathDeleteAll(sTempRoot);
            if (retsError) *retsError = CONSTLIT("Unable to copy TDBs to temp path.");
            return false;
            }

        printf("Uploading %s\n", (LPSTR)ExtensionInfo.Files[i]);
        }

    //  Prepare an update structure

    UGCUpdateHandle_t Update = Ctx.UpdateItemStart(FileId);

    SteamUGC()->SetItemTitle(Update, ExtensionInfo.sName);

    if (!ExtensionInfo.sDesc.IsBlank())
        SteamUGC()->SetItemDescription(Update, ExtensionInfo.sDesc);

    //  If we're creating, set a tag for the type

    switch (ExtensionInfo.iType)
        {
        case extAdventure:
            SteamUGC()->AddItemKeyValueTag(Update, "type", "transcendenceAdventure");
            break;

        case extExtension:
            SteamUGC()->AddItemKeyValueTag(Update, "type", "transcendenceExpansion");
            break;

        case extLibrary:
            SteamUGC()->AddItemKeyValueTag(Update, "type", "transcendenceLibrary");
            break;

        default:
            SteamUGC()->AddItemKeyValueTag(Update, "type", "transcendenceExtension");
            break;
        }

    //  Set the files

    SteamUGC()->SetItemContent(Update, sTempFiles);

    //  Update

    printf("Uploading...");
    if (!Ctx.UpdateItem(Update, strPatternSubst(CONSTLIT("Update to version: %s"), ExtensionInfo.sVersion), retsError))
        {
        pathDeleteAll(sTempRoot);
        return false;
        }

    printf("\nUpdated %s (%08x) to %s.\n", (LPSTR)ExtensionInfo.sName, ExtensionInfo.dwUNID, (LPSTR)ExtensionInfo.sVersion);

    //  Done. Clean up

    pathDeleteAll(sTempRoot);
    return true;
    }

bool CCmdUpload::Init (CXMLElement *pCmdLine, CString *retsError)

//  Init
//
//  Initialize from command line.

    {
    m_sFilespec = pCmdLine->GetAttribute(UPLOAD_SWITCH);
    if (m_sFilespec.IsBlank() || strEquals(m_sFilespec, STR_TRUE))
        {
		m_sFilespec = pCmdLine->GetContentText(0);
		if (m_sFilespec.IsBlank())
			{
			*retsError = CONSTLIT("Expected TDB filespec to upload.");
			return false;
			}
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
