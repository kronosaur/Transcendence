//	Main.cpp
//
//	Implements a console test client for the Grail Programming Language

#include <stdio.h>
#include "Alchemy.h"

#include "Labyrinth.h"

class CTextOut : public IReportProgress
	{
	public:
		virtual void Report (const CString &sLine)
			{
			printf("%s\n", sLine.GetASCIIZPointer());
			}
	};

void GrailMain (void);

int main (int argc, char *argv[ ], char *envp[ ])

//	main
//
//	main entry-point

	{
	if (!kernelInit())
		{
		printf("ERROR: Unable to initialize Alchemy kernel.\n");
		return 1;
		}

	//	Do it

	GrailMain();

	//	Done

	kernelCleanUp();
	return 0;
	}

void GrailMain (void)
	{
	ALERROR error;
	CTextOut Output;
	CLabyrinth Lab(&Output);

	//	Boot

	printf("Grail Language v1.0\n\n");

	if (error = Lab.Boot())
		{
		printf("%s\n", Lab.GetLastErrorMsg().GetASCIIZPointer());
		return;
		}

	printf("Grail ready\n\n");

	//	Command loop

	while (true)
		{
		//	Get user input

		CString sInput;
		printf(": ");
		gets(sInput.GetWritePointer(1024));
		sInput.Truncate(lstrlen(sInput.GetASCIIZPointer()));

		//	If this is the quit command, we're done

		if (strEquals(sInput, CONSTLIT("quit")))
			break;

		//	Process the command

		}

	//	Done

	Lab.Shutdown();
	printf("End session.");
	}
