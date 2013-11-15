//	Path.cpp
//
//	Path routines

#include "Kernel.h"
#include "KernelObjID.h"
#include "shlobj.h"

#define STR_PATH_SEPARATOR				CONSTLIT("\\")

#define STR_COMPANY_NAME				CONSTLIT("CompanyName")
#define STR_COPYRIGHT					CONSTLIT("LegalCopyright")
#define STR_PRODUCT_NAME				CONSTLIT("ProductName")
#define STR_PRODUCT_VERSION				CONSTLIT("ProductVersion")

#define STR_DOT							CONSTLIT(".")
#define STR_DOT_DOT						CONSTLIT("..")
#define STR_STAR						CONSTLIT("*")

void FreePIDL (LPITEMIDLIST pidl);
CString GetVersionString (char *pData, WORD *pLangInfo, const CString &sString);

bool fileCopy (const CString &sSourceFilespec, const CString &sDestFilespec)

//	fileMove
//
//	Moves a file

	{
	if (!::CopyFile(sSourceFilespec.GetASCIIZPointer(), sDestFilespec.GetASCIIZPointer(), FALSE))
		return false;

	return true;
	}

bool fileDelete (const CString &sFilespec, bool bRecycle)

//	fileDelete
//
//	Delete the file permanently from disk.
//	NOTE: Path must be absolute.

	{
	//	If we're recycling, send to recycle bin

	if (bRecycle)
		{
		SHFILEOPSTRUCT FileOp;
		::ZeroMemory(&FileOp, sizeof(FileOp));
		FileOp.hwnd = NULL;
		FileOp.wFunc = FO_DELETE;

		CString sFrom = strPatternSubst(CONSTLIT("%s\0"), pathMakeAbsolute(sFilespec));
		FileOp.pFrom = sFrom.GetASCIIZPointer();
		FileOp.pTo = NULL;
		FileOp.fFlags = FOF_ALLOWUNDO | FOF_NO_UI;

		int iResult = ::SHFileOperation(&FileOp);
		return (iResult == 0 && !FileOp.fAnyOperationsAborted);
		}

	//	Otherwise, just delete

	else
		{
		if (!::DeleteFile(sFilespec.GetASCIIZPointer()))
			return false;
		}

	return true;
	}

bool fileGetFileList (const CString &sRoot, const CString &sPath, const CString &sSearch, DWORD dwFlags, TArray<CString> *retFiles)

//	fileGetFileList
//
//	Returns a list of filespecs that match the given filespec

	{
	WIN32_FIND_DATA FileInfo;

	CString sFullPath = pathAddComponent(sRoot, sPath);
	CString sFilespec = pathAddComponent(sFullPath, sSearch);

	HANDLE hFind = ::FindFirstFile(sFilespec.GetASCIIZPointer(), &FileInfo);
	if (hFind == INVALID_HANDLE_VALUE)
		{
		//	If ERROR_FILE_NOT_FOUND then there are no files that match

		if (::GetLastError() == ERROR_FILE_NOT_FOUND)
			return true;

		//	Otherwise this is an error of some sort

		else
			return false;
		}

	bool bDirectoriesOnly = ((dwFlags & FFL_FLAG_DIRECTORIES_ONLY) ? true : false);

	do
		{
		CString sFound(FileInfo.cFileName, -1);

		//	Skip . and ..

		if (strEquals(sFound, STR_DOT) || strEquals(sFound, STR_DOT_DOT))
			continue;

		//	Skip system and hidden files

		if ((FileInfo.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)
				|| (FileInfo.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM))
			continue;

		//	Is this a directory?

		bool bIsDirectory = ((FileInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? true : false);

		//	Add to the list (but only if not a directory)

		if (bDirectoriesOnly == bIsDirectory)
			{
			if (dwFlags & FFL_FLAG_RELATIVE_FILESPEC)
				retFiles->Insert(pathAddComponent(sPath, sFound));
			else
				retFiles->Insert(pathAddComponent(sFullPath, sFound));
			}

		//	Recurse, if necessary

		if (bIsDirectory && (dwFlags & FFL_FLAG_RECURSIVE))
			{
			if (!fileGetFileList(sRoot, pathAddComponent(sPath, sFound), sSearch, dwFlags, retFiles))
				return false;
			}
		}
	while (::FindNextFile(hFind, &FileInfo));

	//	Done

	::FindClose(hFind);
	return true;
	}

CTimeDate fileGetModifiedTime (const CString &sFilespec)

//	fileGetModifiedTime
//
//	Returns the modified time

	{
	HANDLE hFile = ::CreateFile(sFilespec.GetASCIIZPointer(),
			GENERIC_READ,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return CTimeDate();

	//	Get modified time

	FILETIME ftWrite;
	if (!::GetFileTime(hFile, NULL, NULL, &ftWrite))
		{
		::CloseHandle(hFile);
		return CTimeDate();
		}

	//	Convert

	SYSTEMTIME SystemTime;
	::FileTimeToSystemTime(&ftWrite, &SystemTime);

	//	Done

	::CloseHandle(hFile);
	return CTimeDate(SystemTime);
	}

DWORD fileGetProductVersion (void)

//	fileGetProductVersion
//
//	Returns the version encoded as a DWORD.
//
//	NOTE: This is not very accurate, since it tries to pack a DWORDLONG into
//	a DWORD. Do not use unless you know that your version numbers fit.

	{
	SFileVersionInfo VerInfo;
	fileGetVersionInfo(NULL_STR, &VerInfo);

	BYTE b1 = (BYTE)HIWORD((DWORD)(VerInfo.dwProductVersion >> 32));
	BYTE b2 = (BYTE)LOWORD((DWORD)(VerInfo.dwProductVersion >> 32));
	BYTE b3 = (BYTE)HIWORD((DWORD)VerInfo.dwProductVersion);
	BYTE b4 = (BYTE)LOWORD((DWORD)VerInfo.dwProductVersion);

	return MAKELONG(MAKEWORD(b4, b3), MAKEWORD(b2, b1));
	}

ALERROR fileGetVersionInfo (const CString &sFilename, SFileVersionInfo *retInfo)

//	fileGetVersionInfo
//
//	Returns version information for the file (if sFilename is NULL_STRING then
//	we return information for the current module.)

	{
	CString sPath = sFilename;
	if (sPath.IsBlank())
		{
		char szBuffer[1024];
		int iLen = ::GetModuleFileName(NULL, szBuffer, sizeof(szBuffer)-1);
		sPath = CString(szBuffer, iLen);
		}

	//	Initialize

	retInfo->dwFileVersion = 0;
	retInfo->dwProductVersion = 0;

	//	Figure out how big the version information is

	DWORD dwDummy;
	DWORD dwSize = ::GetFileVersionInfoSize(sPath.GetASCIIZPointer(), &dwDummy);
	if (dwSize == 0)
		return ERR_FAIL;

	//	Load the info

	CString sData;
	char *pData = sData.GetWritePointer(dwSize);
	if (!::GetFileVersionInfo(sPath.GetASCIIZPointer(), 0, dwSize, pData))
		return ERR_FAIL;

	//	Get the fixed-size portion

	VS_FIXEDFILEINFO *pFileInfo;
	DWORD dwFileInfoSize;
	if (::VerQueryValue(pData, "\\", (LPVOID *)&pFileInfo, (PUINT)&dwFileInfoSize))
		{
		retInfo->dwFileVersion = ((ULONG64)pFileInfo->dwFileDateMS << 32) | (ULONG64)pFileInfo->dwFileDateLS;
		retInfo->dwProductVersion = ((ULONG64)pFileInfo->dwProductVersionMS << 32) | (ULONG64)pFileInfo->dwProductVersionLS;
		}

	//	Get language information

	WORD *pLangInfo;
	DWORD dwLangInfoSize;
	::VerQueryValue(pData, "\\VarFileInfo\\Translation", (LPVOID *)&pLangInfo, (PUINT)&dwLangInfoSize);

	//	Get the strings

	retInfo->sCompanyName = GetVersionString(pData, pLangInfo, STR_COMPANY_NAME);
	retInfo->sCopyright = GetVersionString(pData, pLangInfo, STR_COPYRIGHT);
	retInfo->sProductName = GetVersionString(pData, pLangInfo, STR_PRODUCT_NAME);
	retInfo->sProductVersion = GetVersionString(pData, pLangInfo, STR_PRODUCT_VERSION);

	return NOERROR;
	}

bool fileMove (const CString &sSourceFilespec, const CString &sDestFilespec)

//	fileMove
//
//	Moves a file

	{
	if (!::MoveFile(sSourceFilespec.GetASCIIZPointer(), sDestFilespec.GetASCIIZPointer()))
		return false;

	return true;
	}

bool fileOpen (const CString &sFile, const CString &sParameters, const CString &sCurrentFolder, CString *retsError)

//	fileOpen
//
//	Launches the current file.

	{
	int iResult = (int)::ShellExecute(NULL,
			NULL,
			pathMakeAbsolute(sFile.GetASCIIZPointer()).GetASCIIZPointer(),
			(!sParameters.IsBlank() ? sParameters.GetASCIIZPointer() : NULL),
			(!sCurrentFolder.IsBlank() ? pathMakeAbsolute(sCurrentFolder).GetASCIIZPointer() : NULL),
			SW_SHOWDEFAULT);
	if (iResult <= 32)
		{
		if (retsError) *retsError = strPatternSubst(CONSTLIT("Unable to launch program: %s; error = %d"), sFile, iResult);
		return false;
		}

	return true;
	}

CString pathAddComponent (const CString &sPath, const CString &sComponent)

//	pathAddComponent
//
//	Concatenates the given component to the given path and returns
//	the result
//
//	sPath: full pathname to a directory (e.g. "c:\", "\\lawrence\cdrom", "d:\test")
//	sComponent: directory, filename, or wildcard.

	{
	if (sPath.IsBlank())
		return sComponent;
	else if (sComponent.IsBlank())
		return sPath;
	else
		{
		CString sResult = sPath;
		int iPathLength = sResult.GetLength();
		char *pString = sResult.GetPointer();

		//	If the path name does not have a trailing backslash, add one

		if (!sPath.IsBlank() && !pathIsPathSeparator(pString + iPathLength - 1))
			sResult.Append(LITERAL("\\"));

		//	Now concatenate the component

		sResult.Append(sComponent);

		return sResult;
		}
	}

CString pathAddExtensionIfNecessary (const CString &sPath, const CString &sExtension)

//	pathAddExtensionIfNecessary
//
//	If the path has not extension and if it doesn't end in '.' then add
//	the extension

	{
	int iLength = sPath.GetLength();
	char *pStart = sPath.GetASCIIZPointer();
	char *pPos = pStart + iLength;

	while (pPos >= pStart)
		{
		if (*pPos-- == '.')
			return sPath;
		}

	if (*sExtension.GetASCIIZPointer() == '.')
		return strPatternSubst(CONSTLIT("%s%s"), sPath, sExtension);
	else
		return strPatternSubst(CONSTLIT("%s.%s"), sPath, sExtension);
	}

bool pathCreate (const CString &sPath)

//	pathCreate
//
//	Makes sure that the given path exists. Creates all intermediate folders.
//	Returns TRUE if successful.

	{
	CString sTest(sPath.GetASCIIZPointer(), sPath.GetLength());
	char *pPos = sTest.GetASCIIZPointer();

	//	Make sure the path exists from the top down.

	while (*pPos != '\0')
		{
		//	Skip over this backslash

		while (pathIsPathSeparator(pPos))
			pPos++;

		//	Skip to the next backslash

		while (!pathIsPathSeparator(pPos) && *pPos != '\0')
			pPos++;

		//	Trim the path here and see if it exists so far

		char chSaved = *pPos;
		*pPos = '\0';
		
		//	If the path doesn't exist, create it.

		if (!pathExists(sTest))
			{
			if (!::CreateDirectory(sTest.GetASCIIZPointer(), NULL))
				return false;
			}

		*pPos = chSaved;
		}

	return true;
	}

CString pathGetExecutablePath (HINSTANCE hInstance)

//	pathGetExecutablePath
//
//	Returns the path of the given executable. This is the path
//	(without the filename) of the executable (e.g., c:\bin\windows)

	{
	char szBuffer[1024];
	int iLen;
	char *pPos;
	CString sPath;

	//	Get the path

	iLen = GetModuleFileName(hInstance, szBuffer, sizeof(szBuffer)-1);

	//	Skip backwards to the first backslash

	pPos = szBuffer + iLen;
	while (!pathIsPathSeparator(pPos) && pPos != szBuffer)
		pPos--;

	*pPos = '\0';

	//	Create string

	sPath.Transcribe(szBuffer, -1);
	return sPath;
	}

bool pathExists (const CString &sPath)

//	pathExists
//
//	Returns TRUE if the given path exists

	{
	DWORD dwResult = ::GetFileAttributes(sPath.GetASCIIZPointer());
	return (dwResult != 0xffffffff);
	}

CString pathGetExtension (const CString &sPath)

//	pathGetExtension
//
//	Returns the extension (without dot)

	{
	char *pPos;
	int iLength;

	pPos = sPath.GetASCIIZPointer();
	iLength = sPath.GetLength();

	//	Look for the extension

	while (iLength > 0)
		{
		if (pPos[iLength] == '.')
			break;
		iLength--;
		}

	//	Return the extension

	if (iLength == 0)
		return NULL_STR;
	else
		return strSubString(sPath, iLength + 1, -1);
	}

CString pathGetFilename (const CString &sPath)

//	pathGetFilename
//
//	Returns the filename (without the path)

	{
	char *pStart = sPath.GetASCIIZPointer();
	char *pPos = pStart + sPath.GetLength() + 1;

	//	Look for the first backslash

	while (pPos > pStart && !pathIsPathSeparator(pPos - 1))
		pPos--;

	return CString(pPos);
	}

CString pathGetPath (const CString &sPath)

//	pathGetPath
//
//	Returns the path without the filename

	{
	char *pStart = sPath.GetASCIIZPointer();
	char *pPos = pStart + sPath.GetLength() + 1;

	//	Look for the first backslash

	while (pPos > pStart && !pathIsPathSeparator(pPos - 1))
		pPos--;

	return CString(pStart, pPos - pStart);
	}

CString pathGetResourcePath (char *pszResID)

//	pathGetResourcePath
//
//	Converts a resource identifier into a path-like string
//	of the form:
//
//	resID:\@id
//
//	OR
//
//	resID:\{resID}

	{
	if ((DWORD)pszResID < 65536)
		return strPatternSubst(CONSTLIT("resID:\\@%d"), (DWORD)pszResID);
	else
		return strPatternSubst(CONSTLIT("resID:\\%s"), CString(pszResID));
	}

CString pathGetSpecialFolder (ESpecialFolders iFolder)

//	pathGetSpecialFolder
//
//	Returns the path of various user folders on the machine

	{
	//	Figure out the CSIDL

	int iCSIDL;
	switch (iFolder)
		{
		case folderAppData:
			iCSIDL = CSIDL_APPDATA;
			break;

		case folderDocuments:
			iCSIDL = CSIDL_PERSONAL;
			break;

		case folderPictures:
			iCSIDL = CSIDL_MYPICTURES;
			break;

		case folderMusic:
			iCSIDL = CSIDL_MYMUSIC;
			break;

		default:
			ASSERT(false);
			return NULL_STR;
		}

	//	Get the path

	CString sPath;
	char *pDest = sPath.GetWritePointer(MAX_PATH);
	HRESULT hr = ::SHGetFolderPath(NULL, iCSIDL, NULL, SHGFP_TYPE_CURRENT, pDest);
	if (hr != S_OK)
		return NULL_STR;

	//	Truncate to the correct size

	sPath.Truncate(lstrlen(pDest));

	//	Done

	return sPath;
	}

bool pathIsAbsolute (const CString &sPath)

//	pathIsAbsolute
//
//	Returns TRUE if the path is absolute

	{
	char *pPos = sPath.GetASCIIZPointer();

	//	A double back-slash means this is an absolute network path

	if (*pPos == '\\')
		{
		pPos++;
		return (*pPos == '\\');
		}

	//	A drive letter means this is absolute

	else if ((*pPos >= 'a' && *pPos <= 'z') || (*pPos >= 'A' && *pPos <= 'Z'))
		{
		pPos++;
		return (*pPos == ':');
		}

	//	Otherwise, relative

	else
		return false;
	}

bool pathIsFolder (const CString &sFilespec)

//	pathIsFolder
//
//	Returns TRUE if filespec is a folder.

	{
	DWORD dwResult = ::GetFileAttributes(sFilespec.GetASCIIZPointer());
	return (dwResult != 0xffffffff
			&& (dwResult & FILE_ATTRIBUTE_DIRECTORY));
	}

bool pathIsResourcePath (const CString &sPath, char **retpszResID)

//	pathIsResourcePath
//
//	If this path is a resource path (see pathGetResourcePath) then
//	we return TRUE and retpszResID is initialized with a pointer to
//	the identifier (this may be a pointer into sPath)

	{
	if (strStartsWith(sPath, CONSTLIT("resID:\\")))
		{
		char *pPos = sPath.GetASCIIZPointer() + 7;
		if (*pPos == '@')
			*retpszResID = MAKEINTRESOURCE(strParseInt(pPos + 1, 0));
		else
			*retpszResID = pPos;

		return true;
		}
	else
		return false;
	}

bool pathIsWritable (const CString &sFilespec)

//	pathIsWritable
//
//	If sFilespec is a directory, we try to create a temporary file. If it
//	succeeds, then we're writable. If sFilespec is an existing file, we try
//	to open it in write mode.

	{
	if (!pathExists(sFilespec))
		{
		HANDLE hFile = ::CreateFile(sFilespec.GetASCIIZPointer(),
					GENERIC_READ | GENERIC_WRITE,
					FILE_SHARE_READ,
					NULL,
					CREATE_ALWAYS,
					FILE_ATTRIBUTE_NORMAL,
					NULL);
		if (hFile == INVALID_HANDLE_VALUE)
			return false;

		::CloseHandle(hFile);
		fileDelete(sFilespec);
		return true;
		}
	else if (pathIsFolder(sFilespec))
		{
		CString sTestFile = pathAddComponent(sFilespec, CONSTLIT("~temp.txt"));
		HANDLE hFile = ::CreateFile(sTestFile.GetASCIIZPointer(),
					GENERIC_READ | GENERIC_WRITE,
					FILE_SHARE_READ,
					NULL,
					CREATE_ALWAYS,
					FILE_ATTRIBUTE_NORMAL,
					NULL);
		if (hFile == INVALID_HANDLE_VALUE)
			return false;

		::CloseHandle(hFile);
		fileDelete(sTestFile);
		return true;
		}
	else
		{
		HANDLE hFile = ::CreateFile(sFilespec.GetASCIIZPointer(),
					GENERIC_READ | GENERIC_WRITE,
					FILE_SHARE_READ,
					NULL,
					OPEN_ALWAYS,
					FILE_ATTRIBUTE_NORMAL,
					NULL);
		if (hFile == INVALID_HANDLE_VALUE)
			return false;

		::CloseHandle(hFile);
		return true;
		}
	}

CString pathMakeAbsolute (const CString &sPath, const CString &sRoot)

//	pathMakeAbsolute
//
//	Converts a relative path to an absolute path. If the path is already 
//	absolute then we return it unchanged.
//
//	If sRoot is empty then we use the current directory.

	{
	CString sResult;

	DWORD dwResult = ::GetFullPathName(sPath.GetASCIIZPointer(),
			MAX_PATH,
			sResult.GetWritePointer(MAX_PATH),
			NULL);
	if (dwResult == 0)
		return NULL_STR;
	else if (dwResult > MAX_PATH)
		return NULL_STR;
	
	sResult.Truncate(dwResult);
	return sResult;
	}

CString pathMakeRelative (const CString &sFilespec, const CString &sRoot, bool bNoCheck)

//	pathMakeRelative
//
//	Converts an absolute path to a relative path. If the path does not start
//	with the given root then we return NULL_STR.
//
//	We strip out any leading / from the remaining path.

	{
	if (sRoot.GetLength() >= sFilespec.GetLength())
		return NULL_STR;

	if (!bNoCheck && !strStartsWith(sFilespec, sRoot))
		return NULL_STR;

	//	Do we have to strip a slash?

	char *pPos = sFilespec.GetASCIIZPointer() + sRoot.GetLength();
	if (pathIsPathSeparator(pPos))
		pPos++;

	return CString(pPos);
	}

CString pathStripExtension (const CString &sPath)

//	pathStripExtension
//
//	Returns the path without the extension on the filename

	{
	char *pPos;
	int iLength;

	pPos = sPath.GetASCIIZPointer();
	iLength = sPath.GetLength();

	//	Look for the extension

	while (iLength > 0)
		{
		if (pPos[iLength] == '.')
			break;
		iLength--;
		}

	//	Return everything except the extension

	return strSubString(sPath, 0, iLength);
	}

bool pathValidateFilename (const CString &sFilename, CString *retsValidFilename)

//	pathValidateFilename
//
//	Return TRUE if the given filename is valid. If an output parameter is also
//	supplied then we return a valid filename.

	{
	bool bOriginalValid = true;
	CString sValid = sFilename;
	char *pPos = sFilename.GetASCIIZPointer();
	char *pDest = sValid.GetASCIIZPointer();
	while (*pPos)
		{
		switch (*pPos)
			{
			case '<':
			case '>':
			case ':':
			case '/':
			case '\\':
			case '|':
			case '*':
			case '?':
				bOriginalValid = false;
				*pDest = '_';
				break;

			default:
				{
				if (*pPos < ' ')
					{
					bOriginalValid = false;
					*pDest = '_';
					}
				}
			}

		pPos++;
		pDest++;
		}

	if (retsValidFilename)
		*retsValidFilename = sValid;

	return bOriginalValid;
	}

//	HELPERS

void FreePIDL (LPITEMIDLIST pidl)
	{
	LPMALLOC pMalloc;
	if (SHGetMalloc(&pMalloc) != S_OK)
		return;

	pMalloc->Free(pidl);
	pMalloc->Release();
	}

CString GetVersionString (char *pData, WORD *pLangInfo, const CString &sString)
	{
	char szBuffer[1024];

	wsprintf(szBuffer, 
			"\\StringFileInfo\\%04x%04x\\%s",
			pLangInfo[0],
			pLangInfo[1],
			sString.GetASCIIZPointer());

	char *pPos;
	DWORD dwSize;
	if (::VerQueryValue(pData, szBuffer, (LPVOID *)&pPos, (PUINT)&dwSize))
		return CString(pPos, (int)dwSize-1);
	else
		return NULL_STR;
	}
