//	CWiki.cpp
//
//	CWiki class
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

#define PAGE_TAG								CONSTLIT("page")

CWiki::~CWiki (void)

//	CWiki destructor

	{
	int i;

	//	Free all pages

	for (i = 0; i < m_Pages.GetCount(); i++)
		delete m_Pages[i];
	}

ALERROR CWiki::InitFromXML (CXMLElement *pRoot, CString *retsError)

//	InitFromXML
//
//	Initializes the wiki

	{
	ALERROR error;
	int i;

	//	Loop over all elements in the root and pull out the pages

	for (i = 0; i < pRoot->GetContentElementCount(); i++)
		{
		CXMLElement *pItem = pRoot->GetContentElement(i);

		if (strEquals(pItem->GetTag(), PAGE_TAG))
			{
			CPage *pPage = new CPage;

			//	Parse the page

			if (error = pPage->InitFromXML(pItem, retsError))
				{
				delete pPage;
				return error;
				}

			//	Insert in our list

			m_Pages.Insert(pPage);

			//	Insert to title index

			STitleEntry *pEntry = m_TitleIndex.Insert();
			pEntry->sTitle = pPage->GetTitle();
			pEntry->pPage = pPage;
			}
		}

	//	Sort the title index

	m_TitleIndex.Sort();

	//	Done

	return NOERROR;
	}

