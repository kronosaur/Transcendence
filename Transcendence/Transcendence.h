//	Transcendence.h
//
//	Main header file

#ifndef INCL_TRANSCENDENCE
#define INCL_TRANSCENDENCE

#ifdef DEBUG
//#define DEBUG_MOUSE_AIM
#endif

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
#include "PlayerShip.h"

//	Intro

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

class CMessageDisplay
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
		bool LButtonDown (int x, int y);
		bool MouseMove (int x, int y);
		bool MouseWheel (int iDelta, int x, int y);
		void Paint (CG32bitImage &Dest);
		inline void ResetSelection (void) { m_iSelection = 0; m_iHover = -1; m_iFirstInView = 0; m_cxSmoothScroll = 0; m_bInvalid = true; }
		void SelectNext (void);
		void SelectPrev (void);
		inline void SetFontTable (const SFontTable *pFonts) { m_pFonts = pFonts; }
		inline void SetHelpText (const CString &sText) { m_sHelpText = sText; }

	private:
		int HitTest (int x, int y) const;
		void PaintSelection (CG32bitImage &Dest, int x, int y);
		void Update (void);

		CMenuData *m_pMenu;

		RECT m_rcRect;
		CG32bitImage m_Buffer;
		const SFontTable *m_pFonts;
		int m_iSelection;
		int m_iHover;
		int m_cxSmoothScroll;

		RECT m_rcView;
		int m_iFirstInView;
		int m_iCountInView;

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
        inline void SetRect (const RECT &rcRect) { m_rcRect = rcRect;  ComputeButtonRects();  }
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

class CCommandLineDisplay
	{
	public:
		CCommandLineDisplay (const CVisualPalette &VI) :
				m_VI(VI)
			{ }

		~CCommandLineDisplay (void);

		void CleanUp (void);
		inline void ClearInput (void) { m_sInput = NULL_STR; m_iCursorPos = 0; m_bInvalid = true; }
		inline void ClearHint (void) { m_sHint = NULL_STR; m_iScrollPos = 0; m_bInvalid = true; }
		inline const CString &GetInput (void) { return m_sInput; }
		inline int GetOutputLineCount (void) { return GetOutputCount(); }
		inline const RECT &GetRect (void) { return m_rcRect; }
		ALERROR Init (const RECT &rcRect);
		void Input (const CString &sInput);
		void InputBackspace (void);
		void InputDelete (void);
		void InputEnter (void);
		void InputHistoryUp(void);
		void InputHistoryDown(void);
		inline bool IsEnabled (void) const { return m_bEnabled; }
		bool OnChar (char chChar, DWORD dwKeyData);
		void OnKeyDown (int iVirtKey, DWORD dwKeyState);
		void Output (const CString &sOutput, CG32bitPixel rgbColor = CG32bitPixel::Null());
		void Paint (CG32bitImage &Dest);
		inline void SetEnabled (bool bEnabled = true) { m_bEnabled = bEnabled; }

	private:
		enum Constants
			{
			MAX_LINES = 200,
			};

		void AppendOutput (const CString &sLine, CG32bitPixel rgbColor);
		const CString &GetOutput (int iLine);
		CG32bitPixel GetOutputColor (int iLine);
		int GetOutputCount (void);
		void AppendHistory(const CString &sLine);
		const CString &GetHistory(int iLine);
		int GetHistoryCount(void);
		void Update (void);
		const CString GetCurrentCmd (void);
		void AutoCompleteSearch (void);

		const CVisualPalette &m_VI;
		const CG16bitFont *m_pFont = NULL;
		RECT m_rcRect;
		bool m_bEnabled = false;

		CString m_Output[MAX_LINES + 1];
		CG32bitPixel m_OutputColor[MAX_LINES + 1];
		int m_iOutputStart = 0;
		int m_iOutputEnd = 0;
		CString m_sInput;
		CString m_History[MAX_LINES + 1];
		CString m_sHint;
		int m_iHistoryStart = 0;
		int m_iHistoryEnd = 0;
		int m_iHistoryIndex = 0;
		int m_iCursorPos = 0;
		int m_iScrollPos = 0;

		CG32bitImage m_Buffer;
		bool m_bInvalid = true;
		int m_iCounter = 0;
		RECT m_rcCursor;
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

class CTranscendenceWnd : public CUniverse::IHost, public IAniCommand
	{
	public:
		CTranscendenceWnd (HWND hWnd, CTranscendenceController *pTC);

		void Autopilot (bool bTurnOn);
		void CleanUpPlayerShip (void);
		void ClearMessage (void);
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
		inline bool InAutopilot (void) { return m_bAutopilot; }
		inline bool InDockState (void) { return m_State == gsDocked; }
		inline bool InGameState (void) { return m_State == gsInGame; }
		ALERROR InitDisplays (void);
		void OnStargateSystemReady (void);
		void PlayerEndGame (void);
		void PlayerEnteredGate (CSystem *pSystem, 
							    CTopologyNode *pDestNode,
							    const CString &sDestEntryPoint);
		void RedirectDisplayMessage (bool bRedirect = true);
		inline void UpdateDeviceCounterDisplay (void) { m_DeviceDisplay.Invalidate(); }

		//	CUniverse::IHost
		virtual void ConsoleOutput (const CString &sLine) override;
		virtual IPlayerController *CreatePlayerController (void) override;
		virtual IShipController *CreateShipController (const CString &sController) override;
		virtual void DebugOutput (const CString &sLine) override;
		virtual bool FindFont (const CString &sFont, const CG16bitFont **retpFont = NULL) const override;
		virtual void GameOutput (const CString &sLine) override;
		virtual CG32bitPixel GetColor (const CString &sColor) const override;
		virtual const CG16bitFont &GetFont (const CString &sFont) const override;

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
		void CreateIntroHelpAnimation (IAnimatron **retpAnimatron);
		void CreateLongCreditsAnimation (int x, int y, int cyHeight, IAnimatron **retpAnimatron);
		void CreateNewsAnimation (CMultiverseNewsEntry *pEntry, IAnimatron **retpAnimatron);
		void CreatePlayerBarAnimation (IAnimatron **retpAni);
		void CreateScoreAnimation (const CGameRecord &Stats, IAnimatron **retpAnimatron);
		void CreateShipDescAnimation (CShip *pShip, IAnimatron **retpAnimatron);
		void CreateTitleAnimation (IAnimatron **retpAnimatron);
		DWORD GetIntroShipClass (void) { return m_dwIntroShipClass; }
		void DestroyAllIntroShips(void);
		void DestroyPOVIntroShips (void);
		void OnAccountChanged (const CMultiverseModel &Multiverse);
		void OnCommandIntro (const CString &sCmd, void *pData);
		void OnDblClickIntro (int x, int y, DWORD dwFlags);
		void OnLButtonDownIntro (int x, int y, DWORD dwFlags);
		void OnLButtonUpIntro (int x, int y, DWORD dwFlags);
		void OnMouseMoveIntro (int x, int y, DWORD dwFlags);
		void PaintDlgButton (const RECT &rcRect, const CString &sText);
		void SetAccountControls (const CMultiverseModel &Multiverse);
		void SetDebugOption (void);
		void SetMusicOption (void);
		ALERROR StartIntro (CIntroSession *pThis);
		void StopIntro (void);

		ALERROR StartGame (void);

		void ClearDebugLines (void);
		void ComputeScreenSize (void);
		void PaintDebugLines (void);
		void PaintFrameRate (void);
		void PaintLRS (void);
		void PaintMainScreenBorder (CG32bitPixel rgbColor);
		void PaintSnow (CG32bitImage &Dest, int x, int y, int cxWidth, int cyHeight);
		void PaintSRSSnow (void);
		void ReportCrash (void);
		void ReportCrashEvent (CString *retsMessage);
		void ReportCrashSystem (CString *retsMessage) const;
		void ShowErrorMessage (const CString &sError);

		void DoCommsMenu (int iIndex);
		void DoCommsSquadronMenu (const CString &sName, MessageTypes iOrder, DWORD dwData2);
		bool DoGameMenuCommand (DWORD dwCmd);
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

		inline void SetGameCreated (bool bValue = true) { m_bGameCreated = bValue; }
		inline bool IsGameCreated (void) { return m_bGameCreated; }

		inline CGameFile &GetGameFile (void);

		LONG WMChar (char chChar, DWORD dwKeyData);
		LONG WMClose (void);
		LONG WMCreate (CString *retsError);
		LONG WMDestroy (void);
		LONG WMKeyDown (int iVirtKey, DWORD dwKeyData);
		LONG WMKeyUp (int iVirtKey, DWORD dwKeyData);
		LONG WMLButtonDblClick (int x, int y, DWORD dwFlags);
		LONG WMLButtonDown (int x, int y, DWORD dwFlags);
		LONG WMLButtonUp (int x, int y, DWORD dwFlags);
		LONG WMMouseMove (int x, int y, DWORD dwFlags);
		LONG WMMove (int x, int y);
		LONG WMRButtonDown (int x, int y, DWORD dwFlags);
		LONG WMRButtonUp (int x, int y, DWORD dwFlags);
		LONG WMSize (int cxWidth, int cyHeight, int iSize);

		CTranscendenceController *m_pTC;

		//	General
		CString m_sVersion;
		CString m_sCopyright;

		//	Game
		GameState m_State;					//	Game state
		bool m_bAutopilot;					//	Autopilot is ON
		bool m_bPaused;						//	Game paused
		bool m_bPausedStep;					//	Step one frame
		char m_chKeyDown;					//	Processed a WM_KEYDOWN (skip WM_CHAR)
		bool m_bDockKeyDown;				//	Used to de-bounce dock key (so holding down 'D' does not select a dock action).
		int m_iTick;
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

		CDeviceCounterDisplay m_DeviceDisplay;	//	Device counter display
		CLRSDisplay m_LRSDisplay;			//	LRS display
		CMessageDisplay m_MessageDisplay;	//	Message display object
		CMenuDisplay m_MenuDisplay;			//	Menu display
		CPickerDisplay m_PickerDisplay;		//	Picker display

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

#include "GameSettings.h"

//	Transcendence data model class --------------------------------------------

struct STranscendenceSessionCtx
	{
	CHumanInterface *pHI = NULL;
	CTranscendenceModel *pModel = NULL;
	CGameSettings *pSettings = NULL;
	CCommandLineDisplay *pDebugConsole = NULL;
	CSoundtrackManager *pSoundtrack = NULL;
	};

class CTranscendencePlayer : public IPlayerController
	{
	public:
		CTranscendencePlayer (void);

		inline void SetPlayer (CPlayerShipController *pPlayer) { m_pPlayer = pPlayer; }

		//	IPlayerController interface

		virtual ICCItem *CreateGlobalRef (CCodeChain &CC) override { return CC.CreateInteger((int)m_pPlayer); }
        virtual CPlayerGameStats *GetGameStats (void) const override { return &m_pPlayer->GetGameStats(); }
		virtual GenomeTypes GetGenome (void) const override;
		virtual CString GetName (void) const override;
		virtual EUIMode GetUIMode (void) const override;
		virtual void OnMessageFromObj (CSpaceObject *pSender, const CString &sMessage) override;

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

		void OnPlayerChangedShips (CSpaceObject *pOldShip, CSpaceObject *pNewShip, SPlayerChangedShipsCtx &Options);
		void OnPlayerDestroyed (SDestroyCtx &Ctx, CString *retsEpitaph = NULL);
		void OnPlayerDocked (CSpaceObject *pObj);
		void OnPlayerEnteredGate (CTopologyNode *pDestNode, const CString &sDestEntryPoint, CSpaceObject *pStargate);
		void OnPlayerExitedGate (void);
		void OnPlayerTraveledThroughGate (void);
		inline CDockSession &GetDockSession (void) { return m_Universe.GetDockSession(); }
		inline const CDockSession &GetDockSession (void) const { return m_Universe.GetDockSession(); }
		inline ICCItem *GetScreenData (const CString &sAttrib) { return GetScreenStack().GetData(sAttrib); }
        inline CDockScreenStack &GetScreenStack (void) { return m_Universe.GetDockSession().GetFrameStack(); }
        inline const CDockScreenStack &GetScreenStack (void) const { return m_Universe.GetDockSession().GetFrameStack(); }
		ALERROR EndGame (void);
		ALERROR EndGame (const CString &sReason, const CString &sEpitaph, int iScoreChange = 0);
		ALERROR EndGameClose (CString *retsError = NULL);
		ALERROR EndGameDelete (CString *retsError = NULL);
		ALERROR EndGameDestroyed (bool *retbResurrected = NULL);
		ALERROR EndGameNoSave (CString * retsError = NULL);
		ALERROR EndGameSave (CString *retsError = NULL);
		ALERROR EndGameStargate (void);
		ALERROR EnterScreenSession (CSpaceObject *pLocation, CDesignType *pRoot, const CString &sScreen, const CString &sPane, ICCItem *pData, CString *retsError = NULL);
		void ExitScreenSession (bool bForceUndock = false);
		bool FindScreenRoot (const CString &sScreen, CDesignType **retpRoot, CString *retsScreen = NULL, ICCItemPtr *retpData = NULL);
		inline int GetLastHighScore (void) { return m_iLastHighScore; }
		const SFileVersionInfo &GetProgramVersion (void) const { return m_Version; }
		void GetScreenSession (SDockFrame *retFrame);
        inline CSystemMapThumbnails &GetSystemMapThumbnails (void) { return m_SystemMapThumbnails; }
		inline void IncScreenData (const CString &sAttrib, ICCItem *pData, ICCItem **retpResult = NULL) { GetScreenStack().IncData(sAttrib, pData, retpResult); }
		inline bool InScreenSession (void) const { return GetDockSession().InSession(); }
		bool IsGalacticMapAvailable (CString *retsError = NULL);
		void RecordFinalScore (const CString &sEpitaph, const CString &sEndGameReason, bool bEscaped);
		void RefreshScreenSession (void);
		bool ScreenTranslate (const CString &sID, ICCItem *pData, ICCItemPtr &pResult, CString *retsError = NULL) const;
		inline void SetScreenData (const CString &sAttrib, ICCItem *pData) { GetScreenStack().SetData(sAttrib, pData); }
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
		inline CMusicResource *GetCrawlSoundtrack (void) const { return m_pCrawlSoundtrack; }
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
		inline void SetCrawlSoundtrack (DWORD dwTrack) { m_pCrawlSoundtrack = g_pUniverse->FindMusicResource(dwTrack); }
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

		TArray<CString> CalcConditionsWhenDestroyed (SDestroyCtx &Ctx, CSpaceObject *pPlayerShip) const;
		CString CalcEpitaph (SDestroyCtx &Ctx);
		void CalcStartingPos (CShipClass *pStartingShip, DWORD *retdwMap, CString *retsNodeID, CString *retsPos);
		ALERROR CreateAllSystems (const CString &sStartNode, CSystem **retpStartingSystem, CString *retsError);
		void GenerateGameStats (CGameStats *retStats, bool bGameOver = false);
		inline CString GetSaveFilePath (void) const { return (m_SaveFileFolders.GetCount() == 0 ? NULL_STR : m_SaveFileFolders[0]); }
		ALERROR LoadGameStats (const CString &sFilespec, CGameStats *retStats);
		ALERROR LoadHighScoreList (CString *retsError = NULL);
		ALERROR LoadUniverse (const CString &sCollectionFolder, const TArray<CString> &ExtensionFolders, DWORD dwAdventure, const TArray<DWORD> &Extensions, const TSortMap<DWORD, bool> &DisabledExtensions, CString *retsError = NULL);
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
        bool m_bNoCollectionLoad;                   //  Do not load collection
		bool m_bForcePermadeath;					//	Replace resurrection with final stats

		CGameFile m_GameFile;
		CUniverse m_Universe;
		CPlayerShipController *m_pPlayer;

		CGameRecord m_GameRecord;					//	Most recent game record
		CGameStats m_GameStats;						//	Most recent game stats
		CHighScoreList m_HighScoreList;
		int m_iLastHighScore;						//	Index to last high-score

        //  Caches
        CSystemMapThumbnails m_SystemMapThumbnails;

		//	Temporaries
		CDesignType *m_pResurrectType;				//	DesignType that will handle resurrect (or NULL)
		CString m_sEndGameReason;					//	Reason for end game
		CString m_sEpitaph;							//	Epitaph
		int m_iScoreBonus;							//	Score bonus for completing the game
		CG32bitImage *m_pCrawlImage;				//	For epilogue/prologue
		CMusicResource *m_pCrawlSoundtrack;				//	For epilogue/prologue
		CString m_sCrawlText;						//	For epilogue/prologue

		//	Stargate temporaries
		CTopologyNode *m_pDestNode;					//	While player in gate
		CString m_sDestEntryPoint;					//	While player in gate
		CSystem *m_pOldSystem;						//	While player in gate
		CSystemEventList m_TimerEvents;				//	Timer events (temp while we gate)
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
				m_Model(m_HI),
				m_DebugConsole(m_HI.GetVisuals())
			{
			m_SessionCtx.pHI = &m_HI;
			m_SessionCtx.pModel = &m_Model;
			m_SessionCtx.pSettings = &m_Settings;
			m_SessionCtx.pDebugConsole = &m_DebugConsole;
			m_SessionCtx.pSoundtrack = &m_Soundtrack;
			}

		inline CCommandLineDisplay &GetDebugConsole (void) { return m_DebugConsole; }
        inline CGameSession *GetGameSession (void) { return m_pGameSession; }
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
		void InitDebugConsole (void);
		bool InstallUpgrade (CString *retsError);
		bool IsUpgradeReady (void);
		bool RequestCatalogDownload (const TArray<CMultiverseCatalogEntry> &Downloads);
		bool RequestResourceDownload (const TArray<CMultiverseFileRef> &Downloads);
		ALERROR WriteUpgradeFile (IMediaType *pData, CString *retsError);

		States m_iState = stateNone;
		BackgroundStates m_iBackgroundState = stateIdle;
		CTranscendenceModel m_Model;
		STranscendenceSessionCtx m_SessionCtx;

		CCloudService m_Service;
		CMultiverseModel m_Multiverse;
		CSoundtrackManager m_Soundtrack;
		bool m_bUpgradeDownloaded = false;

		CCommandLineDisplay m_DebugConsole;		//	CodeChain debugging console

		CGameSettings m_Settings;

        CGameSession *m_pGameSession = NULL;	//  Keep a pointer so we can call it directly.
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
#include "GameSession.h"

#endif
