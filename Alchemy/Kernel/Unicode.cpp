//	Unicode.cpp
//
//	Implementation of standard string class

#include "Kernel.h"
#include "KernelObjID.h"

DWORD g_CP1252[128] =
	{
	0x20ac, 0x0000, 0x201a, 0x0192, 0x201e, 0x2026, 0x2020, 0x2021, 0x02c6, 0x2030, 0x0160, 0x2039, 0x0152, 0x0000, 0x017d, 0x0000,
	0x0000, 0x2018, 0x2019, 0x201c, 0x201d, 0x2022, 0x2013, 0x2014, 0x02dc, 0x2122, 0x0161, 0x203a, 0x0153, 0x0000, 0x017e, 0x0178,
	0x00a0, 0x00a1, 0x00a2, 0x00a3, 0x00a4, 0x00a5, 0x00a6, 0x00a7, 0x00a8, 0x00a9, 0x00aa, 0x00ab, 0x00ac, 0x00ad, 0x00ae, 0x00af, 
	0x00b0, 0x00b1, 0x00b2, 0x00b3, 0x00b4, 0x00b5, 0x00b6, 0x00b7, 0x00b8, 0x00b9, 0x00ba, 0x00bb, 0x00bc, 0x00bd, 0x00be, 0x00bf, 
	0x00c0, 0x00c1, 0x00c2, 0x00c3, 0x00c4, 0x00c5, 0x00c6, 0x00c7, 0x00c8, 0x00c9, 0x00ca, 0x00cb, 0x00cc, 0x00cd, 0x00ce, 0x00cf, 
	0x00d0, 0x00d1, 0x00d2, 0x00d3, 0x00d4, 0x00d5, 0x00d6, 0x00d7, 0x00d8, 0x00d9, 0x00da, 0x00db, 0x00dc, 0x00dd, 0x00de, 0x00df, 
	0x00e0, 0x00e1, 0x00e2, 0x00e3, 0x00e4, 0x00e5, 0x00e6, 0x00e7, 0x00e8, 0x00e9, 0x00ea, 0x00eb, 0x00ec, 0x00ed, 0x00ee, 0x00ef, 
	0x00f0, 0x00f1, 0x00f2, 0x00f3, 0x00f4, 0x00f5, 0x00f6, 0x00f7, 0x00f8, 0x00f9, 0x00fa, 0x00fb, 0x00fc, 0x00fd, 0x00fe, 0x00ff, 
	};

CString strConvert (const CString &sText, DWORD dwFromCP, DWORD dwToCP)

//	strConvert
//
//	Convert from one codepage to another by using double-byte unicode as an
//	intermediary.

	{
	//	Edge-conditions

	if (sText.IsBlank())
		return NULL_STR;
	else if (dwFromCP == dwToCP)
		return sText;

	//	Optimistically allocate a buffer of some size

	CString sUnicode;
	int iAllocChars = sText.GetLength();
	sUnicode.GetWritePointer(iAllocChars * 2);

	//	Convert

	int iResult = ::MultiByteToWideChar(dwFromCP, 0, sText.GetASCIIZPointer(), sText.GetLength(), (LPWSTR)sUnicode.GetASCIIZPointer(), iAllocChars);
	if (iResult == 0)
		{
		//	If we need a bigger buffer, reallocate

		if (::GetLastError() == ERROR_INSUFFICIENT_BUFFER)
			{
			//	Figure out how many characters we need

			iAllocChars = ::MultiByteToWideChar(dwFromCP, 0, sText.GetASCIIZPointer(), sText.GetLength(), NULL, 0);
			sUnicode.GetWritePointer(iAllocChars * 2);

			//	Convert again

			iResult = ::MultiByteToWideChar(dwFromCP, 0, sText.GetASCIIZPointer(), sText.GetLength(), (LPWSTR)sUnicode.GetASCIIZPointer(), iAllocChars);
			}

		//	If we're still in error, then we fail

		if (iResult == 0)
			return CONSTLIT("{unable to convert to Unicode}");
		}

	//	The result is the number of characters

	int iUnicodeChars = iResult;

	//	Allocate a result buffer

	CString sResult;
	iAllocChars = sText.GetLength() + 10;
	sResult.GetWritePointer(iAllocChars);

	//	Now convert to destination code page

	iResult = ::WideCharToMultiByte(dwToCP, 0, (LPCWSTR)sUnicode.GetASCIIZPointer(), iUnicodeChars, sResult.GetASCIIZPointer(), iAllocChars, NULL, NULL);
	if (iResult == 0)
		{
		//	If we need a bigger buffer, reallocate

		if (::GetLastError() == ERROR_INSUFFICIENT_BUFFER)
			{
			//	Figure out how many characters we need

			iAllocChars = ::WideCharToMultiByte(dwToCP, 0, (LPCWSTR)sUnicode.GetASCIIZPointer(), iUnicodeChars, NULL, 0, NULL, NULL);
			sResult.GetWritePointer(iAllocChars);

			//	Convert again

			iResult = ::WideCharToMultiByte(dwToCP, 0, (LPCWSTR)sUnicode.GetASCIIZPointer(), iUnicodeChars, sResult.GetASCIIZPointer(), iAllocChars, NULL, NULL);
			}

		//	If we're still in error, then we fail

		if (iResult == 0)
			return CONSTLIT("{unable to convert to ANSI}");
		}

	//	The result is the number of characters

	sResult.Truncate(iResult);
	
	//	Done

	return sResult;
	}

CString strEncodeUTF8Char (DWORD dwCodePoint)

//	strEncodeUTF8Char
//
//	Encodes a Unicode character into a UTF8 string

	{
	BYTE szBuffer[4];

	if (dwCodePoint <= 0x007f)
		{
		szBuffer[0] = (BYTE)dwCodePoint;
		return CString((char *)szBuffer, 1);
		}
	else if (dwCodePoint <= 0x07ff)
		{
		szBuffer[0] = (BYTE)(0xc0 | ((dwCodePoint & 0x0700) >> 6) | ((dwCodePoint & 0x00ff) >> 6));
		szBuffer[1] = (BYTE)(0x80 | ((dwCodePoint & 0x003f)));
		return CString((char *)szBuffer, 2);
		}
	else if (dwCodePoint <= 0xffff)
		{
		szBuffer[0] = (BYTE)(0xe0 | ((dwCodePoint & 0xf000) >> 12));
		szBuffer[1] = (BYTE)(0x80 | ((dwCodePoint & 0x0f00) >> 6) | ((dwCodePoint & 0x00c0) >> 6));
		szBuffer[2] = (BYTE)(0x80 | ((dwCodePoint & 0x003f)));
		return CString ((char *)szBuffer, 3);
		}
	else
		{
		szBuffer[0] = (BYTE)(0xf0 | ((dwCodePoint & 0x1c0000) >> 18));
		szBuffer[1] = (BYTE)(0x80 | ((dwCodePoint & 0x030000) >> 12) | ((dwCodePoint & 0xf000) >> 12));
		szBuffer[2] = (BYTE)(0x80 | ((dwCodePoint & 0x0f00) >> 6) | ((dwCodePoint & 0x00c0) >> 6));
		szBuffer[3] = (BYTE)(0x80 | ((dwCodePoint & 0x003f)));
		return CString ((char *)szBuffer, 4);
		}
	}

CString strEncodeW1252ToUTF8Char (char chChar)

//	strEncodeW1252ToUTF8Char
//
//	Encodes a character in Windows CP1252 to UTF8.

	{
	if ((BYTE)chChar < 0x80)
		return CString(&chChar, 1);
	else
		return strEncodeUTF8Char(g_CP1252[(BYTE)chChar - 0x80]);
	}
