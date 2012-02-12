//	Aristotle.cpp
//
//	This program is a simple command line interface to the Alchemy
//	environment

#include "Alchemy.h"
#include "ALConsole.h"

class CCommandLineOptions
	{
	public:
		CString sInitFile;
	};

//	Forwards

ALERROR ParseCommandLine (CStringArray *pArgs, CCommandLineOptions *retOptions);

int alchemyConsoleMain (CStringArray *pArgs)

//	alchemyConsoleMain
//
//	This is the main entry point for a command line program

	{
	ALERROR error;
	CCodeChain CC;
	CCommandLineOptions Options;

	//	Boot the interpreter

	if (error = CC.Boot())
		{
		consolePrint(LITERAL("ERROR: Unable to boot CodeChain interpreter.\n"));
		return -1;
		}

	//	Start

	consolePrint(LITERAL("Aristotle v1.0\n"));
	consolePrint(LITERAL("CodeChain Interpreter\n\n"));

	//	Parse the command line

	if (error = ParseCommandLine(pArgs, &Options))
		return -1;

	//	If we're got an input path, parse it

	if (!Options.sInitFile.IsBlank())
		{
		ICCItem *pError;

		pError = CC.LoadInitFile(Options.sInitFile);
		if (pError->IsError())
			{
			CString sError;
			sError = CC.Unlink(pError);
			consolePrint(LITERAL("%s\n"), sError);
			consolePrint(LITERAL("ERROR: Unable to parse init file: %s\n"), Options.sInitFile);
			return -1;
			}

		pError->Discard(&CC);
		consolePrint(LITERAL("%s: loaded successfully\n"), Options.sInitFile);
		}

	//	Keep looping asking for input

	do
		{
		CString sInput;
		CString sOutput;
		ICCItem *pInput;
		ICCItem *pOutput;

		//	Ask for input

		sInput = consoleInput(LITERAL("> "));

		//	Are we done?

		if (strCompare(sInput, LITERAL("done")) == 0)
			break;

		//	Parse the input

		pInput = CC.Link(sInput, 0, NULL);

		//	Evaluate

		pOutput = CC.TopLevel(pInput);
		pInput->Discard(&CC);

		//	Output result

		sOutput = CC.Unlink(pOutput);
		pOutput->Discard(&CC);
		consolePrint(sOutput);
		consolePrint(LITERAL("\n"));
		}
	while (TRUE);

	return 0;
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

			if (retOptions->sInitFile.IsBlank())
				retOptions->sInitFile = sArg;
			}
		}

	return NOERROR;
	}

