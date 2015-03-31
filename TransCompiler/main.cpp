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

#include "PreComp.h"
#include "TDBCompiler.h"

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

#define TAG_CORE_LIBRARY						CONSTLIT("CoreLibrary")
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
#define ATTRIB_HIT_MASK							CONSTLIT("hitMask")
#define ATTRIB_SHADOW_MASK						CONSTLIT("shadowMask")

class CSaveEntitiesTable
	{
	public:
		CSaveEntitiesTable (CTDBCompiler &Ctx, CExternalEntityTable *pNewTable) :
				 m_Ctx(Ctx),
				 m_pSavedTable(NULL)
			{
			if (pNewTable)
				m_pSavedTable = m_Ctx.PushEntityTable(pNewTable);
			}

		~CSaveEntitiesTable (void)
			{
			if (m_pSavedTable)
				m_Ctx.PopEntityTable(m_pSavedTable);
			}

	private:
		CTDBCompiler &m_Ctx;
		CExternalEntityTable *m_pSavedTable;
	};

void CreateTDB (const CString &sInputFilespec, CXMLElement *pCmdLine);
void DumpTDB (const CString &sFilespec);
ALERROR ParseGameFile (CTDBCompiler &Ctx, const CString &sFilename, CXMLElement **retpData);
ALERROR WriteGameFile (CTDBCompiler &Ctx, const CString &sFilespec, bool bCompress, CDataFile &Out, int *retiGameFile);
ALERROR WriteHeader (CTDBCompiler &Ctx, int iGameFile, CDataFile &Out);
ALERROR WriteModule (CTDBCompiler &Ctx,
					 const CString &sFilename, 
					 const CString &sFolder, 
					 CDataFile &Out, 
					 int *retiModuleEntry = NULL,
					 bool bCore = false);
ALERROR WriteModuleImages (CTDBCompiler &Ctx, CXMLElement *pModule, const CString &sFolder, CDataFile &Out);
ALERROR WriteModuleSounds (CTDBCompiler &Ctx, CXMLElement *pModule, const CString &sFolder, CDataFile &Out);
ALERROR WriteResource (CTDBCompiler &Ctx, const CString &sFilename, const CString &sFolder, bool bCompress, CDataFile &Out);
ALERROR WriteSubModules (CTDBCompiler &Ctx, 
						 CXMLElement *pModule,
						 const CString &sFolder, 
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
	printf("TransCompiler v2.2\n");
	printf("Copyright (c) 2003-2015 by Kronosaur Productions, LLC. All Rights Reserved.\n\n");

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

	//	If we just want to dump an existing TDB then we do that

	if (pCmdLine->GetAttributeBool(ATTRIB_DUMP) || strEquals(pathGetExtension(sInputFilespec), EXTENSION_TDB))
		DumpTDB(sInputFilespec);

	//	Otherwise we are creating a new TDB

	else
		CreateTDB(sInputFilespec, pCmdLine);
	}

void CreateTDB (const CString &sInputFilespec, CXMLElement *pCmdLine)
	{
	ALERROR error;
	CString sError;
	int i;

	//	Prepare a context block

	CTDBCompiler Ctx;

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

	//	Initialize our compiler.

	if (!Ctx.Init(sInputFilespec, sOutputFilespec, EntityFilespecs, pCmdLine, &sError))
		{
		printf("error: %s.\n", sError.GetASCIIZPointer());
		return;
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

	//	Write out the main module and recurse

	int iGameFile;
	if (error = WriteModule(Ctx, pathGetFilename(sInputFilespec), NULL_STR, Out, &iGameFile))
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
			Ctx.ReportError(strPatternSubst(CONSTLIT("Unable to create digest for '%s'."), sOutputFilespec));
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

	Ctx.ReportErrorCount();
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

ALERROR WriteGameFile (CTDBCompiler &Ctx, const CString &sFilespec, bool bCompress, CDataFile &Out, int *retiGameFile)
	{
	ALERROR error;

	CFileReadBlock theFile(pathAddComponent(Ctx.GetRootPath(), sFilespec));
	if (error = theFile.Open())
		{
		Ctx.ReportError(strPatternSubst(CONSTLIT("Unable to open '%s'."), sFilespec));
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
			Ctx.ReportError(sError);
			return ERR_FAIL;
			}

		sData = CString(Output.GetPointer(), Output.GetLength());
		}

	if (error = Out.AddEntry(sData, retiGameFile))
		{
		Ctx.ReportError(strPatternSubst(CONSTLIT("Unable to store '%s'."), sFilespec));
		return error;
		}

	printf("%s\n", sFilespec.GetASCIIZPointer());

	return NOERROR;
	}

ALERROR WriteHeader (CTDBCompiler &Ctx, int iGameFile, CDataFile &Out)
	{
	ALERROR error;
	CMemoryWriteStream Stream;

	if (error = Stream.Create())
		{
		Ctx.ReportError(CONSTLIT("Out of memory"));
		return error;
		}

	//	Write it

	CString sError;
	if (!Ctx.WriteHeader(Stream, iGameFile, &sError))
		{
		Ctx.ReportError(strPatternSubst(CONSTLIT("Unable to write header: %s"), sError));
		return ERR_FAIL;
		}

	//	Write out the header

	Stream.Close();
	CString sData(Stream.GetPointer(), Stream.GetLength(), TRUE);
	int iEntry;
	if (error = Out.AddEntry(sData, &iEntry))
		{
		Ctx.ReportError(CONSTLIT("Unable to write header"));
		return error;
		}

	Out.SetDefaultEntry(iEntry);

	return NOERROR;
	}

ALERROR WriteModule (CTDBCompiler &Ctx, 
					 const CString &sFilename, 
					 const CString &sFolder, 
					 CDataFile &Out, 
					 int *retiModuleEntry,
					 bool bCore)
	{
	ALERROR error;
	int i;

	//	Parse the file

	CXMLElement *pModule;
	CExternalEntityTable *pEntityTable = new CExternalEntityTable;
	CFileReadBlock DataFile(pathAddComponent(Ctx.GetRootPath(), sFilename));
	CString sError;

	printf("Parsing %s...", sFilename.GetASCIIZPointer());
	if (error = CXMLElement::ParseXML(&DataFile, Ctx.GetCoreEntities(), &pModule, &sError, pEntityTable))
		{
		printf("\n");
		Ctx.ReportError(sError);
		return error;
		}

	//	If this is a core module (embedded in the root XML) then we add these
	//	entities to the core. [Ctx takes ownership.]

	if (bCore)
		Ctx.AddEntityTable(pEntityTable);

	//	Chain entity tables (so that any modules that we load get the benefit).
	//	This will chain Ctx.pCoreEntities (and restore it in the destructor).
	//
	//	NOTE: If this is a core module, then we don't do this, since we've
	//	already added the entities to the context block.

	CSaveEntitiesTable SavedEntities(Ctx, (!bCore ? pEntityTable : NULL));

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
		Ctx.AddResource(sFilename, iEntry, bCompress);

	//	Store all the image resources

	if (error = WriteModuleImages(Ctx, pModule, sFolder, Out))
		return error;

	//	Store all the sound resources

	if (error = WriteModuleSounds(Ctx, pModule, sFolder, Out))
		return error;

	//	Store all modules

	if (error = WriteSubModules(Ctx, pModule, sFolder, Out))
		return error;

	//	The root module may have a TranscendenceAdventure tag with modules in it

	for (i = 0; i < pModule->GetContentElementCount(); i++)
		{
		CXMLElement *pItem = pModule->GetContentElement(i);

		if (strEquals(pItem->GetTag(), TAG_CORE_LIBRARY)
				|| strEquals(pItem->GetTag(), TAG_TRANSCENDENCE_ADVENTURE) 
				|| strEquals(pItem->GetTag(), TAG_TRANSCENDENCE_LIBRARY))
			{
			//	If we have a filename, then we need to save the target as a
			//	module.

			CString sFilename;
			if (pItem->FindAttribute(ATTRIB_FILENAME, &sFilename))
				{
				//	Write out the module, making sure to set the core flag.

				if (error = WriteModule(Ctx, sFilename, sFolder, Out, NULL, true))
					return error;

				//	We ignore any other elements.

				continue;
				}

			//	Store all the image resources

			if (error = WriteModuleImages(Ctx, pItem, sFolder, Out))
				return error;

			//	Store all the sound resources

			if (error = WriteModuleSounds(Ctx, pItem, sFolder, Out))
				return error;

			//	Modules

			if (error = WriteSubModules(Ctx, pItem, sFolder, Out))
				return error;
			}
		}

	//	Done

	if (retiModuleEntry)
		*retiModuleEntry = iEntry;

	return NOERROR;
	}

ALERROR WriteSubModules (CTDBCompiler &Ctx, 
						 CXMLElement *pModule,
						 const CString &sFolder, 
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
				if (WriteModule(Ctx, sFilename, sFolder, Out) != NOERROR)
					continue;
				}
			}
		else if (strEquals(pItem->GetTag(), TAG_MODULE))
			{
			CString sFilename = pItem->GetAttribute(ATTRIB_FILENAME);
			if (WriteModule(Ctx, sFilename, sFolder, Out) != NOERROR)
				continue;
			}
		}

	return NOERROR;
	}

ALERROR WriteModuleImages (CTDBCompiler &Ctx, CXMLElement *pModule, const CString &sFolder, CDataFile &Out)
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

			sFilename = pItem->GetAttribute(ATTRIB_SHADOW_MASK);
			if (!sFilename.IsBlank())
				{
				bool bCompress = strEquals(strToLower(pathGetExtension(sFilename)), CONSTLIT("bmp"));
				if (error = WriteResource(Ctx, sFilename, sFolder, bCompress, Out))
					continue;
				}

			sFilename = pItem->GetAttribute(ATTRIB_HIT_MASK);
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

ALERROR WriteModuleSounds (CTDBCompiler &Ctx, CXMLElement *pModule, const CString &sFolder, CDataFile &Out)
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

ALERROR WriteResource (CTDBCompiler &Ctx, const CString &sFilename, const CString &sFolder, bool bCompress, CDataFile &Out)
	{
	ALERROR error;
	CString sFilespec = pathAddComponent(sFolder, sFilename);

	CFileReadBlock theFile(pathAddComponent(Ctx.GetRootPath(), sFilespec));
	if (error = theFile.Open())
		{
		Ctx.ReportError(strPatternSubst(CONSTLIT("Unable to open '%s'."), sFilespec));
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
			Ctx.ReportError(sError);
			return ERR_FAIL;
			}

		sData = CString(Output.GetPointer(), Output.GetLength());
		}

	int iEntry;
	if (error = Out.AddEntry(sData, &iEntry))
		{
		Ctx.ReportError(strPatternSubst(CONSTLIT("Unable to store '%s'."), sFilespec));
		return error;
		}

	Ctx.AddResource(sFilespec, iEntry, bCompress);

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

ALERROR ParseGameFile (CTDBCompiler &Ctx, const CString &sFilename, CXMLElement **retpData)
	{
	ALERROR error;
	CFileReadBlock DataFile(sFilename);
	CString sError;

	printf("Parsing %s...", sFilename.GetASCIIZPointer());
	if (error = CXMLElement::ParseXML(&DataFile, retpData, &sError))
		{
		printf("\n");
		Ctx.ReportError(sError);
		return error;
		}

	printf("done.\n");

	return NOERROR;
	}
