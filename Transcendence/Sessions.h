//	Sessions.h
//
//	Transcendence session classes
//	Copyright (c) 2010 by Kronosaur Productions, LLC. All Rights Reserved.

class CAdventureIntroSession : public IHISession
	{
	public:
		CAdventureIntroSession (CHumanInterface &HI, CCloudService &Service, const CString &sAdventureName);

		//	IHISession virtuals
		virtual ALERROR OnCommand (const CString &sCmd, void *pData = NULL);
		virtual ALERROR OnInit (CString *retsError);
		virtual void OnKeyDown (int iVirtKey, DWORD dwKeyData);
		virtual void OnPaint (CG32bitImage &Screen, const RECT &rcInvalid);
		virtual void OnReportHardCrash (CString *retsMessage);

	private:
		void CmdDone (void);

		CCloudService &m_Service;
		CString m_sAdventureName;
	};

class CChangePasswordSession : public IHISession
	{
	public:
		CChangePasswordSession (CHumanInterface &HI, CCloudService &Service);

		//	IHISession virtuals
		virtual ALERROR OnCommand (const CString &sCmd, void *pData = NULL);
		virtual ALERROR OnInit (CString *retsError);
		virtual void OnKeyDown (int iVirtKey, DWORD dwKeyData);
		virtual void OnPaint (CG32bitImage &Screen, const RECT &rcInvalid);
		virtual void OnReportHardCrash (CString *retsMessage);

	private:
		void CmdCancel (void);
		void CmdChangeComplete (CChangePasswordTask *pTask);
		void CmdOK (void);
		void CreateDlg (IAnimatron **retpDlg);

		CCloudService &m_Service;
		RECT m_rcInputError;
	};

class CChooseAdventureSession : public IHISession
	{
	public:
		CChooseAdventureSession (CHumanInterface &HI, CCloudService &Service, CGameSettings &Settings);

		//	IHISession virtuals
		virtual ALERROR OnCommand (const CString &sCmd, void *pData = NULL);
		virtual ALERROR OnInit (CString *retsError);
		virtual void OnKeyDown (int iVirtKey, DWORD dwKeyData);
		virtual void OnMouseWheel (int iDelta, int x, int y, DWORD dwFlags);
		virtual void OnPaint (CG32bitImage &Screen, const RECT &rcInvalid);
		virtual void OnReportHardCrash (CString *retsMessage);

	private:
		void CmdCancel (void);
		void CmdNextAdventure (void);
		void CmdOK (void);
		void CmdPrevAdventure (void);
		void CmdSelectExtensions (bool bSelect);
		void CreateAdventureDesc (CExtension *pAdventure);
		bool FindAdventurePos (DWORD dwUNID, int *retiPos);
		void SetBackgroundImage (CExtension *pAdventure);
		void SetAdventureDesc (CExtension *pAdventure);
		void SetAdventureStatus (CExtension *pAdventure, int yPos);
		void SetAdventureTitle (CExtension *pAdventure, int *rety);
		void SetExtensions (CExtension *pAdventure, int yPos);

		CCloudService &m_Service;
		CGameSettings &m_Settings;
		TArray<CExtension *> m_AdventureList;
		TArray<CExtension *> m_ExtensionList;
		int m_iSelection;

		CAniVScroller *m_pRoot;
		CG32bitImage *m_pBackground;

		RECT m_rcFull;
		RECT m_rcPane;
		RECT m_rcBackground;
		int m_yBottomSection;
	};

class CGalacticMapSession : public IHISession
	{
	public:
		CGalacticMapSession (CHumanInterface &HI) : IHISession(HI), m_pMap(NULL), m_pPainter(NULL) { }

		//	IHISession virtuals
		virtual void OnCleanUp (void);
		virtual ALERROR OnInit (CString *retsError);
		virtual void OnKeyDown (int iVirtKey, DWORD dwKeyData);
		virtual void OnLButtonDown (int x, int y, DWORD dwFlags, bool *retbCapture);
		virtual void OnPaint (CG32bitImage &Screen, const RECT &rcInvalid);
		virtual void OnReportHardCrash (CString *retsMessage);
		virtual void OnUpdate (bool bTopMost);

	private:
		int GetScale (int iScaleIndex);
		int GetScaleIndex (int iScale);

		CSystemMap *m_pMap;
		CString m_sMapName;

		CGalacticMapPainter *m_pPainter;
		int m_iScale;
		int m_iTargetScaleIndex;
		int m_xCenter;
		int m_yCenter;
		int m_xTargetCenter;
		int m_yTargetCenter;

		int m_iMinScaleIndex;
		int m_iMaxScaleIndex;

		RECT m_rcView;
		RECT m_rcHelp;
	};

class CHelpSession : public IHISession
	{
	public:
		CHelpSession (CHumanInterface &HI) : IHISession(HI) { }

		//	IHISession virtuals
		virtual void OnCleanUp (void);
		virtual ALERROR OnInit (CString *retsError);
		virtual void OnKeyDown (int iVirtKey, DWORD dwKeyData);
		virtual void OnLButtonDown (int x, int y, DWORD dwFlags, bool *retbCapture);
		virtual void OnPaint (CG32bitImage &Screen, const RECT &rcInvalid);
		virtual void OnReportHardCrash (CString *retsMessage);
		virtual void OnUpdate (bool bTopMost);

	private:
		CG32bitImage m_HelpImage;
		int m_iHelpPage;
	};

class CGameSession : public IHISession
	{
	public:
		CGameSession (CHumanInterface &HI, 
					  CGameSettings &Settings,
					  CSoundtrackManager &Soundtrack) : IHISession(HI),
				m_Settings(Settings),
				m_Soundtrack(Soundtrack)
			{ }

		//	IHISession virtuals

		virtual CReanimator &GetReanimator (void) { return g_pTrans->GetReanimator(); }
		virtual void OnAnimate (CG32bitImage &Screen, bool bTopMost) { CSmartLock Lock(g_pUniverse->GetSem()); g_pTrans->Animate(Screen, this, bTopMost); }
		virtual void OnChar (char chChar, DWORD dwKeyData) { g_pTrans->WMChar(chChar, dwKeyData); }
		virtual ALERROR OnCommand (const CString &sCmd, void *pData = NULL) { return NOERROR; }
		virtual ALERROR OnInit (CString *retsError) { m_rcScreen = g_pTrans->m_rcScreen; SetNoCursor(true); return NOERROR; }
		virtual void OnKeyDown (int iVirtKey, DWORD dwKeyData) { g_pTrans->WMKeyDown(iVirtKey, dwKeyData); }
		virtual void OnKeyUp (int iVirtKey, DWORD dwKeyData) { g_pTrans->WMKeyUp(iVirtKey, dwKeyData); }
		virtual void OnLButtonDblClick (int x, int y, DWORD dwFlags) { g_pTrans->WMLButtonDblClick(x, y, dwFlags); }
		virtual void OnLButtonDown (int x, int y, DWORD dwFlags, bool *retbCapture) { g_pTrans->WMLButtonDown(x, y, dwFlags); }
		virtual void OnLButtonUp (int x, int y, DWORD dwFlags) { g_pTrans->WMLButtonUp(x, y, dwFlags); }
		virtual void OnMouseMove (int x, int y, DWORD dwFlags) { g_pTrans->WMMouseMove(x, y, dwFlags); }
		virtual void OnMove (int x, int y) { g_pTrans->WMMove(x, y); }
		virtual void OnReportHardCrash (CString *retsMessage) { *retsMessage = g_pTrans->GetCrashInfo(); }
		virtual void OnSize (int cxWidth, int cyHeight) { g_pTrans->WMSize(cxWidth, cyHeight, 0); }

		//	Helpers

		void PaintInfoText (CG32bitImage &Dest, const CString &sTitle, const TArray<CString> &Body, bool bAboveTargeting = true);
		void PaintSoundtrackTitles (CG32bitImage &Dest);

	private:
		CGameSettings &m_Settings;
		CSoundtrackManager &m_Soundtrack;
		RECT m_rcScreen;					//	RECT of main screen within window.
	};

class CLoadingSession : public IHISession
	{
	public:
		CLoadingSession (CHumanInterface &HI, const CString &sCopyright) : IHISession(HI),
				m_sCopyright(sCopyright),
				m_iTick(0) 
			{ }

		//	IHISession virtuals
		virtual ALERROR OnInit (CString *retsError);
		virtual void OnPaint (CG32bitImage &Screen, const RECT &rcInvalid);
		virtual void OnReportHardCrash (CString *retsMessage);
		virtual void OnUpdate (bool bTopMost);

	private:
		CG32bitImage m_TitleImage;
		CG32bitImage m_StargateImage;
		CString m_sCopyright;
		int m_iTick;
		RECT m_rcStargate;
		int m_cyCopyright;
	};

class CLoginSession : public IHISession
	{
	public:
		CLoginSession (CHumanInterface &HI, CCloudService &Service, const CString &sCommand = NULL_STR);

		//	IHISession virtuals
		virtual void OnCleanUp (void);
		virtual ALERROR OnCommand (const CString &sCmd, void *pData = NULL);
		virtual ALERROR OnInit (CString *retsError);
		virtual void OnKeyDown (int iVirtKey, DWORD dwKeyData);
		virtual void OnLButtonDown (int x, int y, DWORD dwFlags, bool *retbCapture);
		virtual void OnPaint (CG32bitImage &Screen, const RECT &rcInvalid);
		virtual void OnReportHardCrash (CString *retsMessage);
		virtual void OnUpdate (bool bTopMost);

	private:
		enum Dialogs
			{
			dlgNone,
			dlgRegister,
			dlgSignIn,
			};

		void CmdCancel (void);
		void CmdPasswordReset (void);
		void CmdRegister (void);
		void CmdRegisterComplete (CRegisterUserTask *pTask);
		void CmdSignIn (void);
		void CmdSignInComplete (CSignInUserTask *pTask);
		void CmdSuccess (void);

		void CreateDlgMessage (const CVisualPalette &VI, const RECT &rcRect, IAnimatron **retpAni);
		void CreateDlgRegister (const CVisualPalette &VI, IAnimatron **retpAni);
		void CreateDlgSignIn (const CVisualPalette &VI, IAnimatron **retpAni);
		void ShowInitialDlg (void);

		CCloudService &m_Service;

		CString m_sCommand;					//	Command to issue after successful login.

		Dialogs m_iCurrent;					//	Current dialog shown
		RECT m_rcInputError;
		bool m_bBlankEmailWarning;			//	If TRUE we already warned about a blank email.
	};

class CLoadGameSession : public IHISession
	{
	public:
		CLoadGameSession (CHumanInterface &HI, CCloudService &Service, const TArray<CString> &Folders);

		//	IHISession virtuals
		virtual ALERROR OnCommand (const CString &sCmd, void *pData = NULL);
		virtual ALERROR OnInit (CString *retsError);
		virtual void OnKeyDown (int iVirtKey, DWORD dwKeyData);
		virtual void OnPaint (CG32bitImage &Screen, const RECT &rcInvalid);
		virtual void OnReportHardCrash (CString *retsMessage);

	private:
		void CmdCancel (void);
		void CmdDeleteFile (void);
		void CmdOK (void);
		void CmdReadComplete (CListSaveFilesTask *pTask);

		CCloudService &m_Service;
		TArray<CString> m_Folders;
	};

class CModExchangeSession : public IHISession
	{
	public:
		CModExchangeSession (CHumanInterface &HI, CCloudService &Service, CMultiverseModel &Multiverse, CExtensionCollection &Extensions, bool bDebugMode = false);

		//	IHISession virtuals
		virtual ALERROR OnCommand (const CString &sCmd, void *pData = NULL);
		virtual ALERROR OnInit (CString *retsError);
		virtual void OnKeyDown (int iVirtKey, DWORD dwKeyData);
		virtual void OnPaint (CG32bitImage &Screen, const RECT &rcInvalid);
		virtual void OnReportHardCrash (CString *retsMessage);

	private:
		void CmdDone (void);
		void CmdRefresh (bool bFullRefresh = true);
		void CmdRefreshComplete (CListCollectionTask *pTask);

		CCloudService &m_Service;
		CMultiverseModel &m_Multiverse;
		CExtensionCollection &m_Extensions;
		bool m_bDebugMode;
		bool m_bWaitingForRefresh;
		bool m_bRefreshAgain;
	};

class CNewGameSession : public IHISession
	{
	public:
		CNewGameSession (CHumanInterface &HI, CCloudService &Service, const SNewGameSettings &Defaults);

		//	IHISession virtuals
		virtual void OnCleanUp (void);
		virtual ALERROR OnCommand (const CString &sCmd, void *pData = NULL);
		virtual ALERROR OnInit (CString *retsError);
		virtual void OnKeyDown (int iVirtKey, DWORD dwKeyData);
		virtual void OnLButtonDown (int x, int y, DWORD dwFlags, bool *retbCapture);
		virtual void OnMouseWheel (int iDelta, int x, int y, DWORD dwFlags);
		virtual void OnPaint (CG32bitImage &Screen, const RECT &rcInvalid);
		virtual void OnReportHardCrash (CString *retsMessage);
		virtual void OnUpdate (bool bTopMost);

	private:
		void AddClassInfo (CShipClass *pClass, const CDeviceDescList &Devices, const CItem &Item, int x, int y, int cxWidth, DWORD dwOptions, int *retcyHeight, IAnimatron **retpAni);
		void CmdCancel (void);
		void CmdChangeGenome (void);
		void CmdEditName (void);
		void CmdEditNameCancel (void);
		void CmdNextShipClass (void);
		void CmdOK (void);
		void CmdPrevShipClass (void);
		void CreatePlayerGenome (GenomeTypes iGenome, int x, int y, int cxWidth);
		void CreatePlayerName (const CString &sName, int x, int y, int cxWidth);
		void CreateShipClass (CShipClass *pClass, int x, int y, int cxWidth);
		void SetPlayerGenome (GenomeTypes iGenome, int x, int y, int cxWidth);
		void SetPlayerName (const CString &sName, int x, int y, int cxWidth);
		void SetShipClass (CShipClass *pClass, int x, int y, int cxWidth);
		void SetShipClassDesc (const CString &sDesc, int x, int y, int cxWidth);
		void SetShipClassImage (CShipClass *pClass, int x, int y, int cxWidth);
		void SetShipClassName (const CString &sName, int x, int y, int cxWidth);

		CCloudService &m_Service;
		SNewGameSettings m_Settings;

		TSortMap<CString, CShipClass *> m_ShipClasses;
		int m_iCurShipClass;
		bool m_bEditingName;

		CAniVScroller *m_pRoot;

		int m_xLeftCol;
		int m_cxLeftCol;
		int m_xCenterCol;
		int m_cxCenterCol;
		int m_xRightCol;
		int m_cxRightCol;

		int m_xPlayerName;
		int m_yPlayerName;
		int m_cxPlayerName;
		int m_xPlayerGenome;
		int m_yPlayerGenome;
		int m_cxPlayerGenome;
		int m_xShipClass;
		int m_yShipClass;
		int m_cxShipClass;
	};

class CProfileSession : public IHISession
	{
	public:
		CProfileSession (CHumanInterface &HI, CCloudService &Service);

		//	IHISession virtuals
		virtual void OnCleanUp (void);
		virtual ALERROR OnCommand (const CString &sCmd, void *pData = NULL);
		virtual ALERROR OnInit (CString *retsError);
		virtual void OnKeyDown (int iVirtKey, DWORD dwKeyData);
		virtual void OnLButtonDown (int x, int y, DWORD dwFlags, bool *retbCapture);
		virtual void OnMouseWheel (int iDelta, int x, int y, DWORD dwFlags);
		virtual void OnPaint (CG32bitImage &Screen, const RECT &rcInvalid);
		virtual void OnReportHardCrash (CString *retsMessage);
		virtual void OnUpdate (bool bTopMost);

	private:
		enum Panes
			{
			paneNone,
			paneRecords,
			paneAccolades,
			};

		void CreateRecordList (const CVisualPalette &VI, const RECT &rcRect, IAnimatron **retpAni);
		void CmdReadComplete (CReadProfileTask *pTask);
		void GetPaneRect (RECT *retrcRect);

		CCloudService &m_Service;

		Panes m_iPane;						//	Current pane being shown
		RECT m_rcPane;						//	Rect of center pane
		RECT m_rcRecords;					//	Rect of records
	};

class CStatsSession : public IHISession
	{
	public:
		enum Flags
			{
			SHOW_TASK_PROGRESS = 0x00000001,
			};

		CStatsSession (CHumanInterface &HI, CCloudService &Service, CGameStats &Stats, DWORD dwFlags = 0) : IHISession(HI), m_Service(Service), m_dwFlags(dwFlags) { m_Stats.TakeHandoff(Stats); }

		//	IHISession virtuals
		virtual void OnCleanUp (void);
		virtual ALERROR OnCommand (const CString &sCmd, void *pData = NULL);
		virtual ALERROR OnInit (CString *retsError);
		virtual void OnKeyDown (int iVirtKey, DWORD dwKeyData);
		virtual void OnLButtonDown (int x, int y, DWORD dwFlags, bool *retbCapture);
		virtual void OnPaint (CG32bitImage &Screen, const RECT &rcInvalid);
		virtual void OnReportHardCrash (CString *retsMessage);
		virtual void OnUpdate (bool bTopMost);

	private:
		void CreateCopyAnimation (const RECT &rcRect, int iDuration, IAnimatron **retpAni);
		CString GetTaskProgressText (void);
		void SetSelection (int iPos);

		CCloudService &m_Service;
		CGameStats m_Stats;
		DWORD m_dwFlags;
		CG32bitImage m_BackgroundImage;
		RECT m_rcStats;
		RECT m_rcTaskProgress;
	};

class CTextCrawlSession : public IHISession
	{
	public:
		CTextCrawlSession (CHumanInterface &HI,
						   CCloudService &Service,
						   const CG32bitImage *pImage,
						   const CString &sText,
						   const CString &sCmdDone);

		//	IHISession virtuals
		virtual void OnCleanUp (void);
		virtual ALERROR OnCommand (const CString &sCmd, void *pData = NULL);
		virtual ALERROR OnInit (CString *retsError);
		virtual void OnKeyDown (int iVirtKey, DWORD dwKeyData);
		virtual void OnLButtonDown (int x, int y, DWORD dwFlags, bool *retbCapture);
		virtual void OnPaint (CG32bitImage &Screen, const RECT &rcInvalid);
		virtual void OnReportHardCrash (CString *retsMessage);
		virtual void OnUpdate (bool bTopMost);

	private:
		void CreateCrawlAnimation (const CString &sText, const RECT &rcRect, IAnimatron **retpAni);

		CCloudService &m_Service;
		const CG32bitImage *m_pImage;
		CString m_sText;
		CString m_sCmdDone;
		bool m_bWaitAnimation;
	};

class CWaitSession : public IHISession
	{
	public:
		CWaitSession (CHumanInterface &HI, CCloudService &Service, const CString &sTitle);

		//	IHISession virtuals
		virtual ALERROR OnCommand (const CString &sCmd, void *pData = NULL);
		virtual ALERROR OnInit (CString *retsError);
		virtual void OnKeyDown (int iVirtKey, DWORD dwKeyData);
		virtual void OnPaint (CG32bitImage &Screen, const RECT &rcInvalid);
		virtual void OnReportHardCrash (CString *retsMessage);

	private:
		CCloudService &m_Service;
		CString m_sTitle;
	};

//	More sessions

#include "IntroSession.h"
