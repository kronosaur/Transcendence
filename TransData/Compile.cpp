//	Compile.cpp
//
//	Functions to manipulate .tdb files

#include <stdio.h>

#include <windows.h>
#include "Alchemy.h"
#include "XMLUtil.h"
#include "TransData.h"

ALERROR ExtractFile (const CString &sDest, const CString &sData);

void Decompile (const CString &sDataFile, CXMLElement *pCmdLine)
	{
	ALERROR error;
	CString sError;

	//	Open the TDB file

	CString sDb = sDataFile;
	if (strEquals(sDb, CONSTLIT("Transcendence")))
		sDb = CONSTLIT("Transcendence.tdb");

	CString sMainFile = pathStripExtension(pathGetFilename(sDb));

	CResourceDb Resources(sDb);
	if (error = Resources.Open(0, &sError))
		{
		printf("%s\n", (LPSTR)sError);
		return;
		}

	//	Output to a sub folder

	CString sOutputFolder = strPatternSubst(CONSTLIT("%s_Source"), sMainFile);

	//	Extract the main game file

	CString sData;
	if (error = Resources.ExtractMain(&sData))
		{
		printf("Unable to extract file: %s.xml\n", sMainFile.GetASCIIZPointer());
		return;
		}

	CString sOutputFilespec = pathAddComponent(sOutputFolder, strPatternSubst(CONSTLIT("%s.xml"), sMainFile));
	if (error = ExtractFile(sOutputFilespec, sData))
		return;

	//	Extract all the resources

	int iCount = Resources.GetResourceCount();
	for (int i = 0; i < iCount; i++)
		{
		CString sFilespec = Resources.GetResourceFilespec(i);

		if (error = Resources.ExtractResource(sFilespec, &sData))
			{
			printf("Unable to extract file: %s\n", sFilespec.GetASCIIZPointer());
			return;
			}

		if (Resources.GetVersion() < 11)
			sOutputFilespec = pathAddComponent(sOutputFolder, pathAddComponent(CONSTLIT("Resources"), sFilespec));
		else
			sOutputFilespec = pathAddComponent(sOutputFolder, sFilespec);

		if (error = ExtractFile(sOutputFilespec, sData))
			return;
		}
	}

ALERROR ExtractFile (const CString &sDest, const CString &sData)
	{
	ALERROR error;

	//	Make sure the path exists

	pathCreate(pathGetPath(sDest));

	//	Write the file out

	CFileWriteStream theFile(sDest, FALSE);
	if (error = theFile.Create())
		{
		printf("Unable to create file: %s\n", sDest.GetASCIIZPointer());
		return error;
		}

	if (error = theFile.Write(sData.GetASCIIZPointer(), sData.GetLength()))
		{
		printf("Unable to write file: %s\n", sDest.GetASCIIZPointer());
		return error;
		}

	if (error = theFile.Close())
		{
		printf("Unable to write file: %s\n", sDest.GetASCIIZPointer());
		return error;
		}

	printf("%s\n", sDest.GetASCIIZPointer());
	return NOERROR;
	}
