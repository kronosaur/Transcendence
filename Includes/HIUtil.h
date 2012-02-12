//	HIUtil.h
//
//	Human Interface Utilities
//	Copyright (c) 2010 by George Moromisato. All Rights Reserved

#ifndef INCL_HIUTIL
#define INCL_HIUTIL

#ifndef INCL_DIRECTXUTIL
#include "DirectXUtil.h"
#endif

#ifndef INCL_REANIMATOR
#include "Reanimator.h"
#endif

class CHumanInterface;
struct SHIOptions;

//	Command Interface ---------------------------------------------------------
//
//	Some standard commands:
//
//	cmdTaskDone:					Default command sent when a task has completed

class IHICommand
	{
	public:
		IHICommand (void) { }
		virtual ~IHICommand (void) { }

		inline void HICleanUp (void) { OnCleanUp(); }
		inline void HICommand (const CString &sCmd, void *pData = NULL) { OnCommand(sCmd, pData); }
		inline CString HIGetPropertyString (const CString &sProperty) { return OnGetPropertyString(sProperty); }
		ALERROR HIInit (void) { return OnInit(); }
		inline void HISetProperty (const CString &sProperty, const CString &sValue) { OnSetProperty(sProperty, sValue); }

	protected:
		virtual void OnCleanUp (void) { }
		virtual ALERROR OnCommand (const CString &sCmd, void *pData = NULL) { return NOERROR; }
		virtual CString OnGetPropertyString (const CString &sProperty) const { return NULL_STR; }
		virtual ALERROR OnInit (void) { return NOERROR; }
		virtual void OnSetProperty (const CString &sProperty, const CString &sValue) { }
	};

//	Controller Objects --------------------------------------------------------

class IHIController : public IHICommand
	{
	public:
		IHIController (void) { }
		virtual ~IHIController (void) { }

		ALERROR HIBoot (char *pszCommandLine, SHIOptions &Options) { return OnBoot(pszCommandLine, Options); }

	protected:
		virtual ALERROR OnBoot (char *pszCommandLine, SHIOptions &Options) { return NOERROR; }
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
		IHITask (void) { }
		virtual ~IHITask (void) { }

		inline ALERROR HIExecute (ITaskProcessor *pProcessor, CString *retsResult) { return OnExecute(pProcessor, retsResult); }

	protected:
		virtual ALERROR OnExecute (ITaskProcessor *pProcessor, CString *retsResult) { return NOERROR; }
	};

//	Session Objects -----------------------------------------------------------

class IHISession : public IHICommand
	{
	public:
		IHISession (void) { }
		virtual ~IHISession (void) { }

		inline void HIAnimate (CG16bitImage &Screen) { OnAnimate(Screen); }
		inline void HIChar (char chChar, DWORD dwKeyData) { OnChar(chChar, dwKeyData); }
		inline CReanimator &HIGetReanimator (void) { return m_Reanimator; }
		inline void HIInvalidate (const RECT &rcRect);
		inline void HIInvalidate (void);
		inline void HIKeyDown (int iVirtKey, DWORD dwKeyData) { OnKeyDown(iVirtKey, dwKeyData); }
		inline void HIKeyUp (int iVirtKey, DWORD dwKeyData) { OnKeyUp(iVirtKey, dwKeyData); }
		inline void HILButtonDblClick (int x, int y, DWORD dwFlags) { OnLButtonDblClick(x, y, dwFlags); }
		inline void HILButtonDown (int x, int y, DWORD dwFlags) { OnLButtonDown(x, y, dwFlags); }
		inline void HILButtonUp (int x, int y, DWORD dwFlags) { OnLButtonUp(x, y, dwFlags); }
		inline void HIMouseMove (int x, int y, DWORD dwFlags) { OnMouseMove(x, y, dwFlags); }
		inline void HIMove (int x, int y) { OnMove(x, y); }
		void HIPaint (CG16bitImage &Screen);
		inline void HIReportHardCrash (CString *retsMessage) { OnReportHardCrash(retsMessage); }
		inline void HISize (int cxWidth, int cyHeight);
		inline void HIUpdate (void) { OnUpdate(); }

	protected:
		virtual void OnAnimate (CG16bitImage &Screen) { DefaultOnAnimate(Screen); }
		virtual void OnChar (char chChar, DWORD dwKeyData) { }
		virtual void OnKeyDown (int iVirtKey, DWORD dwKeyData) { }
		virtual void OnKeyUp (int iVirtKey, DWORD dwKeyData) { }
		virtual void OnLButtonDblClick (int x, int y, DWORD dwFlags) { }
		virtual void OnLButtonDown (int x, int y, DWORD dwFlags) { }
		virtual void OnLButtonUp (int x, int y, DWORD dwFlags) { }
		virtual void OnMouseMove (int x, int y, DWORD dwFlags) { }
		virtual void OnMove (int x, int y) { }
		virtual void OnReportHardCrash (CString *retsMessage) { }
		virtual void OnPaint (CG16bitImage &Screen, const RECT &rcInvalid) { }
		virtual void OnSize (int cxWidth, int cyHeight) { }
		virtual void OnUpdate (void) { }

		inline IAnimatron *GetPerformance (const CString &sID) { return m_Reanimator.GetPerformance(sID); }
		inline void StartPerformance (const CString &sID, DWORD dwFlags) { m_Reanimator.StartPerformance(sID, dwFlags); }
		void StartPerformance (IAnimatron *pAni, const CString &sID, DWORD dwFlags);

	private:
		void DefaultOnAnimate (CG16bitImage &Screen);

		CReanimator m_Reanimator;
	};

//	Visual Palette ------------------------------------------------------------

class CVisualPalette
	{
	public:
		CVisualPalette (void);

		void AddColor (const CString &sStyle, WORD wColor);
		void AddFont (const CString &sStyle, const CString &sTypeface, int iSize, bool bBold = false, bool bItalic = false, bool bUnderline = false);
		WORD GetColor (const CString &sStyle) const;
		const CG16bitFont &GetFont (const CString &sStyle) const;

	private:
		TMap<CString, WORD> m_ColorTable;
		TMap<CString, CG16bitFont> m_FontTable;
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
		int GetProgress (CString *retsActivity);
		ALERROR Init (HWND hWnd);
		void OnTaskComplete (LPARAM pData);
		void StopAll();

		//	ITaskProcessor
		virtual HANDLE GetStopEvent (void) { return m_hQuitEvent; }
		virtual bool IsStopSignalled (void) { return (::WaitForSingleObject(m_hQuitEvent, 0) != WAIT_TIMEOUT); }
		virtual void SetProgress (const CString &sActivity, int iPercentDone = -1);
		virtual void SetResult (ALERROR error, const CString &sResult);

	private:
		struct STask
			{
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

		inline bool IsInitialized (void) const { return (m_hBackgroundThread != INVALID_HANDLE_VALUE); }
		void PostOnTaskComplete (IHITask *pTask, IHICommand *pListener, const CString &sCmd);

		static DWORD WINAPI Thread (LPVOID pData);

		HWND m_hWnd;

		HANDLE m_hWorkAvailableEvent;
		HANDLE m_hQuitEvent;
		HANDLE m_hBackgroundThread;

		CCriticalSection m_cs;
		TArray<STask> m_Tasks;
		bool m_bExecuting;

		//	Progress
		int m_iPercentDone;
		CString m_sCurActivity;
		ALERROR m_LastError;
		CString m_sLastResult;
	};

//	CHumanInterface -----------------------------------------------------------

const int DEFAULT_SOUND_VOLUME = 7;
const int WM_HI_TASK_COMPLETE = (WM_USER+0x1000);

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

	//	App options
	CString m_sAppTitle;

	//	Display options
	int m_cxScreenDesired;
	int m_cyScreenDesired;
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

	//	Debug options
	bool m_bDebugVideo;
	};

class CHumanInterface
	{
	public:
		CHumanInterface (void);
		~CHumanInterface (void);

		//	Interface
		void AddBackgroundTask (IHITask *pTask, IHICommand *pListener = NULL, const CString &sCmd = NULL_STR);
		void ClosePopupSession (void);
		inline HWND GetHWND (void) { return m_hWnd; }
		inline CBackgroundProcessor &GetBackgroundProcessor (void) { return m_Background; }
		CReanimator &GetReanimator (void);
		inline CG16bitImage &GetScreen (void) { return m_ScreenMgr.GetScreen(); }
		inline CScreenMgr &GetScreenMgr (void) { return m_ScreenMgr; }
		inline CSoundMgr &GetSoundMgr (void) { return m_SoundMgr; }
		inline CVisualPalette &GetVisualPalette (void) { return m_VI; }
		inline void HICommand (const CString &sCmd, void *pData = NULL) { m_pController->HICommand(sCmd, pData); }
		ALERROR OpenPopupSession (IHISession *pSession);
		void ShowHardCrashSession (const CString &sTitle, const CString &sDescription);
		ALERROR ShowSession (IHISession *pSession);
		void Shutdown (void) { ::DestroyWindow(m_hWnd); }

		//	Message Handlers
		void OnAnimate (void);
		inline void OnTaskComplete (LPARAM pData) { m_Background.OnTaskComplete(pData); }
		LONG WMActivateApp (bool bActivate);
		LONG WMChar (char chChar, DWORD dwKeyData);
		ALERROR WMCreate (HMODULE hModule, HWND hWnd, SHIOptions &Options, IHIController *pController);
		void WMDestroy (void);
		LONG WMDisplayChange (int iBitDepth, int cxWidth, int cyHeight);
		LONG WMKeyDown (int iVirtKey, DWORD dwKeyData);
		LONG WMKeyUp (int iVirtKey, DWORD dwKeyData);
		LONG WMLButtonDblClick (int x, int y, DWORD dwFlags);
		LONG WMLButtonDown (int x, int y, DWORD dwFlags);
		LONG WMLButtonUp (int x, int y, DWORD dwFlags);
		LONG WMMouseMove (int x, int y, DWORD dwFlags);
		LONG WMMove (int x, int y);
		LONG WMSize (int cxWidth, int cyHeight, int iSize);

		//	Private, used by other HI classes
		void BeginSessionPaint (CG16bitImage &Screen);
		void BeginSessionUpdate (void);
		void EndSessionPaint (CG16bitImage &Screen);
		void EndSessionUpdate (void);

	private:
		inline void BltScreen (void) { m_ScreenMgr.Blt(); }
		void CleanUp (void);
		inline void FlipScreen (void) { m_ScreenMgr.Flip(); }
		void HardCrash (const CString &sProgramState);
		void PaintFrameRate (void);

		bool m_bDebugVideo;

		IHIController *m_pController;
		IHISession *m_pCurSession;
		TArray<IHISession *> m_SavedSessions;

		HWND m_hWnd;
		CScreenMgr m_ScreenMgr;
		CSoundMgr m_SoundMgr;
		CVisualPalette m_VI;
		CBackgroundProcessor m_Background;
		CFrameRateCounter m_FrameRate;

		char m_chKeyDown;
	};

//	Functions

int HIRunApplication (HINSTANCE hInst, int nCmdShow, LPSTR lpCmdLine, IHIController *pController);

//	Globals 

extern CHumanInterface *g_pHI;
extern HINSTANCE g_hInst;

//	Inlines

inline void IHISession::HIInvalidate (void) { g_pHI->GetScreenMgr().Invalidate(); }
inline void IHISession::HIInvalidate (const RECT &rcRect) { g_pHI->GetScreenMgr().Invalidate(rcRect); }
inline void IHISession::HISize (int cxWidth, int cyHeight) { OnSize(cxWidth, cyHeight); g_pHI->GetScreenMgr().Invalidate(); }

#endif