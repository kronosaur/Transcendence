//	Graphics.h
//
//	This module contains primitive graphics routines below the level
//	of the GUI.

#ifndef INCL_GRAPHICS
#define INCL_GRAPHICS

class IGMediaDb;

//	Raw image format structure definitions

typedef DWORD RAWPIXEL;

//	CGBitmap: A wrapper for a Windows bitmap reference

class CGBitmap : public CObject
	{
	public:
		CGBitmap (void);
		CGBitmap (IGMediaDb *pMediaDb, 
			      DWORD dwUNID,
				  DWORD dwMaskUNID,
				  int x, int y, int cxWidth, int cyHeight);
		~CGBitmap (void);

		void Blt (HDC hDC, int x, int y, RECT *pUpdateRect);
		ALERROR Load (void);
		void PartialBlt (HDC hDC, int x, int y, int cxWidth, int cyHeight, int xOffset, int yOffset, RECT *pUpdateRect);
		ALERROR SetBitmap (IGMediaDb *pMediaDb,
			      DWORD dwUNID,
				  DWORD dwMaskUNID,
				  int x, int y, int cxWidth, int cyHeight);
		void Unload (void);

	private:
		void CleanUpMask (HBITMAP hBitmap, HBITMAP hMask);
		void MaskedBlt (HDC hDestDC,
						  int x,
						  int y,
						  int cxWidth,
						  int cyHeight,
						  HDC hBitmapDC,
						  HBITMAP hBitmask,
						  int xOffset,
						  int yOffset);

		IGMediaDb *m_pMediaDb;
		DWORD m_dwUNID;
		DWORD m_dwMaskUNID;
		HBITMAP m_hBitmap;
		HBITMAP m_hBitmask;
		RECT m_rcRect;
	};

//	CGFont: A wrapper for a Windows font

class CGFont : public CObject
	{
	public:
		CGFont (void);
		CGFont (const CGFont &NewFont);
		virtual ~CGFont (void);

		HFONT GetFont (void);

		inline void SetBold (BOOL bBold) { FlushFont(); m_fBold = (DWORD)bBold; }
		inline void SetHeightInPixels (int iHeight) { FlushFont(); m_iHeight = iHeight; }
		void SetHeightInPoints (int iTenthPoints);
		inline void SetItalic (BOOL bItalic) { FlushFont(); m_fItalic = (DWORD)bItalic; }
		inline void SetTypeface (CString sTypeface) { FlushFont(); m_sTypeface = sTypeface; }

	private:
		void FlushFont (void);

		HFONT m_hFont;
		int m_cyHeight;
		int m_cxWidth;

		DWORD m_fBold:1;
		DWORD m_fItalic:1;
		DWORD m_fUseCellHeight:1;
		int m_iHeight;
		CString m_sTypeface;
	};

//	Media database classes

class IGMediaDb
	{
	public:
		virtual void DeselectBitmap (void) = 0;
		virtual ALERROR GetBitmap (DWORD dwUNID, HBITMAP *rethBitmap) = 0;
		virtual ALERROR LoadBitmap (DWORD dwUNID, HBITMAP *rethBitmap) = 0;
		virtual void ReleaseBitmap (DWORD dwUNID) = 0;
		virtual HDC SelectBitmap (HBITMAP hBitmap) = 0;
	};

class CGResourceFile : public CObject, public IGMediaDb
	{
	public:
		CGResourceFile (void);
		CGResourceFile (HINSTANCE hModule);
		virtual ~CGResourceFile (void);

		//	IGMediaDb virtuals

		virtual void DeselectBitmap (void);
		virtual ALERROR GetBitmap (DWORD dwUNID, HBITMAP *rethBitmap);
		virtual ALERROR LoadBitmap (DWORD dwUNID, HBITMAP *rethBitmap);
		virtual void ReleaseBitmap (DWORD dwUNID);
		virtual HDC SelectBitmap (HBITMAP hBitmap);

	private:
		typedef struct
			{
			DWORD dwUNID;
			HBITMAP hBitmap;
			int iRefCount;
			} CACHEENTRY;

		CACHEENTRY *FindInCache (DWORD dwUNID);

		HINSTANCE m_hModule;
		CStructArray m_BitmapCache;

		HDC m_hBitmapDC;
		HBITMAP m_hCurBitmap;
		HBITMAP m_hOldBitmap;
	};

//	Inlines

inline int rawRed (RAWPIXEL Pixel) { return ((Pixel & 0xFF000000) >> 24); }
inline int rawGreen (RAWPIXEL Pixel) { return ((Pixel & 0x00FF0000) >> 16); }
inline int rawBlue (RAWPIXEL Pixel) { return ((Pixel & 0x0000FF00) >> 8); }
inline int rawAlpha (RAWPIXEL Pixel) { return (Pixel & 0x000000FF); }
inline RAWPIXEL rawPixel (int iRed, int iGreen, int iBlue, int iAlpha)
	{ return (RAWPIXEL)iAlpha | ((RAWPIXEL)iBlue << 8) | ((RAWPIXEL)iGreen << 16) | ((RAWPIXEL)iRed << 24); }

//	DIB functions

enum EBitmapTypes
	{
	bitmapNone,

	bitmapMonochrome,
	bitmapAlpha,
	bitmapRGB,
	};

ALERROR dibConvertToDDB (HBITMAP hDIB, HPALETTE hPalette, HBITMAP *rethBitmap);
ALERROR dibCreate16bitDIB (int cxWidth, int cyHeight, HBITMAP *rethBitmap, WORD **retpPixel);
ALERROR dibCreate24bitDIB (int cxWidth, int cyHeight, DWORD dwFlags, HBITMAP *rethBitmap, BYTE **retpPixel);
ALERROR dibCreate32bitDIB (int cxWidth, int cyHeight, DWORD dwFlags, HBITMAP *rethBitmap, DWORD **retpPixel);
ALERROR dibGetInfo (HBITMAP hDIB, int *retcxWidth, int *retcyHeight, void **retpBase, int *retiStride);
bool dibIs16bit (HBITMAP hDIB);
bool dibIs24bit (HBITMAP hDIB);
ALERROR dibLoadFromBlock (IReadBlock &Data, HBITMAP *rethDIB, EBitmapTypes *retiType = NULL);
ALERROR dibLoadFromFile (CString sFilename, HBITMAP *rethDIB, EBitmapTypes *retiType = NULL);
ALERROR dibLoadFromResource (HINSTANCE hInst, char *szResource, HBITMAP *rethDIB, EBitmapTypes *retiType = NULL);
DWORD dibPaletteSize (LPVOID pv);

//	Raw image format functions

ALERROR rawConvertTo16bitDIB (int cxWidth, int cyHeight, RAWPIXEL *pRaw, DWORD dwFlags, HBITMAP *rethBitmap);
ALERROR rawConvertToDDB (int cxWidth, int cyHeight, RAWPIXEL *pRaw, DWORD dwFlags, HPALETTE hPalette, HBITMAP *rethBitmap);

//	GDI functions

void gdiFillRect (HDC hDC, const RECT *pRect, COLORREF Color);

//	Region functions

ALERROR rgnCreateFromBitmap (HBITMAP hBitmap, HRGN *rethRgn);

//	Miscellaneous functions

COLORREF strToCOLORREF(const CString &sColor);

#endif
