//	Transcendence.h
//
//	Main header file

#ifndef INCL_TRANSCENDENCE
#define INCL_TRANSCENDENCE

#ifndef INCL_TSE
#include "TSE.h"
#endif

#ifndef INCL_TSUI
#include "TSUI.h"
#endif


class CGameSession;
class CGameSettings;
class CIntroSession;
class CTranscendenceWnd;
class CUIResources;
class CTranscendenceController;
class CTranscendenceModel;

extern int g_cxScreen;
extern int g_cyScreen;
const int g_iColorDepth =					16;		//	Desired color depth

const int g_iFramesPerSecond =				30;		//	Desired frames per second
const int FRAME_RATE_COUNT =				51;		//	number of statistics to keep (for debugging)

#define OBJID_CPLAYERSHIPCONTROLLER	MakeOBJCLASSID(100)

extern CTranscendenceWnd *g_pTrans;

#define TICKS_BEFORE_GATE					34
#define TICKS_AFTER_GATE					30
#define TICKS_AFTER_DESTROYED				120

struct SCreateTrans
	{
	HINSTANCE hInst;
	char *pszCommandLine;
	bool bWindowed;
	};

struct SFontTable
	{
	CG16bitFont Small;				//	10 pixels
	CG16bitFont Medium;				//	13 pixels
	CG16bitFont MediumBold;			//	13 pixels bold
	CG16bitFont MediumHeavyBold;	//	14 pixels bold
	CG16bitFont Large;				//	16 pixels
	CG16bitFont LargeBold;			//	16 pixels bold
	CG16bitFont Header;				//	19 pixels (messages, map system name, picker)
	CG16bitFont HeaderBold;			//	19 pixels bold (help page titles)
	CG16bitFont SubTitle;			//	26 pixels (crawl screen, "Loading", medium credits, ship class disp)
	CG16bitFont SubTitleBold;		//	26 pixels bold (not used)
	CG16bitFont SubTitleHeavyBold;	//	28 pixels bold (dock screen counter)
	CG16bitFont Title;				//	56 pixels (dock screen title and large credits)
	CG16bitFont LogoTitle;			//	60 pixel Blue Island

	CG16bitFont Console;			//	Fixed-width font

	CG32bitPixel rgbTitleColor;			//	Large text color
	CG32bitPixel rgbLightTitleColor;	//	A lighter text color
	CG32bitPixel rgbTextColor;			//	Color for large blocks
	CG32bitPixel rgbHelpColor;			//	Faded help text
	CG32bitPixel rgbBackground;			//	Almost black background
	CG32bitPixel rgbSectionBackground;	//	Ligher section background
	CG32bitPixel rgbSelectBackground;	//	Selection background

	CG32bitPixel rgbAltRedColor;		//	Complementary alternate color
	CG32bitPixel rgbAltRedBackground;	//	Complementary background
	CG32bitPixel rgbAltGreenColor;		//	Complementary alternate color
	CG32bitPixel rgbAltGreenBackground;	//	Complementary background
	CG32bitPixel rgbAltYellowColor;		//	Complementary alternate color
	CG32bitPixel rgbAltYellowBackground;//	Complementary background
	CG32bitPixel rgbAltBlueColor;		//	Complementary alternate color
	CG32bitPixel rgbAltBlueBackground;	//	Complementary background

	CG32bitPixel rgbItemTitle;			//	Item title color
	CG32bitPixel rgbItemRef;			//	Item reference text color
	CG32bitPixel rgbItemDesc;			//	Item description color
	CG32bitPixel rgbItemDescSelected;	//	Item description when selected
	};

#include "CGAreas.h"
#include "DockScreen.h"

//	Intro

enum TargetTypes
	{
	targetEnemies,
	targetFriendlies,
	};

struct SNewGameSettings
	{
	SNewGameSettings (void) :
			iPlayerGenome(genomeUnknown),
			dwPlayerShip(0),
			bFullCreate(false),
			bDefaultPlayerName(false)
		{ }

	CString sPlayerName;						//	Character name
	GenomeTypes iPlayerGenome;					//	Genome
	DWORD dwPlayerShip;							//	Starting ship class

	bool bFullCreate;							//	If TRUE, create all systems
	bool bDefaultPlayerName;					//	If TRUE, this is a default player name
	};

struct SAdventureSettings
	{
	CExtension *pAdventure;						//	Adventure to create
	TArray<CExtension *> Extensions;			//	List of extensions
	};

class CPlayerGameStats
	{
	public:
		enum EEventTypes
			{
			eventNone =							0,

			eventEnemyDestroyedByPlayer	=		1,		//	The player destroyed something important
			eventFriendDestroyedByPlayer =		2,		//	The player destroyed something important
			eventSavedByPlayer =				3,		//	The player saved something from being destroyed
			eventMajorDestroyed =				4,		//	A major object was destroyed, but not by the player
			eventMissionSuccess =				5,		//	The player successfully completed a mission
			eventMissionFailure =				6,		//	The player failed a mission
			};

		CPlayerGameStats (void);

		int CalcEndGameScore (void) const;
		void GenerateGameStats (CGameStats &Stats, CSpaceObject *pPlayer, bool bGameOver) const;
		int GetBestEnemyShipsDestroyed (DWORD *retdwUNID = NULL) const;
		CTimeSpan GetGameTime (void) const { return (!m_GameTime.IsBlank() ? m_GameTime : g_pUniverse->GetElapsedGameTime()); }
		CString GetItemStat (const CString &sStat, ICCItem *pItemCriteria) const;
		CString GetKeyEventStat (const CString &sStat, const CString &sNodeID, const CDesignTypeCriteria &Crit) const;
		CTimeSpan GetPlayTime (void) const { return (!m_PlayTime.IsBlank() ? m_PlayTime : g_pUniverse->StopGameTime()); }
		CString GetStat (const CString &sStat) const;
		DWORD GetSystemEnteredTime (const CString &sNodeID);
		int IncStat (const CString &sStat, int iInc = 1);
		void OnGameEnd (CSpaceObject *pPlayer);
		void OnItemBought (const CItem &Item, CurrencyValue iTotalPrice);
		void OnItemDamaged (const CItem &Item, int iHP);
		void OnItemFired (const CItem &Item);
		void OnItemInstalled (const CItem &Item);
		void OnItemSold (const CItem &Item, CurrencyValue iTotalPrice);
		void OnItemUninstalled (const CItem &Item);
		void OnKeyEvent (EEventTypes iType, CSpaceObject *pObj, DWORD dwCauseUNID);
		void OnObjDestroyedByPlayer (const SDestroyCtx &Ctx, CSpaceObject *pPlayer);
		void OnSystemEntered (CSystem *pSystem, int *retiLastVisit = NULL);
		void OnSystemLeft (CSystem *pSystem);
		void ReadFromStream (SLoadCtx &Ctx);
		void SetStat (const CString &sStat, const CString &sValue);
		void WriteToStream (IWriteStream *pStream);

	private:
		struct SItemTypeStats
			{
			int iCountSold;						//	Number of items sold
			CurrencyValue iValueSold;			//	Total value received for selling (credits)

			int iCountBought;					//	Number of items bought
			CurrencyValue iValueBought;			//	Total value spent buying (credits)

			int iCountInstalled;				//	Number of items currently installed on player ship
			DWORD dwFirstInstalled;				//	First time item was installed on player ship
			DWORD dwLastInstalled;				//	Last time item was installed on player ship
			DWORD dwLastUninstalled;			//	Last time item was uninstalled from player ship
			DWORD dwTotalInstalledTime;			//	Total time installed

			int iCountFired;					//	Number of times item (weapon) has been fired by player
			int iHPDamaged;						//	HP absorbed by this item type when installed on player
			};

		struct SKeyEventStats
			{
			EEventTypes iType;					//	Type of event
			DWORD dwTime;						//	When the event happened
			DWORD dwObjUNID;					//	UNID of object
			CString sObjName;					//	For unique objects (e.g., CSCs)
			DWORD dwObjNameFlags;				//	Flags for the name
			DWORD dwCauseUNID;					//	The UNID of the sovereign who caused the event
			};

		struct SKeyEventStatsResult
			{
			CString sNodeID;
			SKeyEventStats *pStats;
			bool bMarked;
			};

		struct SShipClassStats
			{
			int iEnemyDestroyed;				//	Number of enemy ships destroyed
			int iFriendDestroyed;				//	Number of friendly ships destroyed
			};

		struct SStationTypeStats
			{
			int iDestroyed;						//	Number of stations destroyed
			};

		struct SSystemStats
			{
			DWORD dwFirstEntered;				//	First time this system was entered (0xffffffff = never)
			DWORD dwLastEntered;				//	Last time this system was entered (0xffffffff = never)
			DWORD dwLastLeft;					//	Last time this system was left (0xffffffff = never)
			DWORD dwTotalTime;					//	Total time in system (all visits)
			};

		bool AddMatchingKeyEvents (const CString &sNodeID, const CDesignTypeCriteria &Crit, TArray<SKeyEventStats> *pEventList, TArray<SKeyEventStatsResult> *retList) const;
		bool FindItemStats (DWORD dwUNID, SItemTypeStats **retpStats) const;
		CString GenerateKeyEventStat (TArray<SKeyEventStatsResult> &List) const;
		SItemTypeStats *GetItemStats (DWORD dwUNID);
		bool GetMatchingKeyEvents (const CString &sNodeID, const CDesignTypeCriteria &Crit, TArray<SKeyEventStatsResult> *retList) const;
		SShipClassStats *GetShipStats (DWORD dwUNID);
		SStationTypeStats *GetStationStats (DWORD dwUNID);
		SSystemStats *GetSystemStats (const CString &sNodeID);

		int m_iScore;							//	Total score for player
		int m_iResurrectCount;					//	Number of times player has resurrected a game
		CTimeSpan m_PlayTime;					//	Total time spent playing the game
		CTimeSpan m_GameTime;					//	Total elapsed time in the game

		TMap<DWORD, SItemTypeStats> m_ItemStats;
		TMap<DWORD, SShipClassStats> m_ShipStats;
		TMap<DWORD, SStationTypeStats> m_StationStats;
		TMap<CString, SSystemStats> m_SystemStats;
		TMap<CString, TArray<SKeyEventStats>> m_KeyEventStats;

		int m_iExtraSystemsVisited;				//	For backwards compatibility
		int m_iExtraEnemyShipsDestroyed;		//	For backwards compatibility
	};

enum UIMessageTypes
	{
	uimsgUnknown =					-1,
	uimsgEnabledHints =				-2,

	uimsgAllMessages =				0,
	uimsgAllHints =					1,			//	IsEnabled(uimsgAllHints) returns TRUE if ANY hint is enabled
												//	SetEnabled(uimsgAllHints) enables/disables ALL hints

	uimsgCommsHint =				2,
	uimsgDockHint =					3,
	uimsgMapHint =					4,
	uimsgAutopilotHint =			5,
	uimsgGateHint =					6,
	uimsgUseItemHint =				7,
	uimsgRefuelHint =				8,
	uimsgEnableDeviceHint =			9,
	uimsgSwitchMissileHint =		10,
	uimsgFireMissileHint =			11,
	uimsgGalacticMapHint =			12,

	uimsgCount =					13,
	};

class CUIMessageController
	{
	public:
		CUIMessageController (void);

		UIMessageTypes Find (const CString &sMessageName);
		inline bool IsEnabled (UIMessageTypes iMsg) const { return m_bMsgEnabled[iMsg]; }
		void ReadFromStream (SLoadCtx &Ctx);
		void SetEnabled (UIMessageTypes iMsg, bool bEnabled = true);
		void WriteToStream (IWriteStream *pStream);

	private:
		bool IsHint (UIMessageTypes iMsg);

		bool m_bMsgEnabled[uimsgCount];
	};

class CPlayerShipController : public IShipController
	{
	public:
		CPlayerShipController (void);
		~CPlayerShipController (void);

		void Cargo (void);
		inline CurrencyValue Charge (DWORD dwEconUNID, CurrencyValue iCredits) { return m_Credits.IncCredits(dwEconUNID, -iCredits); }

		bool CanShowShipStatus (void);
		void Communications (CSpaceObject *pObj, MessageTypes iMsg, DWORD dwData = 0, DWORD *iodwFormationPlace = NULL);
		void Dock (void);
		inline bool DockingInProgress (void) { return m_pStation != NULL; }
		inline UIMessageTypes FindUIMessage (const CString &sName) { return m_UIMsgs.Find(sName); }
		void Gate (void);
		void GenerateGameStats (CGameStats &Stats, bool bGameOver);
		inline int GetBestEnemyShipsDestroyed (DWORD *retdwUNID = NULL) { return m_Stats.GetBestEnemyShipsDestroyed(retdwUNID); }
		inline CurrencyValue GetCredits (DWORD dwEconUNID) { return m_Credits.GetCredits(dwEconUNID); }
		inline int GetCargoSpace (void) { return (int)(m_pShip->GetCargoSpaceLeft() + 0.5); }
		inline int GetEndGameScore (void) { return m_Stats.CalcEndGameScore(); }
		inline int GetEnemiesDestroyed (void) { return ::strToInt(m_Stats.GetStat(CONSTLIT("enemyShipsDestroyed")), 0); }
		inline CString GetItemStat (const CString &sStat, ICCItem *pItemCriteria) const { return m_Stats.GetItemStat(sStat, pItemCriteria); }
		inline CString GetKeyEventStat (const CString &sStat, const CString &sNodeID, const CDesignTypeCriteria &Crit) const { return m_Stats.GetKeyEventStat(sStat, sNodeID, Crit); }
		inline GenomeTypes GetPlayerGenome (void) const { return m_iGenome; }
		inline CString GetPlayerName (void) const { return m_sName; }
		inline int GetResurrectCount (void) const { return ::strToInt(m_Stats.GetStat(CONSTLIT("resurrectCount")), 0); }
		inline int GetScore (void) { return ::strToInt(m_Stats.GetStat(CONSTLIT("score")), 0); }
		inline CSpaceObject *GetSelectedTarget (void) { return m_pTarget; }
		inline CShip *GetShip (void) { return m_pShip; }
		inline DWORD GetStartingShipClass (void) const { return m_dwStartingShipClass; }
		inline CString GetStat (const CString &sStat) { return m_Stats.GetStat(sStat); }
		inline DWORD GetSystemEnteredTime (const CString &sNodeID) { return m_Stats.GetSystemEnteredTime(sNodeID); }
		inline int GetSystemsVisited (void) { return ::strToInt(m_Stats.GetStat(CONSTLIT("systemsVisited")), 0); }
		inline CTranscendenceWnd *GetTrans (void) { return m_pTrans; }
		inline void IncScore (int iBonus) { m_Stats.IncStat(CONSTLIT("score"), iBonus); }
		void Init (CTranscendenceWnd *pTrans);
		void InsuranceClaim (void);
		inline bool IsGalacticMapAvailable (void) { return (m_pShip && (m_pShip->GetAbility(::ablGalacticMap) > ::ablUninstalled)); }
		inline bool IsMapHUDActive (void) { return m_bMapHUD; }
		inline bool IsUIMessageEnabled (UIMessageTypes iMsg) { return m_UIMsgs.IsEnabled(iMsg); }
		void OnEnemyShipsDetected (void);
		inline void OnGameEnd (void) { m_Stats.OnGameEnd(m_pShip); }
		inline void OnItemBought (const CItem &Item, CurrencyValue iTotalPrice) { m_Stats.OnItemBought(Item, iTotalPrice); }
		inline void OnItemSold (const CItem &Item, CurrencyValue iTotalPrice) { m_Stats.OnItemSold(Item, iTotalPrice); }
		void OnSystemEntered (CSystem *pSystem, int *retiLastVisit = NULL) { m_Stats.OnSystemEntered(pSystem, retiLastVisit); }
		void OnSystemLeft (CSystem *pSystem) { m_Stats.OnSystemLeft(pSystem); }
		inline CurrencyValue Payment (DWORD dwEconUNID, CurrencyValue iCredits) { return m_Credits.IncCredits(dwEconUNID, iCredits); }
		void ReadyNextWeapon (int iDir = 1);
		void ReadyNextMissile (int iDir = 1);
		void SetDestination (CSpaceObject *pTarget);
		inline void SetGenome (GenomeTypes iGenome) { m_iGenome = iGenome; }
		inline void SetMapHUD (bool bActive) { m_bMapHUD = bActive; }
		inline void SetName (const CString &sName) { m_sName = sName; }
		inline void SetResurrectCount (int iCount) { m_Stats.SetStat(CONSTLIT("resurrectCount"), ::strFromInt(iCount)); }
		inline void SetStartingShipClass (DWORD dwUNID) { m_dwStartingShipClass = dwUNID; }
		void SetTarget (CSpaceObject *pTarget);
		void SelectNearestTarget (void);
		void SelectNextFriendly (int iDir = 1);
		void SelectNextTarget (int iDir = 1);
		inline void SetActivate (bool bActivate) { m_bActivate = bActivate; }
		void SetFireMain (bool bFire);
		inline void SetFireMissile (bool bFire);
		inline void SetShip (CShip *pShip) { m_pShip = pShip; }
		inline void SetStopThrust (bool bStop) { m_bStopThrust = bStop; }
		inline void SetUIMessageEnabled (UIMessageTypes iMsg, bool bEnabled = true) { m_UIMsgs.SetEnabled(iMsg, bEnabled); }
		ALERROR SwitchShips (CShip *pNewShip);
		void Undock (void);
		void Update (int iTick);

		//	Device methods
		bool AreAllDevicesEnabled (void);
		void EnableAllDevices (bool bEnable = true);
		bool ToggleEnableDevice (int iDeviceIndex);

		//	Fleet formation methods
		DWORD GetCommsStatus (void);
		bool HasFleet (void);

		//	IShipController virtuals
		virtual void AddOrder (OrderTypes Order, CSpaceObject *pTarget, const IShipController::SData &Data, bool bAddBefore = false);
		virtual void CancelAllOrders (void);
		virtual void CancelCurrentOrder (void);
		virtual void CancelDocking (void);
		virtual CString DebugCrashInfo (void);
		virtual CString GetAISettingString (const CString &sSetting);
		virtual CString GetClass (void) { return CONSTLIT("player"); }
		virtual int GetCombatPower (void);
		virtual CCurrencyBlock *GetCurrencyBlock (void) { return &m_Credits; }
		virtual OrderTypes GetCurrentOrderEx (CSpaceObject **retpTarget = NULL, IShipController::SData *retData = NULL);
		virtual CSpaceObject *GetDestination (void) const { return m_pDestination; }
		virtual EManeuverTypes GetManeuver (void);
		virtual bool GetThrust (void);
		virtual CSpaceObject *GetTarget (CItemCtx &ItemCtx, bool bNoAutoTarget = false) const;
		virtual bool GetReverseThrust (void);
		virtual bool GetStopThrust (void);
		virtual CSpaceObject *GetOrderGiver (void) { return m_pShip; }
		virtual bool GetDeviceActivate (void);
		virtual int GetFireDelay (void) { return (int)((5.0 / STD_SECONDS_PER_UPDATE) + 0.5); }
		virtual void GetWeaponTarget (STargetingCtx &TargetingCtx, CItemCtx &ItemCtx, CSpaceObject **retpTarget, int *retiFireSolution);
		virtual bool IsPlayer (void) const { return true; }
		virtual void ReadFromStream (SLoadCtx &Ctx, CShip *pShip);
		virtual void SetManeuver (EManeuverTypes iManeuver) { m_iManeuver = iManeuver; }
		virtual void SetThrust (bool bThrust) { m_bThrust = bThrust; }
		virtual void WriteToStream (IWriteStream *pStream);

		//	Events
		virtual void OnArmorRepaired (int iSection);
		virtual void OnBlindnessChanged (bool bBlind, bool bNoMessage = false);
		virtual DWORD OnCommunicate (CSpaceObject *pSender, MessageTypes iMessage, CSpaceObject *pParam1, DWORD dwParam2);
		virtual void OnComponentChanged (ObjectComponentTypes iComponent);
		virtual void OnDamaged (const CDamageSource &Cause, CInstalledArmor *pArmor, const DamageDesc &Damage, int iDamage);
		virtual bool OnDestroyCheck (DestructionTypes iCause, const CDamageSource &Attacker);
		virtual void OnDestroyed (SDestroyCtx &Ctx);
		virtual void OnDeviceEnabledDisabled (int iDev, bool bEnable, bool bSilent = false);
		virtual void OnDeviceStatus (CInstalledDevice *pDev, int iEvent);
		virtual void OnDocked (CSpaceObject *pObj);
		virtual void OnDockedObjChanged (CSpaceObject *pLocation);
		virtual void OnEnterGate (CTopologyNode *pDestNode, const CString &sDestEntryPoint, CSpaceObject *pStargate, bool bAscend);
		virtual void OnFuelLowWarning (int iSeq);
		virtual void OnItemDamaged (const CItem &Item, int iHP) { m_Stats.OnItemDamaged(Item, iHP); }
		virtual void OnItemFired (const CItem &Item) { m_Stats.OnItemFired(Item); }
		virtual void OnItemInstalled (const CItem &Item) { m_Stats.OnItemInstalled(Item); }
		virtual void OnItemUninstalled (const CItem &Item) { m_Stats.OnItemUninstalled(Item); }
		virtual void OnLifeSupportWarning (int iSecondsLeft);
		virtual void OnMissionCompleted (CMission *pMission, bool bSuccess);
		virtual void OnNewSystem (CSystem *pSystem);
		virtual void OnObjDamaged (const SDamageCtx &Ctx);
		virtual void OnObjDestroyed (const SDestroyCtx &Ctx);
		virtual void OnPaintSRSEnhancements (CG32bitImage &Dest, SViewportPaintCtx &Ctx);
		virtual void OnProgramDamage (CSpaceObject *pHacker, const ProgramDesc &Program);
		virtual void OnRadiationWarning (int iTicksLeft);
		virtual void OnRadiationCleared (void);
		virtual void OnReactorOverloadWarning (int iSeq);
		void OnStartGame (void);
		virtual void OnStationDestroyed (const SDestroyCtx &Ctx);
		virtual void OnUpdatePlayer (SUpdateCtx &Ctx);
		virtual void OnWeaponStatusChanged (void);
		virtual void OnWreckCreated (CSpaceObject *pWreck);

	private:
		void ClearFireAngle (void);
		CSpaceObject *FindAutoTarget (CItemCtx &ItemCtx) const;
		CSpaceObject *FindDockTarget (void);
		bool HasCommsTarget (void);
		void InitTargetList (TargetTypes iTargetType, bool bUpdate = false);
		void PaintTargetingReticle (SViewportPaintCtx &Ctx, CG32bitImage &Dest, CSpaceObject *pTarget);
		void Reset (void);
		void UpdateHelp (int iTick);

		CTranscendenceWnd *m_pTrans;
		CShip *m_pShip;

		OrderTypes m_iOrder;					//	Last order
		CSpaceObject *m_pTarget;
		CSpaceObject *m_pDestination;
		CSpaceObjectTable m_TargetList;

		CSpaceObject *m_pStation;				//	Station that player is docked with
		bool m_bSignalDock;						//	Tell the model to switch to dock screen

		DWORD m_dwWreckObjID;					//	WreckObjID (temp while we resurrect)

		int m_iLastHelpTick;
		int m_iLastHelpUseTick;
		int m_iLastHelpFireMissileTick;

		EManeuverTypes m_iManeuver;
		bool m_bThrust;
		bool m_bActivate;
		bool m_bStopThrust;

		bool m_bMapHUD;							//	Show HUD on map
		bool m_bDockPortIndicators;				//	Dock ports light up when near by

		CCurrencyBlock m_Credits;				//	Money available to player
		CPlayerGameStats m_Stats;				//	Player stats, including score
		CUIMessageController m_UIMsgs;			//	Status of various UI messages, such as hints

		CString m_sName;						//	Player name
		GenomeTypes m_iGenome;					//	Player genome
		DWORD m_dwStartingShipClass;			//	Starting ship class

		bool m_bUnderAttack;					//	TRUE if we're currently under attack

		CSpaceObject *m_pAutoDock;				//	The current station to dock with if we were to 
												//		press 'D' right now. NULL means no station
												//		to dock with.
		int m_iAutoDockPort;					//	The current dock port.
		CVector m_vAutoDockPort;				//	The current dock port position;

		bool m_bShowAutoTarget;					//	If TRUE, we show the autotarget
		CSpaceObject *m_pAutoTarget;			//	Saved autotarget.
		mutable int m_iAutoTargetTick;

		CSpaceObject *m_pAutoDamage;			//	Show damage bar for this object
		DWORD m_dwAutoDamageExpire;				//	Stop showing on this tick

	friend CObjectClass<CPlayerShipController>;
	};

class CMessageDisplay : public CObject
	{
	public:
		CMessageDisplay (void);

		void ClearAll (void);
		void DisplayMessage (CString sMessage, CG32bitPixel rgbColor);
		void Paint (CG32bitImage &Dest);
		void Update (void);

		inline void SetBlinkTime (int iTime) { m_iBlinkTime = iTime; }
		inline void SetFadeTime (int iTime) { m_iFadeTime = iTime; }
		inline void SetFont (CG16bitFont *pFont) { m_pFont = pFont; }
		inline void SetRect (RECT &rcRect) { m_rcRect = rcRect; }
		inline void SetSteadyTime (int iTime) { m_iSteadyTime = iTime; }

	private:
		enum Constants
			{
			MESSAGE_QUEUE_SIZE = 5,
			};

		enum State
			{
			stateClear,						//	Blank (stays permanently)
			stateNormal,					//	Normal (stays permanently)
			stateBlinking,					//	Blinking (for m_iBlinkTime)
			stateSteady,					//	Normal (for m_iSteadyTime)
			stateFading						//	Fade to black (for m_iFadeTime)
			};

		struct SMessage
			{
			CString sMessage;				//	Message to paint
			int x;							//	Location of message
			State iState;					//	current state (blinking, etc)
			int iTick;						//	Tick count for this message
			CG32bitPixel rgbColor;					//	Color to paint
			};

		inline int Next (int iPos) { return ((iPos + 1) % MESSAGE_QUEUE_SIZE); }
		inline int Prev (int iPos) { return ((iPos + MESSAGE_QUEUE_SIZE - 1) % MESSAGE_QUEUE_SIZE); }

		RECT m_rcRect;

		CG16bitFont *m_pFont;				//	Font to use (not owned)
		int m_iBlinkTime;
		int m_iSteadyTime;
		int m_iFadeTime;

		int m_iFirstMessage;
		int m_iNextMessage;
		SMessage m_Messages[MESSAGE_QUEUE_SIZE];

		int m_cySmoothScroll;
	};

class CArmorDisplay
	{
	public:
		CArmorDisplay (void);
		~CArmorDisplay (void);

		void CleanUp (void);
		inline const RECT &GetRect (void) { return m_rcRect; }
		ALERROR Init (CPlayerShipController *pPlayer, const RECT &rcRect);
		void Paint (CG32bitImage &Dest);
		inline void SetFontTable (const SFontTable *pFonts) { m_pFonts = pFonts; }
		void SetSelection (int iSelection);
		void Update (void);

	private:
		struct STextPaint
			{
			CString sText;
			int x;
			int y;
			const CG16bitFont *pFont;
			CG32bitPixel rgbColor;
			};


		CUniverse *m_pUniverse;
		CPlayerShipController *m_pPlayer;

		RECT m_rcRect;
		CG32bitImage m_Buffer;
		const SFontTable *m_pFonts;
		int m_iSelection;
		TArray<STextPaint> m_Text;

		DWORD m_dwCachedShipID;				//	Cached painters for this ship ID.
		IEffectPainter *m_pShieldPainter;	//	Caches shield painter
	};

#define MAX_SCORES			100

class CHighScoreList
	{
	public:
		CHighScoreList (void);

		ALERROR Load (const CString &sFilename);
		ALERROR Save (const CString &sFilename);

		int AddEntry (const CGameRecord &NewEntry);

		inline int GetCount (void) const { return m_iCount; }
		inline const CGameRecord &GetEntry (int iIndex) const { return m_List[iIndex]; }

	private:
		bool m_bModified;
		int m_iCount;
		CGameRecord m_List[MAX_SCORES];
	};

class CTextCrawlDisplay
	{
	public:
		CTextCrawlDisplay (void);
		~CTextCrawlDisplay (void);

		void CleanUp (void);
		inline const RECT &GetRect (void) { return m_rcRect; }
		ALERROR Init (const RECT &rcRect, const CString &sText);
		void Paint (CG32bitImage &Dest);
		inline void SetFont (const CG16bitFont *pFont) { m_pFont = pFont; }
		void Update (void);

	private:
		RECT m_rcRect;
		RECT m_rcText;
		int m_yPos;
		const CG16bitFont *m_pFont;
		TArray<CString> m_EpilogLines;
	};

#define MAX_MENU_ITEMS				100

class CMenuData
	{
	public:
		enum Flags
			{
			FLAG_GRAYED =			0x00000001,
			FLAG_SORT_BY_KEY =		0x00000002,
			};

		CMenuData (void);

		inline void AddMenuItem (const CString &sKey,
						  const CString &sLabel,
						  DWORD dwFlags,
						  DWORD dwData,
						  DWORD dwData2 = 0) { AddMenuItem(sKey, sLabel, NULL, NULL_STR, dwFlags, dwData, dwData2); }
		void AddMenuItem (const CString &sKey,
						  const CString &sLabel,
						  const CObjectImageArray *pImage,
						  const CString &sExtra,
						  DWORD dwFlags,
						  DWORD dwData,
						  DWORD dwData2 = 0);
		inline void SetTitle (const CString &sTitle) { m_sTitle = sTitle; }

		int FindItemByKey (const CString &sKey);
		bool FindItemData (const CString &sKey, DWORD *retdwData = NULL, DWORD *retdwData2 = NULL);
		inline int GetCount (void) const { return m_iCount; }
		inline DWORD GetItemData (int iIndex) const { return m_List[iIndex].dwData; }
		inline DWORD GetItemData2 (int iIndex) const { return m_List[iIndex].dwData2; }
		inline const CObjectImageArray *GetItemImage (int iIndex) const { return m_List[iIndex].pImage; }
		inline const CString &GetItemExtra (int iIndex) const { return m_List[iIndex].sExtra; }
		inline DWORD GetItemFlags (int iIndex) const { return m_List[iIndex].dwFlags; }
		inline const CString &GetItemKey (int iIndex) const { return m_List[iIndex].sKey; }
		inline const CString &GetItemLabel (int iIndex) const { return m_List[iIndex].sLabel; }
		inline const CString &GetTitle (void) { return m_sTitle; }
		inline void RemoveAll (void) { m_iCount = 0; }

	private:
		struct Entry
			{
			CString sKey;
			CString sLabel;
			const CObjectImageArray *pImage;
			CString sExtra;
			DWORD dwFlags;

			DWORD dwData;
			DWORD dwData2;
			};

		CString m_sTitle;
		int m_iCount;
		Entry m_List[MAX_MENU_ITEMS];
	};

class CMenuDisplay
	{
	public:
		CMenuDisplay (void);
		~CMenuDisplay (void);

		void CleanUp (void);
		inline const RECT &GetRect (void) { return m_rcRect; }
		ALERROR Init (CMenuData *pMenu, const RECT &rcRect);
		inline void Invalidate (void) { m_bInvalid = true; }
		void Paint (CG32bitImage &Dest);
		inline void SetFontTable (const SFontTable *pFonts) { m_pFonts = pFonts; }

		static CString GetHotKeyFromOrdinal (int *ioOrdinal, const TSortMap<CString, bool> &Exclude);

	private:
		void ComputeMenuRect (RECT *retrcRect);
		void Update (void);

		CMenuData *m_pMenu;	

		RECT m_rcRect;
		CG32bitImage m_Buffer;
		const SFontTable *m_pFonts;
		bool m_bInvalid;
	};

class CPickerDisplay
	{
	public:
		CPickerDisplay (void);
		~CPickerDisplay (void);

		void CleanUp (void);
		inline const RECT &GetRect (void) { return m_rcRect; }
		int GetSelection (void);
		ALERROR Init (CMenuData *pMenu, const RECT &rcRect);
		inline void Invalidate (void) { m_bInvalid = true; }
		void Paint (CG32bitImage &Dest);
		inline void ResetSelection (void) { m_iSelection = 0; }
		void SelectNext (void);
		void SelectPrev (void);
		inline void SetFontTable (const SFontTable *pFonts) { m_pFonts = pFonts; }
		inline void SetHelpText (const CString &sText) { m_sHelpText = sText; }

	private:
		void PaintSelection (CG32bitImage &Dest, int x, int y);
		void Update (void);

		CMenuData *m_pMenu;

		RECT m_rcRect;
		CG32bitImage m_Buffer;
		const SFontTable *m_pFonts;
		int m_iSelection;
		bool m_bInvalid;
		CString m_sHelpText;
	};

#define MAX_BUTTONS					10

class CButtonBarData
	{
	public:
		CButtonBarData (void);
		~CButtonBarData (void);

		enum AlignmentTypes
			{
			alignCenter,
			alignLeft,
			alignRight,
			};

		enum StyleTypes
			{
			styleLarge,							//	128x64 button
			styleMedium,						//	64x64 button
			};

		void AddButton (DWORD dwCmdID,
						const CString &sLabel,
						const CString &sDescription,
						const CString &sKey,
						int iImageIndex,
						AlignmentTypes iAlign,
						StyleTypes iStyle = styleLarge);
		void CleanUp (void);
		int FindButton (const CString &sKey);
		inline AlignmentTypes GetAlignment (int iIndex) { return m_Buttons[iIndex].iAlign; }
		inline const RECT &GetButtonRect (int iIndex) { return m_Buttons[iIndex].rcRect; }
		inline DWORD GetCmdID (int iIndex) { return m_Buttons[iIndex].dwCmdID; }
		inline int GetCount (void) { return m_iCount; }
		inline const CString &GetDescription (int iIndex) { return m_Buttons[iIndex].sDescription; }
		inline const CG32bitImage &GetImage (void) { return m_Images; }
		void GetImageSize (int iIndex, RECT *retrcRect);
		inline int GetImageIndex (int iIndex) { return m_Buttons[iIndex].iImageIndex; }
		inline const CString &GetKey (int iIndex) { return m_Buttons[iIndex].sKey; }
		inline const CString &GetLabel (int iIndex) { return m_Buttons[iIndex].sLabel; }
		inline StyleTypes GetStyle (int iIndex) { return m_Buttons[iIndex].iStyle; }
		inline bool GetVisible (int iIndex) { return ((m_Buttons[iIndex].dwFlags & FLAG_VISIBLE) ? true : false); }
		int HitTest (int x, int y);
		ALERROR Init (void);

		inline void SetButtonRect (int iIndex, const RECT &rcRect) { m_Buttons[iIndex].rcRect = rcRect; }
		inline void SetDescription (int iIndex, const CString &sDescription) { m_Buttons[iIndex].sDescription = sDescription; }
		inline void SetLabel (int iIndex, const CString &sLabel) { m_Buttons[iIndex].sLabel = sLabel; }
		inline void SetVisible (int iIndex, bool bVisible) { if (bVisible) m_Buttons[iIndex].dwFlags |= FLAG_VISIBLE; else m_Buttons[iIndex].dwFlags &= ~FLAG_VISIBLE; }

	private:
		enum Flags
			{
			FLAG_VISIBLE = 0x00000001,
			};

		struct Entry
			{
			DWORD dwCmdID;
			CString sLabel;
			CString sDescription;
			CString sKey;
			int iImageIndex;
			AlignmentTypes iAlign;
			StyleTypes iStyle;
			DWORD dwFlags;

			RECT rcRect;
			};

		int m_iCount;
		Entry m_Buttons[MAX_BUTTONS];
		CG32bitImage m_Images;
	};

class CButtonBarDisplay
	{
	public:
		CButtonBarDisplay (void);
		~CButtonBarDisplay (void);

		void CleanUp (void);
		inline const RECT &GetRect (void) { return m_rcRect; }
		ALERROR Init (CTranscendenceWnd *pTrans, 
					  CButtonBarData *pData, 
					  const RECT &rcRect);
		inline void Invalidate (void) { }
		bool OnChar (char chChar);
		bool OnKeyDown (int iVirtKey);
		bool OnLButtonDoubleClick (int x, int y);
		bool OnLButtonDown (int x, int y);
		void OnMouseMove (int x, int y);
		void Paint (CG32bitImage &Dest);
		inline void SetFontTable (const SFontTable *pFonts) { m_pFonts = pFonts; }
		void Update (void);

	private:
		void ComputeButtonRects (void);
		void ComputeButtonSize (int iIndex, int *retcxWidth, int *retcyHeight);
		int ComputeButtonWidth (CButtonBarData::AlignmentTypes iAlignment);
		void ComputeButtonRects (CButtonBarData::AlignmentTypes iAlignment, int x, int y);
		void GetImageRect (int iIndex, bool bSelected, RECT *retrcRect);
		int FindButtonAtPoint (const POINT &pt);

		CTranscendenceWnd *m_pTrans;
		CButtonBarData *m_pButtons;
		RECT m_rcRect;
		const SFontTable *m_pFonts;

		int m_iSelected;
	};

class CDeviceCounterDisplay
	{
	public:
		CDeviceCounterDisplay (void);
		~CDeviceCounterDisplay (void);

		void CleanUp (void);
		inline const RECT &GetRect (void) { return m_rcRect; }
		ALERROR Init (CPlayerShipController *pPlayer, const RECT &rcRect);
		inline void Invalidate (void) { m_bInvalid = true; }
		void Paint (CG32bitImage &Dest);
		inline void SetFontTable (const SFontTable *pFonts) { m_pFonts = pFonts; }
		void Update (void);

	private:
		void PaintDevice (CInstalledDevice *pDevice, int x);

		CPlayerShipController *m_pPlayer;

		RECT m_rcRect;
		RECT m_rcBuffer;
		CG32bitImage m_Buffer;
		const SFontTable *m_pFonts;
		bool m_bInvalid;
		bool m_bEmpty;
	};

class CPlayerDisplay
	{
	public:
		CPlayerDisplay (void);
		~CPlayerDisplay (void);

		void CleanUp (void);
		inline const RECT &GetRect (void) { return m_rcRect; }
		ALERROR Init (CTranscendenceWnd *pTrans, CCloudService *pService, const RECT &rcRect, bool bShowDebugOption);
		inline void Invalidate (void) { m_bInvalid = true; }
		bool OnChar (char chChar);
		bool OnLButtonDoubleClick (int x, int y);
		bool OnLButtonDown (int x, int y);
		void OnMouseMove (int x, int y);
		bool OnKeyDown (int iVirtKey);
		void Paint (CG32bitImage &Dest);
		void Update (void);

	private:
		struct SPlayerShip
			{
			CShipClass *pClass;
			CString sName;
			};

		void PaintBuffer (void);
		void PaintOption (int iPos, const RECT &rcIcon, const CString &sLabel);

		CTranscendenceWnd *m_pTrans;
		CCloudService *m_pService;
		bool m_bMale;
		bool m_bMusicOn;
		bool m_bShowDebugOption;

		CString m_sEditBuffer;
		bool m_bEditing;
		bool m_bClearAll;
		bool m_bNameHover;

		bool m_bInvalid;
		RECT m_rcRect;
		RECT m_rcIcon;
		RECT m_rcName;
		RECT m_rcGenderOption;
		RECT m_rcMusicOption;
		RECT m_rcDebugModeOption;
		CG32bitImage m_IconImage;
		CG32bitImage m_Buffer;
		const SFontTable *m_pFonts;
	};

class CLRSDisplay
	{
	public:
		CLRSDisplay (void) :
				m_rgbBackground(CG32bitPixel(0, 0, 0)),
				m_pBackground(NULL),
				m_pSnow(NULL)
			{ }

		void CleanUp (void);
		inline const RECT &GetRect (void) { return m_rcRect; }
		ALERROR Init (CPlayerShipController *pPlayer, const RECT &rcRect);
		void Paint (CG32bitImage &Dest);
		inline void SetBackgroundColor (CG32bitPixel rgbColor) { m_rgbBackground = rgbColor; }
		inline void SetBackgroundImage (const CG32bitImage *pImage) { m_pBackground = pImage; }
		inline void SetSnowImage (const CG32bitImage *pSnow) { m_pSnow = pSnow; }
		void Update (void);

	private:
		CPlayerShipController *m_pPlayer;

		RECT m_rcRect;
		int m_iDiameter;					//	Diameter of scanner in pixels
		CG32bitImage m_Buffer;
		CG8bitImage m_Mask;
		CG32bitPixel m_rgbBackground;
		const CG32bitImage *m_pBackground;
		const CG32bitImage *m_pSnow;
	};

class CReactorDisplay
	{
	public:
		CReactorDisplay (void);
		~CReactorDisplay (void);

		void CleanUp (void);
		inline const RECT &GetRect (void) { return m_rcRect; }
		ALERROR Init (CPlayerShipController *pPlayer, const RECT &rcRect);
		void Paint (CG32bitImage &Dest);
		inline void SetFontTable (const SFontTable *pFonts) { m_pFonts = pFonts; }
		void Update (void);

	private:
		CPlayerShipController *m_pPlayer;

		RECT m_rcRect;
		CG32bitImage m_Buffer;
		const SFontTable *m_pFonts;
		int m_iTickCount;
		int m_iOverloading;
	};

class CCommandLineDisplay
	{
	public:
		CCommandLineDisplay (void);
		~CCommandLineDisplay (void);

		void CleanUp (void);
		inline void ClearInput (void) { m_sInput = NULL_STR; m_bInvalid = true; }
		inline const CString &GetInput (void) { return m_sInput; }
		inline int GetOutputLineCount (void) { return GetOutputCount(); }
		inline const RECT &GetRect (void) { return m_rcRect; }
		ALERROR Init (CTranscendenceWnd *pTrans, const RECT &rcRect);
		void Input (const CString &sInput);
		void InputBackspace (void);
		void InputEnter (void);
		void InputLastLine (void);
		void OnKeyDown (int iVirtKey, DWORD dwKeyState);
		void Output (const CString &sOutput, CG32bitPixel rgbColor = CG32bitPixel::Null());
		void Paint (CG32bitImage &Dest);
		inline void SetFontTable (const SFontTable *pFonts) { m_pFonts = pFonts; }

	private:
		enum Constants
			{
			MAX_LINES = 80,
			};

		void AppendOutput (const CString &sLine, CG32bitPixel rgbColor);
		const CString &GetOutput (int iLine);
		CG32bitPixel GetOutputColor (int iLine);
		int GetOutputCount (void);
		void Update (void);

		CTranscendenceWnd *m_pTrans;
		const SFontTable *m_pFonts;
		RECT m_rcRect;

		CString m_Output[MAX_LINES + 1];
		CG32bitPixel m_OutputColor[MAX_LINES + 1];
		int m_iOutputStart;
		int m_iOutputEnd;
		CString m_sInput;
		CString m_sLastLine;

		CG32bitImage m_Buffer;
		bool m_bInvalid;
		int m_iCounter;
		RECT m_rcCursor;
	};

class CTargetDisplay
	{
	public:
		CTargetDisplay (void);
		~CTargetDisplay (void);

		void CleanUp (void);
		inline const RECT &GetRect (void) { return m_rcRect; }
		ALERROR Init (CPlayerShipController *pPlayer, const RECT &rcRect);
		inline void Invalidate (void) { m_bInvalid = true; }
		void Paint (CG32bitImage &Dest);
		inline void SetFontTable (const SFontTable *pFonts) { m_pFonts = pFonts; }

	private:
		void PaintDeviceStatus (CShip *pShip, DeviceNames iDev, int x, int y);
		void Update (void);

		CPlayerShipController *m_pPlayer;

		RECT m_rcRect;
		CG32bitImage m_Buffer;
		CG32bitImage *m_pBackground;
		const SFontTable *m_pFonts;
		bool m_bInvalid;
	};

class CUIResources
	{
	public:
		CUIResources (void) : 
				m_pFonts(NULL),
				m_bOptionShowDamageAdjAsHP(false) { }

		ALERROR Init (SFontTable *pFonts);

		void CreateTitleAnimation (int x, int y, int iDuration, IAnimatron **retpAni);
		void CreateLargeCredit (const CString &sCredit, const CString &sName, int x, int y, int iDuration, IAnimatron **retpAni);
		void CreateMediumCredit (const CString &sCredit, TArray<CString> &Names, int x, int y, int iDuration, IAnimatron **retpAni);

	private:
		SFontTable *m_pFonts;						//	Font table

		bool m_bOptionShowDamageAdjAsHP;			//	If TRUE, show damage adj as HP instead of %
	};

#define DEBUG_LINES_COUNT					51

#define CMD_CONTINUE_OLD_GAME				120
#define CMD_START_NEW_GAME					121
#define CMD_QUIT_GAME						122

#define CMD_SELECT_ADVENTURE				201
#define CMD_SELECT_ADVENTURE_CANCEL			202
#define CMD_NEXT_ADVENTURE_OLD				203
#define CMD_PREV_ADVENTURE_OLD				204

#define CMD_LOAD_ADVENTURE					301

#define MAP_SCALE_COUNT						4

class CTranscendenceWnd : public CUniverse::IHost, public IAniCommand
	{
	public:
		CTranscendenceWnd (HWND hWnd, CTranscendenceController *pTC);

		void Animate (CG32bitImage &TheScreen, CGameSession *pSession, bool bTopMost);

		void Autopilot (bool bTurnOn);
		void CleanUpPlayerShip (void);
		void ClearMessage (void);
		inline CString ComposePlayerNameString (const CString &sString, ICCItem *pArgs = NULL);
		inline void DamageFlash (void) { m_iDamageFlash = Min(2, m_iDamageFlash + 2); }
		void DebugConsoleOutput (const CString &sOutput);
		void DisplayMessage (CString sMessage);
		void DoCommand (DWORD dwCmd);
		inline const CString &GetCrashInfo (void) { return m_sCrashInfo; }
		inline bool GetDebugGame (void);
		inline const SFontTable &GetFonts (void) { return m_Fonts; }
		inline CHighScoreList *GetHighScoreListOld (void);
		inline CTranscendenceModel &GetModel (void);
		void GetMousePos (POINT *retpt);
		inline CPlayerShipController *GetPlayer (void);
		inline CReanimator &GetReanimator (void) { return m_Reanimator; }
		inline const CString &GetRedirectMessage (void) { return m_sRedirectMessage; }
		inline CGameSettings &GetSettings (void);
		inline const CUIResources &GetUIRes (void) { return m_UIRes; }
		void HideDockScreen (void);
		inline bool InAutopilot (void) { return m_bAutopilot; }
		inline bool InDockState (void) { return m_State == gsDocked; }
		inline bool InGameState (void) { return m_State == gsInGame; }
		inline bool InMap (void) { return m_bShowingMap; }
		inline bool InMenu (void) { return (m_CurrentMenu != menuNone || m_CurrentPicker != pickNone); }
		void OnObjDestroyed (const SDestroyCtx &Ctx);
		void OnStargateSystemReady (void);
		void PlayerDestroyed (const CString &sText, bool bResurrectionPending);
		void PlayerEndGame (void);
		void PlayerEnteredGate (CSystem *pSystem, 
							    CTopologyNode *pDestNode,
							    const CString &sDestEntryPoint);
		void RedirectDisplayMessage (bool bRedirect = true);
		void SelectArmor (int iSeg);
		CXMLElement *SetCurrentLocalScreens (CXMLElement *pLocalScreens);
		void ShowDockScreen (bool bShow = true);
		ALERROR ShowDockScreen (CSpaceObject *pLocation, CXMLElement *pScreenDesc, const CString &sPane);
		inline void ShowSystemMap (bool bShow = true) { m_bShowingMap = bShow; }
		ALERROR SwitchDockScreen (CXMLElement *pScreenDesc, const CString &sPane);
		inline void UpdateArmorDisplay (void) { m_ArmorDisplay.Update(); }
		inline void UpdateDeviceCounterDisplay (void) { m_DeviceDisplay.Invalidate(); }
		inline void UpdateWeaponStatus (void) { m_TargetDisplay.Invalidate(); }

		//	CUniverse::IHost
		virtual void ConsoleOutput (const CString &sLine);
		virtual IPlayerController *CreatePlayerController (void);
		virtual IShipController *CreateShipController (const CString &sController);
		virtual void DebugOutput (const CString &sLine);
		virtual void GameOutput (const CString &sLine);
		virtual const CG16bitFont *GetFont (const CString &sFont);

		//	IAniCommand
		virtual void OnAniCommand (const CString &sID, const CString &sEvent, const CString &sCmd, DWORD dwData);

	private:
		enum GameState
			{
			gsNone,
			gsIntro,
			gsInGame,
			gsDocked,
			gsEnteringStargate,
			gsWaitingForSystem,
			gsLeavingStargate,
			gsDestroyed,
			gsEndGame,
			};

		enum EpilogState
			{
			esEpitaph,
			};

		enum MenuTypes
			{
			menuNone,
			menuGame,
			menuSelfDestructConfirm,
			menuCommsTarget,
			menuComms,
			menuCommsSquadron,
			menuInvoke,
			};

		enum PickerTypes
			{
			pickNone,
			pickUsableItem,
			pickPower,
			pickEnableDisableItem,
			};

		struct SPreferences
			{
			bool bMusicOn;
			int iSoundVolume;

			bool bModified;
			};

		void CreateCreditsAnimation (IAnimatron **retpAnimatron);
		void CreateLongCreditsAnimation (int x, int y, int cyHeight, IAnimatron **retpAnimatron);
		void CreateNewsAnimation (CMultiverseNewsEntry *pEntry, IAnimatron **retpAnimatron);
		void CreatePlayerBarAnimation (IAnimatron **retpAni);
		void CreateScoreAnimation (const CGameRecord &Stats, IAnimatron **retpAnimatron);
		void CreateShipDescAnimation (CShip *pShip, IAnimatron **retpAnimatron);
		void CreateTitleAnimation (IAnimatron **retpAnimatron);
		int GetHighScoresPos (void);
		DWORD GetIntroShipClass (void) { return m_dwIntroShipClass; }
		void DestroyIntroShips (void);
		void OnAccountChanged (const CMultiverseModel &Multiverse);
		void OnCommandIntro (const CString &sCmd, void *pData);
		void OnDblClickIntro (int x, int y, DWORD dwFlags);
		void OnLButtonDownIntro (int x, int y, DWORD dwFlags);
		void OnLButtonUpIntro (int x, int y, DWORD dwFlags);
		void OnMouseMoveIntro (int x, int y, DWORD dwFlags);
		void PaintDlgButton (const RECT &rcRect, const CString &sText);
		void SetAccountControls (const CMultiverseModel &Multiverse);
		void SetDebugOption (void);
		void SetHighScoresNext (void);
		void SetHighScoresPos (int iPos);
		void SetHighScoresPrev (void);
		void SetHighScoresScroll (void);
		void SetMusicOption (void);
		ALERROR StartIntro (CIntroSession *pThis);
		void StopIntro (void);

		ALERROR StartGame (void);

		void OnKeyDownHelp (int iVirtKey, DWORD dwKeyData);
		void PaintHelpScreen (void);

		void CleanUpDisplays (void);
		void ClearDebugLines (void);
		void ComputeScreenSize (void);
		ALERROR InitDisplays (void);
		void LoadPreferences (void);
		void PaintDebugLines (void);
		void PaintFrameRate (void);
		void PaintLRS (void);
		void PaintMainScreenBorder (void);
		void PaintMap (void);
		void PaintSnow (CG32bitImage &Dest, int x, int y, int cxWidth, int cyHeight);
		void PaintSRSSnow (void);
		void PaintWeaponStatus (void);
		void ReportCrash (void);
		void ReportCrashEvent (CString *retsMessage);
		ALERROR RestartGame (void);
		void SavePreferences (void);
		void ShowErrorMessage (const CString &sError);

		void DoCommsMenu (int iIndex);
		void DoCommsSquadronMenu (const CString &sName, MessageTypes iOrder, DWORD dwData2);
		void DoGameMenuCommand (DWORD dwCmd);
		void DoSelfDestructConfirmCommand (DWORD dwCmd);
		void DoEnableDisableItemCommand (DWORD dwData);
		void DoInvocation (CPower *pPower);
		void DoUseItemCommand (DWORD dwData);
		DWORD GetCommsStatus (void);
		void HideCommsTargetMenu (CSpaceObject *pExlude = NULL);
		void ShowCommsMenu (CSpaceObject *pObj);
		void ShowCommsSquadronMenu (void);
		void ShowCommsTargetMenu (void);
		void ShowEnableDisablePicker (void);
		void ShowInvokeMenu (void);
		void ShowGameMenu (void);
		void ShowUsePicker (void);

		void OpenGameLog (void);
		void CloseGameLog (void);

		inline void SetGameCreated (bool bValue = true) { m_bGameCreated = bValue; }
		inline bool IsGameCreated (void) { return m_bGameCreated; }

		inline CGameFile &GetGameFile (void);

		LONG WMActivateApp (bool bActivate);
		LONG WMChar (char chChar, DWORD dwKeyData);
		LONG WMClose (void);
		LONG WMCreate (CString *retsError);
		LONG WMDestroy (void);
		LONG WMDisplayChange (int iBitDepth, int cxWidth, int cyHeight);
		LONG WMKeyDown (int iVirtKey, DWORD dwKeyData);
		LONG WMKeyUp (int iVirtKey, DWORD dwKeyData);
		LONG WMLButtonDblClick (int x, int y, DWORD dwFlags);
		LONG WMLButtonDown (int x, int y, DWORD dwFlags);
		LONG WMLButtonUp (int x, int y, DWORD dwFlags);
		LONG WMMouseMove (int x, int y, DWORD dwFlags);
		LONG WMMove (int x, int y);
		LONG WMSize (int cxWidth, int cyHeight, int iSize);

		CTranscendenceController *m_pTC;

		//	General
		CString m_sVersion;
		CString m_sCopyright;

		//	Game
		GameState m_State;					//	Game state
		bool m_bShowingMap;					//	Showing system map
		bool m_bAutopilot;					//	Autopilot is ON
		bool m_bPaused;						//	Game paused
		bool m_bPausedStep;					//	Step one frame
		bool m_bDebugConsole;				//	Showing debug console
		char m_chKeyDown;					//	Processed a WM_KEYDOWN (skip WM_CHAR)
		bool m_bDockKeyDown;				//	Used to de-bounce dock key (so holding down 'D' does not select a dock action).
		bool m_bNextWeaponKey;				//	Next weapon key is down
		bool m_bNextMissileKey;				//	Next missile key is down
		bool m_bPrevWeaponKey;				//	Prev weapon key is down
		bool m_bPrevMissileKey;				//	Prev missile key is down
		int m_iTick;
		CDockScreen m_CurrentDock;
		AGScreen *m_pCurrentScreen;
		MenuTypes m_CurrentMenu;
		CMenuData m_MenuData;
		PickerTypes m_CurrentPicker;

		int m_iCountdown;					//	Miscellaneous timer
		CSpaceObject *m_pMenuObj;			//	Object during menu selection
		bool m_bRedirectDisplayMessage;		//	Redirect display msg to dock screen
		CString m_sRedirectMessage;			//	Redirected message

		//	Loading screen
		CString m_sBackgroundError;

		//	Intro screen
		CIntroSession *m_pIntroSession;
		int m_iIntroCounter;
		DWORD m_dwIntroShipClass;
		int m_iLastShipCreated;
		CSystem *m_pIntroSystem;
		bool m_bSavedGame;
		RECT m_rcIntroTop;
		RECT m_rcIntroMain;
		RECT m_rcIntroBottom;
		CButtonBarData m_ButtonBar;
		CButtonBarDisplay m_ButtonBarDisplay;
		DWORD m_dwCreditsPerformance;
		DWORD m_dwTitlesPerformance;
		DWORD m_dwPlayerBarPerformance;
		CString m_sCommand;
		CString m_sNewsURL;

		//	Crawl screen
		bool m_bGameCreated;

		//	Help screen
		bool m_bHelpInvalid;
		int m_iHelpPage;
		CG32bitImage m_HelpImage;
		GameState m_OldState;

		//	Stargate effect
		CStargateEffectPainter *m_pStargateEffect;

		//	Performance options
		bool m_bTransparencyEffects;

		//	hWnds
		HWND m_hWnd;

		//	DirectX
		CUIResources m_UIRes;
		SFontTable m_Fonts;
		CReanimator m_Reanimator;

		//	Main game structures
		RECT m_rcScreen;					//	Rect of entire screen
		RECT m_rcMainScreen;				//	Rect of main (1024x768) area
		RECT m_rcWindow;					//	Rect of main window in screen coordinates
		RECT m_rcWindowScreen;				//	Rect of screen within window

		CG32bitImage *m_pSRSSnow;			//	SRS snow image

		CArmorDisplay m_ArmorDisplay;		//	Armor display object
		CDeviceCounterDisplay m_DeviceDisplay;	//	Device counter display
		CLRSDisplay m_LRSDisplay;			//	LRS display
		CMessageDisplay m_MessageDisplay;	//	Message display object
		CReactorDisplay m_ReactorDisplay;	//	Reactor status display object
		CTargetDisplay m_TargetDisplay;		//	Targeting computer display
		CMenuDisplay m_MenuDisplay;			//	Menu display
		CPickerDisplay m_PickerDisplay;		//	Picker display
		CCommandLineDisplay m_DebugConsole;	//	CodeChain debugging console
		int m_iDamageFlash;					//	0 = no flash; odd = recover; even = flash;
		Metric m_rMapScale[MAP_SCALE_COUNT];//	Map scale
		int m_iMapScale;					//	Map scale index
		int m_iMapZoomEffect;				//	0 = no zoom effect

		CGameStats m_LastStats;				//	Last game stats

		//	DirectX debugging
		int m_iFrameCount;					//	Number of frames so far
		int m_iFrameTime[FRAME_RATE_COUNT];	//	Last n frame times (in milliseconds)
		int m_iPaintTime[FRAME_RATE_COUNT];	//	Last n paint times (in milliseconds)
		int m_iUpdateTime[FRAME_RATE_COUNT];//	Last n update times (in milliseconds)
		int m_iBltTime[FRAME_RATE_COUNT];	//	Last n BltToScreen times (in milliseconds)
		int m_iStartAnimation;

		//	Debug info
#ifdef DEBUG
		CString m_DebugLines[DEBUG_LINES_COUNT];			//	Debug lines
		int m_iDebugLinesStart;
		int m_iDebugLinesEnd;
#endif
		CString m_sCrashInfo;				//	Saved crash info
		CTextFileLog m_GameLog;

	friend LONG APIENTRY MainWndProc (HWND hWnd, UINT message, UINT wParam, LONG lParam);
	friend class CGameSession;
	friend class CIntroSession;
	friend class CTranscendenceController;
	friend class CTranscendenceModel;
	};

//	Key mapping class ---------------------------------------------------------

const DWORD INVALID_VIRT_KEY = 0xFFFFFFFF;

class CGameKeys
	{
	public:
		enum Keys
			{
			keyError =					-1,
			keyNone =					0,

			keyAutopilot =				1,	//	'A'
			keyEnableDevice =			2,	//	'B'
			keyCommunications =			3,	//	'C'
			keyDock =					4,	//	'D'
			keyTargetNextFriendly =		5,	//	'F'
			keyEnterGate =				6,	//	'G'
			keyInvokePower =			7,	//	'I'
			keyShowMap =				8,	//	'M'
			keyPause =					9,	//	'P' and VK_PAUSE
			keySquadronCommands =		10,	//	'Q'
			keyClearTarget =			11,	//	'R'
			keyShipStatus =				12,	//	'S'
			keyTargetNextEnemy =		13,	//	'T'
			keyUseItem =				14,	//	'U'
			keyNextWeapon =				15,	//	'W'
			keyThrustForward =			16,	//	Up and Down
			keyStop =					17,	//	VK_OEM_PERIOD
			keyRotateLeft =				18,	//	left
			keyRotateRight =			19,	//	right
			keyFireWeapon =				20,	//	Ctrl and Space
			keyFireMissile =			21,	//	Shift and Return
			keyNextMissile =			22,	//	Tab
			keyShowHelp =				23,	//	F1
			keyShowGameStats =			24,	//	F2
			keyReserved3 =				25,	//	F3
			keyReserved4 =				26,	//	F4
			keyReserved5 =				27,	//	F5
			keyReserved6 =				28,	//	F6
			keyVolumeUp =				29,	//	F7
			keyVolumeDown =				30,	//	F8
			keyShowConsole =			31,	//	F9

			keySpecial1 =				32,	//	'1'
			keySpecial2 =				33,	//	'2'
			keySpecial3 =				34,	//	'3'
			keySpecial4 =				35,	//	'4'
			keySpecial5 =				36,	//	'5'
			keySpecial6 =				37,	//	'6'
			keySpecial7 =				38,	//	'7'
			keySpecial8 =				39,	//	'8'
			keySpecial9 =				40,	//	'9'
			keySpecial0 =				41,	//	'0'

			keyEnableAllDevices =		42,
			keyDisableAllDevices =		43,
			keyEnableAllDevicesToggle =	44,
			keyEnableDeviceToggle00 =	45,
			keyEnableDeviceToggle01 =	46,
			keyEnableDeviceToggle02 =	47,
			keyEnableDeviceToggle03 =	48,
			keyEnableDeviceToggle04 =	49,
			keyEnableDeviceToggle05 =	50,
			keyEnableDeviceToggle06 =	51,
			keyEnableDeviceToggle07 =	52,
			keyEnableDeviceToggle08 =	53,
			keyEnableDeviceToggle09 =	54,
			keyEnableDeviceToggle10 =	55,
			keyEnableDeviceToggle11 =	56,
			keyEnableDeviceToggle12 =	57,
			keyEnableDeviceToggle13 =	58,
			keyEnableDeviceToggle14 =	59,
			keyEnableDeviceToggle15 =	60,
			keyEnableDeviceToggle16 =	61,
			keyEnableDeviceToggle17 =	62,
			keyEnableDeviceToggle18 =	63,
			keyEnableDeviceToggle19 =	64,
			keyEnableDeviceToggle20 =	65,
			keyEnableDeviceToggle21 =	66,
			keyEnableDeviceToggle22 =	67,
			keyEnableDeviceToggle23 =	68,
			keyEnableDeviceToggle24 =	69,
			keyEnableDeviceToggle25 =	70,
			keyEnableDeviceToggle26 =	71,
			keyEnableDeviceToggle27 =	72,
			keyEnableDeviceToggle28 =	73,
			keyEnableDeviceToggle29 =	74,
			keyEnableDeviceToggle30 =	75,
			keyEnableDeviceToggle31 =	76,

			keyTargetPrevFriendly =		77,
			keyTargetPrevEnemy =		78,
			keyPrevWeapon =				79,
			keyPrevMissile =			80,
			keyShowGalacticMap =		81,

			keyCount =					82,
			};

		CGameKeys (void);

		CGameKeys::Keys GetGameCommand (const CString &sCmd) const;
		inline Keys GetGameCommand (DWORD dwVirtKey) const { return m_iMap[(dwVirtKey < 256 ? dwVirtKey : 0)]; }
		char GetKeyIfChar (Keys iCommand) const;
		ALERROR ReadFromXML (CXMLElement *pDesc);
		ALERROR WriteAsXML (IWriteStream *pOutput);

		static DWORD GetKey (const CString &sKey);
		static CString GetKeyLabel (DWORD dwVirtKey);

	private:
		Keys m_iMap[256];
	};

//	Game settings class -------------------------------------------------------

class IExtraSettingsHandler
	{
	public:
		virtual ALERROR OnLoadSettings (CXMLElement *pDesc, bool *retbModified) { return NOERROR; }
		virtual ALERROR OnSaveSettings (IWriteStream *pOutput) { return NOERROR; }
	};

class CGameSettings
	{
	public:
		enum Options
			{
			//	Game play options
			playerName,						//	Default player name
			playerGenome,					//	Default player genome ("humanMale" or "humanFemale")
			playerShipClass,				//	Default player ship class
			lastAdventure,					//	Last adventure created

			dockPortIndicator,				//	Options for dock port indicator
			allowInvokeLetterHotKeys,		//	Allow invoke entries to have letter hot keys
			noAutoSave,						//	NOT YET IMPLEMENTED
			noFullCreate,					//	If TRUE, we don't create all systems in the topology
			showManeuverEffects,			//	Shows maneuvering effects
			noMissionCheckpoint,			//	Do not save on mission accept

			//	Installation options
			useTDB,							//	Force use of .TDB
			noAutoUpdate,					//	Do not auto upgrade the game

			//	Video options
			forceDirectX,					//	Use DirectX to access screen directly
			forceNonDirectX,				//	Use GDI to draw to screen
			forceExclusive,					//	Blt directly to primary surface
			forceNonExclusive,				//	Blt using cooperative mode
			windowedMode,					//	Use a window (instead of full screen)
			useBackgroundBlt,				//	Blt in the background
			force1024Res,					//	Force 1024x768 resolution
			force600Res,					//	Force 1024x600 resolution
			graphicsQuality,				//	SFX vs performance
			no3DSystemMap,					//	3D system map projection
			maxResolution,					//	Maximum vertical resolution
			noGPUAcceleration,				//	Do not use GPU textures for blts

			//	Sounds options
			noSound,						//	No sound (either music or sound effects)
			noMusic,						//	TRUE if we should play music
			soundVolume,					//	FX volume level
			musicVolume,					//	Music volume level
			musicPath,						//	Path to music folder (may be NULL_STR)

			//	Debug options
			debugMode,						//	Program in debug mode
			debugGame,						//	Game instance in debug mode
			noDebugLog,						//	Do not write out a debug log
			debugVideo,						//	Write out video information
			noCrashPost,					//	Do not post crash log to Multiverse
			noCollectionDownload,			//	Do not automatically download collection
			debugSoundtrack,				//	Soundtrack debugging UI

			//	Constants
			OPTIONS_COUNT = 36,
			};

		CGameSettings (IExtraSettingsHandler *pExtra = NULL) : m_pExtra(pExtra) { }

		inline const CString &GetAppDataFolder (void) const { return m_sAppData; }
		inline bool GetBoolean (int iOption) const { return m_Options[iOption].bValue; }
		inline void GetDefaultExtensions (DWORD dwAdventure, const TArray<CExtension *> &Available, bool bDebugMode, TArray<DWORD> *retList) const { m_Extensions.GetList(dwAdventure, Available, bDebugMode, retList); }
		inline const TArray<CString> &GetExtensionFolders (void) const { return m_ExtensionFolders; }
		inline const CString &GetInitialSaveFile (void) const { return m_sSaveFile; }
		inline int GetInteger (int iOption) const { return m_Options[iOption].iValue; }
		inline const CGameKeys &GetKeyMap (void) const { return m_KeyMap; }
		inline const CString &GetString (int iOption) const { return m_Options[iOption].sValue; }
		ALERROR Load (const CString &sFilespec, CString *retsError = NULL);
		ALERROR ParseCommandLine (char *pszCmdLine);
		ALERROR Save (const CString &sFilespec);
		inline void SetBoolean (int iOption, bool bValue, bool bModifySettings = true) { SetValueBoolean(iOption, bValue, bModifySettings); if (bModifySettings) m_bModified = true; }
		inline void SetDefaultExtensions (DWORD dwAdventure, const TArray<CExtension *> &Available, bool bDebugMode, const TArray<DWORD> &List) { m_Extensions.SetList(dwAdventure, Available, bDebugMode, List); m_bModified = true; }
		inline void SetInteger (int iOption, int iValue, bool bModifySettings = true) { SetValueInteger(iOption, iValue, bModifySettings); if (bModifySettings) m_bModified = true; }
		inline void SetModified (void) { m_bModified = true; }
		inline void SetSettingsHandler (IExtraSettingsHandler *pExtra) { m_pExtra = pExtra; }
		inline void SetString (int iOption, const CString &sValue, bool bModifySettings = true) { SetValueString(iOption, sValue, bModifySettings); if (bModifySettings) m_bModified = true; }

	private:
		struct SOption
			{
			bool bValue;					//	Current boolean value
			int iValue;						//	Current integer value
			CString sValue;					//	Current string value

			CString sSettingsValue;			//	Value of this option in settings file
											//	NOTE: This may differ from the current value
											//	above if we don't want to save a setting
			};

		void LoadFromRegistry (void);
		void SetValue (int iOption, const CString &sValue, bool bSetSettings = false);
		void SetValueBoolean (int iOption, bool bValue, bool bSetSettings = false);
		void SetValueInteger (int iOption, int iValue, bool bSetSettings = false);
		void SetValueString (int iOption, const CString &sValue, bool bSetSettings = false);

		IExtraSettingsHandler *m_pExtra;	//	Additional settings handler
		SOption m_Options[OPTIONS_COUNT];	//	Options
		CGameKeys m_KeyMap;					//	Key map
		CExtensionListMap m_Extensions;		//	Default extensions

		CString m_sAppData;					//	Location of Settings.xml
		TArray<CString> m_SaveFileFolders;	//	List of folders for save files (1st is default)
		TArray<CString> m_ExtensionFolders;//	List of folders for extensions (may be empty)

		CString m_sSaveFile;				//	Optional save file to open on game start

		bool m_bModified;					//	If TRUE, we need to save out settings
	};

//	Transcendence data model class --------------------------------------------

class CTranscendencePlayer : public IPlayerController
	{
	public:
		CTranscendencePlayer (void);

		inline void SetPlayer (CPlayerShipController *pPlayer) { m_pPlayer = pPlayer; }

		//	IPlayerController interface

		virtual ICCItem *CreateGlobalRef (CCodeChain &CC) { return CC.CreateInteger((int)m_pPlayer); }
		virtual GenomeTypes GetGenome (void) const;
		virtual CString GetName (void) const;
		virtual void OnMessageFromObj (CSpaceObject *pSender, const CString &sMessage);

	private:
		CPlayerShipController *m_pPlayer;
	};

class CTranscendenceModel
	{
	public:
		CTranscendenceModel (CHumanInterface &HI);
		~CTranscendenceModel (void) { }

		ALERROR InitAdventure (const SAdventureSettings &Settings, CString *retsError);
		ALERROR StartNewGame (const CString &sUsername, const SNewGameSettings &NewGame, CString *retsError);
		void StartNewGameAbort (void);
		ALERROR StartNewGameBackground (const SNewGameSettings &NewGame, CString *retsError = NULL);
		ALERROR StartGame (bool bNewGame);

		ALERROR GetGameStats (CGameStats *retStats);

		void OnDockedObjChanged (CSpaceObject *pObj);
		void OnPlayerDestroyed (SDestroyCtx &Ctx, CString *retsEpitaph = NULL);
		void OnPlayerDocked (CSpaceObject *pObj);
		void OnPlayerEnteredGate (CTopologyNode *pDestNode, const CString &sDestEntryPoint, CSpaceObject *pStargate);
		void OnPlayerExitedGate (void);
		void OnPlayerTraveledThroughGate (void);
		inline ICCItem *GetScreenData (const CString &sAttrib) { return m_DockFrames.GetData(sAttrib); }
		ALERROR EndGame (void);
		ALERROR EndGame (const CString &sReason, const CString &sEpitaph, int iScoreChange = 0);
		ALERROR EndGameClose (CString *retsError = NULL);
		ALERROR EndGameDestroyed (bool *retbResurrected = NULL);
		ALERROR EndGameSave (CString *retsError = NULL);
		ALERROR EndGameStargate (void);
		ALERROR EnterScreenSession (CSpaceObject *pLocation, CDesignType *pRoot, const CString &sScreen, const CString &sPane, ICCItem *pData, CString *retsError = NULL);
		void ExitScreenSession (bool bForceUndock = false);
		bool FindScreenRoot (const CString &sScreen, CDesignType **retpRoot, CString *retsScreen = NULL, ICCItem **retpData = NULL);
		inline int GetLastHighScore (void) { return m_iLastHighScore; }
		const SFileVersionInfo &GetProgramVersion (void) const { return m_Version; }
		void GetScreenSession (SDockFrame *retFrame);
		inline bool InScreenSession (void) { return !m_DockFrames.IsEmpty(); }
		bool IsGalacticMapAvailable (CString *retsError = NULL);
		void RecordFinalScore (const CString &sEpitaph, const CString &sEndGameReason, bool bEscaped);
		void RefreshScreenSession (void);
		inline void SetScreenData (const CString &sAttrib, ICCItem *pData) { m_DockFrames.SetData(sAttrib, pData); }
		ALERROR ShowPane (const CString &sPane);
		ALERROR ShowScreen (CDesignType *pRoot, const CString &sScreen, const CString &sPane, ICCItem *pData, CString *retsError, bool bReturn = false, bool bFirstFrame = false);
		void ShowShipScreen (void);
		bool ShowShipScreen (CDesignType *pDefaultScreensRoot, CDesignType *pRoot, const CString &sScreen, const CString &sPane, ICCItem *pData, CString *retsError);
		void UseItem (CItem &Item);

		void AddSaveFileFolder (const CString &sFilespec);
		int AddHighScore (const CGameRecord &Score);
		void CleanUp (void);
		inline const CString &GetCopyright (void) { return m_Version.sCopyright; }
		inline CG32bitImage *GetCrawlImage (void) const { return m_pCrawlImage; }
		inline CSoundType *GetCrawlSoundtrack (void) const { return m_pCrawlSoundtrack; }
		inline const CString &GetCrawlText (void) const { return m_sCrawlText; }
		inline bool GetDebugMode (void) const { return m_bDebugMode; }
		inline CGameFile &GetGameFile (void) { return m_GameFile; }
		inline const CGameRecord &GetGameRecord (void) { return m_GameRecord; }
		inline CHighScoreList &GetHighScoreListOld (void) { return m_HighScoreList; }
		inline CPlayerShipController *GetPlayer (void) { return m_pPlayer; }
		inline const CString &GetProductName (void) { return m_Version.sProductName; }
		inline const TArray<CString> &GetSaveFileFolders (void) const { return m_SaveFileFolders; }
		inline CSFXOptions &GetSFXOptions (void) { return m_Universe.GetSFXOptions(); }
		inline CUniverse &GetUniverse (void) { return m_Universe; }
		inline const CString &GetVersion (void) { return m_Version.sProductVersion; }
		ALERROR Init (const CGameSettings &Settings);
		ALERROR InitBackground (const CGameSettings &Settings, const CString &sCollectionFolder, const TArray<CString> &ExtensionFolders, CString *retsError = NULL);
		ALERROR LoadGame (const CString &sSignedInUsername, const CString &sFilespec, CString *retsError);
		inline void ResetPlayer (void) { m_pPlayer = NULL; }
		inline void SetCrawlImage (DWORD dwImage) { m_pCrawlImage = g_pUniverse->GetLibraryBitmap(dwImage); }
		inline void SetCrawlSoundtrack (DWORD dwTrack) { m_pCrawlSoundtrack = g_pUniverse->FindSoundType(dwTrack); }
		inline void SetCrawlText (const CString &sText) { m_sCrawlText = sText; }
		void SetDebugMode (bool bDebugMode = true);
		ALERROR SaveHighScoreList (CString *retsError = NULL);
		ALERROR SaveGame (DWORD dwFlags, CString *retsError = NULL);

	private:
		enum States
			{
			stateUnknown,

			stateCreatingNewGame,					//	Creating a new game in the background
			stateInGame,							//	In normal play
			statePlayerInGateOldSystem,				//	Player removed from the system and in a stargate
			statePlayerInGateNewSystem,				//	Player not yet in the new system
			statePlayerInResurrect,					//	Player removed from the system, but about to be resurrected
			statePlayerDestroyed,					//	Player ship is no longer in universe,
													//		but we're still running
			statePlayerInEndGame,					//	Player hit end game condition
			stateGameOver,							//	GameFile closed
			};

		CString CalcEpitaph (SDestroyCtx &Ctx);
		void CalcStartingPos (CShipClass *pStartingShip, DWORD *retdwMap, CString *retsNodeID, CString *retsPos);
		ALERROR CreateAllSystems (const CString &sStartNode, CSystem **retpStartingSystem, CString *retsError);
		void GenerateGameStats (CGameStats *retStats, bool bGameOver = false);
		inline CString GetSaveFilePath (void) const { return (m_SaveFileFolders.GetCount() == 0 ? NULL_STR : m_SaveFileFolders[0]); }
		ALERROR LoadGameStats (const CString &sFilespec, CGameStats *retStats);
		ALERROR LoadHighScoreList (CString *retsError = NULL);
		ALERROR LoadUniverse (const CString &sCollectionFolder, const TArray<CString> &ExtensionFolders, CString *retsError = NULL);
		void MarkGateFollowers (CSystem *pSystem);
		ALERROR SaveGameStats (const CGameStats &Stats, bool bGameOver = false, bool bEndGame = false);
		void TransferGateFollowers (CSystem *pOldSystem, CSystem *pSystem, CSpaceObject *pStargate);

		CHumanInterface &m_HI;
		States m_iState;

		SFileVersionInfo m_Version;
		TArray<CString> m_SaveFileFolders;			//	List of all save file folders (first is the default)
		bool m_bDebugMode;							//	Game in debug mode (or next game should be in debug mode)
		bool m_bForceTDB;							//	Use TDB even if XML files exist
		bool m_bNoSound;							//	No sound
		bool m_bNoMissionCheckpoint;				//	Do not save game on mission accept

		CGameFile m_GameFile;
		CUniverse m_Universe;
		CPlayerShipController *m_pPlayer;

		CGameRecord m_GameRecord;					//	Most recent game record
		CGameStats m_GameStats;						//	Most recent game stats
		CHighScoreList m_HighScoreList;
		int m_iLastHighScore;						//	Index to last high-score

		//	Docking state
		CSpaceObject *m_pDock;						//	Object we are docked with (NULL if we're not docked with anything)
		CDesignType *m_pDefaultScreensRoot;			//	Default root to look for local screens
		CDockScreenStack m_DockFrames;				//	Stack of dock screens
		TArray<CXMLElement *> m_ScreensInited;		//	List of screens that have called OnInit this session

		//	Temporaries
		CDesignType *m_pResurrectType;				//	DesignType that will handle resurrect (or NULL)
		CString m_sEndGameReason;					//	Reason for end game
		CString m_sEpitaph;							//	Epitaph
		int m_iScoreBonus;							//	Score bonus for completing the game
		CG32bitImage *m_pCrawlImage;				//	For epilogue/prologue
		CSoundType *m_pCrawlSoundtrack;				//	For epilogue/prologue
		CString m_sCrawlText;						//	For epilogue/prologue

		//	Stargate temporaries
		CTopologyNode *m_pDestNode;					//	While player in gate
		CString m_sDestEntryPoint;					//	While player in gate
		CSystem *m_pOldSystem;						//	While player in gate
		CTimedEventList m_TimerEvents;				//	Timer events (temp while we gate)
		TArray<DWORD> m_GateFollowers;				//	ObjID of any wingmen (temp while we gate)
	};

//	CTranscendenceController class --------------------------------------------
//
//	COMMANDS
//
//	uiShowHelp						Show help session

class CTranscendenceController : public IHIController, public IExtraSettingsHandler
	{
	public:
		CTranscendenceController (void) : 
				m_iState(stateNone),
				m_iBackgroundState(stateIdle),
				m_Model(m_HI),
				m_bUpgradeDownloaded(false)
			{ }

		inline const CGameKeys &GetKeyMap (void) const { return m_Settings.GetKeyMap(); }
		inline CTranscendenceModel &GetModel (void) { return m_Model; }
		inline CMultiverseModel &GetMultiverse (void) { return m_Multiverse; }
		inline bool GetOptionBoolean (int iOption) { return m_Settings.GetBoolean(iOption); }
		inline int GetOptionInteger (int iOption) { return m_Settings.GetInteger(iOption); }
		inline CCloudService &GetService (void) { return m_Service; }
		inline CGameSettings &GetSettings (void) { return m_Settings; }
		inline CSoundtrackManager &GetSoundtrack (void) { return m_Soundtrack; }
		void PaintDebugInfo (CG32bitImage &Dest, const RECT &rcScreen);
		void SetOptionBoolean (int iOption, bool bValue);
		void SetOptionInteger (int iOption, int iValue);

		//	IHICommand virtuals
		virtual ALERROR OnBoot (char *pszCommandLine, SHIOptions *retOptions, CString *retsError);
		virtual void OnCleanUp (void);
		virtual bool OnClose (void);
		virtual ALERROR OnCommand (const CString &sCmd, void *pData = NULL);
		virtual ALERROR OnInit (CString *retsError);
		virtual void OnShutdown (EHIShutdownReasons iShutdownCode);

		//	IExtraSettingsHandler
		virtual ALERROR OnLoadSettings (CXMLElement *pDesc, bool *retbModified);
		virtual ALERROR OnSaveSettings (IWriteStream *pOutput);

	private:
		enum States
			{
			stateNone,

			stateLoading,
			stateIntro,
			stateNewGame,
			statePrologue,
			statePrologueDone,
			stateInGame,
			stateEpilogue,
			stateEndGameStats,
			};

		enum BackgroundStates
			{
			stateIdle,

			stateSignedIn,					//	Signed in and idle
			stateLoadingCollection,			//	Waiting for collection to load
			stateDownloadingUpgrade,		//	Downloading an upgrade to the game
			stateDownloadingCatalogEntry,	//	Downloading a TDB file
			stateLoadingNews,				//	Waiting for news to load
			stateDownloadingResource,		//	Downloading a resource file
			};

		void CleanUpUpgrade (void);
		bool CheckAndRunUpgrade (void);
		void DisplayMultiverseStatus (const CString &sStatus, bool bError = false);
		bool InstallUpgrade (CString *retsError);
		bool IsUpgradeReady (void);
		bool RequestCatalogDownload (const TArray<CMultiverseCatalogEntry *> &Downloads);
		bool RequestResourceDownload (const TArray<CMultiverseFileRef> &Downloads);
		ALERROR WriteUpgradeFile (IMediaType *pData, CString *retsError);

		States m_iState;
		BackgroundStates m_iBackgroundState;
		CTranscendenceModel m_Model;

		CCloudService m_Service;
		CMultiverseModel m_Multiverse;
		CSoundtrackManager m_Soundtrack;
		bool m_bUpgradeDownloaded;

		CGameSettings m_Settings;
	};

//	Utility functions

void CopyGalacticMapToClipboard (HWND hWnd, CGalacticMapPainter *pPainter);
void CopyGameStatsToClipboard (HWND hWnd, const CGameStats &GameStats);
void GetCodeChainExtensions (SPrimitiveDefTable *retpTable);
const CG16bitFont &GetFontByName (const SFontTable &Fonts, const CString &sFontName);

//	Animation functions

void CreateGameStatsAnimation (const CGameStats &GameStats, const RECT rcRect, int xCenterLine, IAnimatron **retpAni);

const int GAME_STAT_POSITION_NEXT = -1;
const int GAME_STAT_POSITION_PREV = -2;
const int GAME_STAT_POSITION_NEXT_PAGE = -3;
const int GAME_STAT_POSITION_PREV_PAGE = -4;
const int GAME_STAT_POSITION_HOME = -5;
const int GAME_STAT_POSITION_END = -6;
void SelectGameStat (IAnimatron *pAni, int iStatPos, int cxWidth, int iDuration = durationInfinite);

//	Inlines

inline void CTranscendenceWnd::ClearMessage (void)
	{
	m_MessageDisplay.ClearAll();
	}

inline CString CTranscendenceWnd::ComposePlayerNameString (const CString &sString, ICCItem *pArgs) 
	{
	return ::ComposePlayerNameString(sString, g_pUniverse->GetPlayerName(), g_pUniverse->GetPlayerGenome(), pArgs);
	}

inline bool CTranscendenceWnd::GetDebugGame (void) 
	{
	return m_pTC->GetModel().GetDebugMode(); 
	}

inline CGameFile &CTranscendenceWnd::GetGameFile (void)
	{
	return m_pTC->GetModel().GetGameFile();
	}

inline CHighScoreList *CTranscendenceWnd::GetHighScoreListOld (void)
	{
	return &m_pTC->GetModel().GetHighScoreListOld(); 
	}

inline CTranscendenceModel &CTranscendenceWnd::GetModel (void)
	{
	return m_pTC->GetModel();
	}

inline CPlayerShipController *CTranscendenceWnd::GetPlayer (void)
	{
	return m_pTC->GetModel().GetPlayer();
	}

inline CGameSettings &CTranscendenceWnd::GetSettings (void)
	{
	return m_pTC->GetSettings();
	}

#include "BackgroundTasks.h"
#include "Sessions.h"

#endif