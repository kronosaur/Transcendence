//	CDeviceTable.cpp
//
//	IDeviceGenerator objects

#include "PreComp.h"

#define DEVICE_TAG								CONSTLIT("Device")
#define DEVICES_TAG								CONSTLIT("Devices")
#define DEVICE_SLOT_TAG							CONSTLIT("DeviceSlot")
#define GROUP_TAG								CONSTLIT("Group")
#define ITEM_TAG								CONSTLIT("Item")
#define ITEMS_TAG								CONSTLIT("Items")
#define LEVEL_TABLE_TAG							CONSTLIT("LevelTable")
#define NULL_TAG								CONSTLIT("Null")
#define TABLE_TAG								CONSTLIT("Table")

#define CATEGORIES_ATTRIB						CONSTLIT("categories")
#define CHANCE_ATTRIB							CONSTLIT("chance")
#define COUNT_ATTRIB							CONSTLIT("count")
#define CRITERIA_ATTRIB							CONSTLIT("criteria")
#define DAMAGED_ATTRIB							CONSTLIT("damaged")
#define DEVICE_ID_ATTRIB						CONSTLIT("deviceID")
#define ENHANCED_ATTRIB							CONSTLIT("enhanced")
#define ENHANCEMENT_ATTRIB						CONSTLIT("enhancement")
#define EXTERNAL_ATTRIB							CONSTLIT("external")
#define HP_BONUS_ATTRIB							CONSTLIT("hpBonus")
#define ITEM_ATTRIB								CONSTLIT("item")
#define LEVEL_ATTRIB							CONSTLIT("level")
#define LEVEL_CURVE_ATTRIB						CONSTLIT("levelCurve")
#define LEVEL_FREQUENCY_ATTRIB					CONSTLIT("levelFrequency")
#define LINKED_FIRE_ATTRIB						CONSTLIT("linkedFire")
#define MAX_COUNT_ATTRIB						CONSTLIT("maxCount")
#define MAX_FIRE_ARC_ATTRIB						CONSTLIT("maxFireArc")
#define MIN_FIRE_ARC_ATTRIB						CONSTLIT("minFireArc")
#define OMNIDIRECTIONAL_ATTRIB					CONSTLIT("omnidirectional")
#define POS_ANGLE_ATTRIB						CONSTLIT("posAngle")
#define POS_RADIUS_ATTRIB						CONSTLIT("posRadius")
#define POS_Z_ATTRIB							CONSTLIT("posZ")
#define SECONDARY_WEAPON_ATTRIB					CONSTLIT("secondaryWeapon")
#define TABLE_ATTRIB							CONSTLIT("table")
#define UNID_ATTRIB								CONSTLIT("unid")

class CNullDevice : public IDeviceGenerator
	{
	};

class CSingleDevice : public IDeviceGenerator
	{
	public:
		CSingleDevice (void) : m_pExtraItems(NULL) { }
		~CSingleDevice (void);

		virtual void AddDevices (SDeviceGenerateCtx &Ctx);
		virtual void AddTypesUsed (TSortMap<DWORD, bool> *retTypesUsed);
		virtual ALERROR LoadFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);
		virtual ALERROR OnDesignLoadComplete (SDesignLoadCtx &Ctx);

	private:
		CItemTypeRef m_pItemType;
		DiceRange m_Count;
		int m_iDamaged;
		CRandomEnhancementGenerator m_Enhanced;

		int m_iPosAngle;
		int m_iPosRadius;
		int m_iPosZ;
		bool m_b3DPosition;
		bool m_bDefaultPos;
		bool m_bExternal;

		bool m_bOmnidirectional;
		int m_iMinFireArc;
		int m_iMaxFireArc;
		bool m_bDefaultFireArc;

		DWORD m_dwLinkedFireOptions;
		bool m_bDefaultLinkedFire;

		bool m_bSecondary;

		int m_iSlotBonus;
		bool m_bDefaultSlotBonus;

		IItemGenerator *m_pExtraItems;
	};

class CLevelTableOfDeviceGenerators : public IDeviceGenerator
	{
	public:
		virtual ~CLevelTableOfDeviceGenerators (void);
		virtual void AddDevices (SDeviceGenerateCtx &Ctx);
		virtual void AddTypesUsed (TSortMap<DWORD, bool> *retTypesUsed);
		virtual IDeviceGenerator *GetGenerator (int iIndex) { return m_Table[iIndex].pDevice; }
		virtual int GetGeneratorCount (void) { return m_Table.GetCount(); }
		virtual ALERROR LoadFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);
		virtual ALERROR OnDesignLoadComplete (SDesignLoadCtx &Ctx);

	private:
		struct SEntry
			{
			IDeviceGenerator *pDevice;
			CString sLevelFrequency;
			int iChance;
			};

		DiceRange m_Count;
		TArray<SEntry> m_Table;
		int m_iComputedLevel;
		int m_iTotalChance;
	};

class CTableOfDeviceGenerators : public IDeviceGenerator
	{
	public:
		virtual ~CTableOfDeviceGenerators (void);
		virtual void AddDevices (SDeviceGenerateCtx &Ctx);
		virtual void AddTypesUsed (TSortMap<DWORD, bool> *retTypesUsed);
		virtual IDeviceGenerator *GetGenerator (int iIndex) { return m_Table[iIndex].pDevice; }
		virtual int GetGeneratorCount (void) { return m_Table.GetCount(); }
		virtual ALERROR LoadFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);
		virtual ALERROR OnDesignLoadComplete (SDesignLoadCtx &Ctx);

	private:
		struct SEntry
			{
			IDeviceGenerator *pDevice;
			int iChance;
			};

		DiceRange m_Count;
		TArray<SEntry> m_Table;
		int m_iTotalChance;
	};

class CGroupOfDeviceGenerators : public IDeviceGenerator
	{
	public:
		virtual ~CGroupOfDeviceGenerators (void);
		virtual void AddDevices (SDeviceGenerateCtx &Ctx);
		virtual void AddTypesUsed (TSortMap<DWORD, bool> *retTypesUsed);
		virtual IDeviceGenerator *GetGenerator (int iIndex) { return m_Table[iIndex].pDevice; }
		virtual int GetGeneratorCount (void) { return m_Table.GetCount(); }
		virtual ALERROR LoadFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);
		virtual ALERROR OnDesignLoadComplete (SDesignLoadCtx &Ctx);

		virtual bool FindDefaultDesc (const CItem &Item, SDeviceDesc *retDesc);

	private:
		struct SEntry
			{
			IDeviceGenerator *pDevice;
			int iChance;
			};

		struct SSlotDesc
			{
			CItemCriteria Criteria;
			SDeviceDesc DefaultDesc;
			int iMaxCount;
			};

		SSlotDesc *FindSlotDesc (const CItem &Item);

		DiceRange m_Count;

		TArray<SEntry> m_Table;
		TArray<SSlotDesc> m_SlotDesc;
	};

ALERROR IDeviceGenerator::CreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, IDeviceGenerator **retpGenerator)

//	CreateFromXML
//
//	Creates a new generator

	{
	ALERROR error;
	IDeviceGenerator *pGenerator = NULL;

	if (strEquals(pDesc->GetTag(), DEVICE_TAG) || strEquals(pDesc->GetTag(), ITEM_TAG))
		pGenerator = new CSingleDevice;
	else if (strEquals(pDesc->GetTag(), TABLE_TAG))
		pGenerator = new CTableOfDeviceGenerators;
	else if (strEquals(pDesc->GetTag(), GROUP_TAG) || strEquals(pDesc->GetTag(), DEVICES_TAG))
		pGenerator = new CGroupOfDeviceGenerators;
	else if (strEquals(pDesc->GetTag(), LEVEL_TABLE_TAG))
		pGenerator = new CLevelTableOfDeviceGenerators;
	else if (strEquals(pDesc->GetTag(), NULL_TAG))
		pGenerator = new CNullDevice;
	else
		{
		Ctx.sError = strPatternSubst(CONSTLIT("Unknown device generator: %s"), pDesc->GetTag());
		return ERR_FAIL;
		}

	if (error = pGenerator->LoadFromXML(Ctx, pDesc))
		{
		if (pGenerator)
			delete pGenerator;
		return error;
		}

	*retpGenerator = pGenerator;

	return NOERROR;
	}

ALERROR IDeviceGenerator::InitDeviceDescFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, SDeviceDesc *retDesc)

//	InitDeviceDescFromXML
//
//	Loads a device desc from XML.

	{
	ALERROR error;

	retDesc->iPosAngle = AngleMod(pDesc->GetAttributeInteger(POS_ANGLE_ATTRIB));
	retDesc->iPosRadius = pDesc->GetAttributeInteger(POS_RADIUS_ATTRIB);
	if (!(retDesc->b3DPosition = pDesc->FindAttributeInteger(POS_Z_ATTRIB, &retDesc->iPosZ)))
		retDesc->iPosZ = 0;

	retDesc->bExternal = pDesc->GetAttributeBool(EXTERNAL_ATTRIB);
	retDesc->bOmnidirectional = pDesc->GetAttributeBool(OMNIDIRECTIONAL_ATTRIB);
	retDesc->iMinFireArc = AngleMod(pDesc->GetAttributeInteger(MIN_FIRE_ARC_ATTRIB));
	retDesc->iMaxFireArc = AngleMod(pDesc->GetAttributeInteger(MAX_FIRE_ARC_ATTRIB));

	if (error = CDeviceClass::ParseLinkedFireOptions(Ctx, pDesc->GetAttribute(LINKED_FIRE_ATTRIB), &retDesc->dwLinkedFireOptions))
		return error;

	retDesc->bSecondary = pDesc->GetAttributeBool(SECONDARY_WEAPON_ATTRIB);

	retDesc->iSlotBonus = pDesc->GetAttributeInteger(HP_BONUS_ATTRIB);

	return NOERROR;
	}

//	CSingleDevice -------------------------------------------------------------

CSingleDevice::~CSingleDevice (void)

//	CSingleDevice destructor

	{
	if (m_pExtraItems)
		delete m_pExtraItems;
	}

void CSingleDevice::AddDevices (SDeviceGenerateCtx &Ctx)

//	AddDevices
//
//	Add devices to list

	{
	int i;

	if (m_pItemType == NULL)
		return;

	int iCount = m_Count.Roll();
	for (i = 0; i < iCount; i++)
		{
		//	Initialize the desc

		SDeviceDesc Desc;
		Desc.Item = CItem(m_pItemType, 1);
		if (mathRandom(1, 100) <= m_iDamaged)
			Desc.Item.SetDamaged();
		else
			m_Enhanced.EnhanceItem(Desc.Item);

		//	Find the default settings for the device slot for this device

		SDeviceDesc SlotDesc;
		bool bUseSlotDesc = (Ctx.pRoot ? Ctx.pRoot->FindDefaultDesc(Desc.Item, &SlotDesc) : false);

		//	Set the device position appropriately, either from the <Device> element,
		//	from the slot descriptor at the root, or from defaults.

		if (!m_bDefaultPos)
			{
			Desc.iPosAngle = m_iPosAngle;
			Desc.iPosRadius = m_iPosRadius;
			Desc.iPosZ = m_iPosZ;
			Desc.b3DPosition = m_b3DPosition;
			}
		else if (bUseSlotDesc)
			{
			Desc.iPosAngle = SlotDesc.iPosAngle;
			Desc.iPosRadius = SlotDesc.iPosRadius;
			Desc.iPosZ = SlotDesc.iPosZ;
			Desc.b3DPosition = SlotDesc.b3DPosition;
			}

		//	External

		if (bUseSlotDesc)
			Desc.bExternal = SlotDesc.bExternal;
		else
			Desc.bExternal = m_bExternal;

		//	Set the device fire arc appropriately.

		if (!m_bDefaultFireArc)
			{
			Desc.bOmnidirectional = m_bOmnidirectional;
			Desc.iMinFireArc = m_iMinFireArc;
			Desc.iMaxFireArc = m_iMaxFireArc;
			}
		else if (bUseSlotDesc)
			{
			Desc.bOmnidirectional = SlotDesc.bOmnidirectional;
			Desc.iMinFireArc = SlotDesc.iMinFireArc;
			Desc.iMaxFireArc = SlotDesc.iMaxFireArc;
			}

		//	Set linked fire

		if (!m_bDefaultLinkedFire)
			Desc.dwLinkedFireOptions = m_dwLinkedFireOptions;
		else if (bUseSlotDesc)
			Desc.dwLinkedFireOptions = SlotDesc.dwLinkedFireOptions;
		else
			Desc.dwLinkedFireOptions = 0;

		Desc.bSecondary = m_bSecondary;

		//	Slot bonus

		if (!m_bDefaultSlotBonus)
			Desc.iSlotBonus = m_iSlotBonus;
		else if (bUseSlotDesc)
			Desc.iSlotBonus = SlotDesc.iSlotBonus;
		else
			Desc.iSlotBonus = 0;

		//	Add extra items

		if (m_pExtraItems)
			{
			CItemListManipulator ItemList(Desc.ExtraItems);
			SItemAddCtx ItemCtx(ItemList);
			ItemCtx.iLevel = Ctx.iLevel;

			m_pExtraItems->AddItems(ItemCtx);
			}

		//	Done
		
		Ctx.pResult->AddDeviceDesc(Desc);
		}
	}

void CSingleDevice::AddTypesUsed (TSortMap<DWORD, bool> *retTypesUsed)

//	AddTypesUsed
//
//	Adds list of types used.

	{
	if (m_pItemType)
		retTypesUsed->SetAt(m_pItemType->GetUNID(), true);

	if (m_pExtraItems)
		m_pExtraItems->AddTypesUsed(retTypesUsed);
	}

ALERROR CSingleDevice::LoadFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	LoadFromXML
//
//	Load from XML

	{
	ALERROR error;

	//	Load the item type

	CString sUNID = pDesc->GetAttribute(DEVICE_ID_ATTRIB);
	if (sUNID.IsBlank())
		sUNID = pDesc->GetAttribute(ITEM_ATTRIB);

	if (error = m_pItemType.LoadUNID(Ctx, sUNID))
		return error;

	if (m_pItemType.GetUNID() == 0)
		{
		Ctx.sError = strPatternSubst(CONSTLIT("<%s> element missing item attribute."), pDesc->GetTag());
		return ERR_FAIL;
		}

	//	Load the count

	m_Count.LoadFromXML(pDesc->GetAttribute(COUNT_ATTRIB));
	if (m_Count.IsEmpty())
		m_Count.SetConstant(1);

	//	Load damage chance

	m_iDamaged = pDesc->GetAttributeInteger(DAMAGED_ATTRIB);

	//	Load enhancement chance

	if (error = m_Enhanced.InitFromXML(Ctx, pDesc))
		return error;

	//	Load device position attributes

	if (pDesc->FindAttributeInteger(POS_Z_ATTRIB, &m_iPosZ))
		{
		m_iPosAngle = AngleMod(pDesc->GetAttributeInteger(POS_ANGLE_ATTRIB));
		m_iPosRadius = pDesc->GetAttributeInteger(POS_RADIUS_ATTRIB);
		m_b3DPosition = true;
		m_bDefaultPos = false;
		}
	else if (pDesc->FindAttributeInteger(POS_ANGLE_ATTRIB, &m_iPosAngle))
		{
		m_iPosAngle = AngleMod(m_iPosAngle);
		m_iPosRadius = pDesc->GetAttributeInteger(POS_RADIUS_ATTRIB);
		m_iPosZ = 0;
		m_b3DPosition = false;
		m_bDefaultPos = false;
		}
	else
		{
		m_iPosAngle = 0;
		m_iPosRadius = 0;
		m_iPosZ = 0;
		m_b3DPosition = false;
		m_bDefaultPos = true;
		}

	//	Load fire arc attributes

	if (pDesc->FindAttributeInteger(MIN_FIRE_ARC_ATTRIB, &m_iMinFireArc))
		{
		m_bOmnidirectional = false;
		m_iMinFireArc = AngleMod(m_iMinFireArc);
		m_iMaxFireArc = AngleMod(pDesc->GetAttributeInteger(MAX_FIRE_ARC_ATTRIB));
		m_bDefaultFireArc = false;
		}
	else if (pDesc->FindAttributeBool(OMNIDIRECTIONAL_ATTRIB, &m_bOmnidirectional))
		{
		m_iMinFireArc = 0;
		m_iMaxFireArc = 0;
		m_bDefaultFireArc = false;
		}
	else
		{
		m_bOmnidirectional = false;
		m_iMinFireArc = 0;
		m_iMaxFireArc = 0;
		m_bDefaultFireArc = true;
		}

	m_bExternal = pDesc->GetAttributeBool(EXTERNAL_ATTRIB);

	//	Linked fire options

	CString sLinkedFire;
	if (pDesc->FindAttribute(LINKED_FIRE_ATTRIB, &sLinkedFire))
		{
		if (error = CDeviceClass::ParseLinkedFireOptions(Ctx, sLinkedFire, &m_dwLinkedFireOptions))
			return error;

		m_bDefaultLinkedFire = false;
		}
	else
		{
		m_dwLinkedFireOptions = 0;
		m_bDefaultLinkedFire = true;
		}

	m_bSecondary = pDesc->GetAttributeBool(SECONDARY_WEAPON_ATTRIB);

	//	Slot bonus

	if (pDesc->FindAttributeInteger(HP_BONUS_ATTRIB, &m_iSlotBonus))
		m_bDefaultSlotBonus = false;
	else
		{
		m_iSlotBonus = 0;
		m_bDefaultSlotBonus = true;
		}

	//	Load extra items

	CXMLElement *pItems = pDesc->GetContentElementByTag(ITEMS_TAG);
	if (pItems)
		{
		if (error = IItemGenerator::CreateFromXML(Ctx, pItems, &m_pExtraItems))
			return error;
		}

	return NOERROR;
	}

ALERROR CSingleDevice::OnDesignLoadComplete (SDesignLoadCtx &Ctx)

//	OnDesignLoadComplete
//
//	Resolve references

	{
	ALERROR error;

	if (error = m_pItemType.Bind(Ctx))
		return error;

	if (m_pExtraItems)
		if (error = m_pExtraItems->OnDesignLoadComplete(Ctx))
			return error;

	//	Error checking

	if (m_pItemType)
		if (m_pItemType->GetDeviceClass() == NULL)
			{
			Ctx.sError = strPatternSubst(CONSTLIT("%s is not a device"), m_pItemType->GetName(NULL, true));
			return ERR_FAIL;
			}

	return NOERROR;
	}

//	CTableOfDeviceGenerators --------------------------------------------------

CTableOfDeviceGenerators::~CTableOfDeviceGenerators (void)

//	CTableOfDeviceGenerators destructor

	{
	int i;

	for (i = 0; i < m_Table.GetCount(); i++)
		if (m_Table[i].pDevice)
			delete m_Table[i].pDevice;
	}

void CTableOfDeviceGenerators::AddDevices (SDeviceGenerateCtx &Ctx)

//	AddDevices
//
//	Add devices

	{
	int i, j;

	int iCount = m_Count.Roll();
	for (j = 0; j < iCount; j++)
		{
		int iRoll = mathRandom(1, m_iTotalChance);

		for (i = 0; i < m_Table.GetCount(); i++)
			{
			iRoll -= m_Table[i].iChance;

			if (iRoll <= 0)
				{
				m_Table[i].pDevice->AddDevices(Ctx);
				break;
				}
			}
		}
	}

void CTableOfDeviceGenerators::AddTypesUsed (TSortMap<DWORD, bool> *retTypesUsed)

//	AddTypesUsed
//
//	Adds list of types used.

	{
	int i;

	for (i = 0; i < m_Table.GetCount(); i++)
		m_Table[i].pDevice->AddTypesUsed(retTypesUsed);
	}

ALERROR CTableOfDeviceGenerators::LoadFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	LoadFromXML
//
//	Load from XML

	{
	int i;
	ALERROR error;

	m_Count.LoadFromXML(pDesc->GetAttribute(COUNT_ATTRIB));
	if (m_Count.IsEmpty())
		m_Count.SetConstant(1);

	m_iTotalChance = 0;
	int iCount = pDesc->GetContentElementCount();
	if (iCount > 0)
		{
		m_Table.InsertEmpty(iCount);
		for (i = 0; i < iCount; i++)
			{
			CXMLElement *pEntry = pDesc->GetContentElement(i);
			
			m_Table[i].iChance = pEntry->GetAttributeInteger(CHANCE_ATTRIB);
			m_iTotalChance += m_Table[i].iChance;

			if (error = IDeviceGenerator::CreateFromXML(Ctx, pEntry, &m_Table[i].pDevice))
				{
				m_Table[i].pDevice = NULL;
				return error;
				}
			}
		}

	return NOERROR;
	}

ALERROR CTableOfDeviceGenerators::OnDesignLoadComplete (SDesignLoadCtx &Ctx)

//	OnDesignLoadComplete
//
//	Resolve references

	{
	ALERROR error;
	int i;

	for (i = 0; i < m_Table.GetCount(); i++)
		{
		if (error = m_Table[i].pDevice->OnDesignLoadComplete(Ctx))
			return error;
		}

	return NOERROR;
	}

//	CLevelTableOfDeviceGenerators ---------------------------------------------

CLevelTableOfDeviceGenerators::~CLevelTableOfDeviceGenerators (void)

//	CLevelTableOfDeviceGenerators destructor

	{
	int i;

	for (i = 0; i < m_Table.GetCount(); i++)
		if (m_Table[i].pDevice)
			delete m_Table[i].pDevice;
	}

void CLevelTableOfDeviceGenerators::AddDevices (SDeviceGenerateCtx &Ctx)

//	AddDevices
//
//	Adds devices

	{
	int i, j;

	//	Compute probabilities

	if (Ctx.iLevel != m_iComputedLevel)
		{
		m_iTotalChance = 0;
		for (i = 0; i < m_Table.GetCount(); i++)
			{
			m_Table[i].iChance = GetFrequencyByLevel(m_Table[i].sLevelFrequency, Ctx.iLevel);
			m_iTotalChance += m_Table[i].iChance;
			}

		m_iComputedLevel = Ctx.iLevel;
		}

	//	Generate

	if (m_iTotalChance)
		{
		int iCount = m_Count.Roll();

		for (i = 0; i < iCount; i++)
			{
			int iRoll = mathRandom(1, m_iTotalChance);

			for (j = 0; j < m_Table.GetCount(); j++)
				{
				iRoll -= m_Table[j].iChance;

				if (iRoll <= 0)
					{
					m_Table[j].pDevice->AddDevices(Ctx);
					break;
					}
				}
			}
		}
	}

void CLevelTableOfDeviceGenerators::AddTypesUsed (TSortMap<DWORD, bool> *retTypesUsed)

//	AddTypesUsed
//
//	Adds list of types used.

	{
	int i;

	for (i = 0; i < m_Table.GetCount(); i++)
		m_Table[i].pDevice->AddTypesUsed(retTypesUsed);
	}

ALERROR CLevelTableOfDeviceGenerators::LoadFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	LoadFromXML
//
//	Loads from XML

	{
	int i;
	ALERROR error;

	m_Count.LoadFromXML(pDesc->GetAttribute(COUNT_ATTRIB));
	if (m_Count.IsEmpty())
		m_Count.SetConstant(1);

	for (i = 0; i < pDesc->GetContentElementCount(); i++)
		{
		CXMLElement *pEntry = pDesc->GetContentElement(i);
		SEntry *pNewEntry = m_Table.Insert();

		pNewEntry->sLevelFrequency = pEntry->GetAttribute(LEVEL_FREQUENCY_ATTRIB);

		if (error = IDeviceGenerator::CreateFromXML(Ctx, pEntry, &pNewEntry->pDevice))
			{
			pNewEntry->pDevice = NULL;
			return error;
			}
		}

	m_iComputedLevel = -1;

	return NOERROR;
	}

ALERROR CLevelTableOfDeviceGenerators::OnDesignLoadComplete (SDesignLoadCtx &Ctx)

//	OnDesignLoadComplete
//
//	Bind design

	{
	ALERROR error;
	int i;

	m_iComputedLevel = -1;

	for (i = 0; i < m_Table.GetCount(); i++)
		{
		if (error = m_Table[i].pDevice->OnDesignLoadComplete(Ctx))
			return error;
		}

	return NOERROR;
	}

//	CGroupOfDeviceGenerators --------------------------------------------------

CGroupOfDeviceGenerators::~CGroupOfDeviceGenerators (void)

//	CGroupOfDeviceGenerators destructor

	{
	int i;

	for (i = 0; i < m_Table.GetCount(); i++)
		if (m_Table[i].pDevice)
			delete m_Table[i].pDevice;
	}

void CGroupOfDeviceGenerators::AddDevices (SDeviceGenerateCtx &Ctx)

//	AddDevices
//
//	Add devices

	{
	int i, j;

	int iCount = m_Count.Roll();
	for (j = 0; j < iCount; j++)
		{
		for (i = 0; i < m_Table.GetCount(); i++)
			{
			if (mathRandom(1, 100) <= m_Table[i].iChance)
				m_Table[i].pDevice->AddDevices(Ctx);
			}
		}
	}

void CGroupOfDeviceGenerators::AddTypesUsed (TSortMap<DWORD, bool> *retTypesUsed)

//	AddTypesUsed
//
//	Adds list of types used.

	{
	int i;

	for (i = 0; i < m_Table.GetCount(); i++)
		m_Table[i].pDevice->AddTypesUsed(retTypesUsed);
	}

bool CGroupOfDeviceGenerators::FindDefaultDesc (const CItem &Item, SDeviceDesc *retDesc)

//	FindDefaultDesc
//
//	Looks for a slot descriptor that matches the given item and returns it.

	{
	//	See if the item fits into one of the slots that we've defined. If so, 
	//	then we take the descriptor from the slot.

	SSlotDesc *pSlotDesc = FindSlotDesc(Item);
	if (pSlotDesc)
		{
		*retDesc = pSlotDesc->DefaultDesc;
		return true;
		}

	//	Otherwise we go with default (we assume that retDesc is already 
	//	initialized to default values).
	//
	//	For backwards compatibility, however, we place all weapons 20 pixels
	//	forward.

	ItemCategories iCategory = Item.GetType()->GetCategory();
	if (iCategory == itemcatWeapon || iCategory == itemcatLauncher)
		retDesc->iPosRadius = 20;

	//	Done

	return true;
	}

CGroupOfDeviceGenerators::SSlotDesc *CGroupOfDeviceGenerators::FindSlotDesc (const CItem &Item)

//	FindSlotDesc
//
//	Returns the first slot descriptor that matches the item. If none of the
//	descriptors match, we return NULL.

	{
	int i;

	for (i = 0; i < m_SlotDesc.GetCount(); i++)
		if (Item.MatchesCriteria(m_SlotDesc[i].Criteria))
			return &m_SlotDesc[i];

	return NULL;
	}

ALERROR CGroupOfDeviceGenerators::LoadFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	LoadFromXML
//
//	Load from XML

	{
	int i;
	ALERROR error;

	m_Count.LoadFromXML(pDesc->GetAttribute(COUNT_ATTRIB));
	if (m_Count.IsEmpty())
		m_Count.SetConstant(1);

	//	Load either a <DeviceSlot> element or another device generator.

	for (i = 0; i < pDesc->GetContentElementCount(); i++)
		{
		CXMLElement *pEntry = pDesc->GetContentElement(i);

		if (strEquals(pEntry->GetTag(), DEVICE_SLOT_TAG))
			{
			SSlotDesc *pSlotDesc = m_SlotDesc.Insert();

			CItem::ParseCriteria(pEntry->GetAttribute(CRITERIA_ATTRIB), &pSlotDesc->Criteria);

			if (error = IDeviceGenerator::InitDeviceDescFromXML(Ctx, pEntry, &pSlotDesc->DefaultDesc))
				return error;

			pSlotDesc->iMaxCount = pEntry->GetAttributeIntegerBounded(MAX_COUNT_ATTRIB, 0, -1, -1);
			}
		else
			{
			SEntry *pTableEntry = m_Table.Insert();

			pTableEntry->iChance = pEntry->GetAttributeIntegerBounded(CHANCE_ATTRIB, 0, -1, 100);
			if (error = IDeviceGenerator::CreateFromXML(Ctx, pEntry, &pTableEntry->pDevice))
				{
				pTableEntry->pDevice = NULL;
				return error;
				}
			}
		}

	return NOERROR;
	}

ALERROR CGroupOfDeviceGenerators::OnDesignLoadComplete (SDesignLoadCtx &Ctx)

//	OnDesignLoadComplete
//
//	Resolve references

	{
	int i;
	ALERROR error;

	for (i = 0; i < m_Table.GetCount(); i++)
		if (error = m_Table[i].pDevice->OnDesignLoadComplete(Ctx))
			return error;

	return NOERROR;
	}

//	CDeviceDescList -----------------------------------------------------------

CDeviceDescList::CDeviceDescList (void)

//	CDeviceDescList constructor

	{
	}

CDeviceDescList::~CDeviceDescList (void)

//	CDeviceDescList destructor

	{
	}

void CDeviceDescList::AddDeviceDesc (const SDeviceDesc &Desc)

//	AddDeviceDesc
//
//	Adds a device desc to the list

	{
	m_List.Insert(Desc);
	}

CDeviceClass *CDeviceDescList::GetNamedDevice (DeviceNames iDev) const

//	GetNamedDevice
//
//	Returns the named device (or NULL if not found)

	{
	int i;
	ItemCategories iCatToFind = CDeviceClass::GetItemCategory(iDev);

	for (i = 0; i < GetCount(); i++)
		{
		CDeviceClass *pDevice = GetDeviceClass(i);

		//	See if this is the category that we want to find

		if (pDevice->GetCategory() == iCatToFind)
			return pDevice;
		}

	return NULL;
	}

void CDeviceDescList::RemoveAll (void)

//	RemoveAll
//
//	Removes all devices

	{
	m_List.DeleteAll();
	}
