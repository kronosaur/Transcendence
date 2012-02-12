//	ALGUI.h
//
//	GUI include file. This file must be included after Alchemy.h
//
//	Usage:
//
//	1. Create a controller class descended from IUController.
//	2. Implement Boot method and have it create a window.
//	3. Implement ALGUIMainController function that returns an
//		allocated instance of the controller class.

#ifndef INCL_ALGUI
#define INCL_ALGUI

class CUApplication;
class CUFrame;
class CUWindow;

class IUController
	{
	public:
		virtual ~IUController (void) { }
		virtual ALERROR Boot (CUApplication *pApp) { return ERR_FAIL; }
		virtual ALERROR DisplayError (ALERROR error) { return error; }
		virtual BOOL Notify (int iFromID, int iNotification, DWORD dwData, CObject *pFrom) = 0;
	};

typedef int (IUController::*ControllerNotifyProc)(DWORD dwTag, DWORD dwData);

inline int CallNotifyProc (IUController *pController, ControllerNotifyProc pfMsg, DWORD dwTag, DWORD dwData)
	{
	if (pfMsg)
		return (pController->*pfMsg)(dwTag, dwData);
	else
		return 0;
	}

class IUFrameContent
	{
	public:
		virtual ~IUFrameContent (void) { }

		virtual ALERROR Activate (CUFrame *pFrame, RECT *pRect) { return NOERROR; }
		virtual void Animate (void) { }
		virtual BOOL ButtonDown (POINT ptPoint, DWORD dwFlags) { return FALSE; }
		virtual void ButtonUp (POINT ptPoint, DWORD dwFlags) { }
		virtual void Deactivate (void) { }
		virtual BOOL KeyDown (int iVirtKey, DWORD dwKeyData) { return FALSE; }
		virtual void KeyUp (int iVirtKey, DWORD dwKeyData) { }
		virtual void MouseMove (POINT ptPoint, DWORD dwFlags) { }
		virtual void Paint (HDC hDC, RECT *pUpdateRect) { }
		virtual void Resize (RECT *pRect) { }
		virtual void SetFocus (void) { }
	};

class AutoSizeDesc
	{
	public:
		enum AlignmentTypes
			{
			MinVariable,
			MaxVariable,
			DimVariable,
			Centered,
			};

		AutoSizeDesc (void);

		void CalcRect (RECT *pParentRect, RECT *retrcRect);

		void SetAutoSize (int cxWidth, int cyHeight,
				int iLeftMargin, int iTopMargin, int iRightMargin, int iBottomMargin,
				AlignmentTypes iHorzAlignment, AlignmentTypes iVertAlignment);
		void SetCentered (int cxWidth, int cyHeight);
		void SetFull (void);
		void SetFixed (int x, int y, int cxWidth, int cyHeight);
		void SetBottomPanel (int cyHeight, int xLeftMargin, int xRightMargin);
		void SetLeftPanel (int cxWidth, int yTopMargin, int yBottomMargin);
		void SetTopPanel (int cyHeight, int xLeftMargin, int xRightMargin);

	private:
		void CalcAxis (int iParentMin, 
							 int iParentMax, 
							 int iMinBoundary,
							 int iMaxBoundary,
							 int iDim,
							 AlignmentTypes Alignment,
							 LONG *retiMin, 
							 LONG *retiMax);

		RECT m_rcBoundaries;
		int m_cxWidth;
		int m_cyHeight;
		AlignmentTypes m_HorzAlignment;
		AlignmentTypes m_VertAlignment;
	};

#define FRAME_STATE_OWNEDCONTENT			0x00000001	//	Content object is owned by frame

class CUFrame : public CObject
	{
	public:
		CUFrame (void);
		CUFrame (CUWindow *pWindow, CUFrame *pParent, IUController *pController, int iID, DWORD dwFlags);
		virtual ~CUFrame (void);

		void Animate (void);
		HRGN BeginClip (HDC hDC, RECT &rcRect);
		BOOL ButtonDown (POINT ptPoint, DWORD dwFlags);
		void ButtonUp (POINT ptPoint, DWORD dwFlags);
		void CalcChildRect (CUFrame *pChild, AutoSizeDesc *pDesc, RECT *retrcRect);
		void CaptureMouse (void);
		void DestroyAllChildFrames (void);
		BOOL DestroyFrame (CUFrame *pFrame);
		void EndClip (HDC hDC, HRGN hOldClip);
		CUFrame *FindFrame (int iID);
		inline void GetRect (RECT *retrcRect) { *retrcRect = m_rcRect; }
		inline CUWindow *GetWindow (void) { return m_pWindow; }
		BOOL HasMouseCapture (void);
		void KeyDown (int iVirtKey, DWORD dwKeyData);
		void KeyUp (int iVirtKey, DWORD dwKeyData);
		void MouseMove (POINT ptPoint, DWORD dwFlags);
		void Paint (HDC hDC, RECT *pUpdateRect);
		void ReleaseCapture (void);
		void Resize (void);
		void SetAutoSize (AutoSizeDesc *pDesc);
		inline void SetBackgroundColor (COLORREF rgbColor) { m_rgbBackground = rgbColor; }
		ALERROR SetContent (IUFrameContent *pContent, BOOL bOwned);
		void SetFocus (void);
		void UpdateRect (RECT *pRect);

	private:
		void ReleaseContent (void);

		CUWindow *m_pWindow;
		IUController *m_pController;
		int m_iID;
		DWORD m_dwFlags;
		DWORD m_dwState;

		CUFrame *m_pParent;								//	Parent frame (may be NULL)
		CUFrame *m_pNextSibling;						//	Next sibling in chain
		CUFrame *m_pFirstChild;							//	First child
		RECT m_rcRect;									//	Rect relative to window
		AutoSizeDesc m_AutoSize;						//	AutoSize description
		COLORREF m_rgbBackground;						//	Background color

		IUFrameContent *m_pContent;						//	object that paints and clicks
	};

#define WINDOW_FLAG_ANIMATE					0x00000001	//	Animation on
#define WINDOW_FLAG_FULLSCREEN				0x00000002	//	Cover entire screen

class CUWindow : public CObject
	{
	public:
		CUWindow (void);
		CUWindow (CUApplication *pApp, IUController *pController, int iID, DWORD dwFlags);
		virtual ~CUWindow (void);

		void Animate (void);
		ALERROR Boot (void);
		void CaptureMouse (CUFrame *pFrame);
		ALERROR CreateChildWindow (CUFrame *pFrame,
					char *pszClass,
					DWORD dwStyle,
					DWORD dwStyleEx,
					int iID,
					HWND *rethWnd);
		ALERROR CreateFrame (CUFrame *pParent, int iID, DWORD dwFlags, AutoSizeDesc *pDesc, CUFrame **retpFrame);
		void DestroyFrame (CUFrame *pFrame);
		inline CUFrame *FindFrame (int iID) { return m_pFrame->FindFrame(iID); }
		inline CUApplication *GetApp (void) { return m_pApp; }
		inline HWND GetHWND (void) { return m_hWnd; }
		inline CUFrame *GetMouseCapture (void) { return m_pMouseCapture; }
		inline void GetRect (RECT *retrcRect) { ::GetClientRect(m_hWnd, retrcRect); }
		void Hide (void);
		inline BOOL IsFullScreen (void) { return (m_dwFlags & WINDOW_FLAG_FULLSCREEN ? TRUE : FALSE); }
		inline BOOL NeedsAnimation (void) { return (m_dwFlags & WINDOW_FLAG_ANIMATE ? TRUE : FALSE); }
		void ReleaseCapture (void);
		void SetInputFocus (CUFrame *pFrame);
		void Resize (int x, int y, int cxWidth, int cyHeight);
		void Show (void);
		void ShowMainWindow (void);
		void SetTitle (CString sString);
		void UpdateRect (RECT *pRect);

		inline void SetQueryCloseMsg (ControllerNotifyProc pfMsg) { m_pfQueryCloseMsg = pfMsg; }

		//	Create some standard controls

		ALERROR CreateBitmapButton (IUController *pController,
									CUFrame *pParent, 
									AutoSizeDesc *pDesc,
									IGMediaDb *pMediaDb,
									DWORD dwUNID,
									DWORD dwMaskUNID,
									int x,
									int y,
									int cxWidth,
									int cyHeight,
									DWORD dwTag,
									ControllerNotifyProc pfMsg);

	private:
		LONG WMButtonDown (int x, int y, DWORD dwFlags);
		LONG WMButtonUp (int x, int y, DWORD dwFlags);
		LONG WMChar (TCHAR chChar, DWORD dwKeyData);
		LONG WMClose (void);
		LONG WMDestroy (void);
		LONG WMKeyDown (int iVirtKey, DWORD dwKeyData);
		LONG WMKeyUp (int iVirtKey, DWORD dwKeyData);
		LONG WMMouseMove (int x, int y, DWORD dwFlags);
		LONG WMPaint (void);
		LONG WMSize (DWORD dwSizeType, int cxWidth, int cyHeight);

		CUApplication *m_pApp;
		IUController *m_pController;
		int m_iID;
		DWORD m_dwFlags;

		HWND m_hWnd;
		DWORD m_dwState;
		CUFrame *m_pFrame;
		CUFrame *m_pMouseCapture;
		CUFrame *m_pInputFocus;

		ControllerNotifyProc m_pfQueryCloseMsg;

	friend LONG APIENTRY CUWindowWndProc (HWND hWnd, UINT message, UINT wParam, LONG lParam);
	};

class CUApplication : public CObject
	{
	public:
		CUApplication (void);
		CUApplication (HINSTANCE hInstance, LPSTR lpCmdLine, int nCmdShow);
		virtual ~CUApplication (void);

		ALERROR CreateNewWindow (IUController *pController, int iID, DWORD dwFlags, CUWindow **retpWindow);
		void DestroyWindow (CUWindow *pWindow);
		inline CString GetExecutablePath (void) { return pathGetExecutablePath(m_hInstance); }
		inline int GetInitialShow (void) { return m_iInitialShow; }
		inline int GetFrameDelay (void) { return m_iFrameDelay; }
		inline LPSTR GetIconRes (void) { return m_pszIconRes; }
		inline HINSTANCE GetInstance (void) { return m_hInstance; }
		inline IGMediaDb *GetResourcesDb (void) { return &m_Resources; }
		void Quit (void);
		ALERROR Run (IUController *pController, DWORD dwFlags);
		inline void SetFrameDelay (int iFrameDelay) { m_iFrameDelay = iFrameDelay; }
		inline void SetIconRes (LPSTR pszIconRes) { m_pszIconRes = pszIconRes; }

		//	Controller messages

		inline void SetTimerMsg (ControllerNotifyProc pfMsg) { m_pfTimerMsg = pfMsg; }

	private:
		inline CUWindow *GetMainWindow (void) { return (CUWindow *)m_WindowList.GetObject(0); }

		HINSTANCE m_hInstance;
		CString m_sCommandLine;
		int m_iInitialShow;
		CGResourceFile m_Resources;
		LPSTR m_pszIconRes;

		CObjectArray m_WindowList;
		int m_iFrameDelay;

		ControllerNotifyProc m_pfTimerMsg;
	};

//	Some built-in content objects

class CULineEditor : public CObject, public IUFrameContent
	{
	public:
		CULineEditor (void);
		CULineEditor (IUController *pController);

		//void GetCaretPos (int *retiLine, int *retiOffset, int *retiPos, int *retiSelLen);
		CString GetData (void);
		CString GetLine (int iLine);
		CString GetSelection (void);
		void GotoEndOfLine (void);
		BOOL HasSelection (void);
		void InsertText (CString sText);
		void SetData (CString sData);
		inline void SetFont (CGFont Font) { m_Font = Font; }
		inline void SetMSWinUI (void) { m_fMSWinUI = TRUE; }
		inline void SetSingleLine (void) { m_fSingleLine = TRUE; }
		inline void SetTabStops (int iTabs) { m_iTabs = iTabs; }

		//	Controller messages

		inline void SetActionMsg (ControllerNotifyProc pfMsg) { m_pfActionMsg = pfMsg; }

		//	IUFrame virtuals

		virtual ALERROR Activate (CUFrame *pFrame, RECT *pRect);
		virtual void Deactivate (void);
		virtual void Resize (RECT *pRect);
		virtual void SetFocus (void);

	private:
		ALERROR ExecuteCommand (void);

		LONG WMChar (WPARAM wParam, LPARAM lParam);
		LONG WMKeyDown (WPARAM wParam, LPARAM lParam);

		IUController *m_pController;
		HWND m_hEditWnd;
		CGFont m_Font;
		int m_iTabs;

		DWORD m_fSingleLine:1;
		DWORD m_fMSWinUI:1;

		ControllerNotifyProc m_pfActionMsg;

	friend LONG APIENTRY LineEditorWndProc (HWND hWnd, UINT message, UINT wParam, LONG lParam);
	};

class CUButton : public CObject, public IUFrameContent
	{
	public:
		enum ButtonTypes
			{
			Normal,
			Bitmap,
			};

		CUButton (void);
		CUButton (IUController *pController);

		inline ALERROR SetDownImage (IGMediaDb *pMediaDb, DWORD dwUNID, DWORD dwMaskUNID,
					int x, int y, int cxWidth, int cyHeight)
				{ return m_ButtonDownImage.SetBitmap(pMediaDb, dwUNID, dwMaskUNID, x, y, cxWidth, cyHeight); }
		inline ALERROR SetHoverImage (IGMediaDb *pMediaDb, DWORD dwUNID, DWORD dwMaskUNID,
					int x, int y, int cxWidth, int cyHeight)
				{ return m_HoverImage.SetBitmap(pMediaDb, dwUNID, dwMaskUNID, x, y, cxWidth, cyHeight); }
		inline ALERROR SetUpImage (IGMediaDb *pMediaDb, DWORD dwUNID, DWORD dwMaskUNID,
					int x, int y, int cxWidth, int cyHeight)
				{ return m_ButtonUpImage.SetBitmap(pMediaDb, dwUNID, dwMaskUNID, x, y, cxWidth, cyHeight); }

		inline void SetActionMsg (ControllerNotifyProc pfMsg) { m_pfActionMsg = pfMsg; }
		ALERROR SetImages (IGMediaDb *pMediaDb, DWORD dwUNID, DWORD dwMaskUNID, int x, int y, int cxWidth, int cyHeight);
		inline void SetTag (DWORD dwTag) { m_dwTag = dwTag; }
		void SetType (ButtonTypes iType);

		//	IUFrameContent virtuals

		virtual ALERROR Activate (CUFrame *pFrame, RECT *pRect);
		virtual void Animate (void);
		virtual BOOL ButtonDown (POINT ptPoint, DWORD dwFlags);
		virtual void ButtonUp (POINT ptPoint, DWORD dwFlags);
		virtual void Deactivate (void);
		virtual void MouseMove (POINT ptPoint, DWORD dwFlags);
		virtual void Paint (HDC hDC, RECT *pUpdateRect);
		virtual void Resize (RECT *pRect);

	private:
		IUController *m_pController;
		CUFrame *m_pFrame;
		RECT m_rcRect;

		ButtonTypes m_iType;
		CString m_sCaption;
		CGBitmap m_ButtonUpImage;
		CGBitmap m_ButtonDownImage;
		CGBitmap m_HoverImage;
		DWORD m_dwTag;
		ControllerNotifyProc m_pfActionMsg;

		DWORD m_fButtonDown:1;
		DWORD m_fSelected:1;
		DWORD m_fMouseOver:1;

		int m_iBlinks;
	};

//	Some inlines

inline void CUFrame::CaptureMouse (void) { m_pWindow->CaptureMouse(this); }
inline BOOL CUFrame::HasMouseCapture (void) { return m_pWindow->GetMouseCapture() == this; }
inline void CUFrame::ReleaseCapture (void) { m_pWindow->ReleaseCapture(); }
inline void CUFrame::UpdateRect (RECT *pRect) 
	{ 
	if (pRect)
		m_pWindow->UpdateRect(pRect); 
	else 
		m_pWindow->UpdateRect(&m_rcRect); 
	}

//	This function must be defined by the user. The function should return
//	a controller that implements the Boot method. The Boot method should,
//	at minimum, create a window.

IUController *ALGUIMainController (void);

//	Console routines

CString consoleInput (CString sPrompt);
void consolePrint (CString sLine, ...);

#endif
