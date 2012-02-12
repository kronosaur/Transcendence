//	Main.cpp
//
//	Main program file for Alchemy console program

#include <StdIO.h>
#include "Alchemy.h"
#include "ALConsole.h"

//	User is expected to supply this function

int alchemyConsoleMain (CStringArray *pArgs);

int main (int argc, char *argv[], char **envp)

//	main
//
//	main C program entry-point

	{
	ALERROR error;
	int i;
	int iResult;
	CStringArray *pArgs;

	//	Initialize the Kernel

	if (!kernelInit())
		{
		printf("Alchemy: Unable to initialize kernel.\n");
		exit(1);
		}

	//	Place the arguments in a string array

	pArgs = new CStringArray;
	if (pArgs == NULL)
		{
		printf("Alchemy: Out of memory.\n");
		exit(1);
		}

	//	The first argument is the name of the program being launched

	for (i = 1; i < argc; i++)
		if (error = pArgs->AppendString(LITERAL(argv[i]), NULL))
			{
			printf("Alchemy: Unable to start console (error %d).\n", error);
			delete pArgs;
			exit(1);
			}

	//	Call the main console entry point

	iResult = alchemyConsoleMain(pArgs);

	//	Done with the kernel

	delete pArgs;
	kernelCleanUp();

	return iResult;
	}

CString consoleInput (CString sPrompt)

//	consoleInput
//
//	Input a line of text

	{
	char szBuffer[2048];

	printf(sPrompt.GetASCIIZPointer());
	gets(szBuffer);
	return LITERAL(szBuffer);
	}

void consolePrint (CString sLine, ...)

//	consolePrint
//
//	Prints a string to the console

	{
	char *pArgs;
	CString sParsedLine;

	pArgs = (char *) &sLine + sizeof(sLine);
	sParsedLine = strPattern(sLine, (void **)pArgs);
	printf(sParsedLine.GetASCIIZPointer());
	}
