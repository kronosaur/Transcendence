//	WindowsExt.h
//
//	Djinni extensions that provide a windowed user interface

#ifndef INCL_WINDOWSEXT
#define INCL_WINDOWSEXT

class CLWindow;

class CLSizeOptions
	{
	public:
		enum AxisOptions
			{
			BothFixed,						//	.left and .right are margins
			FixedLower,						//	.left is margin; .right is width
			FixedUpper,						//	.right is margin; .left is width
			};

		CLSizeOptions (void);
		void CalcAxis (AxisOptions iOption, int iPL, int iPU, int iL, int iU, LONG *retiL, LONG *retiU);
		void CalcSize (RECT *pParent, RECT *retrcRect);
		static BOOL ConvertItemToRect (ICCItem *pRect, RECT *retrcRect);
		BOOL ReadFromItem (ICCItem *pRect, ICCItem *pOptions);

		RECT rcRect;						//	Rect
		AxisOptions iHorzOptions;			//	Horizontal options
		AxisOptions iVertOptions;			//	Vertical options
	};

class CLWindowOptions
	{
	public:
		DWORD fBackstore:1;					//	TRUE if using off-screen bmp for painting
	};

class CLArea : public CObject
	{
	public:
		CLArea (void);
		CLArea (IObjectClass *pClass, CLWindow *pWindow, int iID, ICCItem *pController);

		//	CLArea external interface

		inline void ClearModified (void) { m_fModified = FALSE; }
		ICCItem *Create (CLSizeOptions *pSize);
		inline int GetID (void) { return m_iID; }
		inline void GetRect (RECT *retrcRect) { *retrcRect = m_rcRect; }
		inline BOOL IsHidden (void) { return m_fHidden; }
		inline BOOL IsModified (void) { return m_fModified; }
		inline BOOL IsTransparent (void) { return IsTransparentQuery(); }
		inline void Paint (HDC hDC, RECT *pUpdate) { PaintNeeded(hDC, pUpdate); }
		inline void SetModified (void) { m_fModified = TRUE; }
		void SetSize (CLSizeOptions *pOptions);
		void ShowArea (void);
		void WindowSize (RECT *pRect);

		//	Virtuals

		virtual ICCItem *GetData (void);
		virtual ICCItem *SetData (ICCItem *pData);

	protected:

		//	CLArea internal interface

		CCodeChain *GetCC (void);
		CTerraFirma *GetTF (void);

		//	Notifications that may be overridden

		virtual ICCItem *CreateNotify (void) { return GetCC()->CreateTrue(); }
		virtual void DataChanged (void) { }
		virtual BOOL IsTransparentQuery (void) { return FALSE; }
		virtual void PaintNeeded (HDC hDC, RECT *pUpdate) { }
		virtual void SizeChanged (void) { }

		CLWindow *m_pWindow;				//	Parent window
		RECT m_rcRect;						//	Current size of area relative to parent
		CLSizeOptions m_SizeOptions;		//	Autosize options
		ICCItem *m_pController;				//	Area controller (may be NULL)
		int m_iID;							//	ID (Unique for all areas of Window)

		DWORD m_fHidden:1;					//	TRUE if area is hidden
		DWORD m_fDisabled:1;				//	TRUE if area is disabled
		DWORD m_fReadOnly:1;				//	TRUE if area cannot be edited
		DWORD m_fModified:1;				//	TRUE if the area has been modified
		DWORD m_fFirstShow:1;				//	TRUE if area has been shown already
	};

class CLButtonArea : public CLArea
	{
	public:
		CLButtonArea (CLWindow *pWindow, int iID, ICCItem *pController);
	};

class CLImageArea : public CLArea
	{
	public:
		CLImageArea (void);
		CLImageArea (CLWindow *pWindow, int iID, ICCItem *pController);
		virtual ~CLImageArea (void);

		//	CLArea virtuals

		virtual ICCItem *GetData (void);
		virtual ICCItem *SetData (ICCItem *pData);

	protected:

		//	CLArea notifications

		virtual BOOL IsTransparentQuery (void) { return TRUE; }
		virtual void PaintNeeded (HDC hDC, RECT *pUpdate);

		ICCItem *m_pData;					//	Data for the area (may be NULL)
		HBITMAP m_hImage;					//	Device-dependent bitmap
		RECT m_rcImage;						//	Size of image
	};

class CLWindow : public ICCAtom
	{
	public:
		CLWindow (void);
		CLWindow (CLWindowManager *pWM);

		ICCItem *CreateItem (CCodeChain *pCC, CLWindowOptions *pOptions, CLSizeOptions *pSize, ICCItem *pController);

		//	Interface

		int AddArea (CLArea *pArea);
		void BringAreaToFront (CLArea *pArea);
		void DestroyArea (CLArea *pArea);
		CLArea *FindArea (int iID);
		inline void GetRect (RECT *retrcRect) { ::GetClientRect(m_hWnd, retrcRect); }
		inline HDC GetTempDC (void) { return m_hdcBitmap; }
		CLWindowManager *GetWM (void);
		inline BOOL HasBackstore (void) { return m_fBackstore; }
		void HideWindow (void);
		inline BOOL IsVisible (void) { return ::IsWindowVisible(m_hWnd); }
		void SendAreaToBack (CLArea *pArea);
		void SetSize (CLSizeOptions *pOptions);
		void ShowWindow (void);
		void UpdateRect (RECT *pUpdateRect);

		//	ICCItem virtuals

		virtual ICCItem *Clone (CCodeChain *pCC);
		virtual ValueTypes GetValueType (void) { return Complex; }
		virtual BOOL IsIdentifier (void) { return FALSE; }
		virtual BOOL IsFunction (void) { return FALSE; }
		virtual BOOL IsSymbolTable (void) { return TRUE; }
		virtual CString Print (CCodeChain *pCC);
		virtual void Reset (void) { }

	protected:
		virtual void DestroyItem (CCodeChain *pCC);
		virtual ICCItem *StreamItem (CCodeChain *pCC, IWriteStream *pStream);
		virtual ICCItem *UnstreamItem (CCodeChain *pCC, IReadStream *pStream);

	private:
		HRGN ClipRect (HDC hDC, RECT *pRect);
		ALERROR CreateBackstore (int cxWidth, int cyHeight);
		ICCItem *CreateHWND (CCodeChain *pCC, CLWindowOptions *pOptions, RECT *pRect);
		inline CLArea *GetArea (int iArea) { return (CLArea *)m_AreaList.GetObject(iArea); }
		inline int GetAreaCount (void) { return m_AreaList.GetCount(); }
		void PaintRect (HDC hDC, RECT *pUpdateRect);
		void UnclipRect (HDC hDC, HRGN hOldClipRgn);
		LONG WMPaint (void);
		LONG WMSize (int cxWidth, int cyHeight, int iSizeType);

		CLWindowManager *m_pWM;				//	Pointer to window manager
		HWND m_hWnd;						//	HWND
		ICCItem *m_pController;				//	Controller for window
		CLSizeOptions m_SizeOptions;		//	Autosize of window

		CObjectArray m_AreaList;			//	List of areas

		DWORD m_fBackstore:1;				//	TRUE if using off-screen bmp for painting

		HPALETTE m_hPalette;				//	Palette
		HBITMAP m_hBackstore;				//	Backstore BMP
		HDC m_hdcBackstore;					//	Backstore DC
		HDC m_hdcBitmap;					//	Temporary DC (for bltting bitmaps)

	friend LONG APIENTRY CLWindowWndProc (HWND hWnd, UINT message, UINT wParam, LONG lParam);
	};

class CLWindowManager
	{
	public:
		CLWindowManager (CTerraFirma *pTF);

		ICCItem *CreateLWindow (CLWindowOptions *pOptions, CLSizeOptions *pSize, ICCItem *pController);
		void DestroyLWindow (CLWindow *pWindow);
		void GetRect (RECT *retrcRect);
		void WindowSize (RECT *pRect);

		inline CTerraFirma *GetTF (void) { return m_pTF; }
		inline CCodeChain *GetCC (void) { return m_pCC; }

	private:
		inline int FindWindow (CLWindow *pWindow) { return m_WindowList.FindObject(pWindow); }

		CTerraFirma *m_pTF;
		CCodeChain *m_pCC;

		CObjectArray m_WindowList;
	};

#endif
