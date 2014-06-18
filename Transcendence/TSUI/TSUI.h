//	TSUI.h
//
//	Transcendence UI Engine
//	Copyright (c) 2010 by George Moromisato. All Rights Reserved.
//
//	BASIC NOTIFICATIONS
//
//	The following controller commands are used by the system:
//
//	cmdSoundtrackDone: Notification when a music track has finished playing.
//	
//	cmdTaskDone: Default notification when a background task has completed.

#ifndef INCL_TSUI
#define INCL_TSUI

#ifndef INCL_DIRECTXUTIL
#include "DirectXUtil.h"
#endif

#ifndef INCL_REANIMATOR
#include "Reanimator.h"
#endif

#ifndef INCL_TSE
#include "..\TSE\TSE.h"
#endif

class CHumanInterface;
struct SHIOptions;

enum EHIShutdownReasons
	{
	HIShutdownUnknown,

	HIShutdownByUser,
	HIShutdownByHardCrash,

	HIShutdownDestructor,
	};

//	Command Interface ---------------------------------------------------------
//
//	Some standard commands:
//
//	cmdTaskDone:					Default command sent when a task has completed

class IHICommand
	{
	public:
		IHICommand (CHumanInterface &HI) : m_HI(HI) { }
		virtual ~IHICommand (void) { }

		inline void HICleanUp (void) { OnCleanUp(); }
		inline ALERROR HICommand (const CString &sCmd, void *pData = NULL) { return OnCommand(sCmd, pData); }
		inline CString HIGetPropertyString (const CString &sProperty) { return OnGetPropertyString(sProperty); }
		ALERROR HIInit (CString *retsError) { return OnInit(retsError); }
		inline void HISetProperty (const CString &sProperty, const CString &sValue) { OnSetProperty(sProperty, sValue); }

	protected:
		virtual void OnCleanUp (void) { }
		virtual ALERROR OnCommand (const CString &sCmd, void *pData = NULL) { return NOERROR; }
		virtual CString OnGetPropertyString (const CString &sProperty) const { return NULL_STR; }
		virtual ALERROR OnInit (CString *retsError) { return NOERROR; }
		virtual void OnSetProperty (const CString &sProperty, const CString &sValue) { }

		CHumanInterface &m_HI;
	};

//	Controller Objects --------------------------------------------------------

class IHIController : public IHICommand
	{
	public:
		IHIController (CHumanInterface &HI) : IHICommand(HI) { }
		virtual ~IHIController (void) { }

		ALERROR HIBoot (char *pszCommandLine, SHIOptions &Options) { return OnBoot(pszCommandLine, Options); }
		void HIShutdown (EHIShutdownReasons iShutdownCode) { OnShutdown(iShutdownCode); }

	protected:
		virtual ALERROR OnBoot (char *pszCommandLine, SHIOptions &Options);
		virtual void OnShutdown (EHIShutdownReasons iShutdownCode) { }
	};

//	Background Task Objects ---------------------------------------------------

class ITaskProcessor
	{
	public:
		virtual HANDLE GetStopEvent (void) = 0;
		virtual bool IsStopSignalled (void) = 0;
		virtual void SetProgress (const CString &sActivity, int iPercentDone = -1) = 0;
		virtual void SetResult (ALERROR error, const CString &sResult) = 0;
	};

class IHITask : public IHICommand
	{
	public:
		IHITask (CHumanInterface &HI) : IHICommand(HI) { }
		virtual ~IHITask (void) { }

		inline ALERROR GetResult (CString *retsResult) { if (retsResult) *retsResult = m_sResult; return m_Result; }
		inline ALERROR HIExecute (ITaskProcessor *pProcessor, CString *retsResult) { m_Result = OnExecute(pProcessor, &m_sResult); *retsResult = m_sResult; return m_Result; }

	protected:
		virtual ALERROR OnExecute (ITaskProcessor *pProcessor, CString *retsResult) { return NOERROR; }

	private:
		ALERROR m_Result;
		CString m_sResult;
	};

//	Session Objects -----------------------------------------------------------

class IHISession : public IHICommand, public IAniCommand
	{
	public:
		IHISession (CHumanInterface &HI);
		virtual ~IHISession (void) { }

		inline void HIAnimate (CG16bitImage &Screen, bool bTopMost) { OnAnimate(Screen, bTopMost); }
		void HIChar (char chChar, DWORD dwKeyData);
		inline CReanimator &HIGetReanimator (void) { return GetReanimator(); }
		inline void HIInvalidate (const RECT &rcRect);
		inline void HIInvalidate (void);
		void HIKeyDown (int iVirtKey, DWORD dwKeyData);
		void HIKeyUp (int iVirtKey, DWORD dwKeyData);
		void HILButtonDblClick (int x, int y, DWORD dwFlags);
		void HILButtonDown (int x, int y, DWORD dwFlags);
		void HILButtonUp (int x, int y, DWORD dwFlags);
		void HIMouseMove (int x, int y, DWORD dwFlags);
		void HIMouseWheel (int iDelta, int x, int y, DWORD dwFlags);
		inline void HIMove (int x, int y) { OnMove(x, y); }
		void HIPaint (CG16bitImage &Screen);
		inline void HIReportHardCrash (CString *retsMessage) { OnReportHardCrash(retsMessage); }
		inline void HISize (int cxWidth, int cyHeight);
		inline void HIUpdate (bool bTopMost) { OnUpdate(bTopMost); }

		inline bool IsCursorShown (void) const { return !m_bNoCursor; }
		inline bool IsTransparent (void) const { return m_bTransparent; }
		void RegisterPerformanceEvent (IAnimatron *pAni, const CString &sEvent, const CString &sCmd);

		//	Reanimator interface
		inline void AddPerformance (IAnimatron *pAni, const CString &sID) { m_Reanimator.AddPerformance(pAni, sID); }
		inline void DeleteElement (const CString &sID) { m_Reanimator.DeleteElement(sID); }
		inline IAnimatron *GetElement (const CString &sID) { return m_Reanimator.GetElement(sID); }
		inline IAnimatron *GetPerformance (const CString &sID, int *retiFrame = NULL) { return m_Reanimator.GetPerformance(sID, retiFrame); }
		inline bool GetPropertyBool (const CString &sID, const CString &sProp) { return m_Reanimator.GetPropertyBool(sID, sProp); }
		inline WORD GetPropertyColor (const CString &sID, const CString &sProp) { return m_Reanimator.GetPropertyColor(sID, sProp); }
		inline int GetPropertyInteger (const CString &sID, const CString &sProp) { return m_Reanimator.GetPropertyInteger(sID, sProp); }
		inline Metric GetPropertyMetric (const CString &sID, const CString &sProp) { return m_Reanimator.GetPropertyMetric(sID, sProp); }
		inline DWORD GetPropertyOpacity (const CString &sID, const CString &sProp) { return m_Reanimator.GetPropertyOpacity(sID, sProp); }
		inline CString GetPropertyString (const CString &sID, const CString &sProp) { return m_Reanimator.GetPropertyString(sID, sProp); }
		inline CVector GetPropertyVector (const CString &sID, const CString &sProp) { return m_Reanimator.GetPropertyVector(sID, sProp); }
		bool IsElementEnabled (const CString &sID);
		inline void SetInputFocus (const CString &sID) { IAnimatron *pFocus = GetElement(sID); if (pFocus) m_Reanimator.SetInputFocus(pFocus); }
		inline void SetPropertyBool (const CString &sID, const CString &sProp, bool bValue) { m_Reanimator.SetPropertyBool(sID, sProp, bValue); }
		inline void SetPropertyColor (const CString &sID, const CString &sProp, WORD wValue) { m_Reanimator.SetPropertyColor(sID, sProp, wValue); }
		inline void SetPropertyInteger (const CString &sID, const CString &sProp, int iValue) { m_Reanimator.SetPropertyInteger(sID, sProp, iValue); }
		inline void SetPropertyMetric (const CString &sID, const CString &sProp, Metric rValue) { m_Reanimator.SetPropertyMetric(sID, sProp, rValue); }
		inline void SetPropertyOpacity (const CString &sID, const CString &sProp, DWORD dwValue) { m_Reanimator.SetPropertyOpacity(sID, sProp, dwValue); }
		inline void SetPropertyString (const CString &sID, const CString &sProp, const CString &sValue) { m_Reanimator.SetPropertyString(sID, sProp, sValue); }
		inline void SetPropertyVector (const CString &sID, const CString &sProp, const CVector &vValue) { m_Reanimator.SetPropertyVector(sID, sProp, vValue); }
		inline void StartPerformance (const CString &sID, DWORD dwFlags = 0) { m_Reanimator.StartPerformance(sID, dwFlags); }
		void StartPerformance (IAnimatron *pAni, const CString &sID, DWORD dwFlags = 0);
		inline void StopPerformance (const CString &sID) { m_Reanimator.StopPerformance(sID); }

	protected:
		struct SPerformanceEvent
			{
			CString sID;
			CString sEvent;
			};

		virtual CReanimator &GetReanimator (void) { return m_Reanimator; }
		virtual void OnAnimate (CG16bitImage &Screen, bool bTopMost) { DefaultOnAnimate(Screen, bTopMost); }
		virtual void OnChar (char chChar, DWORD dwKeyData) { }
		virtual void OnKeyDown (int iVirtKey, DWORD dwKeyData) { }
		virtual void OnKeyUp (int iVirtKey, DWORD dwKeyData) { }
		virtual void OnLButtonDblClick (int x, int y, DWORD dwFlags) { }
		virtual void OnLButtonDown (int x, int y, DWORD dwFlags) { }
		virtual void OnLButtonUp (int x, int y, DWORD dwFlags) { }
		virtual void OnMouseMove (int x, int y, DWORD dwFlags) { }
		virtual void OnMouseWheel (int iDelta, int x, int y, DWORD dwFlags) { }
		virtual void OnMove (int x, int y) { }
		virtual void OnReportHardCrash (CString *retsMessage) { }
		virtual void OnPaint (CG16bitImage &Screen, const RECT &rcInvalid) { }
		virtual void OnSize (int cxWidth, int cyHeight) { }
		virtual void OnUpdate (bool bTopMost) { }

		bool HandlePageScrollKeyDown (const CString &sScroller, int iVirtKey, DWORD dwKeyData);
		bool HandlePageScrollMouseWheel (const CString &sScroller, int iDelta);
		inline void SetNoCursor (bool bNoCursor = true) { if (bNoCursor != m_bNoCursor) { m_bNoCursor = bNoCursor; } }
		inline void SetTransparent (bool bTransparent = true) { m_bTransparent = bTransparent; }

	private:
		//	IAniCommand virtuals
		virtual void OnAniCommand (const CString &sID, const CString &sEvent, const CString &sCmd, DWORD dwData);

		void DefaultOnAnimate (CG16bitImage &Screen, bool bTopMost);

		bool m_bNoCursor;						//	If TRUE, we hide the cursor when we show the session.
		bool m_bTransparent;					//	If TRUE, session below this one shows through.
		CReanimator m_Reanimator;
	};

//	Keyboard Tracker ----------------------------------------------------------

class CKeyboardTracker
	{
	public:
		inline int GetKeyDownCount (void) { return m_KeysDown.GetCount(); }
		inline void OnKeyDown (int iVirtKey) { m_KeysDown.Insert(iVirtKey); }
		inline void OnKeyUp (int iVirtKey) { int iIndex; if (m_KeysDown.Find(iVirtKey, &iIndex)) m_KeysDown.Delete(iIndex); }

	private:
		TArray<int> m_KeysDown;
	};

//	Background Processor ------------------------------------------------------

class CBackgroundProcessor : public ITaskProcessor
	{
	public:
		CBackgroundProcessor (void) : m_hBackgroundThread(INVALID_HANDLE_VALUE), m_iPercentDone(-1) { }
		~CBackgroundProcessor (void) { CleanUp(); }

		void AddTask (IHITask *pTask, IHICommand *pListener = NULL, const CString &sCmd = NULL_STR);
		void CleanUp (void);
		ALERROR GetLastResult (CString *retsResult);
		int GetProgress (CString *retsActivity = NULL);
		ALERROR Init (HWND hWnd, DWORD dwID);
		void ListenerDestroyed (IHICommand *pListener);
		void OnTaskComplete (LPARAM pData);
		bool RegisterOnAllTasksComplete (IHICommand *pListener, const CString &sCmd = NULL);
		void StopAll();

		//	ITaskProcessor
		virtual HANDLE GetStopEvent (void) { return m_hQuitEvent; }
		virtual bool IsStopSignalled (void) { return (::WaitForSingleObject(m_hQuitEvent, 0) != WAIT_TIMEOUT); }
		virtual void SetProgress (const CString &sActivity, int iPercentDone = -1);
		virtual void SetResult (ALERROR error, const CString &sResult);

	private:
		enum ETaskStatus
			{
			statusReady,
			statusProcessing,
			};

		struct STask
			{
			ETaskStatus iStatus;

			IHITask *pTask;
			IHICommand *pListener;
			CString sCmd;
			};

		struct STaskCompleteMsg
			{
			IHITask *pTask;
			IHICommand *pListener;
			CString sCmd;
			};

		struct SListener
			{
			IHICommand *pListener;
			CString sCmd;
			};

		inline bool IsInitialized (void) const { return (m_hBackgroundThread != INVALID_HANDLE_VALUE); }
		void PostOnAllTasksComplete (void);
		void PostOnTaskComplete (IHITask *pTask);

		static DWORD WINAPI Thread (LPVOID pData);

		HWND m_hWnd;
		DWORD m_dwID;

		HANDLE m_hWorkAvailableEvent;
		HANDLE m_hQuitEvent;
		HANDLE m_hBackgroundThread;

		CCriticalSection m_cs;
		TArray<STask> m_Tasks;
		TArray<SListener> m_GlobalListeners;
		bool m_bExecuting;

		//	Progress
		int m_iPercentDone;
		CString m_sCurActivity;
		ALERROR m_LastError;
		CString m_sLastResult;
	};

//	Timers --------------------------------------------------------------------

class CTimerRegistry
	{
	public:
		CTimerRegistry (void) : m_dwNextID(1) { }

		DWORD AddTimer (HWND hWnd, DWORD dwMilliseconds, IHICommand *pListener, const CString &sCmd, bool bRecurring = true);
		void DeleteTimer (HWND hWnd, DWORD dwID);
		void FireTimer (HWND hWnd, DWORD dwID);
		void ListenerDestroyed (HWND hWnd, IHICommand *pListener);

	private:
		struct SEntry
			{
			DWORD dwID;
			IHICommand *pListener;
			CString sCmd;
			bool bRecurring;
			};

		TArray<SEntry> m_Timers;
		DWORD m_dwNextID;
	};

//	Visual Style Objects ------------------------------------------------------

//	NOTE: The color and font numbers must match the order of the initializing
//	table in CVisualPalette. DO NOT rely on these numbers or save them anywhere.

enum EMetrics
	{
	metricsInputErrorMsgMarginHorz =		64,		//	Distance between anchor object and edge of message box
	metricsInputErrorMsgHeight =			80,		//	Height of input error message box
	metricsInputErrorMsgWidth =				300,	//	Width of input error message box
	};

enum EColorPalette
	{
	colorUnknown =					0,

	colorAreaDeep =					1,
	colorAreaDialog =				2,
	colorAreaDialogHighlight =		3,
	colorAreaDialogInput =			4,
	colorAreaDialogInputFocus =		5,
	colorAreaDialogTitle =			6,
	colorAreaInfoMsg =				7,
	colorAreaWarningMsg =			8,

	colorLineDialogFrame =			9,
	colorLineFrame =				10,

	colorTextHighlight =			11,
	colorTextNormal =				12,
	colorTextFade =					13,
	colorTextAltHighlight =			14,
	colorTextAltNormal =			15,
	colorTextAltFade =				16,
	colorTextWarningMsg =			17,

	colorTextDialogInput =			18,
	colorTextDialogLabel =			19,
	colorTextDialogLink =			20,
	colorTextDialogTitle =			21,
	colorTextDialogWarning =		22,

	colorAreaDockTitle =			23,
	colorTextDockAction =			24,
	colorTextDockActionHotKey =		25,
	colorTextDockText =				26,
	colorTextDockTitle =			27,

	colorCount =					28,
	};

enum EFontScale
	{
	fontUnknown =					0,

	fontSmall =						1,	//	10 pixels
	fontSmallBold =					2,	//	11 pixels bold
	fontMedium =					3,	//	13 pixels
	fontMediumBold =				4,	//	13 pixels bold
	fontMediumHeavyBold =			5,	//	14 pixels bold
	fontLarge =						6,	//	16 pixels
	fontLargeBold =					7,	//	16 pixels bold
	fontHeader =					8,	//	19 pixels
	fontHeaderBold =				9,	//	19 pixels bold
	fontSubTitle =					10,	//	26 pixels
	fontSubTitleBold =				11,	//	26 pixels bold
	fontSubTitleHeavyBold =			12,	//	28 pixels bold
	fontTitle =						13,	//	56 pixels

	fontLogoTitle =					14,	//	60 pixels Blue Island

	fontConsoleMediumHeavy =		15,	//	14 pixels

	fontCount =						16,
	};

enum EImageLibrary
	{
	imageUnknown =					0,

	imageDamageTypeIcons =			1,
	imageProfileIcon =				2,
	imageModExchangeIcon =			3,
	imageMusicIconOn =				4,
	imageMusicIconOff =				5,
	imageCloseIcon =				6,
	imageOKIcon =					7,
	imageLeftIcon =					8,
	imageRightIcon =				9,
	imageUpIcon =					10,
	imageDownIcon =					11,
	imageSmallCloseIcon =			12,
	imageSmallOKIcon =				13,
	imageSmallLeftIcon =			14,
	imageSmallRightIcon =			15,
	imageSmallUpIcon =				16,
	imageSmallDownIcon =			17,
	imageSlotIcon =					18,
	imageSmallSlotIcon =			19,
	imageHumanMale =				20,
	imageHumanFemale =				21,
	imageSmallHumanMale =			22,
	imageSmallHumanFemale =			23,
	imageSmallEditIcon =			24,
	imagePlayIcon =					25,
	imageDebugIcon =				26,

	imageCount =					27,
	};

class CVisualPalette : public IFontTable
	{
	public:
		enum EOptions
			{
			//	CreateButton
			OPTION_BUTTON_DEFAULT =				0x00000001,

			//	CreateCheckbox
			OPTION_CHECKBOX_LARGE_FONT =		0x00000001,

			//	CreateEditControl
			OPTION_EDIT_PASSWORD =				0x00000001,

			//	CreateLink
			OPTION_LINK_MEDIUM_FONT =			0x00000001,
			OPTION_LINK_SUB_TITLE_FONT =		0x00000002,

			//	Session
			OPTION_SESSION_DLG_BACKGROUND =		0x000000001,
			};

		CVisualPalette (void) { }

		//	Initialization and clean up

		void CleanUp (void) { }
		ALERROR Init (HMODULE hModule, CString *retsError = NULL);

		//	Low-level palette elements

		inline WORD GetColor (int iIndex) const { return m_Color[iIndex]; }
		inline const CG16bitFont &GetFont (int iIndex) const { return m_Font[iIndex]; }
		const CG16bitFont &GetFont (const CString &sName, bool *retFound = NULL) const;
		inline const CG16bitImage &GetImage (int iIndex) const { return m_Image[iIndex]; }
		void GetWidescreenRect (CG16bitImage &Screen, RECT *retrcCenter, RECT *retrcFull = NULL) const;

		//	Draw functions

		void DrawDamageTypeIcon (CG16bitImage &Screen, int x, int y, DamageTypes iDamageType) const;
		void DrawSessionBackground (CG16bitImage &Screen, const CG16bitImage &Background, DWORD dwFlags, RECT *retrcCenter = NULL) const;

		//	Reanimator objects

		void CreateButton (CAniSequencer *pContainer,
						   const CString &sID,
						   int x,
						   int y,
						   int cxWidth,
						   int cyHeight,
						   DWORD dwOptions,
						   const CString &sLabel,
						   IAnimatron **retpControl) const;
		void CreateCheckbox (CAniSequencer *pContainer,
							 const CString &sID,
							 int x,
							 int y,
							 int cxWidth,
							 DWORD dwOptions,
							 const CString &sLabel,
							 IAnimatron **retpControl,
							 int *retcyHeight) const;
		void CreateEditControl (CAniSequencer *pContainer, 
								const CString &sID, 
								int x, 
								int y,
								int cxWidth,
								DWORD dwOptions,
								const CString &sLabel, 
								IAnimatron **retpControl, 
								int *retcyHeight) const;
		void CreateHiddenButton (CAniSequencer *pContainer,
								 const CString &sID,
								 int x,
								 int y,
								 int cxWidth,
								 int cyHeight,
								 DWORD dwOptions,
								 IAnimatron **retpControl) const;
		void CreateImageButton (CAniSequencer *pContainer,
								const CString &sID,
								int x,
								int y,
								const CG16bitImage *pImage,
								const CString &sLabel,
								DWORD dwOptions,
								IAnimatron **retpControl) const;
		void CreateImageButtonSmall (CAniSequencer *pContainer,
									 const CString &sID,
									 int x,
									 int y,
									 const CG16bitImage *pImage,
									 DWORD dwOptions,
									 IAnimatron **retpControl) const;
		void CreateLink (CAniSequencer *pContainer,
						 const CString &sID,
						 int x,
						 int y,
						 const CString &sLabel,
						 DWORD dwOptions,
						 IAnimatron **retpControl = NULL,
						 int *retcxWidth = NULL,
						 int *retcyHeight = NULL) const;
		void CreateMessagePane (CAniSequencer *pContainer, 
								const CString &sID, 
								const CString &sTitle,
								const CString &sDesc,
								const RECT &rcRect,
								DWORD dwOptions,
								IAnimatron **retpControl) const;
		void CreateRingAnimation (CAniSequencer *pContainer, int iCount, int iMinRadius, int iInc) const;
		void CreateStdDialog (const RECT &rcRect, const CString &sTitle, IAnimatron **retpDlg, CAniSequencer **retpContainer) const;
		void CreateWaitAnimation (CAniSequencer *pContainer, const CString &sID, const RECT &rcRect, IAnimatron **retpControl) const;

		//	IFontTable methods

		virtual const CG16bitFont *GetFont (const STextFormatDesc &Desc) const { return &GetFont(Desc.sTypeface); }

	private:

		WORD m_Color[colorCount];
		CG16bitFont m_Font[fontCount];
		CG16bitImage m_Image[imageCount];
	};

//	CHumanInterface -----------------------------------------------------------

const int DEFAULT_SOUND_VOLUME =	7;
const int WM_HI_TASK_COMPLETE =		(WM_USER+0x1000);
const int WM_HI_COMMAND =			(WM_USER+0x1001);

struct SHIOptions
	{
	SHIOptions (void) :
			m_cxScreenDesired(1024),
			m_cyScreenDesired(768),
			m_iColorDepthDesired(16),
			m_bWindowedMode(false),
			m_bMultiMonitorMode(false),
			m_bForceDX(false),
			m_bForceNonDX(false),
			m_bForceExclusiveMode(false),
			m_bForceNonExclusiveMode(false),
			m_bForceScreenSize(false),
			m_iSoundVolume(DEFAULT_SOUND_VOLUME),
			m_bDebugVideo(false)
		{ }

	//	Display options
	int m_cxScreenDesired;				//	Ignored if not WindowedMode
	int m_cyScreenDesired;				//	Ignored if not WindowedMode
	int m_iColorDepthDesired;
	bool m_bWindowedMode;
	bool m_bMultiMonitorMode;			//	If TRUE, screen spans all monitors

	bool m_bForceDX;
	bool m_bForceNonDX;
	bool m_bForceExclusiveMode;
	bool m_bForceNonExclusiveMode;
	bool m_bForceScreenSize;

	//	Sound options
	int m_iSoundVolume;
	CString m_sMusicFolder;				//	Path to music folder

	//	Debug options
	bool m_bDebugVideo;
	};

class CHumanInterface
	{
	public:
		enum EFlags
			{
			//	AddBackgroundTask
			FLAG_LOW_PRIORITY =				0x00000001,
			};

		static bool Create (void);
		static void Destroy (void);

		//	Interface

		void ClosePopupSession (void);
		inline HWND GetHWND (void) { return m_hWnd; }
		inline const SHIOptions &GetOptions (void) { return m_Options; }
		CReanimator &GetReanimator (void);
		inline CG16bitImage &GetScreen (void) { return m_ScreenMgr.GetScreen(); }
		inline CScreenMgr &GetScreenMgr (void) { return m_ScreenMgr; }
		inline IHISession *GetSession (void) { return m_pCurSession; }
		inline CSoundMgr &GetSoundMgr (void) { return m_SoundMgr; }
		IHISession *GetTopSession (bool bNonTransparentOnly = true);
		inline const CVisualPalette &GetVisuals (void) { return m_Visuals; }
		inline ALERROR HICommand (const CString &sCmd, void *pData = NULL) { return m_pController->HICommand(sCmd, pData); }
		void HIPostCommand (const CString &sCmd, void *pData = NULL);
		ALERROR InitCodeChainPrimitives (CCodeChain &CC);
		inline bool IsWindowedMode (void) const { return m_Options.m_bWindowedMode; }
		ALERROR OpenPopupSession (IHISession *pSession, CString *retsError = NULL);
		void ShowHardCrashSession (const CString &sTitle, const CString &sDescription);
		ALERROR ShowSession (IHISession *pSession, CString *retsError = NULL);
		void Shutdown (EHIShutdownReasons iCode) { m_iShutdownCode = iCode; ::DestroyWindow(m_hWnd); }
		int SetSoundVolume (int iVolume);

		void AddBackgroundTask (IHITask *pTask, DWORD dwFlags, IHICommand *pListener = NULL, const CString &sCmd = NULL_STR);
		inline CBackgroundProcessor &GetBackgroundProcessor (void) { return m_Background; }
		inline bool RegisterOnAllBackgroundTasksComplete (IHICommand *pListener, const CString &sCmd = NULL) { return m_Background.RegisterOnAllTasksComplete(pListener, sCmd); }

		DWORD AddTimer (DWORD dwMilliseconds, IHICommand *pListener = NULL, const CString &sCmd = NULL_STR, bool bRecurring = true);
		inline void DeleteTimer (DWORD dwID) { m_Timers.DeleteTimer(m_hWnd, dwID); }

		//	Message Handlers
		LONG MCINotifyMode (int iMode);
		void OnAnimate (void);
		void OnPostCommand (LPARAM pData);
		void OnTaskComplete (DWORD dwID, LPARAM pData);
		LONG WMActivateApp (bool bActivate);
		LONG WMChar (char chChar, DWORD dwKeyData);
		ALERROR WMCreate (HMODULE hModule, HWND hWnd, char *pszCommandLine, IHIController *pController);
		void WMDestroy (void);
		LONG WMDisplayChange (int iBitDepth, int cxWidth, int cyHeight);
		LONG WMKeyDown (int iVirtKey, DWORD dwKeyData);
		LONG WMKeyUp (int iVirtKey, DWORD dwKeyData);
		LONG WMLButtonDblClick (int x, int y, DWORD dwFlags);
		LONG WMLButtonDown (int x, int y, DWORD dwFlags);
		LONG WMLButtonUp (int x, int y, DWORD dwFlags);
		LONG WMMouseMove (int x, int y, DWORD dwFlags);
		LONG WMMouseWheel (int iDelta, int x, int y, DWORD dwFlags);
		LONG WMMove (int x, int y);
		LONG WMSize (int cxWidth, int cyHeight, int iSize);
		LONG WMTimer (DWORD dwID);

		//	Private, used by other HI classes
		void BeginSessionPaint (CG16bitImage &Screen);
		void BeginSessionUpdate (void);
		void EndSessionPaint (CG16bitImage &Screen, bool bTopMost);
		void EndSessionUpdate (bool bTopMost);

	private:
		struct SPostCommand
			{
			CString sCmd;
			void *pData;
			};

		CHumanInterface (void);
		~CHumanInterface (void);

		inline void BltScreen (void) { m_ScreenMgr.Blt(); }
		void CalcBackgroundSessions (void);
		void CleanUp (EHIShutdownReasons iShutdownCode);
		inline void FlipScreen (void) { m_ScreenMgr.Flip(); }
		void HardCrash (const CString &sProgramState);
		void PaintFrameRate (void);

		SHIOptions m_Options;
		IHIController *m_pController;
		IHISession *m_pCurSession;
		TArray<IHISession *> m_SavedSessions;
		TArray<IHISession *> m_BackgroundSessions;

		HWND m_hWnd;
		CScreenMgr m_ScreenMgr;
		CBackgroundProcessor m_Background;
		CBackgroundProcessor m_BackgroundLowPriority;
		CTimerRegistry m_Timers;
		CVisualPalette m_Visuals;
		CFrameRateCounter m_FrameRate;

		//	Sound
		CSoundMgr m_SoundMgr;

		char m_chKeyDown;
		EHIShutdownReasons m_iShutdownCode;		//	Valid after call to Shutdown
	};

extern CHumanInterface *g_pHI;

#include "CloudInterface.h"
#include "Painters.h"
#include "Soundtrack.h"
#include "UIHelpers.h"

//	Inlines

inline void IHISession::HIInvalidate (void) { m_HI.GetScreenMgr().Invalidate(); }
inline void IHISession::HIInvalidate (const RECT &rcRect) { m_HI.GetScreenMgr().Invalidate(rcRect); }
inline void IHISession::HISize (int cxWidth, int cyHeight) { OnSize(cxWidth, cyHeight); m_HI.GetScreenMgr().Invalidate(); }

#endif