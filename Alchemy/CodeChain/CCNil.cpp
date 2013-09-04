//	CCNil.cpp
//
//	Implements CCNil class

#include <windows.h>
#include "Kernel.h"
#include "KernelObjID.h"
#include "CodeChain.h"

static CObjectClass<CCNil>g_Class(OBJID_CCNIL, NULL);

CCNil::CCNil (void) : ICCAtom(&g_Class)

//	CCNil constructor

	{
	SetNoRefCount();
	}

ICCItem *CCNil::Clone (CCodeChain *pCC)

//	Clone
//
//	Returns a new item with a single ref-count

	{
	return Reference();
	}

void CCNil::DestroyItem (CCodeChain *pCC)

//	DestroyItem
//
//	Destroys the item

	{
	}

ICCItem *CCNil::StreamItem (CCodeChain *pCC, IWriteStream *pStream)

//	StreamItem
//
//	Stream the sub-class specific data

	{
	return pCC->CreateTrue();
	}

ICCItem *CCNil::UnstreamItem (CCodeChain *pCC, IReadStream *pStream)

//	UnstreamItem
//
//	Unstream the sub-class specific data

	{
	return pCC->CreateTrue();
	}
