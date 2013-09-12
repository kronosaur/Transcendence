//	CPageManager.cpp
//
//	Implements CPageManager class
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include <windows.h>
#include "Kernel.h"
#include "KernelObjID.h"
#include "CodeChain.h"
#include "Pages.h"

CPageManager g_PM;

void CPageManager::ClosePage (IPage *pPage)

//	ClosePage
//
//	Closes the page

	{
	int iIndex;

	if (FindEntry(pPage, &iIndex))
		{
		if (--m_Pages[iIndex].iRefCount <= 0)
			{
			ASSERT(m_Pages[iIndex].iRefCount == 0);
			delete m_Pages[iIndex].pPage;
			m_Pages.Delete(iIndex);
			}
		}
	}

bool CPageManager::FindEntry (const CString &sAddress, int *retiIndex)

//	FindEntry
//
//	Finds the index of the given page by address

	{
	int i;

	for (i = 0; i < m_Pages.GetCount(); i++)
		if (strEquals(sAddress, m_Pages[i].sAddress))
			{
			if (retiIndex)
				*retiIndex = i;
			return true;
			}

	return false;
	}

bool CPageManager::FindEntry (IPage *pPage, int *retiIndex)

//	FindEntry
//
//	Finds the index of the given page or returns FALSE.

	{
	int i;

	for (i = 0; i < m_Pages.GetCount(); i++)
		if (m_Pages[i].pPage == pPage)
			{
			if (retiIndex)
				*retiIndex = i;
			return true;
			}

	return false;
	}

bool CPageManager::IsFileAddress (const CString &sAddress)

//	IsFileAddress
//
//	Returns TRUE if this is a file address

	{
	char *pPos = sAddress.GetASCIIZPointer();

	if (((pPos[0] >= 'a' && pPos[0] <= 'z') || (pPos[0] >= 'A' && pPos[0] <= 'Z'))
			&& pPos[1] == ':')
		return true;
	else if (pPos[0] == '\\' && pPos[0] == '\\')
		return true;
	else
		return false;
	}

ALERROR CPageManager::OpenPage (const CString &sAddress, IPage **retpPage, CString *retsError)

//	OpenPage
//
//	Opens the page at the given address

	{
	ALERROR error;

	//	First look for the page in the cache

	int iIndex;
	if (FindEntry(sAddress, &iIndex))
		{
		m_Pages[iIndex].iRefCount++;
		*retpPage = m_Pages[iIndex].pPage;
		return NOERROR;
		}

	//	Create the proper class depending on the address. If the address starts
	//	with http, then this is a web page.

	IPage *pPage;
	if (strStartsWith(sAddress, CONSTLIT("http:")))
		//	LATER
		return ERR_FAIL;

	//	Check to see if this is a file address

	else if (IsFileAddress(sAddress))
		pPage = new CTextFilePage;

	//	Unknown

	else
		{
		if (retsError)
			*retsError = strPatternSubst(CONSTLIT("Unknown page address: %s"), sAddress);

		return ERR_FAIL;
		}

	//	Open the page

	if (error = pPage->Open(sAddress, retsError))
		{
		delete pPage;
		return error;
		}

	//	Add the entry

	SEntry *pNewEntry = m_Pages.Insert();
	pNewEntry->sAddress = sAddress;
	pNewEntry->pPage = pPage;
	pNewEntry->iRefCount = 1;

	//	Done

	*retpPage = pPage;

	return NOERROR;
	}
