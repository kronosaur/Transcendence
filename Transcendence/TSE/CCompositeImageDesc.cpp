//	CCompositeImageDesc.cpp
//
//	CCompositeImageDesc class
//
//	An entry is one of the following:
//
//	<Image .../>
//
//	<Image>
//		<Composite>
//			{ordered list of entries}
//		</Composite>
//	</Image>
//
//	<Image>
//		<Table>
//			{set of entries}
//		</Table>
//	</Image>
//
//	OPTIONAL:
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
#define IMAGE_TAG								CONSTLIT("Image")
#define IMAGE_COMPOSITE_TAG						CONSTLIT("ImageComposite")
#define IMAGE_VARIANTS_TAG						CONSTLIT("ImageVariants")
#define TABLE_TAG								CONSTLIT("Table")

#define CHANCE_ATTRIB							CONSTLIT("chance")

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
		virtual void InitSelector (CCompositeImageSelector *retSelector);
		virtual void InitSelector (int iVariant, CCompositeImageSelector *retSelector);
		virtual bool IsConstant (void);
		virtual void MarkImage (void);
		virtual ALERROR OnDesignLoadComplete (SDesignLoadCtx &Ctx);

	private:
		TArray<IImageEntry *> m_Layers;

		CObjectImageArray m_Image;
		CG16bitImage *m_pImageCache;
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

class CTableEntry : public IImageEntry
	{
	public:
		virtual ~CTableEntry (void);

		virtual void AddTypesUsed (TSortMap<DWORD, bool> *retTypesUsed);
		virtual void GetImage (const CCompositeImageSelector &Selector, CObjectImageArray *retImage);
		virtual int GetMaxLifetime (void) const;
		virtual int GetVariantCount (void) { return m_Table.GetCount(); }
		virtual ALERROR InitFromXML (SDesignLoadCtx &Ctx, CIDCounter &IDGen, CXMLElement *pDesc);
		virtual void InitSelector (CCompositeImageSelector *retSelector);
		virtual void InitSelector (int iVariant, CCompositeImageSelector *retSelector);
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
static IEffectPainter *g_pMediumDamage = NULL;
static IEffectPainter *g_pLargeDamage = NULL;

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

void CCompositeImageDesc::ApplyModifiers (DWORD dwModifiers, CObjectImageArray *retImage) const

//	ApplyModifiers
//
//	Apply modifiers to the image

	{
	if (dwModifiers & modStationDamage)
		{
		InitDamagePainters();

		//	Create a blank bitmap

		RECT rcImage = retImage->GetImageRect();
		int cxWidth = RectWidth(rcImage);
		int cyHeight = RectHeight(rcImage);
		CG16bitImage *pDest = new CG16bitImage;
		pDest->CreateBlank(cxWidth, cyHeight, true);

		rcImage.left = 0;
		rcImage.top = 0;
		rcImage.right = cxWidth;
		rcImage.bottom = cyHeight;

		//	Start with undamaged image

		retImage->CopyImage(*pDest,
				0,
				0,
				0,
				0);

		//	Add some large damage

		int iCount = (cxWidth / 32) * (cyHeight / 32);
		PaintDamage(*pDest, rcImage, iCount, g_pLargeDamage);

		//	Add some medium damage

		iCount = (cxWidth / 4) + (cyHeight / 4);
		PaintDamage(*pDest, rcImage, iCount, g_pMediumDamage);

		//	Flatten mask

		if (!retImage->HasAlpha())
			pDest->SetTransparentColor(0);

		//	Replace the image

		retImage->Init(pDest, rcImage, 0, 0, true);
		}
	}

CCompositeImageDesc::SCacheEntry *CCompositeImageDesc::FindCacheEntry (const CCompositeImageSelector &Selector, DWORD dwModifiers) const

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
			if (m_Cache[i].dwModifiers == dwModifiers)
				return &m_Cache[i];

		return NULL;
		}

	//	Otherwise, look in the cache for the proper selector

	for (i = 0; i < m_Cache.GetCount(); i++)
		if (m_Cache[i].Selector == Selector && m_Cache[i].dwModifiers == dwModifiers)
			return &m_Cache[i];

	return NULL;
	}

CObjectImageArray &CCompositeImageDesc::GetImage (const CCompositeImageSelector &Selector, DWORD dwModifiers, int *retiRotation) const

//	GetImage
//
//	Returns the image

	{
	//	If the selector has a wreck image, then we use that

	if (Selector.HasShipwreckImage(DEFAULT_SELECTOR_ID))
		{
		if (retiRotation)
			*retiRotation = Selector.GetVariant(DEFAULT_SELECTOR_ID);

		return Selector.GetShipwreckImage(DEFAULT_SELECTOR_ID);
		}

	//	Get the image from the root entry

	else if (m_pRoot)
		{
		//	retiRotation is used as the vertical index into the image.
		//	Since each variant is expected to be its own image, we set to 0.

		if (retiRotation)
			*retiRotation = 0;

		//	Look in the cache

		SCacheEntry *pEntry = FindCacheEntry(Selector, dwModifiers);
		if (pEntry)
			return pEntry->Image;

		//	If not in the cache, add a new entry

		pEntry = m_Cache.Insert();
		pEntry->Selector = Selector;
		pEntry->dwModifiers = dwModifiers;
		m_pRoot->GetImage(Selector, &pEntry->Image);

		//	Apply modifiers

		if (dwModifiers)
			ApplyModifiers(dwModifiers, &pEntry->Image);

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

void CCompositeImageDesc::InitDamagePainters (void)

//	InitDamagePainters
//
//	Initializes station damage bitmaps

	{
	if (g_pMediumDamage == NULL)
		{
		CEffectCreator *pEffect = g_pUniverse->FindEffectType(MEDIUM_STATION_DAMAGE_UNID);
		if (pEffect)
			g_pMediumDamage = pEffect->CreatePainter();
		}

	if (g_pLargeDamage == NULL)
		{
		CEffectCreator *pEffect = g_pUniverse->FindEffectType(LARGE_STATION_DAMAGE_UNID);
		if (pEffect)
			g_pLargeDamage = pEffect->CreatePainter();
		}
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
	else if (strEquals(pDesc->GetTag(), TABLE_TAG) || strEquals(pDesc->GetTag(), IMAGE_VARIANTS_TAG))
		pEntry = new CTableEntry;
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

void CCompositeImageDesc::InitSelector (CCompositeImageSelector *retSelector)

//	InitSelector
//
//	Initializes the selector based on the variants

	{
	if (m_pRoot == NULL)
		return;

	m_pRoot->InitSelector(retSelector);
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

void CCompositeImageDesc::MarkImage (const CCompositeImageSelector &Selector, DWORD dwModifiers)

//	MarkImage
//
//	Marks the image in use

	{
	GetImage(Selector, dwModifiers).MarkImage();
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

void CCompositeImageDesc::PaintDamage (CG16bitImage &Dest, const RECT &rcDest, int iCount, IEffectPainter *pPainter)

//	PaintDamage
//
//	Applies damage to the image

	{
	if (pPainter == NULL)
		return;

	//	Get some data

	int iVariantCount = pPainter->GetVariants();
	if (iVariantCount == 0)
		return;

	int iLifetime = pPainter->GetInitialLifetime();
	if (iLifetime == 0)
		return;

	//	Create context

	SViewportPaintCtx Ctx;

	//	Paint damage

	for (int i = 0; i < iCount; i++)
		{
		//	Pick a random position

		int x = rcDest.left + mathRandom(0, RectWidth(rcDest)-1);
		int y = rcDest.top + mathRandom(0, RectHeight(rcDest)-1);

		//	Set some parameters

		Ctx.iTick = mathRandom(0, iLifetime - 1);
		Ctx.iVariant = mathRandom(0, iVariantCount - 1);
		Ctx.iRotation = mathRandom(0, 359);
		Ctx.iDestiny = mathRandom(0, 359);

		//	Paint

		pPainter->Paint(Dest, x, y, Ctx);
		}
	}

void CCompositeImageDesc::Reinit (void)

//	Reinit
//
//	Reinitialize global data

	{
	if (g_pMediumDamage)
		{
		g_pMediumDamage->Delete();
		g_pMediumDamage = NULL;
		}

	if (g_pLargeDamage)
		{
		g_pLargeDamage->Delete();
		g_pLargeDamage = NULL;
		}
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
	pComp->SetTransparentColor(0);

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

void CCompositeEntry::InitSelector (CCompositeImageSelector *retSelector)

//	InitSelector
//
//	Initializes the selector

	{
	int i;

	for (i = 0; i < m_Layers.GetCount(); i++)
		m_Layers[i]->InitSelector(retSelector);
	}

void CCompositeEntry::InitSelector (int iVariant, CCompositeImageSelector *retSelector)

//	InitSelector
//
//	For now we just initialize a single entry

	{
	int i;

	for (i = 0; i < m_Layers.GetCount(); i++)
		m_Layers[i]->InitSelector(0, retSelector);
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

void CTableEntry::InitSelector (CCompositeImageSelector *retSelector)

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
			m_Table[i].pImage->InitSelector(retSelector);
			return;
			}
		else
			iRoll -= m_Table[i].iChance;
	}

void CTableEntry::InitSelector (int iVariant, CCompositeImageSelector *retSelector)

//	InitSelector
//
//	Initializes selector at the given variant

	{
	retSelector->AddVariant(GetID(), iVariant);
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
