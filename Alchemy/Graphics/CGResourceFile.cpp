//	CGResourceFile.cpp
//
//	Implements CGResourceFile class

#include <windows.h>
#include <MMSystem.h>

#include "Kernel.h"
#include "KernelObjID.h"
#include "Graphics.h"

static CObjectClass<CGResourceFile>g_Class(OBJID_CGRESOURCEFILE, NULL);

CGResourceFile::CGResourceFile (void) : CObject(&g_Class),
		m_BitmapCache(sizeof(CACHEENTRY), 10)

//	CGResourceFile constructor

	{
	}

CGResourceFile::CGResourceFile (HINSTANCE hModule) : CObject(&g_Class),
		m_hModule(hModule),
		m_BitmapCache(sizeof(CACHEENTRY), 10),
		m_hBitmapDC(NULL),
		m_hCurBitmap(NULL),
		m_hOldBitmap(NULL)

//	CGResourceFile constructor

	{
	}

CGResourceFile::~CGResourceFile (void)

//	CGResourceFile destructor

	{
	int i;

	for (i = 0; i < m_BitmapCache.GetCount(); i++)
		{
		CACHEENTRY *pEntry = (CACHEENTRY *)m_BitmapCache.GetStruct(i);

		if (pEntry->hBitmap)
			DeleteObject(pEntry->hBitmap);
		}

	if (m_hBitmapDC)
		{
		ASSERT(m_hCurBitmap == NULL);
		DeleteDC(m_hBitmapDC);
		}
	}

void CGResourceFile::DeselectBitmap (void)

//	DeselectBitmap
//
//	Undo SelectBitmap

	{
	ASSERT(m_hCurBitmap);
	SelectObject(m_hBitmapDC, m_hOldBitmap);
	m_hCurBitmap = NULL;
	}

CGResourceFile::CACHEENTRY *CGResourceFile::FindInCache (DWORD dwUNID)

//	FindInCache
//
//	Looks for the bitmap in the cache. Returns NULL if not found; otherwise
//	it returns an pointer into the m_BitmapCache array

	{
	int i;

	for (i = 0; i < m_BitmapCache.GetCount(); i++)
		{
		CACHEENTRY *pEntry = (CACHEENTRY *)m_BitmapCache.GetStruct(i);

		if (dwUNID == pEntry->dwUNID && pEntry->hBitmap)
			return pEntry;
		}

	return NULL;
	}

ALERROR CGResourceFile::GetBitmap (DWORD dwUNID, HBITMAP *rethBitmap)

//	GetBitmap
//
//	The caller must call ReleaseBitmap when it is done with it. Note that
//	we cache the bitmap so that multiple calls to the same UNID will yield
//	the same bitmap

	{
	ALERROR error;
	CACHEENTRY *pEntry;
	HBITMAP hBitmap;
	CACHEENTRY NewEntry;

	//	Look for the bitmap in the cache

	pEntry = FindInCache(dwUNID);

	//	If we could not find it, we have to load it

	if (pEntry == NULL)
		{
		int iIndex;

		if (error = LoadBitmap(dwUNID, &hBitmap))
			return error;

		//	Add it to the cache. We look for a free slot in the array

		for (iIndex = 0; iIndex < m_BitmapCache.GetCount(); iIndex++)
			{
			CACHEENTRY *pEntry = (CACHEENTRY *)m_BitmapCache.GetStruct(iIndex);
			if (pEntry->hBitmap == NULL)
				break;
			}

		//	If we could not find a free slot, we need to add a new struct

		if (iIndex == m_BitmapCache.GetCount())
			{
			NewEntry.dwUNID = dwUNID;
			NewEntry.hBitmap = hBitmap;
			NewEntry.iRefCount = 0;

			if (error = m_BitmapCache.AppendStruct(&NewEntry, &iIndex))
				{
				DeleteObject(hBitmap);
				return error;
				}

			pEntry = (CACHEENTRY *)m_BitmapCache.GetStruct(iIndex);
			}

		//	Otherwise just modify the slot

		else
			{
			pEntry = (CACHEENTRY *)m_BitmapCache.GetStruct(iIndex);

			pEntry->dwUNID = dwUNID;
			pEntry->hBitmap = hBitmap;
			pEntry->iRefCount = 0;
			}
		}

	//	Increment the refcount

	pEntry->iRefCount++;

	//	Return the bitmap

	*rethBitmap = pEntry->hBitmap;

	//	Done

	return NOERROR;
	}

ALERROR CGResourceFile::LoadBitmap (DWORD dwUNID, HBITMAP *rethBitmap)

//	LoadBitmap
//
//	Load a bitmap from the resource

	{
	HBITMAP hBitmap;

	hBitmap = (HBITMAP)LoadImage(m_hModule,
			MAKEINTRESOURCE(dwUNID),
			IMAGE_BITMAP,
			0, 0,
			LR_CREATEDIBSECTION);
	if (hBitmap == NULL)
		return ERR_FAIL;

	//	Done

	*rethBitmap = hBitmap;

	return NOERROR;
	}

void CGResourceFile::ReleaseBitmap (DWORD dwUNID)

//	ReleaseBitmap
//
//	Releases a bitmap loaded by GetBitmap

	{
	CACHEENTRY *pEntry;

	pEntry = FindInCache(dwUNID);
	ASSERT(pEntry);

	//	Decrement the refcount

	pEntry->iRefCount--;

	//	If we are the last reference, delete the object

	if (pEntry->iRefCount == 0)
		{
		DeleteObject(pEntry->hBitmap);
		pEntry->hBitmap = NULL;
		pEntry->dwUNID = 0;
		}
	}

HDC CGResourceFile::SelectBitmap (HBITMAP hBitmap)

//	SelectBitmap
//
//	Returns a DC with the bitmap selected

	{
	//	If we've already got this bitmap selected, just return

	if (hBitmap == m_hCurBitmap)
		return m_hBitmapDC;

	//	If we need to create a DC, do it now

	ASSERT(m_hCurBitmap == NULL);
	if (m_hBitmapDC == NULL)
		m_hBitmapDC = CreateCompatibleDC(NULL);

	//	Select it

	m_hOldBitmap = (HBITMAP)SelectObject(m_hBitmapDC, hBitmap);
	m_hCurBitmap = hBitmap;

	//	Done

	return m_hBitmapDC;
	}

