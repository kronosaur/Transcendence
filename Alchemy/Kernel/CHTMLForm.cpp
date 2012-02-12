//	CHTMLForm.cpp
//
//	CHTMLForm class
//
//	The encoding is as follows:
//
//	1.	All spaces are converted to '+' (in both keys and values)
//	2.	All non-alphanumerics are URL encoded (e.g., %2B).
//	3.	Keys and values are separated by '='
//	4.	Key value pairs are separated by '&'
//	5.	Duplicate keys are allowed.

#include "Kernel.h"
#include "KernelObjID.h"

#include "Internets.h"

void CHTMLForm::AddField (const CString &sKey, const CString &sValue)

//	AddField
//
//	Adds a field to the form

	{
	SField *pField = m_Fields.Insert();
	pField->sKey = sKey;
	pField->sValue = sValue;
	}

ALERROR CHTMLForm::DecodeFromBuffer (const CString &sMediaType, const CString &sBuffer)

//	DecodeFromBuffer
//
//	Initializes the form from the given encoded buffer

	{
	//	Not yet implemented
	return ERR_FAIL;
	}

ALERROR CHTMLForm::EncodeText (IWriteStream *pOutput, const CString &sText) const

//	EncodeText
//
//	Encodes the text to output buffer

	{
	ALERROR error;
	char *pPos = sText.GetASCIIZPointer();
	char *pEndPos = pPos + sText.GetLength();

	while (pPos < pEndPos)
		{
		if ((*pPos >= 'A' && *pPos <= 'Z')
				|| (*pPos >= 'a' && *pPos <= 'z')
				|| (*pPos >= '0' && *pPos <= '9'))
			{
			if (error = pOutput->Write(pPos, 1))
				return error;
			}
		else if (*pPos == ' ')
			{
			if (error = pOutput->Write("+", 1))
				return error;
			}
		else
			{
			CString sEscape = strPatternSubst(CONSTLIT("%%%02X"), (DWORD)*pPos);
			if (error = pOutput->Write(sEscape.GetASCIIZPointer(), sEscape.GetLength()))
				return error;
			}

		pPos++;
		}

	return NOERROR;
	}

ALERROR CHTMLForm::EncodeToBuffer (IWriteStream *pOutput) const

//	EncodeToBuffer
//
//	Encodes the form to the buffer

	{
	ALERROR error;
	int i;

	for (i = 0; i < m_Fields.GetCount(); i++)
		{
		//	Output field separator

		if (i != 0)
			if (error = pOutput->Write("&", 1))
				return error;

		//	Output key

		if (error = EncodeText(pOutput, m_Fields[i].sKey))
			return error;

		//	Output key/value separator

		if (error = pOutput->Write("=", 1))
			return error;

		//	Output value

		if (error = EncodeText(pOutput, m_Fields[i].sValue))
			return error;
		}

	return NOERROR;
	}

DWORD CHTMLForm::GetEncodedTextLength (const CString &sText) const

//	GetEncodedTextLength
//
//	Returns the length that the given text would be encoded (in bytes)

	{
	char *pPos = sText.GetASCIIZPointer();
	char *pEndPos = pPos + sText.GetLength();

	DWORD dwCount = 0;
	while (pPos < pEndPos)
		{
		if ((*pPos >= 'A' && *pPos <= 'Z')
				|| (*pPos >= 'a' && *pPos <= 'z')
				|| (*pPos >= '0' && *pPos <= '9')
				|| *pPos == ' ')
			dwCount++;
		else
			dwCount += 3;

		pPos++;
		}

	return dwCount;
	}

DWORD CHTMLForm::GetMediaLength (void) const

//	GetMediaLength
//
//	Returns the size of the resulting encoded buffer in bytes

	{
	int i;

	DWORD dwCount = 0;
	for (i = 0; i < m_Fields.GetCount(); i++)
		{
		if (i != 0)
			dwCount++;

		dwCount += GetEncodedTextLength(m_Fields[i].sKey) + 1 + GetEncodedTextLength(m_Fields[i].sValue);
		}

	return dwCount;
	}
