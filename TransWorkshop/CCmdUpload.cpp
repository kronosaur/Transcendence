//	CCmdUpload.cpp
//
//	CCmdUpload class
//	Copyright (c) 2016 by Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

#define FILE_TYPE_TDB							CONSTLIT("tdb")

#define LIBRARIES_SWITCH						CONSTLIT("libraries")
#define UPLOAD_SWITCH							CONSTLIT("upload")

#define STR_TRUE								CONSTLIT("true")

//	The center of an extension cover image is at this position relative to the
//	right edge of the image.

const int RIGHT_COVER_OFFSET =					256 + 160;

bool CCmdUpload::Execute (CSteamCtx &Ctx, CString *retsError)

//  Execute
//
//  Update the file

    {
    CString sRoot = pathGetExecutablePath(NULL);

    //  Load all extensions at the location

    if (!m_Extensions.Init(sRoot, m_sFilespec, retsError))
        return false;

    //  Get the UNID.

    DWORD dwUNID;
	if (!ValidateExtension(m_sFilespec, &dwUNID, retsError))
		return false;

    //  Get information about the extension

    CExtensionDirectory::SExtensionInfo ExtensionInfo;
    if (!m_Extensions.GetExtensionInfo(dwUNID, ExtensionInfo, retsError))
        return false;

    //  Create a temporary directories to hold files. They need to be in a 
    //  directory to upload.

	SUploadPaths Paths;
	if (!CreateUploadPaths(&Paths, retsError))
		return false;

	//	Copy the required content files to the proper directory

	if (!CopyContentFiles(ExtensionInfo, Paths, retsError))
		return false;

	//	Copy the preview image file to the proper directory

	CString sPreviewFilespec;
	if (!CopyPreviewFiles(ExtensionInfo, Paths, &sPreviewFilespec, retsError))
		return false;

    //  Get the FileId to upload to

    PublishedFileId_t FileId;
    bool bCreated;
    if (!Ctx.GetOrCreateItem(dwUNID, ExtensionInfo.iType, &FileId, &bCreated, retsError))
        return false;

    if (bCreated)
        printf("Created new Steam item for %08x. FileId = %lld\n", dwUNID, FileId);
    else
        printf("Found existing Steam item for %08x. FileId = %lld\n", dwUNID, FileId);

    //  Prepare an update structure

    UGCUpdateHandle_t Update = Ctx.UpdateItemStart(FileId);

    SteamUGC()->SetItemTitle(Update, ExtensionInfo.sName);

    if (!ExtensionInfo.sDesc.IsBlank())
        SteamUGC()->SetItemDescription(Update, ExtensionInfo.sDesc);

	//	Set image file

	if (!sPreviewFilespec.IsBlank())
		SteamUGC()->SetItemPreview(Update, sPreviewFilespec);

    //  Set the files

    SteamUGC()->SetItemContent(Update, Paths.sContentPath);

    //  Update

    printf("Uploading...");
    if (!Ctx.UpdateItem(Update, strPatternSubst(CONSTLIT("Update to version: %s"), ExtensionInfo.sVersion), retsError))
        return false;

    printf("\nUpdated %s (%08x) to %s.\n", (LPSTR)ExtensionInfo.sName, ExtensionInfo.dwUNID, (LPSTR)ExtensionInfo.sVersion);

    //  Done.

    return true;
    }

bool CCmdUpload::CopyContentFile (const CString &sFilespec, const SUploadPaths &Paths, CString *retsError) const

//	CopyContentFile
//
//	Copies the given file to the content folder.

	{
	printf("Uploading %s.\n", (LPSTR)sFilespec);

	CString sDest = pathAddComponent(Paths.sContentPath, pathGetFilename(sFilespec));
    if (!fileCopy(sFilespec, sDest))
        {
        if (retsError) *retsError = strPatternSubst(CONSTLIT("Unable to copy file to: %s."), sDest);
        return false;
        }

	return true;
	}

bool CCmdUpload::CopyContentFiles (const CExtensionDirectory::SExtensionInfo &Info, const SUploadPaths &Paths, CString *retsError)

//	CopyContentFiles
//
//	Determines which content files we need to copy and copies them.

	{
	int i;

	//	Start by copying the root extension file

	if (!CopyContentFile(Info.sFilespec, Paths, retsError))
		return false;

    //  If we're uploading all required libraries, then get the info from the
	//	extension.

	if (m_bUploadLibraries)
		{
		for (i = 0; i < Info.Files.GetCount(); i++)
			{
			if (!CopyContentFile(Info.Files[i], Paths, retsError))
				return false;
			}
		}

	//	Otherwise, we upload what the user specified

	else
		{
		for (i = 0; i < m_Libraries.GetCount(); i++)
			{
			DWORD dwUNID;
			if (!ValidateExtension(m_Libraries[i], &dwUNID, retsError))
				return false;

			//	Upload

			if (!CopyContentFile(m_Libraries[i], Paths, retsError))
				return false;
			}
		}

	return true;
	}

bool CCmdUpload::CopyPreviewFile (const CString &sData, const CString &sDestFilespec, CString *retsError) const

//	CopyPreviewFile
//
//	Writes out a preview file.

	{
	printf("Uploading %s.\n", (LPSTR)pathGetFilename(sDestFilespec));

	CFileWriteStream theFile(sDestFilespec, FALSE);
	if (theFile.Create() != NOERROR)
		{
		*retsError = strPatternSubst(CONSTLIT("Unable to create file: %s."), sDestFilespec);
		return false;
		}

	if (theFile.Write(sData.GetASCIIZPointer(), sData.GetLength()) != NOERROR)
		{
		*retsError = strPatternSubst(CONSTLIT("Unable to write to file: %s."), sDestFilespec);
		return false;
		}

	if (theFile.Close() != NOERROR)
		{
		*retsError = strPatternSubst(CONSTLIT("Unable to close file: %s."), sDestFilespec);
		return false;
		}

	return true;
	}

bool CCmdUpload::CopyPreviewFiles (const CExtensionDirectory::SExtensionInfo &Info, const SUploadPaths &Paths, CString *retsPath, CString *retsError)

//	CopyPreviewFiles
//
//	Copy preview files to the proper directory

	{
	//	Initialize

	*retsPath = NULL_STR;

	//	If no image, then path is empty

	if (Info.sCoverImage.IsBlank())
		return true;

	//	Open the TDB

	CResourceDb Resources(Info.sFilespec);
    if (Resources.Open(DFOPEN_FLAG_READ_ONLY, retsError) != NOERROR)
        return false;

	//	Load the resource
	//	LATER: Convert to LoadImageFile (so we can handle PNGs).

	HBITMAP hImage;
	if (Resources.LoadImage(NULL_STR, Info.sCoverImage, &hImage) != NOERROR)
		{
		*retsError = strPatternSubst(CONSTLIT("Unable to load image file in TDB: %s."), Info.sFilespec);
		return false;
		}

	//	If we've got a 16x9 image, then just copy it out unmodified

	CString sData;
	if (dibGetAspectRatio(hImage) == aspect16x9)
		{
		if (Resources.ExtractResource(Info.sCoverImage, &sData) != NOERROR)
			{
			::DeleteObject(hImage);
			*retsError = strPatternSubst(CONSTLIT("Unable to find image file in TDB: %s."), Info.sFilespec);
			return false;
			}
		}

	//	Otherwise, we need to crop the image

	else
		{
		int cxWidth;
		int cyHeight;
		if (dibGetInfo(hImage, &cxWidth, &cyHeight) != NOERROR)
			{
			::DeleteObject(hImage);
			*retsError = strPatternSubst(CONSTLIT("Unable to get DIB info: %s."), Info.sFilespec);
			return false;
			}

		//	If wider than 16x9, crop the left hand side out.

		HBITMAP hCropped;
		if (9 * cxWidth > 16 * cyHeight)
			{
			int cxNewWidth = 16 * cyHeight / 9;

			int xSrc = Min(cxWidth - cxNewWidth, cxWidth - (RIGHT_COVER_OFFSET + (cxNewWidth / 2)));

			if (dibCrop(hImage, xSrc, 0, cxNewWidth, cyHeight, &hCropped) != NOERROR)
				{
				::DeleteObject(hImage);
				*retsError = strPatternSubst(CONSTLIT("Unable to crop DIB: %s."), Info.sFilespec);
				return false;
				}
			}

		//	Otherwise, crop up and down

		else
			{
			int cyNewHeight = 9 * cxWidth / 16;

			if (dibCrop(hImage, 0, (cyHeight - cyNewHeight) / 2, cxWidth, cyNewHeight, &hCropped) != NOERROR)
				{
				::DeleteObject(hImage);
				*retsError = strPatternSubst(CONSTLIT("Unable to crop DIB: %s."), Info.sFilespec);
				return false;
				}
			}

		//	Write out as a JPEG stream

		if (JPEGSaveToMemory(hCropped, 75, &sData) != NOERROR)
			{
			::DeleteObject(hImage);
			*retsError = strPatternSubst(CONSTLIT("Unable to create JPEG: %s."), Info.sFilespec);
			return false;
			}
		}

	//	Done with the image.

	::DeleteObject(hImage);

	//	Save the data to a file

	CString sOutFilespec = pathAddComponent(Paths.sPreviewPath, pathGetFilename(Info.sCoverImage));
	if (!CopyPreviewFile(sData, sOutFilespec, retsError))
		return false;

	//	Return the image path

	*retsPath = sOutFilespec;
	
	//	Done

	return true;
	}

bool CCmdUpload::CreateUploadPaths (SUploadPaths *retPaths, CString *retsError) const

//	CreateTempPath
//
//	Creates a temporary path.

	{
    //  Create a temporary directory to hold files. They need to be in a 
    //  directory to upload.

    do
        {
        retPaths->sRoot = pathAddComponent(pathGetTempPath(), strPatternSubst(CONSTLIT("%x_%x"), GetTickCount(), mathRandom(10000, 99999)));
        }
    while (pathExists(retPaths->sRoot));

    retPaths->sContentPath = pathAddComponent(retPaths->sRoot, CONSTLIT("files"));
    if (!pathCreate(retPaths->sContentPath))
        {
		pathDeleteAll(retPaths->sRoot);
        if (retsError) *retsError = strPatternSubst(CONSTLIT("Unable to create temp path: %s."), retPaths->sRoot);
        return false;
        }

	retPaths->sPreviewPath = pathAddComponent(retPaths->sRoot, CONSTLIT("preview"));
	if (!pathCreate(retPaths->sPreviewPath))
		{
		pathDeleteAll(retPaths->sRoot);
        if (retsError) *retsError = strPatternSubst(CONSTLIT("Unable to create temp path: %s."), retPaths->sRoot);
        return false;
		}

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

	//	Do we upload libraries?

	CString sLibraries = pCmdLine->GetAttribute(LIBRARIES_SWITCH);
	if (sLibraries.IsBlank() || strEquals(sLibraries, STR_TRUE))
		m_bUploadLibraries = true;
	else
		{
		m_bUploadLibraries = false;

		if (strDelimitEx(sLibraries, ',', DELIMIT_TRIM_WHITESPACE, 0, &m_Libraries) != NOERROR)
			{
			*retsError = CONSTLIT("Unable to parse list of libraries.");
			return false;
			}
		}

    return true;
    }

void CCmdUpload::PrintHelp (void)

//	PrintHelp
//
//	Help

	{
	printf("TransWorkshop /upload {TDB file}\n\n");
	printf("   /libraries             Include all required libraries in upload.\n");
	printf("   /libraries:{file list} Include only these libraries in upload.\n");
	}

bool CCmdUpload::ValidateExtension (const CString &sFilespec, DWORD *retdwUNID, CString *retsError) const

//	ValidateExtension
//
//	Looks for the given extension in our list and returns the UNID. Return TRUE
//	if this is a valid extension. FALSE otherwise.

	{
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

	//	Done

	if (retdwUNID)
		*retdwUNID = dwUNID;

	return true;
	}
