//	CPage.cpp
//
//	CPage class
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

#define REVISION_TAG							CONSTLIT("revision")
#define TEXT_TAG								CONSTLIT("text")
#define TITLE_TAG								CONSTLIT("title")

ALERROR CPage::InitFromXML (CXMLElement *pPage, CString *retsError)

//	InitFromXML
//
//	Initializes the page from a page element

	{
	int i;

	for (i = 0; i < pPage->GetContentElementCount(); i++)
		{
		CXMLElement *pItem = pPage->GetContentElement(i);

		if (strEquals(pItem->GetTag(), TITLE_TAG))
			m_sTitle = pItem->GetContentText(0);
		else if (strEquals(pItem->GetTag(), REVISION_TAG))
			{
			CXMLElement *pText = pItem->GetContentElementByTag(TEXT_TAG);
			if (pText)
				m_sText = pText->GetContentText(0);
			}
		}

	return NOERROR;
	}
