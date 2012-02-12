//	Flatland.h
//
//	Defines for Flatland GUI
//
//	Usage:
//
//		1.	Create an XML flatland descriptor file and include it
//			in your application resource as RCDATA 'FLATLAND'.
//		2.	Create an application class that derives from IFlatlandEvents.
//		3.	Use the DEFINE_FLATLAND_APP to specify the app class.

#ifndef INCL_FLATLAND
#define INCL_FLATLAND

#ifndef INCL_XMLUTIL
#include "XMLUtil.h"
#endif

class CFCode;
class CFlatland;
class CFView;
class CFWindow;

class IFlatlandEvents
	{
	public:
		virtual void OnShutdown (void) { }
		virtual ALERROR OnStartup (void) { return NOERROR; }
	};

class IFController
	{
	public:
		virtual void OnEvent (CFView *pView, int iID, int iEvent, DWORD dwData1, const CString &sData2) { }
		virtual void OnViewLoad (CFView *pView, int iID) { }
		virtual void OnViewUnload (CFView *pView, int iID) { }
	};

typedef ALERROR (*CodeCreatorProc)(CFCode **retpCode);
typedef ALERROR (*ViewCreatorProc)(CFView **retpView);

//	Miscellaneous UI objects

class CFTextStyle
	{
	public:
		ALERROR Boot (CXMLElement *pDesc);
		void Draw (HDC hDC, const CString &sText, const RECT &rcRect);

	private:
		CGFont m_Font;
		COLORREF m_rgbColor;
	};

//	CFlatland object

class CFlatland
	{
	public:
		//	Initialization methods
		static ALERROR Boot (HINSTANCE hInstance, int nCmdShow, CString *retsError);
		static void Shutdown (void);

		//	CFWindow methods
		ALERROR CreateNewWindow (const CString &sTemplate,
								 int iID, 
								 IFController *pController,
								 CFWindow **retpWindow);
		void DestroyWindow (CFWindow *pWindow);

		//	CFView methods
		ALERROR CreateNewCode (CXMLElement *pDesc, CFView *pView, CFCode **retpCode);
		ALERROR CreateNewView (CXMLElement *pDesc, 
							   CFWindow *pWindow, 
							   IFController *pController, 
							   CFView **retpView);
		CXMLElement *GetTemplate (const CString &sTemplate);
		ALERROR RegisterCode (const CString &sName, CodeCreatorProc fProc);
		ALERROR RegisterView (const CString &sName, ViewCreatorProc fProc);

		//	CFTextStyle methods
		inline int GetTextStyleCount (void) { return m_TextStyles.GetCount(); }
		inline CFTextStyle *GetTextStyleByIndex (int iIndex) { return (CFTextStyle *)m_TextStyles.GetValue(iIndex); }
		CFTextStyle *FindTextStyle (const CString &sName);

		//	Miscellaneous methods
		void DisplayErrorMessage (const CString &sMsg);
		bool FindColor (const CString &sColor, COLORREF *retpRGB);
		inline IFlatlandEvents *GetApp (void) { return m_pApp; }
		inline HINSTANCE GetInstance (void) { return m_hAppInst; }

	private:
		CFlatland (void);
		ALERROR LoadDescriptor (CXMLElement *pDesc, CString *retsError);
		ALERROR StandardError (ALERROR error, CString *retsError);

		IFlatlandEvents *m_pApp;
		HINSTANCE m_hAppInst;
		CSymbolTable m_Templates;
		CObjectArray m_WindowList;
		CSymbolTable m_Views;
		CSymbolTable m_TextStyles;
		CSymbolTable m_Code;

		int m_iMainWindowShow;
	};

class CFWindow
	{
	public:
		CFWindow (void);
		~CFWindow (void);

		ALERROR Boot (CXMLElement *pDesc, int iID, IFController *pController);
		void CaptureMouse (CFView *pView);
		void Close (void) { ::PostMessage(m_hWnd, WM_CLOSE, 0, 0); }
		ALERROR CreateChildWindow (CFView *pView,
									char *pszClass,
									DWORD dwStyle,
									DWORD dwStyleEx,
									int iID,
									HWND *rethWnd);
		inline CFView *FindView (int iID);
		inline HWND GetHWND (void) { return m_hWnd; }
		inline int GetID (void) { return m_iID; }
		inline void RaiseExternalEvent (int iID, int iEvent, DWORD dwData) { SendEvent(iID, iEvent, dwData, CONSTLIT("")); }
		inline void RaiseExternalEvent (int iID, int iEvent, DWORD dwData1, const CString &sData2) { SendEvent(iID, iEvent, dwData1, sData2); }
		void ReleaseMouseCapture (void);
		void SetView (CFView *pView);
		void SetView (const CString &sTemplate);
		void UpdateView (const RECT &rcRect);

	private:
		void SendEvent (int iID, int iEvent, DWORD dwData1, const CString &sData2);
		LONG WMDestroy (void);
		LONG WMLButtonDown (int x, int y, DWORD dwFlags);
		LONG WMLButtonUp (int x, int y, DWORD dwFlags);
		LONG WMMouseMove (int x, int y, DWORD dwFlags);
		LONG WMPaint (void);
		LONG WMSize (WORD dwSizeType, int cxWidth, int cyHeight);

		HWND m_hWnd;
		int m_iID;
		IFController *m_pController;
		CFView *m_pView;
		CFView *m_pMouseCapture;

	friend LONG APIENTRY CFWindowWndProc (HWND hWnd, UINT message, UINT wParam, LONG lParam);
	};

class CFViewMetrics
	{
	public:
		CFViewMetrics (void);

		void CalcRect (const RECT &rcParent, RECT *retrcView);
		ALERROR InitFromXML (CXMLElement *pItem);

	private:
		enum Flags
			{
			MeasureLeftFromRightEdge =		0x00000001,
			MeasureRightFromRightEdge =		0x00000002,
			MeasureTopFromBottomEdge =		0x00000004,
			MeasureBottomFromBottomEdge =	0x00000008,
			CenteredVertically =			0x00000010,
			CenteredHorizontally =			0x00000020,
			};

		inline int CalcMetric (int iParentMin, int iParentMax, DWORD dwFlag, int iMetric)
			{
			if (m_dwFlags & dwFlag)
				return iParentMax - iMetric;
			else
				return iParentMin + iMetric;
			}

		bool LoadValue (CXMLElement *pItem,
						const CString &sAttribute,
						long *retMetric,
					    DWORD Flag);

		RECT m_rcRect;
		DWORD m_dwFlags;
	};

class CFView
	{
	public:
		typedef LPVOID Iterator;

		CFView (void);
		virtual ~CFView (void);

		void Activate (void);
		ALERROR Boot (CXMLElement *pDesc, CFWindow *pWindow, IFController *pController);
		void Deactivate (void);
		CFView *FindSubView (int iID);
		inline IFController *GetController (void) { return m_pController; }
		inline Iterator GetFirstSubView (void) { return m_pFirstChild; }
		inline CFViewMetrics *GetFrame (void) { return m_pFrame; }
		inline int GetID (void) { return m_iID; }
		inline Iterator GetNextSubView (Iterator SubView) { return ((CFView *)SubView)->m_pNextSibling; }
		inline const RECT &GetRect (void) { return m_rcRect; }
		inline CFView *GetSubView (Iterator SubView) { return (CFView *)SubView; }
		inline CFWindow *GetWindow (void) { return m_pWindow; }
		inline ALERROR InitMetricsFromXML (CXMLElement *pDesc) { return m_Metrics.InitFromXML(pDesc); }
		bool LButtonDown (const POINT &pt, DWORD dwFlags);
		void LButtonUp (const POINT &pt, DWORD dwFlags);
		void MouseMove (const POINT &pt, DWORD dwFlags);
		void Paint (HDC hDC, const RECT &rcUpdateRect);
		inline void SetController (IFController *pController) { m_pController = pController; }
		inline void SetID (int iID) { m_iID = iID; }
		inline void SetFrame (CFViewMetrics *pFrame) { m_pFrame = pFrame; }
		void SetRect (const RECT &rcRect);
		void SetWindow (CFWindow *pWindow);
		inline void UpdateView (const RECT &rcRect) { m_pWindow->UpdateView(rcRect); }

		inline DWORD Message (int iMsg, DWORD dwData) { return Message(iMsg, dwData, CONSTLIT("")); }
		virtual DWORD Message (int iMsg, DWORD dwData1, const CString &sData2) { return 0; }

	protected:
		virtual void OnActivate (void) { }
		virtual void OnCreate (CXMLElement *pDesc) { }
		virtual void OnDeactivate (void) { }
		virtual bool OnLButtonDown (const POINT &pt, DWORD dwFlags) { return false; }
		virtual void OnLButtonUp (const POINT &pt, DWORD dwFlags) { }
		virtual void OnMouseMove (const POINT &pt, DWORD dwFlags) { }
		virtual void OnPaint (HDC hDC, const RECT &rcUpdateRect) { }
		virtual void OnSize (void) { }

		void AddSubView (CFView *pView);
		inline void RaiseEvent (int iEvent, DWORD dwData) { m_pController->OnEvent(this, m_iID, iEvent, dwData, CONSTLIT("")); }
		inline void RaiseEvent (int iEvent, DWORD dwData1, const CString &sData2) { m_pController->OnEvent(this, m_iID, iEvent, dwData1, sData2); }

	private:
		CFWindow *m_pWindow;						//	Our parent window
		IFController *m_pController;				//	Our controller
		int m_iID;									//	ID to identify ourselves to controller

		CFViewMetrics *m_pFrame;					//	Frame for this view (may be NULL)
		CFViewMetrics m_Metrics;					//	Metrics for this view (relative to frame)
		RECT m_rcRect;								//	Current rect relative to window

		CFView *m_pParent;							//	Parent view
		CFView *m_pFirstChild;						//	First child view
		CFView *m_pLastChild;						//	Last child view
		CFView *m_pNextSibling;						//	Next sibling view

		bool m_bLButtonIsDown;						//	TRUE if lbutton is down in view
	};

class CFContainerView : public CFView
	{
	public:
		CFContainerView (void);
		virtual ~CFContainerView (void);

		static ALERROR Create (CFView **retpView);

		virtual DWORD Message (int iMsg, DWORD dwData1, const CString &sData2);

	protected:
		virtual void OnCreate (CXMLElement *pDesc);

	private:
		CFCode *m_pCode;							//	Code to implement this view

		int m_iFrameCount;							//	Number of sub frames
		CFViewMetrics *m_pFrames;					//	Array of sub frames
	};

class CFButtonView : public CFView
	{
	public:
		enum Events
			{
			OnCommand = 1000
			};

		CFButtonView (void);

		static ALERROR Create (CFView **retpView);

	protected:
		virtual void OnCreate (CXMLElement *pDesc);
		virtual bool OnLButtonDown (const POINT &pt, DWORD dwFlags);
		virtual void OnLButtonUp (const POINT &pt, DWORD dwFlags);
		virtual void OnMouseMove (const POINT &pt, DWORD dwFlags);
		virtual void OnPaint (HDC hDC, const RECT &rcUpdateRect);

	private:
		bool m_bMouseOver;							//	TRUE if mouse is over button
		bool m_bButtonDown;							//	TRUE if button is down
	};

class CFImageView : public CFView
	{
	public:
		CFImageView (void);

		static ALERROR Create (CFView **retpView);

	protected:
		virtual void OnCreate (CXMLElement *pDesc);
		virtual void OnPaint (HDC hDC, const RECT &rcUpdateRect);

	private:
		COLORREF m_rgbBackground;
	};

class CFTextView : public CFView
	{
	public:
		enum Messages
			{
			GetText =	1000,
			SetText =	1001,
			};

		CFTextView (void);

		static ALERROR Create (CFView **retpView);

		virtual DWORD Message (int iMsg, DWORD dwData1, const CString &sData2);

	protected:
		virtual void OnCreate (CXMLElement *pDesc);
		virtual void OnPaint (HDC hDC, const RECT &rcUpdateRect);

	private:
		CString m_sText;
		CFTextStyle *m_pDefStyle;
	};

class CFTextEditView : public CFView
	{
	public:
		enum Messages
			{
			GetText =	1000,
			SetText =	1001,
			};

		CFTextEditView (void);

		static ALERROR Create (CFView **retpView);

		virtual DWORD Message (int iMsg, DWORD dwData1, const CString &sData2);

	protected:
		virtual void OnActivate (void);
		virtual void OnCreate (CXMLElement *pDesc);
		virtual void OnDeactivate (void);
		virtual void OnSize (void);

	private:
		HWND m_hCtrl;
		bool m_bSingleLine;
		bool m_bMSWinUI;
	};

class CFCode : public IFController
	{
	public:
		CFCode (void) : m_pView(NULL) { }
		virtual ~CFCode (void) { }

		inline CFView *GetView (void) { return m_pView; }
		inline void SetView (CFView *pView) { m_pView = pView; }

		virtual DWORD Message (int iMsg, DWORD dwData1, const CString &sData2) { return 0; }

	protected:
		inline void RaiseEvent (int iEvent, DWORD dwData) { m_pView->GetController()->OnEvent(m_pView, m_pView->GetID(), iEvent, dwData, CONSTLIT("")); }
		inline void RaiseEvent (int iEvent, DWORD dwData1, const CString &sData2) { m_pView->GetController()->OnEvent(m_pView, m_pView->GetID(), iEvent, dwData1, sData2); }

	private:
		CFView *m_pView;
	};

extern CFlatland *g_pFlatland;

//	Application must use this macro (in a CPP file) to specify
//	a class that will handle Flatland events.

#define DEFINE_FLATLAND_APP(myclass)				\
	IFlatlandEvents *FLCreateAppObject (void)		\
		{											\
		return new myclass;							\
		}											\
													\
	void FLDestroyAppObject (IFlatlandEvents *pApp)	\
		{											\
		delete (myclass *)(pApp);					\
		}

//	Some inlines

inline CFView *CFWindow::FindView (int iID) { return m_pView->FindSubView(iID); }

#endif