//	Entities.cpp
//
//	Generate entities table

#include <stdio.h>

#include <windows.h>
#include "Alchemy.h"
#include "XMLUtil.h"
#include "TransData.h"

#define FILE_ATTRIB							CONSTLIT("file")

bool NextEntity (char **ioPos, char *pEnd, CString *retsName, DWORD *retdwValue);
bool NextEntityReference (char **ioPos, char *pEnd, CString *retsEntity);

void GenerateEntitiesTable (const CString &sDataFile, CXMLElement *pCmdLine)
	{
	int i;
	ALERROR error;

	CString sSourceFile;
	if (!pCmdLine->FindAttribute(FILE_ATTRIB, &sSourceFile))
		{
		printf("Specify file\n");
		return;
		}

	//	Open the data file and parse all entities

	CSymbolTable Entities(FALSE, TRUE);
	CFileReadBlock DataFile(CONSTLIT("Transcendence.xml"));

	if (error = DataFile.Open())
		{
		printf("Unable to open Transcendence.xml\n");
		return;
		}

	char *pPos = DataFile.GetPointer(0, -1);
	char *pEnd = pPos + DataFile.GetLength();

	//	Look for "<!ENTITY"

	CString sName;
	DWORD dwValue;
	while (NextEntity(&pPos, pEnd, &sName, &dwValue))
		Entities.AddEntry(sName, (CObject *)dwValue);

	DataFile.Close();

	//	Open the source file and look for all entity references

	CFileReadBlock SourceFile(sSourceFile);
	if (error = SourceFile.Open())
		{
		printf("Unable to open %s\n", sSourceFile.GetASCIIZPointer());
		return;
		}

	pPos = SourceFile.GetPointer(0, -1);
	pEnd = pPos + SourceFile.GetLength();

	//	Look for entity references

	CSymbolTable Output(TRUE, TRUE);
	while (NextEntityReference(&pPos, pEnd, &sName))
		{
		//	Look for this entity

		DWORD dwValue;
		if (Entities.Lookup(sName, (CObject **)&dwValue) == NOERROR)
			{
			//	Start with the entity keyword

			CString sOutput = strPatternSubst(CONSTLIT("\t<!ENTITY %s"), sName);

			//	Add tabs

			int iTabs = (40 - (sOutput.GetLength() + 3) + 3) / 4;
			if (iTabs)
				sOutput.Append(strRepeat(CONSTLIT("\t"), iTabs));
			else
				sOutput.Append(CONSTLIT(" "));

			//	Add the value

			char szBuffer[1024];
			wsprintf(szBuffer, "\"0x%08X\">\n", dwValue);
			sOutput.Append(CString(szBuffer));

			//	Output to the table (in UNID order)

			Output.AddEntry(CString(szBuffer), new CString(sOutput));
			}
		}

	SourceFile.Close();

	//	Output table

	for (i = 0; i < Output.GetCount(); i++)
		{
		CString *pLine = (CString *)Output.GetValue(i);
		printf(pLine->GetASCIIZPointer());
		}
	}

bool NextEntity (char **ioPos, char *pEnd, CString *retsName, DWORD *retdwValue)
	{
	char *pPos = *ioPos;

	while (true)
		{
		//	Look for '<'

		while (pPos < pEnd && *pPos != '<' && *pPos != ']')
			pPos++;

		if (pPos == pEnd || *pPos == ']')
			return false;

		//	Look for '!'

		pPos++;
		if (*pPos != '!')
			continue;

		//	Look for 'E'

		pPos++;
		if (*pPos != 'E')
			continue;

		//	Skip to whitespace

		while (pPos < pEnd && *pPos != ' ' && *pPos != '\t')
			pPos++;

		if (pPos == pEnd)
			return false;

		//	Skip to non-space

		while (pPos < pEnd && (*pPos == ' ' || *pPos == '\t'))
			pPos++;

		if (pPos == pEnd)
			return false;

		//	Skip to whitespace

		char *pStart = pPos;
		while (pPos < pEnd && *pPos != ' ' && *pPos != '\t')
			pPos++;

		if (pPos == pEnd)
			return false;

		*retsName = CString(pStart, pPos - pStart);

		//	Skip to quote

		while (pPos < pEnd && *pPos != '\"')
			pPos++;

		if (pPos == pEnd)
			return false;

		pPos++;
		*retdwValue = strParseInt(pPos, 0, &pPos);
		if (*retdwValue != 0)
			{
			*ioPos = pPos;
			return true;
			}
		}
	}

bool NextEntityReference (char **ioPos, char *pEnd, CString *retsEntity)
	{
	char *pPos = *ioPos;

	while (true)
		{
		//	Look for '&'

		while (pPos < pEnd && *pPos != '&')
			pPos++;

		if (pPos == pEnd)
			return false;

		pPos++;
		char *pStart = pPos;

		while (pPos < pEnd && *pPos != ';')
			pPos++;

		if (pPos == pEnd)
			return false;

		*retsEntity = CString(pStart, pPos - pStart);

		*ioPos = pPos;
		return true;
		}
	}
