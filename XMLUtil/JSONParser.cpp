//	JSONParser.cpp
//
//	JSON parser
//	Copyright (c) 2011 by Kronosaur Productions, LLC. All Rights Reserved.

#include <windows.h>
#include "Alchemy.h"
#include "JSONUtil.h"

class CJSONParser
	{
	public:
		CJSONParser (const CString &sBuffer);
		bool Parse (CJSONValue *retValue, CString *retsError);

	private:
		enum ETokens
			{
			tkError,
			tkValue,
			tkCloseBracket,
			tkCloseBrace,
			tkColon,
			tkComma,
			};

		ETokens ParseArray (CJSONValue *retValue);
		ETokens ParseLiteral (CJSONValue *retValue);
		ETokens ParseNumber (CJSONValue *retValue);
		ETokens ParseString (CJSONValue *retValue);
		ETokens ParseStruct (CJSONValue *retValue);
		ETokens ParseToken (CJSONValue *retValue);

		char *m_pPos;
		char *m_pPosEnd;
		CString m_sError;
	};

#include <math.h>

typedef TSortMap<CString, CJSONValue> ObjectType;
typedef TArray<CJSONValue> ArrayType;

ALERROR CJSONValue::Deserialize (const CString &sBuffer, CJSONValue *retValue, CString *retsError)

//	Deserialize
//
//	Parse a buffer into a value

	{
	CJSONParser Parser(sBuffer);
	if (!Parser.Parse(retValue, retsError))
		return ERR_FAIL;

	return NOERROR;
	}

void CJSONValue::Serialize (IWriteStream *pOutput) const

//	Serialize
//
//	Serialize to a UTF8 stream

	{
	int i;

	switch (m_iType)
		{
		case typeString:
			pOutput->Write("\"", 1);
			SerializeString(pOutput, strANSIToUTF8(AsString()));
			pOutput->Write("\"", 1);
			break;

		case typeNumber:
			{
			CString sNumber = strFromDouble(AsDouble());
			pOutput->Write(sNumber.GetASCIIZPointer(), sNumber.GetLength());
			break;
			}

		case typeObject:
			{
			ObjectType *pObj = (ObjectType *)m_pValue;
			pOutput->Write("{ ", 2);

			for (i = 0; i < pObj->GetCount(); i++)
				{
				if (i != 0)
					pOutput->Write(", ", 2);

				//	Write the key

				pOutput->Write("\"", 1);
				SerializeString(pOutput, strANSIToUTF8(pObj->GetKey(i)));
				pOutput->Write("\":", 2);

				//	Write the value

				pObj->GetValue(i).Serialize(pOutput);
				}

			pOutput->Write(" }", 2);
			break;
			}

		case typeArray:
			{
			ArrayType *pArray = (ArrayType *)m_pValue;
			pOutput->Write("[ ", 2);

			for (i = 0; i < pArray->GetCount(); i++)
				{
				if (i != 0)
					pOutput->Write(", ", 2);

				pArray->GetAt(i).Serialize(pOutput);
				}

			pOutput->Write(" ]", 2);
			break;
			}

		case typeTrue:
			pOutput->Write("true", 4);
			break;

		case typeFalse:
			pOutput->Write("false", 5);
			break;

		case typeNull:
			pOutput->Write("null", 4);
			break;

		default:
			ASSERT(false);
		}
	}

void CJSONValue::SerializeString (IWriteStream *pOutput, const CString &sText)

//	SerializeString
//
//	Serializes a utf8 string to JSON (excluding the surrounding quotes). If
//	bToUTF8 is TRUE, then we assume that the input string is a Windows Western
//	(CP1252) string and we convert it to utf8.

	{
	char *pPos = sText.GetASCIIZPointer();
	char *pPosEnd = pPos + sText.GetLength();

	//	Keep looping until we're done

	char *pStart = pPos;
	while (pPos < pPosEnd)
		{
		//	Look for characters that we need to escape

		if (*pPos == '\\' || *pPos == '\"' || strIsASCIIControl(pPos))
			{
			//	Write out what we've got so far

			pOutput->Write(pStart, (int)(pPos - pStart));

			//	Escape the character

			switch (*pPos)
				{
				case '\"':
					pOutput->Write("\\\"", 2);
					break;

				case '\\':
					pOutput->Write("\\\\", 2);
					break;

				case '\b':
					pOutput->Write("\\b", 2);
					break;

				case '\f':
					pOutput->Write("\\f", 2);
					break;

				case '\n':
					pOutput->Write("\\n", 2);
					break;

				case '\r':
					pOutput->Write("\\r", 2);
					break;

				case '\t':
					pOutput->Write("\\t", 2);
					break;

				default:
					{
					CString sChar = strPatternSubst("\\u%04x", (DWORD)*pPos);
					pOutput->Write(sChar.GetASCIIZPointer(), sChar.GetLength());
					break;
					}
				}

			pPos++;
			pStart = pPos;
			}
		else
			pPos++;
		}

	//	Write out the remainder

	pOutput->Write(pStart, (int)(pPos - pStart));
	}

//	CJSONParser ----------------------------------------------------------------

CJSONParser::CJSONParser (const CString &sBuffer)

//	CJSONParser constructor

	{
	m_pPos = sBuffer.GetASCIIZPointer();
	m_pPosEnd = m_pPos + sBuffer.GetLength();
	}

CJSONParser::ETokens CJSONParser::ParseArray (CJSONValue *retValue)

//	ParseArray
//
//	Parse an array

	{
	//	Skip the open bracket

	m_pPos++;

	//	Create a new array

	CJSONValue Array(CJSONValue::typeArray);

	//	Parse elements

	while (true)
		{
		CJSONValue Element;
		ETokens iToken = ParseToken(&Element);
		if (iToken == tkCloseBracket)
			break;
		else if (iToken == tkComma)
			;
		else if (iToken == tkValue)
			Array.InsertHandoff(Element);
		else
			{
			m_sError = CONSTLIT("Unable to parse array.");
			return tkError;
			}
		}

	//	Done

	retValue->TakeHandoff(Array);
	return tkValue;
	}

bool CJSONParser::Parse (CJSONValue *retValue, CString *retsError)

//	Parse
//
//	Parses the next element. Returns TRUE if successful and FALSE
//	if there was a parsing error.

	{
	if (ParseToken(retValue) != tkValue)
		{
		*retsError = m_sError;
		return false;
		}

	return true;
	}

CJSONParser::ETokens CJSONParser::ParseLiteral (CJSONValue *retValue)

//	ParseLiteral
//
//	Parse a literal

	{
	if (*m_pPos == 'f')
		{
		m_pPos++;
		if (*m_pPos == 'a')
			{
			m_pPos++;
			if (*m_pPos == 'l')
				{
				m_pPos++;
				if (*m_pPos == 's')
					{
					m_pPos++;
					if (*m_pPos == 'e')
						{
						m_pPos++;
						*retValue = CJSONValue(CJSONValue::typeFalse);
						return tkValue;
						}
					}
				}
			}
		}
	else if (*m_pPos == 'n')
		{
		m_pPos++;
		if (*m_pPos == 'u')
			{
			m_pPos++;
			if (*m_pPos == 'l')
				{
				m_pPos++;
				if (*m_pPos == 'l')
					{
					m_pPos++;
					*retValue = CJSONValue(CJSONValue::typeNull);
					return tkValue;
					}
				}
			}
		}
	else if (*m_pPos == 't')
		{
		m_pPos++;
		if (*m_pPos == 'r')
			{
			m_pPos++;
			if (*m_pPos == 'u')
				{
				m_pPos++;
				if (*m_pPos == 'e')
					{
					m_pPos++;
					*retValue = CJSONValue(CJSONValue::typeTrue);
					return tkValue;
					}
				}
			}
		}

	//	Error

	m_sError = CONSTLIT("Unknown literal value.");
	return tkError;
	}

CJSONParser::ETokens CJSONParser::ParseNumber (CJSONValue *retValue)

//	ParseNumber
//
//	Parse a JSON number

	{
	char *pStart = m_pPos;

	//	Parse the integer part

	int iSign = 1;
	if (*m_pPos == '-')
		{
		iSign = -1;
		m_pPos++;
		}

	char *pInt = m_pPos;
	while (*m_pPos >= '0' && *m_pPos <= '9')
		m_pPos++;

	char *pIntEnd = m_pPos;

	if (pInt == pIntEnd)
		{
		m_sError = CONSTLIT("Invalid number.");
		return tkError;
		}

	//	Do we have a fractional part?

	char *pFrac = NULL;
	char *pFracEnd = pIntEnd;
	if (*m_pPos == '.')
		{
		m_pPos++;

		pFrac = m_pPos;
		while (*m_pPos >= '0' && *m_pPos <= '9')
			m_pPos++;

		pFracEnd = m_pPos;
		if (pFrac == pFracEnd)
			{
			m_sError = CONSTLIT("Invalid floating point number.");
			return tkError;
			}
		}

	//	Do we have an exponential part?

	char *pExp = NULL;
	char *pExpEnd = pFracEnd;
	int iExpSign = 1;
	if (*m_pPos == 'e' || *m_pPos == 'E')
		{
		m_pPos++;

		if (*m_pPos == '+')
			m_pPos++;
		else if (*m_pPos == '-')
			{
			iExpSign = -1;
			m_pPos++;
			}

		pExp = m_pPos;
		while (*m_pPos >= '0' && *m_pPos <= '9')
			m_pPos++;

		pExpEnd = m_pPos;
		if (pExp == pExpEnd)
			{
			m_sError = CONSTLIT("Invalid exponential number.");
			return tkError;
			}
		}

	//	LATER: For now we just do everything with atof. Let we can be more careful
	//	about parsing integers separately.

	CString sNumber(pStart, (int)(pExpEnd - pStart));
	retValue->TakeHandoff(CJSONValue(atof(sNumber.GetASCIIZPointer())));
	return tkValue;
	}

CJSONParser::ETokens CJSONParser::ParseString (CJSONValue *retValue)

//	ParseString
//
//	Parse a JSON string

	{
	CMemoryWriteStream Stream(4096);
	if (Stream.Create() != NOERROR)
		{
		m_sError = CONSTLIT("Out of memory.");
		return tkError;
		}

	//	Skip the open quote

	m_pPos++;

	//	Keep looping

	while (m_pPos < m_pPosEnd && *m_pPos != '\"')
		{
		if (*m_pPos == '\\')
			{
			m_pPos++;

			switch (*m_pPos)
				{
				case '\"':
					Stream.Write("\"", 1);
					break;

				case '\\':
					Stream.Write("\\", 1);
					break;

				case '/':
					Stream.Write("/", 1);
					break;

				case 'b':
					Stream.Write("\b", 1);
					break;

				case 'f':
					Stream.Write("\f", 1);
					break;

				case 'n':
					Stream.Write("\n", 1);
					break;

				case 'r':
					Stream.Write("\r", 1);
					break;

				case 't':
					Stream.Write("\t", 1);
					break;

				case 'u':
					{
					char szBuffer[7];
					szBuffer[0] = '0';
					szBuffer[1] = 'x';
					szBuffer[2] = *++m_pPos;
					szBuffer[3] = *++m_pPos;
					szBuffer[4] = *++m_pPos;
					szBuffer[5] = *++m_pPos;
					szBuffer[6] = '\0';

					DWORD dwHex = strToInt(CString(szBuffer, 4), (int)'?');
					CString sChar = strEncodeUTF8Char(dwHex);
					Stream.Write(sChar.GetASCIIZPointer(), sChar.GetLength());
					break;
					}

				default:
					return tkError;
				}
			}
		else
			Stream.Write(m_pPos, 1);

		m_pPos++;
		}

	//	If we hit the end, then we have an error

	if (*m_pPos != '\"')
		return tkError;

	//	Otherwise, skip then end quote

	m_pPos++;

	//	Done

	retValue->TakeHandoff(CJSONValue(strUTF8ToANSI(CString(Stream.GetPointer(), Stream.GetLength()))));
	return tkValue;
	}

CJSONParser::ETokens CJSONParser::ParseStruct (CJSONValue *retValue)

//	ParseStruct
//
//	Parse a struct

	{
	//	Skip the open brace

	m_pPos++;

	//	Create a new array

	CJSONValue Object(CJSONValue::typeObject);

	//	Parse elements

	while (true)
		{
		CJSONValue Element;
		ETokens iToken = ParseToken(&Element);
		if (iToken == tkCloseBrace)
			break;
		else if (iToken == tkComma)
			continue;
		else if (iToken == tkValue)
			{
			CJSONValue Value;

			//	Parse the colon

			iToken = ParseToken(&Value);
			if (iToken != tkColon)
				{
				m_sError = CONSTLIT("Colon expected in object.");
				return tkError;
				}

			//	Parse the value

			iToken = ParseToken(&Value);
			if (iToken != tkValue)
				{
				m_sError = CONSTLIT("Value expected in object.");
				return tkError;
				}

			//	Add

			Object.InsertHandoff(Element.AsString(), Value);
			}
		else
			{
			m_sError = CONSTLIT("Key expected in object.");
			return tkError;
			}
		}

	//	Done

	retValue->TakeHandoff(Object);
	return tkValue;
	}

CJSONParser::ETokens CJSONParser::ParseToken (CJSONValue *retValue)

//	ParseToken
//
//	Parse a value or token. We expect that m_pPos is initialized
//	at the first character for us to parse. 
//
//	We leave m_pPos on the first character
//	AFTER the token (or on the character that caused an error).

	{
	//	Skip Parse whitespace

	while (*m_pPos == ' ' || *m_pPos == '\t' || *m_pPos == '\r' || *m_pPos == '\n')
		m_pPos++;

	//	Parse token

	switch (*m_pPos)
		{
		case '\0':
			//	Unexpected end of file
			m_sError = CONSTLIT("Unexpected end of stream.");
			return tkError;

		case ':':
			m_pPos++;
			return tkColon;

		case '{':
			return ParseStruct(retValue);

		case '}':
			m_pPos++;
			return tkCloseBrace;

		case '[':
			return ParseArray(retValue);

		case ']':
			m_pPos++;
			return tkCloseBracket;

		case ',':
			m_pPos++;
			return tkComma;

		case '\"':
			return ParseString(retValue);

		default:
			{
			if (*m_pPos == '-' || (*m_pPos >= '0' && *m_pPos <= '9'))
				return ParseNumber(retValue);
			else
				return ParseLiteral(retValue);
			}
		}
	}
