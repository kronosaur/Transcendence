//	CImageLibrary.cpp
//
//	CImageLibrary class

#include "PreComp.h"

#define LOAD_ON_USE_ATTRIB					CONSTLIT("loadOnUse")

CImageLibrary::CImageLibrary (void) : m_Library(FALSE, TRUE)

//	CImageLibrary constructor

	{
	}

CImageLibrary::~CImageLibrary (void)

//	CImageLibrary destructor

	{
	//	Free all images

	for (int i = 0; i < m_Library.GetCount(); i++)
		{
		CObjectImage *pImage = (CObjectImage *)m_Library.GetValue(i);
		delete pImage;
		}
	}

ALERROR CImageLibrary::AddImage (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	AddImage
//
//	Add an image to the library

	{
	ALERROR error;

	//	Create the image desc

	CObjectImage *pNewImage;
	if (error = CObjectImage::CreateFromXML(Ctx, pDesc, &pNewImage))
		return error;

	//	If we defer loading, at least make sure that the images exist

	if (pDesc->GetAttributeBool(LOAD_ON_USE_ATTRIB))
		{
		if (error = pNewImage->Exists(Ctx))
			{
			delete pNewImage;
			return error;
			}
		}

	//	Otherwise, load and lock the image

	else
		{
		if (error = pNewImage->Lock(Ctx))
			{
			delete pNewImage;
			return error;
			}
		}

	//	Add to the library

	if (error = m_Library.AddEntry(pNewImage->GetUNID(), (CObject *)pNewImage))
		{
		if (FindImage(pNewImage->GetUNID()))
			Ctx.sError = strPatternSubst(CONSTLIT("Duplicate UNID: %x"), pNewImage->GetUNID());
		else
			Ctx.sError = strPatternSubst(CONSTLIT("Unable to add image to library: %x"), pNewImage->GetUNID());
		delete pNewImage;
		return error;
		}

	return NOERROR;
	}

void CImageLibrary::ClearMarks (void)

//	ClearMarks
//
//	Clears the marks for all images

	{
	for (int i = 0; i < m_Library.GetCount(); i++)
		{
		CObjectImage *pImage = (CObjectImage *)m_Library.GetValue(i);
		pImage->ClearMark();
		}
	}

CObjectImage *CImageLibrary::FindImage (DWORD dwUNID)

//	FindImage
//
//	Returns the image by UNID (but does not load it)

	{
	//	Find the image in the library

	int iEntry;
	if (m_Library.LookupEx(dwUNID, &iEntry) != NOERROR)
		return NULL;

	CObjectImage *pImage = (CObjectImage *)m_Library.GetValue(iEntry);
	return pImage;
	}

CG16bitImage *CImageLibrary::GetImage (DWORD dwUNID)

//	GetImage
//
//	Returns the image by UNID (loading it, if necessary)

	{
	//	Find the image in the library

	int iEntry;
	if (m_Library.LookupEx(dwUNID, &iEntry) != NOERROR)
		return NULL;

	CObjectImage *pImage = (CObjectImage *)m_Library.GetValue(iEntry);

	return pImage->GetImage();
	}

CG16bitImage *CImageLibrary::GetImageCopy (DWORD dwUNID)

//	GetImageCopy
//
//	Returns a copy of the image by UNID (caller is responsible for
//	freeing the returned object)

	{
	//	Find the image in the library

	int iEntry;
	if (m_Library.LookupEx(dwUNID, &iEntry) != NOERROR)
		return NULL;

	CObjectImage *pImage = (CObjectImage *)m_Library.GetValue(iEntry);

	return pImage->CreateCopy();
	}

void CImageLibrary::Sweep (void)

//	Sweep
//
//	Sweeps all unmarked/unused images

	{
	for (int i = 0; i < m_Library.GetCount(); i++)
		{
		CObjectImage *pImage = (CObjectImage *)m_Library.GetValue(i);
		pImage->Sweep();
		}
	}

