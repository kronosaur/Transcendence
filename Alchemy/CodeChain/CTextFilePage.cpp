//	CTextFilePage.cpp
//
//	Implements CTextFilePage class
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include <windows.h>
#include "Kernel.h"
#include "KernelObjID.h"
#include "CodeChain.h"
#include "Pages.h"

#define TEXT_LINE_TYPE						CONSTLIT("textLine")

CTextFilePage::~CTextFilePage (void)

//	CTextFilePage destructor

	{
	if (m_pFile)
		delete m_pFile;
	}

bool CTextFilePage::EnumHasMore (SPageEnumCtx &Ctx)

//	EnumHasMore
//
//	Returns TRUE if we have more items

	{
	ASSERT(m_pFile);
	return (Ctx.dwPos1 < (DWORD)m_pFile->GetLength());
	}

ICCItem *CTextFilePage::EnumGetNext (CCodeChain &CC, SPageEnumCtx &Ctx)

//	EnumGetNext
//
//	Get the next entry. Must discard the return value (if not NULL)

	{
	char *pPos = m_pFile->GetPointer(Ctx.dwPos1);
	char *pEndPos = m_pFile->GetPointer(0) + m_pFile->GetLength();

	char *pStart = pPos;

	//	Look for the end of the line.

	while (pPos < pEndPos && *pPos != '\n' && *pPos != '\r')
		pPos++;

	//	Create the line

	ICCItem *pLine = CC.CreateString(CString(pStart, pPos - pStart));

	//	Skip to the beginning of the next line.

	if (pPos < pEndPos)
		{
		if (*pPos == '\r')
			{
			pPos++;
			if (*pPos == '\n')
				pPos++;
			}
		else if (*pPos == '\n')
			{
			pPos++;
			if (*pPos == '\r')
				pPos++;
			}
		}

	//	Set context

	Ctx.dwPos1 = pPos - m_pFile->GetPointer(0);

	//	Done

	return pLine;
	}

ALERROR CTextFilePage::EnumReset (CCodeChain &CC, ICCItem *pEnumType, SPageEnumCtx &Ctx, CString *retsError)

//	EnumReset
//
//	Resets the context and initializes based on the desired type
//
//	We understand the following enum types:
//
//	'textLine

	{
	ASSERT(m_pFile);

	if (pEnumType->IsIdentifier())
		{
		CString sValue = pEnumType->GetStringValue();
		if (strEquals(sValue, TEXT_LINE_TYPE))
			{
			Ctx.iType = enumTextLine;
			Ctx.dwPos1 = 0;
			}
		else
			{
			if (retsError)
				*retsError = strPatternSubst(CONSTLIT("Unknown enumeration type: %s."), sValue);
			return ERR_FAIL;
			}
		}
	else
		{
		if (retsError)
			*retsError = CONSTLIT("Unknown enumeration type.");
		return ERR_FAIL;
		}

	return NOERROR;
	}

ALERROR CTextFilePage::Open (const CString &sAddress, CString *retsError)

//	Open
//
//	Opens the page

	{
	ALERROR error;

	ASSERT(m_pFile == NULL);

	m_pFile = new CFileReadBlock(sAddress);
	if (error = m_pFile->Open())
		{
		if (retsError)
			*retsError = strPatternSubst("Unable to open file: %s", sAddress);
		return error;
		}

	return NOERROR;
	}
