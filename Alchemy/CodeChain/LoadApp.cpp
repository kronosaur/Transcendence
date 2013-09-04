//	LoadApp.cpp
//
//	Implementation of LoadApp method for CCodeChain

#include <windows.h>
#include "Kernel.h"
#include "KernelObjID.h"
#include "CodeChain.h"

#define OUTPUT_SIGNATURE							'CCDO'
#define OUTPUT_VERSION								1

typedef struct
	{
	DWORD dwSignature;								//	Always 'CCDO'
	DWORD dwVersion;								//	Version of format
	DWORD dwEntryCount;								//	Number of entries
	} HEADERSTRUCT, *PHEADERSTRUCT;

ICCItem *CCodeChain::LoadApp (HMODULE hModule, char *pszRes)

//	LoadApp
//
//	Loads an application from a resource and runs it
//	The resource must be in .cdo format (see CCLink)

	{
	ALERROR error;
	CResourceReadBlock CodeRes(hModule, pszRes);
	ICCItem *pStartUp;
	ICCItem *pResult;

	if (error = CodeRes.Open())
		return CreateSystemError(error);

	//	Load the app

	if (error = LoadDefinitions(&CodeRes))
		return CreateSystemError(error);

	//	Done with the resource

	if (error = CodeRes.Close())
		return CreateSystemError(error);

	//	Run the app

	pStartUp = Link(LITERAL("(boot)"), 0, NULL);
	pResult = TopLevel(pStartUp, NULL);

	//	Done

	pStartUp->Discard(this);

	return pResult;
	}

ALERROR CCodeChain::LoadDefinitions (IReadBlock *pBlock)

//	LoadDefinitions
//
//	Load the definitions from the block

	{
	ALERROR error;
	char *pData = pBlock->GetPointer(0, -1);
	int iLength = pBlock->GetLength();
	HEADERSTRUCT header;
	int i, iEntries;
	CIntArray Offsets;
	TArray<CString> Names;

	//	Read the header

	if (iLength < sizeof(header))
		return ERR_FAIL;

	utlMemCopy(pData, (char *)&header, sizeof(header));
	pData += sizeof(header);
	iLength -= sizeof(header);

	//	Make sure this is the right version

	if (header.dwSignature != OUTPUT_SIGNATURE)
		return ERR_FAIL;

	if (header.dwVersion != OUTPUT_VERSION)
		return ERR_FAIL;

	iEntries = (int)header.dwEntryCount;

	//	Read all the offsets

	for (i = 0; i < iEntries; i++)
		{
		DWORD dwOffset;

		if (iLength < sizeof(DWORD))
			return ERR_FAIL;

		dwOffset = *((DWORD *)pData);
		pData += sizeof(DWORD);
		iLength -= sizeof(DWORD);

		if (error = Offsets.AppendElement(dwOffset, NULL))
			return error;
		}

	//	Skip text block size

	if (iLength < sizeof(DWORD))
		return ERR_FAIL;

	pData += sizeof(DWORD);
	iLength -= sizeof(DWORD);

	//	Read each definition name

	for (i = 0; i < iEntries; i++)
		{
		char *pStart;
		CString sTemp;

		//	Look for a NULL

		pStart = pData;
		while (iLength > 0 && *pData != '\0')
			{
			pData++;
			iLength--;
			}

		//	End of stream?

		if (iLength == 0)
			return ERR_FAIL;

		//	Create a string

		sTemp.Transcribe(pStart, pData - pStart);
		Names.Insert(sTemp);

		//	Next

		pData++;
		iLength--;
		}

	//	Now we loop again and add all identifiers to the 
	//	global symbol table

	pData = pBlock->GetPointer(0, -1);
	iLength = pBlock->GetLength();

	for (i = 0; i < iEntries; i++)
		{
		CString sName = Names[i];
		int iOffset = Offsets.GetElement(i);
		ICCItem *pName;
		ICCItem *pDefinition;
		ICCItem *pError;

		if (iOffset >= iLength)
			return ERR_FAIL;

		//	Create a constant string of the piece
		//	to load. We need to include all the data to the
		//	end because we don't know how big the data is.
		//	Fortunately, this does not require memory allocation,
		//	so it doesn't matter how big it is.

		{
		CMemoryReadStream DataStream(pData + iOffset, iLength - iOffset);

		if (error = DataStream.Open())
			return error;

		//	Unstream the item

		pDefinition = UnstreamItem(&DataStream);
		if (pDefinition->IsError())
			{
			pDefinition->Discard(this);
			return ERR_FAIL;
			}

		DataStream.Close();
		}

		//	Create an item to represent the name

		pName = CreateString(sName);
		if (pName->IsError())
			{
			pName->Discard(this);
			pDefinition->Discard(this);
			return ERR_FAIL;
			}

		//	Add it to the global symbol table

		pError = m_pGlobalSymbols->AddEntry(this, pName, pDefinition);
		if (pError->IsError())
			{
			pError->Discard(this);
			pName->Discard(this);
			pDefinition->Discard(this);
			return ERR_FAIL;
			}

		//	Don't need these anymore; AddEntry keeps a reference

		pError->Discard(this);
		pName->Discard(this);
		pDefinition->Discard(this);
		}

	return NOERROR;
	}

ICCItem *CCodeChain::LoadInitFile (const CString &sFilename)

//	LoadInitFile
//
//	Loads an initialization file containing
//	definitions

	{
	ALERROR error;
	CFileReadBlock CodeRes(sFilename);

	if (error = CodeRes.Open())
		return CreateSystemError(error);

	//	Load the app

	if (error = LoadDefinitions(&CodeRes))
		return CreateSystemError(error);

	//	Done with the resource

	if (error = CodeRes.Close())
		return CreateSystemError(error);

	return CreateTrue();
	}

