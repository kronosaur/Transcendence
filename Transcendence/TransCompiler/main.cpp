//	TransCompiler
//
//	This program is used to compile Transcendence game and resource files
//
//	Copyright (c) 2003 by George Moromisato. All Rights Reserved.
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
#include "XMLUtil.h"

#define TDB_SIGNATURE							'TRDB'
#define TDB_VERSION								11

#define NOARGS									CONSTLIT("noArgs")
#define SWITCH_HELP								CONSTLIT("help")

#define ATTRIB_INPUT							CONSTLIT("input")
#define ATTRIB_OUTPUT							CONSTLIT("output")

#define TAG_IMAGES								CONSTLIT("Images")
#define TAG_SOUNDS								CONSTLIT("Sounds")
#define TAG_MODULES								CONSTLIT("Modules")
#define TAG_IMAGE								CONSTLIT("Image")
#define TAG_SOUND								CONSTLIT("Sound")
#define TAG_TRANSCENDENCE_ADVENTURE				CONSTLIT("TranscendenceAdventure")

#define ATTRIB_FOLDER							CONSTLIT("folder")
#define ATTRIB_BITMAP							CONSTLIT("bitmap")
#define ATTRIB_BITMASK							CONSTLIT("bitmask")
#define ATTRIB_FILENAME							CONSTLIT("filename")

struct SCompilerCtx
	{
	SCompilerCtx (void) :
			Resources(FALSE, TRUE),
			iErrorCount(0) 
		{ }

	CSymbolTable Resources;
	int iErrorCount;
	};

ALERROR ParseGameFile (SCompilerCtx &Ctx, const CString &sFilename, CXMLElement **retpData);
ALERROR WriteGameFile (SCompilerCtx &Ctx, const CString &sFilespec, CDataFile &Out, int *retiGameFile);
ALERROR WriteHeader (SCompilerCtx &Ctx, int iGameFile, CDataFile &Out);
ALERROR WriteModule (SCompilerCtx &Ctx,
					 const CString &sFilename, 
					 const CString &sFolder, 
					 CExternalEntityTable *pEntityTable,
					 CDataFile &Out, 
					 int *retiModuleEntry);
ALERROR WriteModuleImages (SCompilerCtx &Ctx, CXMLElement *pModule, const CString &sFolder, CDataFile &Out);
ALERROR WriteModuleSounds (SCompilerCtx &Ctx, CXMLElement *pModule, const CString &sFolder, CDataFile &Out);
ALERROR WriteResource (SCompilerCtx &Ctx, const CString &sFilename, const CString &sFolder, CDataFile &Out);
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

	printf("TransCompiler v1.01\n");
	printf("Copyright (c) 2003-2011 by George Moromisato. All Rights Reserved.\n\n");

	if (pCmdLine->GetAttributeBool(NOARGS) || pCmdLine->GetAttributeBool(SWITCH_HELP))
		{
		printf("  /input:{input filespec}\n");
		printf("  /output:{output filespec}\n");
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
	if (sOutputFilespec.IsBlank())
		sOutputFilespec = CONSTLIT("Transcendence.tdb");

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

	//	Write out the main module and recurse

	int iGameFile;
	if (error = WriteModule(Ctx, sInputFilespec, NULL_STR, NULL, Out, &iGameFile))
		goto Done;

	//	Write out the header

	if (error = WriteHeader(Ctx, iGameFile, Out))
		goto Done;

	//	Done

Done:

	Out.Close();
	printf("\n");
	printf("%s - %d error%s\n", sOutputFilespec.GetASCIIZPointer(), Ctx.iErrorCount, (Ctx.iErrorCount == 1 ? "" : "s")); 
	}

ALERROR WriteGameFile (SCompilerCtx &Ctx, const CString &sFilespec, CDataFile &Out, int *retiGameFile)
	{
	ALERROR error;

	CFileReadBlock theFile(sFilespec);
	if (error = theFile.Open())
		{
		printf("error : Unable to open '%s'\n", sFilespec.GetASCIIZPointer());
		Ctx.iErrorCount++;
		return error;
		}

	CString sData(theFile.GetPointer(0, -1), theFile.GetLength(), TRUE);
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

	CString sSave;
	if (error = CObject::Flatten(&Ctx.Resources, &sSave))
		{
		printf("error : Unable to flatten resources map\n");
		Ctx.iErrorCount++;
		return error;
		}

	sSave.WriteToStream(&Stream);

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

	//	Parse the file

	CXMLElement *pModule;
	CExternalEntityTable EntityTable;
	if (pEntityTable)
		{
		CFileReadBlock DataFile(sFilename);
		CString sError;

		printf("Parsing %s...", sFilename.GetASCIIZPointer());
		if (error = CXMLElement::ParseXML(&DataFile, pEntityTable, &pModule, &sError))
			{
			printf("\nerror : %s\n", sError.GetASCIIZPointer());
			Ctx.iErrorCount++;
			return error;
			}

		printf("done.\n");
		}
	else
		{
		CFileReadBlock DataFile(sFilename);
		CString sError;

		printf("Parsing %s...", sFilename.GetASCIIZPointer());
		if (error = CXMLElement::ParseXML(&DataFile, &pModule, &sError, &EntityTable))
			{
			printf("\nerror : %s\n", sError.GetASCIIZPointer());
			Ctx.iErrorCount++;
			return error;
			}

		pEntityTable = &EntityTable;
		printf("done.\n");
		}

	//	Write the module itself

	int iEntry;
	if (error = WriteGameFile(Ctx, sFilename, Out, &iEntry))
		return error;

	//	If the caller doesn't want the module entry, then it means that this is
	//	a module (instead of the main file). If so, add it to the resources table

	if (retiModuleEntry == NULL)
		Ctx.Resources.AddEntry(sFilename, (CObject *)iEntry);

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

	CXMLElement *pAdventure = pModule->GetContentElementByTag(TAG_TRANSCENDENCE_ADVENTURE);
	if (pAdventure)
		{
		if (error = WriteSubModules(Ctx, pAdventure, sFolder, pEntityTable, Out))
			return error;
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
	int i;

	CXMLElement *pModules = pModule->GetContentElementByTag(TAG_MODULES);
	if (pModules)
		{
		for (i = 0; i < pModules->GetContentElementCount(); i++)
			{
			CXMLElement *pItem = pModules->GetContentElement(i);

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
				if (error = WriteResource(Ctx, sFilename, sFolder, Out))
					continue;
				}

			sFilename = pItem->GetAttribute(ATTRIB_BITMASK);
			if (!sFilename.IsBlank())
				{
				if (error = WriteResource(Ctx, sFilename, sFolder, Out))
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
			if (error = WriteResource(Ctx, sFilename, sFolder, Out))
				continue;
			}
		}

	return NOERROR;
	}

ALERROR WriteResource (SCompilerCtx &Ctx, const CString &sFilename, const CString &sFolder, CDataFile &Out)
	{
	ALERROR error;
	CString sFilespec = pathAddComponent(sFolder, sFilename);

	CFileReadBlock theFile(sFilespec);
	if (error = theFile.Open())
		{
		printf("error : Unable to open '%s'\n", sFilespec.GetASCIIZPointer());
		return error;
		}

	CString sData(theFile.GetPointer(0, -1), theFile.GetLength(), TRUE);
	int iEntry;
	if (error = Out.AddEntry(sData, &iEntry))
		{
		printf("error : Unable to store '%s'\n", sFilespec.GetASCIIZPointer());
		return error;
		}

	Ctx.Resources.AddEntry(sFilespec, (CObject *)iEntry);
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

