//	CString.cpp
//
//	CString class
//	Copyright (c) 2000 by NeuroHack, Inc. All Rights Reserved.
//
//	A CString is either NULL or a pointer to an allocated block
//	of the following form:
//
//					int		Length
//	m_pString ->	char	First char of string
//					char	Second char of string
//					char	...
//					char	Last char of string
//					char	'\0' (always NULL-terminated)
//
//	Length does NOT include the null termination.
//
//	If Length is negative then it means that m_pString is a 
//	literal and should not be deleted.

#include "Elements.h"

const CString NULL_STRING;

int intCompare (char *pStr1, int iLen1, char *pStr2, int iLen2);

CString::CString (char *szStr)

//	CString constructor
//
//	Construct a CString from an ASCIIZ string

	{
	Init(szStr, lstrlen(szStr), false);
	}

CString::CString (char *pStr, int iLen)

//	CString constructor
//
//	Construct a string from an array of characters of a given length

	{
	Init(pStr, iLen, false);
	}

CString::CString (char *pStr, int iLen, bool bLiteral)

//	CString construtor

	{
	Init(pStr, iLen, bLiteral);
	}

CString::~CString (void)

//	CString destructor

	{
	CleanUp();
	}

CString &CString::operator= (const CString &sStr)

//	CString operator =

	{
	CleanUp();

	m_pString = sStr.CopyBuffer();
	return *this;
	}

CString &CString::operator= (IMemoryBlock &Block)

//	CString operator = IMemoryBlock *

	{
	CleanUp();

	Init(Block.GetPointer(), Block.GetLength(), false);
	return *this;
	}

void CString::CleanUp (void)

//	CleanUp
//
//	Deallocate

	{
	if (m_pString && !IsLiteral())
		{
		char *pBuffer = m_pString - sizeof(int);
		delete [] pBuffer;
		}
	}

char *CString::CopyBuffer (void) const

//	CopyBuffer
//
//	Returns a copy of m_pString for this string

	{
	if (m_pString && !IsLiteral())
		{
		char *pNewBuffer = new char[BufferLength()];
		utlMemCopy(m_pString - sizeof(int), pNewBuffer, BufferLength());
		return pNewBuffer + sizeof(int);
		}
	else
		return m_pString;
	}

void CString::Delete (int iIndex, int iCount)

//	Delete
//
//	Delete iCount characters at the given index

	{
	ASSERT(!IsLiteral());

	if (iIndex < 0 || iIndex >= GetLength())
		return;

	if (iIndex + iCount > GetLength())
		iCount = GetLength() - iIndex;

	if (iCount <= 0)
		return;

	//	Compute the size of the resulting string

	int iLen = GetLength() - iCount;
	
	//	Deal with NULL strings

	if (iLen == 0)
		{
		CleanUp();
		m_pString = NULL;
		return;
		}

	//	Allocate a new buffer

	char *pBuffer = new char[sizeof(int) + iLen + 1];
	*(int *)pBuffer = iLen;

	//	Copy up to the deletion point

	char *pDest = pBuffer + sizeof(int);
	char *pSource = m_pString;
	char *pSourceEnd = pSource + iIndex;

	while (pSource < pSourceEnd)
		*pDest++ = *pSource++;

	//	Now the remainder

	pSource = m_pString + iIndex + iCount;
	pSourceEnd = m_pString + GetLength();

	while (pSource < pSourceEnd)
		*pDest++ = *pSource++;

	//	NULL-terminate

	*pDest = '\0';

	//	Done

	CleanUp();
	m_pString = pBuffer + sizeof(int);
	}

int CString::GetLength (void) const

//	GetLength
//
//	Returns the length of the string

	{
	if (m_pString)
		return Absolute(GetLengthParameter());
	else 
		return 0;
	}

char *CString::GetWritePointer (int iLength)

//	GetWritePointer
//
//	Returns a pointer that allows the user to write to the string.
//	On return, the buffer is guaranteed to be at least iLength.

	{
	if (iLength == 0)
		{
		//	Note that there is code that relies on Truncate(0)
		//	to free-up the string completely.
		CleanUp();
		m_pString = NULL;
		}
	else
		{
		//	Allocate a new buffer

		char *pBuffer = new char[sizeof(int) + iLength + 1];
		*(int *)pBuffer = iLength;

		//	Copy the current contents of the string

		char *pDest = pBuffer + sizeof(int);
		char *pSource = m_pString;
		int iLeft = min(GetLength(), iLength);

		while (iLeft > 0)
			{
			*pDest++ = *pSource++;
			iLeft--;
			}

		//	NULL-terminate

		pBuffer[iLength + sizeof(int)] = '\0';

		//	Done

		CleanUp();
		m_pString = pBuffer + sizeof(int);
		}

	return m_pString;
	}

void CString::Init (char *pStr, int iLen, bool bLiteral)

//	Init
//
//	Initialize the string

	{
	if (pStr && !bLiteral)
		{
		char *pBuffer = new char[sizeof(int) + iLen + 1];
		*(int *)pBuffer = iLen;
		m_pString = pBuffer + sizeof(int);
		utlMemCopy(pStr, m_pString, iLen);
		m_pString[iLen] = '\0';
		}
	else
		m_pString = pStr;	//	Literal
	}

void CString::Insert (const CString &sStr, int iIndex)

//	Insert
//
//	Inserts the given string at the given position (or at the end,
//	if iIndex == -1).

	{
	if (sStr.GetLength() > 0)
		{
		//	Deal with NULL strings

		if (m_pString == NULL)
			{
			*this = sStr;
			return;
			}

		//	NOTE: This should work fine even if this string is a literal
		//	Compute the size of the resulting string

		if (iIndex == -1) iIndex = GetLength();
		int iLen = GetLength() + sStr.GetLength();

		//	Allocate a new buffer

		char *pBuffer = new char[sizeof(int) + iLen + 1];
		*(int *)pBuffer = iLen;

		//	Copy up to the insertion point

		char *pDest = pBuffer + sizeof(int);
		char *pSource = m_pString;
		char *pSourceEnd = pSource + iIndex;

		while (pSource < pSourceEnd)
			*pDest++ = *pSource++;

		//	Now append the second string

		pSource = sStr.m_pString;
		pSourceEnd = pSource + sStr.GetLength();

		while (pSource < pSourceEnd)
			*pDest++ = *pSource++;

		//	Now the remainder

		pSource = m_pString + iIndex;
		pSourceEnd = m_pString + GetLength();

		while (pSource < pSourceEnd)
			*pDest++ = *pSource++;

		//	NULL-terminate

		*pDest = '\0';

		//	Done

		CleanUp();
		m_pString = pBuffer + sizeof(int);
		}
	}

void CString::Truncate (int iLength)

//	Truncate
//
//	Truncate the string to the given length. The length cannot be greater
//	than the current length of the string

	{
	ASSERT(!IsLiteral());
	ASSERT(iLength >= 0 && iLength <= GetLength());

	//	Call this just to make sure that we have our own copy

	GetWritePointer(iLength);
	}

CString strCat (const CString &sString1, const CString &sString2)

//	strCat
//
//	Returns the concatenation of the two string

	{
	CString sCat;

	sCat = sString1;
	sCat.Append(sString2);
	return sCat;
	}

int strCompare (const CString &sString1, const CString &sString2)

//	strCompare
//
//	Compares two strings are returns 1 if sString1 is > sString2; -1 if sString1
//	is < sString2; and 0 if both strings are equal

	{
	return intCompare(sString1, sString1.GetLength(), sString2, sString2.GetLength());
	}

bool strEquals (const CString &sStr1, const CString &sStr2)

//	strEquals
//
//	Return true if both strings are equal (comparison is case insensitive)

	{
	char *pPos1;
	char *pPos2;
	int i, iLen;

	//	If the strings aren't the same length then don't bother

	iLen = sStr1.GetLength();
	if (iLen != sStr2.GetLength())
		return false;

	//	Setup

	pPos1 = sStr1;
	pPos2 = sStr2;

	//	Compare by commonality

	for (i = 0; i < iLen; i++)
		if (CharLower((LPTSTR)(BYTE)(pPos1[i])) != CharLower((LPTSTR)(BYTE)(pPos2[i])))
			return false;

	return true;
	}

bool strEqualsCase (const CString &sStr1, const CString &sStr2)

//	strEqualsCase
//
//	Return true if both strings are equal (comparison is case sensitive)

	{
	char *pPos1;
	char *pPos2;
	int i, iLen;

	//	If the strings aren't the same length then don't bother

	iLen = sStr1.GetLength();
	if (iLen != sStr2.GetLength())
		return false;

	//	Setup

	pPos1 = sStr1;
	pPos2 = sStr2;

	//	Compare by commonality

	for (i = 0; i < iLen; i++)
		if (pPos1[i] != pPos2[i])
			return false;

	return true;
	}

int strFind (const CString &sString, const CString &sTarget)

//	strFind
//
//	Finds the target string in the given string and returns the
//	offset in sString at which the target starts. If the target is
//	not found anywhere in sString then we return -1

	{
	int iStringLen = sString.GetLength();
	int iTargetLen = sTarget.GetLength();
	char *pString = sString;
	int i;

	//	If the target is null, then we don't match

	if (sTarget.IsBlank())
		return -1;

	//	Search for a match at successive offsets of sString
	//	until we're past the point where sTarget would not
	//	fit.

	for (i = 0; i <= iStringLen - iTargetLen; i++)
		{
		if (intCompare(pString + i, iTargetLen, sTarget, sTarget.GetLength()) == 0)
			return i;
		}

	//	Didn't find it

	return -1;
	}

CString strFromInt (int iInteger, bool bSigned)

//	CStringFromInt
//
//	Converts an integer to a string

	{
	char szString[256];
	int iLen;

	if (bSigned)
		iLen = wsprintf(szString, "%d", iInteger);
	else
		iLen = wsprintf(szString, "%u", iInteger);

	CString sString(szString, iLen);
	return sString;
	}

int strParseInt (char *pStart, int iNullResult, char **retpEnd, bool *retbNullValue)

//	strParseInt
//
//	pStart: Start parsing. Skips any leading whitespace
//	iNullResult: If there are no valid numbers, returns this value
//	retpEnd: Returns the character at which we stopped parsing
//	retbNullValue: Returns true if there are no valid numbers.

	{
	char *pPos;
	bool bNegative;
	bool bFoundNumber;
	bool bHex;
	int iInt;

	//	Preset

	if (retbNullValue)
		*retbNullValue = false;

	pPos = pStart;
	bNegative = false;
	bFoundNumber = false;
	bHex = false;
	iInt = 0;

	//	Skip whitespace

	while (*pPos == ' ' || *pPos == '\t' || *pPos == '\n' || *pPos == '\r')
		pPos++;

	//	If NULL, then we're done

	if (*pPos == '\0')
		{
		if (retbNullValue)
			*retbNullValue = true;

		if (retpEnd)
			*retpEnd = pPos;

		return iNullResult;
		}

	//	If negative, remember it

	if (*pPos == '-')
		{
		bNegative = true;
		pPos++;
		}

	//	See if this is a hex number

	if (*pPos == '0')
		{
		pPos++;
		bFoundNumber = true;

		//	If the next character is x (or X) then we've got
		//	a Hex number

		if (*pPos == 'x' || *pPos == 'X')
			{
			pPos++;
			bHex = true;
			}
		}

	//	Keep parsing

	if (bHex)
		{
		DWORD dwInt = 0;

		while (*pPos != '\0' 
				&& ((*pPos >= '0' && *pPos <= '9') 
					|| (*pPos >= 'a' && *pPos <='f')
					|| (*pPos >= 'A' && *pPos <= 'F')))
			{
			if (*pPos >= '0' && *pPos <= '9')
				dwInt = 16 * dwInt + (*pPos - '0');
			else if (*pPos >= 'A' && *pPos <= 'F')
				dwInt = 16 * dwInt + (10 + (*pPos - 'A'));
			else
				dwInt = 16 * dwInt + (10 + (*pPos - 'a'));

			pPos++;
			}

		iInt = (int)dwInt;
		}
	else
		{
		while (*pPos != '\0' && *pPos >= '0' && *pPos <= '9')
			{
			iInt = 10 * iInt + (*pPos - '0');
			pPos++;
			bFoundNumber = true;
			}
		}

	//	Done?

	if (!bFoundNumber)
		{
		if (retbNullValue)
			*retbNullValue = true;

		if (retpEnd)
			*retpEnd = pPos;

		return iNullResult;
		}

	//	Done!

	if (bNegative)
		iInt = -iInt;

	if (retpEnd)
		*retpEnd = pPos;

	return iInt;
	}

int strParseIntOfBase (char *pStart, int iBase, int iNullResult, char **retpEnd, bool *retbNullValue)

//	strParseIntOfBase
//
//	Parses an integer of the given base

	{
	char *pPos;
	bool bNegative;
	bool bFoundNumber;
	int iInt;

	//	Preset

	if (retbNullValue)
		*retbNullValue = false;

	pPos = pStart;
	bNegative = false;
	bFoundNumber = false;
	iInt = 0;

	//	Skip whitespace

	while (*pPos == ' ' || *pPos == '\t' || *pPos == '\n' || *pPos == '\r')
		pPos++;

	//	If NULL, then we're done

	if (*pPos == '\0')
		{
		if (retbNullValue)
			*retbNullValue = true;

		if (retpEnd)
			*retpEnd = pPos;

		return iNullResult;
		}

	//	If negative, remember it

	if (*pPos == '-')
		{
		bNegative = true;
		pPos++;
		}

	//	See if this is a hex number

	if (*pPos == '0')
		{
		pPos++;
		bFoundNumber = true;

		//	If the next character is x (or X) then we've got
		//	a Hex number

		if (*pPos == 'x' || *pPos == 'X')
			{
			pPos++;
			iBase = 16;
			}
		}

	//	Now parse for numbers

	DWORD dwInt = 0;

	while (*pPos != '\0' 
			&& ((*pPos >= '0' && *pPos <= '9') 
				|| (*pPos >= 'a' && *pPos <='f')
				|| (*pPos >= 'A' && *pPos <= 'F')))
		{
		if (*pPos >= '0' && *pPos <= '9')
			dwInt = (DWORD)iBase * dwInt + (*pPos - '0');
		else if (*pPos >= 'A' && *pPos <= 'F')
			dwInt = (DWORD)iBase * dwInt + (10 + (*pPos - 'A'));
		else
			dwInt = (DWORD)iBase * dwInt + (10 + (*pPos - 'a'));

		pPos++;
		bFoundNumber = true;
		}

	iInt = (int)dwInt;

	//	Done?

	if (!bFoundNumber)
		{
		if (retbNullValue)
			*retbNullValue = true;

		if (retpEnd)
			*retpEnd = pPos;

		return iNullResult;
		}

	//	Done!

	if (bNegative)
		iInt = -iInt;

	if (retpEnd)
		*retpEnd = pPos;

	return iInt;
	}

void strParseWhitespace (char *pPos, char **retpPos)

//	strParseWhitespace
//
//	Skips whitespace

	{
	while (strIsWhitespace(pPos))
		pPos++;

	if (retpPos)
		*retpPos = pPos;
	}

int strToInt (const CString &sString, int iFailResult, bool *retbFailed)

//	CStringToInt
//
//	Converts a string to an integer

	{
	return strParseInt(sString.GetASCIIZ(), iFailResult, NULL, retbFailed);
	}

CString strToLower (const CString &sString)

//	strToLower
//
//	Converts string to all lower-caes

	{
	CString sResult = sString;
	::CharLowerBuff(sResult.GetWritePointer(sResult.GetLength()), sResult.GetLength());
	return sResult;
	}

CString strToUpper (const CString &sString)

//	strToUpper
//
//	Converts string to all upper-case

	{
	CString sUpper = sString;
	::CharUpperBuff(sUpper.GetWritePointer(sUpper.GetLength()), sUpper.GetLength());
	return sUpper;
	}

CString strTrimWhitespace (const CString &sString)

//	strTrimWhitespace
//
//	Removes leading and trailing whitespace

	{
	char *pPos = sString.GetASCIIZ();
	char *pStart;
	char *pEnd;

	strParseWhitespace(pPos, &pStart);

	pEnd = pPos + sString.GetLength();
	while (pEnd > pStart && strIsWhitespace(pEnd-1))
		pEnd--;

	return CString(pStart, pEnd - pStart);
	}

CString strSubString (const CString &sString, int iOffset, int iLength)

//	strSubString
//
//	Returns a substring of the given string

	{
	if (iOffset >= sString.GetLength())
		return CString();
	else
		{
		if (iLength == -1)
			iLength = sString.GetLength() - iOffset;
		else
			iLength = min(iLength, sString.GetLength() - iOffset);

		CString sSub(sString.GetASCIIZ() + iOffset, iLength);
		return sSub;
		}
	}

int intCompare (char *pStr1, int iLen1, char *pStr2, int iLen2)
	{
	//	Setup

	int iLen = min(iLen1, iLen2);

	//	Compare by commonality

	for (int i = 0; i < iLen; i++)
		{
		char chChar1 = (char)::CharLower((LPTSTR)(BYTE)(*pStr1));
		char chChar2 = (char)::CharLower((LPTSTR)(BYTE)(*pStr2));

		if (chChar1 > chChar2)
			return 1;
		else if (chChar1 < chChar2)
			return -1;

		pStr1++;
		pStr2++;
		}

	//	If the strings match up to a point, check to see which is 
	//	longest.

	if (iLen1 > iLen2)
		return 1;
	else if (iLen1 < iLen2)
		return -1;
	else
		return 0;
	}
