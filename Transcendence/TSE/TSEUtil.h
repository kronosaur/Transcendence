//	TSEUtil.h
//
//	Transcendence Utilities

#ifndef INCL_TSE_UTIL
#define INCL_TSE_UTIL

class CDesignType;
class CEconomyType;
class CItemCtx;
class CExtension;
class CLocationDef;
class COrbit;
struct CItemCriteria;
struct SDesignLoadCtx;
struct SDamageCtx;
struct SDestroyCtx;
struct SSystemCreateCtx;

//	Utility inlines

inline int Angle2Direction (int iAngle, int iRotationCount)
	{
	return ((iRotationCount - (iAngle / (360 / iRotationCount))) + (iRotationCount / 4)) % iRotationCount;
	}
inline int Direction2Angle (int iDirection, int iRotationCount)
	{
	return (720 - (360 * (iDirection - (iRotationCount / 4) + iRotationCount) / iRotationCount)) % 360;
	}
inline int DirectionFacesUp (int iDirection, int iRotationCount)
	{
	int iQuarter = (iRotationCount / 4);
	return (iDirection < iQuarter) || (iDirection > (iRotationCount - iQuarter));
	}
inline bool AreAnglesAligned (int iDesiredAngle, int iActualAngle, int iTolerance)
	{
	if (iActualAngle < iDesiredAngle)
		iActualAngle += 360;

	int iOffset = 180 - abs((iActualAngle - iDesiredAngle) - 180);

	return (iOffset <= iTolerance);
	}
inline int AngleOffset (int iAngle1, int iAngle2)
	{
	if (iAngle1 < iAngle2)
		iAngle1 += 360;

	return 180 - abs((iAngle1 - iAngle2) - 180);
	}
inline int AngleRange (int iMinAngle, int iMaxAngle)
	{
	if (iMinAngle > iMaxAngle)
		return (iMaxAngle + 360 - iMinAngle);
	else
		return (iMaxAngle - iMinAngle);
	}
inline int AngleBearing (int iDir, int iTarget)
	{
	int iBearing = AngleRange(iDir, iTarget);
	if (iBearing > 180)
		return iBearing - 360;
	else
		return iBearing;
	}
inline Metric Ticks2Seconds (int iTicks) { return (Metric)iTicks * g_SecondsPerUpdate; }

//	Debugging Support

enum ProgramStates
	{
	psUnknown,							//	No idea what the program is doing now, but
										//		it is none of the other states.
	psAnimating,						//	Program is in Animate() function, but not in
										//		anything more specific.
	psPaintingSRS,						//	We are in Universe.PaintPOV()
	psUpdating,							//	In Universe.Update()
	psUpdatingEvents,					//	In Universe.Update(); updating event objects
	psUpdatingBehavior,					//	In Universe.Update(); updating obj behavior
	psUpdatingObj,						//	In Universe.Update(); updating obj
	psUpdatingHitTest,					//	In Universe.Update(); updating hit test
	psUpdatingMove,						//	In Universe.Update(); updating obj motion
	psUpdatingEncounters,				//	In Universe.Update(); updating random encounters
	psUpdatingExtended,					//	In System.UpdateExtended(); updating obj
	psDestroyNotify,					//	Calling OnObjDestroyed
	psUpdatingReactorDisplay,			//	In CReactorDisplay::Update
	psPaintingMap,						//	In CTranscendenceWnd::PaintMap
	psPaintingLRS,						//	In CTranscendenceWnd::PaintLRS
	psPaintingArmorDisplay,				//	In CArmorDisplay::Paint
	psPaintingMessageDisplay,			//	In CMessageDisplay::Paint
	psPaintingReactorDisplay,			//	In CReactorDisplay::Paint
	psPaintingTargetDisplay,			//	In CTargetDisplay::Paint
	psPaintingDeviceDisplay,			//	In CDeviceCounterDisplay::Paint
	psStargateEnter,					//	In CTranscendenceWnd::EnterStargate
	psStargateEndGame,					//	In EnterStargate, end game
	psStargateLoadingSystem,			//	In EnterStargate, loading system
	psStargateCreatingSystem,			//	In EnterStargate, creating system
	psStargateUpdateExtended,			//	In EnterStargate, updating system
	psStargateTransferringGateFollowers,//	In EnterStargate, transferring gate followers
	psStargateSavingSystem,				//	In EnterStargate, saving the system
	psStargateFlushingSystem,			//	In EnterStargate, flushing old system
	psStargateGarbageCollecting,		//	In EnterStargate, garbage collecting images
	psStargateLoadingBitmaps,			//	In EnterStargate, loading images
	psStargateEnterDone,				//	In EnterStargate, done
	psOnObjDestroyed,					//	In OnObjDestroyed
	psLoadingImages,					//	In CSpaceObject::LoadImages
	psCustomError,						//	g_sProgramError is set
	};

#ifdef DEBUG_PROGRAMSTATE
extern ProgramStates g_iProgramState;
extern CSpaceObject *g_pProgramObj;
extern CTimedEvent *g_pProgramEvent;
extern CString *g_sProgramError;
inline void SetProgramState (ProgramStates iState) { g_iProgramState = iState; g_pProgramObj = NULL; }
inline void SetProgramState (ProgramStates iState, CSpaceObject *pObj) { g_iProgramState = iState; g_pProgramObj = pObj; }
inline void SetProgramObj (CSpaceObject *pObj) { g_pProgramObj = pObj; }
inline void SetProgramEvent (CTimedEvent *pEvent) { g_pProgramEvent = pEvent; }
inline void SetProgramError (const CString &sError) { if (g_sProgramError) delete g_sProgramError; g_sProgramError = new CString(sError); g_iProgramState = psCustomError; }

#define DEBUG_SAVE_PROGRAMSTATE		\
	ProgramStates iDEBUG_SavedState = g_iProgramState;	\
	CSpaceObject *pDEBUG_SavedObj = g_pProgramObj;	\
	CTimedEvent *pDEBUG_SavedEvent = g_pProgramEvent;

#define DEBUG_RESTORE_PROGRAMSTATE	\
	g_iProgramState = iDEBUG_SavedState;	\
	g_pProgramObj = pDEBUG_SavedObj;	\
	g_pProgramEvent = pDEBUG_SavedEvent;

#else
inline void SetProgramState (ProgramStates iState) { }
inline void SetProgramState (ProgramStates iState, CSpaceObject *pObj) { }
inline void SetProgramObj (CSpaceObject *pObj) { }
inline void SetProgramEvent (CTimedEvent *pEvent) { }
inline void SetProgramError (const CString &sError) { }
#define DEBUG_SAVE_PROGRAMSTATE
#define DEBUG_RESTORE_PROGRAMSTATE
#endif

#ifdef DEBUG_PERFORMANCE
extern DWORD g_dwPerformanceTimer;
inline void DebugStartTimer (void) { g_dwPerformanceTimer = ::GetTickCount(); }
inline void DebugStopTimer (char *szTiming)
	{
	char szBuffer[1024];
	wsprintf(szBuffer, "%s: %d ms\n", szTiming, ::GetTickCount() - g_dwPerformanceTimer);
	::OutputDebugString(szBuffer);
	}
#else
inline void DebugStartTimer (void) { }
inline void DebugStopTimer (char *szTiming) { }
#endif

#define TRY(f)	{try { error = f; } catch (...) { error = ERR_FAIL; }}

//	Game load/save structures

#ifdef TRANSCENDENCE_STABLE_RELEASE
#define UPGRADE_ENTRY_UNID								CONSTLIT("Engine:Transcendence.stable")
#else
#define UPGRADE_ENTRY_UNID								CONSTLIT("Engine:Transcendence.next")
#endif

const DWORD API_VERSION =								25;		//	See: LoadExtensionVersion in Utilities.cpp
																//	See: ExtensionVersionToInteger in Utilities.cpp
const DWORD UNIVERSE_SAVE_VERSION =						25;
const DWORD SYSTEM_SAVE_VERSION =						107;	//	See: CSystem.cpp

struct SUniverseLoadCtx
	{
	DWORD dwVersion;					//	See CUniverse.cpp for version history
	DWORD dwSystemVersion;				//	System version when universe was saved

	IReadStream *pStream;				//	Stream to load from
	};

enum ELoadStates
	{
	loadStateUnknown,					//	Unknown load state
	loadStateObject,					//	Inside CSpaceObect::CreateFromStream
	loadStateObjData,					//	Loading obj opaque data
	loadStateObjEffects,				//	Loading the list of effects
	loadStateObjSubClass,				//	Load sub-class specific data
	loadStateEffect,					//	Loading an effect

	//	Make sure you add to the table in Utilities.cpp when adding new 
	//	entries to this enum.
	};

typedef void (*PRESOLVEOBJIDPROC) (void *pCtx, DWORD dwObjID, CSpaceObject *pObj);

class CSpaceObjectAddressResolver
	{
	public:
		bool HasUnresolved (void);
		void InsertRef (DWORD dwObjID, void *pCtx, PRESOLVEOBJIDPROC pfnResolveProc);
		void InsertRef (DWORD dwObjID, CSpaceObject **ppAddr);
		void ResolveRefs (DWORD dwObjID, CSpaceObject *pObj);

	private:
		struct SEntry
			{
			PRESOLVEOBJIDPROC pfnResolveProc;	//	If NULL, then pCtx is an address to fix up.
			void *pCtx;
			};

		TSortMap<DWORD, TArray<SEntry>> m_List;
	};

struct SLoadCtx
	{
	SLoadCtx (void) : 
			dwVersion(SYSTEM_SAVE_VERSION),
			pStream(NULL),
			pSystem(NULL),
			ObjMap(FALSE, TRUE),
			iLoadState(loadStateUnknown),
			dwObjClassID(0)
		{ }

	DWORD dwVersion;					//	See CSystem.cpp for version history

	IReadStream *pStream;				//	Stream to load from
	CSystem *pSystem;					//	System to load into

	CIDTable ObjMap;					//	Map of ID to objects.
	CSpaceObjectAddressResolver ForwardReferences;

	//	For backwards compatibility we keep track of the list of objects
	//	that want a subscription to the given ObjID.

	TSortMap<DWORD, TArray<CSpaceObject *>> Subscribed;

	//	Diagnostics

	ELoadStates iLoadState;				//	Current load state
	DWORD dwObjClassID;					//	ClassID that we're trying to load
	CString sEffectUNID;				//	UNID of effect we're loading
	};

struct SUpdateCtx
	{
	SUpdateCtx (void) :
			pSystem(NULL),
			pPlayer(NULL),
			pDockingObj(NULL),
			bNeedsAutoTarget(false),
			iPlayerPerception(0),
			pTargetObj(NULL),
			rTargetDist2(g_InfiniteDistance * g_InfiniteDistance),
			iMinFireArc(0),
			iMaxFireArc(0),
			bGravityWarning(false)
		{ }

	CSystem *pSystem;					//	Current system
	CSpaceObject *pPlayer;				//	The player

	//	Used to compute nearest docking port to player

	CSpaceObject *pDockingObj;			//	If non-null, nearest object to dock with
	int iDockingPort;					//	Nearest docking port
	CVector vDockingPort;				//	Position of docking port (absolute)
	Metric rDockingPortDist2;			//	Distance from player to docking port

	//	Used to compute player's auto target

	bool bNeedsAutoTarget;				//	TRUE if player's weapon needs an autotarget
	int iPlayerPerception;				//	Player's perception

	CSpaceObject *pTargetObj;			//	If non-null, nearest possible target for player
	Metric rTargetDist2;				//	Distance from player to target
	int iMinFireArc;					//	Fire arc of primary weapon
	int iMaxFireArc;

	bool bGravityWarning;				//	Player in a dangerous gravity field
	};

//	Utility classes

class CIDCounter
	{
	public:
		CIDCounter (DWORD dwFirstID = 1) : m_dwNextID(dwFirstID) { }
		DWORD GetID (void) { return m_dwNextID++; }

	private:
		DWORD m_dwNextID;
	};

class CAttributeCriteria
	{
	public:
		enum EMatchStrength
			{
			matchRequired =					0x00010000,
			matchExcluded =					0x00020000,
			};

		enum EFlags
			{
			flagDefault =					0x00000001,
			};

		CAttributeCriteria (void) :
				m_dwFlags(0)
			{ }

		int AdjLocationWeight (CSystem *pSystem, CLocationDef *pLoc, int iOriginalWeight = 1000) const;
		int AdjStationWeight (CStationType *pType, int iOriginalWeight = 1000) const;
		int CalcLocationWeight (CSystem *pSystem, const CString &sLocationAttribs, const CVector &vPos) const;
		inline int GetCount (void) const { return m_Attribs.GetCount(); }
		const CString &GetAttribAndRequired (int iIndex, bool *retbRequired) const;
		const CString &GetAttribAndWeight (int iIndex, DWORD *retdwMatchStrength, bool *retbIsSpecial = NULL) const;
		inline bool MatchesAll (void) const { return (GetCount() == 0); }
		inline bool MatchesDefault (void) const { return (m_dwFlags & flagDefault); }
		ALERROR Parse (const CString &sCriteria, DWORD dwFlags = 0, CString *retsError = NULL);

		static int CalcLocationWeight (CSystem *pSystem, const CString &sLocationAttribs, const CVector &vPos, const CString &sAttrib, DWORD dwMatchStrength);
		static int CalcWeightAdj (bool bHasAttrib, DWORD dwMatchStrength, int iAttribFreq = -1);

	private:
		enum MatchStrengthEncoding
			{
			CODE_MASK =						0xFFFF0000,
			VALUE_MASK =					0x0000FFFF,

			CODE_REQUIRED =					0x00010000,
			CODE_EXCLUDED =					0x00020000,
			CODE_SEEK =						0x00030000,		//	Value = 1-3
			CODE_AVOID =					0x00040000,		//	Value = 1-3
			CODE_INCREASE_IF =				0x00050000,		//	Value = % to increase
			CODE_INCREASE_UNLESS =			0x00060000,		//	Value = % to increase
			CODE_DECREASE_IF =				0x00070000,		//	Value = % to decrease
			CODE_DECREASE_UNLESS =			0x00080000,		//	Value = % to decrease

			matchSeek1 =					0x00030001,
			matchSeek2 =					0x00030002,
			matchSeek3 =					0x00030003,

			matchAvoid1 =					0x00040001,
			matchAvoid2 =					0x00040002,
			matchAvoid3 =					0x00040003,
			};

		struct SEntry
			{
			CString sAttrib;
			DWORD dwMatchStrength;
			bool bIsSpecial;
			};

		static int CalcWeightAdjCustom (bool bHasAttrib, DWORD dwMatchStrength);
		static int CalcWeightAdjWithAttribFreq (bool bHasAttrib, DWORD dwMatchStrength, int iAttribFreq);

		TArray<SEntry> m_Attribs;
		DWORD m_dwFlags;
	};

class DiceRange
	{
	public:
		DiceRange (void) : m_iFaces(0), m_iCount(0), m_iBonus(0) { }
		DiceRange (int iFaces, int iCount, int iBonus);

		inline int GetAveValue (void) const { return (m_iCount * (m_iFaces + 1) / 2) + m_iBonus; }
		inline Metric GetAveValueFloat (void) const { return (m_iFaces > 0 ? ((m_iCount * (m_iFaces + 1.0) / 2.0) + m_iBonus) : m_iBonus); }
		inline int GetBonus (void) const { return m_iBonus; }
		inline int GetCount (void) const { return m_iCount; }
		inline int GetFaces (void) const { return m_iFaces; }
		inline int GetMaxValue (void) const { return m_iFaces * m_iCount + m_iBonus; }
		inline int GetMinValue (void) const { return m_iCount + m_iBonus; }
		inline bool IsConstant (void) const { return (m_iFaces * m_iCount) == 0; }
		inline bool IsEmpty (void) const { return (m_iFaces == 0 && m_iCount == 0 && m_iBonus == 0); }
		int Roll (void) const;
		int RollSeeded (int iSeed) const;
		ALERROR LoadFromXML (const CString &sAttrib, int iDefault, CString *retsSuffix = NULL);
		inline ALERROR LoadFromXML (const CString &sAttrib, CString *retsSuffix = NULL) { return LoadFromXML(sAttrib, 0, retsSuffix); }
		void ReadFromStream (SLoadCtx &Ctx);
		CString SaveToXML (void) const;
		void Scale (Metric rScale);
		void SetConstant (int iValue) { m_iFaces = 0; m_iCount = 0; m_iBonus = iValue; }
		void WriteToStream (IWriteStream *pStream) const;

	private:
		int m_iFaces;
		int m_iCount;
		int m_iBonus;
	};

class CAttributeDataBlock
	{
	public:
		CAttributeDataBlock (void);
		CAttributeDataBlock (const CAttributeDataBlock &Src);
		CAttributeDataBlock &operator= (const CAttributeDataBlock &Src);
		~CAttributeDataBlock (void);

		inline void DeleteAll (void) { CleanUp(); }
		bool FindData (const CString &sAttrib, const CString **retsData = NULL) const;
		bool FindObjRefData (CSpaceObject *pObj, CString *retsAttrib = NULL) const;
		const CString &GetData (const CString &sAttrib) const;
		CString GetDataAttrib (int iIndex) const { return m_pData->GetKey(iIndex); }
		int GetDataCount (void) const { return (m_pData ? m_pData->GetCount() : 0); }
		CSpaceObject *GetObjRefData (const CString &sAttrib) const;
		inline bool IsEmpty (void) const { return (m_pData == NULL && m_pObjRefData == NULL); }
		bool IsEqual (const CAttributeDataBlock &Src);
		void LoadObjReferences (CSystem *pSystem);
		void MergeFrom (const CAttributeDataBlock &Src);
		void OnObjDestroyed (CSpaceObject *pObj);
		void OnSystemChanged (CSystem *pSystem);
		void ReadFromStream (SLoadCtx &Ctx);
		void ReadFromStream (IReadStream *pStream);
		void SetData (const CString &sAttrib, const CString &sData);
		void SetFromXML (CXMLElement *pData);
		void SetObjRefData (const CString &sAttrib, CSpaceObject *pObj);
		void WriteToStream (IWriteStream *pStream, CSystem *pSystem = NULL);

	private:
		struct SObjRefEntry
			{
			CString sName;
			CSpaceObject *pObj;
			DWORD dwObjID;

			SObjRefEntry *pNext;
			};

		void CleanUp (void);
		void Copy (const CAttributeDataBlock &Copy);
		bool IsXMLText (const CString &sData) const;

		CSymbolTable *m_pData;					//	Opaque string data
		SObjRefEntry *m_pObjRefData;			//	Custom pointers to CSpaceObject *
	};

class CCurrencyBlock
	{
	public:
		CurrencyValue GetCredits (DWORD dwEconomyUNID);
		CurrencyValue GetCredits (const CString &sCurrency);
		CurrencyValue IncCredits (DWORD dwEconomyUNID, CurrencyValue iInc);
		CurrencyValue IncCredits (const CString &sCurrency, CurrencyValue iInc);
		void ReadFromStream (SLoadCtx &Ctx);
		void SetCredits (DWORD dwEconomyUNID, CurrencyValue iValue);
		void SetCredits (const CString &sCurrency, CurrencyValue iValue);
		void WriteToStream (IWriteStream *pStream);

	private:
		struct SEntry
			{
			CurrencyValue iValue;
			};

		TSortMap<DWORD, SEntry> m_Block;
	};

struct SEventHandlerDesc
	{
	CExtension *pExtension;
	ICCItem *pCode;
	};

class CEventHandler
	{
	public:
		CEventHandler (void);
		~CEventHandler (void);

		CEventHandler &operator= (const CEventHandler &Src);

		void AddEvent (const CString &sEvent, ICCItem *pCode);
		ALERROR AddEvent (const CString &sEvent, const CString &sCode, CString *retsError = NULL);
		void DeleteAll (void);
		bool FindEvent (const CString &sEvent, ICCItem **retpCode) const;
		inline int GetCount (void) const { return m_Handlers.GetCount(); }
		const CString &GetEvent (int iIndex, ICCItem **retpCode = NULL) const;
		ALERROR InitFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);
		inline bool IsEmpty (void) const { return m_Handlers.GetCount() == 0; }
		void MergeFrom (const CEventHandler &Src);

	private:
		TSortMap<CString, ICCItem *> m_Handlers;
	};

class CGlobalEventCache
	{
	public:
		CGlobalEventCache (const CString &sEvent) : m_sEvent(sEvent) { }

		inline void DeleteAll (void) { m_Cache.DeleteAll(); }
		inline int GetCount (void) const { return m_Cache.GetCount(); }
		inline CDesignType *GetEntry (int iIndex, SEventHandlerDesc *retEvent = NULL) const
			{
			if (retEvent)
				*retEvent = m_Cache[iIndex].Event;

			return m_Cache[iIndex].pType;
			}
		bool Insert (CDesignType *pType, const CString &sEvent, const SEventHandlerDesc &Event);

	private:
		struct SEntry
			{
			CDesignType *pType;
			SEventHandlerDesc Event;
			};

		CString m_sEvent;
		TArray<SEntry> m_Cache;
	};

class CLanguageDataBlock
	{
	public:
		~CLanguageDataBlock (void);

		CLanguageDataBlock &operator= (const CLanguageDataBlock &Src);

		void AddEntry (const CString &sID, const CString &sText);
		void DeleteAll (void);
		ALERROR InitFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);
		void MergeFrom (const CLanguageDataBlock &Source);
		bool Translate (CSpaceObject *pObj, const CString &sID, ICCItem *pData, ICCItem **retpResult) const;
		bool Translate (CSpaceObject *pObj, const CString &sID, ICCItem *pData, TArray<CString> *retText) const;
		bool Translate (CSpaceObject *pObj, const CString &sID, ICCItem *pData, CString *retsText) const;

	private:
		enum ETranslateResult
			{
			resultArray,
			resultString,
			resultCCItem,

			resultFound,
			resultNotFound,
			};

		struct SEntry
			{
			CString sText;
			ICCItem *pCode;
			};

		ICCItem *ComposeCCItem (CCodeChain &CC, ICCItem *pValue, const CString &sPlayerName, GenomeTypes iPlayerGenome) const;
		ETranslateResult Translate (CSpaceObject *pObj, const CString &sID, ICCItem *pData, TArray<CString> *retText, CString *retsText, ICCItem **retpResult = NULL) const;

		TSortMap<CString, SEntry> m_Data;
	};

enum DestructionTypes
	{
	removedFromSystem				= 0,

	killedByDamage					= 1,	//	Ship destroyed
	killedByRunningOutOfFuel		= 2,	//	Ran out of fuel
	killedByRadiationPoisoning		= 3,	//	Radiation poisoning
	killedBySelf					= 4,	//	Self-destruct
	killedByDisintegration			= 5,	//	Ship disintegrated
	killedByWeaponMalfunction		= 6,	//	Weapon malfunction
	killedByEjecta					= 7,	//	Killed by ejecta
	killedByExplosion				= 8,	//	Killed when a station (or ship) exploded
	killedByShatter					= 9,	//	Killed by shatter effect
	killedByPlayerCreatedExplosion	= 10,	//	Killed by explosion created by the player
	enteredStargate					= 11,	//	Entered a stargate
	killedByOther					= 12,	//	Custom death
	killedByGravity					= 13,	//	Killed by white dwarf/neutron star/black hole

	killedNone						= -1,
	killedCount						= 14
	};

class CDamageSource
	{
	public:
		CDamageSource (void) : m_pSource(NULL), m_iCause(removedFromSystem), m_dwFlags(0), m_pSecondarySource(NULL) { }
		CDamageSource (CSpaceObject *pSource, DestructionTypes iCause = killedByDamage, CSpaceObject *pSecondarySource = NULL, const CString &sSourceName = NULL_STR, DWORD dwSourceFlags = 0);

		inline DestructionTypes GetCause (void) const { return m_iCause; }
		CString GetDamageCauseNounPhrase (DWORD dwFlags);
		CSpaceObject *GetObj (void) const;
		inline CSpaceObject *GetSecondaryObj (void) const { return m_pSecondarySource; }
		CSovereign *GetSovereign (void) const;
		DWORD GetSovereignUNID (void) const;
		inline bool HasDamageCause (void) const { return (m_pSource || !m_sSourceName.IsBlank()); }
		bool IsCausedByEnemyOf (CSpaceObject *pObj) const;
		bool IsCausedByFriendOf (CSpaceObject *pObj) const;
		bool IsCausedByNonFriendOf (CSpaceObject *pObj) const;
		inline bool IsCausedByPlayer (void) const { return ((m_dwFlags & FLAG_IS_PLAYER_CAUSED) ? true : false); }
		inline bool IsEmpty (void) const { return (GetObj() == NULL); }
		inline bool IsPlayer (void) const { return ((m_dwFlags & FLAG_IS_PLAYER) ? true : false); }
		void OnObjDestroyed (CSpaceObject *pObjDestroyed);
		void ReadFromStream (SLoadCtx &Ctx);
		inline void SetCause (DestructionTypes iCause) { m_iCause = iCause; }
		void SetObj (CSpaceObject *pSource);
		void WriteToStream (CSystem *pSystem, IWriteStream *pStream);
		
	private:
		enum Flags
			{
			FLAG_IS_PLAYER					= 0x00000001,
			FLAG_IS_PLAYER_SUBORDINATE		= 0x00000002,
			FLAG_IS_PLAYER_CAUSED			= 0x00000004,
			};

		CSpaceObject *m_pSource;
		DestructionTypes m_iCause;
		DWORD m_dwFlags;

		CString m_sSourceName;
		DWORD m_dwSourceNameFlags;

		CSpaceObject *m_pSecondarySource;
	};

class CDamageAdjDesc
	{
	public:
		CDamageAdjDesc (void) : m_pDefault(NULL)
			{ }

		ALERROR Bind (SDesignLoadCtx &Ctx, const CDamageAdjDesc *pDefault);
		inline int GetAdj (DamageTypes iDamageType) const { return (iDamageType == damageGeneric ? 100 : m_iDamageAdj[iDamageType]); }
		void GetAdjAndDefault (DamageTypes iDamageType, int *retiAdj, int *retiDefault) const;
		int GetHPBonus (DamageTypes iDamageType) const;
		ALERROR InitFromArray (int *pTable);
		ALERROR InitFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, bool bIsDefault = false);

	private:
		enum EAdjustmentTypes
			{
			adjDefault,							//	Use default table
			adjAbsolute,						//	dwAdjValue is an absolute adjustment
			adjRelative,						//	dwAdjValue is a relative percent of default
			};

		struct SAdjDesc
			{
			DWORD dwAdjType:16;					//	Type of adjustment
			DWORD dwAdjValue:16;				//	Adjustment value
			};

		void Compute (const CDamageAdjDesc *pDefault);
		ALERROR InitFromDamageAdj (SDesignLoadCtx &Ctx, const CString &sAttrib, bool bNoDefault);
		ALERROR InitFromHPBonus (SDesignLoadCtx &Ctx, const CString &sAttrib);

		SAdjDesc m_Desc[damageCount];			//	Descriptor for computing adjustment
		int m_iDamageAdj[damageCount];			//	Computed adjustment for type

		const CDamageAdjDesc *m_pDefault;		//	Default table
	};

class CRandomEntryResults
	{
	public:
		CRandomEntryResults (void);
		~CRandomEntryResults (void);

		void AddResult (CXMLElement *pElement, int iCount);
		inline int GetCount (void) { return m_Results.GetCount(); }
		CXMLElement *GetResult (int iIndex);
		int GetResultCount (int iIndex);

	private:
		CStructArray m_Results;
	};

class CRandomEntryGenerator
	{
	public:
		virtual ~CRandomEntryGenerator (void);

		virtual void Generate (CRandomEntryResults &Results) = 0;
		inline CXMLElement *GetElement (void) { return m_pElement; }
		inline int GetPercent (void) { return m_iPercent; }
		inline void SetPercent (int iPercent) { m_iPercent = iPercent; }

		static ALERROR Generate (CXMLElement *pElement, CRandomEntryResults &Results);
		static ALERROR GenerateAsGroup (CXMLElement *pElement, CRandomEntryResults &Results);
		static ALERROR GenerateAsTable (CXMLElement *pElement, CRandomEntryResults &Results);
		static ALERROR LoadFromXML (CXMLElement *pElement, CRandomEntryGenerator **retpGenerator);
		static ALERROR LoadFromXMLAsGroup (CXMLElement *pElement, CRandomEntryGenerator **retpGenerator);

	protected:
		CRandomEntryGenerator (void);
		CRandomEntryGenerator (CXMLElement *pElement);
		inline int GetCount (void) { return m_Count.Roll(); }

	private:
		CXMLElement *m_pElement;
		int m_iPercent;					//	Either chance or probability
		DiceRange m_Count;				//	Count
	};

class CSpaceObjectList
	{
	public:
		CSpaceObjectList (void);
		~CSpaceObjectList (void);

		void Add (CSpaceObject *pObj, int *retiIndex = NULL);
		inline void CleanUp (void) { m_List.DeleteAll(); }
		inline void FastAdd (CSpaceObject *pObj, int *retiIndex = NULL) { if (retiIndex) *retiIndex = m_List.GetCount(); m_List.Insert(pObj); }
		inline bool FindObj (CSpaceObject *pObj, int *retiIndex = NULL) const { return m_List.Find(pObj, retiIndex); }
		inline int GetCount (void) const { return m_List.GetCount(); }
		inline CSpaceObject *GetObj (int iIndex) const { return m_List[iIndex]; }
		inline TArray<CSpaceObject *> &GetRawList (void) { return m_List; }
		inline bool IsEmpty (void) const { return (m_List.GetCount() == 0); }
		void NotifyOnObjDestroyed (SDestroyCtx &Ctx);
		void NotifyOnObjDocked (CSpaceObject *pDockingObj, CSpaceObject *pDockTarget);
		void NotifyOnObjEnteredGate (CSpaceObject *pGatingObj, CTopologyNode *pDestNode, const CString &sDestEntryPoint, CSpaceObject *pStargate);
		void NotifyOnObjJumped (CSpaceObject *pJumpObj);
		void NotifyOnObjReconned (CSpaceObject *pReconnedObj);
		void NotifyOnPlayerBlacklisted (CSpaceObject *pBlacklistingObj);
		void ReadFromStream (SLoadCtx &Ctx, bool bIgnoreMissing = false);
		inline void Remove (int iIndex) { m_List.Delete(iIndex); }
		bool Remove (CSpaceObject *pObj);
		inline void RemoveAll (void) { m_List.DeleteAll(); }
		void RemoveSystemObjs (void);
		void SetAllocSize (int iNewCount);
		inline void SetObj (int iIndex, CSpaceObject *pObj) { m_List[iIndex] = pObj; }
		void Subtract (const CSpaceObjectList &List);
		void WriteToStream (CSystem *pSystem, IWriteStream *pStream);

	private:
		static void ResolveObjProc (void *pCtx, DWORD dwObjID, CSpaceObject *pObj);

		TArray<CSpaceObject *> m_List;
	};

class CSpaceObjectTable
	{
	public:
		CSpaceObjectTable (void);

		void Add (const CString &sKey, CSpaceObject *pObj);
		bool Find (CSpaceObject *pObj, int *retiIndex = NULL);
		inline int GetCount (void) { return m_Table.GetCount(); }
		inline CSpaceObject *Get (int iIndex) { return (CSpaceObject *)m_Table.GetValue(iIndex); }
		inline void Remove (int iIndex) { m_Table.RemoveEntry(m_Table.GetKey(iIndex), NULL); }
		bool Remove (CSpaceObject *pObj);
		inline void RemoveAll (void) { m_Table.RemoveAll(); }

	private:
		CSymbolTable m_Table;
	};

struct SDeviceEnhancementDesc
	{
	SDeviceEnhancementDesc (void) :
		iBonus(0),
		iActivateAdj(100),
		iMinActivateDelay(0),
		iMaxActivateDelay(0) { }

	CString sID;						//	ID of entity conferring bonus

	int iBonus;							//	%increase for hp/damage/etc (0 = none)

	int iActivateAdj;					//	Rate adjustment (100 = no adj)
	int iMinActivateDelay;				//	Do not decrease delay below this value (0 if no limit)
	int iMaxActivateDelay;				//	Do not increase delay above this value (0 if no limit)
	};

//	CTileMap (for nebulosity)

class CTileMapSection;

class CTile
	{
	public:
		CTile (void) : m_dwData(0) { }

		inline DWORD GetTile (void) { return m_dwData; }
		inline DWORD *GetTilePointer (void) { return &m_dwData; }
		inline CTileMapSection *GetTileMapSection (void) { return (CTileMapSection *)m_dwData; }
		inline void SetTile (DWORD dwTile) { m_dwData = dwTile; }
		inline void SetTileMapSection (CTileMapSection *pMap) { m_dwData = (DWORD)pMap; }

	private:
		DWORD m_dwData;
	};

class CTileMapSection
	{
	public:
		CTileMapSection (int iCount) { m_pMap = new CTile [iCount]; }
		~CTileMapSection (void) { delete [] m_pMap; }

		inline DWORD GetTile (int iIndex) { return m_pMap[iIndex].GetTile(); }
		inline DWORD *GetTilePointer (int iIndex) { return m_pMap[iIndex].GetTilePointer(); }
		inline CTileMapSection *GetTileMapSection (int iIndex) { return m_pMap[iIndex].GetTileMapSection(); }
		inline ALERROR ReadFromStream (int iCount, IReadStream *pStream) { return pStream->Read((char *)m_pMap, iCount * sizeof(CTile)); }
		inline void SetTile (int iIndex, DWORD dwTile) { m_pMap[iIndex].SetTile(dwTile); }
		inline void SetTileMapSection (int iIndex, CTileMapSection *pMap) { m_pMap[iIndex].SetTileMapSection(pMap); }
		inline void WriteToStream (int iCount, IWriteStream *pStream) const { pStream->Write((char *)m_pMap, iCount * sizeof(CTile)); }

	private:
		CTile *m_pMap;
	};

struct STileMapSectionPos
	{
	CTileMapSection *pMap;
	int iIndex;
	STileMapSectionPos *pParent;
	};

struct STileMapEnumerator
	{
	STileMapEnumerator (void) : pCurPos(NULL), bDone(false) { }

	STileMapSectionPos *pCurPos;
	bool bDone;
	};

class CTileMap
	{
	public:
		CTileMap (void);
		CTileMap (int iSize, int iScale);
		~CTileMap (void);

		static ALERROR CreateFromStream (IReadStream *pStream, CTileMap **retpMap);
		void GetNext (STileMapEnumerator &i, int *retx, int *rety, DWORD *retdwTile) const;
		DWORD GetTile (int x, int y) const;
		DWORD *GetTilePointer (int x, int y);
		inline int GetScale (void) const { return m_iScale; }
		int GetTotalSize (void) const;
		inline int GetSize (void) const { return m_iSize; }
		bool HasMore (STileMapEnumerator &i) const;
		void Init (int iSize, int iScale);
		ALERROR ReadFromStream (IReadStream *pStream);
		void SetTile (int x, int y, DWORD dwTile);
		void WriteToStream (IWriteStream *pStream) const;

	private:
		void ComputeDenominator (void);
		void CleanUpMapSection (CTileMapSection *pMap, int iScale);
		ALERROR ReadMapSection (IReadStream *pStream, int iScale, CTileMapSection **retpSection);
		bool SelectNext (STileMapEnumerator &i) const;
		void WriteMapSection (CTileMapSection *pMap, int iScale, IWriteStream *pStream) const;

		int m_iSize;
		int m_iScale;
		int m_iFirstDenominator;
		CTileMapSection *m_pMap;
	};

enum SpaceObjectGridFlags
	{
	gridNoBoxCheck			= 0x00000001,	//	Do not check for objects in the box
	};

struct SSpaceObjectGridEnumerator
	{
	SSpaceObjectGridEnumerator (void) : pGridIndexList(NULL) { }
	~SSpaceObjectGridEnumerator (void) { if (pGridIndexList) delete pGridIndexList; }

	CSpaceObject *pObj;						//	Current object
	int iGridIndex;							//	Current grid cell to search
	CSpaceObjectList *pList;				//	Current list
	int iIndex;								//	Current index
	int iListCount;							//	Number of elements in current list
	bool bMore;								//	TRUE if there is more

	int iGridIndexCount;					//	Number of grid indices to traverse
	CSpaceObjectList **pGridIndexList;		//	Array of grid indices to traverse

	bool bCheckBox;							//	If TRUE, only return objects in box
	CVector vLL;							//	Box to check
	CVector vUR;
	};

class CSpaceObjectGrid
	{
	public:
		CSpaceObjectGrid (int iGridSize, Metric rCellSize, Metric rCellBorder);
		~CSpaceObjectGrid (void);

		inline void AddObject (CSpaceObject *pObj);
		void DeleteAll (void);
		void EnumStart (SSpaceObjectGridEnumerator &i, const CVector &vUR, const CVector &vLL, DWORD dwFlags);
		inline bool EnumHasMore (SSpaceObjectGridEnumerator &i) { return i.bMore; }
		CSpaceObject *EnumGetNext (SSpaceObjectGridEnumerator &i);
		inline CSpaceObject *EnumGetNextFast (SSpaceObjectGridEnumerator &i)
			{
			ASSERT(i.iIndex >= 0);
			CSpaceObject *pCurObj = i.pList->GetObj(i.iIndex++);
			if (i.iIndex >= i.iListCount)
				EnumGetNextList(i);
			return pCurObj;
			}
		CSpaceObject *EnumGetNextInBoxPoint (SSpaceObjectGridEnumerator &i);
		void GetObjectsInBox (const CVector &vUR, const CVector &vLL, CSpaceObjectList &Result);

	private:
		bool EnumGetNextList (SSpaceObjectGridEnumerator &i);
		bool GetGridCoord (const CVector &vPos, int *retx, int *rety);
		CSpaceObjectList &GetList (const CVector &vPos);
		inline CSpaceObjectList &GetList (int x, int y) { return m_pGrid[y * m_iGridSize + x]; }

		CSpaceObjectList *m_pGrid;
		CSpaceObjectList m_Outer;

		int m_iGridSize;
		CVector m_vGridSize;
		Metric m_rCellSize;
		Metric m_rCellBorder;
		CVector m_vLL;
		CVector m_vUR;
	};

class CGameTimeKeeper
	{
	public:
		void AddDiscontinuity (int iTick, const CTimeSpan &Duration);
		inline void DeleteAll (void) { m_Discontinuities.DeleteAll(); }
		CTimeSpan GetElapsedTimeAt (int iTick);
		void ReadFromStream (IReadStream *pStream);
		void WriteToStream (IWriteStream *pStream);

	private:
		struct SDiscontinuity
			{
			LONGLONG iDuration;
			int iTick;
			};

		TArray<SDiscontinuity> m_Discontinuities;
	};

//	Events

class CTimedEvent
	{
	public:
		enum Classes
			{
			//	NOTE: These values are stored in the save file
			cTimedEncounterEvent,
			cTimedCustomEvent,
			cTimedRecurringEvent,
			cTimedTypeEvent,
			cTimedMissionEvent,
			};

		CTimedEvent (DWORD dwTick) : m_dwTick(dwTick), m_bDestroyed(false) { }
		virtual ~CTimedEvent (void) { }
		static void CreateFromStream (SLoadCtx &Ctx, CTimedEvent **retpEvent);

		inline DWORD GetTick (void) { return m_dwTick; }
		inline bool IsDestroyed (void) { return m_bDestroyed; }
		inline void SetDestroyed (void) { m_bDestroyed = true; }
		inline void SetTick (DWORD dwTick) { m_dwTick = dwTick; }
		void WriteToStream (CSystem *pSystem, IWriteStream *pStream);

		virtual CString DebugCrashInfo (void) { return NULL_STR; }
		virtual void DoEvent (DWORD dwTick, CSystem *pSystem) = 0;
		virtual CString GetEventHandlerName (void) { return NULL_STR; }
		virtual CSpaceObject *GetEventHandlerObj (void) { return NULL; }
		virtual CDesignType *GetEventHandlerType (void) { return NULL; }
		virtual bool OnObjChangedSystems (CSpaceObject *pObj) { return false; }
		virtual bool OnObjDestroyed (CSpaceObject *pObj) { return false; }

	protected:
		CTimedEvent (void) { }

		virtual void OnReadFromStream (SLoadCtx &Ctx) = 0;
		virtual void OnWriteClassToStream (IWriteStream *pStream) = 0;
		virtual void OnWriteToStream (CSystem *pSystem, IWriteStream *pStream) = 0;

	private:
		DWORD m_dwTick;
		bool m_bDestroyed;
	};

class CTimedEventList
	{
	public:
		~CTimedEventList (void);

		inline void AddEvent (CTimedEvent *pEvent) { m_List.Insert(pEvent); }
		bool CancelEvent (CSpaceObject *pObj, bool bInDoEvent);
		bool CancelEvent (CSpaceObject *pObj, const CString &sEvent, bool bInDoEvent);
		void DeleteAll (void);
		inline int GetCount (void) const { return m_List.GetCount(); }
		inline CTimedEvent *GetEvent (int iIndex) const { return m_List[iIndex]; }
		inline void MoveEvent (int iIndex, CTimedEventList &Dest) { Dest.AddEvent(m_List[iIndex]); m_List.Delete(iIndex); }
		void ReadFromStream (SLoadCtx &Ctx);
		inline void RemoveEvent (int iIndex) { delete m_List[iIndex]; m_List.Delete(iIndex); }
		void Update (DWORD dwTick, CSystem *pSystem);
		void WriteToStream (CSystem *pSystem, IWriteStream *pStream);

	private:
		TArray<CTimedEvent *> m_List;
	};

//	Linked-list template class

template <class TYPE> class TSEListNode
	{
	public:
		TSEListNode (void) : m_pNext(NULL) { }
		~TSEListNode (void)
			{
			TYPE *pNext = GetNext();
			while (pNext)
				{
				TYPE *pDelete = pNext;
				pNext = pNext->GetNext();

				pDelete->m_pNext = NULL;
				delete pDelete;
				}
			}

		int GetCount (void)
			{
			int iCount = 0;
			TSEListNode<TYPE> *pNext = m_pNext;
			while (pNext)
				{
				iCount++;
				pNext = pNext->m_pNext;
				}
			return iCount;
			}

		inline TYPE *GetNext (void) { return (TYPE *)m_pNext; }

		void Insert (TSEListNode<TYPE> *pNewNode)
			{
			pNewNode->m_pNext = m_pNext;
			m_pNext = pNewNode;
			}

		bool IsEmpty (void) const
			{
			return (m_pNext == NULL);
			}

		void ObjDestroyed (CSpaceObject *pObj)
			{
			TSEListNode<TYPE> *pPrev = this;
			TYPE *pNext = GetNext();
			while (pNext)
				{
				bool bRemoveNode;
				pNext->OnObjDestroyed(pObj, &bRemoveNode);
				if (bRemoveNode)
					{
					pPrev->m_pNext = pNext->GetNext();
					TYPE *pDelete = pNext;
					pNext = pNext->GetNext();

					pDelete->m_pNext = NULL;
					delete pDelete;
					}
				else
					{
					pPrev = pNext;
					pNext = pNext->GetNext();
					}
				}
			}

		void ReadFromStream (SLoadCtx &Ctx)
			{
			int i;
			DWORD dwCount;

			Ctx.pStream->Read((char *)&dwCount, sizeof(DWORD));
			TSEListNode<TYPE> *pInsertAt = this;
			for (i = 0; i < (int)dwCount; i++)
				{
				TYPE *pNew = new TYPE;
				pNew->OnReadFromStream(Ctx);
				pInsertAt->Insert(pNew);
				pInsertAt = pNew;
				}
			}

		void Remove (TSEListNode<TYPE> *pNodeToRemove)
			{
			TSEListNode<TYPE> *pPrev = this;
			TYPE *pNext = GetNext();
			while (pNext)
				{
				if (pNext == pNodeToRemove)
					{
					pPrev->m_pNext = pNext->GetNext();
					TYPE *pDelete = pNext;
					pNext = pNext->GetNext();

					pDelete->m_pNext = NULL;
					delete pDelete;
					break;
					}
				else
					{
					pPrev = pNext;
					pNext = pNext->GetNext();
					}
				}
			}

		void WriteToStream (CSystem *pSystem, IWriteStream *pStream)
			{
			DWORD dwCount = GetCount();
			pStream->Write((char *)&dwCount, sizeof(DWORD));
			TYPE *pNext = GetNext();
			while (pNext)
				{
				pNext->OnWriteToStream(pSystem, pStream);
				pNext = pNext->GetNext();
				}
			}

	private:
		TSEListNode<TYPE> *m_pNext;
	};

//	Regen class

class CRegenDesc
	{
	public:
		CRegenDesc (void) : m_bEmpty(true), m_iHPPerCycle(0), m_iHPPerEraRemainder(0) { }
		CRegenDesc (int iHPPerEra);

		void Add (const CRegenDesc &Desc);
		double GetHPPer180 (int iTicksPerCycle = 1) const;
		int GetHPPerEra (void) const;
		CString GetReferenceRate (const CString &sUnits, int iTicksPerCycle = 1) const;
		int GetRegen (int iTick, int iTicksPerCycle = 1) const;
		void Init (int iHPPerEra, int iCyclesPerBurst = 1);
		void InitFromRegen (double rRegen, int iTicksPerCycle = 1);
		ALERROR InitFromRegenString (SDesignLoadCtx &Ctx, const CString &sRegen, int iTicksPerCycle = 1);
		ALERROR InitFromRegenTimeAndHP (SDesignLoadCtx &Ctx, int iRegenTime, int iRegenHP, int iTicksPerCycle = 1);
		ALERROR InitFromRepairRateString (SDesignLoadCtx &Ctx, const CString &sRepairRate, int iTicksPerCycle = 1);
		ALERROR InitFromXML (SDesignLoadCtx &Ctx, 
							 CXMLElement *pDesc, 
							 const CString &sRegenAttrib, 
							 const CString &sRegenRate,
							 const CString &sRegenHP,
							 int iTicksPerCycle = 1);
		inline bool IsEmpty (void) const { return m_bEmpty; }

	private:
		int m_iHPPerCycle;					//	HP gained per cycle
		int m_iHPPerEraRemainder;			//	Extra HP to gain per era (1 era = 360 cycles)

		int m_iCyclesPerBurst;				//	Regen in bursts; each burst is this many cycles

		bool m_bEmpty;						//	If TRUE, no regen
	};

//	CZoneGrid ------------------------------------------------------------------

class CZoneGrid
	{
	public:
		CZoneGrid (void) : m_cxSize(-1), m_cySize(-1), m_iCellSize(-1), m_pDesc(NULL) { }

		ALERROR CreateFromXML (CXMLElement *pDesc, int cxWidth, int cyHeight, int iCellSize);
		bool ChooseRandomPoint (int *retx, int *rety) const;
		ALERROR CreateZone (int cxSize = -1, int cySize = -1, int iCellSize = 1);
		inline const CString &GetName (void) const { return m_sName; }
		int GetValue (int x, int y) const;
		ALERROR LoadFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);

	private:
		ALERROR ApplyZoneOperation (CXMLElement *pOp, CG16bitImage &DestMap);
		ALERROR CreateZoneMap (CG16bitImage &ZoneMap);
		int GetValueRaw (int x, int y) const;
		void MapCoord (int x, int y, int *retx, int *rety) const;
		inline int MapDim (int x) const { return x / m_iCellSize; }
		ALERROR ZoneOpCircle (CXMLElement *pOp, CG16bitImage &DestMap);
		ALERROR ZoneOpCircleGradient (CXMLElement *pOp, CG16bitImage &DestMap);
		ALERROR ZoneOpMultiply (CXMLElement *pOp, CG16bitImage &DestMap);
		ALERROR ZoneOpNoise (CXMLElement *pOp, CG16bitImage &DestMap);

		CString m_sName;
		int m_cxSize;						//	Size of grid in virtual units
		int m_cySize;
		int m_iCellSize;					//	Cell size in virtual units
											//		(1 cell = 1 pixel in m_ZoneMap)

		CG16bitImage m_ZoneMap;				//	Alpha channel used as a BYTE grid

		CXMLElement *m_pDesc;
	};

//	CBitGrid -------------------------------------------------------------------

class CBitGrid
	{
	public:
		CBitGrid (int cxWidth, int cyHeight) : m_cxWidth(cxWidth), m_cyHeight(cyHeight), m_Grid(cxWidth * cyHeight)
			{ }

		inline void Clear (int x, int y) { if (InBounds(x, y)) m_Grid.Clear(Coord(x, y)); }
		inline bool InBounds (int x, int y)
			{
			int xGrid = (x + m_cxWidth / 2);
			int yGrid = (m_cyHeight / 2 - y);
			return (xGrid >= 0 && xGrid < m_cxWidth && yGrid >= 0 && yGrid < m_cyHeight);
			}
		inline bool IsSet (int x, int y) { return m_Grid.IsSet(Coord(x, y)); }
		inline void Set (int x, int y) { if (InBounds(x, y)) m_Grid.Set(Coord(x, y)); }

	private:
		inline DWORD Coord (int x, int y) { return ((m_cyHeight / 2 - y) * m_cxWidth) + (x + m_cxWidth / 2); }

		int m_cxWidth;
		int m_cyHeight;
		CLargeSet m_Grid;
	};

//	C2DFunction ----------------------------------------------------------------

class I2DFunction
	{
	public:
		static ALERROR CreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, I2DFunction **retpFunc);

		virtual ~I2DFunction (void) { }

		inline float Eval (float x, float y) { return OnEval(x, y); }
		inline ALERROR InitFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc) { return OnInitFromXML(Ctx, pDesc); }

	protected:
		virtual float OnEval (float x, float y) { return 0.0f; }
		virtual ALERROR OnInitFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc) { return NOERROR; }
	};

class CNoise2DFunc : public I2DFunction
	{
	protected:
		virtual float OnEval (float x, float y);
		virtual ALERROR OnInitFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);

	private:
		int m_iScale;
	};

//	CComplexArea ---------------------------------------------------------------

class CComplexArea
	{
	public:
		CComplexArea (void);

		inline void ExcludeCircle (int x, int y, int iRadius) { AddCircle(m_ExcludedCircles, x, y, iRadius); }
		inline void ExcludeRect (int x, int y, int cxWidth, int cyHeight, int iRotation = 0) { AddRect(m_ExcludedRects, x, y, cxWidth, cyHeight, iRotation); }
		bool GeneratePointsInArea (int iCount, int iMinSeparation, TArray<int> *retX, TArray<int> *retY);
		bool GeneratePointsInArea (int iCount, int iMinSeparation, CIntGraph *retGraph);
		bool InArea (int x, int y);
		inline void IncludeCircle (int x, int y, int iRadius) { AddCircle(m_IncludedCircles, x, y, iRadius); }
		inline void IncludeRect (int x, int y, int cxWidth, int cyHeight, int iRotation = 0) { AddRect(m_IncludedRects, x, y, cxWidth, cyHeight, iRotation); }
		bool RandomPointInArea (int *retx, int *rety);

	private:
		struct SCircle
			{
			int x;
			int y;
			int iRadius2;
			};

		struct SRect
			{
			int x;
			int y;
			int cxWidth;
			int cyHeight;
			int iRotation;
			};

		void AddCircle (TArray<SCircle> &Array, int x, int y, int iRadius);
		void AddRect (TArray<SRect> &Array, int x, int y, int cxWidth, int cyHeight, int iRotation);
		void AddToBounds (int xLeft, int yTop, int xRight, int yBottom);
		bool InCircle (SCircle &Circle, int x, int y);
		bool InRect (SRect &Rect, int x, int y);

		TArray<SCircle> m_ExcludedCircles;
		TArray<SRect> m_ExcludedRects;
		TArray<SCircle> m_IncludedCircles;
		TArray<SRect> m_IncludedRects;

		RECT m_rcBounds;
	};

//	C3DConversion --------------------------------------------------------------

class C3DConversion
	{
	public:
		static void CalcCoord (int iScale, int iAngle, int iRadius, int iZ, int *retx, int *rety);
		static void CalcCoord (int iScale, int iAngle, int iRadius, int iZ, CVector *retvPos);
		static void CalcCoord (Metric rScale, const CVector &vPos, Metric rPosZ, CVector *retvPos);
		static void CalcCoordCompatible (int iAngle, int iRadius, int *retx, int *rety);

		inline void CleanUp (void) { m_Cache.DeleteAll(); }
		ALERROR Init (CXMLElement *pDesc);
		ALERROR Init (CXMLElement *pDesc, int iDirectionCount, int iScale, int iFacing);
		void Init (int iDirectionCount, int iScale, int iAngle, int iRadius, int iZ, int iFacing);
		void InitCompatible (int iDirectionCount, int iAngle, int iRadius, int iFacing);
		void InitCompatibleXY (int iDirectionCount, int iX, int iY, int iFacing);
		void InitComplete (int iDirectionCount, int iScale, int iFacing);
		void InitXY (int iDirectionCount, int iScale, int iX, int iY, int iZ, int iFacing);
		inline bool IsEmpty (void) const { return (m_Cache.GetCount() == 0); }
		void GetCoord (int iRotation, int *retx, int *rety) const;
		void GetCoordFromDir (int iDirection, int *retx, int *rety) const;
		inline bool PaintFirst (int iDirection) const { return m_Cache[iDirection].bPaintFirst; }

	private:
		struct SEntry
			{
			int x;
			int y;
			bool bPaintFirst;
			};

		bool InitCache (int iDirectionCount);
		ALERROR OverridePaintFirst (const CString &sAttrib, bool bPaintFirstValue);

		int m_iAngle;
		int m_iRadius;
		int m_iZ;
		bool m_bUseCompatible;
		CString m_sBringToFront;
		CString m_sSendToBack;

		TArray<SEntry> m_Cache;
	};

//	Wave Generators ------------------------------------------------------------

class CWaveGenerator2
	{
	public:
		CWaveGenerator2 (Metric rWave0Amp, Metric rWave0Cycles, Metric rWave1Amp, Metric rWave1Cycles, bool b0to1 = true);

		Metric GetValue (Metric rAngle);

	private:
		Metric m_rWave0Amp;
		Metric m_rWave0Cycles;
		Metric m_rWave1Amp;
		Metric m_rWave1Cycles;

		Metric m_rOffset;
	};

//	Local device storage class -------------------------------------------------

class CDeviceStorage
	{
	public:
		CDeviceStorage (void) : m_bModified(false) { }

		const CString &GetData (DWORD dwExtension, const CString &sAttrib) const;
		bool FindData (DWORD dwExtension, const CString &sAttrib, CString *retsData = NULL) const;
		ALERROR Load (const CString &sFilespec, CString *retsError = NULL);
		ALERROR Save (const CString &sFilespec);
		bool SetData (DWORD dwExtension, const CString &sAttrib, const CString &sData);

	private:
		CString MakeKey (DWORD dwExtension, const CString &sAttrib) const;
		bool ValidateAndMakeKey (DWORD dwExtension, const CString &sAttrib, CString *retsKey = NULL) const;

		TSortMap<CString, CString> m_Storage;
		bool m_bModified;
	};

//	Integral Rotation Class ----------------------------------------------------

//	IListData ------------------------------------------------------------------

extern const CItem g_DummyItem;
extern CItemListManipulator g_DummyItemListManipulator;

class IListData
	{
	public:
		virtual ~IListData (void) { }
		virtual void DeleteAtCursor (int iCount) { }
		virtual int GetCount (void) { return 0; }
		virtual int GetCursor (void) { return -1; }
		virtual CString GetDescAtCursor (void) { return NULL_STR; }
		virtual ICCItem *GetEntryAtCursor (CCodeChain &CC) { return CC.CreateNil(); }
		virtual const CItem &GetItemAtCursor (void) { return g_DummyItem; }
		virtual CItemListManipulator &GetItemListManipulator (void) { return g_DummyItemListManipulator; }
		virtual CSpaceObject *GetSource (void) { return NULL; }
		virtual CString GetTitleAtCursor (void) { return NULL_STR; }
		virtual bool IsCursorValid (void) { return false; }
		virtual bool MoveCursorBack (void) { return false; }
		virtual bool MoveCursorForward (void) { return false; }
		virtual void PaintImageAtCursor (CG16bitImage &Dest, int x, int y) { }
		virtual void ResetCursor (void) { }
		virtual void SetCursor (int iCursor) { }
		virtual void SetFilter (const CItemCriteria &Filter) { }
		virtual void SyncCursor (void) { }
	};

//	Miscellaneous utility functions

CString AppendModifiers (const CString &sModifierList1, const CString &sModifierList2);
CString ComposePlayerNameString (const CString &sString, const CString &sPlayerName, int iGenome, ICCItem *pArgs = NULL);
CString GetLoadStateString (ELoadStates iState);
Metric GetScale (CXMLElement *pObj);
bool HasModifier (const CString &sModifierList, const CString &sModifier);
inline bool IsRegisteredUNID (DWORD dwUNID) { return ((dwUNID & 0xF0000000) != 0xD0000000) && ((dwUNID & 0xF0000000) != 0xE0000000); }
inline bool IsReservedUNID (DWORD dwUNID) { return ((dwUNID & 0xF0000000) == 0xF0000000); }

ALERROR ParseDamageTypeList (const CString &sList, TArray<CString> *retList);
void ParseKeyValuePair (const CString &sString, DWORD dwFlags, CString *retsKey, CString *retsValue);

const DWORD PSL_FLAG_ALLOW_WHITESPACE =		0x00000001;
const DWORD PUL_FLAG_HEX =					0x00000002;
void ParseStringList (const CString &sList, DWORD dwFlags, TArray<CString> *retList);
inline void ParseAttributes (const CString &sAttribs, TArray<CString> *retAttribs) { ParseStringList(sAttribs, 0, retAttribs); }
void ParseIntegerList (const CString &sList, DWORD dwFlags, TArray<int> *retList);

void ParseUNIDList (const CString &sList, DWORD dwFlags, TArray<DWORD> *retList);

#ifdef LEVEL_ROMAN_NUMERALS
inline CString strLevel (int iLevel) { return strRomanNumeral(iLevel); }
#else
inline CString strLevel (int iLevel) { return strFromInt(iLevel); }
#endif

#endif