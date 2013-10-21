//	ZipArchive.cpp
//
//	Implements compression functions
//	Portions copyright (c) 2013 by Kronosaur Productions, LLC. All Rights Reserved.

#include "Kernel.h"
#include "KernelObjID.h"
#include "Zip.h"

#define ZLIB_WINAPI
#include "..\..\zlib-1.2.7\zlib.h"

#ifdef ZLIB_H
//	unzip.h needs _ZLIB_H defined (not ZLIB_H, for some reason)
#define _ZLIB_H
#endif

#include "..\..\zlib-1.2.7\contrib\minizip\unzip.h"

const int BUFFER_SIZE = 64 * 1024;

bool arcDecompressFile (const CString &sArchive, const CString &sFilename, IWriteStream &Output, CString *retsError)

//	arcDecompressFile
//
//	Unzips to a stream.

	{
	unzFile theZipFile = unzOpen(sArchive.GetASCIIZPointer());
	if (theZipFile == NULL)
		{
		*retsError = strPatternSubst(CONSTLIT("Unable to open file: %s."), sArchive);
		return false;
		}

	if (unzLocateFile(theZipFile, sFilename.GetASCIIZPointer(), 0) != UNZ_OK)
		{
		unzClose(theZipFile);
		*retsError = strPatternSubst(CONSTLIT("Unable to find file in archive: %s."), sFilename);
		return false;
		}

	if (unzOpenCurrentFile(theZipFile) != UNZ_OK)
		{
		unzClose(theZipFile);
		*retsError = strPatternSubst(CONSTLIT("Unable to open file in archive: %s."), sFilename);
		return false;
		}

	while (true)
		{
		char szBuffer[BUFFER_SIZE];

		int iRead = unzReadCurrentFile(theZipFile, szBuffer, BUFFER_SIZE);
		if (iRead == 0)
			break;
		else if (iRead < 0)
			{
			unzCloseCurrentFile(theZipFile);
			unzClose(theZipFile);
			*retsError = CONSTLIT("Error reading archive.");
			return false;
			}

		Output.Write(szBuffer, iRead);
		}

	//	Returns UNZ_CRCERROR if the file failed its CRC check.

	if (unzCloseCurrentFile(theZipFile) != UNZ_OK)
		{
		unzClose(theZipFile);
		*retsError = strPatternSubst(CONSTLIT("File in archive corrupted: %s."), sArchive);
		return false;
		}

	unzClose(theZipFile);

	return true;
	}

bool arcDecompressFile (const CString &sArchive, const CString &sFile, const CString &sDestFilespec, CString *retsError)

//	arcDecompressFile
//
//	Unzips to a file.

	{
	CFileWriteStream Output(sDestFilespec);
	if (Output.Create() != NOERROR)
		{
		if (retsError) *retsError = strPatternSubst(CONSTLIT("Unable to create output file: %s."), sDestFilespec);
		return false;
		}

	if (!arcDecompressFile(sArchive, sFile, Output, retsError))
		return false;

	//	Done

	Output.Close();
	return true;
	}

bool arcList (const CString &sArchive, TArray<CString> *retFiles, CString *retsError)

//	arcList
//
//	Returns a list of files in the given archive

	{
	unzFile theZipFile = unzOpen(sArchive.GetASCIIZPointer());
	if (theZipFile == NULL)
		{
		if (retsError) *retsError = strPatternSubst(CONSTLIT("Unable to open file: %s."), sArchive);
		return false;
		}

	//	Start at the first file

	if (unzGoToFirstFile(theZipFile) != UNZ_OK)
		{
		unzClose(theZipFile);
		if (retsError) *retsError = strPatternSubst(CONSTLIT("Unable to list files in archive: %s."), sArchive);
		return false;
		}

	//	Loop until we reach the last file

	while (true)
		{
		const int MAX_LEN = 2048;
		CString sFilename;

		if (unzGetCurrentFileInfo(theZipFile, NULL, sFilename.GetWritePointer(MAX_LEN), MAX_LEN, NULL, 0, NULL, 0) != UNZ_OK)
			{
			unzClose(theZipFile);
			if (retsError) *retsError = strPatternSubst(CONSTLIT("Error listing files in archive: %s."), sArchive);
			return false;
			}

		sFilename.Truncate(strlen(sFilename.GetASCIIZPointer()));

		retFiles->Insert(sFilename);

		//	Next

		int err = unzGoToNextFile(theZipFile);
		if (err == UNZ_END_OF_LIST_OF_FILE)
			break;
		else if (err != UNZ_OK)
			{
			unzClose(theZipFile);
			if (retsError) *retsError = strPatternSubst(CONSTLIT("Error listing files in archive: %s."), sArchive);
			return false;
			}

		//	Continue
		}

	//	Done

	unzClose(theZipFile);
	return true;
	}

