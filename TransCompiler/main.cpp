//	TransCompiler
//
//	This program is used to compile Transcendence game and resource files
//
//	Copyright (c) 2003-2012 by Kronosaur Productions, LLC. All Rights Reserved.
//
//	The output file has a default entry with the following format:
//
//	DWORD		'TRDB'
//	DWORD		version (11)
//	DWORD		game file entry ID
//	CString		game title
//	CString		resource table (flattened CSymbolTable)

#include <stdio.h>

#include <windows.h>
#include <ddraw.h>
#include "Alchemy.h"
#include "Crypto.h"
#include "XMLUtil.h"
#include "Zip.h"
#include "TSE.h"

#define TDB_SIGNATURE							'TRDB'
#define TDB_VERSION								12

#define NOARGS									CONSTLIT("noArgs")
#define SWITCH_HELP								CONSTLIT("help")

#define ATTRIB_DIGEST							CONSTLIT("digest")
#define ATTRIB_DUMP								CONSTLIT("dump")
#define ATTRIB_ENTITIES							CONSTLIT("entities")
#define ATTRIB_INPUT							CONSTLIT("input")
#define ATTRIB_OUTPUT							CONSTLIT("output")

#define EXTENSION_TDB							CONSTLIT("tdb")

#define TAG_IMAGES								CONSTLIT("Images")
#define TAG_SOUNDS								CONSTLIT("Sounds")
#define TAG_MODULE								CONSTLIT("Module")
#define TAG_MODULES								CONSTLIT("Modules")
#define TAG_IMAGE								CONSTLIT("Image")
#define TAG_SOUND								CONSTLIT("Sound")
#define TAG_TRANSCENDENCE_ADVENTURE				CONSTLIT("TranscendenceAdventure")
#define TAG_TRANSCENDENCE_LIBRARY				CONSTLIT("TranscendenceLibrary")

#define ATTRIB_FOLDER							CONSTLIT("folder")
#define ATTRIB_BITMAP							CONSTLIT("bitmap")
#define ATTRIB_BITMASK							CONSTLIT("bitmask")
#define ATTRIB_FILENAME							CONSTLIT("filename")

enum EFlags
	{
	//	SResourceEntry flags
	FLAG_COMPRESS_ZLIB =		0x00000001,
	};

struct SResourceEntry
	{
	CString sFilename;
	int iEntryID;
	DWORD dwFlags;
	};

struct SCompilerCtx
	{
	SCompilerCtx (void) :
			iErrorCount(0) 
		{ }

	CString sRootPath;
	TSortMap<CString, SResourceEntry> ResourceMap;
	int iErrorCount;
	};

void DumpTDB (const CString &sFilespec);
ALERROR ReadEntities (const CString &sFilespec, CExternalEntityTable **retpEntityTable);
ALERROR ParseGameFile (SCompilerCtx &Ctx, const CString &sFilename, CXMLElement **retpData);
ALERROR WriteGameFile (SCompilerCtx &Ctx, const CString &sFilespec, bool bCompress, CDataFile &Out, int *retiGameFile);
ALERROR WriteHeader (SCompilerCtx &Ctx, int iGameFile, CDataFile &Out);
ALERROR WriteModule (SCompilerCtx &Ctx,
					 const CString &sFilename, 
					 const CString &sFolder, 
					 CExternalEntityTable *pEntityTable,
					 CDataFile &Out, 
					 int *retiModuleEntry);
ALERROR WriteModuleImages (SCompilerCtx &Ctx, CXMLElement *pModule, const CString &sFolder, CDataFile &Out);
ALERROR WriteModuleSounds (SCompilerCtx &Ctx, CXMLElement *pModule, const CString &sFolder, CDataFile &Out);
ALERROR WriteResource (SCompilerCtx &Ctx, const CString &sFilename, const CString &sFolder, bool bCompress, CDataFile &Out);
ALERROR WriteSubModules (SCompilerCtx &Ctx, 
						 CXMLElement *pModule,
						 const CString &sFolder, 
						 CExternalEntityTable *pEntityTable,
						 CDataFile &Out);

void TransCompiler (CXMLElement *pCmdLine)

//	TransCompiler
//
//	Compile the Transcendence game and resource files
//
//	<TransCompiler
//			input="{input filespec}"
//			output="{output filespec}"
//		/>

	{
	ALERROR error;
	int i;

	printf("TransCompiler v2.0\n");
	printf("Copyright (c) 2003-2013 by Kronosaur Productions, LLC. All Rights Reserved.\n\n");

	if (pCmdLine->GetAttributeBool(NOARGS) || pCmdLine->GetAttributeBool(SWITCH_HELP))
		{
		printf("  /input:{input filespec}\n");
		printf("  /output:{output filespec}\n");
		printf(" [/digest]\n");
		printf(" [/dump]\n");
		printf(" [/entities:{entity filespec}]\n");
		printf("\n");
		return;
		}

	//	Figure out the folder for the input filespec. All filenames will
	//	be relative to this root.

	CString sInputFilespec = pCmdLine->GetAttribute(ATTRIB_INPUT);
	if (sInputFilespec.IsBlank())
		sInputFilespec = CONSTLIT("Transcendence.xml");

	CString sRoot = pathGetPath(sInputFilespec);

	//	Figure out the output filespec

	CString sOutputFilespec = pCmdLine->GetAttribute(ATTRIB_OUTPUT);
	if (sOutputFilespec.IsBlank() && !sInputFilespec.IsBlank())
		sOutputFilespec = strPatternSubst(CONSTLIT("%s.tdb"), pathStripExtension(pathGetFilename(sInputFilespec)));

	//	Optional entities file.

	CString sEntities = pCmdLine->GetAttribute(ATTRIB_ENTITIES);
	TArray<CString> EntityFilespecs;
	if (strDelimitEx(sEntities, ',', DELIMIT_TRIM_WHITESPACE, 0, &EntityFilespecs) != NOERROR)
		{
		printf("error : Unable to parse list of entity filespecs.\n");
		return;
		}

	//	Always add Transcendence.tdb, if not compiling it.

	if (!strEquals(pathGetFilename(sInputFilespec), CONSTLIT("Transcendence.xml")))
		EntityFilespecs.Insert(CONSTLIT("Transcendence.tdb"), 0);

	//	If we just want to dump an existing TDB then we do that

	if (pCmdLine->GetAttributeBool(ATTRIB_DUMP) || strEquals(pathGetExtension(sInputFilespec), EXTENSION_TDB))
		{
		DumpTDB(sInputFilespec);
		}

	//	Otherwise we are creating a new TDB

	else
		{
		//	Add all entities

		CExternalEntityTable *pEntities = NULL;
		for (i = 0; i < EntityFilespecs.GetCount(); i++)
			{
			CExternalEntityTable *pNewEntities;
			if (error = ReadEntities(EntityFilespecs[i], &pNewEntities))
				{
				printf("error : Unable to read entities file '%s'\n", EntityFilespecs[i].GetASCIIZPointer());
				return;
				}

			if (pEntities == NULL)
				pEntities = pNewEntities;
			else
				{
				pNewEntities->SetParent(pEntities);
				pEntities = pNewEntities;
				}
			}

		//	Create the output file

		if (error = CDataFile::Create(sOutputFilespec, 4096, 0))
			{
			printf("error : Unable to create '%s'\n", sOutputFilespec.GetASCIIZPointer());
			return;
			}

		CDataFile Out(sOutputFilespec);
		if (error = Out.Open())
			{
			printf("error : Unable to open '%s'\n", sOutputFilespec.GetASCIIZPointer());
			return;
			}

		//	Prepare a symbol table to hold all the resource files

		SCompilerCtx Ctx;

		//	Get the path of the input

		Ctx.sRootPath = pathGetPath(sInputFilespec);
		sInputFilespec = pathGetFilename(sInputFilespec);

		//	Write out the main module and recurse

		int iGameFile;
		if (error = WriteModule(Ctx, sInputFilespec, NULL_STR, pEntities, Out, &iGameFile))
			goto Done;

		//	Write out the header

		if (error = WriteHeader(Ctx, iGameFile, Out))
			goto Done;

		//	Done with the file

		Out.Close();

		//	If necessary create a digest

		if (pCmdLine->GetAttributeBool(ATTRIB_DIGEST))
			{
			CIntegerIP Digest;

			if (fileCreateDigest(sOutputFilespec, &Digest) != NOERROR)
				{
				Ctx.iErrorCount++;
				printf("error : Unable to create digest for '%s'\n", sOutputFilespec.GetASCIIZPointer());
				goto Done;
				}

			//	Output C++ style constant

			printf("\nstatic BYTE g_Digest[] =\n\t{");

			BYTE *pDigest = Digest.GetBytes();
			for (i = 0; i < Digest.GetLength(); i++)
				{
				if ((i % 10) == 0)
					printf("\n\t");

				printf("%3d, ", (DWORD)pDigest[i]);
				}
			
			printf("\n\t};\n");
			}

	Done:

		printf("\n");
		printf("%s - %d error%s\n", sOutputFilespec.GetASCIIZPointer(), Ctx.iErrorCount, (Ctx.iErrorCount == 1 ? "" : "s"));
		}
	}

void DumpTDB (const CString &sFilespec)
	{
	int i;

	//	Open the TDB

	CString sError;
	CResourceDb TDBFile(sFilespec, true);
	if (TDBFile.Open(DFOPEN_FLAG_READ_ONLY, &sError) != NOERROR)
		{
		printf("error : Unable to open '%s': %s\n", sFilespec.GetASCIIZPointer(), sError.GetASCIIZPointer());
		return;
		}

	//	Dump out all the resources

	for (i = 0; i < TDBFile.GetResourceCount(); i++)
		printf("%s\n", TDBFile.GetResourceFilespec(i).GetASCIIZPointer());
	}

ALERROR ReadEntities (const CString &sFilespec, CExternalEntityTable **retpEntityTable)
	{
	ALERROR error;

	//	Open the data file.

	CDataFile EntitiesTDB(sFilespec);
	if (error = EntitiesTDB.Open(DFOPEN_FLAG_READ_ONLY))
		return error;

	//	Open

	CString sData;
	if (error = EntitiesTDB.ReadEntry(EntitiesTDB.GetDefaultEntry(), &sData))
		return error;

	CMemoryReadStream Stream(sData.GetASCIIZPointer(), sData.GetLength());
	if (error = Stream.Open())
		return error;

	//	Check the signature

	DWORD dwLoad;
	Stream.Read((char *)&dwLoad, sizeof(DWORD));
	if (dwLoad != TDB_SIGNATURE)
		return ERR_FAIL;

	//	Check the version

	Stream.Read((char *)&dwLoad, sizeof(DWORD));
	if (dwLoad > TDB_VERSION)
		return ERR_FAIL;

	//	Read the game file

	int iGameFile;
	Stream.Read((char *)&iGameFile, sizeof(DWORD));

	//	Load the main entry

	CString sGameFile;
	if (error = EntitiesTDB.ReadEntry(iGameFile, &sGameFile))
		return error;

	//	Allocate entities

	CExternalEntityTable *pEntities = new CExternalEntityTable;

	//	Parse the XML file from the buffer

	CBufferReadBlock GameFile(sGameFile);

	CString sError;
	if (error = CXMLElement::ParseEntityTable(&GameFile, pEntities, &sError))
		return error;

	//	Done

	*retpEntityTable = pEntities;

	return NOERROR;
	}

ALERROR WriteGameFile (SCompilerCtx &Ctx, const CString &sFilespec, bool bCompress, CDataFile &Out, int *retiGameFile)
	{
	ALERROR error;

	CFileReadBlock theFile(pathAddComponent(Ctx.sRootPath, sFilespec));
	if (error = theFile.Open())
		{
		printf("error : Unable to open '%s'\n", sFilespec.GetASCIIZPointer());
		Ctx.iErrorCount++;
		return error;
		}

	CString sData(theFile.GetPointer(0, -1), theFile.GetLength(), TRUE);

	if (bCompress)
		{
		CBufferReadBlock Input(sData);

		CMemoryWriteStream Output;
		if (error = Output.Create())
			return ERR_FAIL;

		CString sError;
		if (!zipCompress(Input, compressionZlib, Output, &sError))
			{
			printf("error: %s", sError.GetASCIIZPointer());
			Ctx.iErrorCount++;
			return ERR_FAIL;
			}

		sData = CString(Output.GetPointer(), Output.GetLength());
		}

	if (error = Out.AddEntry(sData, retiGameFile))
		{
		printf("error : Unable to store '%s'\n", sFilespec.GetASCIIZPointer());
		Ctx.iErrorCount++;
		return error;
		}

	printf("%s\n", sFilespec.GetASCIIZPointer());

	return NOERROR;
	}

ALERROR WriteHeader (SCompilerCtx &Ctx, int iGameFile, CDataFile &Out)
	{
	ALERROR error;
	CMemoryWriteStream Stream;
	DWORD dwSave;
	int i;

	if (error = Stream.Create())
		{
		printf("error : Out of memory\n");
		Ctx.iErrorCount++;
		return error;
		}

	//	Signature

	dwSave = TDB_SIGNATURE;
	Stream.Write((char *)&dwSave, sizeof(dwSave));

	//	Version

	dwSave = TDB_VERSION;
	Stream.Write((char *)&dwSave, sizeof(dwSave));

	//	Game file entry

	dwSave = iGameFile;
	Stream.Write((char *)&dwSave, sizeof(dwSave));

	//	Game title

	CString sGameTitle = CONSTLIT("Transcendence");
	sGameTitle.WriteToStream(&Stream);

	//	Resource map

	dwSave = Ctx.ResourceMap.GetCount();
	Stream.Write((char *)&dwSave, sizeof(dwSave));
	for (i = 0; i < Ctx.ResourceMap.GetCount(); i++)
		{
		const SResourceEntry &Entry = Ctx.ResourceMap[i];
		Entry.sFilename.WriteToStream(&Stream);

		Stream.Write((char *)&Entry.iEntryID, sizeof(dwSave));
		Stream.Write((char *)&Entry.dwFlags, sizeof(dwSave));
		}

	//	Write out the header

	Stream.Close();
	CString sData(Stream.GetPointer(), Stream.GetLength(), TRUE);
	int iEntry;
	if (error = Out.AddEntry(sData, &iEntry))
		{
		printf("error : Unable to write out header\n");
		Ctx.iErrorCount++;
		return error;
		}

	Out.SetDefaultEntry(iEntry);

	return NOERROR;
	}

ALERROR WriteModule (SCompilerCtx &Ctx, 
					 const CString &sFilename, 
					 const CString &sFolder, 
					 CExternalEntityTable *pEntityTable,
					 CDataFile &Out, 
					 int *retiModuleEntry)
	{
	ALERROR error;
	int i;

	//	Parse the file

	CXMLElement *pModule;
	CExternalEntityTable EntityTable;
	CFileReadBlock DataFile(pathAddComponent(Ctx.sRootPath, sFilename));
	CString sError;

	printf("Parsing %s...", sFilename.GetASCIIZPointer());
	if (error = CXMLElement::ParseXML(&DataFile, pEntityTable, &pModule, &sError, &EntityTable))
		{
		printf("\nerror : %s\n", sError.GetASCIIZPointer());
		Ctx.iErrorCount++;
		return error;
		}

	//	Chain entity tables (so that any modules that we load get the benefit).

	if (pEntityTable)
		EntityTable.SetParent(pEntityTable);

	pEntityTable = &EntityTable;
	printf("done.\n");

	//	Compress if this is NOT the main file. We can't compress the
	//	main file because we sometimes need to read it partially.

	bool bCompress = (retiModuleEntry == NULL);

	//	Write the module itself

	int iEntry;
	if (error = WriteGameFile(Ctx, sFilename, bCompress, Out, &iEntry))
		return error;

	//	If the caller doesn't want the module entry, then it means that this is
	//	a module (instead of the main file). If so, add it to the resources table

	if (retiModuleEntry == NULL)
		{
		SResourceEntry *pEntry = Ctx.ResourceMap.Insert(sFilename);
		pEntry->sFilename = sFilename;
		pEntry->iEntryID = iEntry;
		pEntry->dwFlags = (bCompress ? FLAG_COMPRESS_ZLIB : 0);
		}

	//	Store all the image resources

	if (error = WriteModuleImages(Ctx, pModule, sFolder, Out))
		return error;

	//	Store all the sound resources

	if (error = WriteModuleSounds(Ctx, pModule, sFolder, Out))
		return error;

	//	Store all modules

	if (error = WriteSubModules(Ctx, pModule, sFolder, pEntityTable, Out))
		return error;

	//	The root module may have a TranscendenceAdventure tag with modules in it

	for (i = 0; i < pModule->GetContentElementCount(); i++)
		{
		CXMLElement *pItem = pModule->GetContentElement(i);

		if (strEquals(pItem->GetTag(), TAG_TRANSCENDENCE_ADVENTURE) || strEquals(pItem->GetTag(), TAG_TRANSCENDENCE_LIBRARY))
			{
			//	Store all the image resources

			if (error = WriteModuleImages(Ctx, pItem, sFolder, Out))
				return error;

			//	Store all the sound resources

			if (error = WriteModuleSounds(Ctx, pItem, sFolder, Out))
				return error;

			//	Modules

			if (error = WriteSubModules(Ctx, pItem, sFolder, pEntityTable, Out))
				return error;
			}
		}

	//	Done

	if (retiModuleEntry)
		*retiModuleEntry = iEntry;

	return NOERROR;
	}

ALERROR WriteSubModules (SCompilerCtx &Ctx, 
						 CXMLElement *pModule,
						 const CString &sFolder, 
						 CExternalEntityTable *pEntityTable,
						 CDataFile &Out)
	{
	int i, j;

	for (i = 0; i < pModule->GetContentElementCount(); i++)
		{
		CXMLElement *pItem = pModule->GetContentElement(i);

		if (strEquals(pItem->GetTag(), TAG_MODULES))
			{
			for (j = 0; j < pItem->GetContentElementCount(); j++)
				{
				CXMLElement *pDesc = pItem->GetContentElement(j);

				CString sFilename = pDesc->GetAttribute(ATTRIB_FILENAME);
				if (WriteModule(Ctx, sFilename, sFolder, pEntityTable, Out, NULL) != NOERROR)
					continue;
				}
			}
		else if (strEquals(pItem->GetTag(), TAG_MODULE))
			{
			CString sFilename = pItem->GetAttribute(ATTRIB_FILENAME);
			if (WriteModule(Ctx, sFilename, sFolder, pEntityTable, Out, NULL) != NOERROR)
				continue;
			}
		}

	return NOERROR;
	}

ALERROR WriteModuleImages (SCompilerCtx &Ctx, CXMLElement *pModule, const CString &sFolder, CDataFile &Out)
	{
	ALERROR error;
	int i;

	for (i = 0; i < pModule->GetContentElementCount(); i++)
		{
		CXMLElement *pItem = pModule->GetContentElement(i);
		if (strEquals(pItem->GetTag(), TAG_IMAGES))
			{
			CString sSubFolder = pathAddComponent(sFolder, pItem->GetAttribute(ATTRIB_FOLDER));

			if (error = WriteModuleImages(Ctx, pItem, sSubFolder, Out))
				return error;
			}
		else if (strEquals(pItem->GetTag(), TAG_IMAGE))
			{
			CString sFilename = pItem->GetAttribute(ATTRIB_BITMAP);
			if (!sFilename.IsBlank())
				{
				bool bCompress = strEquals(strToLower(pathGetExtension(sFilename)), CONSTLIT("bmp"));
				if (error = WriteResource(Ctx, sFilename, sFolder, bCompress, Out))
					continue;
				}

			sFilename = pItem->GetAttribute(ATTRIB_BITMASK);
			if (!sFilename.IsBlank())
				{
				bool bCompress = strEquals(strToLower(pathGetExtension(sFilename)), CONSTLIT("bmp"));
				if (error = WriteResource(Ctx, sFilename, sFolder, bCompress, Out))
					continue;
				}
			}
		}

	return NOERROR;
	}

ALERROR WriteModuleSounds (SCompilerCtx &Ctx, CXMLElement *pModule, const CString &sFolder, CDataFile &Out)
	{
	ALERROR error;
	int i;

	for (i = 0; i < pModule->GetContentElementCount(); i++)
		{
		CXMLElement *pItem = pModule->GetContentElement(i);
		if (strEquals(pItem->GetTag(), TAG_SOUNDS))
			{
			CString sSubFolder = pathAddComponent(sFolder, pItem->GetAttribute(ATTRIB_FOLDER));

			if (error = WriteModuleSounds(Ctx, pItem, sSubFolder, Out))
				return error;
			}
		else if (strEquals(pItem->GetTag(), TAG_SOUND))
			{
			CString sFilename = pItem->GetAttribute(ATTRIB_FILENAME);
			if (error = WriteResource(Ctx, sFilename, sFolder, false, Out))
				continue;
			}
		}

	return NOERROR;
	}

ALERROR WriteResource (SCompilerCtx &Ctx, const CString &sFilename, const CString &sFolder, bool bCompress, CDataFile &Out)
	{
	ALERROR error;
	CString sFilespec = pathAddComponent(sFolder, sFilename);

	CFileReadBlock theFile(pathAddComponent(Ctx.sRootPath, sFilespec));
	if (error = theFile.Open())
		{
		printf("error : Unable to open '%s'\n", sFilespec.GetASCIIZPointer());
		Ctx.iErrorCount++;
		return error;
		}

	CString sData(theFile.GetPointer(0, -1), theFile.GetLength(), TRUE);

	if (bCompress)
		{
		CBufferReadBlock Input(sData);

		CMemoryWriteStream Output;
		if (error = Output.Create())
			return ERR_FAIL;

		CString sError;
		if (!zipCompress(Input, compressionZlib, Output, &sError))
			{
			printf("error: %s", sError.GetASCIIZPointer());
			Ctx.iErrorCount++;
			return ERR_FAIL;
			}

		sData = CString(Output.GetPointer(), Output.GetLength());
		}

	int iEntry;
	if (error = Out.AddEntry(sData, &iEntry))
		{
		printf("error : Unable to store '%s'\n", sFilespec.GetASCIIZPointer());
		Ctx.iErrorCount++;
		return error;
		}

	SResourceEntry *pEntry = Ctx.ResourceMap.Insert(sFilespec);
	pEntry->sFilename = sFilespec;
	pEntry->iEntryID = iEntry;
	pEntry->dwFlags = (bCompress ? FLAG_COMPRESS_ZLIB : 0);

	printf("   %s\n", sFilespec.GetASCIIZPointer());

	return NOERROR;
	}

int main (int argc, char *argv[ ], char *envp[ ])

//	main
//
//	main entry-point

	{
	if (!kernelInit())
		{
		printf("error : Unable to initialize Alchemy kernel.\n");
		return 1;
		}

	//	Do it

	{
	ALERROR error;
	CXMLElement *pCmdLine;
	if (error = CreateXMLElementFromCommandLine(argc, argv, &pCmdLine))
		{
		printf("error : Invalid command line.\n");
		kernelCleanUp();
		return 1;
		}

	TransCompiler(pCmdLine);

	delete pCmdLine;
	}

	//	Done

	kernelCleanUp();
	return 0;
	}

ALERROR ParseGameFile (SCompilerCtx &Ctx, const CString &sFilename, CXMLElement **retpData)
	{
	ALERROR error;
	CFileReadBlock DataFile(sFilename);
	CString sError;

	printf("Parsing %s...", sFilename.GetASCIIZPointer());
	if (error = CXMLElement::ParseXML(&DataFile, retpData, &sError))
		{
		printf("\nerror : %s\n", sError.GetASCIIZPointer());
		Ctx.iErrorCount++;
		return error;
		}

	printf("done.\n");

	return NOERROR;
	}
