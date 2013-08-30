//	Misc.cpp
//
//	Miscellaneous functions

#include <windows.h>
#include "Alchemy.h"
#include "XMLUtil.h"

#define NOARGS								CONSTLIT("noArgs")
#define QUESTION_MARK_SWITCH				CONSTLIT("?")
#define HELP_SWITCH							CONSTLIT("help")
#define H_SWITCH							CONSTLIT("h")

enum ParseCmdState
	{
	stateStart,
	stateSwitch,
	stateParam,
	stateParamStart,
	stateParamQuoted,
	stateArg,
	stateArgQuoted,
	stateDone,
	};

ALERROR CreateXMLElementFromCommandLine (int argc, char *argv[], CXMLElement **retpElement)

//	CreateXMLElementFromCommandLine
//
//	Parses the command line into an element of the following form:
//
//	<PROGRAM-NAME
//			SWITCH1="true"
//			SWITCH2="*.*"
//			help="true"
//			noArgs="true"
//			>
//		arg1<br/>
//		arg2<br/>
//		...
//		argn<br/>
//	</PROGRAM-NAME>
//
//	PROGRAM-NAME is the name of the program (argv[0])
//	SWITCH1-N are arguments preceeded by a "/" or "-". The text after the
//		switch character is used after the argument name. If there is a colon
//		at the end of the switch name then we expect a value for the switch
//		(rather than true or false)
//	help is set to true if the command line contains /? or -? or /help or -help
//	noArgs is set to true if there are no switches or arguments in the commandline
//	arg1-argn are the parameters without leading "/" or "-"
//
//	The caller must free the resulting element

	{
	CXMLElement *pCmdLine = new CXMLElement(CString(argv[0]), NULL);
	if (pCmdLine == NULL)
		return ERR_MEMORY;

	//	Parse the command line

	int iArg = 1;
	bool bNoArgs = true;
	while (iArg < argc)
		{
		char *pPos = argv[iArg];
		ParseCmdState iState = stateStart;
		char *pStart;
		CString sToken;

		while (iState != stateDone)
			{
			switch (iState)
				{
				case stateStart:
					{
					if (*pPos == '/' || *pPos == '-')
						{
						iState = stateSwitch;
						pPos++;
						pStart = pPos;
						}
					else if (*pPos == ' ' || *pPos == '\t')
						pPos++;
					else if (*pPos == '\0')
						iState = stateDone;
					else if (*pPos == '\"')
						{
						pPos++;
						iState = stateArgQuoted;
						pStart = pPos;
						}
					else
						{
						iState = stateArg;
						pStart = pPos;
						}
					break;
					}

				case stateSwitch:
					{
					if (*pPos == '/' || *pPos == ' ' || *pPos == '\0')
						{
						sToken = CString(pStart, (pPos - pStart));
						if (strEquals(sToken, CONSTLIT("?"))
								|| strEquals(sToken, CONSTLIT("help")))
							{
							pCmdLine->AddAttribute(CONSTLIT("help"), CONSTLIT("true"));
							bNoArgs = false;
							}
						else if (!sToken.IsBlank())
							{
							pCmdLine->AddAttribute(sToken, CONSTLIT("true"));
							bNoArgs = false;
							}
						iState = stateStart;
						}
					else if (*pPos == ':')
						{
						sToken = CString(pStart, (pPos - pStart));
						iState = stateParamStart;
						pPos++;
						}
					else
						pPos++;
					break;
					}

				case stateParam:
					{
					if (*pPos == '\0')
						{
						CString sParam(pStart, (pPos - pStart));
						pCmdLine->AddAttribute(sToken, sParam);
						bNoArgs = false;
						iState = stateStart;
						}
					else
						pPos++;
					break;
					}

				case stateParamQuoted:
					{
					if (*pPos == '\"' || *pPos == '\0')
						{
						CString sParam(pStart, (pPos - pStart));
						pCmdLine->AddAttribute(sToken, sParam);
						bNoArgs = false;
						iState = stateStart;

						if (*pPos == '\"')
							pPos++;
						}
					else
						pPos++;
					break;
					}

				case stateParamStart:
					{
					if (*pPos == '\"')
						{
						iState = stateParamQuoted;
						pPos++;
						pStart = pPos;
						}
					else if (*pPos == ' ')
						pPos++;
					else if (*pPos == '\0')
						{
						iState = stateStart;
						}
					else
						{
						pStart = pPos;
						iState = stateParam;
						}
					break;
					}

				case stateArg:
					{
					if (*pPos == ' ' || *pPos == '\0')
						{
						pCmdLine->AppendContent(CString(pStart, (pPos - pStart)));

						CXMLElement *pBR = new CXMLElement(CONSTLIT("BR"), NULL);
						pCmdLine->AppendSubElement(pBR);

						bNoArgs = false;
						iState = stateStart;
						}
					else
						pPos++;
					break;
					}

				case stateArgQuoted:
					{
					if (*pPos == '\"' || *pPos == '\0')
						{
						pCmdLine->AppendContent(CString(pStart, (pPos - pStart)));

						CXMLElement *pBR = new CXMLElement(CONSTLIT("BR"), NULL);
						pCmdLine->AppendSubElement(pBR);

						if (*pPos == '\"')
							pPos++;

						bNoArgs = false;
						iState = stateStart;
						}
					else
						pPos++;
					break;
					}

				default:
					ASSERT(false);
				}
			}

		//	Next
		iArg++;
		}

	//	If no args, add the flag

	if (bNoArgs)
		pCmdLine->AddAttribute(CONSTLIT("noArgs"), CONSTLIT("true"));

	//	Done

	*retpElement = pCmdLine;
	return NOERROR;
	}

ALERROR ParseAttributeIntegerList (const CString &sValue, CIntArray *pList)

//	ParseAttributeIntegerList
//
//	Parses a string into an integer list

	{
	ALERROR error;
	char *pPos = sValue.GetPointer();

	while (*pPos != '\0')
		{
		//	Skip non-numbers

		while (*pPos != '\0' && (*pPos < '0' || *pPos > '9') && *pPos != '-' && *pPos != '+')
			pPos++;

		//	Parse an integer

		if (*pPos != '\0')
			{
			bool bNull;
			int iInt;
			iInt = strParseInt(pPos, 0, &pPos, &bNull);

			//	If we have a valid integer then add it

			if (!bNull)
				{
				if (error = pList->AppendElement(iInt, NULL))
					return error;
				}
			else
				break;
			}
		}

	return NOERROR;
	}
