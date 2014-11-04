//	CCXMLWrapper.cpp
//
//	CCXMLWrapper class
//	Copyright (c) 2014 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"

CCXMLWrapper::CCXMLWrapper (CXMLElement *pXML, ICCItem *pRef) : ICCAtom(NULL)

//	CCXMLWrapper constructor

	{
	ASSERT(pXML);

	//	If we have a reference, then we don't need a copy.

	if (pRef)
		{
		m_pXML = pXML;
		m_pRef = pRef->Reference();
		}

	//	Otherwise, we need a copy

	else
		{
		m_pXML = pXML->OrphanCopy();
		m_pRef = NULL;
		}

	Reference();
	}

ICCItem *CCXMLWrapper::Clone (CCodeChain *pCC)

//	Clone
//
//	Clone the item

	{
	return new CCXMLWrapper(m_pXML);
	}

void CCXMLWrapper::DestroyItem (CCodeChain *pCC)

//	DestroyItem
//
//	Destroy

	{
	if (m_pRef)
		m_pRef->Discard(pCC);

	//	Otherwise we also need to delete the XML

	else
		delete m_pXML;

	delete this;
	}

ICCItem *CCXMLWrapper::StreamItem (CCodeChain *pCC, IWriteStream *pStream)

//	StreamItem
//
//	Stream the item

	{
	//	Not implemented
	ASSERT(false);
	return NULL;
	}

ICCItem *CCXMLWrapper::UnstreamItem (CCodeChain *pCC, IReadStream *pStream)

//	UnstreamItem
//
//	Unstream the item

	{
	//	Not implemented
	ASSERT(false);
	return NULL;
	}
