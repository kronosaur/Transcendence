//	LL1.h
//
//	Library Link: Phase 1

#ifndef INCL_LL1
#define INCL_LL1

#ifndef INCL_CORE
#include "Core.h"
#endif

#ifndef INCL_FORMATTEDBLOCK
#include "FormattedBlock.h"
#endif

class CMainController;

#define OBJID_CPAGEVIEWER				MakeOBJCLASSID(1)
#define OBJID_CCOMMANDBAR				MakeOBJCLASSID(2)
#define OBJID_CCOMMANDLINE				MakeOBJCLASSID(3)
#define OBJID_CPAGEEDITOR				MakeOBJCLASSID(4)

class CCommandBar : public IUController,
					public CObject, 
					public IUFrameContent
	{
	public:
		enum IconTypes
			{
			InvokeLibrarian = 0,
			Index,
			NewEntry,
			EditEntry,
			Accept,
			Cancel,
			Contents,
			Quit,
			AllEntries,
			Home,
			Workbook,
			};

		typedef struct _MenuItemDesc
			{
			IconTypes iIcon;
			DWORD dwTag;
			ControllerNotifyProc pfActionMsg;
			} MenuItemDesc;

		CCommandBar (void);
		CCommandBar (IUController *pController);

		ALERROR SetMenuDesc (MenuItemDesc *pDesc, int iMenuSize);
		
		//	IUController virtuals

		virtual BOOL Notify (int iFromID, int iNotification, DWORD dwData, CObject *pFrom) { return FALSE; }

		//	IUFrame virtuals

		virtual ALERROR Activate (CUFrame *pFrame, RECT *pRect);
		virtual void Deactivate (void);
		virtual void Paint (HDC hDC, RECT *pUpdateRect);
		virtual void Resize (RECT *pRect);

	private:
		ALERROR InitResources (IGMediaDb *pMediaDb);
		void CleanUpResources (void);

		IUController *m_pController;
		CUFrame *m_pFrame;
		RECT m_rcRect;								//	Rect of the entire control
	};

class CCommandLine : public IUController,
					public CObject, 
					public IUFrameContent
	{
	public:
		CCommandLine (void);
		CCommandLine (IUController *pController);

		inline CString GetCommand (void) { return m_pEditor->GetData(); }
		inline void ClearCommand (void) { m_pEditor->SetData(LITERAL("")); }
		inline void SetActionMsg (ControllerNotifyProc pfMsg) { m_pfActionMsg = pfMsg; }

		//	IUController virtuals

		virtual BOOL Notify (int iFromID, int iNotification, DWORD dwData, CObject *pFrom) { return FALSE; }

		//	IUFrameContent virtuals

		virtual ALERROR Activate (CUFrame *pFrame, RECT *pRect);
		virtual void Deactivate (void);
		virtual void Paint (HDC hDC, RECT *pUpdateRect);
		virtual void Resize (RECT *pRect);
		virtual void SetFocus (void) { m_pEditor->SetFocus(); }

	private:
		int CmdAction (DWORD dwTag, DWORD dwDummy2);

		ALERROR InitResources (IGMediaDb *pMediaDb);
		void CleanUpResources (void);

		IUController *m_pController;
		CUFrame *m_pFrame;
		CULineEditor *m_pEditor;
		ControllerNotifyProc m_pfActionMsg;

		RECT m_rcRect;								//	Rect of the entire control
		int m_cxFieldWidth;							//	Width of field
	};

class CPageEditor : public IUController, public CObject, public IUFrameContent
	{
	public:
		CPageEditor (void);
		CPageEditor (IUController *pController);

		ALERROR GetPage (CCore *pCore, CEntry **retpEntry);
		ALERROR SetPage (CEntry *pEntry);
		inline void SetCommandProcessor (CCore *pCore) { m_pCore = pCore; }

		//	IUController virtuals

		virtual BOOL Notify (int iFromID, int iNotification, DWORD dwData, CObject *pFrom) { return FALSE; }

		//	IUFrame virtuals

		virtual ALERROR Activate (CUFrame *pFrame, RECT *pRect);
		virtual void Deactivate (void);
		virtual void Resize (RECT *pRect);

	private:
		void CalcMetrics (RECT &rcRect, RECT *retrcBody, RECT *retrcMeta);
		int CmdAction (DWORD dwTag, DWORD dwDummy2);

		IUController *m_pController;
		CUFrame *m_pFrame;
		RECT m_rcRect;								//	Rect of the entire control
		CCore *m_pCore;								//	Used for command execution (may be NULL)

		CULineEditor *m_pBody;						//	Body
		CULineEditor *m_pMeta;						//	Meta data
	};

class CPageViewer : public CObject, public IUFrameContent
	{
	public:
		CPageViewer (void);
		CPageViewer (IUController *pController);
		virtual ~CPageViewer (void);

		inline CString GetCurrentLink (void) { return m_pClickedOn->GetLinkInfo(); }
		inline void SetActionMsg (ControllerNotifyProc pfMsg) { m_pfActionMsg = pfMsg; }
		ALERROR SetPage (CEntry *pEntry);

		//	IUFrame virtuals

		virtual ALERROR Activate (CUFrame *pFrame, RECT *pRect);
		virtual void Animate (void);
		virtual BOOL ButtonDown (POINT ptPoint, DWORD dwFlags);
		virtual void ButtonUp (POINT ptPoint, DWORD dwFlags);
		virtual void Deactivate (void);
		virtual BOOL KeyDown (int iVirtKey, DWORD dwKeyFlags);
		virtual void MouseMove (POINT ptPoint, DWORD dwFlags);
		virtual void Paint (HDC hDC, RECT *pUpdateRect);
		virtual void Resize (RECT *pRect);

	private:
		ALERROR GenerateFieldSection (CEntry *pEntry, CString *retsFieldSection, int *retiFieldCount);
		void JustifyFrame (HDC hDC, CRTFrame *pFrame, Twips TwipsPerPixel, RECT &rcRect);
		void SetRect (RECT *pRect);
		void VertScroll (int iAmount);

		IUController *m_pController;
		CUFrame *m_pFrame;
		RECT m_rcRect;								//	Rect of the entire control
		RECT m_rcPage;								//	Rect of the full page
		RECT m_rcTextBlock;							//	Rect of the text portion
		RECT m_rcNotes;								//	Rect of the notes block
		RECT m_rcTitle;								//	Rect of the title region
		RECT m_rcBody;								//	Rect of the body region
		RECT m_rcFullBody;							//	Rect of entire body region
		RECT m_rcRelatedLinks;						//	Rect of the related links region
		Twips m_TwipsPerPixel;						//	Calculate twips per pixel

		DWORD m_fLandscape:1;						//	Landscape orientation
		DWORD m_fButtonDown:1;						//	TRUE if button down

		CEntry *m_pEntry;							//	Current entry
		CRTFormatter *m_pFormat;					//	Current global format template
		CRTFrame *m_pBody;							//	Formatted body
		CRTFrame *m_pTitle;							//	Formatted title
		CRTFrame *m_pFields;						//	Formatted fields
		CRTFrame *m_pRelatedLinks;					//	Formatted links

		CRTRun *m_pClickedOn;						//	Clicked on this run
		CRTFrame *m_pClickedOnFrame;				//	Clicked on this frame
		int m_cyScroll;								//	Body scroll position

		ControllerNotifyProc m_pfActionMsg;
	};

class CLibraryPage : public IUController
	{
	public:
		CLibraryPage (CMainController *pController);
		virtual ~CLibraryPage (void);

		ALERROR Boot (CUFrame *pFrame);
		ALERROR CloseCore (void);
		ALERROR DoneWithEdit (void);
		ALERROR EditPage (ENTRYUNID dwUNID);
		ALERROR NewPage (void);
		ALERROR OpenCore (CString sFilename);
		ALERROR ViewPage (CString sURL);
		ALERROR ViewPageByUNID (ENTRYUNID dwUNID);

		virtual BOOL Notify (int iFromID, int iNotification, DWORD dwData, CObject *pFrom) { return FALSE; }

		//	Commands

		int CmdCancelEdits (DWORD dwTag, DWORD dwDummy2);
		int CmdDoCommand (DWORD dwTag, DWORD dwDummy2);
		int CmdEditEntry (DWORD dwTag, DWORD dwDummy2);
		int CmdInvokeLibrarian (DWORD dwTag, DWORD dwDummy2);
		int CmdGotoContents (DWORD dwTag, DWORD dwDummy2);
		int CmdGotoEntry (DWORD dwTag, DWORD dwDummy2);
		int CmdGotoHome (DWORD dwTag, DWORD dwDummy2);
		int CmdGotoIndex (DWORD dwTag, DWORD dwDummy2);
		int CmdNewEntry (DWORD dwTag, DWORD dwDummy2);
		int CmdQuit (DWORD dwTag, DWORD dwDummy2);
		int CmdSaveEdits (DWORD dwTag, DWORD dwDummy2);

	private:
		ALERROR CreateEditor (void);
		ALERROR DisplayError (ALERROR error) { return ErrorSetDisplayed(error); }
		void HideLibrarian (void);
		ALERROR ShowLibrarian (void);

		CMainController *m_pController;
		CCore *m_pCore;
		CUFrame *m_pFrame;
		CCommandBar *m_pCmdBar;
		CPageViewer *m_pViewer;
		CPageEditor *m_pEditor;
		CCommandLine *m_pCmdLine;

		CEntry *m_pEntry;							//	Entry being viewed
		
		ENTRYUNID m_EditUNID;						//	UNID being edited
		DWORD m_fEditMode:1;						//	TRUE if in edit mode
		DWORD m_fNewEntry:1;						//	TRUE if we're editing a new entry
	};

class CMainController : public IUController
	{
	public:
		CMainController (void);

		inline CUApplication *GetApp (void) { return m_pApp; }

		//	Messages

		int CmdQueryClose (DWORD dwTag, DWORD dwDummy2);
		int msgDoCommand (DWORD dwDummy, DWORD dwDummy2);
		int msgTimer (DWORD dwDummy, DWORD dwLastTickCount);

		//	Virtuals

		virtual ALERROR Boot (CUApplication *pApp);
		virtual BOOL Notify (int iFromID, int iNotification, DWORD dwData, CObject *pFrom);

	private:
		CUApplication *m_pApp;
		CUWindow *m_pWindow;

		CLibraryPage *m_pMainPage;
	};

#endif