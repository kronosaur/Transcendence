//	CFormulaText.cpp
//
//	CFormulaText class

#include "PreComp.h"

ALERROR CFormulaText::InitFromString (SDesignLoadCtx &Ctx, const CString &sText)

//	InitFromString
//
//	Initialize

	{
	m_sText = sText;
	return NOERROR;
	}

int CFormulaText::EvalAsInteger (CSpaceObject *pSource, CString *retsPrefix, CString *retsSuffix, CString *retsError) const

//	EvalAsInteger
//
//	Evaluates the formula and returns integer

	{
	char *pPos = m_sText.GetASCIIZPointer();

	//	Skip whitespace

	while (strIsWhitespace(pPos))
		pPos++;

	//	A leading = means a formula

	if (*pPos == '=')
		{
		CCodeChainCtx Ctx;

		//	Save old gSource variable

		Ctx.SaveAndDefineSourceVar(pSource);

		//	Link and evaluate the code

		ICCItem *pExp = Ctx.Link(m_sText, 1, NULL);
		ICCItem *pResult = Ctx.Run(pExp);	//	LATER:Event
		Ctx.Discard(pExp);

		//	Check for and return error

		if (pResult->IsError())
			{
			CString sError = pResult->GetStringValue();
			if (retsError)
				*retsError = sError;

			kernelDebugLogMessage(sError);
			Ctx.Discard(pResult);
			return 0;
			}

		//	Done

		int iResult = pResult->GetIntegerValue();
		Ctx.Discard(pResult);
		return iResult;
		}

	//	If we have a number, then return it

	else if (*pPos == '+' || *pPos == '-' || (*pPos >= '0' && *pPos <= '9'))
		{
		return strToInt(m_sText, 0);
		}

	//	Otherwise, see if we have a prefix

	else
		{
		char *pStart = pPos;
		while (*pPos != '\0' && *pPos != ':' && !strIsWhitespace(pPos))
			pPos++;

		//	If blank, we're done

		if (pStart == pPos)
			return 0;

		//	Otherwise, we have a prefix

		if (retsPrefix)
			*retsPrefix = CString(pStart, pPos - pStart);

		//	Skip to number

		while (*pPos != '\0' && *pPos != '+' && *pPos != '-' && (*pPos < '0' || *pPos > '9'))
			pPos++;

		//	OK if we don't have a number

		if (*pPos == '\0')
			return 0;

		int iValue = strParseInt(pPos, 0, &pPos);

		//	LATER: Get the suffix

		//	Done

		return iValue;
		}
	}

void CFormulaText::ReadFromStream (SLoadCtx &Ctx)

//	ReadFromStream
//
//	Read

	{
	m_sText.ReadFromStream(Ctx.pStream);
	}

void CFormulaText::WriteToStream (IWriteStream *pStream) const

//	WriteToStream
//
//	Write

	{
	m_sText.WriteToStream(pStream);
	}
