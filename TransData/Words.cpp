//	Words.cpp
//
//	Generate statistics about text

#include <stdio.h>

#include <windows.h>
#include "Alchemy.h"
#include "XMLUtil.h"
#include "TransData.h"

#define MODULES_TAG							(CONSTLIT("Modules"))

#define COUNT_ATTRIB						(CONSTLIT("count"))
#define FILENAME_ATTRIB						(CONSTLIT("filename"))
#define NOVEL_ATTRIB						(CONSTLIT("novel"))

struct TraverseCtx
	{
	CSymbolTable *pWordList;
	};

void AddText (TraverseCtx &Ctx, const CString &sText);
void AddWord (TraverseCtx &Ctx, const CString &sWord);
void ParseWordList (TraverseCtx &Ctx, CXMLElement *pElement);

void GenerateWordList (const CString &sDataFile, CXMLElement *pCmdLine)

//	GenerateWordList
//
//	Generate a list of unique words used in the game

	{
	ALERROR error;
	int i;
	CString sError;

	//	Open the XML file

	CResourceDb Resources(sDataFile);
	if (error = Resources.Open(0, &sError))
		{
		printf("%s\n", (LPSTR)sError);
		return;
		}

	CXMLElement *pGameFile;
	if (error = Resources.LoadGameFile(&pGameFile, NULL, &sError))
		{
		printf("%s\n", sError.GetASCIIZPointer());
		return;
		}

	//	Create the context

	CSymbolTable WordList(FALSE, TRUE);
	TraverseCtx Ctx;
	Ctx.pWordList = &WordList;

	//	Recursive descent

	ParseWordList(Ctx, pGameFile);

	//	Parse all modules too

	CXMLElement *pModules = pGameFile->GetContentElementByTag(MODULES_TAG);
	if (pModules)
		{
		for (i = 0; i < pModules->GetContentElementCount(); i++)
			{
			CXMLElement *pModule = pModules->GetContentElement(i);
			CXMLElement *pModuleXML;
			if (error = Resources.LoadModule(NULL_STR, pModule->GetAttribute(FILENAME_ATTRIB), &pModuleXML, &sError))
				{
				printf("%s\n", sError.GetASCIIZPointer());
				return;
				}

			ParseWordList(Ctx, pModuleXML);
			}
		}

	//	Print out the word list

	for (i = 0; i < WordList.GetCount(); i++)
		printf("%s\n", WordList.GetKey(i).GetASCIIZPointer());
	}

void AddText (TraverseCtx &Ctx, const CString &sText)
	{
	enum States
		{
		stateInText,
		stateInDelimeter,
		stateDone,
		};

	//	Parse the text into words

	char *pPos = sText.GetASCIIZPointer();
	char *pStart = pPos;
	int iState = stateInText;
	CString sWord;

	while (iState != stateDone)
		{
		bool bText = strIsAlphaNumeric(pPos);

		switch (iState)
			{
			case stateInText:
				{
				if (!bText)
					{
					AddWord(Ctx, CString(pStart, pPos - pStart));
					iState = stateInDelimeter;
					}
				break;
				}

			case stateInDelimeter:
				{
				if (bText)
					{
					pStart = pPos;
					iState = stateInText;
					}
				break;
				}
			}

		//	Next

		if (*pPos == '\0')
			iState = stateDone;
		else
			pPos++;
		}
	}

bool isAllNumbers (const CString &sWord)
	{
	char *pPos = sWord.GetASCIIZPointer();
	while (*pPos != '\0')
		{
		if (*pPos < '0' || *pPos > '9')
			return false;

		pPos++;
		}

	return true;
	}

void AddWord (TraverseCtx &Ctx, const CString &sWord)
	{
	//	If this is a single character, then skip it

	if (sWord.GetLength() == 1)
		;

	//	Skip words that are all numbers and hex numbers

	else if (isAllNumbers(sWord) || strFind(sWord, CONSTLIT("0x")) == 0)
		;

	//	Otherwise, add it

	else
		{
		Ctx.pWordList->AddEntry(strToLower(sWord), NULL);
		}
	}

void ParseWordList (TraverseCtx &Ctx, CXMLElement *pElement)
	{
	int i;

	//	Parse all attributes

	for (i = 0; i < pElement->GetAttributeCount(); i++)
		AddText(Ctx, pElement->GetAttribute(i));

	//	If this has sub-elements, then recurse

	if (pElement->GetContentElementCount())
		{
		for (i = 0; i < pElement->GetContentElementCount(); i++)
			ParseWordList(Ctx, pElement->GetContentElement(i));
		}

	//	Otherwise, add the content 

	else
		{
		AddText(Ctx, pElement->GetContentText(0));
		}
	}

void WordGenerator (CXMLElement *pCmdLine)
	{
	int i;

	//	Load input file

	CString sFilespec = pCmdLine->GetAttribute(CONSTLIT("input"));
	if (sFilespec.IsBlank())
		{
		printf("ERROR: input filename expected.\n");
		return;
		}

	CFileReadBlock InputFile(sFilespec);
	if (InputFile.Open() != NOERROR)
		{
		printf("ERROR: Unable to open file: %s\n", sFilespec.GetASCIIZPointer());
		return;
		}

	//	"Novel" means that we only generate words that are not
	//	in the input file.

	bool bNovelWordsOnly = pCmdLine->GetAttributeBool(NOVEL_ATTRIB);

	//	Build up a word generator

	CMarkovWordGenerator Generator;
	TMap<CString, DWORD> InputWords;

	//	Read each line of the file

	char *pPos = InputFile.GetPointer(0);
	char *pEndPos = pPos + InputFile.GetLength();
	while (pPos < pEndPos)
		{
		//	Skip whitespace

		while (pPos < pEndPos && (strIsWhitespace(pPos) || *pPos < ' '))
			pPos++;

		//	Parse the line

		char *pStart = pPos;
		while (pPos < pEndPos && *pPos != '\r' && *pPos != '\n' && *pPos >= ' ')
			pPos++;

		CString sWord(pStart, pPos - pStart);

		//	Add the word to the generator

		if (!sWord.IsBlank())
			{
			Generator.AddSample(strTrimWhitespace(sWord));

			//	If we are looking for novel words we need to keep a map
			//	of all words in the input file.

			if (bNovelWordsOnly)
				InputWords.Insert(sWord);
			}
		}

	//	If we have a count, then output a list of random words

	int iCount;
	if (pCmdLine->FindAttributeInteger(COUNT_ATTRIB, &iCount))
		{
		if (iCount > 0)
			{
			TArray<CString> Result;
			Generator.GenerateUnique(iCount, &Result);

			for (i = 0; i < Result.GetCount(); i++)
				if (InputWords.Find(Result[i]))
					{
					Result.Delete(i);
					i--;
					}

			Result.Sort();

			for (i = 0; i < Result.GetCount(); i++)
				printf("%s\n", Result[i].GetASCIIZPointer());
			}
		}

	//	Otherwise, output the generator as XML

	else
		{
		CMemoryWriteStream Output;
		if (Output.Create() != NOERROR)
			{
			printf("ERROR: Out of memory.\n");
			return;
			}

		if (Generator.WriteAsXML(&Output) != NOERROR)
			{
			printf("ERROR: Unable to output generator as XML.\n");
			return;
			}

		Output.Write("\0", 1);
		printf(Output.GetPointer());
		}
	}
