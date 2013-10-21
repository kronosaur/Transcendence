//	CObjectImage.cpp
//
//	CObjectImage class

#include "PreComp.h"

#define UNID_ATTRIB							CONSTLIT("UNID")
#define BITMAP_ATTRIB						CONSTLIT("bitmap")
#define BITMASK_ATTRIB						CONSTLIT("bitmask")
#define BACK_COLOR_ATTRIB					CONSTLIT("backColor")
#define LOAD_ON_USE_ATTRIB					CONSTLIT("loadOnUse")
#define NO_PM_ATTRIB						CONSTLIT("noPM")
#define SPRITE_ATTRIB						CONSTLIT("sprite")

#define FIELD_IMAGE_DESC					CONSTLIT("imageDesc")

CObjectImage::CObjectImage (void) : 
		m_pBitmap(NULL)

//	CObjectImage constructor

	{
	}

CObjectImage::CObjectImage (CG16bitImage *pBitmap, bool bFreeBitmap) :
		m_pBitmap(pBitmap),
		m_bTransColor(false),
		m_bSprite(false),
		m_bPreMult(false),
		m_bLoadOnUse(false),
		m_bFreeBitmap(bFreeBitmap),
		m_bMarked(false),
		m_bLocked(true)

//	CObjectImage constructor

	{
	ASSERT(pBitmap);
	}

CObjectImage::~CObjectImage (void)

//	CObjectImage destructor

	{
	//	If we get created with an UNID of 0 it means that someone else owns the bitmap
	//	This is needed by CObjectImageArray.

	if (m_pBitmap && m_bFreeBitmap)
		delete m_pBitmap;
	}

CG16bitImage *CObjectImage::CreateCopy (CString *retsError)

//	CreateCopy
//
//	Creates a copy of the image. Caller must manually free the image when done.
//
//	NOTE: This is safe to call even if we have not bound.

	{
	//	If we have the image, the we need to make a copy

	if (m_pBitmap)
		{
		CG16bitImage *pResult = new CG16bitImage;
		if (pResult->CreateFromImage(*m_pBitmap) != NOERROR)
			return NULL;

		return pResult;
		}

	//	Otherwise, we load a copy

	CG16bitImage *pResult = GetImage(NULL_STR, retsError);
	m_pBitmap = NULL;	//	Clear out because we don't keep a copy

	return pResult;
	}

ALERROR CObjectImage::Exists (SDesignLoadCtx &Ctx)

//	Exists
//
//	Returns TRUE if the appropriate resources exist

	{
	if (m_pBitmap)
		return NOERROR;

	if (!m_sBitmap.IsBlank() && !Ctx.pResDb->ImageExists(NULL_STR, m_sBitmap))
		{
		Ctx.sError = strPatternSubst(CONSTLIT("Unable to find image: '%s'"), m_sBitmap);
		return ERR_FAIL;
		}

	if (!m_sBitmask.IsBlank() && !Ctx.pResDb->ImageExists(NULL_STR, m_sBitmask))
		{
		Ctx.sError = strPatternSubst(CONSTLIT("Unable to find image: '%s'"), m_sBitmask);
		return ERR_FAIL;
		}

	return NOERROR;
	}

bool CObjectImage::FindDataField (const CString &sField, CString *retsValue)

//	FindDataField
//
//	Returns data about the image

	{
	if (strEquals(sField, FIELD_IMAGE_DESC))
		{
		CG16bitImage *pImage = GetImage(CONSTLIT("data field"));
		if (pImage == NULL)
			{
			*retsValue = NULL_STR;
			return true;
			}

		*retsValue = strPatternSubst(CONSTLIT("=(0x%x 0 0 %d %d)"),
				GetUNID(),
				pImage->GetWidth(),
				pImage->GetHeight());
		}
	else
		return false;

	return true;
	}

CG16bitImage *CObjectImage::GetImage (const CString &sLoadReason, CString *retsError)

//	GetImage
//
//	Returns the image

	{
	try
		{
		//	If we have the image, we're done

		if (m_pBitmap)
			return m_pBitmap;

		//	Open the database

		CResourceDb ResDb(m_sResourceDb, !strEquals(m_sResourceDb, g_pUniverse->GetResourceDb()));
		if (ResDb.Open(DFOPEN_FLAG_READ_ONLY, retsError) != NOERROR)
			return NULL;

		return GetImage(ResDb, sLoadReason, retsError);
		}
	catch (...)
		{
		if (retsError)
			*retsError = strPatternSubst(CONSTLIT("Crash loading resource db: %s"), m_sResourceDb);

		return NULL;
		}
	}

CG16bitImage *CObjectImage::GetImage (CResourceDb &ResDb, const CString &sLoadReason, CString *retsError)

//	GetImage
//
//	Returns the image, loading it if necessary

	{
	ALERROR error;

	//	If we have the image, we're done

	if (m_pBitmap)
		return m_pBitmap;

	//	If necessary we log that we had to load an image (we generally do this
	//	to debug issues with loading images in the middle of play).

	if (g_pUniverse->InDebugMode() && g_pUniverse->LogImageLoad())
		kernelDebugLogMessage("Loading image %s for %s.", m_sBitmap, sLoadReason);

	//	Load the images

	HBITMAP hDIB = NULL;
	HBITMAP hBitmask = NULL;
	if (!m_sBitmap.IsBlank())
		{
		if (error = ResDb.LoadImage(NULL_STR, m_sBitmap, &hDIB))
			{
			if (retsError)
				*retsError = strPatternSubst(CONSTLIT("Unable to load image: '%s'"), m_sBitmap);
			return NULL;
			}
		}

	EBitmapTypes iMaskType = bitmapNone;
	if (!m_sBitmask.IsBlank())
		{
		if (error = ResDb.LoadImage(NULL_STR, m_sBitmask, &hBitmask, &iMaskType))
			{
			if (retsError)
				*retsError = strPatternSubst(CONSTLIT("Unable to load image: '%s'"), m_sBitmask);
			return NULL;
			}
		}

	//	Create a new CG16BitImage

	m_pBitmap = new CG16bitImage;
	if (m_pBitmap == NULL)
		{
		if (retsError)
			*retsError = CONSTLIT("Out of memory");
		return NULL;
		}

	error = m_pBitmap->CreateFromBitmap(hDIB, hBitmask, (m_bPreMult ? CG16bitImage::cfbPreMultAlpha : 0));

	//	We don't need these bitmaps anymore

	if (hDIB)
		{
		::DeleteObject(hDIB);
		hDIB = NULL;
		}

	if (hBitmask)
		{
		::DeleteObject(hBitmask);
		hBitmask = NULL;
		}

	//	Check for error

	if (error)
		{
		delete m_pBitmap;
		m_pBitmap = NULL;
		if (retsError)
			*retsError = strPatternSubst(CONSTLIT("Unable to create bitmap from image: '%s'"), m_sBitmap);
		return NULL;
		}

	//	Transparent color

	if (m_bTransColor)
		m_pBitmap->SetTransparentColor(m_wTransColor);

	//	If we have a monochrom mask, then we assume black is the background color

	else if (iMaskType == bitmapMonochrome)
		m_pBitmap->SetTransparentColor();

	//	Convert to sprite

	if (m_bSprite)
		m_pBitmap->ConvertToSprite();

	m_bFreeBitmap = true;
	return m_pBitmap;
	}

ALERROR CObjectImage::Lock (SDesignLoadCtx &Ctx)

//	Locks the image so that it is always loaded

	{
	//	Note: Since this is called on PrepareBindDesign, we can't
	//	assume that Ctx has the proper resource database. Thus
	//	we have to open it ourselves.

	CG16bitImage *pImage = GetImage(NULL_STR, &Ctx.sError);
	if (pImage == NULL)
		return ERR_FAIL;

	m_bLocked = true;

	return NOERROR;
	}

ALERROR CObjectImage::OnCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	CreateFromXML
//
//	Creates the object from and XML element

	{
	ALERROR error;

	//	Load on use

	if (!pDesc->FindAttributeBool(LOAD_ON_USE_ATTRIB, &m_bLoadOnUse))
		m_bLoadOnUse = true;

	//	Get the file paths

	m_sResourceDb = Ctx.sResDb;
	if (Ctx.sFolder.IsBlank())
		{
		m_sBitmap = pDesc->GetAttribute(BITMAP_ATTRIB);
		m_sBitmask = pDesc->GetAttribute(BITMASK_ATTRIB);
		}
	else
		{
		CString sFilespec;
		if (pDesc->FindAttribute(BITMAP_ATTRIB, &sFilespec))
			m_sBitmap = pathAddComponent(Ctx.sFolder, sFilespec);

		if (pDesc->FindAttribute(BITMASK_ATTRIB, &sFilespec))
			m_sBitmask = pathAddComponent(Ctx.sFolder, sFilespec);
		}

	//	Transparent color

	CString sTransColor;
	if (m_bTransColor = pDesc->FindAttribute(BACK_COLOR_ATTRIB, &sTransColor))
		m_wTransColor = LoadRGBColor(sTransColor);

	//	Sprite

	m_bSprite = pDesc->GetAttributeBool(SPRITE_ATTRIB);

	//	Pre-multiply transparency

	m_bPreMult = !pDesc->GetAttributeBool(NO_PM_ATTRIB);

	//	Initialize

	m_bMarked = false;
	m_bLocked = false;
	m_pBitmap = NULL;

	//	If we're loading on use, make sure the image exists. For other
	//	images we don't bother checking because we will load them
	//	soon enough.

	if (m_bLoadOnUse)
		if (error = Exists(Ctx))
			return error;

	//	Done

	return NOERROR;
	}

ALERROR CObjectImage::OnPrepareBindDesign (SDesignLoadCtx &Ctx)

//	OnPrepareBindDesign
//
//	Load the image, if necessary

	{
	ALERROR error;

	//	If we're not loading resources, then this doesn't matter

	if (Ctx.bNoResources)
		NULL;

	//	If we're supposed to load the image, load it now

	else if (!m_bLoadOnUse)
		{
		if (error = Lock(Ctx))
			return error;
		}

	//	Done

	return NOERROR;
	}

void CObjectImage::OnUnbindDesign (void)

//	OnUnbindDesign
//
//	Free our image

	{
	if (m_pBitmap && m_bLoadOnUse)
		{
		delete m_pBitmap;
		m_pBitmap = NULL;
		m_bLocked = false;
		}
	}

void CObjectImage::Sweep (void)

//	Sweep
//
//	Garbage collect the image, if it is not marked (i.e., in use)

	{
	if (!m_bLocked && !m_bMarked && m_pBitmap)
		{
		delete m_pBitmap;
		m_pBitmap = NULL;
		}
	}
