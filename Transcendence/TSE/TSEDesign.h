//	TSEDesign.h
//
//	Transcendence design classes

#ifndef INCL_TSE_DESIGN
#define INCL_TSE_DESIGN

class CCommunicationsHandler;
class CCreatePainterCtx;
class CDockScreen;
class CDynamicDesignTable;
class CEffect;
class CGameStats;
class CItemCtx;
class CItemEnhancementStack;
class CObjectImageArray;
class COrbit;
class COrderList;
class CMultiverseCollection;
class CMultiverseCatalogEntry;
class CShipClass;
class CSystem;
class CSystemMap;
class CTopology;
class CTopologyDescTable;
class CTradingDesc;
class IDeviceGenerator;
struct SDestroyCtx;
struct SSystemCreateCtx;

//	Constants & Enums

const int MAX_OBJECT_LEVEL =			25;	//	Max level for space objects
const int MAX_ITEM_LEVEL =				25;	//	Max level for items

enum ItemCategories
	{
	itemcatNone =			0xffffffff,

	itemcatMisc =			0x00000001,		//	General item with no object UNID
	itemcatArmor =			0x00000002,		//	Armor items
	itemcatWeapon =			0x00000004,		//	Primary weapons
	itemcatMiscDevice =		0x00000008,		//	Misc device
	itemcatLauncher =		0x00000010,		//	Launcher
	itemcatNano =			0x00000020,
	itemcatReactor =		0x00000040,		//	Reactor device
	itemcatShields =		0x00000080,		//	Shield device
	itemcatCargoHold =		0x00000100,		//	Cargo expansion device
	itemcatFuel =			0x00000200,		//	Fuel items
	itemcatMissile =		0x00000400,		//	Missiles
	itemcatDrive =			0x00000800,		//	Drive device
	itemcatUseful =			0x00001000,		//	Usable item

	itemcatWeaponMask =		0x00000014,		//	Either primary or launcher
	itemcatDeviceMask =		0x000009DC,		//	Any device

	itemcatCount =			13,				//	Total count
	};

enum ETradeServiceTypes
	{
	serviceNone =						0,

	serviceBuy =						1,
	serviceSell =						2,
	serviceAcceptDonations =			3,
	serviceRefuel =						4,
	serviceRepairArmor =				5,
	serviceReplaceArmor =				6,
	serviceInstallDevice =				7,
	serviceRemoveDevice =				8,
	serviceUpgradeDevice =				9,
	serviceEnhanceItem =				10,
	serviceRepairItem =					11,
	serviceCustom =						12,

	serviceCount =						13,
	};

//	CFormulaText

class CFormulaText
	{
	public:
		int EvalAsInteger (CSpaceObject *pSource, CString *retsPrefix = NULL, CString *retsSuffix = NULL, CString *retsError = NULL) const;
		inline void InitFromInteger (int iValue) { m_sText = strFromInt(iValue); }
		ALERROR InitFromString (SDesignLoadCtx &Ctx, const CString &sText);
		inline bool IsEmpty (void) const { return m_sText.IsBlank(); }
		void ReadFromStream (SLoadCtx &Ctx);
		inline void SetInteger (int iValue) { m_sText = strFromInt(iValue); }
		void WriteToStream (IWriteStream *pStream) const;

	private:
		CString m_sText;
	};

//	Item Criteria --------------------------------------------------------------

struct CItemCriteria
	{
	CItemCriteria (void);
	CItemCriteria (const CItemCriteria &Copy);
	~CItemCriteria (void);

	CItemCriteria &operator= (const CItemCriteria &Copy);

	int GetMaxLevelMatched (void) const;

	DWORD dwItemCategories;			//	Set of ItemCategories to match on
	DWORD dwExcludeCategories;		//	Categories to exclude
	DWORD dwMustHaveCategories;		//	ANDed categories

	WORD wFlagsMustBeSet;			//	These flags must be set
	WORD wFlagsMustBeCleared;		//	These flags must be cleared

	bool bUsableItemsOnly;			//	Item must be usable
	bool bExcludeVirtual;			//	Exclude virtual items
	bool bInstalledOnly;			//	Item must be installed
	bool bNotInstalledOnly;			//	Item must not be installed
	bool bLauncherMissileOnly;		//	Item must be a missile for a launcher

	TArray<CString> ModifiersRequired;		//	Required modifiers
	TArray<CString> ModifiersNotAllowed;	//	Exclude these modifiers
	TArray<CString> SpecialAttribRequired;	//	Special required attributes
	TArray<CString> SpecialAttribNotAllowed;//	Exclude these special attributes
	CString Frequency;				//	If not blank, only items with these frequencies

	int iEqualToLevel;				//	If not -1, only items of this level
	int iGreaterThanLevel;			//	If not -1, only items greater than this level
	int iLessThanLevel;				//	If not -1, only items less than this level
	int iEqualToPrice;				//	If not -1, only items at this price
	int iGreaterThanPrice;			//	If not -1, only items greater than this price
	int iLessThanPrice;				//	If not -1, only items less than this price
	int iEqualToMass;				//	If not -1, only items of this mass (in kg)
	int iGreaterThanMass;			//	If not -1, only items greater than this mass (in kg)
	int iLessThanMass;				//	If not -1, only items less than this mass (in kg)

	ICCItem *pFilter;				//	Filter returns Nil for excluded items
	};

enum EDisplayAttributeTypes
	{
	attribNeutral,
	attribPositive,
	attribNegative,
	};

enum EAttributeTypes
	{
	attribTypeLocation,
	attribTypeItem,
	};

struct SDisplayAttribute
	{
	SDisplayAttribute (EDisplayAttributeTypes iTypeCons, const CString &sTextCons) :
			iType(iTypeCons),
			sText(sTextCons)
		{ }

	EDisplayAttributeTypes iType;
	CString sText;

	RECT rcRect;					//	Reserved for callers
	};

class CDisplayAttributeDefinitions
	{
	public:
		void AccumulateAttributes (const CItem &Item, TArray<SDisplayAttribute> *retList) const;
		void Append (const CDisplayAttributeDefinitions &Attribs);
		inline void DeleteAll (void) { m_Attribs.DeleteAll(); m_ItemAttribs.DeleteAll(); }
		int GetLocationAttribFrequency (const CString &sAttrib) const;
		ALERROR InitFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);
		inline bool IsEmpty (void) const { return ((m_Attribs.GetCount() == 0) && (m_ItemAttribs.GetCount() == 0)); }

	private:
		struct SItemEntry
			{
			CItemCriteria Criteria;
			EDisplayAttributeTypes iType;
			CString sText;
			};

		struct SAttribDesc
			{
			EAttributeTypes iType;
			CString sName;			//	Human readable name

			//	Location attributes

			int iFrequency;			//	% of locations with this attribute. (1-99)
			};

		TSortMap<CString, SAttribDesc> m_Attribs;
		TArray<SItemEntry> m_ItemAttribs;
	};

//	Base Design Type ----------------------------------------------------------
//
//	To add a new DesignType:
//
//	[ ]	Add it to DesignTypes enum
//	[ ] Increment designCount
//	[ ] Add a char??? entry
//	[ ] Add entry to DESIGN_CHAR in CDesignType (make sure it matches the char??? entry)
//	[ ] Add entry to DESIGN_CLASS_NAME in CDesignType
//	[ ] Add case to CDesignTypeCriteria::ParseCriteria (if type should be enumerable)
//	[ ] Add constructor call to CDesignType::CreateFromXML

enum DesignTypes
	{
	designItemType =					0,
	designItemTable =					1,
	designShipClass =					2,
	designEnergyFieldType =				3,
	designSystemType =					4,
	designStationType =					5,
	designSovereign =					6,
	designDockScreen =					7,
	designEffectType =					8,
	designPower =						9,

	designSpaceEnvironmentType =		10,
	designShipTable =					11,
	designAdventureDesc =				12,
	designGlobals =						13,
	designImage =						14,
	designSound =						15,
	designMissionType =					16,
	designSystemTable =					17,
	designSystemMap =					18,
	designNameGenerator =				19,

	designEconomyType =					20,
	designTemplateType =				21,
	designGenericType =					22,

	designCount	=						23, 

	designSetAll =						0xffffffff,
	charEconomyType =					'$',
	charAdventureDesc =					'a',
	charItemTable =						'b',
	charEffectType =					'c',
	charDockScreen =					'd',
	charSpaceEnvironmentType =			'e',
	charEnergyFieldType =				'f',
	charGlobals =						'g',
	charShipTable =						'h',
	charItemType =						'i',
	//	j
	//	k
	//	l
	charImage =							'm',
	charMissionType =					'n',
	//	o
	charPower =							'p',
	charSystemTable =					'q',
	//	r
	charShipClass =						's',
	charStationType =					't',
	charSound =							'u',
	charSovereign =						'v',
	charNameGenerator =					'w',
	charGenericType =					'x',

	charSystemType =					'y',
	charSystemMap =						'z',
	charTemplateType =					'_',
	};

class CDesignTypeCriteria
	{
	public:
		inline bool ChecksLevel (void) const { return (m_iGreaterThanLevel != INVALID_COMPARE || m_iLessThanLevel != INVALID_COMPARE); }
		inline const CString &GetExcludedAttrib (int iIndex) const { return m_sExclude[iIndex]; }
		inline int GetExcludedAttribCount (void) const { return m_sExclude.GetCount(); }
		inline const CString &GetExcludedSpecialAttrib (int iIndex) const { return m_sExcludeSpecial[iIndex]; }
		inline int GetExcludedSpecialAttribCount (void) const { return m_sExcludeSpecial.GetCount(); }
		inline const CString &GetRequiredAttrib (int iIndex) const { return m_sRequire[iIndex]; }
		inline int GetRequiredAttribCount (void) const { return m_sRequire.GetCount(); }
		inline const CString &GetRequiredSpecialAttrib (int iIndex) const { return m_sRequireSpecial[iIndex]; }
		inline int GetRequiredSpecialAttribCount (void) const { return m_sRequireSpecial.GetCount(); }
		inline bool IncludesVirtual (void) const { return m_bIncludeVirtual; }
		inline bool MatchesDesignType (DesignTypes iType) const
			{ return ((m_dwTypeSet & (1 << iType)) ? true : false); }
		bool MatchesLevel (int iMinLevel, int iMaxLevel) const;
		static ALERROR ParseCriteria (const CString &sCriteria, CDesignTypeCriteria *retCriteria);

	private:
		enum Flags
			{
			INVALID_COMPARE = -1000,
			};

		DWORD m_dwTypeSet;
		TArray<CString> m_sRequire;
		TArray<CString> m_sExclude;
		TArray<CString> m_sRequireSpecial;
		TArray<CString> m_sExcludeSpecial;

		int m_iGreaterThanLevel;
		int m_iLessThanLevel;

		bool m_bIncludeVirtual;
	};

//	CDesignType

class CDesignType
	{
	public:
		enum ECachedHandlers
			{
			evtCanInstallItem			= 0,
			evtCanRemoveItem			= 1,
			evtOnGlobalTypesInit		= 2,
			evtOnObjDestroyed			= 3,
			evtOnSystemObjAttacked		= 4,
			evtOnSystemWeaponFire		= 5,

			evtCount					= 6,
			};

		CDesignType (void);
		virtual ~CDesignType (void);
		void CreateClone (CDesignType **retpType);
		static ALERROR CreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, CDesignType **retpType);

		ALERROR BindDesign (SDesignLoadCtx &Ctx);
		ALERROR ComposeLoadError (SDesignLoadCtx &Ctx, const CString &sError);
		inline ALERROR FinishBindDesign (SDesignLoadCtx &Ctx) { return OnFinishBindDesign(Ctx); }
		ALERROR InitFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, bool bIsOverride = false);
		bool MatchesCriteria (const CDesignTypeCriteria &Criteria);
		void MergeType (CDesignType *pSource);
		ALERROR PrepareBindDesign (SDesignLoadCtx &Ctx);
		inline void PrepareReinit (void) { OnPrepareReinit(); }
		void ReadFromStream (SUniverseLoadCtx &Ctx);
		void Reinit (void);
		inline void UnbindDesign (void) { OnUnbindDesign(); }
		void WriteToStream (IWriteStream *pStream);

		inline void AddExternals (TArray<CString> *retExternals) { OnAddExternals(retExternals); }
		void AddTypesUsed (TSortMap<DWORD, bool> *retTypesUsed);
		static CDesignType *AsType (CDesignType *pType) { return pType; }
		inline CEffectCreator *FindEffectCreatorInType (const CString &sUNID) { return OnFindEffectCreator(sUNID); }
		bool FindEventHandler (const CString &sEvent, SEventHandlerDesc *retEvent = NULL) const;
		inline bool FindEventHandler (ECachedHandlers iEvent, SEventHandlerDesc *retEvent = NULL) const { if (retEvent) *retEvent = m_EventsCache[iEvent]; return (m_EventsCache[iEvent].pCode != NULL); }
		bool FindStaticData (const CString &sAttrib, const CString **retpData) const;
		void FireCustomEvent (const CString &sEvent, ECodeChainEvents iEvent = eventNone, ICCItem *pData = NULL, ICCItem **retpResult = NULL);
		bool FireGetCreatePos (CSpaceObject *pBase, CSpaceObject *pTarget, CSpaceObject **retpGate, CVector *retvPos);
		void FireGetGlobalAchievements (CGameStats &Stats);
		bool FireGetGlobalDockScreen (const SEventHandlerDesc &Event, CSpaceObject *pObj, CString *retsScreen, ICCItem **retpData, int *retiPriority);
		bool FireGetGlobalPlayerPriceAdj (const SEventHandlerDesc &Event, ETradeServiceTypes iService, CSpaceObject *pProvider, const CItem &Item, ICCItem *pData, int *retiPriceAdj);
		int FireGetGlobalResurrectPotential (void);
		void FireObjCustomEvent (const CString &sEvent, CSpaceObject *pObj, ICCItem **retpResult);
		ALERROR FireOnGlobalDockPaneInit (const SEventHandlerDesc &Event, void *pScreen, DWORD dwScreenUNID, const CString &sScreen, const CString &sPane, CString *retsError);
		void FireOnGlobalMarkImages (const SEventHandlerDesc &Event);
		void FireOnGlobalObjDestroyed (const SEventHandlerDesc &Event, SDestroyCtx &Ctx);
		ALERROR FireOnGlobalPlayerChangedShips (CSpaceObject *pOldShip, CString *retsError = NULL);
		ALERROR FireOnGlobalPlayerEnteredSystem (CString *retsError = NULL);
		ALERROR FireOnGlobalPlayerLeftSystem (CString *retsError = NULL);
		ALERROR FireOnGlobalResurrect (CString *retsError = NULL);
		ALERROR FireOnGlobalSystemCreated (SSystemCreateCtx &SysCreateCtx, CString *retsError = NULL);
		void FireOnGlobalSystemStarted (const SEventHandlerDesc &Event);
		void FireOnGlobalSystemStopped (const SEventHandlerDesc &Event);
		ALERROR FireOnGlobalTopologyCreated (CString *retsError = NULL);
		ALERROR FireOnGlobalTypesInit (SDesignLoadCtx &Ctx);
		ALERROR FireOnGlobalUniverseCreated (const SEventHandlerDesc &Event);
		ALERROR FireOnGlobalUniverseLoad (const SEventHandlerDesc &Event);
		ALERROR FireOnGlobalUniverseSave (const SEventHandlerDesc &Event);
		void FireOnGlobalUpdate (const SEventHandlerDesc &Event);
		void FireOnRandomEncounter (CSpaceObject *pObj = NULL);
		inline DWORD GetAPIVersion (void) const { return m_dwVersion; }
		inline const CString &GetAttributes (void) { return m_sAttributes; }
		inline CString GetDataField (const CString &sField) { CString sValue; FindDataField(sField, &sValue); return sValue; }
		inline int GetDataFieldInteger (const CString &sField) { CString sValue; if (FindDataField(sField, &sValue)) return strToInt(sValue, 0, NULL); else return 0; }
		inline const CDisplayAttributeDefinitions &GetDisplayAttributes (void) const { return m_DisplayAttribs; }
		ICCItem *GetEventHandler (const CString &sEvent) const;
		void GetEventHandlers (const CEventHandler **retHandlers, TSortMap<CString, SEventHandlerDesc> *retInheritedHandlers);
		CExtension *GetExtension (void) const { return m_pExtension; }
		inline const CString &GetGlobalData (const CString &sAttrib) { return m_GlobalData.GetData(sAttrib); }
		inline CDesignType *GetInheritFrom (void) const { return m_pInheritFrom; }
		inline CXMLElement *GetLocalScreens (void) const { return m_pLocalScreens; }
		ICCItem *GetProperty (CCodeChainCtx &Ctx, const CString &sProperty);
		CXMLElement *GetScreen (const CString &sUNID);
		const CString &GetStaticData (const CString &sAttrib) const;
		CString GetTypeClassName (void) const;
		inline DWORD GetUNID (void) const { return m_dwUNID; }
		inline CXMLElement *GetXMLElement (void) const { return m_pXML; }
		bool HasAttribute (const CString &sAttrib) const;
		inline bool HasEvents (void) const { return !m_Events.IsEmpty() || (m_pInheritFrom && m_pInheritFrom->HasEvents()); }
		inline bool HasLiteralAttribute (const CString &sAttrib) const { return ::HasModifier(m_sAttributes, sAttrib); }
		bool HasSpecialAttribute (const CString &sAttrib) const;
		void InitCachedEvents (int iCount, char **pszEvents, SEventHandlerDesc *retEvents);
		inline bool IsClone (void) const { return m_bIsClone; }
		inline bool IsModification (void) const { return m_bIsModification; }
		inline void MarkImages (void) { OnMarkImages(); }
		inline void SetGlobalData (const CString &sAttrib, const CString &sData) { m_GlobalData.SetData(sAttrib, sData); }
		inline void SetUNID (DWORD dwUNID) { m_dwUNID = dwUNID; }
		inline void SetXMLElement (CXMLElement *pDesc) { m_pXML = pDesc; }
		inline void TopologyInitialized (void) { OnTopologyInitialized(); }
		bool Translate (CSpaceObject *pObj, const CString &sID, ICCItem *pData, ICCItem **retpResult);
		bool TranslateText (CSpaceObject *pObj, const CString &sID, ICCItem *pData, CString *retsText);

		static CString GetTypeChar (DesignTypes iType);

		//	CDesignType overrides
		virtual bool FindDataField (const CString &sField, CString *retsValue);
		virtual CCommunicationsHandler *GetCommsHandler (void) { return NULL; }
		virtual CTradingDesc *GetTradingDesc (void) { return NULL; }
		virtual CString GetTypeName (DWORD *retdwFlags = NULL) { if (retdwFlags) *retdwFlags = 0; return GetDataField(CONSTLIT("name")); }
		virtual int GetLevel (int *retiMinLevel = NULL, int *retiMaxLevel = NULL) const { if (retiMinLevel) *retiMinLevel = -1; if (retiMaxLevel) *retiMaxLevel = -1; return -1; }
		virtual DesignTypes GetType (void) const = 0;
		virtual bool IsVirtual (void) const { return false; }

	protected:
		ALERROR AddEventHandler (const CString &sEvent, const CString &sCode, CString *retsError = NULL) { return m_Events.AddEvent(sEvent, sCode, retsError); }
		bool IsValidLoadXML (const CString &sTag);
		void ReadGlobalData (SUniverseLoadCtx &Ctx);
		void ReportEventError (const CString &sEvent, ICCItem *pError);

		//	CDesignType overrides
		virtual void OnAddExternals (TArray<CString> *retExternals) { }
		virtual void OnAddTypesUsed (TSortMap<DWORD, bool> *retTypesUsed) { }
		virtual ALERROR OnBindDesign (SDesignLoadCtx &Ctx) { return NOERROR; }
		virtual ALERROR OnCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc) { return NOERROR; }
		virtual CEffectCreator *OnFindEffectCreator (const CString &sUNID) { return NULL; }
		virtual bool OnFindEventHandler (const CString &sEvent, SEventHandlerDesc *retEvent = NULL) const { return false; }
		virtual ALERROR OnFinishBindDesign (SDesignLoadCtx &Ctx) { return NOERROR; }
		virtual ICCItem *OnGetProperty (CCodeChainCtx &Ctx, const CString &sProperty) { return NULL; }
		virtual bool OnHasSpecialAttribute (const CString &sAttrib) const { return sAttrib.IsBlank(); }
		virtual void OnInitFromClone (CDesignType *pSource) { ASSERT(false); }
		virtual void OnMarkImages (void) { }
		virtual void OnMergeType (CDesignType *pSource) { ASSERT(false); }
		virtual ALERROR OnPrepareBindDesign (SDesignLoadCtx &Ctx) { return NOERROR; }
		virtual void OnPrepareReinit (void) { }
		virtual void OnReadFromStream (SUniverseLoadCtx &Ctx) { }
		virtual void OnReinit (void) { }
		virtual void OnTopologyInitialized (void) { }
		virtual void OnUnbindDesign (void) { }
		virtual void OnWriteToStream (IWriteStream *pStream) { }

	private:
		void AddUniqueHandlers (TSortMap<CString, SEventHandlerDesc> *retInheritedHandlers);
		inline SEventHandlerDesc *GetInheritedCachedEvent (ECachedHandlers iEvent) { return (m_EventsCache[iEvent].pCode ? &m_EventsCache[iEvent] : (m_pInheritFrom ? m_pInheritFrom->GetInheritedCachedEvent(iEvent) : NULL)); }
		void InitCachedEvents (void);
		bool InSelfReference (CDesignType *pType);
		void MergeLanguageTo (CLanguageDataBlock &Dest);
		bool TranslateVersion2 (CSpaceObject *pObj, const CString &sID, ICCItem **retpResult);

		DWORD m_dwUNID;
		CExtension *m_pExtension;				//	Extension
		DWORD m_dwVersion;						//	Extension version
		CXMLElement *m_pXML;					//	Optional XML for this type

		DWORD m_dwInheritFrom;					//	Inherit from this type
		CDesignType *m_pInheritFrom;			//	Inherit from this type

		CString m_sAttributes;					//	Type attributes
		CAttributeDataBlock m_StaticData;		//	Static data
		CAttributeDataBlock m_GlobalData;		//	Global (variable) data
		CAttributeDataBlock m_InitGlobalData;	//	Initial global data
		CLanguageDataBlock m_Language;			//	Language data
		CEventHandler m_Events;					//	Event handlers
		CXMLElement *m_pLocalScreens;			//	Local dock screen
		CDisplayAttributeDefinitions m_DisplayAttribs;	//	Display attribute definitions

		bool m_bIsModification;					//	TRUE if this modifies the type it overrides
		bool m_bIsClone;						//	TRUE if we cloned this from another type

		SEventHandlerDesc m_EventsCache[evtCount];	//	Cached events
	};

template <class CLASS> class CDesignTypeRef
	{
	public:
		CDesignTypeRef (void) : m_pType(NULL), m_dwUNID(0) { }

		inline operator CLASS *() const { return m_pType; }
		inline CLASS * operator->() const { return m_pType; }

		ALERROR Bind (SDesignLoadCtx &Ctx)
			{
			if (m_dwUNID)
				{
				CDesignType *pBaseType = g_pUniverse->FindDesignType(m_dwUNID);
				if (pBaseType == NULL)
					{
					Ctx.sError = strPatternSubst(CONSTLIT("Unknown design type: %x"), m_dwUNID);
					return ERR_FAIL;
					}

				m_pType = CLASS::AsType(pBaseType);
				if (m_pType == NULL)
					{
					Ctx.sError = strPatternSubst(CONSTLIT("Specified type is invalid: %x"), m_dwUNID);
					return ERR_FAIL;
					}
				}

			return NOERROR;
			}

		inline DWORD GetUNID (void) const { return m_dwUNID; }
		ALERROR LoadUNID (SDesignLoadCtx &Ctx, const CString &sUNID) { return ::LoadUNID(Ctx, sUNID, &m_dwUNID); }

		void Set (CLASS *pType)
			{
			m_pType = pType;
			if (pType)
				m_dwUNID = pType->GetUNID();
			else
				m_dwUNID = 0;
			}

		void SetUNID (DWORD dwUNID)
			{
			if (dwUNID != m_dwUNID)
				{
				m_dwUNID = dwUNID;
				m_pType = NULL;
				}
			}

	protected:
		CLASS *m_pType;
		DWORD m_dwUNID;
	};

//	Generic Type ---------------------------------------------------------------

class CGenericType : public CDesignType
	{
	public:
		//	CDesignType overrides

		static CGenericType *AsType (CDesignType *pType) { return ((pType && pType->GetType() == designGenericType) ? (CGenericType *)pType : NULL); }
		virtual DesignTypes GetType (void) const { return designGenericType; }
	};

//	Design Type References ----------------------------------------------------

class CItemTypeRef : public CDesignTypeRef<CItemType>
	{
	public:
		CItemTypeRef (void) { }
		CItemTypeRef (CItemType *pType) { Set(pType); }
		inline ALERROR Bind (SDesignLoadCtx &Ctx) { return CDesignTypeRef<CItemType>::Bind(Ctx); }
		ALERROR Bind (SDesignLoadCtx &Ctx, ItemCategories iCategory);
	};

class CArmorClassRef : public CDesignTypeRef<CArmorClass>
	{
	public:
		ALERROR Bind (SDesignLoadCtx &Ctx);
	};

class CDeviceClassRef : public CDesignTypeRef<CDeviceClass>
	{
	public:
		ALERROR Bind (SDesignLoadCtx &Ctx);
		void Set (CDeviceClass *pDevice);
	};

class CWeaponFireDescRef : public CDesignTypeRef<CWeaponFireDesc>
	{
	public:
		ALERROR Bind (SDesignLoadCtx &Ctx);
	};

class CDockScreenTypeRef
	{
	public:
		CDockScreenTypeRef (void) : m_pType(NULL), m_pLocal(NULL) { }

		inline operator CDockScreenType *() const { return m_pType; }
		inline CDockScreenType * operator->() const { return m_pType; }

		ALERROR Bind (SDesignLoadCtx &Ctx, CXMLElement *pLocalScreens = NULL);
		CXMLElement *GetDesc (void) const;
		CDesignType *GetDockScreen (CDesignType *pRoot, CString *retsName) const;
		CString GetStringUNID (CDesignType *pRoot) const;
		inline const CString &GetUNID (void) const { return m_sUNID; }
		void LoadUNID (SDesignLoadCtx &Ctx, const CString &sUNID);

		ALERROR Bind (CXMLElement *pLocalScreens = NULL);
		inline void LoadUNID (const CString &sUNID) { m_sUNID = sUNID; }

	private:
		CString m_sUNID;
		CDockScreenType *m_pType;
		CXMLElement *m_pLocal;
	};

class CEconomyTypeRef
	{
	public:
		CEconomyTypeRef (void) : m_pType(NULL) { }

		inline operator CEconomyType *() const { return m_pType; }
		inline CEconomyType * operator->() const { return m_pType; }

		ALERROR Bind (SDesignLoadCtx &Ctx);
		void LoadUNID (const CString &sUNID) { m_sUNID = sUNID; }
		void Set (DWORD dwUNID);
		inline void Set (CEconomyType *pType) { m_pType = pType; }

	private:
		CString m_sUNID;
		CEconomyType *m_pType;
	};

class CEffectCreatorRef : public CDesignTypeRef<CEffectCreator>
	{
	public:
		CEffectCreatorRef (void) : m_pSingleton(NULL), m_bDelete(false) { }
		~CEffectCreatorRef (void);

		CEffectCreatorRef &operator= (const CEffectCreatorRef &Source);

		ALERROR Bind (SDesignLoadCtx &Ctx);
		ALERROR CreateBeamEffect (SDesignLoadCtx &Ctx, CXMLElement *pDesc, const CString &sUNID);
		ALERROR CreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, const CString &sUNID);
		IEffectPainter *CreatePainter (CCreatePainterCtx &Ctx);
		inline bool IsEmpty (void) const { return (m_dwUNID == 0 && m_pType == NULL); }
		ALERROR LoadEffect (SDesignLoadCtx &Ctx, const CString &sUNID, CXMLElement *pDesc, const CString &sAttrib);
		ALERROR LoadSimpleEffect (SDesignLoadCtx &Ctx, const CString &sUNID, CXMLElement *pDesc);
		void Set (CEffectCreator *pEffect);

	private:
		IEffectPainter *m_pSingleton;
		bool m_bDelete;
	};

class CItemTableRef : public CDesignTypeRef<CItemTable>
	{
	};

class CGenericTypeRef : public CDesignTypeRef<CGenericType>
	{
	};

class COverlayTypeRef : public CDesignTypeRef<COverlayType>
	{
	};

class CShipClassRef : public CDesignTypeRef<CShipClass>
	{
	};

class CShipTableRef : public CDesignTypeRef<CShipTable>
	{
	};

class CSovereignRef : public CDesignTypeRef<CSovereign>
	{
	};

class CStationTypeRef : public CDesignTypeRef<CStationType>
	{
	};

class CSystemMapRef : public CDesignTypeRef<CSystemMap>
	{
	};

//	Classes and structs

class CCurrencyAndValue
	{
	public:
		ALERROR Bind (SDesignLoadCtx &Ctx);
		inline CEconomyType *GetCurrencyType (void) const { return m_pCurrency; }
		inline CurrencyValue GetValue (void) const { return m_iValue; }
		ALERROR InitFromXML (SDesignLoadCtx &Ctx, const CString &sDesc);

	private:
		CurrencyValue m_iValue;
		CEconomyTypeRef m_pCurrency;
	};

class CCurrencyAndRange
	{
	public:
		ALERROR Bind (SDesignLoadCtx &Ctx);
		inline CEconomyType *GetCurrencyType (void) const { return m_pCurrency; }
		inline const DiceRange &GetDiceRange (void) const { return m_Value; }
		ALERROR InitFromXML (SDesignLoadCtx &Ctx, const CString &sDesc);
		inline CurrencyValue Roll (void) const { return m_Value.Roll(); }

	private:
		DiceRange m_Value;
		CEconomyTypeRef m_pCurrency;
	};

struct SViewportPaintCtx
	{
	SViewportPaintCtx (void) :
			pCenter(NULL),
			xCenter(0),
			yCenter(0),
			pObj(NULL),
			iPerception(4),				//	LATER: Same as CSpaceObject::perceptNormal (but we haven't included it yet).
			wSpaceColor(0),
			fNoSelection(false),
			fNoRecon(false),
			fNoDockedShips(false),
			fEnhancedDisplay(false),
			fNoStarfield(false),
			fShowManeuverEffects(false),
			bInFront(false),
			bFade(false),
			iTick(0),
			iVariant(0),
			iDestiny(0),
			iRotation(0),
			iMaxLength(-1)
		{ }

	//	Viewport metrics

	CSpaceObject *pCenter;				//	Center object (viewport perspective)
	CVector vCenterPos;					//	Center of viewport
	RECT rcView;						//	Viewport
	int xCenter;						//	Center of viewport (pixels)
	int yCenter;						//	Center of viewport (pixels)
	ViewportTransform XForm;			//	Converts from object to screen viewport coordinates
										//		Screen viewport coordinates has positive-Y down.
	ViewportTransform XFormRel;			//	In the case of effects, this Xform has been translated
										//		to offset for the effect position

	CVector vDiagonal;					//	Length of 1/2 viewport diagonal (in global coordinates).
	CVector vUR;						//	upper-right and lower-left of viewport in global
	CVector vLL;						//		coordinates.

	CVector vEnhancedDiagonal;			//	Length of 1/2 enhanced viewport diagonal
	CVector vEnhancedUR;
	CVector vEnhancedLL;

	int iPerception;					//	Perception
	Metric rIndicatorRadius;			//	Radius of circle to show target indicators (in pixels)
	WORD wSpaceColor;					//	Space color
	WORD wSpare;

	//	Options

	DWORD fNoSelection:1;
	DWORD fNoRecon:1;
	DWORD fNoDockedShips:1;
	DWORD fEnhancedDisplay:1;
	DWORD fNoStarfield:1;
	DWORD fShowManeuverEffects:1;
	DWORD fSpare7:1;
	DWORD fSpare8:1;

	DWORD dwSpare:24;

	CSpaceObject *pObj;					//	Current object being painted
	RECT rcObjBounds;					//	Object bounds in screen coordinates.
	int yAnnotations;					//	Start of free area for annotations (This start at the
										//		bottom of the object bounds, and each annotation
										//		should increment the value appropriately.

	//	May be modified by callers

	bool bInFront;						//	If TRUE, paint elements in front of object (otherwise, behind)
	bool bFade;							//	If TRUE, we're painting a fading element
	int iTick;
	int iVariant;
	int iDestiny;
	int iRotation;						//	An angle 0-359
	int iMaxLength;						//	Max length of object (used for projectiles); -1 == no limit
	};

class CMapViewportCtx
	{
	public:
		CMapViewportCtx (void);
		CMapViewportCtx (const CVector &vCenter, const RECT &rcView, Metric rMapScale);

		inline const CVector &GetCenterPos (void) const { return m_vCenter; }
		inline const RECT &GetViewportRect (void) const { return m_rcView; }
		inline ViewportTransform &GetXform (void) { return m_Trans; }
		bool IsInViewport (CSpaceObject *pObj) const;
		void Transform (const CVector &vPos, int *retx, int *rety) const;

	private:
		CVector m_vCenter;				//	Center of viewport in global coordinate
		RECT m_rcView;					//	RECT of viewport
		Metric m_rMapScale;				//	Map scale (klicks per pixel)

		CVector m_vUR;					//	Upper-right of viewport bounds (in global coordinates)
		CVector m_vLL;					//	Lower-left of viewport bounds
		int m_xCenter;					//	Center of viewport
		int m_yCenter;					//		(in viewport coordinate)

		ViewportTransform m_Trans;		//	Transform
	};

struct SPointInObjectCtx
	{
	SPointInObjectCtx (void) :
			pObjImage(NULL),
			pImage(NULL)
		{ }


	//	Used by CStation
	const CObjectImageArray *pObjImage;

	//	Used by CObjectImageArray
	CG16bitImage *pImage;				//	Image
	RECT rcImage;						//	RECT of valid image
	int xImageOffset;					//	Offset to convert from point coords to image coords
	int yImageOffset;
	};

class CObjectImage : public CDesignType
	{
	public:
		CObjectImage (void);
		CObjectImage (CG16bitImage *pBitmap, bool bFreeBitmap = false);
		~CObjectImage (void);

		CG16bitImage *CreateCopy (CString *retsError = NULL);
		ALERROR Exists (SDesignLoadCtx &Ctx);
		CG16bitImage *GetImage (const CString &sLoadReason, CString *retsError = NULL);
		CG16bitImage *GetImage (CResourceDb &ResDb, const CString &sLoadReason, CString *retsError = NULL);
		inline CString GetImageFilename (void) { return m_sBitmap; }
		inline bool HasAlpha (void) { return (m_pBitmap ? m_pBitmap->HasAlpha() : false); }

		inline void ClearMark (void) { m_bMarked = false; }
		ALERROR Lock (SDesignLoadCtx &Ctx);
		inline void Mark (void) { GetImage(NULL_STR); m_bMarked = true; }
		void Sweep (void);

		//	CDesignType overrides
		static CObjectImage *AsType (CDesignType *pType) { return ((pType && pType->GetType() == designImage) ? (CObjectImage *)pType : NULL); }
		virtual bool FindDataField (const CString &sField, CString *retsValue);
		virtual DesignTypes GetType (void) const { return designImage; }

	protected:
		//	CDesignType overrides
		virtual ALERROR OnCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);
		virtual ALERROR OnPrepareBindDesign (SDesignLoadCtx &Ctx);
		virtual void OnUnbindDesign (void);

	private:
		CString m_sResourceDb;			//	Resource db
		CString m_sBitmap;				//	Bitmap resource within db
		CString m_sBitmask;				//	Bitmask resource within db
		bool m_bTransColor;				//	If TRUE, m_wTransColor is valid
		WORD m_wTransColor;				//	Transparent color
		bool m_bSprite;					//	If TRUE, convert to sprite after loading
		bool m_bPreMult;				//	If TRUE, image is premultiplied with alpha
		bool m_bLoadOnUse;				//	If TRUE, image is only loaded when needed
		bool m_bFreeBitmap;				//	If TRUE, we free the bitmap when done

		CG16bitImage *m_pBitmap;		//	Loaded image (NULL if not loaded)
		bool m_bMarked;					//	Marked
		bool m_bLocked;					//	Image is never unloaded
	};

class CObjectImageArray : public CObject
	{
	public:
		CObjectImageArray (void);
		CObjectImageArray (const CObjectImageArray &Source);
		~CObjectImageArray (void);
		CObjectImageArray &operator= (const CObjectImageArray &Source);

		ALERROR Init (CG16bitImage *pBitmap, const RECT &rcImage, int iFrameCount, int iTicksPerFrame, bool bFreeBitmap);
		ALERROR Init (DWORD dwBitmapUNID, const RECT &rcImage, int iFrameCount, int iTicksPerFrame);
		ALERROR InitFromXML (CXMLElement *pDesc);
		ALERROR InitFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, bool bResolveNow = false, int iDefaultRotationCount = 1);
		ALERROR OnDesignLoadComplete (SDesignLoadCtx &Ctx);

		void CleanUp (void);
		void CopyImage (CG16bitImage &Dest, int x, int y, int iFrame, int iRotation) const;
		inline DWORD GetBitmapUNID (void) const { return m_dwBitmapUNID; }
		CString GetFilename (void) const;
		inline int GetFrameCount (void) const { return m_iFrameCount; }
		inline CG16bitImage &GetImage (const CString &sLoadReason) const { return *(m_pImage->GetImage(sLoadReason)); }
		inline const RECT &GetImageRect (void) const { return m_rcImage; }
		RECT GetImageRect (int iTick, int iRotation, int *retxCenter = NULL, int *retyCenter = NULL) const;
		RECT GetImageRectAtPoint (int x, int y) const;
		int GetImageViewportSize (void) const;
		bool GetImageOffset (int iTick, int iRotation, int *retx, int *rety) const;
		inline int GetRotationCount (void) const { return m_iRotationCount; }
		inline int GetTicksPerFrame (void) const { return m_iTicksPerFrame; }
		inline bool HasAlpha (void) const { return (m_pImage ? m_pImage->HasAlpha() : false); }
		bool ImagesIntersect (int iTick, int iRotation, int x, int y, const CObjectImageArray &Image2, int iTick2, int iRotation2) const;
		inline bool IsEmpty (void) const { return ((m_pImage == NULL) && (m_dwBitmapUNID == 0)); }
		inline bool IsLoaded (void) const { return (m_pImage != NULL); }
		void MarkImage (void);
		void PaintImage (CG16bitImage &Dest, int x, int y, int iTick, int iRotation, bool bComposite = false) const;
		void PaintImageGrayed (CG16bitImage &Dest, int x, int y, int iTick, int iRotation) const;
		void PaintImageShimmering (CG16bitImage &Dest,
								   int x,
								   int y,
								   int iTick,
								   int iRotation,
								   DWORD byOpacity) const;
		void PaintImageUL (CG16bitImage &Dest, int x, int y, int iTick, int iRotation) const;
		void PaintImageWithGlow (CG16bitImage &Dest,
								 int x,
								 int y,
								 int iTick,
								 int iRotation,
								 COLORREF rgbGlowColor) const;
		void PaintRotatedImage (CG16bitImage &Dest,
								int x,
								int y,
								int iTick,
								int iRotation,
								bool bComposite = false) const;
		void PaintScaledImage (CG16bitImage &Dest,
							   int x,
							   int y,
							   int iTick,
							   int iRotation,
							   int cxWidth,
							   int cyHeight,
							   bool bComposite = false) const;
		void PaintSilhoutte (CG16bitImage &Dest,
							 int x,
							 int y,
							 int iTick,
							 int iRotation,
							 WORD wColor) const;
		bool PointInImage (int x, int y, int iTick, int iRotation) const;
		bool PointInImage (SPointInObjectCtx &Ctx, int x, int y) const;
		void PointInImageInit (SPointInObjectCtx &Ctx, int iTick, int iRotation) const;
		void ReadFromStream (SLoadCtx &Ctx);
		void SetRotationCount (int iRotationCount);
		void TakeHandoff (CObjectImageArray &Source);
		void WriteToStream (IWriteStream *pStream) const;

	private:
		enum BlendingTypes
			{
			blendNormal,
			blendLighten,
			};

		struct OffsetStruct
			{
			int x;
			int y;
			};

		void ComputeRotationOffsets (void);
		void ComputeRotationOffsets (int xOffset, int yOffset);
		void ComputeSourceXY (int iTick, int iRotation, int *retxSrc, int *retySrc) const;
		inline void ComputeSourceXY (int iTick, int iRotation, LONG *retxSrc, LONG *retySrc) const { ComputeSourceXY(iTick, iRotation, (int *)retxSrc, (int *)retySrc); }
		void CopyFrom (const CObjectImageArray &Source);
		void GenerateGlowImage (int iRotation) const;
		void GenerateScaledImages (int iRotation, int cxWidth, int cyHeight) const;

		DWORD m_dwBitmapUNID;				//	UNID of bitmap (0 if none)
		CObjectImage *m_pImage;				//	Image (if UNID is 0, we own this structure)
		RECT m_rcImage;
		int m_iFrameCount;
		int m_iTicksPerFrame;
		int m_iFlashTicks;
		int m_iRotationCount;
		int m_iRotationOffset;
		OffsetStruct *m_pRotationOffset;
		int m_iBlending;
		int m_iViewportSize;				//	Size of 3D viewport in pixels (default to image width)
		int m_iFramesPerColumn;				//	Rotation frames spread out over multiple columns

		//	Glow effect
		mutable CG16bitImage *m_pGlowImages;
		mutable CG16bitImage *m_pScaledImages;

	friend CObjectClass<CObjectImageArray>;
	};

const DWORD DEFAULT_SELECTOR_ID = 0;

class CCompositeImageSelector
	{
	public:
		enum ETypes
			{
			typeNone,

			typeVariant,
			typeShipClass,
			typeItemType,
			};

		CCompositeImageSelector (void) { }

		bool operator== (const CCompositeImageSelector &Val) const;

		void AddFlotsam (DWORD dwID, CItemType *pItemType);
		void AddShipwreck (DWORD dwID, CShipClass *pWreckClass, int iVariant = -1);
		void AddVariant (DWORD dwID, int iVariant);
		inline void DeleteAll (void) { m_Sel.DeleteAll(); }
		inline int GetCount (void) const { return m_Sel.GetCount(); }
		CObjectImageArray &GetFlotsamImage (DWORD dwID = DEFAULT_SELECTOR_ID) const;
		CItemType *GetFlotsamType (DWORD dwID = DEFAULT_SELECTOR_ID) const;
		CShipClass *GetShipwreckClass (DWORD dwID = DEFAULT_SELECTOR_ID) const;
		CObjectImageArray &GetShipwreckImage (DWORD dwID = DEFAULT_SELECTOR_ID) const;
		ETypes GetType (DWORD dwID) const;
		int GetVariant (DWORD dwID) const;
		inline bool HasShipwreckImage (DWORD dwID = DEFAULT_SELECTOR_ID) const { return (GetShipwreckClass(dwID) != NULL); }
		void ReadFromStream (SLoadCtx &Ctx);
		void WriteToStream (IWriteStream *pStream) const;

	private:
		struct SEntry
			{
			DWORD dwID;
			int iVariant;					//	If -1 and dwExtra != 0, then this is an ItemType
			DWORD dwExtra;					//	Either 0 or a pointer to CItemType or CShipClass.
			};

		SEntry *FindEntry (DWORD dwID) const;
		ETypes GetEntryType (const SEntry &Entry) const;

		TArray<SEntry> m_Sel;
	};

struct SSelectorInitCtx
	{
	SSelectorInitCtx (void) :
			pSystem(NULL)
		{ }

	CSystem *pSystem;
	CVector vObjPos;
	CString sLocAttribs;
	};

class IImageEntry
	{
	public:
		virtual ~IImageEntry (void) { }

		virtual void AddTypesUsed (TSortMap<DWORD, bool> *retTypesUsed) { }
		inline DWORD GetID (void) { return m_dwID; }
		virtual void GetImage (const CCompositeImageSelector &Selector, CObjectImageArray *retImage) = 0;
		virtual int GetMaxLifetime (void) const { return 0; }
		virtual int GetVariantCount (void) = 0;
		virtual ALERROR InitFromXML (SDesignLoadCtx &Ctx, CIDCounter &IDGen, CXMLElement *pDesc) { return NOERROR; }
		virtual void InitSelector (SSelectorInitCtx &InitCtx, CCompositeImageSelector *retSelector) { }
		virtual bool IsConstant (void) = 0;
		virtual void MarkImage (void) { }
		virtual ALERROR OnDesignLoadComplete (SDesignLoadCtx &Ctx) { return NOERROR; }

	protected:
		DWORD m_dwID;
	};

class CCompositeImageModifiers
	{
	public:
		CCompositeImageModifiers (void) :
				m_wFadeColor(0),
				m_wFadeOpacity(0),
				m_fStationDamage(false)
			{ }

		bool operator== (const CCompositeImageModifiers &Val) const;

		void Apply (CObjectImageArray *retImage) const;
		inline bool IsEmpty (void) const { return (m_wFadeOpacity == 0 && !m_fStationDamage); }
		inline void SetFadeColor (WORD wColor, DWORD dwOpacity) { m_wFadeColor = wColor; m_wFadeOpacity = (WORD)dwOpacity; }
		inline void SetStationDamage (bool bValue = true) { m_fStationDamage = bValue; }

		static void Reinit (void);

	private:
		static void InitDamagePainters (void);
		static void PaintDamage (CG16bitImage &Dest, const RECT &rcDest, int iCount, IEffectPainter *pPainter);

		CG16bitImage *CreateCopy (CObjectImageArray *pImage, RECT *retrcNewImage) const;

		WORD m_wFadeColor;					//	Apply a wash on top of image
		WORD m_wFadeOpacity;				//		0 = no wash

		DWORD m_fStationDamage:1;			//	Apply station damage to image
		DWORD m_dwSpare:31;
	};

class CCompositeImageDesc
	{
	public:
		CCompositeImageDesc (void);
		~CCompositeImageDesc (void);

		inline void AddTypesUsed (TSortMap<DWORD, bool> *retTypesUsed) { if (m_pRoot) m_pRoot->AddTypesUsed(retTypesUsed); }
		CObjectImageArray &GetImage (const CCompositeImageSelector &Selector, const CCompositeImageModifiers &Modifiers = CCompositeImageModifiers(), int *retiRotation = NULL) const;
		int GetMaxLifetime (void) const;
		inline IImageEntry *GetRoot (void) const { return m_pRoot; }
		inline int GetVariantCount (void) { return (m_pRoot ? m_pRoot->GetVariantCount() : 0); }
		static ALERROR InitEntryFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, CIDCounter &IDGen, IImageEntry **retpEntry);
		ALERROR InitFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);
		void InitSelector (SSelectorInitCtx &InitCtx, CCompositeImageSelector *retSelector);
		inline bool IsConstant (void) const { return m_bConstant; }
		inline bool IsEmpty (void) { return (GetVariantCount() == 0); }
		void MarkImage (void);
		void MarkImage (const CCompositeImageSelector &Selector, const CCompositeImageModifiers &Modifiers = CCompositeImageModifiers());
		ALERROR OnDesignLoadComplete (SDesignLoadCtx &Ctx);
		void Reinit (void);

	private:
		struct SCacheEntry
			{
			CCompositeImageSelector Selector;
			CCompositeImageModifiers Modifiers;
			CObjectImageArray Image;
			};

		SCacheEntry *FindCacheEntry (const CCompositeImageSelector &Selector, const CCompositeImageModifiers &Modifiers) const;

		IImageEntry *m_pRoot;
		bool m_bConstant;
		mutable TArray<SCacheEntry> m_Cache;
	};

//	Sounds

class CSoundRef
	{
	public:
		CSoundRef (void) : m_dwUNID(0), m_iSound(-1)
			{ }

		ALERROR Bind (SDesignLoadCtx &Ctx);
		DWORD GetUNID (void) const { return m_dwUNID; }
		int GetSound (void) const { return m_iSound; }
		inline bool IsNull (void) const { return (m_dwUNID == 0); }
		ALERROR LoadUNID (SDesignLoadCtx &Ctx, const CString &sAttrib);
		void PlaySound (CSpaceObject *pSource);

	private:
		DWORD m_dwUNID;
		int m_iSound;
	};

class CSoundType : public CDesignType
	{
	public:
		CSoundType (void) : m_iNextSegment(0)
			{ }

		~CSoundType (void) { }

		int FindSegment (int iPos);
		const CString &GetAlbum (void) const;
		inline const CString &GetComposedBy (void) const { return m_sComposedBy; }
		CString GetFilename (void) const { return m_sFilename; }
		CString GetFilespec (void) const;
		inline const CAttributeCriteria &GetLocationCriteria (void) const { return m_LocationCriteria; }
		int GetNextFadePos (int iPos);
		int GetNextPlayPos (void);
		inline const CString &GetPerformedBy (void) const { return m_sPerformedBy; }
		inline int GetPriority (void) const { return m_iPriority; }
		inline int GetSegmentCount (void) const { return (m_Segments.GetCount() == 0 ? 1 : m_Segments.GetCount()); }
		inline const CString &GetTitle (void) const { return m_sTitle; }
		inline void Init (DWORD dwUNID, const CString &sFilespec, int iPriority = 0) { SetUNID(dwUNID); m_sFilespec = sFilespec; m_iPriority = iPriority; }
		void SetLastPlayPos (int iPos);

		//	CDesignType overrides
		static CSoundType *AsType (CDesignType *pType) { return ((pType && pType->GetType() == designSound) ? (CSoundType *)pType : NULL); }
		virtual bool FindDataField (const CString &sField, CString *retsValue);
		virtual DesignTypes GetType (void) const { return designSound; }

	protected:
		//	CDesignType overrides
		virtual void OnAddExternals (TArray<CString> *retExternals) { if (!m_sFilespec.IsBlank()) retExternals->Insert(m_sFilespec); }
		virtual ALERROR OnCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);

	private:
		struct SSegmentDesc
			{
			int iStartPos;
			int iEndPos;				//	-1 = end of track
			};

		CString m_sResourceDb;			//	Resource db
		CString m_sFilename;			//	Filename
		CString m_sFilespec;			//	Sound resource within db

		CString m_sTitle;
		CString m_sComposedBy;
		CString m_sPerformedBy;
		int m_iPriority;				//	Track priority
		CAttributeCriteria m_LocationCriteria;	//	Play in specific systems

		TArray<SSegmentDesc> m_Segments;
		int m_iNextSegment;				//	Index of last segment played
	};

//	Damage

enum SpecialDamageTypes
	{
	specialNone			= -1,

	specialRadiation	= 0,
	specialBlinding		= 1,
	specialEMP			= 2,
	specialDeviceDamage	= 3,
	specialDisintegration = 4,
	specialMomentum		= 5,
	specialShieldDisrupt = 6,
	specialWMD			= 7,
	specialMining		= 8,

	specialDeviceDisrupt= 9,
	specialWormhole		= 10,
	specialFuel			= 11,
	specialShatter		= 12,
	specialArmor		= 13,
	};

class DamageDesc
	{
	public:
		enum Flags 
			{
			flagAverageDamage = 0x00000001,
			};

		DamageDesc (void) { }
		DamageDesc (DamageTypes iType, const DiceRange &Damage) : m_iType(iType),
				m_Damage(Damage),
				m_iBonus(0),
				m_iCause(killedByDamage),
				m_EMPDamage(0),
				m_MomentumDamage(0),
				m_RadiationDamage(0),
				m_DisintegrationDamage(0),
				m_DeviceDisruptDamage(0),
				m_BlindingDamage(0),
				m_SensorDamage(0),
				m_ShieldDamage(0),
				m_ArmorDamage(0),
				m_WormholeDamage(0),
				m_FuelDamage(0),
				m_fNoSRSFlash(0),
				m_fAutomatedWeapon(0),
				m_DeviceDamage(0),
				m_MassDestructionAdj(0),
				m_MiningAdj(0),
				m_ShatterDamage(0),
				m_dwSpare2(0)
			{ }

		inline void AddBonus (int iBonus) { m_iBonus += iBonus; }
		void AddEnhancements (CItemEnhancementStack *pEnhancements);
		inline bool CausesSRSFlash (void) const { return (m_fNoSRSFlash ? false : true); }
		inline Metric GetAverageDamage (void) const { return m_Damage.GetAveValueFloat(); }
		inline DestructionTypes GetCause (void) const { return m_iCause; }
		inline DamageTypes GetDamageType (void) const { return m_iType; }
		CString GetDesc (DWORD dwFlags = 0);
		int GetMinDamage (void);
		int GetMaxDamage (void);
		int GetSpecialDamage (SpecialDamageTypes iSpecial) const;
		bool IsAutomatedWeapon (void) const { return (m_fAutomatedWeapon ? true : false); }
		bool IsEnergyDamage (void) const;
		bool IsMatterDamage (void) const;
		ALERROR LoadFromXML (SDesignLoadCtx &Ctx, const CString &sAttrib);
		void ReadFromStream (SLoadCtx &Ctx);
		int RollDamage (void) const;
		inline void SetAutomatedWeapon (void) { m_fAutomatedWeapon = true; }
		inline void SetCause (DestructionTypes iCause) { m_iCause = iCause; }
		void SetDamage (int iDamage);
		inline void SetDamageType (DamageTypes iType) { m_iType = iType; }
		inline void SetNoSRSFlash (void) { m_fNoSRSFlash = true; }
		void SetSpecialDamage (SpecialDamageTypes iSpecial, int iLevel);
		void WriteToStream (IWriteStream *pStream) const;

		inline int GetArmorDamageLevel (void) const { return (int)m_ArmorDamage; }
		inline int GetBlindingDamage (void) const { return (int)m_BlindingDamage; }
		inline int GetDeviceDamage (void) const { return (int)m_DeviceDamage; }
		inline int GetDeviceDisruptDamage (void) const { return (int)m_DeviceDisruptDamage; }
		inline int GetDisintegrationDamage (void) const { return (int)m_DisintegrationDamage; }
		inline int GetEMPDamage (void) const { return (int)m_EMPDamage; }
		inline int GetMassDestructionAdj (void) const { return (int)(m_MassDestructionAdj ? (2 * (m_MassDestructionAdj * m_MassDestructionAdj) + 2) : 0); }
		inline int GetMiningAdj (void) const { return (int)(m_MiningAdj ? (2 * (m_MiningAdj * m_MiningAdj) + 2) : 0); }
		inline int GetMomentumDamage (void) const { return (int)m_MomentumDamage; }
		inline int GetRadiationDamage (void) const { return (int)m_RadiationDamage; }
		inline int GetShatterDamage (void) const { return (int)m_ShatterDamage; }
		inline int GetShieldDamageLevel (void) const { return (int)m_ShieldDamage; }

		static SpecialDamageTypes ConvertToSpecialDamageTypes (const CString &sValue);

	private:
		ALERROR LoadTermFromXML (SDesignLoadCtx &Ctx, const CString &sType, const CString &sArg);
		ALERROR ParseTerm (SDesignLoadCtx &Ctx, char *pPos, CString *retsKeyword, CString *retsValue, char **retpPos);

		DamageTypes m_iType;					//	Type of damage
		DiceRange m_Damage;						//	Amount of damage
		int m_iBonus;							//	Bonus to damage (%)
		DestructionTypes m_iCause;				//	Cause of damage

		//	Extra damage
		DWORD m_EMPDamage:3;					//	Ion (paralysis) damage
		DWORD m_MomentumDamage:3;				//	Momentum damage
		DWORD m_RadiationDamage:3;				//	Radiation damage
		DWORD m_DeviceDisruptDamage:3;			//	Disrupt devices damage
		DWORD m_BlindingDamage:3;				//	Optical sensor damage
		DWORD m_SensorDamage:3;					//	Long-range sensor damage
		DWORD m_WormholeDamage:3;				//	Teleport
		DWORD m_FuelDamage:3;					//	Drain fuel
		DWORD m_DisintegrationDamage:3;			//	Disintegration damage
		DWORD m_dwSpare1:3;

		DWORD m_fNoSRSFlash:1;					//	If TRUE, damage should not cause SRS flash
		DWORD m_fAutomatedWeapon:1;				//	TRUE if this damage is caused by automated weapon

		DWORD m_DeviceDamage:3;					//	Damage to devices
		DWORD m_MassDestructionAdj:3;			//	Adj for mass destruction
		DWORD m_MiningAdj:3;					//	Adj for mining capability
		DWORD m_ShatterDamage:3;				//	Shatter damage
		DWORD m_dwSpare2:20;

		BYTE m_ShieldDamage;					//	Shield damage (level)
		BYTE m_ArmorDamage;						//	Armor damage (level)
		BYTE m_Spare2;
		BYTE m_Spare3;
	};

enum EDamageResults
	{
	damageNoDamage =				0,
	damageAbsorbedByShields =		1,
	damageArmorHit =				2,
	damageStructuralHit =			3,
	damageDestroyed =				4,
	damagePassthrough =				5,	//	When we hit another missile (or small obj) we pass through
	damagePassthroughDestroyed =	6,	//	Target destroyed, but we pass through
	damageDestroyedAbandoned =		7,	//	Station was abandoned, but object not destroyed
	damageNoDamageNoPassthrough =	8,	//	No damage; stop any passthrough

	damageResultCount =				9,
	};

struct SDamageCtx
	{
	CSpaceObject *pObj;							//	Object hit
	CWeaponFireDesc *pDesc;						//	WeaponFireDesc
	DamageDesc Damage;							//	Damage
	int iDirection;								//	Direction that hit came from
	CVector vHitPos;							//	Hit at this position
	CSpaceObject *pCause;						//	Object that directly caused the damage
	CDamageSource Attacker;						//	Ultimate attacker

	int iDamage;								//	Damage hp
	int iSectHit;								//	Armor section hit on object

	//	These are used within armor/shield processing
	int iHPLeft;								//	HP left on armor/shields (before damage)
	int iAbsorb;								//	Damage absorbed by shields
	int iShieldDamage;							//	Damage taken by shields
	int iOriginalAbsorb;						//	Computed absorb value, if shot had not been reflected
	int iOriginalShieldDamage;					//	Computed shield damage value, if shot had not been reflected

	//	Damage effects
	bool bBlind;								//	If true, shot will blind the target
	int iBlindTime;
	bool bDeviceDisrupt;						//	If true, shot will disrupt devices
	int iDisruptTime;
	bool bDeviceDamage;							//	If true, shot will damage devices
	bool bDisintegrate;							//	If true, shot will disintegrate target
	bool bParalyze;								//	If true, shot will paralyze the target
	int iParalyzeTime;
	bool bRadioactive;							//	If true, shot will irradiate the target
	bool bReflect;								//	If true, armor/shields reflected the shot
	bool bShatter;								//	If true, shot will shatter the target
	};

struct SDestroyCtx
	{
	SDestroyCtx (void) :
		pObj(NULL),
		pWreck(NULL),
		iCause(removedFromSystem),
		bResurrectPending(false),
		pResurrectedObj(NULL) { }

	CSpaceObject *pObj;							//	Object destroyed
	CDamageSource Attacker;						//	Ultimate attacker
	CSpaceObject *pWreck;						//	Wreck left behind
	DestructionTypes iCause;					//	Cause of damage

	bool bResurrectPending;						//	TRUE if this object will be resurrected
	CSpaceObject *pResurrectedObj;				//	Pointer to resurrected object
	};

class DamageTypeSet
	{
	public:
		DamageTypeSet (void) : m_dwSet(0) { }

		ALERROR InitFromXML (const CString &sAttrib);
		void Add (int iType) { if (iType > damageGeneric) m_dwSet |= (1 << iType); }
		bool InSet (int iType) { return (iType <= damageGeneric ? false : ((m_dwSet & (1 << iType)) ? true : false)); }
		inline bool IsEmpty (void) const { return (m_dwSet == 0); }
		void Remove (int iType) { if (iType > damageGeneric) m_dwSet &= ~(1 << iType); }

	private:
		DWORD m_dwSet;
	};

//	WeaponFireDesc

enum FireTypes
	{
	ftArea,
	ftBeam,
	ftMissile,
	ftParticles,
	ftRadius,
	};

enum ParticleTypes
	{
	particleImage,
	particlePlasma,
	particleBolt,
	};

//	Communications

//	DO NOT CHANGE NUMBERS
//	NOTE: These numbers must match-up with CSovereign.cpp and with the
//	definitions in the <Sovereign> elements. And with Utilities.cpp

enum MessageTypes
	{
	msgNone =						0,	//	None
	msgAttack =						1,	//	Order to attack (pParam1 is target)
	msgDestroyBroadcast =			2,	//	Broadcast order to destroy target (pParam1 is target)
	msgHitByFriendlyFire =			3,	//	Hit by friendly fire (pParam1 is attacker)
	msgQueryEscortStatus =			4,	//	Replies with Ack if is escorting pParam1
	msgQueryFleetStatus =			5,	//	Replies with Ack if is in formation with pParam1
	msgEscortAttacked =				6,	//	pParam1 attacked escort ship (or defender)
	msgEscortReportingIn =			7,	//	pParam1 is now escorting the ship
	msgWatchTargets =				8,	//	"watch your targets!"
	msgNiceShooting =				9,	//	"nice shooting!"
	msgFormUp =						10,	//	Enter formation (dwParam2 is formation place)
	msgBreakAndAttack =				11,	//	Break formation and attack
	msgQueryCommunications =		12,	//	Returns comms capabilities
	msgAbort =						13,	//	Cancel attack order
	msgWait =						14,	//	Wait there for a while
	msgQueryWaitStatus =			15,	//	Replies with Ack if waiting
	msgAttackInFormation =			16,	//	Fire weapon while staying in formation
	msgAttackDeter =				17, //	Attack target, but return after a while (pParam1 is target)
	msgQueryAttackStatus =			18, //	Replies with Ack if attacking a target
	msgDockingSequenceEngaged =		19,	//	Sent by a station to anyone docking
	};

enum ResponseTypes
	{
	resNoAnswer =					0,
	resAck =						1,

	resCanBeInFormation =			0x0010,
	resCanAttack =					0x0020,
	resCanBreakAndAttack =			0x0040,
	resCanAbortAttack =				0x0080,
	resCanWait =					0x0100,
	resCanFormUp =					0x0200,
	resCanAttackInFormation =		0x0400,
	};

class CCommunicationsHandler
	{
	public:
		struct SMessage
			{
			CString sMessage;
			CString sShortcut;

			SEventHandlerDesc InvokeEvent;
			SEventHandlerDesc OnShowEvent;
			};

		CCommunicationsHandler (void);
		~CCommunicationsHandler (void);

		CCommunicationsHandler &operator= (const CCommunicationsHandler &Src);
		ALERROR InitFromXML (CXMLElement *pDesc, CString *retsError);

		void DeleteAll (void);
		int FindMessage (const CString &sMessage) const;
		inline int GetCount (void) const { return m_Messages.GetCount(); }
		inline const SMessage &GetMessage (int iIndex) { return m_Messages[iIndex]; }
		void Merge (CCommunicationsHandler &New);

	private:
		int FindByShortcut (const CString &sShortcut, int *retiInsert = NULL);
		TArray<SMessage> m_Messages;
	};

//	Equipment (Abilities)

enum Abilities
	{
	ablUnknown =				-1,

	ablShortRangeScanner =		0,		//	Main viewscreen
	ablLongRangeScanner =		1,		//	LRS
	ablSystemMap =				2,		//	System map display
	ablAutopilot =				3,		//	Autopilot
	ablExtendedScanner =		4,		//	Extended marks on viewscreen
	ablTargetingSystem =		5,		//	Targeting computer
	ablGalacticMap =			6,		//	Galactic map display
	};

enum AbilityModifications
	{
	ablModificationUnknown =	-1,

	ablInstall =				0,		//	Install the ability
	ablRemove =					1,		//	Remove the ability (if installed)
	ablDamage =					2,		//	Damage the ability (if installed)
	ablRepair =					3,		//	Repair the ability (if damaged)
	};

enum AbilityModificationOptions
	{
	ablOptionUnknown =			0x00000000,

	ablOptionNoMessage =		0x00000001,	//	Do not show a message to player
	};

enum AbilityStatus
	{
	ablStatusUnknown =			-1,

	ablUninstalled =			0,		//	>0 means that is installed (though it could be damaged)
	ablInstalled =				1,
	ablDamaged =				2,
	};

//	Item Types

enum ItemEnhancementTypes
	{
	etNone =							0x0000,
	etBinaryEnhancement =				0x0001,
	etLoseEnhancement =					0x0002,	//	Lose enhancements

	etStrengthen =						0x0100,	//	+hp, data1 = %increase (10% increments)
	etRegenerate =						0x0200,	//	data1 = rate
	etReflect =							0x0300,	//	data2 = damage type reflected
	etRepairOnHit =						0x0400,	//	repair damage on hit, data2 = damage type of hit
	etResist =							0x0500,	//	-damage, data1 = %damage adj
	etResistEnergy =					0x0600,	//	-energy damage, data1 = %damage adj (90%, 80%, etc)
	etResistMatter =					0x0700,	//	-matter damage, data1 = %damage adj (90%, 80%, etc)
	etResistByLevel =					0x0800,	//	-damage, data1 = %damage adj, data2 = damage type
	etResistByDamage =					0x0900,	//	-damage, data1 = %damage adj, data2 = damage type
	etResistByDamage2 =					0x0a00,	//	-damage, data1 = %damage adj, data2 = damage type
	etSpecialDamage =					0x0b00,	//	Immunity to damage effects:
												//		data2 = 0: immune to radiation
												//		data2 = 1: immune to blinding
												//		... see SpecialDamageTypes
	etImmunityIonEffects =				0x0c00,	//	Immunity to ion effects (blinding, EMP, etc.)
												//		(if disadvantage, interferes with shields)
	etPhotoRegenerate =					0x0d00,	//	regen near sun
	etPhotoRecharge =					0x0e00,	//	refuel near sun
	etPowerEfficiency =					0x0f00,	//	power usage decrease, 01 = 90%/110%, 02 = 80%/120%
	etSpeedOld =						0x1000,	//	decrease cycle time
	etTurret =							0x1100,	//	weapon turret, data1 is angle
	etMultiShot =						0x1200,	//	multiple shots, data2 = count, data1 = %weakening
	etSpeed =							0x1300,	//	decrease cycle time
												//		A = adj (0-100)
												//			[if disavantage] adj by 5% starting at 100%.
												//
												//		B = min delay in ticks (do not decrease below this; 0 = no limit)
												//		C = max delay in ticks (do not increase above this; 0 = no limit)
	etConferSpecialDamage =				0x1400,	//	weapon gains special damage
												//		A = SpecialDamageTypes
												//		B = damage level
	etHPBonus =							0x1500,	//	+hp%, like etStrengthen
												//		X = %increase

	etData1Mask =						0x000f,	//	4-bits of data (generally for damage adj)
	etData2Mask =						0x00f0,	//	4-bits of data (generally for damage type)
	etTypeMask =						0x7f00,	//	Type
	etDisadvantage =					0x8000,	//	If set, this is a disadvantage

	etDataAMask =					0x000000ff,	//	8-bits of data
	etDataAMax =						   255,
	etDataBMask =					0x00ff0000,	//	8-bits of data
	etDataBMax =						   255,
	etDataCMask =					0xff000000,	//	8-bits of data
	etDataCMax =						   255,
	etDataXMask =					0xffff0000,	//	16-bits of data
	etDataXMax =						 65535,
	};

enum EnhanceItemStatus
	{
	eisOK,										//	Enhancement OK
	eisNoEffect,								//	Nothing happens
	eisAlreadyEnhanced,							//	Already has this exact enhancement
	eisWorse,									//	A disadvantage was made worse
	eisRepaired,								//	Disadvantage was repaired
	eisEnhancementRemoved,						//	Enhancement removed
	eisEnhancementReplaced,						//	Enhancement replaced by another enhancement
	eisBetter,									//	Enhancement made better
	};

class CItemEnhancement
	{
	public:
		CItemEnhancement (void) : m_dwID(OBJID_NULL), m_dwMods(0), m_pEnhancer(NULL), m_iExpireTime(-1) { }
		CItemEnhancement (DWORD dwMods) : m_dwID(OBJID_NULL), m_dwMods(dwMods), m_pEnhancer(NULL), m_iExpireTime(-1) { }

		inline DWORD AsDWORD (void) const { return m_dwMods; }
		EnhanceItemStatus Combine (const CItem &Item, CItemEnhancement Enhancement);
		int GetAbsorbAdj (const DamageDesc &Damage) const;
		int GetActivateRateAdj (int *retiMinDelay = NULL, int *retiMaxDelay = NULL) const;
		int GetDamageAdj (const DamageDesc &Damage) const;
		inline DamageTypes GetDamageType (void) const { return (DamageTypes)(DWORD)((m_dwMods & etData2Mask) >> 4); }
		inline int GetDataA (void) const { return (int)(DWORD)(m_dwMods & etDataAMask); }
		inline int GetDataB (void) const { return (int)(DWORD)((m_dwMods & etDataBMask) >> 16); }
		inline int GetDataC (void) const { return (int)(DWORD)((m_dwMods & etDataCMask) >> 24); }
		inline int GetDataX (void) const { return (int)(DWORD)((m_dwMods & etDataXMask) >> 16); }
		CString GetEnhancedDesc (const CItem &Item, CSpaceObject *pInstalled, CInstalledDevice *pDevice = NULL) const;
		int GetEnhancedRate (int iRate) const;
		inline CItemType *GetEnhancementType (void) const { return m_pEnhancer; }
		inline int GetExpireTime (void) const { return m_iExpireTime; }
		int GetHPAdj (void) const;
		int GetHPBonus (void) const;
		inline DWORD GetID (void) const { return m_dwID; }
		inline int GetLevel (void) const { return (int)(DWORD)(m_dwMods & etData1Mask); }
		inline int GetLevel2 (void) const { return (int)(DWORD)((m_dwMods & etData2Mask) >> 4); }
		inline DWORD GetModCode (void) const { return m_dwMods; }
		int GetPowerAdj (void) const;
		SpecialDamageTypes GetSpecialDamage (int *retiLevel = NULL) const;
		inline ItemEnhancementTypes GetType (void) const { return (ItemEnhancementTypes)(m_dwMods & etTypeMask); }
		int GetValueAdj (const CItem &Item) const;
		ALERROR InitFromDesc (ICCItem *pItem, CString *retsError);
		ALERROR InitFromDesc (const CString &sDesc, CString *retsError);
		ALERROR InitFromDesc (SDesignLoadCtx &Ctx, const CString &sDesc);
		inline bool IsBlindingImmune (void) const { return IsIonEffectImmune() || ((GetType() == etSpecialDamage) && GetLevel2() == specialBlinding && !IsDisadvantage()); }
		inline bool IsDecaying (void) const { return ((GetType() == etRegenerate) && IsDisadvantage()); }
		inline bool IsDeviceDamageImmune (void) const { return IsIonEffectImmune() || ((GetType() == etSpecialDamage) && GetLevel2() == specialDeviceDamage && !IsDisadvantage()); }
		inline bool IsDisadvantage (void) const { return ((m_dwMods & etDisadvantage) ? true : false); }
		inline bool IsDisintegrationImmune (void) const { return ((GetType() == etSpecialDamage) && GetLevel2() == specialDisintegration && !IsDisadvantage()); }
		inline bool IsEMPImmune (void) const { return IsIonEffectImmune() || ((GetType() == etSpecialDamage) && GetLevel2() == specialEMP && !IsDisadvantage()); }
		inline bool IsEmpty (void) const { return (m_dwMods == 0 && m_pEnhancer == NULL); }
		inline bool IsEnhancement (void) const { return (m_dwMods && !IsDisadvantage()); }
		bool IsEqual (const CItemEnhancement &Comp) const;
		inline bool IsNotEmpty (void) const { return !IsEmpty(); }
		inline bool IsPhotoRecharge (void) const { return ((GetType() == etPhotoRecharge) && !IsDisadvantage()); }
		inline bool IsPhotoRegenerating (void) const { return ((GetType() == etPhotoRegenerate) && !IsDisadvantage()); }
		inline bool IsRadiationImmune (void) const { return ((GetType() == etSpecialDamage) && GetLevel2() == 0 && !IsDisadvantage()); }
		inline bool IsRegenerating (void) const { return ((GetType() == etRegenerate) && !IsDisadvantage()); }
		inline bool IsReflective (void) const { return ((GetType() == etReflect) && !IsDisadvantage()); }
		bool IsReflective (const DamageDesc &Damage, int *retiReflectChance = NULL) const;
		inline bool IsShatterImmune (void) const { return ((GetType() == etSpecialDamage) && GetLevel2() == specialShatter && !IsDisadvantage()); }
		inline bool IsShieldInterfering (void) const { return ((GetType() == etImmunityIonEffects) && IsDisadvantage()); }
		inline bool IsStacking (void) const { return (GetType() == etStrengthen && GetLevel() == 0); }
		void ReadFromStream (DWORD dwVersion, IReadStream *pStream);
		void ReadFromStream (SLoadCtx &Ctx);
		inline void SetEnhancementType (CItemType *pType) { m_pEnhancer = pType; }
		inline void SetExpireTime (int iTime) { m_iExpireTime = iTime; }
		inline void SetID (DWORD dwID) { m_dwID = dwID; }
		void SetModBonus (int iBonus);
		inline void SetModCode (DWORD dwMods) { m_dwMods = dwMods; }
		inline void SetModImmunity (SpecialDamageTypes iSpecial) { m_dwMods = Encode12(etSpecialDamage, 0, (int)iSpecial); }
		inline void SetModReflect (DamageTypes iDamageType) { m_dwMods = Encode12(etReflect, 0, (int)iDamageType); }
		inline void SetModResistDamage (DamageTypes iDamageType, int iAdj) { m_dwMods = Encode12(etResistByDamage | (iAdj > 100 ? etDisadvantage : 0), DamageAdj2Level(iAdj), (int)iDamageType); }
		inline void SetModResistDamageClass (DamageTypes iDamageType, int iAdj) { m_dwMods = Encode12(etResistByDamage2 | (iAdj > 100 ? etDisadvantage : 0), DamageAdj2Level(iAdj), (int)iDamageType); }
		inline void SetModResistDamageTier (DamageTypes iDamageType, int iAdj) { m_dwMods = Encode12(etResistByLevel | (iAdj > 100 ? etDisadvantage : 0), DamageAdj2Level(iAdj), (int)iDamageType); }
		inline void SetModResistEnergy (int iAdj) { m_dwMods = Encode12(etResistEnergy | (iAdj > 100 ? etDisadvantage : 0), DamageAdj2Level(iAdj)); }
		inline void SetModResistMatter (int iAdj) { m_dwMods = Encode12(etResistMatter | (iAdj > 100 ? etDisadvantage : 0), DamageAdj2Level(iAdj)); }
		void SetModSpecialDamage (SpecialDamageTypes iSpecial, int iLevel = 0);
		void SetModSpeed (int iAdj, int iMinDelay = 0, int iMaxDelay = 0);
		void WriteToStream (IWriteStream *pStream) const;

	private:
		inline bool IsIonEffectImmune (void) const { return ((GetType() == etImmunityIonEffects) && !IsDisadvantage()); }

		static int DamageAdj2Level (int iDamageAdj);
		static DWORD EncodeABC (DWORD dwTypeCode, int A = 0, int B = 0, int C = 0);
		static DWORD EncodeAX (DWORD dwTypeCode, int A = 0, int X = 0);
		static DWORD Encode12 (DWORD dwTypeCode, int Data1 = 0, int Data2 = 0);
		static int Level2Bonus (int iLevel, bool bDisadvantage = false);
		static int Level2DamageAdj (int iLevel, bool bDisadvantage = false);

		DWORD m_dwID;							//	Global ID
		DWORD m_dwMods;							//	Mod code
		CItemType *m_pEnhancer;					//	Item that added this mod (may be NULL)
		int m_iExpireTime;						//	Universe tick when mod expires (-1 == no expiration)
	};

class CItemEnhancementStack
	{
	public:
		CItemEnhancementStack (void) :
				m_bCacheValid(false),
				m_dwRefCount(1)
			{ }

		inline CItemEnhancementStack *AddRef (void) { m_dwRefCount++; return this; }
		void ApplySpecialDamage (DamageDesc *pDamage) const;
		int CalcActivateDelay (CItemCtx &DeviceCtx) const;
		void Delete (void);
		int GetBonus (void) const;
		inline int GetCount (void) const { return m_Stack.GetCount(); }
		const DamageDesc &GetDamage (void) const;
		void Insert (const CItemEnhancement &Mods);
		void InsertActivateAdj (int iAdj, int iMin, int iMax);
		void InsertHPBonus (int iBonus);
		inline bool IsEmpty (void) const { return (m_Stack.GetCount() == 0); }

		static void ReadFromStream (SLoadCtx &Ctx, CItemEnhancementStack **retpStack);
		static void WriteToStream (CItemEnhancementStack *pStack, IWriteStream *pStream);

	private:
		~CItemEnhancementStack (void) { }
		void CalcCache (void) const;

		TArray<CItemEnhancement> m_Stack;

		mutable bool m_bCacheValid;				//	If TRUE, these cache values are OK.
		mutable int m_iBonus;					//	Cached bonus
		mutable DamageDesc m_Damage;			//	Cached damage descriptor

		DWORD m_dwRefCount;
	};

class CItem
	{
	public:
		CItem (void);
		CItem (const CItem &Copy);
		CItem (CItemType *pItemType, int iCount);
		~CItem (void);
		CItem &operator= (const CItem &Copy);

		DWORD AddEnhancement (const CItemEnhancement &Enhancement);
		inline void ClearDamaged (void) { m_dwFlags &= ~flagDamaged; }
		inline void ClearDisrupted (void) { if (m_pExtra) m_pExtra->m_dwDisruptedTime = 0; }
		inline void ClearEnhanced (void) { m_dwFlags &= ~flagEnhanced; }
		inline void ClearModFlag (int iMod) { m_dwFlags &= ~Mod2Flags(iMod); }
		static CItem CreateItemByName (const CString &sName, const CItemCriteria &Criteria, bool bActualName = false);
		inline bool IsArmor (void) const;
		inline bool IsDevice (void) const;
		bool IsEqual (const CItem &Item, bool bIgnoreInstalled = false) const;
		bool FireCanBeInstalled (CSpaceObject *pSource, CString *retsError) const;
		bool FireCanBeUninstalled (CSpaceObject *pSource, CString *retsError) const;
		void FireOnAddedAsEnhancement (CSpaceObject *pSource, const CItem &ItemEnhanced, EnhanceItemStatus iStatus) const;
		void FireOnDisabled (CSpaceObject *pSource) const;
		void FireOnEnabled (CSpaceObject *pSource) const;
		void FireOnInstall (CSpaceObject *pSource) const;
		void FireOnObjDestroyed (CSpaceObject *pSource, const SDestroyCtx &Ctx) const;
		bool FireOnReactorOverload (CSpaceObject *pSource) const;
		void FireOnRemovedAsEnhancement (CSpaceObject *pSource, const CItem &ItemEnhanced) const;
		void FireOnUninstall (CSpaceObject *pSource) const;
		inline int GetCharges (void) const { return (m_pExtra ? (int)m_pExtra->m_dwCharges : 0); }
		inline int GetCount (void) const { return (int)m_dwCount; }
		inline CEconomyType *GetCurrencyType (void) const;
		inline CString GetData (const CString &sAttrib) const { return (m_pExtra ? m_pExtra->m_Data.GetData(sAttrib) : NULL_STR); }
		CString GetDesc (void) const;
		bool GetDisplayAttributes (CItemCtx &Ctx, TArray<SDisplayAttribute> *retList) const;
		DWORD GetDisruptedDuration (void) const;
		CString GetEnhancedDesc (CSpaceObject *pInstalled = NULL) const;
		inline int GetInstalled (void) const { return (int)(char)m_dwInstalled; }
		inline Metric GetMass (void) const { return GetMassKg() / 1000.0; }
		int GetMassKg (void) const;
		inline const CItemEnhancement &GetMods (void) const { return (m_pExtra ? m_pExtra->m_Mods : m_NullMod); }
		static const CItem &GetNullItem (void) { return m_NullItem; }
		static const CItemEnhancement &GetNullMod (void) { return m_NullMod; }
		ICCItem *GetProperty (CCodeChainCtx *pCCCtx, CItemCtx &Ctx, const CString &sName) const;
		CString GetReference (CItemCtx &Ctx, int iVariant = -1, DWORD dwFlags = 0) const;
		bool GetReferenceDamageAdj (CSpaceObject *pInstalled, DWORD dwFlags, int *retiHP, int *retArray) const;
		bool GetReferenceDamageType (CSpaceObject *pInstalled, int iVariant, DWORD dwFlags, DamageTypes *retiDamage, CString *retsReference) const;
		CString GetNounPhrase (DWORD dwFlags) const;
		int GetTradePrice (CSpaceObject *pObj, bool bActual = false) const;
		inline CItemType *GetType (void) const { return m_pItemType; }
		int GetValue (bool bActual = false) const;
		inline bool HasMods (void) const { return (m_pExtra && m_pExtra->m_Mods.IsNotEmpty()); }
		bool HasSpecialAttribute (const CString &sAttrib) const;
		inline bool IsDamaged (void) const { return (m_dwFlags & flagDamaged ? true : false); }
		inline bool IsDisrupted (void) const;
		inline bool IsDisrupted (DWORD dwNow) const { return (m_pExtra ? (m_pExtra->m_dwDisruptedTime >= dwNow) : false); }
		inline bool IsEnhanced (void) const { return (m_dwFlags & flagEnhanced ? true : false); }
		inline bool IsInstalled (void) const { return (m_dwInstalled != 0xff); }
		inline bool IsMarkedForDelete (void) { return (m_dwCount == 0xffff); }
		inline void MarkForDelete (void) { m_dwCount = 0xffff; }
		bool RemoveEnhancement (DWORD dwID);
		inline void SetCharges (int iCharges) { Extra(); m_pExtra->m_dwCharges = iCharges; }
		inline void SetCount (int iCount) { m_dwCount = (DWORD)iCount; }
		inline void SetDamaged (void) { m_dwFlags |= flagDamaged; }
		inline void SetDamaged (bool bDamaged) { ClearDamaged(); if (bDamaged) SetDamaged(); }
		inline void SetData (const CString &sAttrib, const CString &sData) { Extra(); m_pExtra->m_Data.SetData(sAttrib, sData); }
		void SetDisrupted (DWORD dwDuration);
		inline void SetEnhanced (void) { m_dwFlags |= flagEnhanced; }
		inline void SetEnhanced (bool bEnhanced) { ClearEnhanced(); if (bEnhanced) SetEnhanced(); }
		inline void SetInstalled (int iInstalled) { m_dwInstalled = (BYTE)(char)iInstalled; }
		bool SetProperty (CItemCtx &Ctx, const CString &sName, ICCItem *pValue, CString *retsError);

		static CString GenerateCriteria (const CItemCriteria &Criteria);
		static void InitCriteriaAll (CItemCriteria *retCriteria);
		static const CItem &NullItem (void) { return CItem::m_NullItem; }
		static void ParseCriteria (const CString &sCriteria, CItemCriteria *retCriteria);
		bool MatchesCriteria (const CItemCriteria &Criteria) const;

		void ReadFromStream (SLoadCtx &Ctx);
		void WriteToStream (IWriteStream *pStream);

		void ReadFromCCItem (CCodeChain &CC, ICCItem *pBuffer);
		ICCItem *WriteToCCItem (CCodeChain &CC) const;

	private:
		enum PFlags
			{
			flagDamaged =		0x01,			//	Item is damaged

			flagEnhanced =		0x10,			//	Item is enhanced (Mod 1)
			flagMod2 =			0x20,			//	Mod 2
			flagMod3 =			0x40,			//	Mod 3
			flagMod4 =			0x80,			//	Mod 4
			};

		struct SExtra
			{
			DWORD m_dwCharges;					//	Charges for items
			DWORD m_dwVariant;					//	Affects stats based on type (e.g., for armor, this is maxHP)
			DWORD m_dwDisruptedTime;			//	If >0, the tick on which disruption expires

			CItemEnhancement m_Mods;			//	Class-specific modifications (e.g., armor enhancements)

			CAttributeDataBlock m_Data;			//	Opaque data
			};

		void Extra (void);
		static bool IsExtraEmpty (const SExtra *pExtra);
		bool IsExtraEqual (SExtra *pSrc) const;
		DWORD Mod2Flags (int iMod) const;

		CItemType *m_pItemType;

		DWORD m_dwCount:16;						//	Number of items
		DWORD m_dwFlags:8;						//	Miscellaneous flags
		DWORD m_dwInstalled:8;					//	Location where item is installed

		SExtra *m_pExtra;						//	Extra data (may be NULL)

		static CItem m_NullItem;
		static CItemEnhancement m_NullMod;
	};

class CItemList
	{
	public:
		CItemList (void);
		CItemList (const CItemList &Src);
		~CItemList (void);

		CItemList &operator= (const CItemList &Src);

		void AddItem (const CItem &Item);
		void DeleteAll (void);
		void DeleteItem (int iIndex);
		inline int GetCount (void) const { return m_List.GetCount(); }
		inline CItem &GetItem (int iIndex) { return *m_List[iIndex]; }
		inline const CItem &GetItem (int iIndex) const { return *m_List[iIndex]; }
		void ReadFromStream (SLoadCtx &Ctx);
		void SortItems (void);
		void WriteToStream (IWriteStream *pStream);

	private:
		void Copy (const CItemList &Src);

		TArray<CItem *> m_List;
	};

class CItemListManipulator
	{
	public:
		CItemListManipulator (CItemList &ItemList);
		~CItemListManipulator (void);

		void AddItem (const CItem &Item);
		void AddItems (const CItemList &ItemList);

		inline int GetCount (void) { return m_ViewMap.GetCount(); }
		inline int GetCursor (void) { return m_iCursor; }
		inline void SetCursor (int iCursor) { m_iCursor = Min(Max(-1, iCursor), GetCount() - 1); }
		bool SetCursorAtItem (const CItem &Item);
		void SetFilter (const CItemCriteria &Filter);
		bool Refresh (const CItem &Item);

		inline bool IsCursorValid (void) { return (m_iCursor != -1 && m_iCursor < m_ItemList.GetCount()); }
		bool MoveCursorBack (void);
		bool MoveCursorForward (void);
		void ResetCursor (void);
		void SyncCursor (void);

		DWORD AddItemEnhancementAtCursor (const CItemEnhancement &Mods, int iCount = 1);
		void ClearDisruptedAtCursor (int iCount = 1);
		void DeleteAtCursor (int iCount);
		void DeleteMarkedItems (void);
		const CItem &GetItemAtCursor (void);
		CItem *GetItemPointerAtCursor (void);
		void MarkDeleteAtCursor (int iCount);
		void RemoveItemEnhancementAtCursor (DWORD dwID, int iCount = 1);
		void SetChargesAtCursor (int iCharges, int iCount = 1);
		void SetCountAtCursor (int iCount);
		void SetDamagedAtCursor (bool bDamaged, int iCount = 1);
		void SetDataAtCursor (const CString &sAttrib, const CString &sData, int iCount = 1);
		void SetDisruptedAtCursor (DWORD dwDuration, int iCount = 1);
		void SetEnhancedAtCursor (bool bEnhanced);
		void SetInstalledAtCursor (int iInstalled);
		bool SetPropertyAtCursor (CSpaceObject *pSource, const CString &sName, ICCItem *pValue, int iCount, CString *retsError);
		void TransferAtCursor (int iCount, CItemList &DestList);

	private:
		int FindItem (const CItem &Item);
		void GenerateViewMap (void);
		void MoveItemTo (const CItem &NewItem, const CItem &OldItem);

		CItemList &m_ItemList;
		int m_iCursor;
		CIntArray m_ViewMap;

		bool m_bUseFilter;
		CItemCriteria m_Filter;
	};

enum DeviceNames
	{
	devNone = -1,

	devFirstName = 0,

	devPrimaryWeapon = 0,
	devMissileWeapon = 1,

	devShields = 8,
	devDrive = 9,
	devCargo = 10,
	devReactor = 11,

	devNamesCount = 12
	};

struct DriveDesc
	{
	DWORD dwUNID;								//	UNID source (either ship class or device)
	Metric rMaxSpeed;							//	Max speed (Km/sec)
	int iThrust;								//	Thrust (GigaNewtons--gasp!)
	int iPowerUse;								//	Power used while thrusting (1/10 megawatt)

	DWORD fInertialess:1;						//	Inertialess drive
	DWORD dwSpare:31;
	};

struct ReactorDesc
	{
	ReactorDesc (void) : 
			pFuelCriteria(NULL),
			fFreeFuelCriteria(false)
		{ }

	~ReactorDesc (void)
		{
		if (pFuelCriteria && fFreeFuelCriteria)
			delete pFuelCriteria;
		}

	ReactorDesc &operator= (const ReactorDesc &Src)
		{
		iMaxPower = Src.iMaxPower;
		iMaxFuel = Src.iMaxFuel;
		iPowerPerFuelUnit = Src.iPowerPerFuelUnit;

		pFuelCriteria = (Src.pFuelCriteria ? new CItemCriteria(*Src.pFuelCriteria) : NULL);
		iMinFuelLevel = Src.iMinFuelLevel;
		iMaxFuelLevel = Src.iMaxFuelLevel;

		fDamaged = Src.fDamaged;
		fEnhanced = Src.fEnhanced;
		fFreeFuelCriteria = (pFuelCriteria != NULL);

		return *this;
		}

	int iMaxPower;								//	Maximum power output
	int iMaxFuel;								//	Maximum fuel space
	int iPowerPerFuelUnit;						//	MW/10-tick per fuel unit

	CItemCriteria *pFuelCriteria;
	int iMinFuelLevel;							//	Min tech level of fuel (-1 if using fuelCriteria)
	int iMaxFuelLevel;							//	Max tech level of fuel (-1 if using fuelCriteria)

	DWORD fDamaged:1;							//	TRUE if damaged
	DWORD fEnhanced:1;							//	TRUE if enhanced
	DWORD fFreeFuelCriteria:1;					//	TRUE if we own pFuelCriteria
	DWORD dwSpare:29;
	};

class CInstalledArmor
	{
	public:
		inline EDamageResults AbsorbDamage (CSpaceObject *pSource, SDamageCtx &Ctx);
		void FinishInstall (CSpaceObject *pSource);
		inline CArmorClass *GetClass (void) const { return m_pArmorClass; }
		inline int GetDamageEffectiveness (CSpaceObject *pAttacker, CInstalledDevice *pWeapon);
		inline int GetHitPoints (void) const { return m_iHitPoints; }
		inline int GetMaxHP (CSpaceObject *pSource);
		inline const CItemEnhancement &GetMods (void) { return m_pItem->GetMods(); }
		inline int GetSect (void) const { return m_iSect; }
		inline int IncHitPoints (int iChange) { m_iHitPoints += iChange; return m_iHitPoints; }
		void Install (CSpaceObject *pObj, CItemListManipulator &ItemList, int iSect, bool bInCreate = false);
		inline bool IsComplete (void) const { return (m_fComplete ? true : false); }
		void SetComplete (CSpaceObject *pSource, bool bComplete = true);
		inline void SetHitPoints (int iHP) { m_iHitPoints = iHP; }
		inline void SetSect (int iSect) { m_iSect = iSect; }
		void ReadFromStream (CSpaceObject *pSource, int iSect, SLoadCtx &Ctx);
		void WriteToStream (IWriteStream *pStream);

	private:
		CItem *m_pItem;								//	Item
		CArmorClass *m_pArmorClass;					//	Armor class used
		int m_iHitPoints;							//	Hit points left

		int m_iSect:16;								//	Armor section
		int m_iSpare:16;

		DWORD m_fComplete:1;						//	All armor segments the same
		DWORD m_fSpare:31;
	};

class CArmorClass : public CObject
	{
	public:
		enum ECachedHandlers
			{
			evtGetMaxHP					= 0,
			evtOnArmorDamage			= 1,

			evtCount					= 2,
			};

		EDamageResults AbsorbDamage (CItemCtx &ItemCtx, SDamageCtx &Ctx);
		void AddTypesUsed (TSortMap<DWORD, bool> *retTypesUsed);
		void CalcAdjustedDamage (CItemCtx &ItemCtx, SDamageCtx &Ctx);
		int CalcAverageRelativeDamageAdj (void);
		int CalcBalance (void);
		void CalcDamageEffects (CItemCtx &ItemCtx, SDamageCtx &Ctx);
		int CalcPowerUsed (CInstalledArmor *pArmor);
		static ALERROR CreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, CItemType *pType, CArmorClass **retpArmor);
		bool FindDataField (const CString &sField, CString *retsValue);
		inline bool FindEventHandlerArmorClass (ECachedHandlers iEvent, SEventHandlerDesc *retEvent = NULL) const { if (retEvent) *retEvent = m_CachedEvents[iEvent]; return (m_CachedEvents[iEvent].pCode != NULL); }
		inline int GetBlindingDamageAdj (void) { return m_iBlindingDamageAdj; }
		inline int GetCompleteBonus (void) { return m_iArmorCompleteBonus; }
		inline int GetDamageAdj (DamageTypes iDamage) { return m_DamageAdj.GetAdj(iDamage); }
		int GetDamageAdjForWeaponLevel (int iLevel);
		int GetDamageEffectiveness (CSpaceObject *pAttacker, CInstalledDevice *pWeapon);
		inline int GetDeviceDamageAdj (void) { return m_iDeviceDamageAdj; }
		inline int GetEMPDamageAdj (void) { return m_iEMPDamageAdj; }
		inline int GetInstallCost (void) { return m_iInstallCost; }
		ICCItem *GetItemProperty (CItemCtx &Ctx, const CString &sName);
		inline CItemType *GetItemType (void) { return m_pItemType; }
		int GetMaxHP (CItemCtx &ItemCtx, bool bForceComplete = false);
		inline int GetMaxHPBonus (void) const { return m_iMaxHPBonus; }
		inline CString GetName (void);
		CString GetReference (CItemCtx &Ctx, int iVariant = -1);
		bool GetReferenceDamageAdj (const CItem *pItem, CSpaceObject *pInstalled, int *retiHP, int *retArray);
		inline int GetRepairCost (void) { return m_iRepairCost; }
		inline int GetRepairTech (void) { return m_iRepairTech; }
		CString GetShortName (void);
		inline int GetStealth (void) const { return m_iStealth; }
		inline DWORD GetUNID (void);
		inline bool IsBlindingDamageImmune (CItemCtx &ItemCtx);
		inline bool IsDeviceDamageImmune (CItemCtx &ItemCtx);
		inline bool IsDisintegrationImmune (CItemCtx &ItemCtx);
		inline bool IsEMPDamageImmune (CItemCtx &ItemCtx);
		inline bool IsRadiationImmune (CItemCtx &ItemCtx);
		bool IsReflective (CItemCtx &ItemCtx, const DamageDesc &Damage);
		inline bool IsShatterImmune (CItemCtx &ItemCtx);
		inline bool IsShieldInterfering (CItemCtx &ItemCtx);
		ALERROR OnBindDesign (SDesignLoadCtx &Ctx);
		void Update (CInstalledArmor *pArmor, CSpaceObject *pObj, int iTick, bool *retbModified);

		static int GetStdCost (int iLevel);
		static int GetStdDamageAdj (int iLevel, DamageTypes iDamage);
		static int GetStdEffectiveHP (int iLevel);
		static int GetStdHP (int iLevel);
		static int GetStdMass (int iLevel);

	private:
		CArmorClass (void);

		int CalcArmorDamageAdj (const DamageDesc &Damage) const;
		int GetDamageAdj (CItemEnhancement Mods, const DamageDesc &Damage);
		int FireGetMaxHP (CItemCtx &ItemCtx, int iMaxHP) const;
		void FireOnArmorDamage (CItemCtx &ItemCtx, SDamageCtx &Ctx);

		int m_iHitPoints;						//	Hit points for this armor class
		int m_iRepairCost;						//	Cost per HP to repair
		int m_iInstallCost;						//	Cost to install
		int m_iRepairTech;						//	Tech required to repair
		int m_iArmorCompleteBonus;				//	Extra HP if armor is complete
		int m_iStealth;							//	Stealth level
		int m_iPowerUse;						//	Power consumed (1/10 MWs)
		int m_iMaxHPBonus;						//	Max HP bonus allowed for this armor
		CRegenDesc m_Regen;						//	Regeneration desc
		CRegenDesc m_Decay;						//	Decay desc
		
		int m_iDamageAdjLevel;					//	Level to use for intrinsic damage adj
		CDamageAdjDesc m_DamageAdj;				//	Adjustments for damage type
		DamageTypeSet m_Reflective;				//	Types of damage reflected
		int m_iEMPDamageAdj;					//	Adjust for EMP damage
		int m_iBlindingDamageAdj;				//	Adjust for blinding damage
		int m_iDeviceDamageAdj;					//	Adjust for device damage

		DWORD m_fPhotoRepair:1;					//	TRUE if repairs when near a star
		DWORD m_fRadiationImmune:1;				//	TRUE if immune to radiation
		DWORD m_fPhotoRecharge:1;				//	TRUE if refuels when near a star
		DWORD m_fShieldInterference:1;			//	TRUE if armor interferes with shields
		DWORD m_fDisintegrationImmune:1;		//	TRUE if immune to disintegration
		DWORD m_fShatterImmune:1;				//	TRUE if immune to shatter
		DWORD m_fSpare7:1;
		DWORD m_fSpare8:1;

		DWORD m_dwSpare:24;

		CItemType *m_pItemType;					//	Item for this armor

		SEventHandlerDesc m_CachedEvents[evtCount];

	friend CObjectClass<CArmorClass>;
	};

class CDeviceClass : public CObject
	{
	public:
		enum CounterTypes
			{
			cntNone,							//	No counter
			cntTemperature,						//	Current device temperature (0-100)
			cntRecharge,						//	Current recharge level (0-100)
			cntCapacitor,						//	Current capacitor level (0-100)
			};

		enum DeviceNotificationTypes
			{
			statusDisruptionRepaired,

			failDamagedByDisruption,
			failDeviceHitByDamage,
			failDeviceHitByDisruption,
			failDeviceOverheat,
			failShieldFailure,
			failWeaponExplosion,
			failWeaponJammed,
			failWeaponMisfire,
			};

		enum LinkedFireOptions
			{
			lkfAlways =				0x0000001,	//	Linked to fire button
			lkfTargetInRange =		0x0000002,	//	Fire only if the target is in range
			lkfEnemyInRange =		0x0000004,	//	Fire only an enemy is in range
			};

		enum ECachedHandlers
			{
			evtGetOverlayType			= 0,

			evtCount					= 1,
			};

		enum Flags
			{
			flagNoPowerReference =		0x00000001,
			};

		CDeviceClass (IObjectClass *pClass) : CObject(pClass), m_pItemType(NULL) { }
		virtual ~CDeviceClass (void) { }

		bool AccumulateEnhancements (CItemCtx &Device, CInstalledDevice *pTarget, TArray<CString> &EnhancementIDs, CItemEnhancementStack *pEnhancements);
		void AddTypesUsed (TSortMap<DWORD, bool> *retTypesUsed);
		ALERROR Bind (SDesignLoadCtx &Ctx);
		inline CEffectCreator *FindEffectCreator (const CString &sUNID) { return OnFindEffectCreator(sUNID); }
		inline bool FindEventHandlerDeviceClass (ECachedHandlers iEvent, SEventHandlerDesc *retEvent = NULL) const { if (retEvent) *retEvent = m_CachedEvents[iEvent]; return (m_CachedEvents[iEvent].pCode != NULL); }
		COverlayType *FireGetOverlayType(CItemCtx &Ctx) const;
		inline CString GetDataField (const CString &sField) { CString sValue; FindDataField(sField, &sValue); return sValue; }
		inline int GetDataFieldInteger (const CString &sField) { CString sValue; if (FindDataField(sField, &sValue)) return strToInt(sValue, 0, NULL); else return 0; }
		inline CItemType *GetItemType (void) { return m_pItemType; }
		inline int GetLevel (void) const;
		inline int GetMaxHPBonus (void) const { return m_iMaxHPBonus; }
		inline CString GetName (void);
		inline COverlayType *GetOverlayType(void) const { return m_pOverlayType; }
		CString GetReferencePower (CItemCtx &Ctx);
		inline ItemCategories GetSlotCategory (void) const { return (m_iSlotCategory == itemcatNone ? GetCategory() : m_iSlotCategory); }
		inline int GetSlotsRequired (void) { return m_iSlots; }
		inline DWORD GetUNID (void);
		inline void MarkImages (void) { OnMarkImages(); }

		virtual bool AbsorbDamage (CInstalledDevice *pDevice, CSpaceObject *pShip, SDamageCtx &Ctx) { Ctx.iAbsorb = 0; return false; }
		virtual bool AbsorbsWeaponFire (CInstalledDevice *pDevice, CSpaceObject *pSource, CInstalledDevice *pWeapon) { return false; }
		virtual bool Activate (CInstalledDevice *pDevice, 
							   CSpaceObject *pSource, 
							   CSpaceObject *pTarget,
							   bool *retbSourceDestroyed,
							   bool *retbConsumedItems = NULL) { return false; }
		virtual CWeaponClass *AsWeaponClass (void) { return NULL; }
		virtual int CalcFireSolution (CInstalledDevice *pDevice, CSpaceObject *pSource, CSpaceObject *pTarget) { return -1; }
		virtual int CalcPowerUsed (CInstalledDevice *pDevice, CSpaceObject *pSource) { return 0; }
		virtual bool CanBeDamaged (void) { return true; }
		virtual bool CanBeDisabled (CItemCtx &Ctx) { return (GetPowerRating(Ctx) != 0); }
		virtual bool CanHitFriends (void) { return true; }
		virtual bool CanRotate (CItemCtx &Ctx, int *retiMinFireArc = NULL, int *retiMaxFireArc = NULL) { return false; }
		virtual void Deplete (CInstalledDevice *pDevice, CSpaceObject *pSource) { }
		virtual bool FindDataField (const CString &sField, CString *retsValue) { return false; }
		virtual bool FindDataField (int iVariant, const CString &sField, CString *retsValue) { return false; }
		virtual int GetActivateDelay (CInstalledDevice *pDevice, CSpaceObject *pSource) { return 0; }
		virtual int GetAmmoVariant (const CItemType *pItem) const { return -1; }
		virtual int GetCargoSpace (void) { return 0; }
		virtual ItemCategories GetCategory (void) const = 0;
		virtual int GetCounter (CInstalledDevice *pDevice, CSpaceObject *pSource, CounterTypes *retiType = NULL) { return 0; }
		virtual const DamageDesc *GetDamageDesc (CItemCtx &Ctx) { return NULL; }
		virtual int GetDamageEffectiveness (CSpaceObject *pAttacker, CInstalledDevice *pWeapon) { return 0; }
		virtual int GetDamageType (CInstalledDevice *pDevice = NULL, int iVariant = -1) { return damageGeneric; }
		virtual int GetDefaultFireAngle (CInstalledDevice *pDevice, CSpaceObject *pSource) { return 0; }
		virtual bool GetDeviceEnhancementDesc (CInstalledDevice *pDevice, CSpaceObject *pSource, CInstalledDevice *pWeapon, SDeviceEnhancementDesc *retDesc) { return false; }
		virtual const DriveDesc *GetDriveDesc (CInstalledDevice *pDevice = NULL, CSpaceObject *pSource = NULL) { return NULL; }
		virtual ICCItem *GetItemProperty (CItemCtx &Ctx, const CString &sName);
		virtual DWORD GetLinkedFireOptions (CItemCtx &Ctx) { return 0; }
		virtual Metric GetMaxEffectiveRange (CSpaceObject *pSource, CInstalledDevice *pDevice, CSpaceObject *pTarget) { return 0.0; }
		virtual int GetPowerRating (CItemCtx &Ctx) { return 0; }
		virtual const ReactorDesc *GetReactorDesc (CInstalledDevice *pDevice = NULL, CSpaceObject *pSource = NULL) { return NULL; }
		virtual CString GetReference (CItemCtx &Ctx, int iVariant = -1, DWORD dwFlags = 0);
		virtual bool GetReferenceDamageAdj (const CItem *pItem, CSpaceObject *pInstalled, int *retiHP, int *retArray) const { return false; }
		virtual bool GetReferenceDamageType (CItemCtx &Ctx, int iVariant, DamageTypes *retiDamage, CString *retsReference) const { return false; }
		virtual void GetSelectedVariantInfo (CSpaceObject *pSource, 
											 CInstalledDevice *pDevice,
											 CString *retsLabel,
											 int *retiAmmoLeft,
											 CItemType **retpType = NULL) { if (retsLabel) *retsLabel = NULL_STR; if (retiAmmoLeft) *retiAmmoLeft = -1; if (retpType) *retpType = NULL; }
		virtual void GetStatus (CInstalledDevice *pDevice, CSpaceObject *pSource, int *retiStatus, int *retiMaxStatus) { *retiStatus = 0; *retiMaxStatus = 0; }
		virtual int GetValidVariantCount (CSpaceObject *pSource, CInstalledDevice *pDevice) { return 0; }
		virtual int GetWeaponEffectiveness (CSpaceObject *pSource, CInstalledDevice *pDevice, CSpaceObject *pTarget) { return 0; }
		virtual bool IsAmmoWeapon (void) { return false; }
		virtual bool IsAreaWeapon (CSpaceObject *pSource, CInstalledDevice *pDevice) { return false; }
		virtual bool IsAutomatedWeapon (void) { return false; }
		virtual bool IsExternal (void) { return (m_fExternal ? true : false); }
		virtual bool IsFuelCompatible (CItemCtx &Ctx, const CItem &FuelItem) { return false; }
		virtual bool IsTrackingWeapon (CItemCtx &Ctx) { return false; }
		virtual bool IsVariantSelected (CSpaceObject *pSource, CInstalledDevice *pDevice) { return true; }
		virtual bool IsWeaponAligned (CSpaceObject *pShip, CInstalledDevice *pDevice, CSpaceObject *pTarget, int *retiAimAngle = NULL, int *retiFireAngle = NULL) { return false; }
		virtual bool NeedsAutoTarget (CItemCtx &Ctx, int *retiMinFireArc = NULL, int *retiMaxFireArc = NULL) { return false; }
		virtual void OnInstall (CInstalledDevice *pDevice, CSpaceObject *pSource, CItemListManipulator &ItemList) { }
		virtual void OnUninstall (CInstalledDevice *pDevice, CSpaceObject *pSource, CItemListManipulator &ItemList) { }
		virtual void Recharge (CInstalledDevice *pDevice, CShip *pShip, int iStatus) { }
		virtual bool RequiresItems (void) { return false; }
		virtual void Reset (CInstalledDevice *pDevice, CSpaceObject *pSource) { }
		virtual bool SelectFirstVariant (CSpaceObject *pSource, CInstalledDevice *pDevice) { return false; }
		virtual bool SelectNextVariant (CSpaceObject *pSource, CInstalledDevice *pDevice, int iDir = 1) { return false; }
		virtual bool SetItemProperty (CItemCtx &Ctx, const CString &sName, ICCItem *pValue, CString *retsError);
		virtual bool ShowActivationDelayCounter (CSpaceObject *pSource, CInstalledDevice *pDevice) { return false; }
		virtual void Update (CInstalledDevice *pDevice, 
							 CSpaceObject *pSource, 
							 int iTick,
							 bool *retbSourceDestroyed,
							 bool *retbConsumedItems = NULL) { }
		virtual bool ValidateSelectedVariant (CSpaceObject *pSource, CInstalledDevice *pDevice) { return false; }

		static bool FindAmmoDataField (CItemType *pItem, const CString &sField, CString *retsValue);
		static bool FindWeaponFor (CItemType *pItem, CDeviceClass **retpWeapon = NULL, int *retiVariant = NULL, CWeaponFireDesc **retpDesc = NULL);
		static ItemCategories GetItemCategory (DeviceNames iDev);
		static CString GetLinkedFireOptionString (DWORD dwOptions);
		static ALERROR ParseLinkedFireOptions (SDesignLoadCtx &Ctx, const CString &sDesc, DWORD *retdwOptions);

	protected:
		inline ItemCategories GetDefinedSlotCategory (void) { return m_iSlotCategory; }
		ALERROR InitDeviceFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, CItemType *pType);

		virtual bool OnAccumulateEnhancements (CItemCtx &Device, CInstalledDevice *pTarget, TArray<CString> &EnhancementIDs, CItemEnhancementStack *pEnhancements) { return false; }
		virtual void OnAddTypesUsed (TSortMap<DWORD, bool> *retTypesUsed) { }
		virtual ALERROR OnDesignLoadComplete (SDesignLoadCtx &Ctx) { return NOERROR; }
		virtual CEffectCreator *OnFindEffectCreator (const CString &sUNID) { return NULL; }
		virtual void OnMarkImages (void) { }

	private:
		struct SEnhancerDesc
			{
			CString sType;						//	Type of enhancement
			CItemCriteria Criteria;				//	Items that we enhance
			CItemEnhancement Enhancement;		//	Enhancement confered
			};

		CItemType *m_pItemType;					//	Item for device
		int m_iSlots;							//	Number of device slots required
		ItemCategories m_iSlotCategory;			//	Count as this category (for device slot purposes)

		COverlayTypeRef m_pOverlayType;			//	Associated overlay (may be NULL)

		int m_iMaxHPBonus;						//	Max HP bonus for this device
		TArray<SEnhancerDesc> m_Enhancements;	//	Enhancements confered on other items

		SEventHandlerDesc m_CachedEvents[evtCount];	//	Cached events

		DWORD m_fExternal:1;					//	Device is external
		DWORD m_dwSpare:31;
	};

class CRandomEnhancementGenerator
	{
	public:
		CRandomEnhancementGenerator (void) : m_iChance(0), m_dwMods(0), m_pCode(NULL) { }
		~CRandomEnhancementGenerator (void);

		CRandomEnhancementGenerator &operator= (const CRandomEnhancementGenerator &Src);

		void EnhanceItem (CItem &Item);
		inline int GetChance (void) const { return m_iChance; }
		ALERROR InitFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);

	private:
		int m_iChance;
		DWORD m_dwMods;
		ICCItem *m_pCode;
	};

//	IItemGenerator

struct SItemAddCtx
	{
	SItemAddCtx (CItemListManipulator &theItemList) : 
			ItemList(theItemList),
			pSystem(NULL),
			iLevel(1)
		{ }

	CItemListManipulator &ItemList;				//	Item list to add items to

	CSystem *pSystem;							//	System where we're creating items
	CVector vPos;								//	Position to use (for LocationCriteriaTable)
	int iLevel;									//	Level to use for item create (for LevelTable)
	};

class IItemGenerator
	{
	public:
		static ALERROR CreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, IItemGenerator **retpGenerator);
		static ALERROR CreateLookupTable (SDesignLoadCtx &Ctx, DWORD dwUNID, IItemGenerator **retpGenerator);
		static ALERROR CreateRandomItemTable (const CItemCriteria &Crit, 
											  const CString &sLevelFrequency,
											  IItemGenerator **retpGenerator);

		virtual ~IItemGenerator (void) { }
		virtual void AddItems (SItemAddCtx &Ctx) { }
		virtual void AddTypesUsed (TSortMap<DWORD, bool> *retTypesUsed) { }
		virtual CurrencyValue GetAverageValue (int iLevel) { return 0; }
		virtual IItemGenerator *GetGenerator (int iIndex) { return NULL; }
		virtual int GetGeneratorCount (void) { return 0; }
		virtual CItemType *GetItemType (int iIndex) { return NULL; }
		virtual int GetItemTypeCount (void) { return 0; }
		virtual ALERROR LoadFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc) { return NOERROR; }
		virtual ALERROR OnDesignLoadComplete (SDesignLoadCtx &Ctx) { return NOERROR; }
	};

//	IDeviceGenerator

struct SDeviceDesc
	{
	SDeviceDesc (void) :
			iPosAngle(0),
			iPosRadius(0),
			iPosZ(0),
			b3DPosition(false),
			bExternal(false),
			bOmnidirectional(false),
			iMinFireArc(0),
			iMaxFireArc(0),
			bSecondary(false),
			dwLinkedFireOptions(0),
			iSlotBonus(0)
		{ }

	CItem Item;

	int iPosAngle;
	int iPosRadius;
	int iPosZ;
	bool b3DPosition;
	bool bExternal;

	bool bOmnidirectional;
	int iMinFireArc;
	int iMaxFireArc;
	bool bSecondary;

	DWORD dwLinkedFireOptions;

	int iSlotBonus;

	CItemList ExtraItems;
	};

class CDeviceDescList
	{
	public:
		CDeviceDescList (void);
		~CDeviceDescList (void);

		void AddDeviceDesc (const SDeviceDesc &Desc);
		inline int GetCount (void) const { return m_List.GetCount(); }
		inline CDeviceClass *GetDeviceClass (int iIndex) const;
		inline const SDeviceDesc &GetDeviceDesc (int iIndex) const { return m_List[iIndex]; }
		CDeviceClass *GetNamedDevice (DeviceNames iDev) const;
		void RemoveAll (void);

	private:
		TArray<SDeviceDesc> m_List;
	};

struct SDeviceGenerateCtx
	{
	SDeviceGenerateCtx (void) :
			iLevel(1),
			pRoot(NULL),
			pResult(NULL)
		{ }

	int iLevel;
	IDeviceGenerator *pRoot;

	CDeviceDescList *pResult;
	};

class IDeviceGenerator
	{
	public:
		static ALERROR CreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, IDeviceGenerator **retpGenerator);

		virtual ~IDeviceGenerator (void) { }
		virtual void AddDevices (SDeviceGenerateCtx &Ctx) { }
		virtual void AddTypesUsed (TSortMap<DWORD, bool> *retTypesUsed) { }
		virtual IDeviceGenerator *GetGenerator (int iIndex) { return NULL; }
		virtual int GetGeneratorCount (void) { return 0; }
		virtual ALERROR LoadFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc) { return NOERROR; }
		virtual ALERROR OnDesignLoadComplete (SDesignLoadCtx &Ctx) { return NOERROR; }

		virtual bool FindDefaultDesc (const CItem &Item, SDeviceDesc *retDesc) { return false; }

		static ALERROR InitDeviceDescFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, SDeviceDesc *retDesc);
	};

//	Object properties

enum ScaleTypes
	{
	scaleNone =						-1,

	scaleStar =						0,
	scaleWorld =					1,
	scaleStructure =				2,
	scaleShip =						3,
	scaleFlotsam =					4,
	};

//	IShipGenerator

struct SShipCreateCtx
	{
	SShipCreateCtx (void) :
			pSystem(NULL),
			pGate(NULL),
			pBase(NULL),
			pBaseSovereign(NULL),
			pTarget(NULL),
			pEncounterInfo(NULL),
			pOverride(NULL),
			dwFlags(0)
		{ }

	CSystem *pSystem;							//	System to create ship in
	CSpaceObject *pGate;						//	Gate where ship will appear (may be NULL)
	CVector vPos;								//	Position where ship will appear (only if pGate is NULL)
	DiceRange PosSpread;						//	Distance from vPos (in light-seconds)
	CSpaceObject *pBase;						//	Base for this ship (may be NULL)
	CSovereign *pBaseSovereign;					//	Only if pBase is NULL
	CSpaceObject *pTarget;						//	Target for ship orders (may be NULL)
	CStationType *pEncounterInfo;				//	Encounter info (may be NULL)
	CDesignType *pOverride;						//	Override to apply to ships (may be NULL)

	DWORD dwFlags;								//	Flags

	CSpaceObjectList Result;					//	List of ships created

	enum Flags
		{
		SHIPS_FOR_STATION =			0x00000001,	//	Create ships at station creation time
		ATTACK_NEAREST_ENEMY =		0x00000002,	//	After all other orders, attack nearest enemy
		RETURN_RESULT =				0x00000004,	//	Add created ships to Result
		};
	};

class IShipGenerator
	{
	public:
		static ALERROR CreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, IShipGenerator **retpGenerator);
		static ALERROR CreateFromXMLAsGroup (SDesignLoadCtx &Ctx, CXMLElement *pDesc, IShipGenerator **retpGenerator);

		virtual ~IShipGenerator (void) { }
		virtual void AddTypesUsed (TSortMap<DWORD, bool> *retTypesUsed) { }
		virtual void CreateShips (SShipCreateCtx &Ctx) { }
		virtual Metric GetAverageLevelStrength (int iLevel) { return 0.0; }
		virtual ALERROR LoadFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc) { return NOERROR; }
		virtual ALERROR OnDesignLoadComplete (SDesignLoadCtx &Ctx) { return NOERROR; }
		virtual ALERROR ValidateForRandomEncounter (void) { return NOERROR; }
	};

//	Ship properties

enum ObjectComponentTypes
	{
	comArmor,
	comCargo,
	comShields,
	comWeapons,
	comDrive,
	comReactor,
	comDeviceCounter,					//	One or more devices need to show a counter
	};

struct SArmorSegmentImageDesc
	{
	CString sName;								//	Name of segment

	int xDest;									//	Coordinates of destination
	int yDest;
	int xHP;									//	Coordinates of HP display
	int yHP;
	int yName;									//	Coordinates of armor name
	int cxNameBreak;							//	Line break
	int xNameDestOffset;
	int yNameDestOffset;

	CObjectImageArray Image;					//	Image for armor damage
	};

struct SArmorImageDesc
	{
	CObjectImageArray ShipImage;				//	Image for ship (with no armor)

	TSortMap<int, SArmorSegmentImageDesc> Segments;
	};

struct SReactorImageDesc
	{
	CObjectImageArray ReactorImage;

	CObjectImageArray PowerLevelImage;
	int xPowerLevelImage;
	int yPowerLevelImage;

	CObjectImageArray FuelLevelImage;
	int xFuelLevelImage;
	int yFuelLevelImage;

	CObjectImageArray FuelLowLevelImage;

	RECT rcReactorText;
	RECT rcPowerLevelText;
	RECT rcFuelLevelText;
	};

struct SShieldImageDesc
	{
	CEffectCreatorRef pShieldEffect;			//	Effect for display shields HUD

	CObjectImageArray Image;					//	Image for shields
	};

struct SWeaponImageDesc
	{
	CObjectImageArray Image;					//	Background image
	};

class CPlayerSettings
	{
	public:
		CPlayerSettings (void) : 
				m_pArmorDescInherited(NULL),
				m_pReactorDescInherited(NULL),
				m_pShieldDescInherited(NULL),
				m_pWeaponDescInherited(NULL)
			{ }

		~CPlayerSettings (void) { CleanUp(); }

		CPlayerSettings &operator= (const CPlayerSettings &Source);

		void AddTypesUsed (TSortMap<DWORD, bool> *retTypesUsed);
		ALERROR Bind (SDesignLoadCtx &Ctx, CShipClass *pClass);
		CEffectCreator *FindEffectCreator (const CString &sUNID);
		inline const SArmorImageDesc &GetArmorDesc (void) const { return (m_fHasArmorDesc ? m_ArmorDesc : *m_pArmorDescInherited); }
		inline const SArmorSegmentImageDesc *GetArmorDesc (int iSegment) const { return (m_fHasArmorDesc ? m_ArmorDesc.Segments.GetAt(iSegment) : m_pArmorDescInherited->Segments.GetAt(iSegment)); }
		inline const CString &GetDesc (void) const { return m_sDesc; }
		inline const CDockScreenTypeRef &GetDockServicesScreen (void) const { return m_pDockServicesScreen; }
		inline DWORD GetLargeImage (void) const { return m_dwLargeImage; }
		inline const SReactorImageDesc &GetReactorDesc (void) const { return (m_fHasReactorDesc ? m_ReactorDesc : *m_pReactorDescInherited); }
		inline const SShieldImageDesc &GetShieldDesc (void) const { return (m_fHasShieldDesc ? m_ShieldDesc : *m_pShieldDescInherited); }
		inline const CDockScreenTypeRef &GetShipScreen (void) const { return m_pShipScreen; }
		inline const CCurrencyAndRange &GetStartingCredits (void) const { return m_StartingCredits; }
		inline DWORD GetStartingMap (void) const { return m_dwStartMap; }
		inline const CString &GetStartingNode (void) const { return m_sStartNode; }
		inline const CString &GetStartingPos (void) const { return m_sStartPos; }
		inline const SWeaponImageDesc *GetWeaponDesc (void) const { return (m_fHasWeaponDesc ? &m_WeaponDesc : m_pWeaponDescInherited); }
		ALERROR InitFromXML (SDesignLoadCtx &Ctx, CShipClass *pClass, CXMLElement *pDesc);
		inline bool IsDebugOnly (void) const { return (m_fDebug ? true : false); }
		inline bool IsIncludedInAllAdventures (void) const { return (m_fIncludeInAllAdventures ? true : false); }
		inline bool IsInitialClass (void) const { return (m_fInitialClass ? true : false); }
		void MergeFrom (const CPlayerSettings &Src);

		const SArmorImageDesc *GetArmorImageDescRaw (void) const { return (m_fHasArmorDesc ? &m_ArmorDesc : NULL); }
		const SReactorImageDesc *GetReactorImageDescRaw (void) const { return (m_fHasReactorDesc ? &m_ReactorDesc : NULL); }
		const SShieldImageDesc *GetShieldImageDescRaw (void) const { return (m_fHasShieldDesc ? &m_ShieldDesc : NULL); }
		const SWeaponImageDesc *GetWeaponImageDescRaw (void) const { return (m_fHasWeaponDesc ? &m_WeaponDesc : NULL); }

	private:
		void CleanUp (void);
		ALERROR ComposeLoadError (SDesignLoadCtx &Ctx, const CString &sError);
		int GetArmorSegment (SDesignLoadCtx &Ctx, CShipClass *pClass, CXMLElement *pDesc);

		CString m_sDesc;							//	Description
		DWORD m_dwLargeImage;						//	UNID of large image

		//	Miscellaneous
		CCurrencyAndRange m_StartingCredits;		//	Starting credits
		DWORD m_dwStartMap;							//	Starting map UNID
		CString m_sStartNode;						//	Starting node (may be blank)
		CString m_sStartPos;						//	Label of starting position (may be blank)
		CDockScreenTypeRef m_pShipScreen;			//	Ship screen
		CDockScreenTypeRef m_pDockServicesScreen;	//	Screen used for ship upgrades (may be NULL)

		//	Armor
		SArmorImageDesc m_ArmorDesc;
		const SArmorImageDesc *m_pArmorDescInherited;

		//	Shields
		SShieldImageDesc m_ShieldDesc;
		const SShieldImageDesc *m_pShieldDescInherited;

		//	Reactor
		SReactorImageDesc m_ReactorDesc;
		const SReactorImageDesc *m_pReactorDescInherited;

		//	Weapons
		SWeaponImageDesc m_WeaponDesc;
		const SWeaponImageDesc *m_pWeaponDescInherited;

		//	Flags
		DWORD m_fInitialClass:1;					//	Use ship class at game start
		DWORD m_fDebug:1;							//	Debug only
		DWORD m_fHasArmorDesc:1;					//	TRUE if m_pArmorDesc initialized
		DWORD m_fHasReactorDesc:1;					//	TRUE if m_ReactorDesc initialized
		DWORD m_fHasShieldDesc:1;					//	TRUE if m_ShieldDesc initialized
		DWORD m_fIncludeInAllAdventures:1;			//	TRUE if we should always include this ship
		DWORD m_fHasWeaponDesc:1;					//	TRUE if m_WeaponDesc initialized
		DWORD m_fSpare8:1;

		DWORD m_dwSpare:24;
	};

enum AICombatStyles
	{
	aicombatStandard =						0,	//	Normal dogfighting
	aicombatStandOff =						1,	//	Missile ship combat
	aicombatFlyby =							2,	//	Maximize relative speed wrt target
	aicombatNoRetreat =						3,	//	Do not turn away from target
	aicombatChase =							4,	//	Get in position behind the target
	aicombatAdvanced =						5,	//	Dogfight, keeping proper distance from target
	};

class CAISettings
	{
	public:
		CAISettings (void);

		inline bool AscendOnGate (void) const { return m_fAscendOnGate; }
		inline AICombatStyles GetCombatStyle (void) const { return m_iCombatStyle; }
		inline int GetFireAccuracy (void) const { return m_iFireAccuracy; }
		inline int GetFireRangeAdj (void) const { return m_iFireRangeAdj; }
		inline int GetFireRateAdj (void) const { return m_iFireRateAdj; }
		inline Metric GetMinCombatSeparation (void) const { return m_rMinCombatSeparation; }
		inline int GetPerception (void) const { return m_iPerception; }
		CString GetValue (const CString &sSetting);
		ALERROR InitFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);
		void InitToDefault (void);
		inline bool IsAggressor (void) const { return m_fAggressor; }
		inline bool IsFlocker (void) const { return m_fFlockFormation; }
		inline bool IsNonCombatant (void) const { return m_fNonCombatant; }
		inline bool NoAttackOnThreat (void) const { return m_fNoAttackOnThreat; }
		inline bool NoDogfights (void) const { return m_fNoDogfights; }
		inline bool NoFriendlyFire (void) const { return m_fNoFriendlyFire; }
		inline bool NoFriendlyFireCheck (void) const { return m_fNoFriendlyFireCheck; }
		inline bool NoNavPaths (void) const { return m_fNoNavPaths; }
		inline bool NoOrderGiver (void) const { return m_fNoOrderGiver; }
		inline bool NoShieldRetreat (void) const { return m_fNoShieldRetreat; }
		inline bool NoTargetsOfOpportunity (void) const { return m_fNoTargetsOfOpportunity; }
		void ReadFromStream (SLoadCtx &Ctx);
		inline void SetMinCombatSeparation (Metric rValue) { m_rMinCombatSeparation = rValue; }
		CString SetValue (const CString &sSetting, const CString &sValue);
		void WriteToStream (IWriteStream *pStream);

		static AICombatStyles ConvertToAICombatStyle (const CString &sValue);
		static CString ConvertToID (AICombatStyles iStyle);

	private:
		AICombatStyles m_iCombatStyle;			//	Combat style

		int m_iFireRateAdj;						//	Adjustment to weapon's fire rate (10 = normal; 20 = double delay)
		int m_iFireRangeAdj;					//	Adjustment to range (100 = normal; 50 = half range)
		int m_iFireAccuracy;					//	Percent chance of hitting
		int m_iPerception;						//	Perception

		Metric m_rMinCombatSeparation;			//	Min separation from other ships while in combat

		DWORD m_fNoShieldRetreat:1;				//	Ship does not retreat when shields go down
		DWORD m_fNoDogfights:1;					//	Don't turn ship to face target
		DWORD m_fNonCombatant:1;				//	Tries to stay out of trouble
		DWORD m_fNoFriendlyFire:1;				//	Cannot hit friends
		DWORD m_fAggressor:1;					//	Attack targets of opportunity even if they haven't attacked
		DWORD m_fNoFriendlyFireCheck:1;			//	Do not check to see if friends are in line of fire
		DWORD m_fNoOrderGiver:1;				//	Always treated as the decider
		DWORD m_fAscendOnGate:1;				//	If TRUE, we ascend when the ship gates out

		DWORD m_fNoNavPaths:1;					//	If TRUE, do not use nav paths
		DWORD m_fNoAttackOnThreat:1;			//	Do not attack enemies while escorting (unless ordered)
		DWORD m_fNoTargetsOfOpportunity:1;		//	If TRUE, do not attack targets of opportunity
		DWORD m_fFlockFormation:1;				//	If TRUE, flock with like ships
		DWORD m_fSpare5:1;
		DWORD m_fSpare6:1;
		DWORD m_fSpare7:1;
		DWORD m_fSpare8:1;

		DWORD m_dwSpare:16;
	};

enum ProgramTypes
	{
	progNOP,
	progShieldsDown,
	progReboot,
	progDisarm,

	progCustom,
	};

struct ProgramDesc
	{
	ProgramTypes iProgram;
	CString sProgramName;
	int iAILevel;

	//	Used for custom programs
	CEvalContext *pCtx;
	ICCItem *ProgramCode;
	};

struct STargetingCtx
	{
	STargetingCtx (void) :
			bRecalcTargets(true)
		{ }

	TArray<CSpaceObject *> Targets;
	bool bRecalcTargets;
	};

//	Ship Structure and Compartments --------------------------------------------

enum ECompartmentTypes
	{
	deckUnknown =						-1,

	deckGeneral =						0,	//	General interior compartment or deck
	deckMainDrive =						1,	//	Main drive
	deckCargo =							2,	//	Cargo hold
	};

struct SCompartmentDesc
	{
	SCompartmentDesc (void) :
			iType(deckUnknown),
			iMaxHP(0),
			fDefault(false)
		{
		rcPos.left = 0;
		rcPos.top = 0;
		rcPos.right = 0;
		rcPos.bottom = 0;
		}

	CString sName;							//	User-visible name (e.g., "bridge")
	ECompartmentTypes iType;				//	Type of compartment
	int iMaxHP;								//	Initial HP
	RECT rcPos;								//	Position and size relative to image

	DWORD fDefault:1;						//	Default compartment (any space not used by another compartment)
	};

class CShipInteriorDesc
	{
	public:
		ALERROR BindDesign (SDesignLoadCtx &Ctx);
		inline int GetCount (void) const { return m_Compartments.GetCount(); }
		inline const SCompartmentDesc &GetCompartment (int iIndex) const { return m_Compartments[iIndex]; }
		int GetHitPoints (void) const;
		ALERROR InitFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);
		inline bool IsEmpty (void) const { return m_Compartments.GetCount() == 0; }

	private:
		TArray<SCompartmentDesc> m_Compartments;
	};

class CShipInterior
	{
	public:
		EDamageResults Damage (CShip *pShip, const CShipInteriorDesc &Desc, SDamageCtx &Ctx);
		void GetHitPoints (CShip *pShip, const CShipInteriorDesc &Desc, int *retiHP, int *retiMaxHP = NULL) const;
		void Init (const CShipInteriorDesc &Desc);
		inline bool IsEmpty (void) const { return m_Compartments.GetCount() == 0; }
		void ReadFromStream (CShip *pShip, const CShipInteriorDesc &Desc, SLoadCtx &Ctx);
		void SetHitPoints (CShip *pShip, const CShipInteriorDesc &Desc, int iHP);
		void WriteToStream (IWriteStream *pStream);

	private:
		struct SCompartmentEntry
			{
			SCompartmentEntry (void) :
					iHP(0)
				{ }

			int iHP;						//	HP left

			//	Temporaries
			bool bHit;						//	TRUE if this compartment got a direct hit
			};

		struct SHitTestCtx
			{
			SHitTestCtx (CShip *pShipArg, const CShipInteriorDesc &DescArg) :
					pShip(pShipArg),
					Desc(DescArg),
					iPos(-1)
				{ }

			CShip *pShip;
			const CShipInteriorDesc &Desc;

			int iPos;
			TSortMap<DWORD, int> HitOrder;
			};

		int FindNextCompartmentHit (SHitTestCtx &HitCtx, int xHitPos, int yHitPos);
		bool PointInCompartment (SHitTestCtx &HitCtx, const SCompartmentDesc &CompDesc, int xHitPos, int yHitPos) const;

		TArray<SCompartmentEntry> m_Compartments;
	};

//	Maneuvering

enum EManeuverTypes
	{
	NoRotation,

	RotateLeft,
	RotateRight,
	};

class CIntegralRotationDesc
	{
	public:
		enum EConstants
			{
			ROTATION_FRACTION =				1024,
			};

		CIntegralRotationDesc (void) { }

		ALERROR Bind (SDesignLoadCtx &Ctx, CObjectImageArray &Image);
		inline int GetFrameAngle (void) const { return (int)((360.0 / m_iCount) + 0.5); }
		inline int GetFrameCount (void) const { return m_iCount; }
		int GetFrameIndex (int iAngle) const;
		int GetManeuverDelay (void) const;
		int GetManeuverability (void) const;
		inline int GetMaxRotationSpeed (void) const { return m_iMaxRotationRate; }
		Metric GetMaxRotationSpeedPerTick (void) const;
		inline int GetMaxRotationTimeTicks (void) const { Metric rSpeed = GetMaxRotationSpeedPerTick(); return (rSpeed > 0.0 ? (int)(360.0 / rSpeed) : 0); }
		inline int GetRotationAccel (void) const { return m_iRotationAccel; }
		inline int GetRotationAccelStop (void) const { return m_iRotationAccelStop; }
		Metric GetRotationAccelPerTick (void) const;
		inline int GetRotationAngle (int iIndex) const { return m_Rotations[iIndex % m_iCount].iRotation; }
		ALERROR InitFromXML (SDesignLoadCtx &Ctx, const CString &sUNID, CXMLElement *pDesc);

	private:
		struct SEntry
			{
			int iRotation;					//	Angle at this rotation position
			};

		void InitRotationCount (int iCount);

		int m_iCount;						//	Number of rotations
		Metric m_rDegreesPerTick;			//	Rotations per tick
		Metric m_rAccelPerTick;				//	Degrees acceleration per tick
		Metric m_rAccelPerTickStop;			//	Degrees acceleration per tick when stoping rotation
		int m_iManeuverability;				//	Only for backwards compatibility (during InitFromXML)

		int m_iMaxRotationRate;				//	Rotations per tick (in 1/1000ths of a rotation)
		int m_iRotationAccel;				//	Rotation acceleration (in 1/1000ths of a rotation)
		int m_iRotationAccelStop;			//	Rotation acceleration when stopping rotation (in 1/1000th of a rotation)
		TArray<SEntry> m_Rotations;			//	Entries for each rotation
	};

class CIntegralRotation
	{
	public:
		CIntegralRotation (void) :
				m_iRotationFrame(0),
				m_iRotationSpeed(0),
				m_iMaxRotationRate(CIntegralRotationDesc::ROTATION_FRACTION),
				m_iRotationAccel(CIntegralRotationDesc::ROTATION_FRACTION),
				m_iRotationAccelStop(CIntegralRotationDesc::ROTATION_FRACTION),
				m_iLastManeuver(NoRotation)
			{ }

		~CIntegralRotation (void);

		inline int GetFrameIndex (void) const { return GetFrameIndex(m_iRotationFrame); }
		inline EManeuverTypes GetLastManeuver (void) const { return m_iLastManeuver; }
		inline Metric GetManeuverRatio (void) const { return (Metric)m_iMaxRotationRate / CIntegralRotationDesc::ROTATION_FRACTION; }
		EManeuverTypes GetManeuverToFace (const CIntegralRotationDesc &Desc, int iAngle) const;
		int GetRotationAngle (const CIntegralRotationDesc &Desc) const;
		void Init (const CIntegralRotationDesc &Desc, int iRotationAngle = -1);
		void ReadFromStream (SLoadCtx &Ctx, const CIntegralRotationDesc &Desc);
		void SetRotationAngle (const CIntegralRotationDesc &Desc, int iAngle);
		void Update (const CIntegralRotationDesc &Desc, EManeuverTypes iManeuver);
		void UpdateAccel (const CIntegralRotationDesc &Desc, Metric rHullMass = 0.0, Metric rItemMass = 0.0);
		void WriteToStream (IWriteStream *pStream) const;

	private:
		int CalcFinalRotationFrame (const CIntegralRotationDesc &Desc) const;
		inline int GetFrameIndex (int iFrame) const { return (iFrame / CIntegralRotationDesc::ROTATION_FRACTION); }

		int m_iRotationFrame;				//	Current rotation (in 1/1000ths of a rotation)
		int m_iRotationSpeed;				//	Current rotation speed (+ clockwise; - counterclockwise; in 1/1000ths)

		int m_iMaxRotationRate;				//	Current max speed
		int m_iRotationAccel;				//	Current rotation acceleration
		int m_iRotationAccelStop;			//	Current rotation accel for stopping rotation

		EManeuverTypes m_iLastManeuver;		//	Maneuver on last update
	};

//	Object effects

class CObjectEffectDesc
	{
	public:
		enum ETypes
			{
			effectNone =				0x00000000,	//	No effect

			effectThrustLeft =			0x00000001,	//	Left (counter-clockwise) thrusters
			effectThrustRight =			0x00000002,	//	Right (clockwise) thrusters
			effectThrustMain =			0x00000004,	//	Forward thrust
			effectThrustStop =			0x00000008,	//	Stop thrusters
			effectWeaponFire =			0x00000010,	//	Weapon flash
			};

		struct SEffectDesc
			{
			ETypes iType;					//	Type of effect
			CEffectCreatorRef pEffect;		//	Effect type
			C3DConversion PosCalc;			//	Position of effect
			int iRotation;					//	Direction of effect
			};

		ALERROR Bind (SDesignLoadCtx &Ctx, const CObjectImageArray &Image);
		inline IEffectPainter *CreatePainter (CCreatePainterCtx &Ctx, int iIndex) { return m_Effects[iIndex].pEffect.CreatePainter(Ctx); }
		CEffectCreator *FindEffectCreator (const CString &sUNID) const;
		inline int GetEffectCount (void) const { return m_Effects.GetCount(); }
		int GetEffectCount (DWORD dwEffects) const;
		const SEffectDesc &GetEffectDesc (int iIndex) const { return m_Effects[iIndex]; }
		ALERROR InitFromXML (SDesignLoadCtx &Ctx, const CString &sUNID, CXMLElement *pDesc);
		void MarkImages (void);

		static bool IsManeuverEffect (const SEffectDesc &Desc) { return (((DWORD)Desc.iType & (effectThrustLeft | effectThrustRight | effectThrustStop)) ? true : false); }

	private:
		TArray<SEffectDesc> m_Effects;
	};

class CObjectEffectList
	{
	public:
		~CObjectEffectList (void);

		void AccumulateBounds (CSpaceObject *pObj, const CObjectEffectDesc &Desc, int iRotation, RECT *ioBounds);
		void Init (const CObjectEffectDesc &Desc, const TArray<IEffectPainter *> &Painters);
		void Move (CSpaceObject *pObj, const CVector &vOldPos, bool *retbBoundsChanged = NULL);
		void Paint (SViewportPaintCtx &Ctx, const CObjectEffectDesc &Desc, DWORD dwEffects, CG16bitImage &Dest, int x, int y);
		void PaintAll (SViewportPaintCtx &Ctx, const CObjectEffectDesc &Desc, CG16bitImage &Dest, int x, int y);
		void Update (CSpaceObject *pObj, const CObjectEffectDesc &Desc, int iRotation, DWORD dwEffects);

	private:
		struct SFixedEffect
			{
			SFixedEffect (void) :
					pPainter(NULL)
				{ }

			IEffectPainter *pPainter;
			};

		void CleanUp (void);

		TArray<SFixedEffect> m_FixedEffects;
	};

//	Ship Controller

enum EOrderFlags
	{
	ORDER_FLAG_DELETE_ON_STATION_DESTROYED =	0x00000001,	//	Delete the order when target is station destroyed
	ORDER_FLAG_UPDATE_ON_NEW_PLAYER_SHIP =		0x00000002,	//	Update target if player changes ship
	ORDER_FLAG_NOTIFY_ON_STATION_DESTROYED =	0x00000004,	//	Notify controller when any station destroyed
	};

class IShipController
	{
	public:
		enum EDataTypes
			{
			dataNone,						//	dwData is ignored
			dataInteger,					//	dwData is a 32-bit integer
			dataPair,						//	dwData is two 16-bit integers
			dataString,						//	dwData is a pointer to a CString
			};

		struct SData
			{
			SData (void) : iDataType(dataNone)
				{ }

			SData (DWORD dwData) : iDataType(dataInteger),
					dwData1(dwData)
				{ }

			SData (DWORD dwData1Arg, DWORD dwData2Arg) : iDataType(dataPair),
					dwData1(dwData1Arg),
					dwData2(dwData2Arg)
				{ }

			SData (const CString &sDataArg) : iDataType(dataString),
					sData(sDataArg)
				{ }

			DWORD AsInteger (void) const { if (iDataType == dataInteger || iDataType == dataPair) return dwData1; else return 0; }
			DWORD AsInteger2 (void) const { if (iDataType == dataPair) return dwData2; else return 0; }

			EDataTypes iDataType;

			DWORD dwData1;
			DWORD dwData2;
			CString sData;
			};

		//	When adding a new order, also add to:
		//
		//	g_OrderTypes in Utilities.cpp

		enum OrderTypes
			{
			orderNone,					//	Nothing to do

			orderGuard,					//	pTarget = target to guard
			orderDock,					//	pTarget = station to dock with
			orderDestroyTarget,			//	pTarget = target to destroy; dwData = timer (0 == indefinitely)
			orderWait,					//	dwData = seconds to wait (0 == indefinitely)
			orderGate,					//	Gate out of the system (pTarget = optional gate)

			orderGateOnThreat,			//	Gate out of system if threatened
			orderGateOnStationDestroyed,//	Gate out if station is destroyed
			orderPatrol,				//	pTarget = target to orbit; dwData = radius (light-seconds)
			orderEscort,				//	pTarget = target to escort; dwData1 = angle; dwData2 = radius
			orderScavenge,				//	Scavenge for scraps

			orderFollowPlayerThroughGate,//	Follow the player through the nearest gate
			orderAttackNearestEnemy,	//	Attack the nearest enemy
			orderTradeRoute,			//	Moves from station to station
			orderWander,				//	Wander around system, avoiding enemies
			orderLoot,					//	pTarget = target to loot

			orderHold,					//	Stay in place; dwData = seconds (0 == indefinitely)
			orderMine,					//	Mine asteroids; pTarget = base
			orderWaitForPlayer,			//	Wait for player to return to the system
			orderDestroyPlayerOnReturn,	//	Wait to attack player when she returns
			orderFollow,				//	pTarget = target to follow (like escort, but no defense)

			orderNavPath,				//	dwData = nav path ID to follow
			orderGoTo,					//	Go to the given object (generally a marker)
			orderWaitForTarget,			//	Hold until pTarget is in range; dwData1 = radius (0 = LRS range); dwData2 = timer
			orderWaitForEnemy,			//	Hold until any enemy is in LRS range (or dwData timer expires)
			orderBombard,				//	Hold and attack target from here; pTarget = target; dwData = time

			orderApproach,				//	Get within dwData ls of pTarget
			orderAimAtTarget,			//	Same as orderDestroyTarget, but we never fire
			orderOrbit,					//	pTarget = target to orbit; dwData1 = radius (light-seconds); dwData2 = timer
			orderHoldCourse,			//	dwData1 = course; dwData2 = distance (light-seconds)
			orderTurnTo,				//	dwData = angle to face

			orderDestroyTargetHold,		//	pTarget = target to destroy; dwData = timer
			orderAttackStation,			//	pTarget = station to destroy
			orderFireEvent,				//	pTarget = object that gets event; Data = event name
			orderWaitForUndock,			//	Wait for pTarget to undock
			orderSendMessage,			//	pTarget = object that gets message; Data = message to send

			orderAttackArea,			//	pTarget = center; dwData1 = radius (light-seconds); dwData2 = timer.
			orderHoldAndAttack,			//	pTarget = target to destroy; dwData = timer.
			};

		virtual ~IShipController (void) { }

		virtual void Behavior (void) { }
		virtual void CancelDocking (void) { }
		virtual CString DebugCrashInfo (void) { return NULL_STR; }
		virtual void DebugPaintInfo (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx) { }
		virtual bool FollowsObjThroughGate (CSpaceObject *pLeader) { return false; }
		virtual CString GetAISetting (const CString &sSetting) { return NULL_STR; }
		virtual const CAISettings *GetAISettings (void) { return NULL; }
		virtual CSpaceObject *GetBase (void) const { return NULL; }
		virtual int GetCombatPower (void) = 0;
		virtual CCurrencyBlock *GetCurrencyBlock (void) { return NULL; }
		virtual CSpaceObject *GetDestination (void) const { return NULL; }
		virtual bool GetDeviceActivate (void) = 0;
		virtual CSpaceObject *GetEscortPrincipal (void) const { return NULL; }
		virtual int GetFireDelay (void) { return 0; }
		virtual int GetFireRateAdj (void) { return 10; }
		virtual EManeuverTypes GetManeuver (void) = 0;
		virtual CSpaceObject *GetOrderGiver (void) = 0;
		virtual GenomeTypes GetPlayerGenome (void) { return genomeUnknown; }
		virtual CString GetPlayerName (void) { return NULL_STR; }
		virtual bool GetReverseThrust (void) = 0;
		virtual CSpaceObject *GetShip (void) { return NULL; }
		virtual bool GetStopThrust (void) = 0;
		virtual CSpaceObject *GetTarget (CItemCtx &ItemCtx, bool bNoAutoTarget = false) const { return NULL; }
		virtual bool GetThrust (void) = 0;
		virtual void GetWeaponTarget (STargetingCtx &TargetingCtx, CItemCtx &ItemCtx, CSpaceObject **retpTarget, int *retiFireSolution) { }
		virtual bool IsAngryAt (CSpaceObject *pObj) const { return false; }
		virtual bool IsPlayer (void) const { return false; }
		virtual bool IsPlayerWingman (void) const { return false; }
		virtual void ReadFromStream (SLoadCtx &Ctx, CShip *pShip) { ASSERT(false); }
		virtual CString SetAISetting (const CString &sSetting, const CString &sValue) { return NULL_STR; }
		virtual void SetCommandCode (ICCItem *pCode) { }
		virtual void SetManeuver (EManeuverTypes iManeuver) { }
		virtual void SetShipToControl (CShip *pShip) { }
		virtual void SetThrust (bool bThrust) { }
		virtual void SetPlayerWingman (bool bIsWingman) { }
		virtual void WriteToStream (IWriteStream *pStream) { ASSERT(false); }

		virtual void AddOrder (OrderTypes Order, CSpaceObject *pTarget, const IShipController::SData &Data, bool bAddBefore = false) { }
		virtual void CancelAllOrders (void) { }
		virtual void CancelCurrentOrder (void) { }
		virtual DWORD GetCurrentOrderData (void) { return 0; }
		virtual OrderTypes GetCurrentOrderEx (CSpaceObject **retpTarget = NULL, IShipController::SData *retData = NULL) { return orderNone; }

		//	Events

		virtual void OnArmorRepaired (int iSection) { }
		virtual void OnAttacked (CSpaceObject *pAttacker, const DamageDesc &Damage) { }
		virtual void OnBlindnessChanged (bool bBlind, bool bNoMessage = false) { }
		virtual DWORD OnCommunicate (CSpaceObject *pSender, MessageTypes iMessage, CSpaceObject *pParam1, DWORD dwParam2) { return resNoAnswer; }
		virtual void OnComponentChanged (ObjectComponentTypes iComponent) { }
		virtual void OnDamaged (const CDamageSource &Cause, CInstalledArmor *pArmor, const DamageDesc &Damage, int iDamage) { }
		virtual void OnDeviceEnabledDisabled (int iDev, bool bEnabled, bool bSilent = false) { }
		virtual void OnDeviceStatus (CInstalledDevice *pDev, int iEvent) { }
		virtual bool OnDestroyCheck (DestructionTypes iCause, const CDamageSource &Attacker) { return true; }
		virtual void OnDestroyed (SDestroyCtx &Ctx) { }
		virtual void OnDocked (CSpaceObject *pObj) { }
		virtual void OnDockedObjChanged (CSpaceObject *pLocation) { }
		virtual void OnEnterGate (CTopologyNode *pDestNode, const CString &sDestEntryPoint, CSpaceObject *pStargate, bool bAscend) { }
		virtual void OnFuelLowWarning (int iSeq) { }
		virtual void OnItemDamaged (const CItem &Item, int iHP) { }
		virtual void OnItemFired (const CItem &Item) { }
		virtual void OnItemInstalled (const CItem &Item) { }
		virtual void OnItemUninstalled (const CItem &Item) { }
		virtual void OnLifeSupportWarning (int iSecondsLeft) { }
		virtual void OnMessage (CSpaceObject *pSender, const CString &sMsg) { }
		virtual void OnMissionCompleted (CMission *pMission, bool bSuccess) { }
		virtual void OnNewSystem (CSystem *pSystem) { }
		virtual void OnObjDamaged (const SDamageCtx &Ctx) { }
		virtual void OnObjDestroyed (const SDestroyCtx &Ctx) { }
		virtual void OnObjEnteredGate (CSpaceObject *pObj, CTopologyNode *pDestNode, const CString &sDestEntryPoint, CSpaceObject *pStargate) { }
		virtual void OnPaintSRSEnhancements (CG16bitImage &Dest, SViewportPaintCtx &Ctx) { }
		virtual void OnPlayerChangedShips (CSpaceObject *pOldShip) { }
		virtual void OnPlayerObj (CSpaceObject *pPlayer) { }
		virtual void OnProgramDamage (CSpaceObject *pHacker, const ProgramDesc &Program) { }
		virtual void OnRadiationWarning (int iTicksLeft) { }
		virtual void OnRadiationCleared (void) { }
		virtual void OnReactorOverloadWarning (int iSeq) { }
		virtual void OnStatsChanged (void) { }
		virtual void OnStationDestroyed (const SDestroyCtx &Ctx) { }
		virtual void OnSystemLoaded (void) { }
		virtual void OnUpdatePlayer (SUpdateCtx &Ctx) { }
		virtual void OnWeaponStatusChanged (void) { }
		virtual void OnWreckCreated (CSpaceObject *pWreck) { }
	};

struct SShipGeneratorCtx
	{
	SShipGeneratorCtx (void) :
			pItems(NULL),
			pOnCreate(NULL),
			iOrder(IShipController::orderNone),
			pBase(NULL),
			pTarget(NULL),
			dwCreateFlags(0)
		{ }

	CString sName;								//	If not blank, use as name of ship
	DWORD dwNameFlags;							//	Name flags (only if sName is not blank)
	IItemGenerator *pItems;						//	Items to add to ship (may be NULL)
	CAttributeDataBlock InitialData;			//	Initial data
	ICCItem *pOnCreate;							//	Additional OnCreate code (may be NULL)
	DWORD dwCreateFlags;

	IShipController::OrderTypes iOrder;			//	Order for ship
	IShipController::SData OrderData;			//	Order data

	CSpaceObject *pBase;						//	Base for ship (may be NULL)
	CSpaceObject *pTarget;						//	Target for ship (may be NULL)
	};

//	Effects

enum ParticlePaintStyles
	{
	paintPlain,
	paintFlame,
	paintSmoke,
	paintImage,
	paintLine,
	};

struct SParticlePaintDesc
	{
	SParticlePaintDesc (void)
		{
		iStyle = paintPlain;

		pImage = NULL;
		iVariants = 0;
		bRandomStartFrame = false;
		bDirectional = false;

		iMaxLifetime = -1;
		iMinWidth = 4;
		iMaxWidth = 4;

		wPrimaryColor = CG16bitImage::RGBValue(255, 255, 255);
		wSecondaryColor = CG16bitImage::RGBValue(0, 0, 0);
		}

	ParticlePaintStyles iStyle;

	CObjectImageArray *pImage;					//	Image for each particle
	int iVariants;								//	No of image variants
	bool bRandomStartFrame;						//	Start at a random animation
	bool bDirectional;							//	Different images for each direction

	int iMaxLifetime;							//	Max lifetime for particles
	int iMinWidth;								//	Min width of particle
	int iMaxWidth;								//	Max width of particle

	WORD wPrimaryColor;							//	Primary color 
	WORD wSecondaryColor;						//	Secondary color
	};

struct SEffectHitDesc
	{
	CSpaceObject *pObjHit;						//	Object that was hit by the effect
	CVector vHitPos;							//	Position hit
	int iHitStrength;							//	Number of particles hitting (or 0-100 hit strength)
	};

typedef TArray<SEffectHitDesc> CEffectHitResults;

struct SEffectMoveCtx
	{
	SEffectMoveCtx (void) :
			pObj(NULL)
		{ }

	CSpaceObject *pObj;							//	The object that owns the effect
	CVector vOldPos;							//	Old position of object
	};

struct SEffectUpdateCtx
	{
	SEffectUpdateCtx (void) : 
			pSystem(NULL),
			pObj(NULL),
			iRotation(0),
			bFade(false),

			pDamageDesc(NULL),
			pEnhancements(NULL),
			iCause(killedByDamage),
			bAutomatedWeapon(false),

			iTotalParticleCount(1),
			iWakePotential(0),

			bDestroy(false)
		{ }

	//	Object context
	CSystem *pSystem;							//	Current system
	CSpaceObject *pObj;							//	The object that the effect is part of
	int iRotation;								//	Rotation
	CVector vEmitPos;							//	Emittion pos (if not center of effect)
												//		Relative to center of effect.
	bool bFade;									//	Effect fading

	//	Damage context
	CWeaponFireDesc *pDamageDesc;				//	Damage done by particles (may be NULL)
	CItemEnhancementStack *pEnhancements;		//	Damage enhancements (may be NULL)
	DestructionTypes iCause;					//	Cause of damage
	bool bAutomatedWeapon;						//	TRUE if this is an automated attack
	CDamageSource Attacker;						//	Attacker

	//	Particle context
	int iTotalParticleCount;					//	Total particles
	int iWakePotential;							//	Objects influence particle motion (0-100)

	//	Outputs
	CEffectHitResults Hits;						//	Filled in with the objects that hit
	bool bDestroy;								//	Destroy the effect
	};

class CEffectParamDesc
	{
	public:
		enum EDataTypes
			{
			typeNull =						0,

			typeColorConstant =				1,
			typeIntegerConstant =			2,
			typeIntegerDiceRange =			3,
			typeIntegerExpression =			4,
			typeStringConstant =			5,
			typeBoolConstant =				6,
			typeVectorConstant =			7,
			};

		CEffectParamDesc (void) : m_iType(typeNull), m_pItem(NULL)
			{ }

		~CEffectParamDesc (void);

		bool EvalBool (CCreatePainterCtx &Ctx) const;
		WORD EvalColor (CCreatePainterCtx &Ctx) const;
		DiceRange EvalDiceRange (CCreatePainterCtx &Ctx, int iDefault = -1) const;
		int EvalIdentifier (CCreatePainterCtx &Ctx, LPSTR *pIDMap, int iMax, int iDefault = 0) const;
		int EvalInteger (CCreatePainterCtx &Ctx) const;
		int EvalIntegerBounded (CCreatePainterCtx &Ctx, int iMin, int iMax = -1, int iDefault = -1) const;
		CString EvalString (CCreatePainterCtx &Ctx) const;
		CVector EvalVector (CCreatePainterCtx &Ctx) const;
		inline EDataTypes GetType (void) const { return m_iType; }
		inline void InitBool (bool bValue) { CleanUp(); m_dwData = (bValue ? 1 : 0); m_iType = typeBoolConstant; }
		inline void InitColor (WORD wValue) { CleanUp(); m_dwData = wValue; m_iType = typeColorConstant; }
		inline void InitDiceRange (const DiceRange &Value) { CleanUp(); m_DiceRange = Value; m_iType = typeIntegerDiceRange; }
		inline void InitInteger (int iValue) { CleanUp(); m_dwData = iValue; m_iType = typeIntegerConstant; }
		inline void InitNull (void) { CleanUp(); }
		inline void InitString (const CString &sValue) { CleanUp(); m_sData = sValue; m_iType = typeStringConstant; }
		inline void InitVector (const CVector &vValue) { CleanUp(); m_vVector = vValue; m_iType = typeVectorConstant; }
		ALERROR InitColorFromXML (SDesignLoadCtx &Ctx, const CString &sValue);
		ALERROR InitIdentifierFromXML (SDesignLoadCtx &Ctx, const CString &sValue, LPSTR *pIDMap);
		ALERROR InitIntegerFromXML (SDesignLoadCtx &Ctx, const CString &sValue);
		ALERROR InitStringFromXML (SDesignLoadCtx &Ctx, const CString &sValue);
		bool IsConstant (void);
		void ReadFromStream (SLoadCtx &Ctx);
		void WriteToStream (IWriteStream *pStream);

	private:
		void CleanUp (void);
		bool FindIdentifier (const CString &sValue, LPSTR *pIDMap, DWORD *retdwID = NULL) const;

		EDataTypes m_iType;

		DWORD m_dwData;
		CString m_sData;

		DiceRange m_DiceRange;
		CVector m_vVector;

		ICCItem *m_pItem;
	};

class IEffectPainter
	{
	public:
		IEffectPainter (bool bSingleton = false) : m_bSingleton(bSingleton)
			{ }

		void GetBounds (RECT *retRect);
		void GetBounds (const CVector &vPos, CVector *retvUR, CVector *retvLL);
		inline bool IsSingleton (void) const { return m_bSingleton; }
		inline void OnUpdate (void) { SEffectUpdateCtx Ctx; OnUpdate(Ctx); }
		inline void PlaySound (CSpaceObject *pSource);
		inline void ReadFromStream (SLoadCtx &Ctx) { OnReadFromStream(Ctx); }
		static CString ReadUNID (SLoadCtx &Ctx);
		inline void SetSingleton (bool bSingleton = true) { m_bSingleton = bSingleton; }
		static ALERROR ValidateClass (SLoadCtx &Ctx, const CString &sOriginalClass);
		void WriteToStream (IWriteStream *pStream);

#ifdef DEBUG_SINGLETON_EFFECTS
		virtual ~IEffectPainter (void)
			{
			if (m_bSingleton)
				{
				::kernelDebugLogMessage("Delete singleton painter: %08x", (DWORD)this);
				}
			}
#else
		virtual ~IEffectPainter (void) { }
#endif
		virtual bool CanPaintComposite (void) { return false; }
		virtual void Delete (void) { if (!m_bSingleton) delete this; }
		virtual CEffectCreator *GetCreator (void) = 0;
		virtual int GetFadeLifetime (void) { return 0; }
		virtual int GetLifetime (void) { return GetInitialLifetime(); }
		virtual void GetParam (const CString &sParam, CEffectParamDesc *retValue) { retValue->InitNull(); }
		virtual bool GetParamList (TArray<CString> *retList) const { return false; }
		virtual int GetParticleCount (void) { return 1; }
		virtual bool GetParticlePaintDesc (SParticlePaintDesc *retDesc) { return false; }
		virtual Metric GetRadius (void) const;
		virtual void GetRect (RECT *retRect) const;
		virtual int GetVariants (void) const { return 1; }
		virtual void OnBeginFade (void) { }
		virtual void OnMove (SEffectMoveCtx &Ctx, bool *retbBoundsChanged = NULL) { if (retbBoundsChanged) *retbBoundsChanged = false; }
		virtual void OnUpdate (SEffectUpdateCtx &Ctx) { }
		virtual void Paint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx) = 0;
		virtual void PaintComposite (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx) { Paint(Dest, x, y, Ctx); }
		virtual void PaintFade (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx) { }
		virtual void PaintHit (CG16bitImage &Dest, int x, int y, const CVector &vHitPos, SViewportPaintCtx &Ctx) { }
		virtual bool PointInImage (int x, int y, int iTick, int iVariant = 0, int iRotation = 0) const { return false; }
		virtual void SetParam (CCreatePainterCtx &Ctx, const CString &sParam, const CEffectParamDesc &Value) { }
		virtual void SetParamMetric (const CString &sParam, Metric rValue) { }
		virtual bool SetParamString (const CString &sParam, const CString &sValue) { return false; }
		virtual bool SetProperty (const CString &sProperty, ICCItem *pValue) { return false; }
		virtual void SetPos (const CVector &vPos) { }
		virtual void SetVariants (int iVariants) { }

	protected:
		virtual void OnReadFromStream (SLoadCtx &Ctx);
		virtual void OnWriteToStream (IWriteStream *pStream);

	private:
		int GetInitialLifetime (void);

		bool m_bSingleton;
	};

class CEffectPainterRef
	{
	public:
		CEffectPainterRef (void) :
				m_pPainter(NULL)
			{ }

		~CEffectPainterRef (void) { Delete(); }

		inline operator IEffectPainter *() const { return m_pPainter; }
		inline IEffectPainter * operator->() const { return m_pPainter; }

		void Delete (void)
			{
			if (m_pPainter && m_bDelete)
				m_pPainter->Delete();

			m_pPainter = NULL;
			}

		inline bool IsEmpty (void) const { return (m_pPainter == NULL); }

		void Set (IEffectPainter *pPainter)
			{
			Delete();
			if (pPainter)
				{
				m_pPainter = pPainter;
				m_bDelete = !pPainter->IsSingleton();
				}
			}

	private:
		IEffectPainter *m_pPainter;
		bool m_bDelete;
	};

//	Topology Descriptors -------------------------------------------------------

const int INFINITE_NODE_DIST =					1000000;
const DWORD END_GAME_SYSTEM_UNID =				0x00ffffff;

struct SRequiredEncounterDesc
	{
	CStationType *pType;					//	Encounter to create
	int iLeftToCreate;						//	Left to create
	};

class CTopologyNode
	{
	public:
		struct SDistanceTo
			{
			TArray<CString> AttribsRequired;
			TArray<CString> AttribsNotAllowed;

			CString sNodeID;

			int iMinDist;
			int iMaxDist;
			};

		struct SCriteriaCtx
			{
			SCriteriaCtx (void) :
					pTopology(NULL)
				{ }

			CTopology *pTopology;
			};

		struct SCriteria
			{
			int iChance;								//	Probability 0-100 of matching criteria
			int iMinStargates;							//	Match if >= this many stargates
			int iMaxStargates;							//	Match if <= this many stargates
			int iMinInterNodeDist;						//	Used by <DistributeNodes> (maybe move there)
			int iMaxInterNodeDist;
			TArray<CString> AttribsRequired;			//	Does not match if any of these attribs are missing
			TArray<CString> AttribsNotAllowed;			//	Does not match if any of these attribs are present
			TArray<SDistanceTo> DistanceTo;				//	Matches if node is within the proper distance of another node or nodes
			TArray<CString> SpecialRequired;			//	Special attributes
			TArray<CString> SpecialNotAllowed;			//	Special attributes
			};

		CTopologyNode (const CString &sID, DWORD SystemUNID, CSystemMap *pMap);
		~CTopologyNode (void);
		static void CreateFromStream (SUniverseLoadCtx &Ctx, CTopologyNode **retpNode);

		void AddAttributes (const CString &sAttribs);
		ALERROR AddGateInt (const CString &sName, const CString &sDestNode, const CString &sEntryPoint);
		ALERROR AddStargate (const CString &sGateID, const CString &sDestNodeID, const CString &sDestGateID);
		ALERROR AddStargateConnection (CTopologyNode *pDestNode, bool bOneWay = false, const CString &sFromName = NULL_STR, const CString &sToName = NULL_STR);
		int CalcMatchStrength (const CAttributeCriteria &Criteria);
		void ChangeRequiredEncounter (CStationType *pType, int iChange = 1);
		bool FindStargate (const CString &sName, CString *retsDestNode = NULL, CString *retsEntryPoint = NULL);
		bool FindStargateTo (const CString &sDestNode, CString *retsName = NULL, CString *retsDestGateID = NULL);
		CString FindStargateName (const CString &sDestNode, const CString &sEntryPoint);
		inline const CString &GetAttributes (void) const { return m_sAttributes; }
		inline int GetCalcDistance (void) const { return m_iCalcDistance; }
		inline const CString &GetData (const CString &sAttrib) const { return m_Data.GetData(sAttrib); }
		inline CSystemMap *GetDisplayPos (int *retxPos = NULL, int *retyPos = NULL);
		inline const CString &GetEndGameReason (void) { return m_sEndGameReason; }
		inline const CString &GetEpitaph (void) { return m_sEpitaph; }
		inline const CString &GetID (void) const { return m_sID; }
		CTopologyNode *GetGateDest (const CString &sName, CString *retsEntryPoint = NULL);
		inline int GetLevel (void) { return m_iLevel; }
		ICCItem *GetProperty (const CString &sName);
		int GetRequiredEncounter (CStationType *pType) const;
		inline int GetStargateCount (void) { return m_NamedGates.GetCount(); }
		CString GetStargate (int iIndex);
		CTopologyNode *GetStargateDest (int iIndex, CString *retsEntryPoint = NULL);
		inline CSystem *GetSystem (void) { return m_pSystem; }
		inline DWORD GetSystemID (void) { return m_dwID; }
		inline const CString &GetSystemName (void) { return m_sName; }
		inline DWORD GetSystemTypeUNID (void) { return m_SystemUNID; }
		inline bool HasAttribute (const CString &sAttrib) { return ::HasModifier(m_sAttributes, sAttrib); }
		bool HasSpecialAttribute (const CString &sAttrib) const;
		ALERROR InitFromAdditionalXML (CXMLElement *pDesc, CString *retsError);
		ALERROR InitFromAttributesXML (CXMLElement *pAttributes, CString *retsError);
		ALERROR InitFromSystemXML (CXMLElement *pSystem, CString *retsError);
		static bool IsCriteriaAll (const SCriteria &Crit);
		inline bool IsEndGame (void) const { return (m_SystemUNID == END_GAME_SYSTEM_UNID); }
		inline bool IsKnown (void) const { return m_bKnown; }
		inline bool IsMarked (void) const { return m_bMarked; }
		bool MatchesCriteria (SCriteriaCtx &Ctx, const SCriteria &Crit);
		static ALERROR ParseCriteria (CXMLElement *pCrit, SCriteria *retCrit, CString *retsError = NULL);
		static ALERROR ParseCriteria (const CString &sCriteria, SCriteria *retCrit, CString *retsError = NULL);
		static ALERROR ParseCriteriaInt (const CString &sCriteria, SCriteria *retCrit);
		static ALERROR ParsePosition (const CString &sValue, int *retx, int *rety);
		static ALERROR ParseStargateString (const CString &sStargate, CString *retsNodeID, CString *retsGateName);
		inline void SetCalcDistance (int iDist) { m_iCalcDistance = iDist; }
		inline void SetData (const CString &sAttrib, const CString &sData) { m_Data.SetData(sAttrib, sData); }
		inline void SetEndGameReason (const CString &sReason) { m_sEndGameReason = sReason; }
		inline void SetEpitaph (const CString &sEpitaph) { m_sEpitaph = sEpitaph; }
		inline void SetKnown (bool bKnown = true) { m_bKnown = bKnown; }
		inline void SetLevel (int iLevel) { m_iLevel = iLevel; }
		inline void SetMarked (bool bValue = true) { m_bMarked = bValue; }
		inline void SetName (const CString &sName) { m_sName = sName; }
		inline void SetPos (int xPos, int yPos) { m_xPos = xPos; m_yPos = yPos; }
		bool SetProperty (const CString &sName, ICCItem *pValue, CString *retsError);
		void SetStargateDest (const CString &sName, const CString &sDestNode, const CString &sEntryPoint);
		inline void SetSystem (CSystem *pSystem) { m_pSystem = pSystem; }
		inline void SetSystemID (DWORD dwID) { m_dwID = dwID; }
		inline void SetSystemUNID (DWORD dwUNID) { m_SystemUNID = dwUNID; }
		void WriteToStream (IWriteStream *pStream);

		inline void AddVariantLabel (const CString &sVariant) { m_VariantLabels.Insert(sVariant); }
		bool HasVariantLabel (const CString &sVariant);

	private:
		struct StarGateDesc
			{
			CString sDestNode;
			CString sDestEntryPoint;
			CTopologyNode *pDestNode;			//	Cached for efficiency (may be NULL)
			};

		CString GenerateStargateName (void);

		CString m_sID;							//	ID of node

		DWORD m_SystemUNID;						//	UNID of system type
		CString m_sName;						//	Name of system
		int m_iLevel;							//	Level of system

		CSystemMap *m_pMap;						//	May be NULL
		int m_xPos;								//	Position on map (cartessian)
		int m_yPos;

		CSymbolTable m_NamedGates;				//	Name to StarGateDesc

		CString m_sAttributes;					//	Attributes
		TArray<CString> m_VariantLabels;		//	Variant labels
		CString m_sEpitaph;						//	Epitaph if this is endgame node
		CString m_sEndGameReason;				//	End game reason if this is endgame node

		CAttributeDataBlock m_Data;				//	Opaque data

		CSystem *m_pSystem;						//	NULL if not yet created
		DWORD m_dwID;							//	ID of system instance

		bool m_bKnown;							//	TRUE if node is visible on galactic map
		bool m_bMarked;							//	Temp variable used during painting
		int m_iCalcDistance;					//	Temp variable used during distance calc
	};

class CTopologyNodeList
	{
	public:
		inline CTopologyNode *operator [] (int iIndex) const { return m_List.GetAt(iIndex); }

		void Delete (CTopologyNode *pNode);
		inline void Delete (int iIndex) { m_List.Delete(iIndex); }
		inline void DeleteAll (void) { m_List.DeleteAll(); }
		ALERROR Filter (CTopologyNode::SCriteriaCtx &Ctx, CXMLElement *pCriteria, CTopologyNodeList *retList, CString *retsError);
		ALERROR Filter (CTopologyNode::SCriteriaCtx &Ctx, CTopologyNode::SCriteria &Crit, CTopologyNodeList *ioList);
		bool FindNode (CTopologyNode *pNode, int *retiIndex = NULL);
		bool FindNode (const CString &sID, int *retiIndex = NULL);
		inline CTopologyNode *GetAt (int iIndex) const { return m_List.GetAt(iIndex); }
		inline int GetCount (void) const { return m_List.GetCount(); }
		inline void Insert (CTopologyNode *pNode) { m_List.Insert(pNode); }
		bool IsNodeInRangeOf (CTopologyNode *pNode, int iMin, int iMax, const TArray<CString> &AttribsRequired, const TArray<CString> &AttribsNotAllowed, CTopologyNodeList &Checked);
		void RestoreMarks (TArray<bool> &Saved);
		void SaveAndSetMarks (bool bMark, TArray<bool> *retSaved);
		inline void Shuffle (void) { m_List.Shuffle(); }

	private:
		TArray<CTopologyNode *> m_List;
	};

enum ENodeDescTypes
	{
	ndNode =			0x01,					//	A single node
	ndNodeTable =		0x02,					//	A table of descriptors (each of which can only appear once)
	ndFragment =		0x03,					//	A topology of nodes (used as fragment)
	ndNetwork =			0x04,					//	A network of nodes (used as fragment)
	ndRandom =			0x05,					//	Randomly generated network
	ndNodeGroup =		0x06,					//	A group of nodes with stargates
	};

class CTopologyDesc
	{
	public:
		CTopologyDesc (void);
		~CTopologyDesc (void);
		ALERROR BindDesign (SDesignLoadCtx &Ctx);
		CEffectCreator *FindEffectCreator (const CString &sUNID);
		CString GetAttributes (void);
		inline CXMLElement *GetDesc (void) const { return m_pDesc; }
		inline CEffectCreator *GetLabelEffect (void) const { return m_pLabelEffect; }
		inline CSystemMap *GetMap (void) const { return m_pMap; }
		inline CEffectCreator *GetMapEffect (void) const { return m_pMapEffect; }
		inline const CString &GetID (void) const { return m_sID; }
		bool GetPos (int *retx, int *rety);
		CXMLElement *GetSystem (void);
		inline CTopologyDesc *GetTopologyDesc (int iIndex);
		inline int GetTopologyDescCount (void);
		inline CTopologyDescTable *GetTopologyDescTable (void) { return m_pDescList; }
		inline ENodeDescTypes GetType (void) const { return m_iType; }
		inline bool IsAbsoluteNode (void) const { return (*m_sID.GetASCIIZPointer() != '+'); }
		bool IsEndGameNode (CString *retsEpitaph = NULL, CString *retsReason = NULL) const;
		inline bool IsRootNode (void) const { return ((m_dwFlags & FLAG_IS_ROOT_NODE) ? true : false); }
		ALERROR LoadFromXML (SDesignLoadCtx &Ctx, CXMLElement *pXMLDesc, CSystemMap *pMap, const CString &sParentUNID);
		inline void SetRootNode (void) { m_dwFlags |= FLAG_IS_ROOT_NODE; }

	private:
		enum Flags
			{
			FLAG_IS_ROOT_NODE = 0x00000001,
			};

		CSystemMap *m_pMap;						//	Map that contains this descriptor (may be NULL)
		CString m_sID;							//	ID of node
		ENodeDescTypes m_iType;					//	Type of node
		CXMLElement *m_pDesc;					//	XML for node definition
		DWORD m_dwFlags;

		CEffectCreatorRef m_pLabelEffect;		//	Effect to paint on label layer
		CEffectCreatorRef m_pMapEffect;			//	Effect to paint on galactic map

		CTopologyDescTable *m_pDescList;		//	Some node types (e.g., ndNetwork) have sub-nodes.
												//		This is a table of all subnodes
	};

class CTopologyDescTable
	{
	public:
		CTopologyDescTable (void);
		~CTopologyDescTable (void);

		ALERROR AddRootNode (SDesignLoadCtx &Ctx, const CString &sNodeID);
		ALERROR BindDesign (SDesignLoadCtx &Ctx);
		void CleanUp (void);
		inline void DeleteIDMap (void) { delete m_pIDToDesc; m_pIDToDesc = NULL; }
		CEffectCreator *FindEffectCreator (const CString &sUNID);
		CTopologyDesc *FindTopologyDesc (const CString &sID);
		CXMLElement *FindTopologyDescXML (const CString &sNodeID);
		CTopologyDesc *FindRootNodeDesc (const CString &sID);
		CXMLElement *FindRootNodeDescXML (const CString &sNodeID);
		inline const CString &GetFirstNodeID (void) { return m_sFirstNode; }
		inline int GetRootNodeCount (void) { return m_RootNodes.GetCount(); }
		inline CTopologyDesc *GetRootNodeDesc (int iIndex) { return m_RootNodes[iIndex]; }
		inline CXMLElement *GetRootNodeDescXML (int iIndex) { return m_RootNodes[iIndex]->GetDesc(); }
		inline CTopologyDesc *GetTopologyDesc (int iIndex) { return m_Table[iIndex]; }
		inline int GetTopologyDescCount (void) { return m_Table.GetCount(); }
		ALERROR LoadFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, CSystemMap *pMap, const CString &sParentUNID, bool bAddFirstAsRoot = false);
		ALERROR LoadNodeFromXML (SDesignLoadCtx &Ctx, CXMLElement *pNode, CSystemMap *pMap, const CString &sParentUNID, CTopologyDesc **retpNode = NULL);

	private:
		void InitIDMap (void);

		TArray<CTopologyDesc *> m_Table;
		TMap<CString, CTopologyDesc *> *m_pIDToDesc;
		TArray<CTopologyDesc *> m_RootNodes;

		CString m_sFirstNode;						//	Node where player starts (if not specified elsewhere)
	};

inline CTopologyDesc *CTopologyDesc::GetTopologyDesc (int iIndex) { return (m_pDescList ? m_pDescList->GetTopologyDesc(iIndex) : NULL); }
inline int CTopologyDesc::GetTopologyDescCount (void) { return (m_pDescList ? m_pDescList->GetTopologyDescCount() : 0); }

struct STopologyCreateCtx
	{
	STopologyCreateCtx (void) :
			pMap(NULL),
			pFragmentTable(NULL),
			pNodesAdded(NULL),
			bInFragment(false),
			xOffset(0),
			yOffset(0),
			iRotation(0)
		{ }

	CSystemMap *pMap;								//	Map that we're currently processing
	TArray<CTopologyDescTable *> Tables;			//	List of tables to look up
	CTopologyDescTable *pFragmentTable;
	CTopologyNodeList *pNodesAdded;					//	Output of nodes added

	bool bInFragment;
	CString sFragmentPrefix;
	int xOffset;
	int yOffset;
	int iRotation;
	CString sFragmentExitID;
	CString sFragmentExitGate;
	CString sFragmentAttributes;

	CString sError;
	};

//	Games and Records ----------------------------------------------------------

struct SBasicGameStats
	{
	int iSystemsVisited;
	int iEnemiesDestroyed;
	DWORD dwBestEnemyDestroyed;
	int iBestEnemyDestroyedCount;
	};

class CGameRecord
	{
	public:
		enum EDescParts
			{
			descCharacter =				0x00000001,
			descShip =					0x00000002,
			descEpitaph =				0x00000004,
			descDate =					0x00000008,
			descPlayTime =				0x00000010,
			descResurrectCount =		0x00000020,

			descAll =					0x0000003F,
			};

		enum Flags
			{
			FLAG_OMIT_WAS =				0x00000001,	//	Remove leading "was" from "was destroyed..."
			};

		CGameRecord (void);

		CString GetAdventureID (void) const;
		inline DWORD GetAdventureUNID (void) const { return m_dwAdventure; }
		CString GetDescription (DWORD dwParts = descAll) const;
		CString GetEndGameEpitaph (DWORD dwFlags = 0) const;
		inline const CString &GetEndGameReason (void) const { return m_sEndGameReason; }
		inline const CString &GetGameID (void) const { return m_sGameID; }
		inline GenomeTypes GetPlayerGenome (void) const { return m_iGenome; }
		inline const CString &GetPlayerName (void) const { return m_sName; }
		inline CString GetPlayTimeString (void) const { return m_Duration.Format(NULL_STR); }
		inline int GetResurrectCount (void) const { return m_iResurrectCount; }
		inline int GetScore (void) const { return m_iScore; }
		inline CString GetShipClass (void) const { return m_sShipClass; }
		inline const CString &GetUsername (void) const { return m_sUsername; }
		ALERROR InitFromJSON (const CJSONValue &Value);
		ALERROR InitFromXML (CXMLElement *pDesc);
		inline bool IsDebug (void) const { return m_bDebugGame; }
		inline bool IsRegistered (void) const { return m_bRegisteredGame; }
		static GenomeTypes LoadGenome (const CString &sAttrib);
		void SaveToJSON (CJSONValue *retOutput) const;
		inline void SetAdventureUNID (DWORD dwUNID) { m_dwAdventure = dwUNID; }
		inline void SetDebug (bool bDebug = true) { m_bDebugGame = bDebug; }
		inline void SetEndGameEpitaph (const CString &sEpitaph) { m_sEpitaph = sEpitaph; }
		inline void SetEndGameReason (const CString &sReason) { m_sEndGameReason = sReason; }
		inline void SetExtensions (const TArray<DWORD> &Extensions) { m_Extensions = Extensions; }
		inline void SetGameID (const CString &sGameID) { m_sGameID = sGameID; }
		inline void SetPlayerGenome (GenomeTypes iGenome) { m_iGenome = iGenome; }
		inline void SetPlayerName (const CString &sName) { m_sName = sName; }
		inline void SetPlayTime (const CTimeSpan &Time) { m_Duration = Time; }
		inline void SetRegistered (bool bRegistered = true) { m_bRegisteredGame = bRegistered; }
		inline void SetResurrectCount (int iCount) { m_iResurrectCount = iCount; }
		inline void SetScore (int iScore) { m_iScore = iScore; }
		void SetShipClass (DWORD dwUNID);
		void SetSystem (CSystem *pSystem);
		inline void SetUsername (const CString &sUsername) { m_sUsername = sUsername; }
		ALERROR WriteToXML (IWriteStream &Stream);

	private:
		//	Basic
		CString m_sUsername;					//	Username
		CString m_sGameID;						//	ID of game played
		DWORD m_dwAdventure;					//	UNID of adventure
		TArray<DWORD> m_Extensions;				//	UNID of included extensions

		CString m_sName;						//	Character name
		GenomeTypes m_iGenome;					//	Character genome

		DWORD m_dwShipClass;					//	Ship class UNID
		CString m_sShipClass;					//	Ship class
		CString m_sSystem;						//	NodeID of current system
		CString m_sSystemName;					//	Name of current system

		CTimeDate m_CreatedOn;					//	Game created on this date (set by server)
		CTimeDate m_ReportedOn;					//	Time/date of latest report (set by server)
		CTimeSpan m_Duration;					//	Time played
		bool m_bRegisteredGame;					//	If TRUE, this is a registered game
		bool m_bDebugGame;						//	If TRUE, this is a debug game

		//	Stats
		int m_iScore;
		int m_iResurrectCount;

		//	End Game
		CString m_sEndGameReason;
		CString m_sEpitaph;
	};

class CGameStats
	{
	public:
		inline void DeleteAll (void) { m_Stats.DeleteAll(); }
		inline int GetCount (void) const { return m_Stats.GetCount(); }
		inline const CString &GetDefaultSectionName (void) const { return m_sDefaultSectionName; }
		void GetEntry (int iIndex, CString *retsStatName, CString *retsStatValue, CString *retsSection) const;
		void Insert (const CString &sStatName, const CString &sStatValue = NULL_STR, const CString &sSection = NULL_STR, const CString &sSortKey = NULL_STR);
		ALERROR LoadFromStream (IReadStream *pStream);
		void SaveToJSON (CJSONValue *retOutput) const;
		ALERROR SaveToStream (IWriteStream *pStream) const;
		inline void SetDefaultSectionName (const CString &sName) { m_sDefaultSectionName = sName; }
		inline void Sort (void) { m_Stats.Sort(); }
		void TakeHandoff (CGameStats &Source);
		ALERROR WriteAsText (IWriteStream *pOutput) const;

	private:
		struct SStat
			{
			//	Needed for TArray's sort method
			bool operator > (const SStat &Right) const { return (strCompare(sSortKey, Right.sSortKey) == 1); }
			bool operator < (const SStat &Right) const { return (strCompare(sSortKey, Right.sSortKey) == -1); }

			CString sSortKey;
			CString sStatName;
			CString sStatValue;
			};

		void ParseSortKey (const CString &sSortKey, CString *retsSection, CString *retsSectionSortKey) const;

		TArray<SStat> m_Stats;
		CString m_sDefaultSectionName;
	};

class CAdventureRecord
	{
	public:
		enum ESpecialScoreIDs
			{
			personalBest =			10000,
			mostRecent =			10001,

			specialIDFirst =		10000,
			specialIDCount =		2,
			};

		CAdventureRecord (void) : m_dwAdventure(0) { }
		CAdventureRecord (DWORD dwAdventure, int iHighScoreCount);

		bool FindRecordByGameID (const CString &sGameID, DWORD *retdwID = NULL) const;
		CString GetAdventureName (void) const;
		DWORD GetAdventureUNID (void) const { return m_dwAdventure; }
		inline int GetHighScoreCount (void) const { return m_HighScores.GetCount(); }
		CGameRecord &GetRecordAt (DWORD dwID);
		void Init (DWORD dwAdventure);
		CGameRecord &InsertHighScore (void) { return *m_HighScores.Insert(); }
		inline bool IsSpecialID (DWORD dwID) const { return (dwID >= specialIDFirst && dwID < (specialIDFirst + specialIDCount)); }

	private:
		DWORD m_dwAdventure;
		TArray<CGameRecord> m_HighScores;
		CGameRecord m_Special[specialIDCount];
	};

class CUserProfile
	{
	public:
		CUserProfile (void) { }

		bool FindAdventureRecord (DWORD dwAdventure, int *retiIndex = NULL);
		inline CAdventureRecord &GetAdventureRecord (int iIndex) { return m_Records[iIndex]; }
		inline int GetAdventureRecordCount (void) { return m_Records.GetCount(); }
		void Init (const CString &sUsername);
		CAdventureRecord &InsertAdventureRecord (DWORD dwAdventure);

	private:
		CString m_sUsername;
		TArray<CAdventureRecord> m_Records;
	};

//	CItemType -----------------------------------------------------------------

class CItemCtx
	{
	public:
		CItemCtx (const CItem &Item) : m_pItem(&Item), m_pSource(NULL), m_pArmor(NULL), m_pDevice(NULL) { }
		CItemCtx (const CItem *pItem = NULL, CSpaceObject *pSource = NULL) : m_pItem(pItem), m_pSource(pSource), m_pArmor(NULL), m_pDevice(NULL) { }
		CItemCtx (const CItem *pItem, CSpaceObject *pSource, CInstalledArmor *pArmor) : m_pItem(pItem), m_pSource(pSource), m_pArmor(pArmor), m_pDevice(NULL) { }
		CItemCtx (const CItem *pItem, CSpaceObject *pSource, CInstalledDevice *pDevice) : m_pItem(pItem), m_pSource(pSource), m_pArmor(NULL), m_pDevice(pDevice) { }
		CItemCtx (CSpaceObject *pSource, CInstalledArmor *pArmor) : m_pItem(NULL), m_pSource(pSource), m_pArmor(pArmor), m_pDevice(NULL) { }
		CItemCtx (CSpaceObject *pSource, CInstalledDevice *pDevice) : m_pItem(NULL), m_pSource(pSource), m_pArmor(NULL), m_pDevice(pDevice) { }

		void ClearItemCache (void);
		ICCItem *CreateItemVariable (CCodeChain &CC);
		CInstalledArmor *GetArmor (void);
		CArmorClass *GetArmorClass (void);
		CInstalledDevice *GetDevice (void);
		CDeviceClass *GetDeviceClass (void);
		const CItem &GetItem (void);
		const CItemEnhancement &GetMods (void);
		inline CSpaceObject *GetSource (void) { return m_pSource; }
		inline bool IsItemNull (void) const { return (m_pItem == NULL || m_pItem->GetType() == NULL); }

	private:
		const CItem *GetItemPointer (void);

		const CItem *m_pItem;					//	The item
		CItem m_Item;							//	A cached item, if we need to cons one up.
		CSpaceObject *m_pSource;				//	Where the item is installed (may be NULL)
		CInstalledArmor *m_pArmor;				//	Installation structure (may be NULL)
		CInstalledDevice *m_pDevice;			//	Installation structure (may be NULL)
	};

class CItemType : public CDesignType
	{
	public:
		enum Flags
			{
			//	Flags for GetReference, 
			//	GetReferenceDamageAdj, 
			//	and GetReferenceDamateType
			FLAG_ACTUAL_ITEM =			0x00000001,	//	Ignore Unknown flag
			};

		enum ECachedHandlers
			{
			//	This list must match CACHED_EVENTS array in CItemType.cpp

			evtGetDescription			= 0,
			evtGetName					= 1,
			evtGetTradePrice			= 2,
			evtOnInstall				= 3,
			evtOnEnabled				= 4,
			evtOnRefuel					= 5,

			evtCount					= 6,
			};

		CItemType (void);
		virtual ~CItemType (void);

		inline bool AreChargesValued (void) const { return (m_fValueCharges ? true : false); }
		inline void ClearKnown (void) { m_fKnown = false; }
		inline void ClearShowReference (void) { m_fReference = false; }
		void CreateEmptyFlotsam (CSystem *pSystem, const CVector &vPos, const CVector &vVel, CSovereign *pSovereign, CStation **retpFlotsam);
		inline bool FindEventHandlerItemType (ECachedHandlers iEvent, SEventHandlerDesc *retEvent = NULL) const { if (retEvent) *retEvent = m_CachedEvents[iEvent]; return (m_CachedEvents[iEvent].pCode != NULL); }
		int GetApparentLevel (void) const;
		CDeviceClass *GetAmmoLauncher (int *retiVariant = NULL) const;
		inline CArmorClass *GetArmorClass (void) const { return m_pArmor; }
		ItemCategories GetCategory (void) const;
		inline int GetCharges (void) const { return (m_fInstanceData ? m_InitDataValue.Roll() : 0); }
		inline CEconomyType *GetCurrencyType (void) const { return m_iValue.GetCurrencyType(); }
		inline const CString &GetData (void) const { return m_sData; }
		const CString &GetDesc (void) const;
		inline CDeviceClass *GetDeviceClass (void) const { return m_pDevice; }
		inline CObjectImageArray &GetFlotsamImage (void) { if (!m_FlotsamImage.IsLoaded()) CreateFlotsamImage(); return m_FlotsamImage; }
		inline int GetFrequency (void) const { return m_Frequency; }
		int GetFrequencyByLevel (int iLevel);
		inline const CObjectImageArray &GetImage (void) { return m_Image; }
		int GetInstallCost (void) const;
		inline const DiceRange &GetNumberAppearing (void) const { return m_NumberAppearing; }
		inline Metric GetMass (CItemCtx &Ctx) const { return GetMassKg(Ctx) / 1000.0; }
		int GetMassKg (CItemCtx &Ctx) const;
		inline int GetMaxCharges (void) const { return (m_fInstanceData ? m_InitDataValue.GetMaxValue() : 0); }
		int GetMaxHPBonus (void) const;
		inline CWeaponFireDesc *GetMissileDesc (void) const { return m_pMissile;  }
		inline DWORD GetModCode (void) const { return m_dwModCode; }
		CString GetName (DWORD *retdwFlags, bool bActualName = false) const;
		CString GetNounPhrase (DWORD dwFlags = 0) const;
		CString GetReference (CItemCtx &Ctx, int iVariant = -1, DWORD dwFlags = 0) const;
		ItemCategories GetSlotCategory (void) const;
		CString GetSortName (void) const;
		inline CItemType *GetUnknownType (void) { return m_pUnknownType; }
		inline ICCItem *GetUseCode (void) const { return m_pUseCode; }
		inline const CString &GetUseKey (void) const { return m_sUseKey; }
		inline CXMLElement *GetUseScreen (void) const;
		inline CDesignType *GetUseScreen (CString *retsName);
		int GetValue (CItemCtx &Ctx, bool bActual = false) const;
		inline bool HasOnRefuelCode (void) const { return FindEventHandlerItemType(evtOnRefuel); }
		inline bool HasOnInstallCode (void) const { return FindEventHandlerItemType(evtOnInstall); }
		void InitRandomNames (void);
		bool IsAmmunition (void) const;
		inline bool IsArmor (void) const { return (m_pArmor != NULL); }
		inline bool IsDevice (void) const { return (m_pDevice != NULL); }
		inline bool IsKnown (void) const { return (m_fKnown ? true : false); }
		bool IsFuel (void) const;
		bool IsMissile (void) const;
		inline bool IsUsable (void) const { return ((m_pUseCode != NULL) || (m_pUseScreen != NULL)); }
		inline bool IsUsableInCockpit (void) const { return (m_pUseCode != NULL); }
		inline bool IsUsableOnlyIfEnabled (void) const { return (m_fUseEnabled ? true : false); }
		inline bool IsUsableOnlyIfInstalled (void) const { return (m_fUseInstalled ? true : false); }
		inline bool IsUsableOnlyIfUninstalled (void) const { return (m_fUseUninstalled ? true : false); }
		inline void SetKnown (void) { m_fKnown = true; }
		inline void SetShowReference (void) { m_fReference = true; }
		inline bool ShowReference (void) const { return (m_fReference ? true : false); }

		//	CDesignType overrides
		static CItemType *AsType (CDesignType *pType) { return ((pType && pType->GetType() == designItemType) ? (CItemType *)pType : NULL); }
		virtual bool FindDataField (const CString &sField, CString *retsValue);
		virtual int GetLevel (int *retiMinLevel = NULL, int *retiMaxLevel = NULL) const { if (retiMinLevel) *retiMinLevel = m_iLevel; if (retiMaxLevel) *retiMaxLevel = m_iLevel; return m_iLevel; }
		virtual DesignTypes GetType (void) const { return designItemType; }
		virtual bool IsVirtual (void) const { return (m_fVirtual ? true : false); }

		static CString GetItemCategory (ItemCategories iCategory);
		static bool ParseItemCategory (const CString &sCategory, ItemCategories *retCategory = NULL);

	protected:
		//	CDesignType overrides
		virtual void OnAddTypesUsed (TSortMap<DWORD, bool> *retTypesUsed);
		virtual ALERROR OnBindDesign (SDesignLoadCtx &Ctx);
		virtual ALERROR OnCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);
		virtual CEffectCreator *OnFindEffectCreator (const CString &sUNID);
		virtual bool OnHasSpecialAttribute (const CString &sAttrib) const;
		virtual void OnReadFromStream (SUniverseLoadCtx &Ctx);
		virtual void OnReinit (void);
		virtual void OnWriteToStream (IWriteStream *pStream);

	private:
		void CreateFlotsamImage (void);
		CStationType *GetFlotsamStationType (void);
		CString GetUnknownName (int iIndex, DWORD *retdwFlags = NULL);

		CString m_sName;						//	Full name of item
		DWORD m_dwNameFlags;					//	Name flags
		CString m_sUnknownName;					//	Name of item when unknown (may be NULL)
		CString m_sSortName;					//	Name to sort by

		int m_iLevel;							//	Level of item
		CCurrencyAndValue m_iValue;				//	Value in some currency
		int m_iMass;							//	Mass in kilograms
		FrequencyTypes m_Frequency;				//	Frequency
		DiceRange m_NumberAppearing;			//	Number appearing

		CObjectImageArray m_Image;				//	Image of item
		CString m_sDescription;					//	Description of item
		CItemTypeRef m_pUnknownType;			//	Type to show if not known
		TArray<CString> m_UnknownNames;			//	List of unknown names (if we are the unknown item placeholder)
		DiceRange m_InitDataValue;				//	Initial data value

		int m_iExtraMassPerCharge;				//	Extra mass per charge (in kilos)
		int m_iExtraValuePerCharge;				//	Extra value per charge (may be negative)

		//	Events
		SEventHandlerDesc m_CachedEvents[evtCount];

		//	Usable items
		CDockScreenTypeRef m_pUseScreen;		//	Screen shown when used (may be NULL)
		ICCItem *m_pUseCode;					//	Code when using item from cockpit
		CString m_sUseKey;						//	Hotkey to invoke usage
		DWORD m_dwModCode;						//	Mod code conveyed to items we enhance

		//	Armor
		CArmorClass *m_pArmor;					//	Armor properties (may be NULL)

		//	Devices
		CDeviceClass *m_pDevice;				//	Device properties (may be NULL)

		//	Missiles
		CWeaponFireDesc *m_pMissile;			//	Missile desc (may be NULL)

		//	Flotsam
		CG16bitImage m_FlotsamBitmap;			//	Image used for flotsam
		CObjectImageArray m_FlotsamImage;		//	Image used for flotsam

		DWORD m_fRandomDamaged:1;				//	Randomly damaged when found
		DWORD m_fKnown:1;						//	Is this type known?
		DWORD m_fReference:1;					//	Does this type show reference info?
		DWORD m_fDefaultReference:1;			//	TRUE if this shows reference by default
		DWORD m_fInstanceData:1;				//	TRUE if we need to set instance data at create time
		DWORD m_fVirtual:1;						//	TRUE if this is a virtual item needed for a weapon that invokes
		DWORD m_fUseInstalled:1;				//	If TRUE, item can only be used when installed
		DWORD m_fValueCharges:1;				//	TRUE if value should be adjusted based on charges

		DWORD m_fUseUninstalled:1;				//	If TRUE, item can only be used when uninstalled
		DWORD m_fUseEnabled:1;					//	If TRUE, item can only be used when enabled
		DWORD m_fSpare3:1;
		DWORD m_fSpare4:1;
		DWORD m_fSpare5:1;
		DWORD m_fSpare6:1;
		DWORD m_fSpare7:1;
		DWORD m_fSpare8:1;

		DWORD m_dwSpare:16;

		CString m_sData;						//	Category-specific data
	};

//	CItemTable ----------------------------------------------------------------

class CItemTable : public CDesignType
	{
	public:
		CItemTable (void);
		virtual ~CItemTable (void);

		inline void AddItems (SItemAddCtx &Ctx) { if (m_pGenerator) m_pGenerator->AddItems(Ctx); }
		inline CurrencyValue GetAverageValue (int iLevel) { return (m_pGenerator ? m_pGenerator->GetAverageValue(iLevel) : 0); }
		inline IItemGenerator *GetGenerator (void) { return m_pGenerator; }

		//	CDesignType overrides
		static CItemTable *AsType (CDesignType *pType) { return ((pType && pType->GetType() == designItemTable) ? (CItemTable *)pType : NULL); }
		virtual bool FindDataField (const CString &sField, CString *retsValue);
		virtual DesignTypes GetType (void) const { return designItemTable; }

	protected:
		//	CDesignType overrides
		virtual void OnAddTypesUsed (TSortMap<DWORD, bool> *retTypesUsed);
		virtual ALERROR OnBindDesign (SDesignLoadCtx &Ctx);
		virtual ALERROR OnCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);

	private:
		IItemGenerator *m_pGenerator;
	};

//	CShipClass ----------------------------------------------------------------

class CShipClass : public CDesignType
	{
	public:
		enum EBalanceTypes
			{
			typeUnknown,

			typeMinion,
			typeStandard,
			typeElite,
			typeBoss,
			typeNonCombatant,

			typeTooWeak,
			typeTooStrong,
			typeArmorTooWeak,
			typeArmorTooStrong,
			typeWeaponsTooWeak,
			typeWeaponsTooStrong,
			};

		enum VitalSections
			{
			sectNonCritical	= 0x00000000,		//	Ship is not automatically
												//	destroyed (chance of being
												//	destroyed based on the power
												//	of the blast relative to the
												//	original armor HPs)

			//	NOTE: These must match the order in CShipClass.cpp (ParseNonCritical)

			sectDevice0		= 0x00000001,
			sectDevice1		= 0x00000002,
			sectDevice2		= 0x00000004,
			sectDevice3		= 0x00000008,
			sectDevice4		= 0x00000010,
			sectDevice5		= 0x00000020,
			sectDevice6		= 0x00000040,
			sectDevice7		= 0x00000080,
			sectDeviceMask	= 0x000000ff,

			sectManeuver	= 0x00000100,		//	Maneuvering damaged (quarter speed turn)
			sectDrive		= 0x00000200,		//	Drive damaged (half-speed)
			sectScanners	= 0x00000400,		//	Unable to target
			sectTactical	= 0x00000800,		//	Unable to fire weapons
			sectCargo		= 0x00001000,		//	Random cargo destroyed

			sectCritical	= 0x00010000,		//	Ship destroyed
			};

		struct HullSection
			{
			int iStartAt;						//	Start of section in degrees
			int iSpan;							//	Size of section in degrees
			CArmorClassRef pArmor;				//	Type of armor for hull
			CRandomEnhancementGenerator Enhanced;//	Mods
			DWORD dwAreaSet;					//	Areas that this section protects
			};

		CShipClass (void);
		virtual ~CShipClass (void);

		inline int Angle2Direction (int iAngle) const { return m_RotationDesc.GetFrameIndex(iAngle); }
		inline int AlignToRotationAngle (int iAngle) const { return m_RotationDesc.GetRotationAngle(m_RotationDesc.GetFrameIndex(iAngle)); }
		Metric CalcMass (const CDeviceDescList &Devices) const;
		int CalcScore (void);
		void CreateEmptyWreck (CSystem *pSystem, CShip *pShip, const CVector &vPos, const CVector &vVel, CSovereign *pSovereign, CStation **retpWreck);
		void CreateExplosion (CShip *pShip, CSpaceObject *pWreck = NULL);
		void CreateWreck (CShip *pShip, CSpaceObject **retpWreck = NULL);
		inline bool FindDeviceSlotDesc (const CItem &Item, SDeviceDesc *retDesc) { return (m_pDevices ? m_pDevices->FindDefaultDesc(Item, retDesc) : false); }
		void GenerateDevices (int iLevel, CDeviceDescList &Devices);
		CString GenerateShipName (DWORD *retdwFlags);
		inline const CAISettings &GetAISettings (void) { return m_AISettings; }
		const SArmorImageDesc *GetArmorDescInherited (void);
		inline int GetCargoSpace (void) { return m_iCargoSpace; }
		inline CGenericType *GetCharacter (void) { return m_Character; }
		inline CGenericType *GetCharacterClass (void) { return m_CharacterClass; }
		inline int GetCyberDefenseLevel (void) { return m_iCyberDefenseLevel; }
		inline DWORD GetDefaultBkgnd (void) { return m_dwDefaultBkgnd; }
		inline int GetDockingPortCount (void) { return m_DockingPorts.GetCount(); }
		CVector GetDockingPortOffset (int iRotation);
		inline const TArray<CVector> &GetDockingPortPositions (void) { return m_DockingPorts; }
		void GetDriveDesc (DriveDesc *retDriveDesc) const;
		inline CObjectEffectDesc &GetEffectsDesc (void) { return m_Effects; }
		IShipGenerator *GetEscorts (void) { return m_pEscorts; }
		CWeaponFireDesc *GetExplosionType (void) { return m_pExplosionType; }
		inline CXMLElement *GetFirstDockScreen (void) { return m_pDefaultScreen.GetDesc(); }
		inline CDesignType *GetFirstDockScreen (CString *retsName) { return m_pDefaultScreen.GetDockScreen(this, retsName); }
		inline const DriveDesc *GetHullDriveDesc (void) const { return &m_DriveDesc; }
		inline int GetHullMass (void) const { return m_iMass; }
		inline int GetHullSectionCount (void) const { return m_Hull.GetCount(); }
		inline HullSection *GetHullSection (int iIndex) const { return &m_Hull[iIndex]; }
		int GetHullSectionAtAngle (int iAngle);
		inline const CObjectImageArray &GetImage (void) const { return m_Image; }
		inline const CObjectImageArray &GetImageSmall (void) { return m_Image; }
		inline const CShipInteriorDesc &GetInteriorDesc (void) const { return m_Interior; }
		inline int GetManeuverability (void) const { return m_RotationDesc.GetManeuverability(); }
		inline int GetMaxArmorMass (void) const { return m_iMaxArmorMass; }
		inline int GetMaxCargoSpace (void) const { return m_iMaxCargoSpace; }
		inline int GetMaxDevices (void) const { return m_iMaxDevices; }
		inline int GetMaxNonWeapons (void) const { return m_iMaxNonWeapons; }
		inline int GetMaxReactorPower (void) const { return m_iMaxReactorPower; }
		int GetMaxStructuralHitPoints (void) const;
		inline int GetMaxWeapons (void) const { return m_iMaxWeapons; }
		CString GetName (DWORD *retdwFlags = NULL);
		CString GetNounPhrase (DWORD dwFlags);
		inline const CPlayerSettings *GetPlayerSettings (void) const { return m_pPlayerSettings; }
		inline IItemGenerator *GetRandomItemTable (void) const { return m_pItems; }
		inline const ReactorDesc *GetReactorDesc (void) { return &m_ReactorDesc; }
		const SReactorImageDesc *GetReactorDescInherited (void);
		inline int GetRotationAngle (void) { return m_RotationDesc.GetFrameAngle(); }
		inline const CIntegralRotationDesc &GetRotationDesc (void) const { return m_RotationDesc; }
		inline int GetRotationRange (void) { return m_RotationDesc.GetFrameCount(); }
		inline int GetScore (void) { return m_iScore; }
		const SShieldImageDesc *GetShieldDescInherited (void);
		inline DWORD GetShipNameFlags (void) { return m_dwShipNameFlags; }
		CString GetShortName (void);
		inline const CString &GetClassName (void) { return m_sName; }
		inline const CString &GetManufacturerName (void) const { return m_sManufacturer; }
		inline const CString &GetShipTypeName (void) { return m_sTypeName; }
		const SWeaponImageDesc *GetWeaponDescInherited (void);
		inline int GetWreckChance (void) { return m_iLeavesWreck; }
		CObjectImageArray &GetWreckImage (void) { if (!m_WreckImage.IsLoaded()) CreateWreckImage(); return m_WreckImage; }
		void GetWreckImage (CObjectImageArray *retWreckImage);
		int GetWreckImageVariants (void);
		inline bool HasDockingPorts (void) { return (m_fHasDockingPorts ? true : false); }
		inline bool HasOnAttackedByPlayerEvent (void) const { return (m_fHasOnAttackedByPlayerEvent ? true : false); }
		inline bool HasOnOrderChangedEvent (void) const { return (m_fHasOnOrderChangedEvent ? true : false); }
		inline bool HasOnOrdersCompletedEvent (void) const { return (m_fHasOnOrdersCompletedEvent ? true : false); }
		inline bool HasShipName (void) const { return !m_sShipNames.IsBlank(); }
		void InitEffects (CShip *pShip, CObjectEffectList *retEffects);
		void InstallEquipment (CShip *pShip);
		inline bool IsDebugOnly (void) { return (m_pPlayerSettings && m_pPlayerSettings->IsDebugOnly()); }
		inline bool IsIncludedInAllAdventures (void) { return (m_pPlayerSettings && m_pPlayerSettings->IsIncludedInAllAdventures()); }
		inline bool IsPlayerShip (void) { return (m_pPlayerSettings != NULL); }
		inline bool IsShownAtNewGame (void) { return (m_pPlayerSettings && m_pPlayerSettings->IsInitialClass() && !IsVirtual()); }
		inline bool IsTimeStopImmune (void) { return (m_fTimeStopImmune ? true : false); }
		void MarkImages (bool bMarkDevices);
		void Paint (CG16bitImage &Dest, 
					int x, 
					int y, 
					const ViewportTransform &Trans, 
					int iDirection, 
					int iTick, 
					bool bThrusting = false,
					bool bRadioactive = false,
					DWORD byInvisible = 0);
		void PaintMap (CMapViewportCtx &Ctx, 
					CG16bitImage &Dest, 
					int x, 
					int y, 
					int iDirection, 
					int iTick, 
					bool bThrusting,
					bool bRadioactive);

		//	CDesignType overrides
		static CShipClass *AsType (CDesignType *pType) { return ((pType && pType->GetType() == designShipClass) ? (CShipClass *)pType : NULL); }
		virtual bool FindDataField (const CString &sField, CString *retsValue);
		virtual CCommunicationsHandler *GetCommsHandler (void);
		virtual int GetLevel (int *retiMinLevel = NULL, int *retiMaxLevel = NULL) const { if (retiMinLevel) *retiMinLevel = m_iLevel; if (retiMaxLevel) *retiMaxLevel = m_iLevel; return m_iLevel; }
		virtual CTradingDesc *GetTradingDesc (void) { return m_pTrade; }
		virtual DesignTypes GetType (void) const { return designShipClass; }
		virtual CString GetTypeName (DWORD *retdwFlags = NULL) { return GetName(retdwFlags); }
		virtual bool IsVirtual (void) const { return (m_fVirtual ? true : false); }

		static void UnbindGlobal (void);

	protected:
		//	CDesignType overrides
		virtual void OnAddTypesUsed (TSortMap<DWORD, bool> *retTypesUsed);
		virtual ALERROR OnBindDesign (SDesignLoadCtx &Ctx);
		virtual ALERROR OnCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);
		virtual CEffectCreator *OnFindEffectCreator (const CString &sUNID);
		virtual ALERROR OnFinishBindDesign (SDesignLoadCtx &Ctx);
		virtual bool OnHasSpecialAttribute (const CString &sAttrib) const;
		virtual void OnInitFromClone (CDesignType *pSource);
		virtual void OnMarkImages (void) { MarkImages(true); }
		virtual void OnMergeType (CDesignType *pSource);
		virtual void OnReadFromStream (SUniverseLoadCtx &Ctx);
		virtual void OnReinit (void);
		virtual void OnUnbindDesign (void);
		virtual void OnWriteToStream (IWriteStream *pStream);

	private:
		enum PrivateConstants
			{
			maxExhaustImages = 5
			};

		struct SEquipmentDesc
			{
			Abilities iEquipment;
			bool bInstall;
			};

		struct SExhaustDesc
			{
			C3DConversion PosCalc;
			};

		EBalanceTypes CalcBalanceType (CString *retsDesc = NULL) const;
		CItemType *CalcBestMissile (const SDeviceDesc &Device) const;
		Metric CalcCombatStrength (void) const;
		Metric CalcDamageRate (int *retiAveWeaponLevel = NULL, int *retiMaxWeaponLevel = NULL) const;
		Metric CalcDefenseRate (void) const;
		inline Metric CalcDodgeRate (void) const { return CalcManeuverValue(true); }
		int CalcLevel (void) const;
		Metric CalcManeuverValue (bool bDodge = false) const;
		int ComputeDeviceLevel (const SDeviceDesc &Device) const;
		void ComputeMovementStats (CDeviceDescList &Devices, int *retiSpeed, int *retiThrust, int *retiManeuver);
		int ComputeScore (const CDeviceDescList &Devices,
						  int iArmorLevel,
						  int iPrimaryWeapon,
						  int iSpeed,
						  int iThrust,
						  int iManeuver,
						  bool bPrimaryIsLauncher);
		void CreateWreckImage (void);
		void FindBestMissile (CDeviceClass *pLauncher, IItemGenerator *pItems, CItemType **retpMissile) const;
		void FindBestMissile (CDeviceClass *pLauncher, const CItemList &Items, CItemType **retpMissile) const;
		CString GetGenericName (DWORD *retdwFlags = NULL);
		inline int GetManeuverDelay (void) const { return m_RotationDesc.GetManeuverDelay(); }
		CPlayerSettings *GetPlayerSettingsInherited (void) const;
		CStationType *GetWreckDesc (void);
		void InitShipNamesIndices (void);
		void PaintThrust (CG16bitImage &Dest, 
						int x, 
						int y, 
						const ViewportTransform &Trans, 
						int iDirection, 
						int iTick,
						bool bInFrontOnly);

		CString m_sManufacturer;				//	Name of manufacturer
		CString m_sName;						//	Class name
		CString m_sTypeName;					//	Name of type
		DWORD m_dwClassNameFlags;				//	Flags for class name

		CString m_sShipNames;					//	Names to use for individual ship
		DWORD m_dwShipNameFlags;				//	Flags for ship name
		TArray<int> m_ShipNamesIndices;			//	Shuffled indices for ship names
		int m_iShipName;						//	Current ship name index

		int m_iScore;							//	Score when destroyed
		int m_iLevel;							//	Ship class level

		int m_iMass;							//	Empty mass (tons)
		int m_iSize;							//	Length in meters
		int m_iCargoSpace;						//	Available cargo space (tons)
		CIntegralRotationDesc m_RotationDesc;	//	Rotation and maneuverability
		double m_rThrustRatio;					//	If non-zero, then m_DriveDesc thrust is set based on this.
		DriveDesc m_DriveDesc;					//	Drive descriptor
		ReactorDesc m_ReactorDesc;				//	Reactor descriptor
		int m_iCyberDefenseLevel;				//	Cyber defense level

		int m_iMaxArmorMass;					//	Max mass of single armor segment
		int m_iMaxCargoSpace;					//	Max amount of cargo space with expansion (tons)
		int m_iMaxReactorPower;					//	Max compatible reactor power
		int m_iMaxDevices;						//	Max number of devices
		int m_iMaxWeapons;						//	Max number of weapon devices (including launchers)
		int m_iMaxNonWeapons;					//	Max number of non-weapon devices

		int m_iLeavesWreck;						//	Chance that it leaves a wreck
		int m_iStructuralHP;					//	Structual hp of wreck
		CStationTypeRef m_pWreckType;				//	Station type to use as wreck

		TArray<HullSection> m_Hull;
		CShipInteriorDesc m_Interior;			//	Interior structure
		IDeviceGenerator *m_pDevices;			//	Generator of devices
		CDeviceDescList m_AverageDevices;		//	Average complement of devices (only for stats)

		TArray<SEquipmentDesc> m_Equipment;		//	Initial equipment

		CAISettings m_AISettings;				//	AI controller data
		CPlayerSettings *m_pPlayerSettings;		//	Player settings data
		IItemGenerator *m_pItems;				//	Random items

		//	Escorts
		IShipGenerator *m_pEscorts;				//	Escorts

		//	Character
		CGenericTypeRef m_CharacterClass;		//	Character class
		CGenericTypeRef m_Character;			//	Character for ship

		//	Docking
		TArray<CVector> m_DockingPorts;			//	Position of docking ports
		CDockScreenTypeRef m_pDefaultScreen;	//	Default screen
		DWORD m_dwDefaultBkgnd;					//	Default background screen
		CTradingDesc *m_pTrade;					//	Trade descriptors

		CCommunicationsHandler m_OriginalCommsHandler;
		CCommunicationsHandler m_CommsHandler;	//	Communications handler

		//	Image
		CObjectImageArray m_Image;				//	Image of ship
		CObjectEffectDesc m_Effects;			//	Effects for ship

		//	Wreck image
		CG16bitImage m_WreckBitmap;				//	Image to use when ship is wrecked
		CObjectImageArray m_WreckImage;			//	Image to use when ship is wrecked

		//	Explosion
		CWeaponFireDescRef m_pExplosionType;	//	Explosion to create when ship is destroyed

		//	Exhaust
		CObjectImageArray m_ExhaustImage;		//	Image of drive exhaust
		TArray<SExhaustDesc> m_Exhaust;			//	Drive exhaust painting

		//	Misc
		DWORD m_fRadioactiveWreck:1;			//	TRUE if wreck is always radioactive
		DWORD m_fHasDockingPorts:1;				//	TRUE if ship has docking ports
		DWORD m_fTimeStopImmune:1;				//	TRUE if ship is immune to stop-time
		DWORD m_fHasOnOrderChangedEvent:1;		//	TRUE if ship has an event on each new order
		DWORD m_fHasOnAttackedByPlayerEvent:1;	//	TRUE if ship has an event when attacked by player
		DWORD m_fHasOnOrdersCompletedEvent:1;	//	TRUE if ship has an event when orders complete
		DWORD m_fCommsHandlerInit:1;			//	TRUE if comms handler has been initialized
		DWORD m_fVirtual:1;						//	TRUE if ship class is virtual (e.g., a base class)

		DWORD m_fInheritedPlayerSettings:1;		//	TRUE if m_pPlayerSettings is inherited from a base class
		DWORD m_fScoreOverride:1;				//	TRUE if score is specified in XML
		DWORD m_fLevelOverride:1;				//	TRUE if level is specified in XML
		DWORD m_fInheritedDevices:1;			//	TRUE if m_pDevices is inherited from another class
		DWORD m_fInheritedItems:1;				//	TRUE if m_pItems is inherited from another class
		DWORD m_fInheritedEscorts:1;			//	TRUE if m_pEscorts is inherited from another class
		DWORD m_fCyberDefenseOverride:1;		//	TRUE if cyberDefenseLevel is specified in XML
		DWORD m_fInheritedTrade:1;				//	TRUE if m_pTrade is inherited from another class

		DWORD m_fSpare:16;
	};

//	CEffectCreator ------------------------------------------------------------

class CCreatePainterCtx
	{
	public:
		CCreatePainterCtx (void) :
				m_iLifetime(0),
				m_pWeaponFireDesc(NULL),
				m_bUseObjectCenter(false),
				m_bTracking(false),
				m_pData(NULL)
			{ }

		~CCreatePainterCtx (void);

		void AddDataInteger (const CString &sField, int iValue);
		ICCItem *GetData (void);
		inline int GetLifetime (void) const { return m_iLifetime; }
		inline bool IsTracking (void) const { return m_bTracking; }
		inline void SetLifetime (int iLifetime) { m_iLifetime = iLifetime; }
		inline void SetTrackingObject (bool bValue = true) { m_bTracking = bValue; }
		inline void SetUseObjectCenter (bool bValue = true) { m_bUseObjectCenter = bValue; }
		inline void SetWeaponFireDesc (CWeaponFireDesc *pDesc) { m_pWeaponFireDesc = pDesc; }
		inline bool UseObjectCenter (void) const { return m_bUseObjectCenter; }

	private:
		struct SDataEntry
			{
			CString sField;
			int iValue;
			};

		void SetWeaponFireDescData (CCodeChain &CC, CCSymbolTable *pTable, CWeaponFireDesc *pDesc);

		int m_iLifetime;						//	Optional lifetime 0 = use creator defaults; -1 = infinite;
		CWeaponFireDesc *m_pWeaponFireDesc;		//	Optional weapon fire desc
		TArray<SDataEntry> m_Data;				//	Data to add

		bool m_bUseObjectCenter;				//	If TRUE, particle clouds always use the object as center
		bool m_bTracking;						//	If TRUE, object sets velocity

		ICCItem *m_pData;						//	Generated data
	};

class CEffectCreator : public CDesignType
	{
	public:
		enum ECachedHandlers
			{
			evtGetParameters			= 0,

			evtCount					= 1,
			};

		enum EInstanceTypes
			{
			instGame,
			instOwner,
			instCreator,
			};

		CEffectCreator (void);
		virtual ~CEffectCreator (void);

		static ALERROR CreateBeamEffect (SDesignLoadCtx &Ctx, CXMLElement *pDesc, const CString &sUNID, CEffectCreator **retpCreator);
		static ALERROR CreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, const CString &sUNID, CEffectCreator **retpCreator);
		static ALERROR CreateSimpleFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, const CString &sUNID, CEffectCreator **retpCreator);
		static ALERROR CreateTypeFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, CEffectCreator **retpCreator);
		static ALERROR CreateFromTag (const CString &sTag, CEffectCreator **retpCreator);
		static IEffectPainter *CreatePainterFromStream (SLoadCtx &Ctx, bool bNullCreator = false);
		static IEffectPainter *CreatePainterFromStreamAndCreator (SLoadCtx &Ctx, CEffectCreator *pCreator);
		static CEffectCreator *FindEffectCreator (const CString &sUNID);
		inline bool FindEventHandlerEffectType (ECachedHandlers iEvent, SEventHandlerDesc *retEvent = NULL) const { if (retEvent) *retEvent = m_CachedEvents[iEvent]; return (m_CachedEvents[iEvent].pCode != NULL); }
		static void WritePainterToStream (IWriteStream *pStream, IEffectPainter *pPainter);

		inline CWeaponFireDesc *GetDamageDesc (void) { return m_pDamage; }
		inline EInstanceTypes GetInstance (void) const { return m_iInstance; }
		inline const CString &GetUNIDString (void) { return m_sUNID; }
		bool IsValidUNID (void);
		void PlaySound (CSpaceObject *pSource = NULL);

		//	Virtuals

		virtual ALERROR CreateEffect (CSystem *pSystem,
									  CSpaceObject *pAnchor,
									  const CVector &vPos,
									  const CVector &vVel,
									  int iRotation,
									  int iVariant = 0,
									  CSpaceObject **retpEffect = NULL);
		virtual IEffectPainter *CreatePainter (CCreatePainterCtx &Ctx) = 0;
		virtual int GetLifetime (void) { return 0; }
		virtual CEffectCreator *GetSubEffect (int iIndex) { return NULL; }
		virtual CString GetTag (void) = 0;
		virtual void SetLifetime (int iLifetime) { }
		virtual void SetVariants (int iVariants) { }

		//	CDesignType overrides
		static CEffectCreator *AsType (CDesignType *pType) { return ((pType && pType->GetType() == designEffectType) ? (CEffectCreator *)pType : NULL); }
		virtual DesignTypes GetType (void) const { return designEffectType; }

	protected:
		//	CDesignType overrides
		virtual ALERROR OnBindDesign (SDesignLoadCtx &Ctx);
		virtual ALERROR OnCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);
		virtual CEffectCreator *OnFindEffectCreator (const CString &sUNID);
		virtual bool OnFindEventHandler (const CString &sEvent, SEventHandlerDesc *retEvent = NULL) const;

		//	CEffectCreator overrides

		virtual ALERROR OnEffectCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, const CString &sUNID) { return NOERROR; }
		virtual ALERROR OnEffectBindDesign (SDesignLoadCtx &Ctx) { return NOERROR; }
		virtual void OnEffectPlaySound (CSpaceObject *pSource);

		void InitPainterParameters (CCreatePainterCtx &Ctx, IEffectPainter *pPainter);

	private:
		ALERROR InitBasicsFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);

		CString m_sUNID;
		CSoundRef m_Sound;
		EInstanceTypes m_iInstance;

		CWeaponFireDesc *m_pDamage;

		//	Events
		CEventHandler m_Events;				//	Local events
		SEventHandlerDesc m_CachedEvents[evtCount];
	};

//	COverlayType ----------------------------------------------------------

class COverlayType : public CDesignType
	{
	public:
		enum ECounterDisplay
			{
			counterNone,						//	Do not show a counter
			counterProgress,					//	Show as progress bar
			counterRadius,						//	Show as circle of given radius (pixels)
			};

		COverlayType(void);
		virtual ~COverlayType(void);

		bool AbsorbsWeaponFire (CInstalledDevice *pWeapon);
		inline bool Disarms (void) const { return m_fDisarmShip; }
		inline WORD GetCounterColor (void) const { return m_wCounterColor; }
		inline const CString &GetCounterLabel (void) const { return m_sCounterLabel; }
		inline int GetCounterMax (void) const { return m_iCounterMax; }
		inline ECounterDisplay GetCounterStyle (void) const { return m_iCounterType; }
		int GetDamageAbsorbed (CSpaceObject *pSource, SDamageCtx &Ctx);
		inline Metric GetDrag (void) const { return m_rDrag; }
		inline CEffectCreator *GetEffectCreator (void) const { return m_pEffect; }
		inline CEffectCreator *GetHitEffectCreator (void) const { return m_pHitEffect; }
		int GetWeaponBonus (CInstalledDevice *pDevice, CSpaceObject *pSource);
		inline bool HasOnUpdateEvent (void) { return m_fHasOnUpdateEvent; }
		inline bool IsHitEffectAlt (void) { return m_fAltHitEffect; }
		inline bool IsShieldOverlay (void) { return m_fShieldOverlay; }
		inline bool IsShipScreenDisabled (void) { return m_fDisableShipScreen; }
		inline bool Paralyzes (void) const { return m_fParalyzeShip; }
		inline bool RotatesWithShip (void) { return m_fRotateWithShip; }
		inline bool Spins (void) const { return m_fSpinShip; }

		//	CDesignType overrides
		static COverlayType *AsType(CDesignType *pType) { return ((pType && pType->GetType() == designEnergyFieldType) ? (COverlayType *)pType : NULL); }
		virtual bool FindDataField (const CString &sField, CString *retsValue);
		virtual DesignTypes GetType (void) const { return designEnergyFieldType; }

	protected:
		//	CDesignType overrides
		virtual void OnAddTypesUsed (TSortMap<DWORD, bool> *retTypesUsed);
		virtual ALERROR OnBindDesign (SDesignLoadCtx &Ctx);
		virtual ALERROR OnCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);
		virtual CEffectCreator *OnFindEffectCreator (const CString &sUNID);

	private:
		int m_iAbsorbAdj[damageCount];			//	Damage absorbed by the field
		DamageTypeSet m_WeaponSuppress;			//	Types of weapons suppressed
		int m_iBonusAdj[damageCount];			//	Adjustment to weapons damage
		Metric m_rDrag;							//	Drag coefficient (1.0 = no drag)

		CEffectCreator *m_pEffect;				//	Effect for field
		CEffectCreator *m_pHitEffect;			//	Effect when field is hit by damage

		ECounterDisplay m_iCounterType;			//	Type of counter to paint
		CString m_sCounterLabel;				//	Label for counter
		int m_iCounterMax;						//	Max value of counter (for progress bar)
		WORD m_wCounterColor;					//	Counter color

		DWORD m_fHasOnUpdateEvent:1;			//	TRUE if we have OnUpdate
		DWORD m_fAltHitEffect:1;				//	If TRUE, hit effect replaces normal effect
		DWORD m_fRotateWithShip:1;				//	If TRUE, we rotate along with source rotation
		DWORD m_fShieldOverlay:1;				//	If TRUE, we are above hull/armor
		DWORD m_fParalyzeShip:1;				//	If TRUE, ship is paralyzed
		DWORD m_fDisarmShip:1;					//	If TRUE, ship is disarmed
		DWORD m_fDisableShipScreen:1;			//	If TRUE, player cannot bring up ship screen
		DWORD m_fSpinShip:1;					//	If TRUE, ship spins uncontrollably

		DWORD m_dwSpare:24;
	};

//	CSystemType ---------------------------------------------------------------

class CSystemType : public CDesignType
	{
	public:
		enum ECachedHandlers
			{
			evtOnObjJumpPosAdj			= 0,

			evtCount					= 1,
			};

		CSystemType (void);
		virtual ~CSystemType (void);

		inline bool FindEventHandlerSystemType (ECachedHandlers iEvent, SEventHandlerDesc *retEvent = NULL) const { if (retEvent) *retEvent = m_CachedEvents[iEvent]; return (m_CachedEvents[iEvent].pCode != NULL); }
		ALERROR FireOnCreate (SSystemCreateCtx &SysCreateCtx, CString *retsError = NULL);
		bool FireOnObjJumpPosAdj (CSpaceObject *pPos, CVector *iovPos);
		ALERROR FireSystemCreateCode (SSystemCreateCtx &SysCreateCtx, ICCItem *pCode, const COrbit &OrbitDesc, CString *retsError);
		inline DWORD GetBackgroundUNID (void) { return m_dwBackgroundUNID; }
		inline CXMLElement *GetDesc (void) { return m_pDesc; }
		inline CXMLElement *GetLocalSystemTables (void) { return m_pLocalTables; }
		inline Metric GetSpaceScale (void) const { return m_rSpaceScale; }
		inline Metric GetTimeScale (void) const { return m_rTimeScale; }
		inline bool HasExtraEncounters (void) const { return !m_bNoExtraEncounters; }
		inline bool HasRandomEncounters (void) const { return !m_bNoRandomEncounters; }

		//	CDesignType overrides
		static CSystemType *AsType (CDesignType *pType) { return ((pType && pType->GetType() == designSystemType) ? (CSystemType *)pType : NULL); }
		virtual DesignTypes GetType (void) const { return designSystemType; }

	protected:
		//	CDesignType overrides
		virtual ALERROR OnBindDesign (SDesignLoadCtx &Ctx);
		virtual ALERROR OnCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);

	private:
		DWORD m_dwBackgroundUNID;
		Metric m_rSpaceScale;				//	Klicks per pixel
		Metric m_rTimeScale;				//	Seconds of game time per real time

		CXMLElement *m_pDesc;				//	System definition
		CXMLElement *m_pLocalTables;		//	Local system tables

		bool m_bNoRandomEncounters;			//	TRUE if we don't have random encounters
		bool m_bNoExtraEncounters;			//	TRUE if we don't add new encounters to
											//		satisfy minimums.

		SEventHandlerDesc m_CachedEvents[evtCount];
	};

//	CDockScreenType -----------------------------------------------------------

class CDockScreenType : public CDesignType
	{
	public:
		CDockScreenType (void);
		virtual ~CDockScreenType (void);

		static CString GetStringUNID (CDesignType *pRoot, const CString &sScreen);
		inline CXMLElement *GetDesc (void) { return m_pDesc; }
		static CDesignType *ResolveScreen (CDesignType *pLocalScreen, const CString &sScreen, CString *retsScreenActual = NULL, bool *retbIsLocal = NULL);

		//	CDesignType overrides
		static CDockScreenType *AsType (CDesignType *pType) { return ((pType && pType->GetType() == designDockScreen) ? (CDockScreenType *)pType : NULL); }
		virtual DesignTypes GetType (void) const { return designDockScreen; }

	protected:
		//	CDesignType overrides
		virtual ALERROR OnBindDesign (SDesignLoadCtx &Ctx) { return NOERROR; }
		virtual ALERROR OnCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);

	private:
		CXMLElement *m_pDesc;
	};

class CStationEncounterDesc
	{
	public:
		CStationEncounterDesc (void) :
				m_bSystemCriteria(false),
				m_bAutoLevelFrequency(false),
				m_bNumberAppearing(false),
				m_bMaxCountLimit(false),
				m_iMaxCountInSystem(-1)
			{ }

		int CalcLevelFromFrequency (void) const;
		ALERROR InitFromStationTypeXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);
		ALERROR InitFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);
		void InitLevelFrequency (void);
		inline bool CanBeRandomlyEncountered (void) const { return (!m_sLevelFrequency.IsBlank() || m_bNumberAppearing); }
		int GetCountOfRandomEncounterLevels (void) const;
		inline Metric GetEnemyExclusionRadius (void) const { return m_rEnemyExclusionRadius; }
		int GetFrequencyByLevel (int iLevel) const;
		int GetFrequencyByNode (CTopologyNode *pSystem, CStationType *pType) const;
		inline const CString &GetLocationCriteria (void) { return m_sLocationCriteria; }
		inline int GetMaxAppearing (void) const { return (m_bMaxCountLimit ? m_MaxAppearing.Roll() : -1); }
		inline int GetNumberAppearing (void) const { return (m_bNumberAppearing ? m_NumberAppearing.Roll() : -1); }
		inline bool IsUniqueInSystem (void) const { return (m_iMaxCountInSystem == 1); }
		void ReadFromStream (SUniverseLoadCtx &Ctx);
		void WriteToStream (IWriteStream *pStream);

	private:
		bool m_bSystemCriteria;				//	If TRUE we have system criteria
		CTopologyNode::SCriteria m_SystemCriteria;	//	System criteria

		CString m_sLevelFrequency;			//	String array of frequency distribution by level
		CString m_sLocationCriteria;		//	Criteria for location
		Metric m_rEnemyExclusionRadius;		//	No enemy stations within this radius
		bool m_bAutoLevelFrequency;			//	We generated m_sLevelFrequency and need to save it.

		bool m_bNumberAppearing;			//	If TRUE, must create this exact number in game
		DiceRange m_NumberAppearing;		//	Create this number in the game

		bool m_bMaxCountLimit;				//	If FALSE, no limit
		DiceRange m_MaxAppearing;

		int m_iMaxCountInSystem;			//	-1 means no limit
	};

class CStationEncounterCtx
	{
	public:
		void AddEncounter (CSystem *pSystem);
		bool CanBeEncountered (const CStationEncounterDesc &Desc);
		bool CanBeEncounteredInSystem (CSystem *pSystem, CStationType *pStationType, const CStationEncounterDesc &Desc);
		int GetFrequencyByLevel (int iLevel, const CStationEncounterDesc &Desc);
		int GetFrequencyForNode (CTopologyNode *pNode, CStationType *pStation, const CStationEncounterDesc &Desc);
		int GetFrequencyForSystem (CSystem *pSystem, CStationType *pStation, const CStationEncounterDesc &Desc);
		int GetMinimumForNode (CTopologyNode *pNode, const CStationEncounterDesc &Desc);
		int GetRequiredForNode (CTopologyNode *pNode, const CStationEncounterDesc &Desc);
		inline int GetTotalCount (void) const { return m_Total.iCount; }
		inline int GetTotalLimit (void) const { return m_Total.iLimit; }
		inline int GetTotalMinimum (void) const { return m_Total.iMinimum; }
		void IncMinimumForNode (CTopologyNode *pNode, const CStationEncounterDesc &Desc, int iInc = 1);
		void ReadFromStream (SUniverseLoadCtx &Ctx);
		void Reinit (const CStationEncounterDesc &Desc);
		void WriteToStream (IWriteStream *pStream);

	private:
		struct SEncounterStats
			{
			SEncounterStats (void) :
					iCount(0),
					iLimit(-1),
					iMinimum(0)
				{ }

			int iCount;						//	Number of times encountered
			int iLimit;						//	Encounter limit (-1 = no limit)
			int iMinimum;					//	Minimum encounters (-1 = no limit)
			};

		SEncounterStats m_Total;			//	Encounters in entire game
		TSortMap<int, SEncounterStats> m_ByLevel;	//	Encounters by system level
		TSortMap<CString, SEncounterStats> m_ByNode;	//	Encounters by topology node
	};

//	Trading --------------------------------------------------------------------

class CTradingDesc
	{
	public:
		enum Flags
			{
			FLAG_NO_INVENTORY_CHECK =	0x00000001,	//	Do not check to see if item exists
			FLAG_NO_DONATION =			0x00000002,	//	Do not return prices for donations
			FLAG_NO_PLAYER_ADJ =		0x00000004,	//	Do not compute special player adjs
			};

		CTradingDesc (void);
		~CTradingDesc (void);

		inline void AddBuyOrder (CItemType *pType, const CString &sCriteria, int iPriceAdj)
			{ AddOrder(pType, sCriteria, iPriceAdj, FLAG_BUYS); }
		inline void AddSellOrder (CItemType *pType, const CString &sCriteria, int iPriceAdj)
			{ AddOrder(pType, sCriteria, iPriceAdj, FLAG_SELLS); }
		bool Buys (CSpaceObject *pObj, const CItem &Item, DWORD dwFlags, int *retiPrice = NULL, int *retiMaxCount = NULL);
		int Charge (CSpaceObject *pObj, int iCharge);
		bool GetArmorInstallPrice (CSpaceObject *pObj, const CItem &Item, DWORD dwFlags, int *retiPrice) const;
		bool GetArmorRepairPrice (CSpaceObject *pObj, const CItem &Item, int iHPToRepair, DWORD dwFlags, int *retiPrice) const;
		bool GetDeviceInstallPrice (CSpaceObject *pObj, const CItem &Item, DWORD dwFlags, int *retiPrice) const;
		bool GetDeviceRemovePrice (CSpaceObject *pObj, const CItem &Item, DWORD dwFlags, int *retiPrice) const;
		inline CEconomyType *GetEconomyType (void) { return m_pCurrency; }
		inline int GetMaxCurrency (void) { return m_iMaxCurrency; }
		int GetMaxLevelMatched (ETradeServiceTypes iService) const;
		bool GetRefuelItemAndPrice (CSpaceObject *pObj, CSpaceObject *pObjToRefuel, DWORD dwFlags, CItemType **retpItemType, int *retiPrice) const;
		inline int GetReplenishCurrency (void) { return m_iReplenishCurrency; }
		bool Sells (CSpaceObject *pObj, const CItem &Item, DWORD dwFlags, int *retiPrice = NULL);
		void SetEconomyType (CEconomyType *pCurrency) { m_pCurrency.Set(pCurrency); }
		void SetMaxCurrency (int iMaxCurrency) { m_iMaxCurrency = iMaxCurrency; }
		void SetReplenishCurrency (int iReplenishCurrency) { m_iReplenishCurrency = iReplenishCurrency; }

		static ALERROR CreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, CTradingDesc **retpTrade);
		void OnCreate (CSpaceObject *pObj);
		ALERROR OnDesignLoadComplete (SDesignLoadCtx &Ctx);
		void OnUpdate (CSpaceObject *pObj);
		void ReadFromStream (SLoadCtx &Ctx);
		void RefreshInventory (CSpaceObject *pObj, int iPercent = 100);
		void WriteToStream (IWriteStream *pStream);

		static int CalcPriceForService (ETradeServiceTypes iService, CSpaceObject *pProvider, const CItem &Item, int iCount, DWORD dwFlags);
		static CString ServiceToString (ETradeServiceTypes iService);

	private:
		enum InternalFlags
			{
			//	Flags for SServiceDesc

			FLAG_ACTUAL_PRICE =			0x00000004,	//	TRUE if we compute actual price
			FLAG_INVENTORY_ADJ =		0x00000008,	//	TRUE if we adjust the inventory

			//	DEPRECATED: We don't store these flags, but we require the values
			//	for older versions.

			FLAG_SELLS =				0x00000001,	//	TRUE if station sells this item type
			FLAG_BUYS =					0x00000002,	//	TRUE if station buys this item type
			FLAG_ACCEPTS_DONATIONS =	0x00000010,	//	TRUE if we accept donations
			};

		struct SServiceDesc
			{
			ETradeServiceTypes iService;				//	Type of service
			CString sID;						//	ID of order

			CItemTypeRef pItemType;				//	Item type
			CItemCriteria ItemCriteria;			//	If ItemType is NULL, this is the criteria

			CFormulaText PriceAdj;				//	Price adjustment
			CFormulaText InventoryAdj;			//	% of item count found at any one time

			DWORD dwFlags;						//	Flags
			};

		void AddOrder (CItemType *pItemType, const CString &sCriteria, int iPriceAdj, DWORD dwFlags);
		CString ComputeID (ETradeServiceTypes iService, DWORD dwUNID, const CString &sCriteria, DWORD dwFlags);
		int ComputeMaxCurrency (CSpaceObject *pObj);
		inline int ComputePrice (CSpaceObject *pObj, const CItem &Item, int iCount, const SServiceDesc &Commodity, DWORD dwFlags) const { return ComputePrice(pObj, m_pCurrency, Item, iCount, Commodity, dwFlags); }
		bool FindService (ETradeServiceTypes iService, const CItem &Item, const SServiceDesc **retpDesc);
		bool Matches (const CItem &Item, const SServiceDesc &Commodity) const;
		void ReadServiceFromFlags (DWORD dwFlags, ETradeServiceTypes *retiService, DWORD *retdwFlags);
		bool SetInventoryCount (CSpaceObject *pObj, SServiceDesc &Desc, CItemType *pItemType);

		static int ComputePrice (CSpaceObject *pObj, CEconomyType *pCurrency, const CItem &Item, int iCount, const SServiceDesc &Commodity, DWORD dwFlags);

		CEconomyTypeRef m_pCurrency;
		int m_iMaxCurrency;
		int m_iReplenishCurrency;

		TArray<SServiceDesc> m_List;
	};

//	CStationType --------------------------------------------------------------

const int STATION_REPAIR_FREQUENCY =	30;

class CStationType : public CDesignType
	{
	public:
		enum ESizeClass
			{
			sizeNone =				0,

			//	World size classes

			worldSizeA =			1,		//	1-24 km
			worldSizeB =			2,		//	25-74 km
			worldSizeC =			3,		//	75-299 km
			worldSizeD =			4,		//	300-749 km

			worldSizeE =			5,		//	750-1,499 km
			worldSizeF =			6,		//	1,500-2,999 km
			worldSizeG =			7,		//	3,000-4,499 km

			worldSizeH =			8,		//	4,500-7,499 km
			worldSizeI =			9,		//	7,500-14,999 km
			worldSizeJ =			10,		//	15,000-29,999 km

			worldSizeK =			11,		//	30,000-74,999 km
			worldSizeL =			12,		//	75,000-149,999 km
			worldSizeM =			13,		//	150,000+ km
			};

		CStationType (void);
		virtual ~CStationType (void);
		static void Reinit (void);

		inline bool AlertWhenAttacked (void) { return (mathRandom(1, 100) <= m_iAlertWhenAttacked); }
		inline bool AlertWhenDestroyed (void) { return (mathRandom(1, 100) <= m_iAlertWhenDestroyed); }
		inline bool CanAttack (void) const { return (m_fCanAttack ? true : false); }
		inline bool CanBeEncountered (void) { return m_EncounterRecord.CanBeEncountered(m_RandomPlacement); }
		inline bool CanBeEncountered (CSystem *pSystem) { return m_EncounterRecord.CanBeEncounteredInSystem(pSystem, this, m_RandomPlacement); }
		inline bool CanBeEncounteredRandomly (void) { return m_RandomPlacement.CanBeRandomlyEncountered(); }
		inline bool CanBeHitByFriends (void) { return (m_fNoFriendlyTarget ? false : true); }
		inline bool CanHitFriends (void) { return (m_fNoFriendlyFire ? false : true); }
		CString GenerateRandomName (const CString &sSubst, DWORD *retdwFlags);
		inline CXMLElement *GetAbandonedScreen (void) { return m_pAbandonedDockScreen.GetDesc(); }
		inline CDesignType *GetAbandonedScreen (CString *retsName) { return m_pAbandonedDockScreen.GetDockScreen(this, retsName); }
		inline CArmorClass *GetArmorClass (void) { return (m_pArmor ? m_pArmor->GetArmorClass() : NULL); }
		inline CEffectCreator *GetBarrierEffect (void) { return m_pBarrierEffect; }
		inline IShipGenerator *GetConstructionTable (void) { return m_pConstruction; }
		CSovereign *GetControllingSovereign (void);
		inline DWORD GetDefaultBkgnd (void) { return m_dwDefaultBkgnd; }
		inline CXMLElement *GetDesc (void) { return m_pDesc; }
		inline CString GetDestNodeID (void) { return m_sStargateDestNode; }
		inline CString GetDestEntryPoint (void) { return m_sStargateDestEntryPoint; }
		inline int GetEjectaAdj (void) { return m_iEjectaAdj; }
		CWeaponFireDesc *GetEjectaType (void) { return m_pEjectaType; }
		inline Metric GetEnemyExclusionRadius (void) const { return m_RandomPlacement.GetEnemyExclusionRadius(); }
		CWeaponFireDesc *GetExplosionType (void) { return m_pExplosionType; }
		inline int GetEncounterFrequency (void) { return m_iEncounterFrequency; }
		inline int GetEncounterMinimum (CTopologyNode *pNode) { return m_EncounterRecord.GetMinimumForNode(pNode, m_RandomPlacement); }
		inline CStationEncounterCtx &GetEncounterRecord (void) { return m_EncounterRecord; }
		inline int GetEncounterRequired (CTopologyNode *pNode) { return m_EncounterRecord.GetRequiredForNode(pNode, m_RandomPlacement); }
		inline IShipGenerator *GetEncountersTable (void) { return m_pEncounters; }
		inline int GetFireRateAdj (void) { return m_iFireRateAdj; }
		inline CXMLElement *GetFirstDockScreen (void) { return m_pFirstDockScreen.GetDesc(); }
		inline CDesignType *GetFirstDockScreen (CString *retsName) { return m_pFirstDockScreen.GetDockScreen(this, retsName); }
		inline int GetFrequencyByLevel (int iLevel) { return m_EncounterRecord.GetFrequencyByLevel(iLevel, m_RandomPlacement); }
		inline int GetFrequencyForNode (CTopologyNode *pNode) { return m_EncounterRecord.GetFrequencyForNode(pNode, this, m_RandomPlacement); }
		inline int GetFrequencyForSystem (CSystem *pSystem) { return m_EncounterRecord.GetFrequencyForSystem(pSystem, this, m_RandomPlacement); }
		inline CEffectCreator *GetGateEffect (void) { return m_pGateEffect; }
		inline Metric GetGravityRadius (void) const { return m_rGravityRadius; }
		inline const CObjectImageArray &GetImage (const CCompositeImageSelector &Selector, const CCompositeImageModifiers &Modifiers, int *retiRotation = NULL) { return m_Image.GetImage(Selector, Modifiers, retiRotation); }
		inline IImageEntry *GetImageRoot (void) const { return m_Image.GetRoot(); }
		inline int GetImageVariants (void) { return m_iImageVariants; }
		inline int GetInitialHitPoints (void) { return m_iHitPoints; }
		inline IShipGenerator *GetInitialShips (int iDestiny, int *retiCount) { *retiCount = (!m_ShipsCount.IsEmpty() ? m_ShipsCount.RollSeeded(iDestiny) : 1); return m_pInitialShips; }
		Metric GetLevelStrength (int iLevel);
		inline const CString &GetLocationCriteria (void) { return m_RandomPlacement.GetLocationCriteria(); }
		inline Metric GetMass (void) { return m_rMass; }
		inline int GetMinShips (int iDestiny) { return (!m_ShipsCount.IsEmpty() ? m_ShipsCount.RollSeeded(iDestiny) : m_iMinShips); }
		inline Metric GetMaxEffectiveRange (void) { return m_rMaxAttackDistance; }
		inline int GetMaxHitPoints (void) { return m_iMaxHitPoints; }
		inline int GetMaxLightDistance (void) { return m_iMaxLightDistance; }
		inline int GetMaxShipConstruction (void) { return m_iMaxConstruction; }
		inline int GetMaxStructuralHitPoints (void) { return m_iMaxStructuralHP; }
		const CString &GetName (DWORD *retdwFlags = NULL);
		inline DWORD GetNameFlags (void) { return m_dwNameFlags; }
		CString GetNounPhrase (DWORD dwFlags);
		inline int GetNumberAppearing (void) const { return m_EncounterRecord.GetTotalMinimum(); }
		inline Metric GetParallaxDist (void) const { return m_rParallaxDist; }
		inline IItemGenerator *GetRandomItemTable (void) { return m_pItems; }
		inline DWORD GetRandomNameFlags (void) { return m_dwRandomNameFlags; }
		inline const CRegenDesc &GetRegenDesc (void) { return m_Regen; }
		IShipGenerator *GetReinforcementsTable (void);
		inline CXMLElement *GetSatellitesDesc (void) { return m_pSatellitesDesc; }
		inline ScaleTypes GetScale (void) const { return m_iScale; }
		inline int GetSize (void) const { return m_iSize; }
		inline int GetShipConstructionRate (void) { return m_iShipConstructionRate; }
		inline const CRegenDesc &GetShipRegenDesc (void) { return m_ShipRegen; }
		inline CSovereign *GetSovereign (void) const { return m_pSovereign; }
		inline COLORREF GetSpaceColor (void) { return m_rgbSpaceColor; }
		inline int GetStealth (void) const { return m_iStealth; }
		inline int GetStructuralHitPoints (void) { return m_iStructuralHP; }
		inline int GetTempChance (void) const { return m_iChance; }
		inline bool HasAnimations (void) const { return (m_pAnimations != NULL); }
		inline bool HasGravity (void) const { return (m_rGravityRadius > 0.0); }
		inline bool HasRandomNames (void) const { return !m_sRandomNames.IsBlank(); }
		inline bool HasWreckImage (void) const { return (!IsImmutable() && m_iMaxHitPoints > 0); }
		inline void IncEncounterMinimum (CTopologyNode *pNode, int iInc = 1) { m_EncounterRecord.IncMinimumForNode(pNode, m_RandomPlacement, iInc); }
		inline bool IsActive (void) { return (m_fInactive ? false : true); }
		inline bool IsOutOfPlaneObject (void) { return (m_fOutOfPlane ? true : false); }
		inline bool IsBeacon (void) { return (m_fBeacon ? true : false); }
		inline bool IsBlacklistEnabled (void) { return (m_fNoBlacklist ? false : true); }
		inline bool IsDestroyWhenEmpty (void) { return (m_fDestroyWhenEmpty ? true : false); }
		inline bool IsEnemyDockingAllowed (void) { return (m_fAllowEnemyDocking ? true : false); }
		inline bool IsImmutable (void) const { return (m_fImmutable ? true : false); }
		inline bool IsMultiHull (void) { return (m_fMultiHull ? true : false); }
		inline bool IsMobile (void) const { return (m_fMobile ? true : false); }
		inline bool IsRadioactive (void) { return (m_fRadioactive ? true : false); }
		inline bool IsSign (void) { return (m_fSign ? true : false); }
		bool IsSizeClass (ESizeClass iClass) const;
		inline bool IsShipEncounter (void) { return (m_fShipEncounter ? true : false); }
		inline bool IsStatic (void) { return (m_fStatic ? true : false); }
		inline bool IsTimeStopImmune (void) { return (m_fTimeStopImmune ? true : false); }
		inline bool IsUniqueInSystem (void) const { return m_RandomPlacement.IsUniqueInSystem(); }
		inline bool IsWall (void) { return (m_fWall ? true : false); }
		void MarkImages (const CCompositeImageSelector &Selector);
		void PaintAnimations (CG16bitImage &Dest, int x, int y, int iTick);
		void PaintDockPortPositions (CG16bitImage &Dest, int x, int y);
		void SetImageSelector (SSelectorInitCtx &InitCtx, CCompositeImageSelector *retSelector);
		inline void SetEncountered (CSystem *pSystem) { m_EncounterRecord.AddEncounter(pSystem); }
		inline void SetTempChance (int iChance) { m_iChance = iChance; }
		inline bool ShowsMapIcon (void) { return (m_fNoMapIcon ? false : true); }
		inline bool UsesReverseArticle (void) { return (m_fReverseArticle ? true : false); }

		//	CDesignType overrides
		static CStationType *AsType (CDesignType *pType) { return ((pType && pType->GetType() == designStationType) ? (CStationType *)pType : NULL); }
		virtual bool FindDataField (const CString &sField, CString *retsValue);
		virtual int GetLevel (int *retiMinLevel = NULL, int *retiMaxLevel = NULL) const;
		virtual CTradingDesc *GetTradingDesc (void) { return m_pTrade; }
		virtual DesignTypes GetType (void) const { return designStationType; }
		virtual CString GetTypeName (DWORD *retdwFlags = NULL) { return GetName(retdwFlags); }
		virtual bool IsVirtual (void) const { return (m_fVirtual ? true : false); }

		static Metric CalcSatelliteHitsToDestroy (CXMLElement *pSatellites, int iLevel, bool bIgnoreChance = false);
		static Metric CalcSatelliteStrength (CXMLElement *pSatellites, int iLevel, bool bIgnoreChance = false);
		static Metric CalcSatelliteTreasureValue (CXMLElement *pSatellites, int iLevel, bool bIgnoreChance = false);
		static ScaleTypes ParseScale (const CString sValue);
		static ESizeClass ParseSizeClass (const CString sValue);

	protected:
		//	CDesignType overrides
		virtual void OnAddTypesUsed (TSortMap<DWORD, bool> *retTypesUsed);
		virtual ALERROR OnBindDesign (SDesignLoadCtx &Ctx);
		virtual ALERROR OnCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);
		virtual ALERROR OnFinishBindDesign (SDesignLoadCtx &Ctx);
		virtual bool OnHasSpecialAttribute (const CString &sAttrib) const;
		virtual void OnMarkImages (void);
		virtual void OnReadFromStream (SUniverseLoadCtx &Ctx);
		virtual void OnReinit (void);
		virtual void OnTopologyInitialized (void);
		virtual void OnWriteToStream (IWriteStream *pStream);

	private:
		struct SAnimationSection
			{
			int m_x;
			int m_y;
			CObjectImageArray m_Image;
			};

		void AddTypesUsedByXML (CXMLElement *pElement, TSortMap<DWORD, bool> *retTypesUsed);
		Metric CalcBalance (int iLevel);
		Metric CalcDefenderStrength (int iLevel);
		int CalcHitsToDestroy (int iLevel);
		Metric CalcTreasureValue (int iLevel);
		Metric CalcWeaponStrength (int iLevel);
		CString ComposeLoadError (const CString &sError);
		void InitStationDamage (void);

		CXMLElement *m_pDesc;

		//	Basic station descriptors
		CString m_sName;								//	Name
		DWORD m_dwNameFlags;							//	Flags
		CString m_sRandomNames;							//	Random names
		DWORD m_dwRandomNameFlags;						//	Flags
		CSovereignRef m_pSovereign;						//	Sovereign
		ScaleTypes m_iScale;							//	Scale
		Metric m_rParallaxDist;							//	Parallax distance for background objects
		int m_iLevel;									//	Station level
		Metric m_rMass;									//	Mass of station
														//		For stars, this is in solar masses
														//		For worlds, this is in Earth masses
														//		Otherwise, in metric tons
		int m_iSize;									//	Size
														//		For stars and worlds, this is in kilometers
														//		Otherwise, in meters
		int m_iFireRateAdj;								//	Fire rate adjustment

		//	Armor & HP
		CItemTypeRef m_pArmor;							//	Armor class
		int m_iHitPoints;								//	Hit points at creation time
		int m_iMaxHitPoints;							//	Max hit points
		CRegenDesc m_Regen;								//	Repair rate
		int m_iStealth;									//	Stealth

		int m_iStructuralHP;							//	Initial structural hit points
		int m_iMaxStructuralHP;							//	Max structural hp (0 = station is permanent)

		//	Devices
		int m_iDevicesCount;							//	Number of devices in array
		CInstalledDevice *m_Devices;					//	Device array

		//	Items
		IItemGenerator *m_pItems;						//	Random item table
		CTradingDesc *m_pTrade;							//	Trading structure

		DWORD m_fMobile:1;								//	Station moves
		DWORD m_fWall:1;								//	Station is a wall
		DWORD m_fInactive:1;							//	Station starts inactive
		DWORD m_fDestroyWhenEmpty:1;					//	Station is destroyed when last item removed
		DWORD m_fAllowEnemyDocking:1;					//	Station allows enemies to dock
		DWORD m_fNoFriendlyFire:1;						//	Station cannot hit friends
		DWORD m_fSign:1;								//	Station is a text sign
		DWORD m_fBeacon:1;								//	Station is a nav beacon

		DWORD m_fRadioactive:1;							//	Station is radioactive
		DWORD m_fCanAttack:1;							//	Station is active (i.e., will react if attacked)
		DWORD m_fShipEncounter:1;						//	This is a ship encounter
		DWORD m_fImmutable:1;							//	Station can not take damage or become radioactive, etc.
		DWORD m_fNoMapIcon:1;							//	Do not show on map
		DWORD m_fMultiHull:1;							//	Only harmed by WMD damage
		DWORD m_fTimeStopImmune:1;						//	TRUE if station is immune to time-stop
		DWORD m_fNoBlacklist:1;							//	Does not blacklist player if attacked

		DWORD m_fReverseArticle:1;						//	Use "a" instead of "an" and vice versa
		DWORD m_fStatic:1;								//	Use CStatic instead of CStation
		DWORD m_fOutOfPlane:1;							//	Background or foreground object
		DWORD m_fNoFriendlyTarget:1;					//	Station cannot be hit by friends
		DWORD m_fVirtual:1;								//	Virtual stations do not show up
		DWORD m_fSpare6:1;
		DWORD m_fSpare7:1;
		DWORD m_fSpare8:1;

		DWORD m_dwSpare:8;

		//	Images
		CCompositeImageDesc m_Image;
		int m_iImageVariants;							//	Number of variants
		CIntArray m_ShipWrecks;							//	Class IDs to use as image (for shipwrecks)
		int m_iAnimationsCount;							//	Number of animation sections
		SAnimationSection *m_pAnimations;				//	Animation sections (may be NULL)

		//	Docking
		CDockScreenTypeRef m_pFirstDockScreen;			//	First screen (may be NULL)
		CDockScreenTypeRef m_pAbandonedDockScreen;		//	Screen to use when abandoned (may be NULL)
		DWORD m_dwDefaultBkgnd;							//	Default background screen

		//	Behaviors
		int m_iAlertWhenAttacked;						//	Chance that station will warn others when attacked
		int m_iAlertWhenDestroyed;						//	Chance that station will warn others when destroyed
		Metric m_rMaxAttackDistance;					//	Max range at which station guns attack

		//	Random occurrence
		CStationEncounterDesc m_RandomPlacement;		//	Random encounter information
		CStationEncounterCtx m_EncounterRecord;			//	Record of encounters so far

		//	Ships
		DiceRange m_ShipsCount;							//	Station should have this number of ship
		IShipGenerator *m_pInitialShips;				//	Ships at creation time
		IShipGenerator *m_pReinforcements;				//	Reinforcements table
		int m_iMinShips;								//	Min ships at station
		IShipGenerator *m_pEncounters;					//	Random encounters table
		int m_iEncounterFrequency;						//	Frequency of random encounter
		CRegenDesc m_ShipRegen;							//	Regen for ships docked with us
		IShipGenerator *m_pConstruction;				//	Ships built by station
		int m_iShipConstructionRate;					//	Ticks between each construction
		int m_iMaxConstruction;							//	Stop building when we get this many ships

		//	Satellites
		CXMLElement *m_pSatellitesDesc;

		//	Explosion
		CWeaponFireDescRef m_pExplosionType;			//	Explosion to create when station is destroyed

		//	Ejecta
		int m_iEjectaAdj;								//	Adjustment to probability for ejecta when hit by weapon
														//		0 = no chance of ejecta
														//		100 = normal chance
														//		>100 = greater than normal chance
		CWeaponFireDescRef m_pEjectaType;				//	Type of ejecta generated

		//	Stellar objects
		COLORREF m_rgbSpaceColor;						//	Space color
		int m_iMaxLightDistance;						//	Max distance at which there is no (effective) light from star
		Metric m_rGravityRadius;						//	Gravity radius

		//	Stargates
		CString m_sStargateDestNode;					//	Dest node
		CString m_sStargateDestEntryPoint;				//	Dest entry point
		CEffectCreatorRef m_pGateEffect;				//	Effect when object gates in/out of station

		//	Miscellaneous
		CEffectCreatorRef m_pBarrierEffect;				//	Effect when object hits station
		CSovereignRef m_pControllingSovereign;			//	If controlled by different sovereign
														//	(e.g., centauri occupation)
		//	Temporary
		int m_iChance;									//	Used when computing chance of encounter
	};

//	CEconomyType --------------------------------------------------------------

class CEconomyType : public CDesignType
	{
	public:
		CEconomyType (void) { }

		CurrencyValue Exchange (CEconomyType *pFrom, CurrencyValue iAmount);
		inline const CString &GetCurrencyNamePlural (void) { return m_sCurrencyPlural; }
		inline const CString &GetCurrencyNameSingular (void) { return m_sCurrencySingular; }
		inline const CString &GetSID (void) { return m_sSID; }
		inline bool IsCreditEquivalent (void) { return (m_iCreditConversion == 100); }
		static CurrencyValue ExchangeToCredits (CEconomyType *pFrom, CurrencyValue iAmount);
		static CString RinHackGet (CSpaceObject *pObj);
		static CurrencyValue RinHackInc (CSpaceObject *pObj, CurrencyValue iInc);
		static void RinHackSet (CSpaceObject *pObj, const CString &sData);

		//	CDesignType overrides
		static CEconomyType *AsType (CDesignType *pType) { return ((pType && pType->GetType() == designEconomyType) ? (CEconomyType *)pType : NULL); }
		virtual bool FindDataField (const CString &sField, CString *retsValue);
		virtual DesignTypes GetType (void) const { return designEconomyType; }

	protected:
		//	CDesignType overrides
		virtual ALERROR OnBindDesign (SDesignLoadCtx &Ctx);
		virtual ALERROR OnCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);

	private:
		CString m_sSID;									//	String ID (e.g., "credit")
		CString m_sCurrencyName;						//	Annotated name
		CString m_sCurrencySingular;					//	Singular form: "1 credit"
		CString m_sCurrencyPlural;						//	Plural form: "10 credits"; "You don't have enough credits"

		CurrencyValue m_iCreditConversion;				//	Commonwealth Credits that 100 units of the currency is worth
	};

//	CSovereign ----------------------------------------------------------------

enum DispositionClasses
	{
	alignNone =					-1,

	alignConstructiveChaos =	0,
	alignConstructiveOrder =	1,
	alignNeutral =				2,
	alignDestructiveOrder =		3,
	alignDestructiveChaos =		4,
	};

class CSovereign : public CDesignType
	{
	public:
		enum Alignments
			{
			alignUnknown =				-1,

			alignRepublic =				0,	//	constructive	community		knowledge		evolution		(constructive chaos)
			alignFederation =			1,	//	constructive	community		knowledge		tradition		(constructive order)
			alignUplifter =				2,	//	constructive	community		spirituality	evolution		(constructive chaos)
			alignFoundation =			3,	//	constructive	community		spirituality	tradition		(neutral)
			alignCompetitor =			4,	//	constructive	independence	knowledge		evolution		(neutral)
			alignArchivist =			5,	//	constructive	independence	knowledge		tradition		(constructive order)
			alignSeeker =				6,	//	constructive	independence	spirituality	evolution		(constructive chaos)
			alignHermit =				7,	//	constructive	independence	spirituality	tradition		(constructive order)

			alignCollective =			8,	//	destructive		community		knowledge		evolution		(destructive chaos)
			alignEmpire =				9,	//	destructive		community		knowledge		tradition		(destructive order)
			alignSterelizer =			10,	//	destructive		community		spirituality	evolution		(destructive chaos)
			alignCorrector =			11,	//	destructive		community		spirituality	tradition		(destructive order)
			alignMegalomaniac =			12,	//	destructive		independence	knowledge		evolution		(destructive chaos)
			alignCryptologue =			13,	//	destructive		independence	knowledge		tradition		(destructive order)
			alignPerversion =			14,	//	destructive		independence	spirituality	evolution		(destructive chaos)
			alignSolipsist =			15,	//	destructive		independence	spirituality	tradition		(destructive order)

			alignUnorganized =			16,	//	unorganized group of beings										(neutral)
			alignSubsapient =			17,	//	animals, zoanthropes, cyberorgs, and other creatures			(neutral)
			alignPredator =				18,	//	aggressive subsapient											(destructive chaos)

			alignCount =				19,
			};

		enum Disposition
			{
			dispEnemy = 0,
			dispNeutral = 1,
			dispFriend = 2,
			};

		CSovereign (void);
		~CSovereign (void);

		void DeleteRelationships (void);
		inline void FlushEnemyObjectCache (void) { m_pEnemyObjectsSystem = NULL; }
		Disposition GetDispositionTowards (CSovereign *pSovereign, bool bCheckParent = true);
		inline const CSpaceObjectList &GetEnemyObjectList (CSystem *pSystem) { InitEnemyObjectList(pSystem); return m_EnemyObjects; }
		CString GetText (MessageTypes iMsg);
		inline bool IsEnemy (CSovereign *pSovereign) { return (m_bSelfRel || (pSovereign != this)) && (GetDispositionTowards(pSovereign) == dispEnemy); }
		inline bool IsFriend (CSovereign *pSovereign) { return (!m_bSelfRel && (pSovereign == this)) || (GetDispositionTowards(pSovereign) == dispFriend); }
		static Alignments ParseAlignment (const CString &sAlign);
		void SetDispositionTowards (CSovereign *pSovereign, Disposition iDisp);

		//	CDesignType overrides
		static CSovereign *AsType (CDesignType *pType) { return ((pType && pType->GetType() == designSovereign) ? (CSovereign *)pType : NULL); }
		virtual bool FindDataField (const CString &sField, CString *retsValue);
		virtual DesignTypes GetType (void) const { return designSovereign; }

	protected:
		//	CDesignType overrides
		virtual void OnAddTypesUsed (TSortMap<DWORD, bool> *retTypesUsed);
		virtual ALERROR OnBindDesign (SDesignLoadCtx &Ctx);
		virtual ALERROR OnCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);
		virtual ALERROR OnPrepareBindDesign (SDesignLoadCtx &Ctx);
		virtual void OnPrepareReinit (void);
		virtual void OnReadFromStream (SUniverseLoadCtx &Ctx);
		virtual void OnReinit (void);
		virtual void OnWriteToStream (IWriteStream *pStream);

	private:
		struct SRelationship
			{
			CSovereign *pSovereign;
			Disposition iDisp;

			SRelationship *pNext;
			};

		bool CalcSelfRel (void);
		SRelationship *FindRelationship (CSovereign *pSovereign, bool bCheckParent = false);
		inline Alignments GetAlignment (void) { return m_iAlignment; }
		void InitEnemyObjectList (CSystem *pSystem);
		void InitRelationships (void);

		CString m_sName;
		Alignments m_iAlignment;
		CXMLElement *m_pInitialRelationships;

		SRelationship *m_pFirstRelationship;	//	List of individual relationships

		bool m_bSelfRel;						//	TRUE if relationship with itself is not friendly
		CSystem *m_pEnemyObjectsSystem;			//	System that we've cached enemy objects
		CSpaceObjectList m_EnemyObjects;		//	List of enemy objects that can attack
	};

//	CPower --------------------------------------------------------------------

class CPower : public CDesignType
	{
	public:
		CPower (void);
		virtual ~CPower (void);

		inline ICCItem *GetCode (void) { return m_pCode; }
		inline int GetInvokeCost (void) { return m_iInvokeCost; }
		inline const CString &GetInvokeKey (void) { return m_sInvokeKey; }
		inline const CString &GetName (void) { return m_sName; }
		inline ICCItem *GetOnInvokedByPlayer (void) { return m_pOnInvokedByPlayer; }
		inline ICCItem *GetOnShow (void) { return m_pOnShow; }
		void Invoke (CSpaceObject *pSource, CSpaceObject *pTarget, CString *retsError = NULL);
		void InvokeByPlayer (CSpaceObject *pSource, CSpaceObject *pTarget, CString *retsError = NULL);
		bool OnShow (CSpaceObject *pSource, CSpaceObject *pTarget, CString *retsError = NULL);
		bool OnDestroyCheck (CSpaceObject *pSource, DestructionTypes iCause, const CDamageSource &Attacker);

		//	CDesignType overrides
		static CPower *AsType (CDesignType *pType) { return ((pType && pType->GetType() == designPower) ? (CPower *)pType : NULL); }
		virtual DesignTypes GetType (void) const { return designPower; }

	protected:
		//	CDesignType overrides
		virtual ALERROR OnBindDesign (SDesignLoadCtx &Ctx) { return NOERROR; }
		virtual ALERROR OnCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);

	private:
		CString m_sName;
		int m_iInvokeCost;
		CString m_sInvokeKey;

		ICCItem *m_pCode;
		ICCItem *m_pOnShow;
		ICCItem *m_pOnInvokedByPlayer;
		ICCItem *m_pOnDestroyCheck;
	};

//	CSpaceEnvironmentType -----------------------------------------------------

class CSpaceEnvironmentType : public CDesignType
	{
	public:
		CSpaceEnvironmentType (void) { }

		ALERROR FireOnUpdate (CSpaceObject *pObj, CString *retsError = NULL);
		inline Metric GetDragFactor (void) { return m_rDragFactor; }
		inline bool HasOnUpdateEvent (void) { return m_bHasOnUpdateEvent; }
		inline bool IsLRSJammer (void) { return m_bLRSJammer; }
		inline bool IsShieldJammer (void) { return m_bShieldJammer; }
		inline bool IsSRSJammer (void) { return m_bSRSJammer; }
		void Paint (CG16bitImage &Dest, int x, int y, int xTile, int yTile, DWORD dwEdgeMask);
		void PaintLRS (CG16bitImage &Dest, int x, int y);
		void PaintMap (CG16bitImage &Dest, int x, int y, int cxWidth, int cyHeight, DWORD dwFade, DWORD dwEdgeMask);

		//	CDesignType overrides
		static CSpaceEnvironmentType *AsType (CDesignType *pType) { return ((pType && pType->GetType() == designSpaceEnvironmentType) ? (CSpaceEnvironmentType *)pType : NULL); }
		virtual DesignTypes GetType (void) const { return designSpaceEnvironmentType; }

	protected:
		//	CDesignType overrides
		virtual ALERROR OnBindDesign (SDesignLoadCtx &Ctx);
		virtual ALERROR OnCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);
		virtual void OnMarkImages (void);

	private:
		enum EConstants
			{
			TILES_IN_TILE_SET =				15,
			};

		struct STileDesc
			{
			CG16bitRegion Region;
			};

		enum EEdgeTypes
			{
			edgeNone,

			edgeCloud,					//	Isolated (no edges)
			edgeStraight,				//	Open on one side only
			edgeCorner,					//	Open on two sides next to each other
			edgePeninsula,				//	Open on three sides
			edgeNarrow,					//	Open on two sides opposite each other
			};

		struct SEdgeDesc
			{
			EEdgeTypes iType;			//	Type of edge
			int iRotation;				//	Angle pointing from nebula to space

			//	Transform
			Metric rXx;
			Metric rXy;
			Metric rXc;					//	(as fraction of tile size)
			Metric rYx;
			Metric rYy;
			Metric rYc;					//	(as fraction of tile size)

			//	Wave parameters
			Metric rHalfWaveAngle;		//	Angle in half wave (radians)
			Metric rPixelsPerHalfWave;	//	Size of half wave as fraction of tile size
			Metric rWaveMin;			//	Min value of wave (to scale wave from 0.0-1.0)
			Metric rWaveScale;			//	To scale to 0.0-1.0
			Metric rMinAmplitude;		//	Minimum amplitude
			Metric rMaxAmplitude;		//	Max amplitude
			};

		void CreateAutoTileSet (int iVariants);
		void CreateEdgeTile (const SEdgeDesc &EdgeDesc, STileDesc *retTile);
		void CreateTileSet (const CObjectImageArray &Edges);

		CObjectImageArray m_Image;
		CObjectImageArray m_EdgeMask;
		int m_iImageTileCount;			//	Tiles in m_Image
		COLORREF m_rgbMapColor;			//	Color of tile on map
		DWORD m_dwOpacity;				//	Opacity (0-255)

		bool m_bLRSJammer;				//	If TRUE, LRS is disabled
		bool m_bShieldJammer;			//	If TRUE, shields are disabled
		bool m_bSRSJammer;				//	If TRUE, SRS is disabled
		bool m_bHasOnUpdateEvent;		//	If TRUE, we have an OnUpdate event
		bool m_bAutoEdges;				//	If TRUE, we automatically created edges
		Metric m_rDragFactor;			//	Coefficient of drag (1.0 = no drag)

		//	We check for damage every 15 ticks
		int m_iDamageChance;			//	Chance of damage
		DamageDesc m_Damage;			//	Damage caused to objects in environment

		//	Generated tiles
		int m_iTileSize;				//	Size of tiles (in pixels)
		int m_iVariantCount;			//	Number of variants
		TArray<STileDesc> m_TileSet;

		static SEdgeDesc EDGE_DATA[TILES_IN_TILE_SET];
	};

//	CShipTable ----------------------------------------------------------------

class CShipTable : public CDesignType
	{
	public:
		CShipTable (void);
		virtual ~CShipTable (void);

		inline void CreateShips (SShipCreateCtx &Ctx) { if (m_pGenerator) m_pGenerator->CreateShips(Ctx); }
		inline Metric GetAverageLevelStrength (int iLevel) { return (m_pGenerator ? m_pGenerator->GetAverageLevelStrength(iLevel) : 0.0); }
		ALERROR ValidateForRandomEncounter (void) { if (m_pGenerator) return m_pGenerator->ValidateForRandomEncounter(); }

		//	CDesignType overrides
		static CShipTable *AsType (CDesignType *pType) { return ((pType && pType->GetType() == designShipTable) ? (CShipTable *)pType : NULL); }
		virtual DesignTypes GetType (void) const { return designShipTable; }

	protected:
		//	CDesignType overrides
		virtual void OnAddTypesUsed (TSortMap<DWORD, bool> *retTypesUsed) { if (m_pGenerator) m_pGenerator->AddTypesUsed(retTypesUsed); }
		virtual ALERROR OnBindDesign (SDesignLoadCtx &Ctx);
		virtual ALERROR OnCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);

	private:
		IShipGenerator *m_pGenerator;
	};

//	Missions ------------------------------------------------------------------

class CMissionType : public CDesignType
	{
	public:
		inline bool CanBeDeleted (void) const { return m_fAllowDelete; }
		inline bool CanBeEncountered (void) const { return (m_iMaxAppearing == -1 || m_iAccepted < m_iMaxAppearing); }
		inline bool CloseIfOutOfSystem (void) const { return m_fCloseIfOutOfSystem; }
		inline bool FailureWhenOwnerDestroyed (void) const { return !m_fNoFailureOnOwnerDestroyed; }
		inline bool FailureWhenOutOfSystem (void) const { return (m_iFailIfOutOfSystem != -1); }
		inline bool ForceUndockAfterDebrief (void) const { return m_fForceUndockAfterDebrief; }
		inline const CString &GetName (void) const { return m_sName; }
		inline int GetExpireTime (void) const { return m_iExpireTime; }
		inline int GetOutOfSystemTimeOut (void) const { return m_iFailIfOutOfSystem; }
		inline int GetPriority (void) const { return m_iPriority; }
		inline bool HasDebrief (void) const { return !m_fNoDebrief; }
		inline void IncAccepted (void) { m_iAccepted++; }
		inline bool KeepsStats (void) const { return !m_fNoStats; }

		//	CDesignType overrides

		static CMissionType *AsType (CDesignType *pType) { return ((pType && pType->GetType() == designMissionType) ? (CMissionType *)pType : NULL); }
		virtual bool FindDataField (const CString &sField, CString *retsValue);
		virtual int GetLevel (int *retiMinLevel = NULL, int *retiMaxLevel = NULL) const { if (retiMinLevel) *retiMinLevel = m_iMinLevel; if (retiMaxLevel) *retiMaxLevel = m_iMaxLevel; return (m_iMinLevel + m_iMaxLevel) / 2; }
		virtual DesignTypes GetType (void) const { return designMissionType; }

	protected:
		//	CDesignType overrides

		virtual ALERROR OnBindDesign (SDesignLoadCtx &Ctx);
		virtual ALERROR OnCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);
		virtual void OnReadFromStream (SUniverseLoadCtx &Ctx);
		virtual void OnReinit (void);
		virtual void OnWriteToStream (IWriteStream *pStream);

	private:
		//	Basic properties

		CString m_sName;					//	Internal name
		int m_iPriority;					//	Relative priority (default = 1)

		//	Mission creation

		int m_iMinLevel;					//	Minimum system level supported
		int m_iMaxLevel;					//	Maximum system level supported
		DiceRange m_MaxAppearing;			//	Limit to number of times mission has been accepted by player
											//		(NULL if no limit)
		int m_iExpireTime;					//	Mission expires after this amount
											//		of time if not accepted.
		int m_iFailIfOutOfSystem;			//	If player is out of mission system for this amount of time
											//		(in ticks) then mission fails (-1 means on timeout)

		//	Mission stats

		int m_iMaxAppearing;				//	Limit to number of times mission can appear (-1 = no limit)
		int m_iAccepted;					//	Number of times player has accepted this mission type

		DWORD m_fNoFailureOnOwnerDestroyed:1;	//	If TRUE, mission does not fail when owner destroyed
		DWORD m_fNoDebrief:1;				//	If TRUE, mission is closed on success
		DWORD m_fNoStats:1;					//	If TRUE, mission is not included in count of missions assigned/completed
		DWORD m_fCloseIfOutOfSystem:1;		//	If TRUE, mission is closed if player leaves system.
		DWORD m_fForceUndockAfterDebrief:1;	//	If TRUE, default mission screen undocks after debrief
		DWORD m_fAllowDelete:1;				//	If TRUE, player can delete mission
		DWORD m_fSpare7:1;
		DWORD m_fSpare8:1;

		DWORD m_dwSpare:24;
	};

//	Adventures and Extensions -------------------------------------------------

class CAdventureDesc : public CDesignType
	{
	public:
		void FireOnGameEnd (const CGameRecord &Game, const SBasicGameStats &BasicStats);
		void FireOnGameStart (void);
		inline const CDamageAdjDesc *GetArmorDamageAdj (int iLevel) const { return &m_ArmorDamageAdj[iLevel - 1]; }
		inline DWORD GetBackgroundUNID (void) { return m_dwBackgroundUNID; }
		CString GetDesc (void);
		inline DWORD GetExtensionUNID (void) { return m_dwExtensionUNID; }
		inline const CString &GetName (void) { return m_sName; }
		inline const CDamageAdjDesc *GetShieldDamageAdj (int iLevel) const { return &m_ShieldDamageAdj[iLevel - 1]; }
		inline DWORD GetStartingMapUNID (void) { return m_dwStartingMap; }
		inline const CString &GetStartingNodeID (void) { return m_sStartingNodeID; }
		inline const CString &GetStartingPos (void) { return m_sStartingPos; }
		ALERROR GetStartingShipClasses (TSortMap<CString, CShipClass *> *retClasses, CString *retsError);
		inline const CString &GetWelcomeMessage (void) { return m_sWelcomeMessage; }
		inline bool IsCurrentAdventure (void) { return (m_fIsCurrentAdventure ? true : false); }
		inline bool IsInDefaultResource (void) { return (m_fInDefaultResource ? true : false); }
		bool IsValidStartingClass (CShipClass *pClass);
		inline void SetCurrentAdventure (bool bCurrent = true) { m_fIsCurrentAdventure = bCurrent; }

		//	CDesignType overrides

		static CAdventureDesc *AsType (CDesignType *pType) { return ((pType && pType->GetType() == designAdventureDesc) ? (CAdventureDesc *)pType : NULL); }
		virtual bool FindDataField (const CString &sField, CString *retsValue);
		virtual DesignTypes GetType (void) const { return designAdventureDesc; }

		//	Helpers

		static const CDamageAdjDesc *GetDefaultArmorDamageAdj (int iLevel);
		static const CDamageAdjDesc *GetDefaultShieldDamageAdj (int iLevel);

	protected:
		//	CDesignType overrides
		virtual ALERROR OnBindDesign (SDesignLoadCtx &Ctx);
		virtual ALERROR OnCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);
		virtual void OnUnbindDesign (void) { m_fIsCurrentAdventure = false; }

	private:
		static void InitDefaultDamageAdj (void);

		DWORD m_dwExtensionUNID;

		CString m_sName;						//	Name of adventure
		DWORD m_dwBackgroundUNID;				//	Background image to use for choice screen
		CString m_sWelcomeMessage;				//	Equivalent of "Welcome to Transcendence!"

		CDesignTypeCriteria m_StartingShips;	//	Starting ship criteria
		DWORD m_dwStartingMap;					//	Default system map to load
		CString m_sStartingNodeID;				//	NodeID where we start
		CString m_sStartingPos;					//	Named object at which we start

		CDamageAdjDesc m_ArmorDamageAdj[MAX_ITEM_LEVEL];
		CDamageAdjDesc m_ShieldDamageAdj[MAX_ITEM_LEVEL];

		DWORD m_fIsCurrentAdventure:1;			//	TRUE if this is the current adventure
		DWORD m_fInDefaultResource:1;			//	TRUE if adventure is a module in the default resource
		DWORD m_fIncludeOldShipClasses:1;		//	TRUE if we should include older extensions (even if 
												//		they don't match starting ship criteria).
	};

//	Name Generator -------------------------------------------------------------

class CNameGenerator : public CDesignType
	{
	public:
		CNameGenerator (void);
		virtual ~CNameGenerator (void);

		//	CDesignType overrides
		static CNameGenerator *AsType (CDesignType *pType) { return ((pType && pType->GetType() == designNameGenerator) ? (CNameGenerator *)pType : NULL); }
		virtual DesignTypes GetType (void) const { return designNameGenerator; }

	protected:
		//	CDesignType overrides
		virtual ALERROR OnBindDesign (SDesignLoadCtx &Ctx);
		virtual ALERROR OnCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);

	private:
	};

//	System Map -----------------------------------------------------------------

class ITopologyProcessor
	{
	public:
		static ALERROR CreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, const CString &sUNID, ITopologyProcessor **retpProc);
		static ALERROR CreateFromXMLAsGroup (SDesignLoadCtx &Ctx, CXMLElement *pDesc, const CString &sUNID, ITopologyProcessor **retpProc);

		virtual ~ITopologyProcessor (void) { }
		inline ALERROR BindDesign (SDesignLoadCtx &Ctx) { return OnBindDesign(Ctx); }
		inline CEffectCreator *FindEffectCreator (const CString &sUNID) { return OnFindEffectCreator(sUNID); }
		ALERROR InitFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, const CString &sUNID);
		inline ALERROR Process (CSystemMap *pMap, CTopology &Topology, CTopologyNodeList &NodeList, CString *retsError) { return OnProcess(pMap, Topology, NodeList, retsError); }

	protected:
		virtual ALERROR OnBindDesign (SDesignLoadCtx &Ctx) { return NOERROR; }
		virtual CEffectCreator *OnFindEffectCreator (const CString &sUNID) { return NULL; }
		virtual ALERROR OnInitFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, const CString &sUNID) { return NOERROR; }
		virtual ALERROR OnProcess (CSystemMap *pMap, CTopology &Topology, CTopologyNodeList &NodeList, CString *retsError) { return NOERROR; }

		CTopologyNodeList *FilterNodes (CTopology &Topology, CTopologyNode::SCriteria &Criteria, CTopologyNodeList &Unfiltered, CTopologyNodeList &Filtered);
		void RestoreMarks (CTopology &Topology, TArray<bool> &Saved);
		void SaveAndMarkNodes (CTopology &Topology, CTopologyNodeList &NodeList, TArray<bool> *retSaved);
	};

class CSystemMap : public CDesignType
	{
	public:
		struct SNodeExtra
			{
			CTopologyNode *pNode;
			int iStargateCount;
			};

		struct SSortEntry
			{
			SNodeExtra *pExtra;
			int iSort;
			};

		CSystemMap (void) : m_bAdded(false), m_bDebugShowAttributes(false) { }
		virtual ~CSystemMap (void);

		void AddAnnotation (CEffectCreator *pEffect, int x, int y, int iRotation, DWORD *retdwID = NULL);
		ALERROR AddFixedTopology (CTopology &Topology, CString *retsError);
		bool DebugShowAttributes (void) const { return m_bDebugShowAttributes; }
		CG16bitImage *CreateBackgroundImage (void);
		void GetBackgroundImageSize (int *retcx, int *retcy);
		inline CSystemMap *GetDisplayMap (void) { return (m_pPrimaryMap != NULL ? m_pPrimaryMap : this); }
		inline const CString &GetName (void) const { return m_sName; }
		inline void GetScale (int *retiInitial, int *retiMin, int *retiMax) { if (retiInitial) *retiInitial = m_iInitialScale; if (retiMin) *retiMin = m_iMinScale; if (retiMax) *retiMax = m_iMaxScale; }
		inline const CString &GetStartingNodeID (void) { return m_FixedTopology.GetFirstNodeID(); }
		inline bool IsStartingMap (void) const { return m_bStartingMap; }

		//	CDesignType overrides
		static CSystemMap *AsType (CDesignType *pType) { return ((pType && pType->GetType() == designSystemMap) ? (CSystemMap *)pType : NULL); }
		virtual DesignTypes GetType (void) const { return designSystemMap; }

	protected:
		//	CDesignType overrides
		virtual ALERROR OnBindDesign (SDesignLoadCtx &Ctx);
		virtual ALERROR OnCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);
		virtual CEffectCreator *OnFindEffectCreator (const CString &sUNID);
		virtual void OnReadFromStream (SUniverseLoadCtx &Ctx);
		virtual void OnReinit (void);
		virtual void OnWriteToStream (IWriteStream *pStream);

	private:
		struct SMapAnnotation
			{
			DWORD dwID;

			IEffectPainter *pPainter;
			int xOffset;
			int yOffset;
			int iTick;
			int iRotation;
			};

		ALERROR ExecuteCreator (STopologyCreateCtx &Ctx, CTopology &Topology, CXMLElement *pCreator);

		CString m_sName;						//	Name of the map (for the player)
		DWORD m_dwBackgroundImage;				//	Background image to use
		int m_iInitialScale;					//	Initial map display scale (100 = 100%)
		int m_iMaxScale;						//	Max zoom
		int m_iMinScale;						//	Min zoom

		CSystemMapRef m_pPrimaryMap;			//	If not NULL, place nodes on given map
		TArray<CSystemMapRef> m_Uses;			//	List of maps that we rely on.

		//	Topology generation
		CTopologyDescTable m_FixedTopology;
		TArray<CXMLElement *> m_Creators;
		TArray<ITopologyProcessor *> m_Processors;
		bool m_bStartingMap;					//	Do not load unless specified by player ship

		//	Annotations
		TArray<SMapAnnotation> m_Annotations;

		//	Temporaries
		bool m_bAdded;							//	TRUE if map was added to topology

		//	Debug
		bool m_bDebugShowAttributes;
	};

int KeyCompare (const CSystemMap::SSortEntry &Key1, const CSystemMap::SSortEntry &Key2);

//	System Tables --------------------------------------------------------------

class CSystemTable : public CDesignType
	{
	public:
		CSystemTable (void) : m_pTable(NULL) { }
		virtual ~CSystemTable (void);

		CXMLElement *FindElement (const CString &sElement) const { return (m_pTable ? m_pTable->GetContentElementByTag(sElement) : NULL); }

		//	CDesignType overrides
		static CSystemTable *AsType (CDesignType *pType) { return ((pType && pType->GetType() == designSystemTable) ? (CSystemTable *)pType : NULL); }
		virtual DesignTypes GetType (void) const { return designSystemTable; }

	protected:
		//	CDesignType overrides
		virtual ALERROR OnCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc) { m_pTable = pDesc->OrphanCopy(); return NOERROR; }

	private:
		CXMLElement *m_pTable;
	};

//	Template Types -------------------------------------------------------------

class CTemplateType : public CDesignType
	{
	public:
		CTemplateType (void) { }
		virtual ~CTemplateType (void) { }

		//	CDesignType overrides
		static CTemplateType *AsType (CDesignType *pType) { return ((pType && pType->GetType() == designTemplateType) ? (CTemplateType *)pType : NULL); }
		virtual DesignTypes GetType (void) const { return designTemplateType; }

	protected:
		//	CDesignType overrides
		virtual ALERROR OnCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc) { return NOERROR; }

	private:
	};

//	Associated Structures ------------------------------------------------------

class CInstalledDevice
	{
	public:
		CInstalledDevice (void);
		~CInstalledDevice (void);

		CInstalledDevice &operator= (const CInstalledDevice &Obj);

		//	Create/Install/uninstall/Save/Load methods

		void FinishInstall (CSpaceObject *pSource);
		inline CDeviceClass *GetClass (void) const { return m_pClass; }
		ALERROR InitFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);
		void InitFromDesc (const SDeviceDesc &Desc);
		void Install (CSpaceObject *pObj, CItemListManipulator &ItemList, int iDeviceSlot, bool bInCreate = false);
		ALERROR OnDesignLoadComplete (SDesignLoadCtx &Ctx);
		void ReadFromStream (CSpaceObject *pSource, SLoadCtx &Ctx);
		inline void SetClass (CDeviceClass *pClass) { m_pClass.Set(pClass); }
		void Uninstall (CSpaceObject *pObj, CItemListManipulator &ItemList);
		void Update (CSpaceObject *pSource, 
					 int iTick, 
					 bool *retbSourceDestroyed,
					 bool *retbConsumedItems = NULL,
					 bool *retbDisrupted = NULL);
		void WriteToStream (IWriteStream *pStream);

		//	These methods are about CInstalledDevice properties; not about properties
		//	of the device class. For example, IsOmniDirectional does not check the
		//	properties of the device class

		inline int GetCharges (CSpaceObject *pSource) { return (m_pItem ? m_pItem->GetCharges() : 0); }
		inline DWORD GetData (void) const { return m_dwData; }
		inline int GetDeviceSlot (void) const { return m_iDeviceSlot; }
		inline CItemEnhancementStack *GetEnhancements (void) const { return m_pEnhancements; }
		inline int GetFireArc (void) const { return (IsOmniDirectional() ? 360 : AngleRange(m_iMinFireArc, m_iMaxFireArc)); }
		inline int GetFireAngle (void) const { return m_iFireAngle; }
		inline CItem *GetItem (void) const { return m_pItem; }
		DWORD GetLinkedFireOptions (void) const;
		inline int GetMinFireArc (void) const { return m_iMinFireArc; }
		inline int GetMaxFireArc (void) const { return m_iMaxFireArc; }
		inline CEnergyField *GetOverlay (void) const { return m_pOverlay; }
		inline int GetPosAngle (void) const { return m_iPosAngle; }
		inline int GetPosRadius (void) const { return m_iPosRadius; }
		inline int GetPosZ (void) const { return m_iPosZ; }
		inline int GetRotation (void) const { return AngleMiddle(m_iMinFireArc, m_iMaxFireArc); }
		inline int GetSlotBonus (void) const { return m_iSlotBonus; }
		inline int GetSlotPosIndex (void) const { return m_iSlotPosIndex; }
		inline int GetTemperature (void) const { return m_iTemperature; }
		inline int GetTimeUntilReady (void) const { return m_iTimeUntilReady; }
		inline void IncTemperature (int iChange) { m_iTemperature += iChange; }
		inline bool IsDirectional (void) const { return (m_iMinFireArc != m_iMaxFireArc); }
		inline bool IsEmpty (void) const { return m_pClass == NULL; }
		inline bool IsEnabled (void) const { return m_fEnabled; }
		inline bool IsExternal (void) const { return m_fExternal; }
		inline bool IsLastActivateSuccessful (void) const { return m_fLastActivateSuccessful; }
		inline bool IsOmniDirectional (void) const { return (m_fOmniDirectional ? true : false); }
		inline bool IsOptimized (void) const { return m_fOptimized; }
		inline bool IsOverdrive (void) const { return m_fOverdrive; }
		inline bool IsReady (void) const { return (m_iTimeUntilReady == 0); }
		inline bool IsRegenerating (void) const { return (m_fRegenerating ? true : false); }
		inline bool IsTriggered (void) const { return (m_fTriggered ? true : false); }
		inline bool IsWaiting (void) const { return (m_fWaiting ? true : false); }
		inline void SetActivateDelay (int iDelay) { m_iActivateDelay = iDelay; }
		inline void SetData (DWORD dwData) { m_dwData = dwData; }
		inline void SetDeviceSlot (int iDev) { m_iDeviceSlot = iDev; }
		inline void SetEnabled (bool bEnabled) { m_fEnabled = bEnabled; }
		void SetEnhancements (CItemEnhancementStack *pStack);
		inline void SetFireAngle (int iAngle) { m_iFireAngle = iAngle; }
		inline void SetFireArc (int iMinFireArc, int iMaxFireArc) { m_iMinFireArc = iMinFireArc; m_iMaxFireArc = iMaxFireArc; }
		inline void SetLastActivateSuccessful (bool bSuccessful) { m_fLastActivateSuccessful = bSuccessful; }
		void SetLinkedFireOptions (DWORD dwOptions);
		inline void SetOmniDirectional (bool bOmnidirectional = true) { m_fOmniDirectional = bOmnidirectional; }
		inline void SetOptimized (bool bOptimized) { m_fOptimized = bOptimized; }
		inline void SetOverdrive (bool bOverdrive) { m_fOverdrive = bOverdrive; }
		inline void SetOverlay (CEnergyField *pOverlay) { m_pOverlay = pOverlay; }
		inline void SetPosAngle (int iAngle) { m_iPosAngle = iAngle; }
		inline void SetPosRadius (int iRadius) { m_iPosRadius = iRadius; }
		inline void SetPosZ (int iZ) { m_iPosZ = iZ; m_f3DPosition = (iZ != 0); }
		inline void SetRegenerating (bool bRegenerating) { m_fRegenerating = bRegenerating; }
		inline void SetSecondary (bool bSecondary = true) { m_fSecondaryWeapon = bSecondary; }
		inline void SetSlotBonus (int iBonus) { m_iSlotBonus = iBonus; }
		inline void SetSlotPosIndex (int iIndex) { m_iSlotPosIndex = iIndex; }
		inline void SetTemperature (int iTemperature) { m_iTemperature = iTemperature; }
		inline void SetTimeUntilReady (int iDelay) { m_iTimeUntilReady = iDelay; }
		inline void SetTriggered (bool bTriggered) { m_fTriggered = bTriggered; }
		inline void SetWaiting (bool bWaiting) { m_fWaiting = bWaiting; }

		//	These are wrapper methods for a CDeviceClass method of the same name.
		//	We add our object pointer as a parameter to the call.

		inline bool AbsorbDamage (CSpaceObject *pShip, SDamageCtx &Ctx) { if (!IsEmpty()) return m_pClass->AbsorbDamage(this, pShip, Ctx); else return false; }
		inline bool AccumulateEnhancements (CSpaceObject *pSource, CInstalledDevice *pTarget, TArray<CString> &EnhancementIDs, CItemEnhancementStack *pEnhancements) { return m_pClass->AccumulateEnhancements(CItemCtx(pSource, this), pTarget, EnhancementIDs, pEnhancements); }
		inline bool Activate (CSpaceObject *pSource, 
							  CSpaceObject *pTarget,
							  bool *retbSourceDestroyed,
							  bool *retbConsumedItems = NULL)
			{ return m_pClass->Activate(this, pSource, pTarget, retbSourceDestroyed, retbConsumedItems); }
		int CalcPowerUsed (CSpaceObject *pSource);
		inline bool CanBeDamaged (void) { return m_pClass->CanBeDamaged(); }
		inline bool CanBeDisabled (CItemCtx &Ctx) { return m_pClass->CanBeDisabled(Ctx); }
		inline bool CanHitFriends (void) { return m_pClass->CanHitFriends(); }
		inline bool CanRotate (CItemCtx &Ctx) { return m_pClass->CanRotate(Ctx); }
		inline void Deplete (CSpaceObject *pSource) { m_pClass->Deplete(this, pSource); }
		int GetActivateDelay (CSpaceObject *pSource);
		inline ItemCategories GetCategory (void) const { return m_pClass->GetCategory(); }
		inline int GetCounter (CSpaceObject *pSource, CDeviceClass::CounterTypes *retiCounter = NULL) { return m_pClass->GetCounter(this, pSource, retiCounter); }
		inline const DamageDesc *GetDamageDesc (CItemCtx &Ctx) { return m_pClass->GetDamageDesc(Ctx); }
		inline int GetDamageEffectiveness (CSpaceObject *pAttacker, CInstalledDevice *pWeapon) { return m_pClass->GetDamageEffectiveness(pAttacker, pWeapon); }
		inline int GetDamageType (int iVariant = -1) { return m_pClass->GetDamageType(this, iVariant); }
		inline int GetDefaultFireAngle (CSpaceObject *pSource) { return m_pClass->GetDefaultFireAngle(this, pSource); }
		bool GetDeviceEnhancementDesc (CSpaceObject *pSource, CInstalledDevice *pWeapon, SDeviceEnhancementDesc *retDesc) { return m_pClass->GetDeviceEnhancementDesc(this, pSource, pWeapon, retDesc); }
		inline const DriveDesc *GetDriveDesc (CSpaceObject *pSource) { return m_pClass->GetDriveDesc(this, pSource); }
		inline const ReactorDesc *GetReactorDesc (CSpaceObject *pSource) { return m_pClass->GetReactorDesc(this, pSource); }
		inline Metric GetMaxEffectiveRange (CSpaceObject *pSource, CSpaceObject *pTarget = NULL) { return m_pClass->GetMaxEffectiveRange(pSource, this, pTarget); }
		inline CString GetName (void) { return m_pClass->GetName(); }
		CVector GetPos (CSpaceObject *pSource);
		CVector GetPosOffset (CSpaceObject *pSource);
		inline int GetPowerRating (CItemCtx &Ctx) { return m_pClass->GetPowerRating(Ctx); }
		inline void GetSelectedVariantInfo (CSpaceObject *pSource, 
											CString *retsLabel,
											int *retiAmmoLeft,
											CItemType **retpType = NULL)
			{ m_pClass->GetSelectedVariantInfo(pSource, this, retsLabel, retiAmmoLeft, retpType); }
		inline ItemCategories GetSlotCategory (void) const { return m_pClass->GetSlotCategory(); }
		inline void GetStatus (CSpaceObject *pSource, int *retiStatus, int *retiMaxStatus) { m_pClass->GetStatus(this, pSource, retiStatus, retiMaxStatus); }
		inline CSpaceObject *GetTarget (CSpaceObject *pSource) const;
		inline int GetValidVariantCount (CSpaceObject *pSource) { return m_pClass->GetValidVariantCount(pSource, this); }
		inline int GetWeaponEffectiveness (CSpaceObject *pSource, CSpaceObject *pTarget) { return m_pClass->GetWeaponEffectiveness(pSource, this, pTarget); }
		int IncCharges (CSpaceObject *pSource, int iChange);
		inline bool IsAutomatedWeapon (void) { return m_pClass->IsAutomatedWeapon(); }
		inline bool IsAreaWeapon (CSpaceObject *pSource) { return m_pClass->IsAreaWeapon(pSource, this); }
		inline bool IsFuelCompatible (CItemCtx &Ctx, const CItem &FuelItem) { return m_pClass->IsFuelCompatible(Ctx, FuelItem); }
		bool IsLinkedFire (CItemCtx &Ctx, ItemCategories iTriggerCat = itemcatNone) const;
		inline bool IsSecondaryWeapon (void) const;
		bool IsSelectable (CItemCtx &Ctx) const;
		inline bool IsVariantSelected (CSpaceObject *pSource) { return (m_pClass ? m_pClass->IsVariantSelected(pSource, this) : true); }
		inline void Recharge (CShip *pShip, int iStatus) { m_pClass->Recharge(this, pShip, iStatus); }
		inline void Reset (CSpaceObject *pShip) { m_pClass->Reset(this, pShip); }
		inline void SelectFirstVariant (CSpaceObject *pSource) { m_pClass->SelectFirstVariant(pSource, this); }
		inline void SelectNextVariant (CSpaceObject *pSource, int iDir = 1) { m_pClass->SelectNextVariant(pSource, this, iDir); }
		inline void SetTarget (CSpaceObject *pObj);
		inline bool ShowActivationDelayCounter (CSpaceObject *pSource) { return m_pClass->ShowActivationDelayCounter(pSource, this); }

		//	These are wrapper methods for the CItem behind this device.

		inline DWORD GetDisruptedDuration (void) const { return (m_pItem ? m_pItem->GetDisruptedDuration() : 0); }
		CString GetEnhancedDesc (CSpaceObject *pSource, const CItem *pItem = NULL);
		inline const CItemEnhancement &GetMods (void) const { return (m_pItem ? m_pItem->GetMods() : CItem::GetNullMod()); }
		inline bool IsDamaged (void) const { return (m_pItem ? m_pItem->IsDamaged() : false); }
		inline bool IsDisrupted (void) const { return (m_pItem ? m_pItem->IsDisrupted() : false); }
		inline bool IsEnhanced (void) const { return (m_pItem ? m_pItem->IsEnhanced() : false); }
		inline bool IsWeaponAligned (CSpaceObject *pShip, CSpaceObject *pTarget, int *retiAimAngle = NULL, int *retiFireAngle = NULL) { return m_pClass->IsWeaponAligned(pShip, this, pTarget, retiAimAngle, retiFireAngle); }

	private:
		CItem *m_pItem;							//	Item installed in this slot
		CDeviceClassRef m_pClass;				//	The device class that is installed here
		CEnergyField *m_pOverlay;				//	Overlay (if associated)
		DWORD m_dwTargetID;						//	ObjID of target (for tracking secondary weapons)
		CItemEnhancementStack *m_pEnhancements;	//	List of enhancements (may be NULL)

		DWORD m_dwData;							//	Data specific to device class

		int m_iDeviceSlot:16;					//	Device slot

		int m_iPosAngle:16;						//	Position of installation (degrees)
		int m_iPosRadius:16;					//	Position of installation (pixels)
		int m_iPosZ:16;							//	Position of installation (height)
		int m_iMinFireArc:16;					//	Min angle of fire arc (degrees)
		int m_iMaxFireArc:16;					//	Max angle of fire arc (degrees)

		int m_iTimeUntilReady:16;				//	Timer counting down until ready to activate
		int m_iFireAngle:16;					//	Last fire angle

		int m_iTemperature:16;					//	Temperature for weapons
		int m_iActivateDelay:16;				//	Cached activation delay
		int m_iSlotBonus:16;					//	Bonus from device slot itself
		int m_iSlotPosIndex:16;					//	Slot placement

		DWORD m_fOmniDirectional:1;				//	Installed on turret
		DWORD m_fOverdrive:1;					//	Device has overdrive installed
		DWORD m_fOptimized:1;					//	Device is optimized by alien engineers
		DWORD m_fSecondaryWeapon:1;				//	Secondary weapon
		DWORD m_fEnabled:1;						//	Device is enabled
		DWORD m_fExternal:1;					//	Device is external to hull
		DWORD m_fWaiting:1;						//	Waiting for cooldown, etc.
		DWORD m_fTriggered:1;					//	Device trigger is down (e.g., weapon is firing)

		DWORD m_fRegenerating:1;				//	TRUE if we regenerated on the last tick
		DWORD m_fLastActivateSuccessful:1;		//	TRUE if we successfully fired (last time we tried)
		DWORD m_f3DPosition:1;					//	If TRUE we use m_iPosZ to compute position
		DWORD m_fLinkedFireAlways:1;			//	If TRUE, lkfAlways
		DWORD m_fLinkedFireTarget:1;			//	If TRUE, lkfTarget
		DWORD m_fLinkedFireEnemy:1;				//	If TRUE, lkfEnemy
		DWORD m_fSpare7:1;
		DWORD m_fSpare8:1;

		DWORD m_dwSpare:16;						//	Spare flags
	};

//	CDesignCollection

class CDesignList
	{
	public:
		CDesignList (void) : m_List(128) { }
		~CDesignList (void) { }

		inline void AddEntry (CDesignType *pType) { m_List.Insert(pType); }
		void Delete (DWORD dwUNID);
		void DeleteAll (bool bFree = false);
		inline int GetCount (void) const { return m_List.GetCount(); }
		inline CDesignType *GetEntry (int iIndex) const { return m_List[iIndex]; }

	private:
		TArray<CDesignType *> m_List;
	};

class CDesignTable
	{
	public:
		CDesignTable (bool bFreeTypes = false) : m_bFreeTypes(bFreeTypes) { }
		~CDesignTable (void) { DeleteAll(); }

		ALERROR AddEntry (CDesignType *pEntry);
		ALERROR AddOrReplaceEntry (CDesignType *pEntry, CDesignType **retpOldEntry = NULL);
		void Delete (DWORD dwUNID);
		void DeleteAll (void);
		CDesignType *FindByUNID (DWORD dwUNID) const;
		inline int GetCount (void) const { return m_Table.GetCount(); }
		inline CDesignType *GetEntry (int iIndex) const { return m_Table.GetValue(iIndex); }
		ALERROR Merge (const CDesignTable &Source, CDesignList *ioOverride = NULL);
		ALERROR Merge (const CDynamicDesignTable &Source, CDesignList *ioOverride = NULL);

	private:
		TSortMap<DWORD, CDesignType *> m_Table;
		bool m_bFreeTypes;
	};

enum EExtensionTypes
	{
	extUnknown,

	extBase,
	extAdventure,
	extLibrary,
	extExtension,
	};

class CExtension
	{
	public:
		enum ELoadStates
			{
			loadNone,

			loadEntities,					//	We've loaded the entities, but nothing else.
			loadAdventureDesc,				//	We've loaded adventure descriptors, but no other types
			loadComplete,					//	We've loaded all design types.
			};

		enum EFolderTypes
			{
			folderUnknown,

			folderBase,						//	Base folder (only for base XML)
			folderCollection,				//	Collection folder
			folderExtensions,				//	Extensions folder
			};

		struct SLibraryDesc
			{
			DWORD dwUNID;					//	UNID of library that we use
			DWORD dwRelease;				//	Release of library that we use
			};

		CExtension (void);
		~CExtension (void);

		static ALERROR CreateBaseFile (SDesignLoadCtx &Ctx, CXMLElement *pDesc, CExternalEntityTable *pEntities, TArray<CExtension *> *retExtensions);
		static ALERROR CreateExtension (SDesignLoadCtx &Ctx, CXMLElement *pDesc, EFolderTypes iFolder, CExternalEntityTable *pEntities, CExtension **retpExtension);
		static ALERROR CreateExtensionStub (const CString &sFilespec, EFolderTypes iFolder, CExtension **retpExtension, CString *retsError);

		bool CanExtend (CExtension *pAdventure) const;
		void CleanUp (void);
		void CreateIcon (int cxWidth, int cyHeight, CG16bitImage **retpIcon) const;
		ALERROR ExecuteGlobals (SDesignLoadCtx &Ctx);
		inline CAdventureDesc *GetAdventureDesc (void) const { return m_pAdventureDesc; }
		inline DWORD GetAPIVersion (void) const { return m_dwAPIVersion; }
		inline DWORD GetAutoIncludeAPIVersion (void) const { return m_dwAutoIncludeAPIVersion; }
		CG16bitImage *GetCoverImage (void) const;
		inline const TArray<CString> &GetCredits (void) const { return m_Credits; }
		inline const CString &GetDisabledReason (void) const { return m_sDisabledReason; }
		inline CString GetDesc (void) { return (m_pAdventureDesc ? m_pAdventureDesc->GetDesc() : NULL_STR); }
		inline const CDesignTable &GetDesignTypes (void) { return m_DesignTypes; }
		inline const CIntegerIP &GetDigest (void) const { return m_Digest; }
		inline CExternalEntityTable *GetEntities (void) { return m_pEntities; }
		inline const TArray<CString> &GetExternalResources (void) const { return m_Externals; }
		inline const CString &GetFilespec (void) const { return m_sFilespec; }
		inline EFolderTypes GetFolderType (void) const { return m_iFolderType; }
		inline const SLibraryDesc &GetLibrary (int iIndex) const { return m_Libraries[iIndex]; }
		inline int GetLibraryCount (void) const { return m_Libraries.GetCount(); }
		inline ELoadStates GetLoadState (void) const { return m_iLoadState; }
		inline const CTimeDate &GetModifiedTime (void) const { return m_ModifiedTime; }
		inline const CString &GetName (void) const { return m_sName; }
		inline CTopologyDescTable &GetTopology (void) { return m_Topology; }
		inline DWORD GetRelease (void) const { return m_dwRelease; }
		inline EExtensionTypes GetType (void) const { return m_iType; }
		inline DWORD GetUNID (void) const { return m_dwUNID; }
		inline const CString &GetVersion (void) const { return m_sVersion; }
		inline bool IsAutoInclude (void) const { return m_bAutoInclude; }
		inline bool IsDebugOnly (void) const { return m_bDebugOnly; }
		inline bool IsDisabled (void) const { return m_bDisabled; }
		inline bool IsHidden (void) const { return m_bPrivate; }
		inline bool IsMarked (void) const { return m_bMarked; }
		inline bool IsRegistered (void) const { return m_bRegistered; }
		inline bool IsRegistrationVerified (void) { return (m_bRegistered && m_bVerified); }
		ALERROR Load (ELoadStates iDesiredState, IXMLParserController *pResolver, bool bNoResources, bool bKeepXML, CString *retsError);
		inline void SetDeleted (void) { m_bDeleted = true; }
		inline void SetDisabled (const CString &sReason) { if (!m_bDisabled) { m_sDisabledReason = sReason; m_bDisabled = true; } }
		inline void SetDigest (const CIntegerIP &Digest) { m_Digest = Digest; }
		inline void SetMarked (bool bMarked = true) { m_bMarked = bMarked; }
		inline void SetModifiedTime (const CTimeDate &Time) { m_ModifiedTime = Time; }
		inline void SetName (const CString &sName) { m_sName = sName; }
		inline void SetVerified (bool bVerified = true) { m_bVerified = bVerified; }
		void SweepImages (void);
		inline bool UsesXML (void) const { return m_bUsesXML; }

		static ALERROR ComposeLoadError (SDesignLoadCtx &Ctx, CString *retsError);
		static void DebugDump (CExtension *pExtension, bool bFull = false);

	private:
		struct SGlobalsEntry
			{
			CString sFilespec;
			ICCItem *pCode;
			};

		static ALERROR CreateExtensionFromRoot (const CString &sFilespec, CXMLElement *pDesc, EFolderTypes iFolder, CExternalEntityTable *pEntities, DWORD dwInheritAPIVersion, CExtension **retpExtension, CString *retsError);

		void CleanUpXML (void);
		ALERROR LoadDesignElement (SDesignLoadCtx &Ctx, CXMLElement *pDesc);
		ALERROR LoadDesignType (SDesignLoadCtx &Ctx, CXMLElement *pDesc, CDesignType **retpType = NULL);
		ALERROR LoadGlobalsElement (SDesignLoadCtx &Ctx, CXMLElement *pDesc);
		ALERROR LoadImagesElement (SDesignLoadCtx &Ctx, CXMLElement *pDesc);
		ALERROR LoadLibraryElement (SDesignLoadCtx &Ctx, CXMLElement *pDesc);
		ALERROR LoadModuleContent (SDesignLoadCtx &Ctx, CXMLElement *pDesc);
		ALERROR LoadModuleElement (SDesignLoadCtx &Ctx, CXMLElement *pDesc);
		ALERROR LoadModulesElement (SDesignLoadCtx &Ctx, CXMLElement *pDesc);
		ALERROR LoadSoundElement (SDesignLoadCtx &Ctx, CXMLElement *pDesc);
		ALERROR LoadSoundsElement (SDesignLoadCtx &Ctx, CXMLElement *pDesc);
		ALERROR LoadSystemTypesElement (SDesignLoadCtx &Ctx, CXMLElement *pDesc);

		CString m_sFilespec;				//	Extension file
		DWORD m_dwUNID;						//	UNID of extension
		EExtensionTypes m_iType;			//	Either adventure, extension, or base

		ELoadStates m_iLoadState;			//	Current load state
		EFolderTypes m_iFolderType;			//	Folder that extension came from
		CTimeDate m_ModifiedTime;			//	Timedate of extension file
		CIntegerIP m_Digest;				//	Digest (for registered files)
		DWORD m_dwAPIVersion;				//	Version of API that we're using
		CExternalEntityTable *m_pEntities;	//	Entities defined by this extension
		CString m_sDisabledReason;			//	Reason why extension is disabled

		CString m_sName;					//	Extension name
		DWORD m_dwRelease;					//	Release number
		CString m_sVersion;					//	User-visible version number
		DWORD m_dwCoverUNID;				//	UNID of cover image
		CDesignTable m_DesignTypes;			//	Design types defined by extension
		CTopologyDescTable m_Topology;		//	Topology defined by extension 
											//		(for backwards compatibility)
		TArray<SGlobalsEntry> m_Globals;	//	Globals
		TArray<CString> m_Credits;			//	List of names for credits

		TArray<SLibraryDesc> m_Libraries;	//	Extensions that we use.
		TArray<DWORD> m_Extends;			//	UNIDs that this extension extends
		DWORD m_dwAutoIncludeAPIVersion;	//	Library adds compatibility to any
											//		extension at or below this
											//		API version.
		TArray<CString> m_Externals;		//	External resources

		CXMLElement *m_pRootXML;			//	Root XML representation (may be NULL)
		TSortMap<CString, CXMLElement *> m_ModuleXML;	//	XML for modules

		mutable CG16bitImage *m_pCoverImage;	//	Large cover image

		CAdventureDesc *m_pAdventureDesc;	//	If extAdventure, this is the descriptor

		bool m_bMarked;						//	Used by CExtensionCollection for various things
		bool m_bDebugOnly;					//	Only load in debug mode
		bool m_bRegistered;					//	UNID indicates this is a registered extension
		bool m_bVerified;					//	Signature and license verified
		bool m_bPrivate;					//	Do not show in stats
		bool m_bDisabled;					//	Disabled (for some reason)
		bool m_bDeleted;
		bool m_bAutoInclude;				//	Extension should always be included (if appropriate)
		bool m_bUsesXML;					//	Extension uses XML from other extensions
	};

class CExtensionCollection
	{
	public:
		enum Flags
			{
			//	Load

			FLAG_NO_RESOURCES =		0x00000001,	//	Do not load resources.
			FLAG_DEBUG_MODE =		0x00000002,	//	Game run with /debug
			FLAG_DESC_ONLY =		0x00000004,	//	Load adventure descs only
			FLAG_ERROR_ON_DISABLE =	0x00000008,	//	Return an error if an extension was loaded disabled
												//		(due to missing dependencies, etc.)

			//	FindExtension

			FLAG_ADVENTURE_ONLY =	0x00000010,	//	Must be an adventure (not found otherwise)

			//	ComputeAvailableExtension

			FLAG_INCLUDE_AUTO =		0x00000020,	//	Include extensions that are automatic
			FLAG_AUTO_ONLY =		0x00000040,	//	Only include extensions that are automatic
			FLAG_ACCUMULATE =		0x00000080,	//	Add to result list
			};

		CExtensionCollection (void);
		~CExtensionCollection (void);

		inline void AddExtensionFolder (const CString &sFilespec) { m_ExtensionFolders.Insert(sFilespec); }
		void CleanUp (void);
		ALERROR ComputeAvailableAdventures (DWORD dwFlags, TArray<CExtension *> *retList, CString *retsError);
		ALERROR ComputeAvailableExtensions (CExtension *pAdventure, DWORD dwFlags, const TArray<DWORD> &Extensions, TArray<CExtension *> *retList, CString *retsError);
		ALERROR ComputeBindOrder (CExtension *pAdventure, const TArray<CExtension *> &DesiredExtensions, DWORD dwFlags, TArray<CExtension *> *retList, CString *retsError);
		void DebugDump (void);
		bool FindAdventureFromDesc (DWORD dwUNID, DWORD dwFlags = 0, CExtension **retpExtension = NULL);
		bool FindBestExtension (DWORD dwUNID, DWORD dwRelease = 0, DWORD dwFlags = 0, CExtension **retpExtension = NULL);
		bool FindExtension (DWORD dwUNID, DWORD dwRelease, CExtension::EFolderTypes iFolder, CExtension **retpExtension = NULL);
		void FreeDeleted (void);
		CString GetExternalResourceFilespec (CExtension *pExtension, const CString &sFilename) const;
		bool GetRequiredResources (TArray<CString> *retFilespecs);
		void InitEntityResolver (CExtension *pExtension, DWORD dwFlags, CEntityResolverList *retResolver);
		bool IsRegisteredGame (CExtension *pAdventure, const TArray<CExtension *> &DesiredExtensions, DWORD dwFlags);
		ALERROR Load (const CString &sFilespec, DWORD dwFlags, CString *retsError);
		inline bool LoadedInDebugMode (void) { return m_bLoadedInDebugMode; }
		ALERROR LoadNewExtension (const CString &sFilespec, const CIntegerIP &FileDigest, CString *retsError);
		inline void SetCollectionFolder (const CString &sFilespec) { m_sCollectionFolder = sFilespec; }
		void SetRegisteredExtensions (const CMultiverseCollection &Collection, TArray<CMultiverseCatalogEntry *> *retNotFound);
		void SweepImages (void);
		void UpdateCollectionStatus (CMultiverseCollection &Collection, int cxIconSize, int cyIconSize);

		static int Compare (CExtension *pExt1, CExtension *pExt2, bool bDebugMode);

	private:
		void AddOrReplace (CExtension *pExtension);
		ALERROR AddToBindList (CExtension *pExtension, DWORD dwFlags, const TArray<CExtension *> &Compatibility, TArray<CExtension *> *retList, CString *retsError);
		void ClearAllMarks (void);
		void ComputeCompatibilityLibraries (CExtension *pAdventure, DWORD dwFlags, TArray<CExtension *> *retList);
		ALERROR ComputeFilesToLoad (const CString &sFilespec, CExtension::EFolderTypes iFolder, TSortMap<CString, int> &List, CString *retsError);
		ALERROR LoadBaseFile (const CString &sFilespec, DWORD dwFlags, CString *retsError);
		ALERROR LoadFile (const CString &sFilespec, CExtension::EFolderTypes iFolder, DWORD dwFlags, const CIntegerIP &CheckDigest, bool *retbReload, CString *retsError);
		ALERROR LoadFolderStubsOnly (const CString &sFilespec, CExtension::EFolderTypes iFolder, DWORD dwFlags, CString *retsError);
		bool ReloadDisabledExtensions (DWORD dwFlags);

		CString m_sCollectionFolder;		//	Path to collection folder
		TArray<CString> m_ExtensionFolders;	//	Paths to extension folders

		CCriticalSection m_cs;				//	Protects modifications
		TArray<CExtension *> m_Extensions;	//	All loaded extensions
		bool m_bReloadNeeded;				//	If TRUE we need to reload our folders
		bool m_bLoadedInDebugMode;			//	If TRUE we loaded in debug mode
		bool m_bKeepXML;					//	If TRUE we need to keep XML for all types

		TArray<CExtension *> m_Deleted;		//	Keep around until next bind

		//	Indices for easy access

		CExtension *m_pBase;				//	Base extension
		TSortMap<DWORD, TArray<CExtension *> > m_ByUNID;
		TSortMap<CString, CExtension *> m_ByFilespec;
	};

class CDynamicDesignTable
	{
	public:
		CDynamicDesignTable (void) { }
		~CDynamicDesignTable (void) { CleanUp(); }

		ALERROR DefineType (CExtension *pExtension, DWORD dwUNID, ICCItem *pSource, CDesignType **retpType = NULL, CString *retsError = NULL);
		void Delete (DWORD dwUNID);
		inline void DeleteAll (void) { CleanUp(); }
		inline int GetCount (void) const { return m_Table.GetCount(); }
		inline CDesignType *GetType (int iIndex) const { return m_Table[iIndex].pType; }
		void ReadFromStream (SUniverseLoadCtx &Ctx);
		void WriteToStream (IWriteStream *pStream);

	private:
		struct SEntry
			{
			CExtension *pExtension;
			DWORD dwUNID;
			CString sSource;
			CDesignType *pType;
			};

		void CleanUp (void);
		ALERROR Compile (SEntry *pEntry, CDesignType **retpType, CString *retsError = NULL);
		ALERROR CreateType (SEntry *pEntry, CXMLElement *pDesc, CDesignType **retpType, CString *retsError = NULL);
		inline SEntry *GetEntry (int iIndex) const { return &m_Table[iIndex]; }

		TSortMap<DWORD, SEntry> m_Table;
	};

struct SDesignLoadCtx
	{
	SDesignLoadCtx (void) :
			pResDb(NULL),
			pExtension(NULL),
			pType(NULL),
			bBindAsNewGame(false),
			bNoResources(false),
			bNoVersionCheck(false),
			bKeepXML(false),
			bLoadAdventureDesc(false),
			bLoadModule(false),
			dwInheritAPIVersion(0)
		{ }

	inline DWORD GetAPIVersion (void) const { return (pExtension ? pExtension->GetAPIVersion() : API_VERSION); }

	//	Context
	CString sResDb;							//	ResourceDb filespec
	CResourceDb *pResDb;					//	Open ResourceDb object
	CString sFolder;						//	Folder context (used when loading images)
	CExtension *pExtension;					//	Extension
	CDesignType *pType;						//	Current type being loaded
	bool bLoadAdventureDesc;				//	If TRUE, we are loading an adventure desc only
	bool bLoadModule;						//	If TRUE, we are loading elements in a module
	DWORD dwInheritAPIVersion;				//	APIVersion of parent (if base file)

	//	Options
	bool bBindAsNewGame;					//	If TRUE, then we are binding a new game
	bool bNoResources;
	bool bNoVersionCheck;
	bool bKeepXML;							//	Keep XML around

	//	Output
	CString sError;
	CString sErrorFilespec;					//	File in which error occurred.
	};

class CDesignCollection
	{
	public:
		enum ECachedHandlers
			{
			evtGetGlobalAchievements	= 0,
			evtGetGlobalDockScreen		= 1,
			evtGetGlobalPlayerPriceAdj	= 2,
			evtOnGlobalDockPaneInit		= 3,
			evtOnGlobalMarkImages		= 4,
			evtOnGlobalObjDestroyed		= 5,
			evtOnGlobalSystemStarted	= 6,
			evtOnGlobalSystemStopped	= 7,
			evtOnGlobalUniverseCreated	= 8,
			evtOnGlobalUniverseLoad		= 9,
			evtOnGlobalUniverseSave		= 10,
			evtOnGlobalUpdate			= 11,

			evtCount					= 12
			};

		enum EFlags
			{
			//	GetImage flags
			FLAG_IMAGE_COPY =			0x00000001,
			FLAG_IMAGE_LOCK =			0x00000002,
			};

		CDesignCollection (void);
		~CDesignCollection (void);

		ALERROR AddDynamicType (CExtension *pExtension, DWORD dwUNID, ICCItem *pSource, bool bNewGame, CString *retsError);
		ALERROR BindDesign (const TArray<CExtension *> &BindOrder, bool bNewGame, bool bNoResources, CString *retsError);
		void CleanUp (void);
		void ClearImageMarks (void);
		inline CEconomyType *FindEconomyType (const CString &sID) { CEconomyType **ppType = m_EconomyIndex.GetAt(sID); return (ppType ? *ppType : NULL); }
		inline CDesignType *FindEntry (DWORD dwUNID) const { return m_AllTypes.FindByUNID(dwUNID); }
		CExtension *FindExtension (DWORD dwUNID) const;
		CXMLElement *FindSystemFragment (const CString &sName, CSystemTable **retpTable = NULL) const;
		void FireGetGlobalAchievements (CGameStats &Stats);
		bool FireGetGlobalDockScreen (CSpaceObject *pObj, CString *retsScreen, ICCItem **retpData, int *retiPriority = NULL);
		bool FireGetGlobalPlayerPriceAdj (ETradeServiceTypes iService, CSpaceObject *pProvider, const CItem &Item, ICCItem *pData, int *retiPriceAdj);
		void FireOnGlobalMarkImages (void);
		void FireOnGlobalObjDestroyed (SDestroyCtx &Ctx);
		void FireOnGlobalPaneInit (void *pScreen, CDesignType *pRoot, const CString &sScreen, const CString &sPane);
		void FireOnGlobalPlayerChangedShips (CSpaceObject *pOldShip);
		void FireOnGlobalPlayerEnteredSystem (void);
		void FireOnGlobalPlayerLeftSystem (void);
		void FireOnGlobalSystemCreated (SSystemCreateCtx &SysCreateCtx);
		void FireOnGlobalSystemStarted (void);
		void FireOnGlobalSystemStopped (void);
		ALERROR FireOnGlobalTypesInit (SDesignLoadCtx &Ctx);
		void FireOnGlobalUniverseCreated (void);
		void FireOnGlobalUniverseLoad (void);
		void FireOnGlobalUniverseSave (void);
		void FireOnGlobalUpdate (int iTick);
		inline int GetCount (void) const { return m_AllTypes.GetCount(); }
		inline int GetCount (DesignTypes iType) const { return m_ByType[iType].GetCount(); }
		inline const CDisplayAttributeDefinitions &GetDisplayAttributes (void) const { return m_DisplayAttribs; }
		DWORD GetDynamicUNID (const CString &sName);
		void GetEnabledExtensions (TArray<CExtension *> *retExtensionList);
		inline CDesignType *GetEntry (int iIndex) const { return m_AllTypes.GetEntry(iIndex); }
		inline CDesignType *GetEntry (DesignTypes iType, int iIndex) const { return m_ByType[iType].GetEntry(iIndex); }
		inline CExtension *GetExtension (int iIndex) { return m_BoundExtensions[iIndex]; }
		inline int GetExtensionCount (void) { return m_BoundExtensions.GetCount(); }
		CG16bitImage *GetImage (DWORD dwUNID, DWORD dwFlags = 0);
		CString GetStartingNodeID (void);
		CTopologyDescTable *GetTopologyDesc (void) const { return m_pTopology; }
		inline bool HasDynamicTypes (void) { return (m_DynamicTypes.GetCount() > 0); }
		bool IsAdventureExtensionBound (DWORD dwUNID);
		bool IsAdventureExtensionLoaded (DWORD dwUNID);
		bool IsRegisteredGame (void);
		void MarkGlobalImages (void);
		void NotifyTopologyInit (void);
		void ReadDynamicTypes (SUniverseLoadCtx &Ctx);
		void Reinit (void);
		void SweepImages (void);
		void WriteDynamicTypes (IWriteStream *pStream);

	private:
		ALERROR AddExtension (SDesignLoadCtx &Ctx, EExtensionTypes iType, DWORD dwUNID, bool bDefaultResource, CExtension **retpExtension);
		void CacheGlobalEvents (CDesignType *pType);
		ALERROR CreateTemplateTypes (SDesignLoadCtx &Ctx);
		ALERROR ResolveOverrides (SDesignLoadCtx &Ctx);

		//	Loaded types. These are initialized at load-time and never change.

		CDesignTable m_Base;
		CTopologyDescTable m_BaseTopology;
		CExternalEntityTable m_BaseEntities;

		//	Cached data initialized at bind-time

		TArray<CExtension *> m_BoundExtensions;
		CDesignTable m_AllTypes;
		CDesignList m_ByType[designCount];
		CDesignList m_OverrideTypes;
		CDesignList m_CreatedTypes;
		CTopologyDescTable *m_pTopology;
		CExtension *m_pAdventureExtension;
		CAdventureDesc *m_pAdventureDesc;
		TSortMap<CString, CEconomyType *> m_EconomyIndex;
		CDisplayAttributeDefinitions m_DisplayAttribs;
		CGlobalEventCache *m_EventsCache[evtCount];

		//	Dynamic design types

		CDynamicDesignTable m_DynamicTypes;
		TSortMap<CString, CDesignType *> m_DynamicUNIDs;
	};

//	Utility functions

IShipController *CreateShipController (const CString &sAI);
DWORD ExtensionVersionToInteger (DWORD dwVersion);
CString GenerateLevelFrequency (const CString &sLevelFrequency, int iCenterLevel);
CString GenerateRandomName (const CString &sList, const CString &sSubst);
CString GenerateRandomNameFromTemplate (const CString &sName, const CString &sSubst = NULL_STR);
CString GetDamageName (DamageTypes iType);
CString GetDamageShortName (DamageTypes iType);
CString GetDamageType (DamageTypes iType);
int GetDiceCountFromAttribute(const CString &sValue);
int GetFrequency (const CString &sValue);
int GetFrequencyByLevel (const CString &sLevelFrequency, int iLevel);
CString GetItemCategoryID (ItemCategories iCategory);
CString GetItemCategoryName (ItemCategories iCategory);
bool IsConstantName (const CString &sList);
bool IsEnergyDamage (DamageTypes iType);
bool IsMatterDamage (DamageTypes iType);
COLORREF LoadCOLORREF (const CString &sString);
ALERROR LoadDamageAdj (CXMLElement *pItem, const CString &sAttrib, int *retiAdj, int *retiCount = NULL);
DamageTypes LoadDamageTypeFromXML (const CString &sAttrib);
DWORD LoadExtensionVersion (const CString &sVersion);
DWORD LoadNameFlags (CXMLElement *pDesc);
WORD LoadRGBColor (const CString &sString);
ALERROR LoadUNID (SDesignLoadCtx &Ctx, const CString &sString, DWORD *retdwUNID);
bool SetFrequencyByLevel (CString &sLevelFrequency, int iLevel, int iFreq);

//	Inline implementations

inline EDamageResults CInstalledArmor::AbsorbDamage (CSpaceObject *pSource, SDamageCtx &Ctx) { return m_pArmorClass->AbsorbDamage(CItemCtx(pSource, this), Ctx); }
inline int CInstalledArmor::GetDamageEffectiveness (CSpaceObject *pAttacker, CInstalledDevice *pWeapon) { return m_pArmorClass->GetDamageEffectiveness(pAttacker, pWeapon); }
inline int CInstalledArmor::GetMaxHP (CSpaceObject *pSource) { return m_pArmorClass->GetMaxHP(CItemCtx(pSource, this)); }

inline bool CInstalledDevice::IsSecondaryWeapon (void) const 
	{
	DWORD dwLinkedFire;
	return (m_fSecondaryWeapon 
			|| (dwLinkedFire = m_pClass->GetLinkedFireOptions(CItemCtx(NULL, (CInstalledDevice *)this))) == CDeviceClass::lkfEnemyInRange
			|| dwLinkedFire == CDeviceClass::lkfTargetInRange);
	}

inline bool CItem::IsArmor (void) const { return (m_pItemType && m_pItemType->GetArmorClass()); }
inline bool CItem::IsDevice (void) const { return (m_pItemType && m_pItemType->GetDeviceClass()); }
inline CEconomyType *CItem::GetCurrencyType (void) const { return m_pItemType->GetCurrencyType(); }

inline CDeviceClass *CDeviceDescList::GetDeviceClass (int iIndex) const { return m_List[iIndex].Item.GetType()->GetDeviceClass(); }

inline CString CArmorClass::GetName (void) { return m_pItemType->GetNounPhrase(); }
inline DWORD CArmorClass::GetUNID (void) { return m_pItemType->GetUNID(); }
inline bool CArmorClass::IsBlindingDamageImmune (CItemCtx &ItemCtx) { return (m_iBlindingDamageAdj == 0 || ItemCtx.GetMods().IsBlindingImmune()); }
inline bool CArmorClass::IsDeviceDamageImmune (CItemCtx &ItemCtx) { return (m_iDeviceDamageAdj == 0 || ItemCtx.GetMods().IsDeviceDamageImmune()); }
inline bool CArmorClass::IsDisintegrationImmune (CItemCtx &ItemCtx) { return (m_fDisintegrationImmune || ItemCtx.GetMods().IsDisintegrationImmune()); }
inline bool CArmorClass::IsEMPDamageImmune (CItemCtx &ItemCtx) { return (m_iEMPDamageAdj == 0 || ItemCtx.GetMods().IsEMPImmune()); }
inline bool CArmorClass::IsRadiationImmune (CItemCtx &ItemCtx) { return (m_fRadiationImmune || ItemCtx.GetMods().IsRadiationImmune()); }
inline bool CArmorClass::IsShatterImmune (CItemCtx &ItemCtx) { return (m_fShatterImmune || (ItemCtx.GetMods().IsShatterImmune())); }
inline bool CArmorClass::IsShieldInterfering (CItemCtx &ItemCtx) { return (m_fShieldInterference || ItemCtx.GetMods().IsShieldInterfering()); }

inline int CDeviceClass::GetLevel (void) const { return m_pItemType->GetLevel(); }
inline CString CDeviceClass::GetName (void) { return m_pItemType->GetName(NULL); }
inline DWORD CDeviceClass::GetUNID (void) { return m_pItemType->GetUNID(); }

inline CXMLElement *CItemType::GetUseScreen (void) const { return m_pUseScreen.GetDesc(); }
inline CDesignType *CItemType::GetUseScreen (CString *retsName) { return m_pUseScreen.GetDockScreen(this, retsName); }

inline bool DamageDesc::IsEnergyDamage (void) const { return ::IsEnergyDamage(m_iType); }
inline bool DamageDesc::IsMatterDamage (void) const { return ::IsMatterDamage(m_iType); }

inline void IEffectPainter::PlaySound (CSpaceObject *pSource) { GetCreator()->PlaySound(pSource); }

inline CSystemMap *CTopologyNode::GetDisplayPos (int *retxPos, int *retyPos) { if (retxPos) *retxPos = m_xPos; if (retyPos) *retyPos = m_yPos; return (m_pMap ? m_pMap->GetDisplayMap() : NULL); }

#endif