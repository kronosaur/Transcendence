//	CMultiverseNewsEntry.cpp
//
//	CMultiverseNewsEntry Class
//	Copyright (c) 2013 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"

#define FIELD_ID								CONSTLIT("id")
#define FIELD_BODY								CONSTLIT("body")
#define FIELD_BODY_TOP_IMAGE_URL				CONSTLIT("bodyTopImageURL")
#define FIELD_BODY_TOP_IMAGE_MASK_URL			CONSTLIT("bodyTopImageMaskURL")
#define FIELD_DEST_TEXT							CONSTLIT("destText")
#define FIELD_DEST_URL							CONSTLIT("destURL")
#define FIELD_DISPLAY_IF_IN_COLLECTION			CONSTLIT("displayIfInCollection")
#define FIELD_DISPLAY_UNLESS_IN_COLLECTION		CONSTLIT("displayUnlessInCollection")
#define FIELD_TITLE								CONSTLIT("title")

CMultiverseNewsEntry::CMultiverseNewsEntry (const CMultiverseNewsEntry &Src)

//	CMultiverseNewsEntry constructor

	{
	m_sID = Src.m_sID;
	m_sTitle = Src.m_sTitle;
	m_sBody = Src.m_sBody;
	m_sCallToActionText = Src.m_sCallToActionText;
	m_sCallToActionURL = Src.m_sCallToActionURL;
	m_sImageURL = Src.m_sImageURL;
	m_sImageMaskURL = Src.m_sImageMaskURL;

	m_sImageFilespec = Src.m_sImageFilespec;
	m_sImageMaskFilespec = Src.m_sImageMaskFilespec;
	m_Image.CreateFromImage(Src.m_Image);

	m_bShown = Src.m_bShown;
	}

CMultiverseNewsEntry::~CMultiverseNewsEntry (void)

//	CMultiverseNewsEntry destructor

	{
	}

ALERROR CMultiverseNewsEntry::CreateFromJSON (const CJSONValue &Entry, CMultiverseNewsEntry **retpEntry, CString *retsResult)

//	CreateFromJSON
//
//	Creates a new entry.

	{
	CMultiverseNewsEntry *pNewEntry = new CMultiverseNewsEntry;

	pNewEntry->m_sID = Entry.GetElement(FIELD_ID).AsString();
	pNewEntry->m_sTitle = Entry.GetElement(FIELD_TITLE).AsString();
	pNewEntry->m_sBody = Entry.GetElement(FIELD_BODY).AsString();
	pNewEntry->m_sCallToActionText = Entry.GetElement(FIELD_DEST_TEXT).AsString();
	pNewEntry->m_sCallToActionURL = Entry.GetElement(FIELD_DEST_URL).AsString();
	pNewEntry->m_sImageURL = Entry.GetElement(FIELD_BODY_TOP_IMAGE_URL).AsString();
	pNewEntry->m_sImageMaskURL = Entry.GetElement(FIELD_BODY_TOP_IMAGE_MASK_URL).AsString();

	ParseUNIDList(Entry.GetElement(FIELD_DISPLAY_IF_IN_COLLECTION).AsString(), PUL_FLAG_HEX, &pNewEntry->m_RequiredUNIDs);
	ParseUNIDList(Entry.GetElement(FIELD_DISPLAY_UNLESS_IN_COLLECTION).AsString(), PUL_FLAG_HEX, &pNewEntry->m_ExcludedUNIDs);

	//	Done

	*retpEntry = pNewEntry;

	return NOERROR;
	}

void CMultiverseNewsEntry::FindImage (const CString &sImageURL, 
									  const CString &sImageFilename,
									  const CString &sCacheFilespec,
									  CString *retsImageFilespec,
									  TSortMap<CString, CString> *retDownloads)

//	FindImage
//
//	Looks for the given image. If not found, we add it to the list of downloads

	{
	if (!sImageURL.IsBlank())
		{
		*retsImageFilespec = pathAddComponent(sCacheFilespec, strPatternSubst(CONSTLIT("%s_%s.%s"), sImageFilename, m_sID, pathGetExtension(m_sImageURL)));
		if (!pathExists(*retsImageFilespec))
			retDownloads->Insert(sImageURL, *retsImageFilespec);
		}
	}

void CMultiverseNewsEntry::FindImages (const CString &sCacheFilespec, TSortMap<CString, CString> *retDownloads)

//	FindImages
//
//	Looks for the required images in the given cache. If we can't find them, we
//	add them to the list of files to download.

	{
	FindImage(m_sImageURL,
			CONSTLIT("NewsImage"),
			sCacheFilespec,
			&m_sImageFilespec,
			retDownloads);

	FindImage(m_sImageMaskURL,
			CONSTLIT("NewImageMask"),
			sCacheFilespec,
			&m_sImageMaskFilespec,
			retDownloads);
	}

CG16bitImage *CMultiverseNewsEntry::LoadImage (void)

//	LoadImage
//
//	Loads and returns the image. Returns NULL if we don't have an image.

	{
	//	If the image is empty, try to load it.

	if (m_Image.IsEmpty())
		{
		//	If we have no image, we're done

		if (m_sImageFilespec.IsBlank())
			return NULL;

		try
			{
			m_Image.CreateFromFile(m_sImageFilespec, m_sImageMaskFilespec, 0);
			}
		catch (...)
			{
			::kernelDebugLogMessage("Crash loading image: %s.", m_sImageFilespec);
			return NULL;
			}
		}

	//	Done

	return &m_Image;
	}

CG16bitImage *CMultiverseNewsEntry::LoadImageHandoff (void)

//	LoadImageHandoff
//
//	Loads and returns the image. Returns NULL if we don't have an image.
//	Caller is responsible for freeing.

	{
	//	If we have no image, we're done

	if (m_sImageFilespec.IsBlank())
		return NULL;

	//	Load the image.

	try
		{
		CG16bitImage *pImage = new CG16bitImage;
		if (pImage->CreateFromFile(m_sImageFilespec, m_sImageMaskFilespec, 0) != NOERROR)
			{
			delete pImage;
			return NULL;
			}

		return pImage;
		}
	catch (...)
		{
		::kernelDebugLogMessage("Crash loading image: %s.", m_sImageFilespec);
		return NULL;
		}
	}
