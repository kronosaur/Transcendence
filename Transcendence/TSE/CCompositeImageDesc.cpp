//	CCompositeImageDesc.cpp
//
//	CCompositeImageDesc class
//
//	An entry is one of the following:
//
//	<Image .../>
//
//	<Composite>
//		{ordered list of entries}
//	</Composite>
//
//	<Effect>
//		{some effect tag}
//	</Effect>
//
//	<FilterColorize color="...">
//		{an entry}
//	</FilterColorize>
//
//	<LocationCriteriaTable>
//		<{entry} criteria="..." />
//	</LocationCriteriaTable>
//
//	<Table>
//		{set of entries}
//	</Table>
//
//	The outer-most entry must be one of the following:
//
//	<Image .../>
//
//	<Image>
//		{an entry}
//	</Image>
//
//	<ImageComposite>
//		{ordered list of entries}
//	</ImageComposite>
//
//	An <ImageVariants> element is:
//
//	<ImageVariants>
//		{set of entries}
//	</ImageVariants>

#include "PreComp.h"

#define COMPOSITE_TAG							CONSTLIT("Composite")
#define EFFECT_TAG								CONSTLIT("Effect")
#define FILTER_COLORIZE_TAG						CONSTLIT("FilterColorize")
#define IMAGE_TAG								CONSTLIT("Image")
#define IMAGE_COMPOSITE_TAG						CONSTLIT("ImageComposite")
#define IMAGE_EFFECT_TAG						CONSTLIT("ImageEffect")
#define IMAGE_VARIANTS_TAG						CONSTLIT("ImageVariants")
#define LOCATION_CRITERIA_TABLE_TAG				CONSTLIT("LocationCriteriaTable")
#define TABLE_TAG								CONSTLIT("Table")

#define CHANCE_ATTRIB							CONSTLIT("chance")
#define COLOR_ATTRIB							CONSTLIT("color")
#define CRITERIA_ATTRIB							CONSTLIT("criteria")
#define EFFECT_ATTRIB							CONSTLIT("effect")
#define HUE_ATTRIB								CONSTLIT("hue")
#define SATURATION_ATTRIB						CONSTLIT("saturation")

class CCompositeEntry : public IImageEntry
	{
	public:
		CCompositeEntry (void) : m_pImageCache(NULL) { }
		virtual ~CCompositeEntry (void);

		virtual void AddTypesUsed (TSortMap<DWORD, bool> *retTypesUsed) { retTypesUsed->SetAt(m_Image.GetBitmapUNID(), true); }
		virtual void GetImage (const CCompositeImageSelector &Selector, CObjectImageArray *retImage);
		virtual int GetMaxLifetime (void) const;
		virtual int GetVariantCount (void) { return 1; }
		virtual ALERROR InitFromXML (SDesignLoadCtx &Ctx, CIDCounter &IDGen, CXMLElement *pDesc);
		virtual void InitSelector (SSelectorInitCtx &InitCtx, CCompositeImageSelector *retSelector);
		virtual bool IsConstant (void);
		virtual void MarkImage (void);
		virtual ALERROR OnDesignLoadComplete (SDesignLoadCtx &Ctx);

	private:
		TArray<IImageEntry *> m_Layers;

		CObjectImageArray m_Image;
		CG16bitImage *m_pImageCache;
	};

class CEffectEntry : public IImageEntry
	{
	public:
		CEffectEntry (void) { }
		virtual ~CEffectEntry (void);

		virtual void AddTypesUsed (TSortMap<DWORD, bool> *retTypesUsed) { if (m_pEffect) m_pEffect->AddTypesUsed(retTypesUsed); }
		virtual void GetImage (const CCompositeImageSelector &Selector, CObjectImageArray *retImage);
		virtual int GetMaxLifetime (void) const;
		virtual int GetVariantCount (void) { return 1; }
		virtual ALERROR InitFromXML (SDesignLoadCtx &Ctx, CIDCounter &IDGen, CXMLElement *pDesc);
		virtual bool IsConstant (void) { return true; }
		virtual void MarkImage (void);
		virtual ALERROR OnDesignLoadComplete (SDesignLoadCtx &Ctx);

	private:
		CEffectCreatorRef m_pEffect;
	};

class CFilterColorizeEntry : public IImageEntry
	{
	public:
		CFilterColorizeEntry (void) : m_pSource(NULL) { }
		virtual ~CFilterColorizeEntry (void);

		virtual void AddTypesUsed (TSortMap<DWORD, bool> *retTypesUsed) { if (m_pSource) m_pSource->AddTypesUsed(retTypesUsed); }
		virtual void GetImage (const CCompositeImageSelector &Selector, CObjectImageArray *retImage);
		virtual int GetMaxLifetime (void) const;
		virtual int GetVariantCount (void) { return 1; }
		virtual ALERROR InitFromXML (SDesignLoadCtx &Ctx, CIDCounter &IDGen, CXMLElement *pDesc);
		virtual void InitSelector (SSelectorInitCtx &InitCtx, CCompositeImageSelector *retSelector);
		virtual bool IsConstant (void);
		virtual void MarkImage (void);
		virtual ALERROR OnDesignLoadComplete (SDesignLoadCtx &Ctx);

	private:
		IImageEntry *m_pSource;
		DWORD m_dwHue;
		DWORD m_dwSaturation;
		WORD m_wColor;
	};

class CImageEntry : public IImageEntry
	{
	public:
		virtual void AddTypesUsed (TSortMap<DWORD, bool> *retTypesUsed) { retTypesUsed->SetAt(m_Image.GetBitmapUNID(), true); }
		virtual void GetImage (const CCompositeImageSelector &Selector, CObjectImageArray *retImage) { *retImage = m_Image; }
		virtual int GetMaxLifetime (void) const { return m_Image.GetFrameCount() * m_Image.GetTicksPerFrame(); }
		virtual int GetVariantCount (void) { return 1; }
		virtual ALERROR InitFromXML (SDesignLoadCtx &Ctx, CIDCounter &IDGen, CXMLElement *pDesc);
		virtual bool IsConstant (void) { return true; }
		virtual void MarkImage (void) { m_Image.MarkImage(); }
		virtual ALERROR OnDesignLoadComplete (SDesignLoadCtx &Ctx) { return m_Image.OnDesignLoadComplete(Ctx); }

	private:
		CObjectImageArray m_Image;
	};

class CLocationCriteriaTableEntry : public IImageEntry
	{
	public:
		virtual ~CLocationCriteriaTableEntry (void);

		virtual void AddTypesUsed (TSortMap<DWORD, bool> *retTypesUsed);
		virtual void GetImage (const CCompositeImageSelector &Selector, CObjectImageArray *retImage);
		virtual int GetMaxLifetime (void) const;
		virtual int GetVariantCount (void) { return m_Table.GetCount(); }
		virtual ALERROR InitFromXML (SDesignLoadCtx &Ctx, CIDCounter &IDGen, CXMLElement *pDesc);
		virtual void InitSelector (SSelectorInitCtx &InitCtx, CCompositeImageSelector *retSelector);
		virtual bool IsConstant (void) { return (m_Table.GetCount() == 0 || ((m_Table.GetCount() == 1) && m_Table[0].pImage->IsConstant())); }
		virtual void MarkImage (void);
		virtual ALERROR OnDesignLoadComplete (SDesignLoadCtx &Ctx);

	private:
		struct SEntry
			{
			IImageEntry *pImage;
			CAttributeCriteria Criteria;
			};

		TArray<SEntry> m_Table;
		int m_iDefault;
	};

class CTableEntry : public IImageEntry
	{
	public:
		virtual ~CTableEntry (void);

		virtual void AddTypesUsed (TSortMap<DWORD, bool> *retTypesUsed);
		virtual void GetImage (const CCompositeImageSelector &Selector, CObjectImageArray *retImage);
		virtual int GetMaxLifetime (void) const;
		virtual int GetVariantCount (void) { return m_Table.GetCount(); }
		virtual ALERROR InitFromXML (SDesignLoadCtx &Ctx, CIDCounter &IDGen, CXMLElement *pDesc);
		virtual void InitSelector (SSelectorInitCtx &InitCtx, CCompositeImageSelector *retSelector);
		virtual bool IsConstant (void) { return (m_Table.GetCount() == 0 || ((m_Table.GetCount() == 1) && m_Table[0].pImage->IsConstant())); }
		virtual void MarkImage (void);
		virtual ALERROR OnDesignLoadComplete (SDesignLoadCtx &Ctx);

	private:
		struct SEntry
			{
			IImageEntry *pImage;
			int iChance;
			};

		TArray<SEntry> m_Table;
		int m_iTotalChance;
	};

static CObjectImageArray EMPTY_IMAGE;

CCompositeImageDesc::CCompositeImageDesc (void) : m_pRoot(NULL)

//	CCompositeImageDesc constructor

	{
	}

CCompositeImageDesc::~CCompositeImageDesc (void)

//	CCompositeImageDesc destructor

	{
	if (m_pRoot)
		delete m_pRoot;
	}

CCompositeImageDesc::SCacheEntry *CCompositeImageDesc::FindCacheEntry (const CCompositeImageSelector &Selector, const CCompositeImageModifiers &Modifiers) const

//	FindCacheEntry
//
//	Returns the cached entry (or NULL)

	{
	int i;

	//	If we're constant, then we can short-circuit, since there is only
	//	one selector.

	if (m_bConstant)
		{
		for (i = 0; i < m_Cache.GetCount(); i++)
			if (m_Cache[i].Modifiers == Modifiers)
				return &m_Cache[i];

		return NULL;
		}

	//	Otherwise, look in the cache for the proper selector

	for (i = 0; i < m_Cache.GetCount(); i++)
		if (m_Cache[i].Selector == Selector && m_Cache[i].Modifiers == Modifiers)
			return &m_Cache[i];

	return NULL;
	}

CObjectImageArray &CCompositeImageDesc::GetImage (const CCompositeImageSelector &Selector, const CCompositeImageModifiers &Modifiers, int *retiRotation) const

//	GetImage
//
//	Returns the image

	{
	CCompositeImageSelector::ETypes iType = Selector.GetType(DEFAULT_SELECTOR_ID);

	//	If the selector has a wreck image, then we use that

	if (iType == CCompositeImageSelector::typeShipClass)
		{
		if (retiRotation)
			*retiRotation = Selector.GetVariant(DEFAULT_SELECTOR_ID);

		return Selector.GetShipwreckImage(DEFAULT_SELECTOR_ID);
		}

	//	If the selector has an item image, then we use that

	else if (iType == CCompositeImageSelector::typeItemType)
		{
		if (retiRotation)
			*retiRotation = 0;

		return Selector.GetFlotsamImage(DEFAULT_SELECTOR_ID);
		}

	//	Get the image from the root entry

	else if (m_pRoot)
		{
		//	retiRotation is used as the vertical index into the image.
		//	Since each variant is expected to be its own image, we set to 0.

		if (retiRotation)
			*retiRotation = 0;

		//	Look in the cache

		SCacheEntry *pEntry = FindCacheEntry(Selector, Modifiers);
		if (pEntry)
			return pEntry->Image;

		//	If not in the cache, add a new entry

		pEntry = m_Cache.Insert();
		pEntry->Selector = Selector;
		pEntry->Modifiers = Modifiers;
		m_pRoot->GetImage(Selector, &pEntry->Image);

		//	Apply modifiers

		if (!Modifiers.IsEmpty())
			Modifiers.Apply(&pEntry->Image);

		//	Done

		return pEntry->Image;
		}
	else
		return EMPTY_IMAGE;
	}

int CCompositeImageDesc::GetMaxLifetime (void) const

//	GetMaxLifetime
//
//	Get maximum lifetime

	{
	if (m_pRoot == NULL)
		return 0;

	return m_pRoot->GetMaxLifetime();
	}

ALERROR CCompositeImageDesc::InitEntryFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, CIDCounter &IDGen, IImageEntry **retpEntry)

//	InitEntryFromXML
//
//	Initializes the desc from XML

	{
	ALERROR error;

	IImageEntry *pEntry;
	if (strEquals(pDesc->GetTag(), IMAGE_TAG))
		{
		if (pDesc->GetContentElementCount() == 0)
			pEntry = new CImageEntry;
		else
			return InitEntryFromXML(Ctx, pDesc->GetContentElement(0), IDGen, retpEntry);
		}
	else if (strEquals(pDesc->GetTag(), IMAGE_COMPOSITE_TAG) || strEquals(pDesc->GetTag(), COMPOSITE_TAG))
		pEntry = new CCompositeEntry;
	else if (strEquals(pDesc->GetTag(), IMAGE_EFFECT_TAG) || strEquals(pDesc->GetTag(), EFFECT_TAG))
		pEntry = new CEffectEntry;
	else if (strEquals(pDesc->GetTag(), TABLE_TAG) || strEquals(pDesc->GetTag(), IMAGE_VARIANTS_TAG))
		pEntry = new CTableEntry;
	else if (strEquals(pDesc->GetTag(), FILTER_COLORIZE_TAG))
		pEntry = new CFilterColorizeEntry;
	else if (strEquals(pDesc->GetTag(), LOCATION_CRITERIA_TABLE_TAG))
		pEntry = new CLocationCriteriaTableEntry;
	else
		{
		Ctx.sError = strPatternSubst(CONSTLIT("Unknown image tag: %s"), pDesc->GetTag());
		return ERR_FAIL;
		}

	//	Init

	if (error = pEntry->InitFromXML(Ctx, IDGen, pDesc))
		return error;

	*retpEntry = pEntry;

	return NOERROR;
	}

ALERROR CCompositeImageDesc::InitFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	InitFromXML
//
//	Initializes the desc from XML

	{
	ALERROR error;

	ASSERT(m_pRoot == NULL);

	CIDCounter IDGen;
	if (error = InitEntryFromXML(Ctx, pDesc, IDGen, &m_pRoot))
		return error;

	m_bConstant = m_pRoot->IsConstant();

	return NOERROR;
	}

void CCompositeImageDesc::InitSelector (SSelectorInitCtx &InitCtx, CCompositeImageSelector *retSelector)

//	InitSelector
//
//	Initializes the selector based on the variants

	{
	if (m_pRoot == NULL)
		return;

	m_pRoot->InitSelector(InitCtx, retSelector);
	}

void CCompositeImageDesc::MarkImage (void)

//	MarkImage
//
//	Marks all images

	{
	if (m_pRoot == NULL)
		return;

	m_pRoot->MarkImage();
	}

void CCompositeImageDesc::MarkImage (const CCompositeImageSelector &Selector, const CCompositeImageModifiers &Modifiers)

//	MarkImage
//
//	Marks the image in use

	{
	GetImage(Selector, Modifiers).MarkImage();
	}

ALERROR CCompositeImageDesc::OnDesignLoadComplete (SDesignLoadCtx &Ctx)

//	OnDesignLoadComplete
//
//	Handle deferred loading

	{
	ALERROR error;

	if (m_pRoot == NULL)
		return NOERROR;

	//	Load
	//
	//	NOTE: We rely on the fact that the cache is empty before OnDesignLoadComplete.
	//	If any calls to GetImage are made before this call, then the cache will have
	//	an invalid (NULL) image.

	if (error = m_pRoot->OnDesignLoadComplete(Ctx))
		return error;

	return NOERROR;
	}

void CCompositeImageDesc::Reinit (void)

//	Reinit
//
//	Reinitialize

	{
	m_Cache.DeleteAll();
	}

//	CCompositeEntry ------------------------------------------------------------

CCompositeEntry::~CCompositeEntry (void)

//	CCompositeEntry destructor

	{
	int i;

	for (i = 0; i < m_Layers.GetCount(); i++)
		delete m_Layers[i];

	if (m_pImageCache)
		delete m_pImageCache;
	}

void CCompositeEntry::GetImage (const CCompositeImageSelector &Selector, CObjectImageArray *retImage)

//	GetImage
//
//	Fills in the image

	{
	int i;

	//	Null case

	if (m_Layers.GetCount() == 0)
		{
		*retImage = EMPTY_IMAGE;
		return;
		}

	//	Get all the layers

	TArray<CObjectImageArray> Result;
	Result.InsertEmpty(m_Layers.GetCount());
	for (i = 0; i < m_Layers.GetCount(); i++)
		m_Layers[i]->GetImage(Selector, &Result[i]);

	//	Create the composited image
	//
	//	First we need to determine the size of the final image, based
	//	on the size and position of each layer.

	int xMin = 0;
	int xMax = 0;
	int yMin = 0;
	int yMax = 0;

	for (i = 0; i < m_Layers.GetCount(); i++)
		{
		CObjectImageArray &LayerImage = Result[i];
		const RECT &rcRect = LayerImage.GetImageRect();

		int xImageOffset = 0;
		int yImageOffset = 0;

		int xMaxImage = (RectWidth(rcRect) / 2) + xImageOffset;
		int xMinImage = xMaxImage - RectWidth(rcRect);
		int yMaxImage = (RectHeight(rcRect) / 2) + yImageOffset;
		int yMinImage = yMaxImage - RectHeight(rcRect);

		xMin = Min(xMin, xMinImage);
		xMax = Max(xMax, xMaxImage);
		yMin = Min(yMin, yMinImage);
		yMax = Max(yMax, yMaxImage);
		}

	//	Create destination image

	int cxWidth = xMax - xMin;
	int cyHeight = yMax - yMin;
	if (cxWidth <= 0 || cyHeight <= 0)
		{
		*retImage = EMPTY_IMAGE;
		return;
		}

	CG16bitImage *pComp = new CG16bitImage;
	pComp->CreateBlank(cxWidth, cyHeight, false);
	pComp->SetTransparentColor();

	int xCenter = cxWidth / 2;
	int yCenter = cyHeight / 2;

	//	Blt on the destination

	for (i = 0; i < m_Layers.GetCount(); i++)
		{
		CObjectImageArray &LayerImage = Result[i];
		const RECT &rcRect = LayerImage.GetImageRect();

		//	Paint the image

		LayerImage.PaintImage(*pComp,
				xCenter,
				yCenter,
				0,
				0);
		}

	//	Initialize an image

	RECT rcFinalRect;
	rcFinalRect.left = 0;
	rcFinalRect.top = 0;
	rcFinalRect.right = cxWidth;
	rcFinalRect.bottom = cyHeight;

	CObjectImageArray Comp;
	Comp.Init(pComp, rcFinalRect, 0, 0, true);

	//	Done

	retImage->TakeHandoff(Comp);
	}

int CCompositeEntry::GetMaxLifetime (void) const

//	GetMaxLifetime
//
//	Returns the maximum lifetime

	{
	int i;
	int iMaxLifetime = 0;

	for (i = 0; i < m_Layers.GetCount(); i++)
		{
		int iLifetime = m_Layers[i]->GetMaxLifetime();
		if (iLifetime > iMaxLifetime)
			iMaxLifetime = iLifetime;
		}

	return iMaxLifetime;
	}

ALERROR CCompositeEntry::InitFromXML (SDesignLoadCtx &Ctx, CIDCounter &IDGen, CXMLElement *pDesc)

//	InitFromXML
//
//	Initialize from XML

	{
	ALERROR error;
	int i;

	m_dwID = IDGen.GetID();

	//	Load each sub-entry in turn

	int iCount = pDesc->GetContentElementCount();
	if (iCount == 0)
		return NOERROR;

	m_Layers.InsertEmpty(iCount);
	for (i = 0; i < iCount; i++)
		{
		if (error = CCompositeImageDesc::InitEntryFromXML(Ctx, pDesc->GetContentElement(i), IDGen, &m_Layers[i]))
			return error;
		}

	//	Done

	return NOERROR;
	}

void CCompositeEntry::InitSelector (SSelectorInitCtx &InitCtx, CCompositeImageSelector *retSelector)

//	InitSelector
//
//	Initializes the selector

	{
	int i;

	for (i = 0; i < m_Layers.GetCount(); i++)
		m_Layers[i]->InitSelector(InitCtx, retSelector);
	}

bool CCompositeEntry::IsConstant (void)

//	IsConstant
//
//	Returns TRUE if this is a constant entry

	{
	int i;

	for (i = 0; i < m_Layers.GetCount(); i++)
		if (!m_Layers[i]->IsConstant())
			return false;

	return true;
	}

void CCompositeEntry::MarkImage (void)

//	MarkImage
//
//	Mark all images

	{
	int i;

	for (i = 0; i < m_Layers.GetCount(); i++)
		m_Layers[i]->MarkImage();
	}

ALERROR CCompositeEntry::OnDesignLoadComplete (SDesignLoadCtx &Ctx)

//	OnDesignLoadComplete
//
//	Bind design

	{
	ALERROR error;
	int i;
	
	for (i = 0; i < m_Layers.GetCount(); i++)
		{
		if (error = m_Layers[i]->OnDesignLoadComplete(Ctx))
			return error;
		}

	return NOERROR;
	}

//	CEffectEntry ---------------------------------------------------------------

CEffectEntry::~CEffectEntry (void)

//	CFilterColorizeEntry destructor

	{
	}

void CEffectEntry::GetImage (const CCompositeImageSelector &Selector, CObjectImageArray *retImage)

//	GetImage
//
//	Fills in the image

	{
	//	Short circuit

	if (m_pEffect.IsEmpty())
		{
		*retImage = EMPTY_IMAGE;
		return;
		}

	//	Create a painter

	CCreatePainterCtx Ctx;
	IEffectPainter *pPainter = m_pEffect->CreatePainter(Ctx);
	if (pPainter == NULL)
		{
		*retImage = EMPTY_IMAGE;
		return;
		}

	//	Get the painter bounds

	RECT rcBounds;
	pPainter->GetBounds(&rcBounds);
	int cxWidth = RectWidth(rcBounds);
	int cyHeight = RectHeight(rcBounds);

	//	Manual compositing

	bool bCanComposite = pPainter->CanPaintComposite();

	//	Create a resulting image

	CG16bitImage *pDest = new CG16bitImage;
	pDest->CreateBlank(cxWidth, cyHeight, true, 0x00, (bCanComposite ? 0x00 : 0xff));

	//	Set up paint context

	SViewportPaintCtx PaintCtx;

	//	Since we don't have an object, we use the viewport center to indicate
	//	the center of the object.

	PaintCtx.xCenter = -rcBounds.left;
	PaintCtx.yCenter = -rcBounds.top;

	//	Paint

	pPainter->PaintComposite(*pDest, (cxWidth / 2), (cyHeight / 2), PaintCtx);

	//	Initialize an image

	RECT rcFinalRect;
	rcFinalRect.left = 0;
	rcFinalRect.top = 0;
	rcFinalRect.right = cxWidth;
	rcFinalRect.bottom = cyHeight;

	CObjectImageArray Comp;
	Comp.Init(pDest, rcFinalRect, 0, 0, true);

	//	Done

	retImage->TakeHandoff(Comp);
	pPainter->Delete();
	}

int CEffectEntry::GetMaxLifetime (void) const

//	GetMaxLifetime
//
//	Returns the maximum lifetime

	{
	return 0;
	}

ALERROR CEffectEntry::InitFromXML (SDesignLoadCtx &Ctx, CIDCounter &IDGen, CXMLElement *pDesc)

//	InitFromXML
//
//	Initialize from XML

	{
	ALERROR error;

	m_dwID = IDGen.GetID();

	//	Load

	if (error = m_pEffect.LoadEffect(Ctx,
			NULL_STR,
			pDesc,
			pDesc->GetAttribute(EFFECT_ATTRIB)))
		return error;

	//	Done

	return NOERROR;
	}

void CEffectEntry::MarkImage (void)

//	MarkImage
//
//	Mark all images

	{
	if (m_pEffect)
		m_pEffect->MarkImages();
	}

ALERROR CEffectEntry::OnDesignLoadComplete (SDesignLoadCtx &Ctx)

//	OnDesignLoadComplete
//
//	Bind design

	{
	ALERROR error;

	if (error = m_pEffect.Bind(Ctx))
		return error;

	return NOERROR;
	}

//	CFilterColorizeEntry -------------------------------------------------------

CFilterColorizeEntry::~CFilterColorizeEntry (void)

//	CFilterColorizeEntry destructor

	{
	if (m_pSource)
		delete m_pSource;
	}

void CFilterColorizeEntry::GetImage (const CCompositeImageSelector &Selector, CObjectImageArray *retImage)

//	GetImage
//
//	Fills in the image

	{
	//	Null case

	if (m_pSource == NULL)
		{
		*retImage = EMPTY_IMAGE;
		return;
		}

	//	Get the source image (which we want to colorize)

	CObjectImageArray Source;
	m_pSource->GetImage(Selector, &Source);
	const RECT &rcSource = Source.GetImageRect();
	CG16bitImage &SourceImage = Source.GetImage(NULL_STR);
	int cxWidth = RectWidth(rcSource);
	int cyHeight = RectHeight(rcSource);
	if (!Source.IsLoaded() || cxWidth == 0 || cyHeight == 0)
		{
		*retImage = EMPTY_IMAGE;
		return;
		}

	//	Create the destination image

	CG16bitImage *pDest = new CG16bitImage;
	pDest->CreateBlank(cxWidth, cyHeight, SourceImage.HasAlpha());
	if (!SourceImage.HasAlpha())
		pDest->SetTransparentColor();

	//	Blt the to the destination with colorization

	CopyBltColorize(*pDest,
			0,
			0,
			cxWidth,
			cyHeight,
			SourceImage,
			rcSource.left,
			rcSource.top,
			m_dwHue,
			m_dwSaturation);

	//	Initialize an image

	RECT rcFinalRect;
	rcFinalRect.left = 0;
	rcFinalRect.top = 0;
	rcFinalRect.right = cxWidth;
	rcFinalRect.bottom = cyHeight;

	CObjectImageArray Comp;
	Comp.Init(pDest, rcFinalRect, 0, 0, true);

	//	Done

	retImage->TakeHandoff(Comp);
	}

int CFilterColorizeEntry::GetMaxLifetime (void) const

//	GetMaxLifetime
//
//	Returns the maximum lifetime

	{
	if (m_pSource == NULL)
		return 0;

	return m_pSource->GetMaxLifetime();
	}

ALERROR CFilterColorizeEntry::InitFromXML (SDesignLoadCtx &Ctx, CIDCounter &IDGen, CXMLElement *pDesc)

//	InitFromXML
//
//	Initialize from XML

	{
	ALERROR error;

	m_dwID = IDGen.GetID();

	CString sColor;
	if (pDesc->FindAttribute(COLOR_ATTRIB, &sColor))
		{
		COLORREF rgbColor = LoadCOLORREF(sColor);
		SColorHSB hsbColor = CG16bitPixel::RGBToHSB(CG16bitPixel::RGBToRGBReal(rgbColor));
		m_dwHue = (DWORD)hsbColor.rHue;
		m_dwSaturation = (DWORD)(hsbColor.rSaturation * 100.0);
		}
	else
		{
		m_dwHue = pDesc->GetAttributeIntegerBounded(HUE_ATTRIB, 0, 359, 0);
		m_dwSaturation = pDesc->GetAttributeIntegerBounded(SATURATION_ATTRIB, 0, 100, 100);
		}

	//	Load sub-entry

	if (pDesc->GetContentElementCount() == 0)
		return NOERROR;

	if (error = CCompositeImageDesc::InitEntryFromXML(Ctx, pDesc->GetContentElement(0), IDGen, &m_pSource))
		return error;

	//	Done

	return NOERROR;
	}

void CFilterColorizeEntry::InitSelector (SSelectorInitCtx &InitCtx, CCompositeImageSelector *retSelector)

//	InitSelector
//
//	Initializes the selector

	{
	if (m_pSource)
		m_pSource->InitSelector(InitCtx, retSelector);
	}

bool CFilterColorizeEntry::IsConstant (void)

//	IsConstant
//
//	Returns TRUE if this is a constant entry

	{
	if (m_pSource == NULL)
		return true;

	return m_pSource->IsConstant();
	}

void CFilterColorizeEntry::MarkImage (void)

//	MarkImage
//
//	Mark all images

	{
	if (m_pSource)
		m_pSource->MarkImage();
	}

ALERROR CFilterColorizeEntry::OnDesignLoadComplete (SDesignLoadCtx &Ctx)

//	OnDesignLoadComplete
//
//	Bind design

	{
	ALERROR error;

	if (m_pSource == NULL)
		return NOERROR;

	if (error = m_pSource->OnDesignLoadComplete(Ctx))
		return error;

	return NOERROR;
	}

//	CImageEntry ----------------------------------------------------------------

ALERROR CImageEntry::InitFromXML (SDesignLoadCtx &Ctx, CIDCounter &IDGen, CXMLElement *pDesc)

//	InitFromXML
//
//	Initialize from XML

	{
	ALERROR error;

	m_dwID = IDGen.GetID();

	//	Initialize the image

	if (error = m_Image.InitFromXML(Ctx, pDesc))
		return error;

	return NOERROR;
	}

//	CLocationCriteriaTableEntry ------------------------------------------------

CLocationCriteriaTableEntry::~CLocationCriteriaTableEntry (void)

//	CTableEntry destructor

	{
	int i;

	for (i = 0; i < m_Table.GetCount(); i++)
		delete m_Table[i].pImage;
	}

void CLocationCriteriaTableEntry::AddTypesUsed (TSortMap<DWORD, bool> *retTypesUsed)

//	AddTypesUsed
//
//	Add types used by this entry

	{
	int i;

	for (i = 0; i < m_Table.GetCount(); i++)
		m_Table[i].pImage->AddTypesUsed(retTypesUsed);
	}

void CLocationCriteriaTableEntry::GetImage (const CCompositeImageSelector &Selector, CObjectImageArray *retImage)

//	GetImage
//
//	Returns the image

	{
	int iIndex = Selector.GetVariant(GetID());
	if (iIndex < 0 || iIndex >= m_Table.GetCount())
		{
		*retImage = EMPTY_IMAGE;
		return;
		}

	m_Table[iIndex].pImage->GetImage(Selector, retImage);
	}

int CLocationCriteriaTableEntry::GetMaxLifetime (void) const

//	GetMaxLifetime
//
//	Returns the maximum lifetime

	{
	int i;
	int iMaxLifetime = 0;

	for (i = 0; i < m_Table.GetCount(); i++)
		{
		int iLifetime = m_Table[i].pImage->GetMaxLifetime();
		if (iLifetime > iMaxLifetime)
			iMaxLifetime = iLifetime;
		}

	return iMaxLifetime;
	}

ALERROR CLocationCriteriaTableEntry::InitFromXML (SDesignLoadCtx &Ctx, CIDCounter &IDGen, CXMLElement *pDesc)

//	InitFromXML
//
//	Initialize from XML

	{
	ALERROR error;
	int i;

	m_dwID = IDGen.GetID();
	m_iDefault = -1;

	//	Load each sub-entry in turn

	int iCount = pDesc->GetContentElementCount();
	if (iCount == 0)
		return NOERROR;

	m_Table.InsertEmpty(iCount);
	for (i = 0; i < iCount; i++)
		{
		CXMLElement *pItem = pDesc->GetContentElement(i);

		if (error = CCompositeImageDesc::InitEntryFromXML(Ctx, pItem, IDGen, &m_Table[i].pImage))
			return error;

		//	Load the criteria

		CString sCriteria = pItem->GetAttribute(CRITERIA_ATTRIB);
		if (error = m_Table[i].Criteria.Parse(sCriteria, 0, &Ctx.sError))
			return error;

		if (m_iDefault == -1 && m_Table[i].Criteria.MatchesDefault())
			m_iDefault = i;
		}

	//	If we don't have a default, the pick the last item.

	if (m_iDefault == -1 
			&& m_Table.GetCount() > 0)
		m_iDefault = m_Table.GetCount() - 1;

	//	Done

	return NOERROR;
	}

void CLocationCriteriaTableEntry::InitSelector (SSelectorInitCtx &InitCtx, CCompositeImageSelector *retSelector)

//	InitSelector
//
//	Initializes the selector

	{
	int i;

	//	Generate the table based on criteria and location

	TProbabilityTable<int> ProbTable;
	for (i = 0; i < m_Table.GetCount(); i++)
		{
		//	Compute the probability of this entry at the
		//	given location.

		int iChance = m_Table[i].Criteria.CalcLocationWeight(InitCtx.pSystem, InitCtx.sLocAttribs, InitCtx.vObjPos);
		if (iChance > 0)
			ProbTable.Insert(i, iChance);
		}

	//	If none match, then add the default element

	if (ProbTable.GetCount() == 0 && m_iDefault != -1)
		ProbTable.Insert(m_iDefault, 1000);

	//	Roll

	if (ProbTable.GetCount() > 0)
		{
		int iRoll = ProbTable.RollPos();
		int iIndex = ProbTable[iRoll];
		retSelector->AddVariant(GetID(), iIndex);
		m_Table[iIndex].pImage->InitSelector(InitCtx, retSelector);
		}
	}

void CLocationCriteriaTableEntry::MarkImage (void)

//	MarkImage
//
//	Mark all images

	{
	int i;

	for (i = 0; i < m_Table.GetCount(); i++)
		m_Table[i].pImage->MarkImage();
	}

ALERROR CLocationCriteriaTableEntry::OnDesignLoadComplete (SDesignLoadCtx &Ctx)

//	OnDesignLoadComplete
//
//	Bind design

	{
	ALERROR error;
	int i;
	
	for (i = 0; i < m_Table.GetCount(); i++)
		{
		if (error = m_Table[i].pImage->OnDesignLoadComplete(Ctx))
			return error;
		}

	return NOERROR;
	}

//	CTableEntry ----------------------------------------------------------------

CTableEntry::~CTableEntry (void)

//	CTableEntry destructor

	{
	int i;

	for (i = 0; i < m_Table.GetCount(); i++)
		delete m_Table[i].pImage;
	}

void CTableEntry::AddTypesUsed (TSortMap<DWORD, bool> *retTypesUsed)

//	AddTypesUsed
//
//	Add types used by this entry

	{
	int i;

	for (i = 0; i < m_Table.GetCount(); i++)
		m_Table[i].pImage->AddTypesUsed(retTypesUsed);
	}

void CTableEntry::GetImage (const CCompositeImageSelector &Selector, CObjectImageArray *retImage)

//	GetImage
//
//	Returns the image

	{
	int iIndex = Selector.GetVariant(GetID());
	if (iIndex < 0 || iIndex >= m_Table.GetCount())
		{
		*retImage = EMPTY_IMAGE;
		return;
		}

	m_Table[iIndex].pImage->GetImage(Selector, retImage);
	}

int CTableEntry::GetMaxLifetime (void) const

//	GetMaxLifetime
//
//	Returns the maximum lifetime

	{
	int i;
	int iMaxLifetime = 0;

	for (i = 0; i < m_Table.GetCount(); i++)
		{
		int iLifetime = m_Table[i].pImage->GetMaxLifetime();
		if (iLifetime > iMaxLifetime)
			iMaxLifetime = iLifetime;
		}

	return iMaxLifetime;
	}

ALERROR CTableEntry::InitFromXML (SDesignLoadCtx &Ctx, CIDCounter &IDGen, CXMLElement *pDesc)

//	InitFromXML
//
//	Initialize from XML

	{
	ALERROR error;
	int i;

	m_dwID = IDGen.GetID();

	//	Load each sub-entry in turn

	int iCount = pDesc->GetContentElementCount();
	if (iCount == 0)
		return NOERROR;

	m_iTotalChance = 0;
	m_Table.InsertEmpty(iCount);
	for (i = 0; i < iCount; i++)
		{
		CXMLElement *pItem = pDesc->GetContentElement(i);

		if (error = CCompositeImageDesc::InitEntryFromXML(Ctx, pItem, IDGen, &m_Table[i].pImage))
			return error;

		//	Load the chance

		m_Table[i].iChance = pItem->GetAttributeIntegerBounded(CHANCE_ATTRIB, 0, -1, 1);
		m_iTotalChance += m_Table[i].iChance;
		}

	//	Done

	return NOERROR;
	}

void CTableEntry::InitSelector (SSelectorInitCtx &InitCtx, CCompositeImageSelector *retSelector)

//	InitSelector
//
//	Initializes the selector

	{
	int i;

	//	Roll dice and pick an entry

	int iRoll = mathRandom(1, m_iTotalChance);
	for (i = 0; i < m_Table.GetCount(); i++)
		if (iRoll <= m_Table[i].iChance)
			{
			retSelector->AddVariant(GetID(), i);
			m_Table[i].pImage->InitSelector(InitCtx, retSelector);
			return;
			}
		else
			iRoll -= m_Table[i].iChance;
	}

void CTableEntry::MarkImage (void)

//	MarkImage
//
//	Mark all images

	{
	int i;

	for (i = 0; i < m_Table.GetCount(); i++)
		m_Table[i].pImage->MarkImage();
	}

ALERROR CTableEntry::OnDesignLoadComplete (SDesignLoadCtx &Ctx)

//	OnDesignLoadComplete
//
//	Bind design

	{
	ALERROR error;
	int i;
	
	for (i = 0; i < m_Table.GetCount(); i++)
		{
		if (error = m_Table[i].pImage->OnDesignLoadComplete(Ctx))
			return error;
		}

	return NOERROR;
	}
