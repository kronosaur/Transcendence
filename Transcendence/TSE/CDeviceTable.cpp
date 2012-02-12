//	CDeviceTable.cpp
//
//	IDeviceGenerator objects

#include "PreComp.h"

#define GROUP_TAG								CONSTLIT("Group")
#define TABLE_TAG								CONSTLIT("Table")
#define ITEM_TAG								CONSTLIT("Item")
#define ITEMS_TAG								CONSTLIT("Items")
#define NULL_TAG								CONSTLIT("Null")
#define DEVICE_TAG								CONSTLIT("Device")
#define DEVICES_TAG								CONSTLIT("Devices")
#define LEVEL_TABLE_TAG							CONSTLIT("LevelTable")

#define UNID_ATTRIB								CONSTLIT("unid")
#define ITEM_ATTRIB								CONSTLIT("item")
#define COUNT_ATTRIB							CONSTLIT("count")
#define TABLE_ATTRIB							CONSTLIT("table")
#define CHANCE_ATTRIB							CONSTLIT("chance")
#define CRITERIA_ATTRIB							CONSTLIT("criteria")
#define LEVEL_ATTRIB							CONSTLIT("level")
#define LEVEL_CURVE_ATTRIB						CONSTLIT("levelCurve")
#define DAMAGED_ATTRIB							CONSTLIT("damaged")
#define CATEGORIES_ATTRIB						CONSTLIT("categories")
#define LEVEL_FREQUENCY_ATTRIB					CONSTLIT("levelFrequency")
#define ENHANCED_ATTRIB							CONSTLIT("enhanced")
#define ENHANCEMENT_ATTRIB						CONSTLIT("enhancement")
#define DEVICE_ID_ATTRIB						CONSTLIT("deviceID")
#define OMNIDIRECTIONAL_ATTRIB					CONSTLIT("omnidirectional")
#define POS_ANGLE_ATTRIB						CONSTLIT("posAngle")
#define POS_RADIUS_ATTRIB						CONSTLIT("posRadius")
#define POS_Z_ATTRIB							CONSTLIT("posZ")
#define MIN_FIRE_ARC_ATTRIB						CONSTLIT("minFireArc")
#define MAX_FIRE_ARC_ATTRIB						CONSTLIT("maxFireArc")
#define SECONDARY_WEAPON_ATTRIB					CONSTLIT("secondaryWeapon")

#define STR_G_ITEM								CONSTLIT("gItem")

class CNullDevice : public IDeviceGenerator
	{
	};

class CSingleDevice : public IDeviceGenerator
	{
	public:
		CSingleDevice (void) : m_pExtraItems(NULL) { }
		~CSingleDevice (void);

		virtual void AddDevices (int iLevel, CDeviceDescList &Result);
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

		bool m_bOmnidirectional;
		int m_iMinFireArc;
		int m_iMaxFireArc;
		bool m_bSecondary;

		IItemGenerator *m_pExtraItems;
	};

class CLevelTableOfDeviceGenerators : public IDeviceGenerator
	{
	public:
		virtual ~CLevelTableOfDeviceGenerators (void);
		virtual void AddDevices (int iLevel, CDeviceDescList &Result);
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
		virtual void AddDevices (int iLevel, CDeviceDescList &Result);
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
		virtual void AddDevices (int iLevel, CDeviceDescList &Result);
		virtual IDeviceGenerator *GetGenerator (int iIndex) { return m_Table[iIndex].pDevice; }
		virtual int GetGeneratorCount (void) { return m_iTableCount; }
		virtual ALERROR LoadFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);
		virtual ALERROR OnDesignLoadComplete (SDesignLoadCtx &Ctx);

	private:
		struct SEntry
			{
			IDeviceGenerator *pDevice;
			int iChance;
			};

		DiceRange m_Count;

		int m_iTableCount;
		SEntry *m_Table;
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

//	CSingleDevice -------------------------------------------------------------

CSingleDevice::~CSingleDevice (void)

//	CSingleDevice destructor

	{
	if (m_pExtraItems)
		delete m_pExtraItems;
	}

void CSingleDevice::AddDevices (int iLevel, CDeviceDescList &Result)

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

		//	If this is a weapon and we have a default position,
		//	then place the weapon at the front of the ship.

		if (m_bDefaultPos 
				&& (m_pItemType->GetCategory() == itemcatWeapon
					|| m_pItemType->GetCategory() == itemcatLauncher))
			{
			Desc.iPosAngle = 0;
			Desc.iPosRadius = 20;
			Desc.iPosZ = 0;
			Desc.b3DPosition = false;
			}

		//	Otherwise, just set values

		else
			{
			Desc.iPosAngle = m_iPosAngle;
			Desc.iPosRadius = m_iPosRadius;
			Desc.iPosZ = m_iPosZ;
			Desc.b3DPosition = m_b3DPosition;
			}

		Desc.bOmnidirectional = m_bOmnidirectional;
		Desc.iMinFireArc = m_iMinFireArc;
		Desc.iMaxFireArc = m_iMaxFireArc;
		Desc.bSecondary = m_bSecondary;

		//	Add extra items

		if (m_pExtraItems)
			{
			CItemListManipulator ItemList(Desc.ExtraItems);
			SItemAddCtx Ctx(ItemList);
			Ctx.iLevel = iLevel;

			m_pExtraItems->AddItems(Ctx);
			}

		//	Done
		
		Result.AddDeviceDesc(Desc);
		}
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

	m_pItemType.LoadUNID(Ctx, sUNID);
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

	//	Load device desc attributes

	if (pDesc->FindAttributeInteger(POS_Z_ATTRIB, &m_iPosZ))
		{
		m_iPosAngle = pDesc->GetAttributeInteger(POS_ANGLE_ATTRIB);
		m_iPosRadius = pDesc->GetAttributeInteger(POS_RADIUS_ATTRIB);
		m_b3DPosition = true;
		m_bDefaultPos = false;
		}
	else if (pDesc->FindAttributeInteger(POS_ANGLE_ATTRIB, &m_iPosAngle))
		{
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

	m_bOmnidirectional = pDesc->GetAttributeBool(OMNIDIRECTIONAL_ATTRIB);
	m_iMinFireArc = pDesc->GetAttributeInteger(MIN_FIRE_ARC_ATTRIB);
	m_iMaxFireArc = pDesc->GetAttributeInteger(MAX_FIRE_ARC_ATTRIB);
	m_bSecondary = pDesc->GetAttributeBool(SECONDARY_WEAPON_ATTRIB);

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

void CTableOfDeviceGenerators::AddDevices (int iLevel, CDeviceDescList &Result)

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
				m_Table[i].pDevice->AddDevices(iLevel, Result);
				break;
				}
			}
		}
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
				return error;
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

void CLevelTableOfDeviceGenerators::AddDevices (int iLevel, CDeviceDescList &Result)

//	AddDevices
//
//	Adds devices

	{
	int i, j;

	//	Compute probabilities

	if (iLevel != m_iComputedLevel)
		{
		m_iTotalChance = 0;
		for (i = 0; i < m_Table.GetCount(); i++)
			{
			m_Table[i].iChance = GetFrequencyByLevel(m_Table[i].sLevelFrequency, iLevel);
			m_iTotalChance += m_Table[i].iChance;
			}

		m_iComputedLevel = iLevel;
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
					m_Table[j].pDevice->AddDevices(iLevel, Result);
					break;
					}
				}
			}
		}
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
			return error;
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

	if (m_Table)
		{
		for (i = 0; i < m_iTableCount; i++)
			if (m_Table[i].pDevice)
				delete m_Table[i].pDevice;

		delete [] m_Table;
		}
	}

void CGroupOfDeviceGenerators::AddDevices (int iLevel, CDeviceDescList &Result)

//	AddDevices
//
//	Add devices

	{
	int i, j;

	int iCount = m_Count.Roll();
	for (j = 0; j < iCount; j++)
		{
		for (i = 0; i < m_iTableCount; i++)
			{
			if (mathRandom(1, 100) <= m_Table[i].iChance)
				m_Table[i].pDevice->AddDevices(iLevel, Result);
			}
		}
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

	m_iTableCount = pDesc->GetContentElementCount();
	if (m_iTableCount > 0)
		{
		m_Table = new SEntry [m_iTableCount];
		utlMemSet(m_Table, sizeof(SEntry) * m_iTableCount, 0);

		for (i = 0; i < m_iTableCount; i++)
			{
			CXMLElement *pEntry = pDesc->GetContentElement(i);
			
			m_Table[i].iChance = pEntry->GetAttributeInteger(CHANCE_ATTRIB);
			if (m_Table[i].iChance == 0)
				m_Table[i].iChance = 100;

			if (error = IDeviceGenerator::CreateFromXML(Ctx, pEntry, &m_Table[i].pDevice))
				return error;
			}
		}
	else
		m_Table = NULL;

	return NOERROR;
	}

ALERROR CGroupOfDeviceGenerators::OnDesignLoadComplete (SDesignLoadCtx &Ctx)

//	OnDesignLoadComplete
//
//	Resolve references

	{
	int i;
	ALERROR error;

	for (i = 0; i < m_iTableCount; i++)
		{
		if (error = m_Table[i].pDevice->OnDesignLoadComplete(Ctx))
			return error;
		}

	return NOERROR;
	}

//	CDeviceDescList -----------------------------------------------------------

const int ALLOC_SIZE =							16;

CDeviceDescList::CDeviceDescList (void) :
		m_pDesc(NULL),
		m_iCount(0),
		m_iAlloc(0)

//	CDeviceDescList constructor

	{
	}

CDeviceDescList::~CDeviceDescList (void)

//	CDeviceDescList destructor

	{
	if (m_pDesc)
		delete [] m_pDesc;
	}

void CDeviceDescList::AddDeviceDesc (const SDeviceDesc &Desc)

//	AddDeviceDesc
//
//	Adds a device desc to the list

	{
	//	Allocate, if necessary

	if (m_iCount == m_iAlloc)
		{
		int iNewAlloc = m_iAlloc + ALLOC_SIZE;
		SDeviceDesc *pNewDesc = new SDeviceDesc [iNewAlloc];

		for (int i = 0; i < m_iCount; i++)
			pNewDesc[i] = m_pDesc[i];

		if (m_pDesc)
			delete [] m_pDesc;

		m_pDesc = pNewDesc;
		m_iAlloc = iNewAlloc;
		}

	//	Add to the end

	m_pDesc[m_iCount++] = Desc;
	}

CDeviceClass *CDeviceDescList::GetNamedDevice (DeviceNames iDev) const

//	GetNamedDevice
//
//	Returns the named device (or NULL if not found)

	{
	int i;

	for (i = 0; i < m_iCount; i++)
		{
		CDeviceClass *pDevice = GetDeviceClass(i);

		if (iDev == devPrimaryWeapon && pDevice->GetCategory() == itemcatWeapon)
			return pDevice;
		else if (iDev == devMissileWeapon && pDevice->GetCategory() == itemcatLauncher)
			return pDevice;
		else if (iDev == devShields && pDevice->GetCategory() == itemcatShields)
			return pDevice;
		else if (iDev == devDrive && pDevice->GetCategory() == itemcatDrive)
			return pDevice;
		else if (iDev == devCargo && pDevice->GetCategory() == itemcatCargoHold)
			return pDevice;
		else if (iDev == devReactor && pDevice->GetCategory() == itemcatReactor)
			return pDevice;
		}

	return NULL;
	}

void CDeviceDescList::RemoveAll (void)

//	RemoveAll
//
//	Removes all devices

	{
	if (m_pDesc)
		{
		delete [] m_pDesc;
		m_pDesc = NULL;
		}

	m_iAlloc = 0;
	m_iCount = 0;
	}
