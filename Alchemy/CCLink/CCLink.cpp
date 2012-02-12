//	CCLink.cpp
//
//	This program converts a text file containing a list of CodeChain
//	definitions into a binary file.
//	
//	CodeChain Definition Format (.cdf)
//	
//	The input file is called a CodeChain Definition Format and
//	is a text file containing one or more CodeChain lists. Each list
//	is an entry with two items. The first is an identifier and the
//	second is a definition to associate with the identifier.
//
//	CodeChain Definition Output (.cdo)
//
//	The output file is a binary file that contains the evaluated
//	definitions. The format follows:
//
//	DWORD	'CCDO'
//	DWORD	Version number
//	DWORD	The number of entries
//
//	DWORD	Offset for entry 1
//	DWORD	Offset for entry 2
//		...
//
//	DWORD	Length of entry identifiers block
//	char	Identifiers in order separated
//			by terminating NULL
//
//	Definition data follows

#include "Alchemy.h"
#include "ALConsole.h"

#define OUTPUT_SIGNATURE							'CCDO'
#define OUTPUT_VERSION								1

#define INPUT_EXTENSION								LITERAL(".cdf")
#define OUTPUT_EXTENSION							LITERAL(".cdo")

typedef struct
	{
	DWORD dwSignature;								//	Always 'CCDO'
	DWORD dwVersion;								//	Version of format
	DWORD dwEntryCount;								//	Number of entries
	} HEADERSTRUCT, *PHEADERSTRUCT;

class CCommandLineOptions
	{
	public:
		CString sInputPath;
		CString sOutputPath;
	};

//	Forwards

ALERROR GenerateOutput (CSymbolTable *pEntries, CFileWriteStream *pOutput);
ALERROR LinkIt (CCodeChain *pCC, CCommandLineOptions *pOptions, int *retiDefinitions);
ALERROR ParseCommandLine (CStringArray *pArgs, CCommandLineOptions *retOptions);

int alchemyConsoleMain (CStringArray *pArgs)

//	alchemyConsoleMain
//
//	This is the main entry point for a command line program

	{
	ALERROR error;
	CCodeChain CC;
	CCommandLineOptions Options;
	int iDefinitions;

	//	Boot the interpreter

	if (error = CC.Boot())
		{
		consolePrint(LITERAL("ERROR: Unable to boot CodeChain interpreter.\n"));
		return -1;
		}

	//	Start

	consolePrint(LITERAL("CCLink v1.0\n"));
	consolePrint(LITERAL("CodeChain Definition Format Linker\n\n"));

	//	Parse the command line

	if (error = ParseCommandLine(pArgs, &Options))
		return -1;

	//	Do it

	if (error = LinkIt(&CC, &Options, &iDefinitions))
		return -1;

	//	Done

	consolePrint(LITERAL("\nProcessed %d definition%p\n"), iDefinitions);

	return 0;
	}

ALERROR GenerateOutput (CSymbolTable *pEntries, CFileWriteStream *pOutput)

//	GenerateOutput
//
//	Generates the output file

	{
	ALERROR error;
	HEADERSTRUCT header;
	CIntArray Offsets;
	int i;
	DWORD dwTotalStringLength;
	DWORD dwPos;

	//	Prepare the header

	utlMemSet(&header, sizeof(header), 0);
	header.dwSignature = OUTPUT_SIGNATURE;
	header.dwVersion = OUTPUT_VERSION;
	header.dwEntryCount = pEntries->GetCount();

	if (error = pOutput->Write((char *)&header, sizeof(header), NULL))
		{
		consolePrint(LITERAL("ERROR: Unable to write to output file\n"));
		return error;
		}

	//	Figure out the total length of all the identifiers

	dwTotalStringLength = 0;
	for (i = 0; i < pEntries->GetCount(); i++)
		{
		CString sKey;

		sKey = pEntries->GetKey(i);
		dwTotalStringLength += sKey.GetLength() + 1;
		}

	dwTotalStringLength = AlignUp(dwTotalStringLength, sizeof(DWORD));

	//	Now compute the offsets for each entry

	dwPos = sizeof(header) + (sizeof(DWORD) * pEntries->GetCount()) + sizeof(DWORD) + dwTotalStringLength;
	for (i = 0; i < pEntries->GetCount(); i++)
		{
		CString *pValue;

		if (error = Offsets.AppendElement((int)dwPos, 0))
			{
			consolePrint(LITERAL("ERROR: Out of memory\n"));
			return error;
			}

		pValue = (CString *)pEntries->GetValue(i);
		dwPos += pValue->GetLength();
		}

	//	Now we're ready to write out the offset table

	for (i = 0; i < Offsets.GetCount(); i++)
		{
		DWORD dwOffset = (DWORD)Offsets.GetElement(i);

		if (error = pOutput->Write((char *)&dwOffset, sizeof(dwOffset), NULL))
			{
			consolePrint(LITERAL("ERROR: Unable to write to output file\n"));
			return error;
			}
		}

	//	Now write out the size of the string block

	if (error = pOutput->Write((char *)&dwTotalStringLength, sizeof(dwTotalStringLength), NULL))
		{
		consolePrint(LITERAL("ERROR: Unable to write to output file\n"));
		return error;
		}

	//	Now write out each string with the NULL termination

	for (i = 0; i < pEntries->GetCount(); i++)
		{
		CString sKey = pEntries->GetKey(i);

		if (error = pOutput->Write(sKey.GetPointer(), sKey.GetLength() + 1, NULL))
			{
			consolePrint(LITERAL("ERROR: Unable to write to output file\n"));
			return error;
			}

		dwTotalStringLength -= sKey.GetLength() + 1;
		}

	ASSERT(dwTotalStringLength >= 0 && dwTotalStringLength < sizeof(DWORD));

	//	Write out any padding

	if (dwTotalStringLength)
		{
		if (error = pOutput->Write((char *)&dwTotalStringLength, dwTotalStringLength, NULL))
			{
			consolePrint(LITERAL("ERROR: Unable to write to output file\n"));
			return error;
			}
		}

	//	Now write out each definition

	for (i = 0; i < pEntries->GetCount(); i++)
		{
		CString *pValue = (CString *)pEntries->GetValue(i);

		if (error = pOutput->Write(pValue->GetPointer(), pValue->GetLength(), NULL))
			{
			consolePrint(LITERAL("ERROR: Unable to write to output file\n"));
			return error;
			}
		}

	return NOERROR;
	}

ALERROR LinkIt (CCodeChain *pCC, CCommandLineOptions *pOptions, int *retiDefinitions)

//	LinkIt
//
//	Process the input file and generates the output

	{
	ALERROR error;
	CFileReadBlock Input(pOptions->sInputPath);
	CFileWriteStream Output(pOptions->sOutputPath, FALSE);
	ICCItem *pItem = NULL;
	int iPos;
	CSymbolTable Entries(TRUE, FALSE);

	//	Open the input file

	if (error = Input.Open())
		{
		consolePrint(LITERAL("ERROR: Unable to open file: %s\n"), pOptions->sInputPath);
		return error;
		}

	CString sInput(Input.GetPointer(0, Input.GetLength()), Input.GetLength(), TRUE);

	//	Read in each definition

	iPos = 0;
	do
		{
		int iLinked;

		//	Free from previous iteration

		if (pItem)
			pItem->Discard(pCC);

		//	Parse the next definition

		pItem = pCC->Link(sInput, iPos, &iLinked);
		if (pItem->IsError())
			{
			consolePrint(LITERAL("ERROR: Error parsing file\n"));
			return ERR_FAIL;
			}

		//	If this is not Nil, then we add it to our list

		if (!pItem->IsNil())
			{
			ICCItem *pIdentifier;
			ICCItem *pDefinition;
			ICCItem *pEval;
			CString *pStorage;

			//	The item better be a list of exactly two items

			if (pItem->GetCount() != 2)
				{
				consolePrint(LITERAL("ERROR: Definition must be a two-item list\n"));
				return ERR_FAIL;
				}

			//	Get the two items

			pIdentifier = pItem->GetElement(0);
			pDefinition = pItem->GetElement(1);

			//	The first must be a string

			if (!pIdentifier->IsIdentifier())
				{
				consolePrint(LITERAL("ERROR: Identifier expected\n"));
				return ERR_FAIL;
				}

			//	Print out the identifier

			consolePrint(LITERAL("%s\n"), pIdentifier->GetStringValue());

			//	Evaluate the definition

			pEval = pCC->TopLevel(pDefinition);

			//	Stream the result into a string

			pStorage = new CString;
			if (pStorage)
				{
				CMemoryWriteStream Stream(0);
				ICCItem *pError;

				if (error = Stream.Create())
					{
					consolePrint(LITERAL("ERROR: Out of memory\n"));
					return error;
					}

				pError = pCC->StreamItem(pEval, &Stream);
				if (pError->IsError())
					{
					consolePrint(LITERAL("ERROR: Unable to stream definition\n"));
					return ERR_FAIL;
					}

				pError->Discard(pCC);

				//	Convert to string

				CString sString(Stream.GetPointer(), Stream.GetLength());
				*pStorage = sString;
				}

			//	Add the string to the symbol table

			if (error = Entries.AddEntry(pIdentifier->GetStringValue(), pStorage))
				{
				if (error == ERR_FAIL)
					consolePrint(LITERAL("ERROR: Identifier already defined\n"));
				else
					consolePrint(LITERAL("ERROR: Out of memory\n"));
				return error;
				}

			//	Done with this stuff

			pEval->Discard(pCC);
			}

		//	Next

		iPos += iLinked;
		}
	while (pItem && !pItem->IsNil());

	//	Open the output file

	if (error = Output.Create())
		{
		consolePrint(LITERAL("ERROR: Unable to create output file: %s\n"), pOptions->sOutputPath);
		return error;
		}

	//	Output

	if (error = GenerateOutput(&Entries, &Output))
		return error;

	//	Done

	Output.Close();
	Input.Close();

	if (retiDefinitions)
		*retiDefinitions = Entries.GetCount();

	return NOERROR;
	}

ALERROR ParseCommandLine (CStringArray *pArgs, CCommandLineOptions *retOptions)

//	ParseCommandLine
//
//	Parses the command line

	{
	int i;

	//	Loop through the command line

	for (i = 0; i < pArgs->GetCount(); i++)
		{
		CString sArg = pArgs->GetStringValue(i);
		char *pPos;

		pPos = sArg.GetPointer();
		if (*pPos == '/')
			NULL;
		else
			{
			//	If we haven't yet got the input filename,
			//	then this must be it

			if (retOptions->sInputPath.IsBlank())
				retOptions->sInputPath = sArg;
			}
		}

	//	Convert the input path to an output path

	retOptions->sOutputPath = strCat(pathStripExtension(retOptions->sInputPath), OUTPUT_EXTENSION);

	return NOERROR;
	}
