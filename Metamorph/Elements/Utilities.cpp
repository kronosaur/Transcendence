//	Utilities.cpp
//
//	Miscellaneous utilities
//	Copyright (c) 2000 by NeuroHack, Inc. All Rights Reserved.

#include "Elements.h"

DECLARE_CONST_STRING( STR_PATH_SEPARATOR,			"\\")

CException errOSError (void)
	{
	return CException(errFail);
	}

CException errOSError (const CString &sError)
	{
	return CException(errFail, sError);
	}

CString pathAbsolutePath (const CString &sPath)

//	pathAbsolutePath
//
//	Makes the given path absolute (based on the current directory)
//
//	Test:
//
//		c:\
//		\\leibowitz\c\
//		\test\
//		\test\test2
//		.\test
//		..\test\
//		..\..\test\
//		test\test2

	{
	enum States
		{
		stateStart,
		stateSingleDot,
		stateSingleSlash,
		stateComponent,
		stateFirstCharacter,
		};

	//	Get the current path. NOTE: GetCurrentDirectory does not return
	//	a trailing backslash UNLESS the current directory is the root.

	char szCurrentDir[4096];
	int iLen = ::GetCurrentDirectory(sizeof(szCurrentDir), szCurrentDir);
	ASSERT(iLen < sizeof(szCurrentDir));
	char *pDest = szCurrentDir + iLen;

	//	Append a backslash

	if (*(pDest - 1) != '\\')
		*pDest++ = '\\';

	//	We use szCurrentDir to build the resulting path.

	char *pPos = sPath.GetASCIIZ();
	int iState = stateStart;
	while (*pPos != '\0')
		{
		switch (iState)
			{
			case stateStart:
				{
				//	If we have a leading backslash then this is
				//	either an absolute path or a UNC path.

				if (*pPos == '\\')
					iState = stateSingleSlash;

				//	A leading dot means either this directory or one
				//	directory up.

				else if (*pPos == '.')
					iState = stateSingleDot;

				//	A character means a drive letter or a directory/filename

				else
					{
					iState = stateFirstCharacter;
					*pDest++ = *pPos;
					}
				break;
				}

			case stateSingleSlash:
				{
				//	If we get a second slash then this is an absolute
				//	UNC path name.

				if (*pPos == '\\')
					return sPath;

				//	If we get character then this is an absolute path
				//	starting with the current drive.

				else
					{
					pDest = szCurrentDir;
					while (*pDest != '\\' && *pDest != '\0')
						pDest++;

					pPos = sPath.GetASCIIZ();
					while (*pPos != '\0')
						*pDest++ = *pPos++;

					*pDest = '\0';
					return CString(szCurrentDir);
					}
				break;
				}

			case stateSingleDot:
				{
				//	If we get a second dot then this says that we
				//	should pop up one directory

				if (*pPos == '.')
					{
					pDest--;
					pDest--;
					while (*pDest != '\\')
						pDest--;

					pDest++;
					pPos++;		//	skip next backslash
					iState = stateStart;
					}

				//	Otherwise, a slash means that we are relative
				//	to the current directory.

				else if (*pPos == '\\')
					{
					iState = stateStart;
					}
				else
					ASSERT(false);
				break;
				}

			case stateFirstCharacter:
				{
				//	A colon means that we've got an absolute path

				if (*pPos == ':')
					return sPath;

				//	Otherwise, we've got a normal component

				else
					{
					*pDest++ = *pPos;
					iState = stateComponent;
					}
				break;
				}

			case stateComponent:
				{
				*pDest++ = *pPos;
				break;
				}
			}

		pPos++;
		}

	//	Done

	*pDest = '\0';
	return CString(szCurrentDir);
	}

CString pathAddComponent (const CString &sPath, const CString &sComponent)

//	pathAddComponent
//
//	Concatenates the given component to the given path and returns
//	the result
//
//	sPath: full pathname to a directory (e.g. "c:\", "\\lawrence\cdrom", "d:\test")
//	sComponent: directory, filename, or wildcard.

	{
	CString sResult = sPath;
	int iPathLength = sResult.GetLength();
	char *pString = sResult.GetASCIIZ();

	//	If the path name does not have a trailing backslash, add one

	if (!sPath.IsBlank() && pString[iPathLength-1] != '\\')
		sResult.Append(STR_PATH_SEPARATOR);

	//	Now concatenate the component

	sResult.Append(sComponent);

	return sResult;
	}

bool pathCreate (const CString &sPath)

//	pathCreate
//
//	Makes sure that the given path exists. Creates all intermediate folders.
//	Returns TRUE if successful.

	{
	CString sTest = sPath;
	char *pPos = sTest.GetASCIIZ();

	//	Make sure the path exists from the top down.

	while (*pPos != '\0')
		{
		//	Skip over this backslash

		while (*pPos == '\\' && *pPos != '0')
			pPos++;

		//	Skip to the next backslash

		while (*pPos != '\\' && *pPos != '\0')
			pPos++;

		//	Trim the path here and see if it exists so far

		char chSaved = *pPos;
		*pPos = '\0';
		
		//	If the path doesn't exist, create it.

		if (!pathExists(sTest))
			{
			if (!::CreateDirectory(sTest.GetASCIIZ(), NULL))
				return false;
			}

		*pPos = chSaved;
		}

	return true;
	}

bool pathExists (const CString &sPath)

//	pathExists
//
//	Returns TRUE if the given path exists

	{
	DWORD dwResult = ::GetFileAttributes(sPath.GetASCIIZ());
	return (dwResult != 0xffffffff);
	}

bool pathIsAbsolute (const CString &sPath)

//	pathIsAbsolute
//
//	Returns TRUE if the path is absolute

	{
	char *pPos = sPath.GetASCIIZ();
	if (*pPos == '\\')
		return true;
	else if (*pPos != '\0' && pPos[1] == ':')
		return true;
	else
		return false;
	}

CString pathGetExecutablePath (HINSTANCE hInstance)

//	pathGetExecutablePath
//
//	Returns the path of the given executable. This is the path
//	(without the filename) of the executable (e.g., c:\bin\windows)

	{
	char szBuffer[1024];
	int iLen;
	char *pPos;
	CString sPath;

	//	Get the path

	iLen = ::GetModuleFileName(hInstance, szBuffer, sizeof(szBuffer)-1);

	//	Skip backwards to the first backslash

	pPos = szBuffer + iLen;
	while (*pPos != '\\' && pPos != szBuffer)
		pPos--;

	*pPos = '\0';

	//	Create string

	return CString(szBuffer);
	}

CString pathGetExtension (const CString &sPath)

//	pathGetExtension
//
//	Returns the extension (without dot)

	{
	char *pPos;
	int iLength;

	pPos = sPath.GetASCIIZ();
	iLength = sPath.GetLength();

	//	Look for the extension

	while (iLength > 0)
		{
		if (pPos[iLength] == '.')
			break;
		iLength--;
		}

	//	Return the extension

	return strSubString(sPath, iLength + 1, -1);
	}

CString pathGetFilename (const CString &sPath)

//	pathGetFilename
//
//	Returns the filename (without the path)

	{
	char *pStart = sPath.GetASCIIZ();
	char *pPos = pStart + sPath.GetLength() + 1;

	//	Look for the first backslash

	while (pPos > pStart && *(pPos - 1) != '\\')
		pPos--;

	return CString(pPos);
	}

CString pathGetPath (const CString &sPath)

//	pathGetPath
//
//	Returns the path without the filename

	{
	char *pStart = sPath.GetASCIIZ();
	char *pPos = pStart + sPath.GetLength() + 1;

	//	Look for the first backslash

	while (pPos > pStart && *(pPos - 1) != '\\')
		pPos--;

	return CString(pStart, pPos - pStart);
	}

CString pathRelativePath (const CString &sRoot, const CString &sPath)

//	pathRelativePath
//
//	Returns a path relative to Root that points to sPath. If no
//	relative path is possible, returns an absolute path

	{
	CString sP1 = pathAbsolutePath(sRoot);
	CString sP2 = pathAbsolutePath(sPath);

	char *pRoot = sP1.GetASCIIZ();
	char *pPath = sP2.GetASCIIZ();

	//	Loop over the path and find out where we diverge from root

	while (*pPath != '\0' && *pRoot != '\0' && *pRoot == *pPath)
		{
		pRoot++;
		pPath++;
		}

	//	If the whole of root isn't in sPath, then we are not below root

	if (*pRoot != '\0')
		return sPath;
	else
		return CString(pPath);
	}

CString pathStripExtension (const CString &sPath)

//	pathStripExtension
//
//	Returns the path without the extension on the filename

	{
	char *pPos;
	int iLength;

	pPos = sPath.GetASCIIZ();
	iLength = sPath.GetLength();

	//	Look for the extension

	while (iLength > 0)
		{
		if (pPos[iLength] == '.')
			break;
		iLength--;
		}

	//	Return everything except the extension

	return strSubString(sPath, 0, iLength);
	}

#define mix(a,b,c) \
	{ \
	a -= b; a -= c; a ^= (c>>13); \
	b -= c; b -= a; b ^= (a<<8); \
	c -= a; c -= b; c ^= (b>>13); \
	a -= b; a -= c; a ^= (c>>12); \
	b -= c; b -= a; b ^= (a<<16); \
	c -= a; c -= b; c ^= (b>>5); \
	a -= b; a -= c; a ^= (c>>3); \
	b -= c; b -= a; b ^= (a<<10); \
	c -= a; c -= b; c ^= (b>>15); \
	}

DWORD utlHashFunctionCase (BYTE *pKey, int iKeyLen)

//	utlHashFunction
//
//	Hash the data
//
//	Source: Bob Jenkins
//	http://burtleburtle.net/bob/hash/evahash.html

	{
	DWORD initval = 1013;
	BYTE *k = pKey;
	register DWORD a,b,c,len;

	//	Set up the internal state
	len = iKeyLen;
	a = b = 0x9e3779b9;		//	the golden ratio; an arbitrary value
	c = initval;			//	the previous hash value (arbitrary)

	//	---------------------------------------- handle most of the key

	while (len >= 12)
		{
		a += (k[0] +((DWORD)k[1]<<8) +((DWORD)k[2]<<16) +((DWORD)k[3]<<24));
		b += (k[4] +((DWORD)k[5]<<8) +((DWORD)k[6]<<16) +((DWORD)k[7]<<24));
		c += (k[8] +((DWORD)k[9]<<8) +((DWORD)k[10]<<16)+((DWORD)k[11]<<24));
		mix(a,b,c);
		k += 12; len -= 12;
		}

	//	------------------------------------- handle the last 11 bytes
	c += iKeyLen;
	switch(len)				//	all the case statements fall through
		{
		case 11: c+=((DWORD)k[10]<<24);
		case 10: c+=((DWORD)k[9]<<16);
		case 9 : c+=((DWORD)k[8]<<8);
		//	the first byte of c is reserved for the length */
		case 8 : b+=((DWORD)k[7]<<24);
		case 7 : b+=((DWORD)k[6]<<16);
		case 6 : b+=((DWORD)k[5]<<8);
		case 5 : b+=k[4];
		case 4 : a+=((DWORD)k[3]<<24);
		case 3 : a+=((DWORD)k[2]<<16);
		case 2 : a+=((DWORD)k[1]<<8);
		case 1 : a+=k[0];
		//	case 0: nothing left to add */
		}

	mix(a,b,c);

	//	-------------------------------------------- report the result
	return c;
	}

void utlMemSet (void *pDest, int Count, char Value)

//	utlMemSet
//
//	Initializes a block of memory to the given value.
//	
//	Inputs:
//		pDest: Pointer to block of memory to initialize
//		Count: Length of block in bytes
//		Value: Value to initialize to

	{
	char *pPos = (char *)pDest;
	char *pEndPos = pPos + Count;
	DWORD dwValue;
	DWORD *pdwPos;

	//	Store the initial unaligned piece

	while (pPos < pEndPos && ((DWORD)pPos % sizeof(DWORD)))
		*pPos++ = Value;

	//	Store the aligned piece

	dwValue = ((BYTE)Value) << 24 | ((BYTE)Value) << 16 | ((BYTE)Value) << 8 | (BYTE)Value;
	pdwPos = (DWORD *)pPos;
	while (pdwPos < (DWORD *)(pEndPos - (sizeof(DWORD) - 1)))
		*pdwPos++ = dwValue;

	//	Store the ending unaligned piece

	pPos = (char *)pdwPos;
	while (pPos < pEndPos)
		*pPos++ = Value;
	}

void utlMemCopy (void *pSource, void *pDest, int iCount)

//	utlMemCopy
//
//	Copies a block of memory of Count bytes from pSource to pDest.
//	
//	Inputs:
//		pSource: Pointer to source memory block
//		pDest: Pointer to destination memory block
//		dwCount: Number of bytes to copy

	{
	char *pS = (char *)pSource;
	char *pD = (char *)pDest;

	for (; iCount > 0; iCount--)
		*pD++ = *pS++;
	}

bool utlMemCompare (void *pSource, void *pDest, int iCount)

//	utlMemCompare
//
//	Compare two blocks of memory for equality

	{
	char *pS = (char *)pSource;
	char *pD = (char *)pDest;

	for (; iCount > 0; iCount--)
		if (*pD++ != *pS++)
			return false;

	return true;
	}
