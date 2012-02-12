//	HI.h
//
//	Transcendence UI Engine
//	Copyright (c) 2010 by George Moromisato. All Rights Reserved.

#ifndef INCL_HI
#define INCL_HI

#ifndef INCL_KERNEL
#include "Kernel.h"
#endif

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
		IHICommand (CHumanInterface &HI) : m_HI(HI) { }
		virtual ~IHICommand (void) { }

		ALERROR HIBoot (char *pszCommandLine, SHIOptions &Options) { return OnBoot(pszCommandLine, Options); }
		inline void HICleanUp (void) { OnCleanUp(); }
		inline void HICommand (const CString &sCmd, void *pData = NULL) { OnCommand(sCmd, pData); }
		ALERROR HIInit (void) { return OnInit(); }

	protected:
		virtual ALERROR OnBoot (char *pszCommandLine, SHIOptions &Options) { return NOERROR; }
		virtual void OnCleanUp (void) { }
		virtual ALERROR OnCommand (const CString &sCmd, void *pData = NULL) { return NOERROR; }
		virtual ALERROR OnInit (void) { return NOERROR; }

		CHumanInterface &m_HI;
	};

//	Background Task Objects ---------------------------------------------------

class IHITask : public IHICommand
	{
	public:
		IHITask (CHumanInterface &HI) : IHICommand(HI) { }
		virtual ~IHITask (void) { }

		inline void HIExecute (void) { OnExecute(); }

	protected:
		virtual void OnExecute (void) { }
	};

//	Session Objects -----------------------------------------------------------

class IHISession : public IHICommand
	{
	public:
		IHISession (CHumanInterface &HI) : IHICommand(HI) { }
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
		virtual void OnReportHardCrash (CString *retsMessage) { }
		virtual void OnPaint (CG16bitImage &Screen, const RECT &rcInvalid) { }
		virtual void OnSize (int cxWidth, int cyHeight) { }
		virtual void OnUpdate (void) { }

		inline IAnimatron *GetPerformance (const CString &sID) { return m_Reanimator.GetPerformance(sID); }
		inline void StartPerformance (const CString &sID, DWORD dwFlags) { m_Reanimator.StartPerformance(sID, dwFlags); }
		void StartPerformance (IAnimatron *pAni, const CString &sID, DWORD dwFlags);
		inline void StopAllPerformances (void) { m_Reanimator.StopAll(); }

	private:
		void DefaultOnAnimate (CG16bitImage &Screen);

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

class CBackgroundProcessor
	{
	public:
		CBackgroundProcessor (void) : m_hBackgroundThread(INVALID_HANDLE_VALUE) { }
		~CBackgroundProcessor (void) { CleanUp(); }

		void AddTask (IHITask *pTask, IHICommand *pListener = NULL, const CString &sCmd = NULL_STR);
		void CleanUp (void);
		ALERROR Init (HWND hWnd);
		void OnTaskComplete (LPARAM pData);
		void StopAll();

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
	};

//	Visual Style Objects ------------------------------------------------------

//	NOTE: The color and font numbers must match the order of the initializing
//	table in CVisualPalette. DO NOT rely on these numbers or save them anywhere.

enum EColorPalette
	{
	colorUnknown =					0,

	colorBackgroundDeep =			1,

	colorLineFrame =				2,

	colorTextHighlight =			3,
	colorTextNormal =				4,
	colorTextFade =					5,

	colorCount =					6,
	};

enum EFontScale
	{
	fontUnknown =					0,

	fontSmall =						1,	//	10 pixels
	fontMedium =					2,	//	13 pixels
	fontMediumBold =				3,	//	13 pixels bold
	fontMediumHeavyBold =			4,	//	14 pixels bold
	fontLarge =						5,	//	16 pixels
	fontLargeBold =					6,	//	16 pixels bold
	fontHeader =					7,	//	19 pixels
	fontHeaderBold =				8,	//	19 pixels bold
	fontSubTitle =					9,	//	26 pixels
	fontSubTitleBold =				10,	//	26 pixels bold
	fontSubTitleHeavyBold =			11,	//	28 pixels bold
	fontTitle =						12,	//	56 pixels

	fontLogoTitle =					13,	//	60 pixels Blue Island

	fontConsoleMediumHeavy =		14,	//	14 pixels

	fontCount =						15,
	};

enum EImageLibrary
	{
	imageUnknown =					0,

	imageDamageTypeIcons =			1,

	imageCount =					2,
	};

class CVisualPalette
	{
	public:
		CVisualPalette (void) { }

		void CleanUp (void) { }
		void DrawSessionBackground (CG16bitImage &Screen, CG16bitImage &Background, RECT *retrcCenter = NULL) const;
		inline WORD GetColor (int iIndex) const { return m_Color[iIndex]; }
		inline const CG16bitFont &GetFont (int iIndex) const { return m_Font[iIndex]; }
		void GetWidescreenRect (CG16bitImage &Screen, RECT *retrcCenter, RECT *retrcFull = NULL) const;
		ALERROR Init (HMODULE hModule, CString *retsError = NULL);

	private:
		WORD m_Color[colorCount];
		CG16bitFont m_Font[fontCount];
		CG16bitImage m_Image[imageCount];
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
			m_bForceDX(false),
			m_bForceNonDX(false),
			m_bForceExclusiveMode(false),
			m_bForceNonExclusiveMode(false),
			m_bForceScreenSize(false),
			m_iSoundVolume(DEFAULT_SOUND_VOLUME),
			m_bDebugVideo(false)
		{ }

	//	Display options
	int m_cxScreenDesired;
	int m_cyScreenDesired;
	int m_iColorDepthDesired;
	bool m_bWindowedMode;

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
		CReanimator &GetReanimator (void);
		inline CG16bitImage &GetScreen (void) { return m_ScreenMgr.GetScreen(); }
		inline CScreenMgr &GetScreenMgr (void) { return m_ScreenMgr; }
		inline CSoundMgr &GetSoundMgr (void) { return m_SoundMgr; }
		inline const CVisualPalette &GetVisuals (void) { return m_Visuals; }
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
		LONG WMClose (void);
		ALERROR WMCreate (HMODULE hModule, HWND hWnd, char *pszCommandLine, IHICommand *pController);
		void WMDestroy (void);
		LONG WMDisplayChange (int iBitDepth, int cxWidth, int cyHeight);
		LONG WMKeyDown (int iVirtKey, DWORD dwKeyData);
		LONG WMKeyUp (int iVirtKey, DWORD dwKeyData);
		LONG WMLButtonDblClick (int x, int y, DWORD dwFlags);
		LONG WMLButtonDown (int x, int y, DWORD dwFlags);
		LONG WMLButtonUp (int x, int y, DWORD dwFlags);
		LONG WMMouseMove (int x, int y, DWORD dwFlags);

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

		IHICommand *m_pController;
		IHISession *m_pCurSession;
		TArray<IHISession *> m_SavedSessions;

		HWND m_hWnd;
		CScreenMgr m_ScreenMgr;
		CSoundMgr m_SoundMgr;
		CBackgroundProcessor m_Background;
		CVisualPalette m_Visuals;
		CFrameRateCounter m_FrameRate;

		char m_chKeyDown;
	};

//	Inlines

inline void IHISession::HIInvalidate (void) { m_HI.GetScreenMgr().Invalidate(); }
inline void IHISession::HIInvalidate (const RECT &rcRect) { m_HI.GetScreenMgr().Invalidate(rcRect); }
inline void IHISession::HISize (int cxWidth, int cyHeight) { OnSize(cxWidth, cyHeight); m_HI.GetScreenMgr().Invalidate(); }

#endif