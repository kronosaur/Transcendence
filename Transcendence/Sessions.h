//	Sessions.h
//
//	Transcendence session classes
//	Copyright (c) 2010 by Kronosaur Productions, LLC. All Rights Reserved.

class CAdventureIntroSession : public IHISession
	{
	public:
		CAdventureIntroSession (CHumanInterface &HI, CCloudService &Service, const CString &sAdventureName);

		//	IHISession virtuals
		virtual ALERROR OnCommand (const CString &sCmd, void *pData = NULL) override;
		virtual ALERROR OnInit (CString *retsError) override;
		virtual void OnKeyDown (int iVirtKey, DWORD dwKeyData) override;
		virtual void OnPaint (CG32bitImage &Screen, const RECT &rcInvalid) override;
		virtual void OnReportHardCrash (CString *retsMessage) override;

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
		virtual ALERROR OnCommand (const CString &sCmd, void *pData = NULL) override;
		virtual ALERROR OnInit (CString *retsError) override;
		virtual void OnKeyDown (int iVirtKey, DWORD dwKeyData) override;
		virtual void OnPaint (CG32bitImage &Screen, const RECT &rcInvalid) override;
		virtual void OnReportHardCrash (CString *retsMessage) override;

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
		virtual ALERROR OnCommand (const CString &sCmd, void *pData = NULL) override;
		virtual ALERROR OnInit (CString *retsError) override;
		virtual void OnKeyDown (int iVirtKey, DWORD dwKeyData) override;
		virtual void OnMouseWheel (int iDelta, int x, int y, DWORD dwFlags) override;
		virtual void OnPaint (CG32bitImage &Screen, const RECT &rcInvalid) override;
		virtual void OnReportHardCrash (CString *retsMessage) override;

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
        struct SOptions
            {
            SOptions (void) :
                    xCenter(0),
                    yCenter(0),
                    iScale(0),
                    pSelected(NULL),
                    pCurNode(NULL)
                { }

            int xCenter;
            int yCenter;
            int iScale;                     //  0 = use defaults for everything
            CTopologyNode *pSelected;
            CTopologyNode *pCurNode;        //  Node at the time we saved options
            };

        CGalacticMapSession (CHumanInterface &HI, CGameSettings &Settings, CSystemMapThumbnails &SystemMapThumbnails, SOptions &SavedState);

		//	IHISession virtuals
        virtual void OnChar (char chChar, DWORD dwKeyData) override;
		virtual void OnCleanUp (void) override;
		virtual ALERROR OnInit (CString *retsError) override;
		virtual void OnKeyDown (int iVirtKey, DWORD dwKeyData) override;
		virtual void OnLButtonDown (int x, int y, DWORD dwFlags, bool *retbCapture) override;
        virtual void OnLButtonUp (int x, int y, DWORD dwFlags) override;
        virtual void OnMouseMove (int x, int y, DWORD dwFlags) override;
        virtual void OnMouseWheel (int iDelta, int x, int y, DWORD dwFlags) override;
		virtual void OnPaint (CG32bitImage &Screen, const RECT &rcInvalid) override;
		virtual void OnReportHardCrash (CString *retsMessage) override;
		virtual void OnUpdate (bool bTopMost);

	private:
        void SaveState (void);
        void Select (CTopologyNode *pNode, bool bNoSound = false);
        void SetTargetScale (void);

        CGameSettings &m_Settings;
        CSystemMapThumbnails &m_SystemMapThumbnails;
        SOptions &m_SavedState;
		CSystemMap *m_pMap;
        int m_iMinScale;
        int m_iMaxScale;
		int m_iMinScaleIndex;
		int m_iMaxScaleIndex;

		CGalacticMapPainter *m_pPainter;
        CMapLegendPainter m_HelpPainter;

		RECT m_rcView;

        CMapScaleCounter m_Scale;           //  Map scale (100 = normal)
		int m_xCenter;                      //  Current center (in galactic coordinates)
		int m_yCenter;

		int m_xTargetCenter;                //  Desired center
		int m_yTargetCenter;

        bool m_bDragging;                   //  TRUE if we're dragging the map
        int m_xAnchor;                      //  Click anchor (in galactic coordinates)
        int m_yAnchor;
        int m_xAnchorCenter;                //  Map center at time of anchor drop
        int m_yAnchorCenter;
	};

class CHelpSession : public IHISession
	{
	public:
		CHelpSession (CHumanInterface &HI) : IHISession(HI) { }

		//	IHISession virtuals
		virtual void OnCleanUp (void) override;
		virtual ALERROR OnInit (CString *retsError) override;
		virtual void OnKeyDown (int iVirtKey, DWORD dwKeyData) override;
		virtual void OnLButtonDown (int x, int y, DWORD dwFlags, bool *retbCapture) override;
		virtual void OnPaint (CG32bitImage &Screen, const RECT &rcInvalid) override;
		virtual void OnReportHardCrash (CString *retsMessage) override;
		virtual void OnUpdate (bool bTopMost) override;

	private:
		CG32bitImage m_HelpImage;
		int m_iHelpPage;
	};

class CKeyboardMapSession : public IHISession
    {
	public:
		CKeyboardMapSession (CHumanInterface &HI, CCloudService &Service, CGameSettings &Settings);

		//	IHISession virtuals
		virtual ALERROR OnCommand (const CString &sCmd, void *pData = NULL) override;
		virtual ALERROR OnInit (CString *retsError) override;
		virtual void OnKeyDown (int iVirtKey, DWORD dwKeyData) override;
		virtual void OnLButtonDown (int x, int y, DWORD dwFlags, bool *retbCapture) override;
        virtual void OnLButtonUp (int x, int y, DWORD dwFlags) override;
        virtual void OnMouseMove (int x, int y, DWORD dwFlags) override;
		virtual void OnPaint (CG32bitImage &Screen, const RECT &rcInvalid) override;
		virtual void OnReportHardCrash (CString *retsMessage) override;
		virtual void OnUpdate (bool bTopMost) override;

	private:
        enum EKeySymbols
            {
            symbolNone,

            symbolArrowUp,
            symbolArrowDown,
            symbolArrowLeft,
            symbolArrowRight,
            };

        enum EKeyFlags
            {
            FLAG_RESERVED = 0x00000001,
            };

		enum EDeviceTypes
			{
			deviceNone,

			deviceKeyboard,
			deviceMouse,
			deviceNumpad,
			};

        struct SKeyData
            {
            char *pszKeyID;
            int xCol;                       //  A normal key takes up two columns (col 0 = left-most)
            int yRow;                       //  A normal key takes one row (row 0 = top row)
            int cxWidth;                    //  Width of the key in columns
			int cyHeight;					//	Height in rows
            char *pszLabel;                 //  Label in keyboard
            EKeySymbols iSymbol;            //  Symbol for label label
            DWORD dwFlags;
            };

		struct SDeviceData
			{
			EDeviceTypes iDevice;			//	Device type
			char *pszLabel;					//	Label for the device (e.g., keyboard, mouse, controller)
			const SKeyData *pKeys;			//	Keys available to map
			int iKeyCount;					//	Number of keys in map

			int iCols;						//	Columns
			int iRows;						//	Rows
			int xOffset;
			int yOffset;
			};

        struct SKeyDesc
            {
            CString sKeyID;                 //  ID of key in CGameKeys
			DWORD dwVirtKey;				//	Virtual key
            RECT rcRect;                    //  Rect of key to draw
            CString sLabel;                 //  Key label
            EKeySymbols iSymbol;            //  Symbols for label
            int iCmdBinding;                //  Index into m_Commands (-1 = none)
            DWORD dwFlags;
            };

        struct SCommandDesc
            {
			SCommandDesc (void) :
					iCmd(CGameKeys::keyNone),
					iKeyBinding(-1),
					cxLabel(0),
					rcRect{ 0, 0, 0, 0 }
				{ }

            CGameKeys::Keys iCmd;           //  Command
			CString sKeyBinding;			//	ID of key we're bound to
            int iKeyBinding;                //  Index into m_Keys (-1 = none)
            CString sLabel;                 //  Command label
            int cxLabel;                    //  Width of command label

            RECT rcRect;                    //  Draw the command label here.
            TArray<POINT> Line;             //  Line to connect command to key
            };

        struct STargetCtx
            {
            int iCmdIndex;                  //  Index of command hit (-1 = none)
            int iKeyIndex;                  //  Index of key hit (-1 = none)
            bool bInKey;                    //  In key (otherwise, in command)
            };

        void ArrangeCommandLabels (const RECT &rcRect, const RECT &rcKeyboard);
        void CmdClearBinding (void);
        void CmdNextLayout (void);
        void CmdPrevLayout (void);
        void CmdResetDefault (void);
        void CreateDeviceSelector (void);
		void InitBindings (void);
		void InitCommands (void);
		void InitDevice (const SDeviceData &Device);
        bool HitTest (int x, int y, STargetCtx &Ctx);
        void PaintKey (CG32bitImage &Screen, const SKeyDesc &Key, CG32bitPixel rgbBack, CG32bitPixel rgbText, bool bFlash);
		void UpdateDeviceSelector (void);
		void UpdateMenu (void);

        CCloudService &m_Service;
		CGameSettings &m_Settings;
        TArray<SKeyDesc> m_Keys;
        TArray<SCommandDesc> m_Commands;
		RECT m_rcRect;

        //  Keyboard metrics (valid after OnInit)

        int m_xKeyboard;                    //  Position of keyboard
        int m_yKeyboard;
        int m_cxKeyboard;                   //  Size of keyboard
        int m_cyKeyboard;
        int m_cxKeyCol;                     //  Size of a key column (half the size of a key row)
        int m_cyKeyRow;                     //  Height of a key
        TSortMap<CString, int> m_KeyIDToIndex;

        bool m_bEditable;                   //  TRUE if we can edit this layout
		int m_iDevice;						//	Selected device
        int m_iHoverKey;                    //  Hovering over this key (edit mode only)
        int m_iHoverCommand;                //  Hovering over this command
        int m_iSelectedCommand;             //  Command selected
		int m_iTick;
		int m_iFlashKey;					//	Index of key (in m_Keys) to flash
		int m_iFlashUntil;					//	Stop flashing on this tick

		static const SDeviceData DEVICE_DATA[];
        static const int DEVICE_DATA_COUNT;

        static const SKeyData KEYBOARD_DATA[];
        static const int KEYBOARD_DATA_COUNT;
        static const SKeyData NUMPAD_DATA[];
        static const int NUMPAD_DATA_COUNT;
        static const SKeyData MOUSE_DATA[];
        static const int MOUSE_DATA_COUNT;
    };

class CLoadingSession : public IHISession
	{
	public:
		CLoadingSession (CHumanInterface &HI, const CString &sCopyright) : IHISession(HI),
				m_sCopyright(sCopyright),
				m_iTick(0) 
			{ }

		//	IHISession virtuals
		virtual ALERROR OnInit (CString *retsError) override;
		virtual void OnPaint (CG32bitImage &Screen, const RECT &rcInvalid) override;
		virtual void OnReportHardCrash (CString *retsMessage) override;
		virtual void OnUpdate (bool bTopMost) override;

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
		virtual void OnCleanUp (void) override;
		virtual ALERROR OnCommand (const CString &sCmd, void *pData = NULL) override;
		virtual ALERROR OnInit (CString *retsError) override;
		virtual void OnKeyDown (int iVirtKey, DWORD dwKeyData) override;
		virtual void OnLButtonDown (int x, int y, DWORD dwFlags, bool *retbCapture) override;
		virtual void OnPaint (CG32bitImage &Screen, const RECT &rcInvalid) override;
		virtual void OnReportHardCrash (CString *retsMessage) override;
		virtual void OnUpdate (bool bTopMost) override;

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
		CLoadGameSession (CHumanInterface &HI, CCloudService &Service, const TArray<CString> &Folders, bool m_bFilterPermadeath);

		//	IHISession virtuals
		virtual ALERROR OnCommand (const CString &sCmd, void *pData = NULL) override;
		virtual ALERROR OnInit (CString *retsError) override;
		virtual void OnKeyDown (int iVirtKey, DWORD dwKeyData) override;
		virtual void OnPaint (CG32bitImage &Screen, const RECT &rcInvalid) override;
		virtual void OnReportHardCrash (CString *retsMessage) override;

	private:
		void CmdCancel (void);
		void CmdDeleteFile (void);
		void CmdOK (void);
		void CmdReadComplete (CListSaveFilesTask *pTask);

		CCloudService &m_Service;
		TArray<CString> m_Folders;
		bool m_bFilterPermadeath;
	};

class CModExchangeSession : public IHISession
	{
	public:
		CModExchangeSession (CHumanInterface &HI, CCloudService &Service, CMultiverseModel &Multiverse, CExtensionCollection &Extensions, bool bDebugMode = false);

		//	IHISession virtuals
		virtual ALERROR OnCommand (const CString &sCmd, void *pData = NULL) override;
		virtual ALERROR OnInit (CString *retsError) override;
		virtual void OnKeyDown (int iVirtKey, DWORD dwKeyData) override;
		virtual void OnPaint (CG32bitImage &Screen, const RECT &rcInvalid) override;
		virtual void OnReportHardCrash (CString *retsMessage) override;

	private:
		enum EStates
			{
			stateNone,

			stateWaitingForList,
			stateWaitingForReload,
			};

		bool CanBeEnabledDisabled (const CMultiverseCatalogEntry *pCatalogEntry = NULL) const;
		void CmdDisableExtension (void);
		void CmdDone (void);
		void CmdEnableExtension (void);
		void CmdOnSelectionChanged (void);
		void CmdRefresh (void);
		void CmdReload (void);
		void CmdRefreshComplete (CListCollectionTask *pTask);
		TArray<CUIHelper::SMenuEntry> CreateMenu (CMultiverseCatalogEntry *pCatalogEntry = NULL);
		bool GetCurrentSelection (CMultiverseCatalogEntry &Entry) const;
		DWORD GetCurrentSelectionUNID (void) const;
		void OnCollectionUpdated (void);
		void StartListCollectionTask (DWORD dwSelect = 0);
		void StartWaitAnimation (void);

		CCloudService &m_Service;
		CMultiverseModel &m_Multiverse;
		CExtensionCollection &m_Extensions;
		TArray<CMultiverseCatalogEntry> m_Collection;
		TSharedPtr<CG32bitImage> m_pGenericIcon;

		EStates m_iState = stateNone;
		DWORD m_dwSelect = 0;
		bool m_bShowLibraries = false;
		bool m_bDebugMode = false;
		bool m_bRefreshAgain = false;
	};

class CNewGameSession : public IHISession
	{
	public:
		CNewGameSession (CHumanInterface &HI, CCloudService &Service, const SNewGameSettings &Defaults);

		//	IHISession virtuals
		virtual void OnCleanUp (void) override;
		virtual ALERROR OnCommand (const CString &sCmd, void *pData = NULL) override;
		virtual ALERROR OnInit (CString *retsError) override;
		virtual void OnKeyDown (int iVirtKey, DWORD dwKeyData) override;
		virtual void OnLButtonDown (int x, int y, DWORD dwFlags, bool *retbCapture) override;
		virtual void OnMouseWheel (int iDelta, int x, int y, DWORD dwFlags) override;
		virtual void OnPaint (CG32bitImage &Screen, const RECT &rcInvalid) override;
		virtual void OnReportHardCrash (CString *retsMessage) override;
		virtual void OnUpdate (bool bTopMost) override;

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
		virtual void OnCleanUp (void) override;
		virtual ALERROR OnCommand (const CString &sCmd, void *pData = NULL) override;
		virtual ALERROR OnInit (CString *retsError) override;
		virtual void OnKeyDown (int iVirtKey, DWORD dwKeyData) override;
		virtual void OnLButtonDown (int x, int y, DWORD dwFlags, bool *retbCapture) override;
		virtual void OnMouseWheel (int iDelta, int x, int y, DWORD dwFlags) override;
		virtual void OnPaint (CG32bitImage &Screen, const RECT &rcInvalid) override;
		virtual void OnReportHardCrash (CString *retsMessage) override;
		virtual void OnUpdate (bool bTopMost) override;

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
		virtual void OnCleanUp (void) override;
		virtual ALERROR OnCommand (const CString &sCmd, void *pData = NULL) override;
		virtual ALERROR OnInit (CString *retsError) override;
		virtual void OnKeyDown (int iVirtKey, DWORD dwKeyData) override;
		virtual void OnLButtonDown (int x, int y, DWORD dwFlags, bool *retbCapture) override;
		virtual void OnMouseWheel (int iDelta, int x, int y, DWORD dwFlags) override;
		virtual void OnPaint (CG32bitImage &Screen, const RECT &rcInvalid) override;
		virtual void OnReportHardCrash (CString *retsMessage) override;
		virtual void OnUpdate (bool bTopMost) override;

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
		virtual void OnCleanUp (void) override;
		virtual ALERROR OnCommand (const CString &sCmd, void *pData = NULL) override;
		virtual ALERROR OnInit (CString *retsError) override;
		virtual void OnKeyDown (int iVirtKey, DWORD dwKeyData) override;
		virtual void OnLButtonDown (int x, int y, DWORD dwFlags, bool *retbCapture) override;
		virtual void OnPaint (CG32bitImage &Screen, const RECT &rcInvalid) override;
		virtual void OnReportHardCrash (CString *retsMessage) override;
		virtual void OnUpdate (bool bTopMost) override;

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
		virtual ALERROR OnCommand (const CString &sCmd, void *pData = NULL) override;
		virtual ALERROR OnInit (CString *retsError) override;
		virtual void OnKeyDown (int iVirtKey, DWORD dwKeyData) override;
		virtual void OnPaint (CG32bitImage &Screen, const RECT &rcInvalid) override;
		virtual void OnReportHardCrash (CString *retsMessage) override;

	private:
		CCloudService &m_Service;
		CString m_sTitle;
	};

//	More sessions

#include "IntroSession.h"
