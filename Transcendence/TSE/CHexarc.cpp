//	CHexarc.cpp
//
//	CHexarc class
//	Copyright (c) 2012 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"

#define TYPE_AEON_BINARY						CONSTLIT("AEON2011:binary:v1")
#define TYPE_AEON_ERROR							CONSTLIT("AEON2011:hexeError:v1")
#define TYPE_AEON_IPINTEGER						CONSTLIT("AEON2011:ipInteger:v1")

#define ERR_PASSWORD_MUST_BE_LONGER				CONSTLIT("Passwords must be at least 6 characters.")

bool CHexarc::ConvertIPIntegerToString (const CJSONValue &Value, CString *retsValue)

//	ConvertIPIntegerToString
//
//	Converts an AEON CIPInteger encoded in JSON to a string buffer of raw bytes.

	{
	if (!strEquals(Value.GetElement(0).AsString(), TYPE_AEON_IPINTEGER))
		return false;

	const CString &sEncoded = Value.GetElement(1).AsString();
	CMemoryReadStream Stream(sEncoded.GetASCIIZPointer(), sEncoded.GetLength());
	if (Stream.Open() != NOERROR)
		return false;

	CBase64Decoder Decoder(&Stream);
	if (Decoder.Open() != NOERROR)
		return false;
	
	//	The first DWORD is a signature (which we can ignore)

	DWORD dwLoad;
	Decoder.Read((char *)&dwLoad, sizeof(DWORD));

	//	Next is the size

	DWORD dwSize;
	Decoder.Read((char *)&dwSize, sizeof(DWORD));

	//	Allocate enough room on the string

	char *pDest = retsValue->GetWritePointer(dwSize);

	//	Read

	Decoder.Read(pDest, dwSize);

	//	Done

	return true;
	}

bool CHexarc::ConvertToDigest (const CJSONValue &Value, CDigest *retDigest)

//	ConvertToDigest
//
//	Converts an AEON CIPInteger encoded in JSON to a CDigest object.

	{
	CIntegerIP Result;
	if (!ConvertToIntegerIP(Value, &Result))
		return false;

	//	Make sure we are the right size

	if (Result.GetLength() != 20)
		return false;

	//	Move to digest

	retDigest->TakeHandoff(Result);

	//	Done

	return true;
	}

bool CHexarc::ConvertToIntegerIP (const CJSONValue &Value, CIntegerIP *retValue)

//	ConvertToIntegerIP
//
//	Converts an AEON CIPInteger encoded in JSON to a CIntegerIP object.

	{
	if (!strEquals(Value.GetElement(0).AsString(), TYPE_AEON_IPINTEGER))
		return false;

	const CString &sEncoded = Value.GetElement(1).AsString();
	CMemoryReadStream Stream(sEncoded.GetASCIIZPointer(), sEncoded.GetLength());
	if (Stream.Open() != NOERROR)
		return false;

	CBase64Decoder Decoder(&Stream);
	if (Decoder.Open() != NOERROR)
		return false;
	
	//	The first DWORD is a signature (which we can ignore)

	DWORD dwLoad;
	Decoder.Read((char *)&dwLoad, sizeof(DWORD));

	//	Next is the size

	DWORD dwSize;
	Decoder.Read((char *)&dwSize, sizeof(DWORD));

	//	Load the bytes

	CIntegerIP Result(dwSize);
	Decoder.Read((char *)Result.GetBytes(), dwSize);

	//	Return

	retValue->TakeHandoff(Result);

	//	Done

	return true;
	}

bool CHexarc::ConvertToJSON (const CIntegerIP &Value, CJSONValue *retValue)

//	ConvertToJSON
//
//	Converts a CIntegerIP to an AEON CIPInteger encoded in JSON

	{
	//	First we generate the encoded CIPInteger

	CMemoryWriteStream Buffer;
	if (Buffer.Create() != NOERROR)
		return false;

	CBase64Encoder Encoder(&Buffer);
	if (Encoder.Create() != NOERROR)
		return false;

	//	Signature

	DWORD dwSave = 'IP1+';
	Encoder.Write((char *)&dwSave, sizeof(DWORD));

	//	Size

	dwSave = Value.GetLength();
	Encoder.Write((char *)&dwSave, sizeof(DWORD));

	//	Bytes

	Encoder.Write((char *)Value.GetBytes(), dwSave);

	//	Done

	Encoder.Close();

	//	Create the array

	CJSONValue IPInteger(CJSONValue::typeArray);
	IPInteger.InsertHandoff(CJSONValue(TYPE_AEON_IPINTEGER));
	IPInteger.InsertHandoff(CJSONValue(CString(Buffer.GetPointer(), Buffer.GetLength())));

	//	Done

	retValue->TakeHandoff(IPInteger);
	return true;
	}

bool CHexarc::CreateCredentials (const CString &sUsername, const CString &sPassword, CJSONValue *retValue)

//	CreateCredentials
//
//	Creates credentials for signing in to Hexarc

	{
	return CHexarc::ConvertToJSON(CDigest(CBufferReadBlock(strPatternSubst(CONSTLIT("%s:HEXARC01:%s"), strToLower(sUsername), sPassword))), retValue);
	}

bool CHexarc::CreateCredentials (const CString &sUsername, const CString &sPassword, CString *retsValue)

//	CreateCredentials
//
//	Creates credentials for signing in to Hexarc

	{
	CDigest PasswordHash(CBufferReadBlock(strPatternSubst("%s:HEXARC01:%s", strToLower(sUsername), sPassword)));
	*retsValue = CString((char *)PasswordHash.GetBytes(), PasswordHash.GetLength());
	return true;
	}

CString CHexarc::GetFilenameFromFilePath (const CString &sFilePath)

//	GetFilenameFromFilePath
//
//	Parses a Multiverse filePath and returns the filename.

	{
	char *pPosStart = sFilePath.GetASCIIZPointer();
	char *pPos = pPosStart + sFilePath.GetLength() - 1;

	//	Back up until we see the first slash.

	while (pPos >= pPosStart && *pPos != '/')
		pPos--;

	//	Found

	if (*pPos == '/')
		return CString(pPos + 1);

	//	Never found slash; return entire string

	return sFilePath;
	}

bool CHexarc::HasSpecialAeonChars (const CString &sValue)

//	HasSpecialAeonChars
//
//	Returns TRUE if the string has characters that are considered special by
//	Aeon. [If so, then the string needs to be quoted.]

	{
	char *pPos = sValue.GetASCIIZPointer();
	char *pEndPos = pPos + sValue.GetLength();

	//	If we start with a number, then we need quotes

	if (strIsDigit(pPos))
		return true;

	//	Check for reserved characters

	while (pPos < pEndPos)
		{
		//	We check for alpha first because it is faster (and most common)

		if (strIsASCIIAlpha(pPos))
			;

		//	Underscores and periods are OK

		else if (*pPos == '_' || *pPos == '.')
			;

		//	Otherwise...

		else if (*pPos == ' ' || strIsASCIIControl(pPos) || strIsASCIISymbol(pPos))
			return true;

		pPos++;
		}

	return false;
	}

bool CHexarc::IsBinary (const CJSONValue &Value, CString *retsData)

//	IsBinary
//
//	Returns TRUE if the JSON value is a binary value (and optionally returns
//	it).

	{
	if (!strEquals(Value.GetElement(0).AsString(), TYPE_AEON_BINARY))
		return false;

	if (retsData == NULL)
		return true;

	const CString &sEncoded = Value.GetElement(1).AsString();
	CMemoryReadStream Stream(sEncoded.GetASCIIZPointer(), sEncoded.GetLength());
	if (Stream.Open() != NOERROR)
		return false;

	CBase64Decoder Decoder(&Stream);
	if (Decoder.Open() != NOERROR)
		return false;

	//	Read the length

	DWORD dwLength;
	Decoder.Read((char *)&dwLength, sizeof(DWORD));
	if (dwLength == 0)
		{
		*retsData = NULL_STR;
		return true;
		}

	//	Read the buffer

	char *pPos = retsData->GetWritePointer(dwLength);
	Decoder.Read(pPos, dwLength);

	//	Done

	return true;
	}

bool CHexarc::IsError (const CJSONValue &Value, CString *retsError, CString *retsDesc)

//	IsError
//
//	Returns TRUE if the JSON value is an error

	{
	if (!strEquals(Value.GetElement(0).AsString(), TYPE_AEON_ERROR))
		return false;

	const CString &sEncoded = Value.GetElement(1).AsString();
	CMemoryReadStream Stream(sEncoded.GetASCIIZPointer(), sEncoded.GetLength());
	if (Stream.Open() != NOERROR)
		return false;

	CBase64Decoder Decoder(&Stream);
	if (Decoder.Open() != NOERROR)
		return false;

	//	Read the strings

	if (retsError)
		retsError->ReadFromStream(&Decoder);

	if (retsDesc)
		{
		if (retsError == NULL)
			{
			CString sDummy;
			sDummy.ReadFromStream(&Decoder);
			}

		retsDesc->ReadFromStream(&Decoder);
		}

	//	Done

	return true;
	}

bool CHexarc::ValidatePasswordComplexity (const CString &sPassword, CString *retsResult)

//	ValidatePasswordComplexity
//
//	Make sure that the password is complex enough.

	{
	if (sPassword.GetLength() < 6)
		{
		if (retsResult)
			*retsResult = ERR_PASSWORD_MUST_BE_LONGER;
		return false;
		}

	return true;
	}

void CHexarc::WriteAsAeon (const CJSONValue &Value, IWriteStream &Stream)

//	WriteAsAeon
//
//	Write in AEON format

	{
	int i;

	switch (Value.GetType())
		{
		case CJSONValue::typeString:
			{
			//	NOTE: When we convert an empty string to JSON it gets encoded 
			//	as a JSON empty string (not as null). This means that when we
			//	serialize, we serialize to an empty string.
			//
			//	At deserialize time, Hexarc will convert the empty string into
			//	an empty string datum (not nil).
			//
			//	Thus if we're trying to write out an empty JSON string to
			//	serialized AEON format, we need to write out an empty string.

			CString sValue = Value.AsString();
			if (sValue.IsBlank())
				Stream.Write("\"\"", 2);
			else
				{
				bool bQuote = HasSpecialAeonChars(sValue);
				if (bQuote)
					Stream.Write("\"", 1);

				CJSONValue::SerializeString(&Stream, strANSIToUTF8(sValue));

				if (bQuote)
					Stream.Write("\"", 1);
				}
				
			break;
			}

		case CJSONValue::typeNumber:
			{
			CString sValue;
			double rValue = Value.AsDouble();
			int iInt = (int)rValue;
			if ((double)iInt == rValue)
				sValue = strFromInt(iInt);
			else
				sValue = strFromDouble(rValue);
			Stream.Write(sValue.GetASCIIZPointer(), sValue.GetLength());
			break;
			}

		case CJSONValue::typeObject:
			{
			Stream.Write("{", 1);

			for (i = 0; i < Value.GetCount(); i++)
				{
				if (i != 0)
					Stream.Write(" ", 1);

				//	Write the key

				CJSONValue Key(Value.GetKey(i));
				WriteAsAeon(Key, Stream);

				//	Separator

				Stream.Write(":", 1);

				//	Write the value

				WriteAsAeon(Value.GetElement(i), Stream);
				}

			Stream.Write("}", 1);
			break;
			}

		case CJSONValue::typeArray:
			{
			Stream.Write("(", 1);

			for (i = 0; i < Value.GetCount(); i++)
				{
				if (i != 0)
					Stream.Write(" ", 1);

				WriteAsAeon(Value.GetElement(i), Stream);
				}

			Stream.Write(")", 1);
			break;
			}

		case CJSONValue::typeTrue:
			Stream.Write("true", 4);
			break;

		case CJSONValue::typeNull:
		case CJSONValue::typeFalse:
			Stream.Write("nil", 3);
			break;
		}
	}
