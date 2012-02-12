//	Output.cpp
//
//	Functions for outputing data
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

void OutputPageList (CWiki &Wiki, CXMLElement *pCmdLine)

//	OutputPageList
//
//	Outputs the list of all pages

	{
	int i;

	for (i = 0; i < Wiki.GetPageCount(); i++)
		{
		CPage *pPage = Wiki.GetPage(i);

		CString sTitle = strUTF8ToANSI(pPage->GetTitle());
		
		printf("%s\n", sTitle.GetASCIIZPointer());
		}
	}
