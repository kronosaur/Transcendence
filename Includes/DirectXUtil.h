//	DirectXUtil.h
//
//	DirectX Utilities
//
//	We require:
//
//		windows.h
//		ddraw.h
//		Alchemy.h

#ifndef INCL_DIRECTXUTIL
#define INCL_DIRECTXUTIL

#ifndef __DDRAW_INCLUDED__
#include "ddraw.h"
#endif

#ifndef __DSOUND_INCLUDED__
#include "dsound.h"
#endif

#ifndef _INC_VFW
#include "vfw.h"
#endif

#ifndef INCL_EUCLID
#include "Euclid.h"
#endif

#define BLT_RANGE_CHECK

#define ALPHA_BITS				8
#define ALPHA_LEVELS			256

const int MAX_COORD =			1000000;
const int MIN_COORD =			-1000000;

//	We pick a color that is unlikely to be confused
//	(e.g., black is very common, so we can't use that).

const WORD DEFAULT_TRANSPARENT_COLOR = 0xfc1f;

typedef double RealPixelChannel;

class CG16bitFont;
class CG16bitImage;

enum AlignmentStyles
	{
	alignLeft =				0x00000001,
	alignCenter =			0x00000002,
	alignRight =			0x00000004,
	alignTop =				0x00000008,
	alignMiddle =			0x00000010,
	alignBottom =			0x00000020,
	};

struct SPoint
	{
	int x;
	int y;
	};

inline int PointsToPixels (int iPoints, int iDPI = 96) { return ::MulDiv(iPoints, iDPI, 72); }

struct SColorHSB
	{
	SColorHSB (void)
		{ }

	SColorHSB (double rHueParam, double rSaturationParam, double rBrightnessParam) :
			rHue(rHueParam),
			rSaturation(rSaturationParam),
			rBrightness(rBrightnessParam)
		{ }

	double rHue;			//	0-360
	double rSaturation;		//	0-1
	double rBrightness;		//	0-1
	};

struct SColorRGB
	{
	SColorRGB (void)
		{ }

	SColorRGB (double rRedParam, double rGreenParam, double rBlueParam) :
			rRed(rRedParam),
			rGreen(rGreenParam),
			rBlue(rBlueParam)
		{ }

	double rRed;			//	0-1
	double rGreen;			//	0-1
	double rBlue;			//	0-1
	};

class CG16bitPixel
	{
	public:
		static COLORREF Blend (COLORREF rgbFrom, COLORREF rgbTo, double rUnitRange);
		static COLORREF Blend (COLORREF rgbNegative, COLORREF rgbCenter, COLORREF rgbPositive, double rDoubleRange);
		static COLORREF Desaturate (COLORREF rgbColor);
		static inline DWORD GetBlue8bit (WORD wPixel) { return GetBlue5bit(wPixel) << 3; }
		static inline DWORD GetGreen8bit (WORD wPixel) { return GetGreen6bit(wPixel) << 2; }
		static inline DWORD GetRed8bit (WORD wPixel) { return GetRed5bit(wPixel) << 3; }
		static SColorRGB HSBToRGB (const SColorHSB &hsbColor);
		static inline COLORREF PixelToRGB (WORD wPixel) { return RGB(GetRed8bit(wPixel), GetGreen8bit(wPixel), GetBlue8bit(wPixel)); }
		static inline SColorRGB PixelToRGBReal (WORD wPixel) { return RGBToRGBReal(PixelToRGB(wPixel)); }
		static SColorHSB RGBToHSB (const SColorRGB &rgbColor);
		static inline SColorHSB RGBToHSB (WORD wPixel) { return RGBToHSB(PixelToRGBReal(wPixel)); }
		static inline WORD RGBToPixel (DWORD dwRed8bit, DWORD dwGreen8bit, DWORD dwBlue8bit) { return (WORD)((dwBlue8bit >> 3) | ((dwGreen8bit >> 2) << 5) | ((dwRed8bit >> 3) << 11)); }
		static inline WORD RGBToPixel (COLORREF rgbColor) { return RGBToPixel(GetRValue(rgbColor), GetGValue(rgbColor), GetBValue(rgbColor)); }
		static COLORREF RGBRealToRGB (const SColorRGB &rgbColor);
		static SColorRGB RGBToRGBReal (COLORREF rgbColor);

	private:
		static inline WORD GetBlue5bit (WORD wPixel) { return (wPixel & 0x1f); }
		static inline WORD GetGreen6bit (WORD wPixel) { return (wPixel & 0x7e0) >> 5; }
		static inline WORD GetRed5bit (WORD wPixel) { return (wPixel & 0xf800) >> 11; }

		WORD m_wPixel;
	};

class CG16bitSprite
	{
	public:
		CG16bitSprite (void);
		~CG16bitSprite (void);

		ALERROR CreateFromImage (const CG16bitImage &Source);
		void ColorTransBlt (CG16bitImage &Dest, int xDest, int yDest, int xSrc, int ySrc, int cxWidth, int cyHeight);
		inline int GetHeight (void) const { return m_cyHeight; }
		inline int GetWidth (void) const { return m_cxWidth; }

	private:
		enum SpriteCodes
			{
			codeSkip =	0x0001,
			codeRun =	0x0002,
			};

		void DeleteData (void);

		int m_cxWidth;
		int m_cyHeight;
		WORD *m_pCode;
		WORD **m_pLineIndex;
	};

class CG16bitImage : public CObject
	{
	public:
		enum Flags
			{
			cfbPreMultAlpha =		0x00000001,
			cfbDesaturateAlpha =	0x00000002,
			};

		enum MarkerTypes
			{
			markerPixel,
			markerSmallRound,
			markerSmallSquare,
			markerSmallCross,
			markerMediumCross,
			markerSmallFilledSquare,
			markerSmallCircle,
			markerTinyCircle,
			};

		enum SurfaceTypes
			{
			r5g5b5,
			r5g6b5,
			r8g8b8,

			stUnknown,
			};

		struct SDrawLineCtx
			{
			CG16bitImage *pDest;

			int x1;					//	Start
			int y1;
			int x2;					//	End
			int y2;

			int x;					//	Current
			int y;

			int dx;					//	delta x
			int ax;					//	2 * abs(dx)
			int sx;					//	sign(dx)

			int dy;					//	delta y
			int ay;					//	2 * abs(dy)
			int sy;					//	sign(dy)

			int d;					//	leftover

			inline bool IsXDominant(void) { return (ax > ay); }
			};

		typedef void (*DRAWLINEPROC)(SDrawLineCtx *pCtx);

		CG16bitImage (void);
		virtual ~CG16bitImage (void);

		bool AdjustCoords (int *xSrc, int *ySrc, int cxSrc, int cySrc,
						   int *xDest, int *yDest,
						   int *cxWidth, int *cyHeight) const;
		bool AdjustScaledCoords (Metric *xSrc, Metric *ySrc, int cxSrc, int cySrc,
								 Metric xSrcInc, Metric ySrcInc,
								 int *xDest, int *yDest,
								 int *cxDest, int *cyDest);
		int AdjustTextX (const CG16bitFont &Font, const CString &sText, AlignmentStyles iAlign, int x);
		void AssociateSurface (LPDIRECTDRAW7 pDD);
		void ConvertToSprite (void);
		ALERROR CreateBlank (int cxWidth, int cyHeight, bool bAlphaMask, WORD wInitColor = 0, BYTE byInitAlpha = 0xff);
		ALERROR CreateBlankAlpha (int cxWidth, int cyHeight);
		ALERROR CreateBlankAlpha (int cxWidth, int cyHeight, BYTE byOpacity);
		ALERROR CreateFromBitmap (HBITMAP hBitmap, HBITMAP hBitmask = NULL, DWORD dwFlags = 0);
		ALERROR CreateFromFile (const CString &sImageFilespec, const CString &sMaskFilespec = NULL_STR, DWORD dwFlags = 0);
		ALERROR CreateFromImage (const CG16bitImage &Image);
		ALERROR CreateFromImageTransformed (const CG16bitImage &Source,
											int xSrc, 
											int ySrc, 
											int cxSrc, 
											int cySrc,
											Metric rScaleX, 
											Metric rScaleY, 
											Metric rRotation);
		inline void Destroy (void) { DeleteData(); }
		void DiscardSurface (void);
		void SetBlending (WORD wAlpha);
		void SetTransparentColor (WORD wColor = DEFAULT_TRANSPARENT_COLOR);

		void Blt (int xSrc, int ySrc, int cxWidth, int cyHeight, const CG16bitImage &Source, int xDest, int yDest);
		void BltGray (int xSrc, int ySrc, int cxWidth, int cyHeight, int iAlpha, const CG16bitImage &Source, int xDest, int yDest);
		void BltLighten (int xSrc, int ySrc, int cxWidth, int cyHeight, int iAlpha, const CG16bitImage &Source, int xDest, int yDest);
		void BltToDC (HDC hDC, int x, int y);
		void BltToSurface (LPDIRECTDRAWSURFACE7 pSurface, SurfaceTypes iType);
		void BltToSurface (LPDIRECTDRAWSURFACE7 pSurface, SurfaceTypes iType, const RECT &rcDest);
		void BltWithMask (int xSrc, int ySrc, int cxWidth, int cyHeight, const CG16bitImage &Mask, const CG16bitImage &Source, int xDest, int yDest);
		void ColorTransBlt (int xSrc, int ySrc, int cxWidth, int cyHeight, DWORD dwOpacity, const CG16bitImage &Source, int xDest, int yDest);
		void CompositeTransBlt (int xSrc, int ySrc, int cxWidth, int cyHeight, DWORD dwOpacity, const CG16bitImage &Source, int xDest, int yDest);
		void CopyAlpha (int xSrc, int ySrc, int cxWidth, int cyHeight, const CG16bitImage &Soruce, int xDest, int yDest);
		ALERROR CopyToClipboard (void);
		void ClearMaskBlt (int xSrc, int ySrc, int cxWidth, int cyHeight, const CG16bitImage &Source, int xDest, int yDest, WORD wColor = DEFAULT_TRANSPARENT_COLOR);
		void DrawDot (int x, int y, WORD wColor, MarkerTypes iMarker);
		inline void DrawLine (int x1, int y1, int x2, int y2, int iWidth, WORD wColor) { BresenhamLineAA(x1, y1, x2, y2, iWidth, wColor); }
		inline void DrawLineTrans (int x1, int y1, int x2, int y2, int iWidth, WORD wColor, DWORD dwOpacity) { BresenhamLineAATrans(x1, y1, x2, y2, iWidth, wColor, dwOpacity); }
		void DrawLineProc (SDrawLineCtx *pCtx, DRAWLINEPROC pfProc);
		void DrawLineProcInit (int x1, int y1, int x2, int y2, SDrawLineCtx *pCtx);
		inline void DrawBiColorLine (int x1, int y1, int x2, int y2, int iWidth, WORD wColor1, WORD wColor2) { BresenhamLineAAFade(x1, y1, x2, y2, iWidth, wColor1, wColor2); }
		inline void DrawPixel (int x, int y, WORD wColor) { if (x >= m_rcClip.left && y >= m_rcClip.top && x < m_rcClip.right && y < m_rcClip.bottom) *GetPixel(GetRowStart(y), x) = wColor; }
		inline void DrawPixelTrans (int x, int y, WORD wColor, BYTE byTrans)
				{
				if (x >= m_rcClip.left && y >= m_rcClip.top && x < m_rcClip.right && y < m_rcClip.bottom) 
					{
					WORD *pPos = GetPixel(GetRowStart(y), x);
					*pPos = BlendPixel(*pPos, wColor, byTrans);
					}
				}
		static inline void DrawPixelTrans (WORD *pPos, WORD wColor, DWORD byTrans)
				{
				*pPos = BlendPixel(*pPos, wColor, byTrans);
				}
		inline void DrawPlainLine (int x1, int y1, int x2, int y2, WORD wColor) { BresenhamLine(x1, y1, x2, y2, wColor); }
		inline void DrawText (int x, int y, const CG16bitFont &Font, WORD wColor, CString sText, DWORD dwFlags = 0, int *retx = NULL);
		void Fill (int x, int y, int cxWidth, int cyHeight, WORD wColor);
		void FillRGB (int x, int y, int cxWidth, int cyHeight, COLORREF rgbValue);
		void FillColumn (int x, int y, int cyHeight, WORD wColor);
		void FillColumnTrans (int x, int y, int cyHeight, WORD wColor, DWORD byOpacity);
		void FillLine (int x, int y, int cxWidth, WORD wColor);
		void FillLineGray (int x, int y, int cxWidth, WORD wColor, DWORD byOpacity);
		void FillLineTrans (int x, int y, int cxWidth, WORD wColor, DWORD byOpacity);
		void FillMask (int xSrc, int ySrc, int cxWidth, int cyHeight, const CG16bitImage &Source, WORD wColor, int xDest, int yDest, BYTE byOpacity = 0xff);
		void FillTrans (int x, int y, int cxWidth, int cyHeight, WORD wColor, DWORD byOpacity);
		void FillTransGray (int x, int y, int cxWidth, int cyHeight, WORD wColor, DWORD byOpacity);
		void FillTransRGB (int x, int y, int cxWidth, int cyHeight, COLORREF rgbValue, int iAlpha);
		inline BYTE *GetAlphaRow (int iRow) const { return (BYTE *)(m_pAlpha + (iRow * m_iAlphaRowSize)); }
		inline BYTE *GetAlphaValue (int x, int y) const { return ((BYTE *)(m_pAlpha + y * m_iAlphaRowSize)) + x; }
		inline WORD GetBackColor (void) const { return m_wBackColor; }
		inline const RECT &GetClipRect (void) const { return m_rcClip; }
		inline int GetHeight (void) const { return m_cyHeight; }
		inline WORD *GetPixel (WORD *pRowStart, int x) const { return pRowStart + x; }
		WORD GetPixelAlpha (int x, int y);
		inline WORD *GetRowStart (int y) const { return (WORD *)(m_pRGB + y * m_iRGBRowSize); }
		inline LPDIRECTDRAWSURFACE7 GetSurface (void) const { return m_pSurface; }
		static SurfaceTypes GetSurfaceType (LPDIRECTDRAWSURFACE7 pSurface);
		inline int GetWidth (void) const { return m_cxWidth; }
		inline bool HasAlpha (void) const { return (m_pAlpha != NULL); }
		inline bool HasMask (void) const { return m_bHasMask; }
		inline bool HasRGB (void) const { return (m_pRGB != NULL); }
		void IntersectMask (int xMask, int yMask, int cxMask, int cyMask, const CG16bitImage &Mask, int xDest, int yDest);
		inline bool IsEmpty (void) const { return (m_cxWidth == 0 || m_cyHeight == 0); }
		inline bool IsSprite (void) const { return m_pSprite != NULL; }
		inline bool IsTransparent (void) const { return m_pRedAlphaTable != NULL; }
		void MaskedBlt (int xSrc, int ySrc, int cxWidth, int cyHeight, CG16bitImage &Source, int xDest, int yDest);
		inline BYTE *NextAlphaRow (BYTE *pAlpha) const { return (BYTE *)(((DWORD *)pAlpha) + m_iAlphaRowSize); }
		inline WORD *NextRow (WORD *pRow) const { return pRow + (m_iRGBRowSize * 2); }
		ALERROR ReadFromStream (IReadStream *pStream);
		void ResetClipRect (void);
		ALERROR SaveAsWindowsBMP (const CString &sFilespec);
		void SetClipRect (const RECT &rcClip);
		void SetPixelTrans (int x, int y, WORD wColor, DWORD byOpacity);
		void SwapBuffers (CG16bitImage &Other);
		void TransBlt (int xSrc, int ySrc, int cxWidth, int cyHeight, int iAlpha, CG16bitImage &Source, int xDest, int yDest);
		void WriteToStream (IWriteStream *pStream);
		void WriteToWindowsBMP (IWriteStream *pStream);

		static inline BYTE BlendAlpha (BYTE dwOpacity1, BYTE dwOpacity2) { return (BYTE)255 - (BYTE)(((DWORD)(255 - dwOpacity1) * (DWORD)(255 - dwOpacity2)) / 255); }
		static WORD BlendPixel (WORD pxDest, WORD pxSource, DWORD byOpacity);
		static WORD BlendPixelGray (WORD pxDest, WORD pxSource, DWORD byOpacity);
		static WORD BlendPixelPM (DWORD pxDest, DWORD pxSource, DWORD byOpacity);
		static WORD FadeColor (WORD wStart, WORD wEnd, int iFade);
		static inline WORD DarkenPixel (DWORD pxSource, DWORD byOpacity) { return BlendPixel(RGBValue(0,0,0), (WORD)pxSource, byOpacity); }
		static inline WORD DesaturateValue (WORD wColor) { return ((GreenValue(wColor) * 59) + (RedValue(wColor) * 30) + (BlueValue(wColor) * 11)) / 255; }
		static inline WORD LightenPixel (DWORD pxSource, DWORD byOpacity) { return BlendPixel(RGBValue(255,255,255), (WORD)pxSource, byOpacity); }
		static inline bool IsGrayscaleValue (WORD wColor) { return ((BlueValue(wColor) == GreenValue(wColor)) && (GreenValue(wColor) == RedValue(wColor))); }
		static inline WORD GrayscaleValue (WORD wValue) { return ((wValue << 8) & 0xf800) | ((wValue << 3) & 0x7c0) | (wValue >> 3); }
		static inline WORD RGBValue (WORD wRed, WORD wGreen, WORD wBlue) { return ((wRed << 8) & 0xf800) | ((wGreen << 3) & 0x7e0) | (wBlue >> 3); }
		static inline WORD BlueValue (WORD wColor) { return GetBlueValue(wColor) << 3; }
		static inline WORD GreenValue (WORD wColor) { return GetGreenValue(wColor) << 2; }
		static inline WORD RedValue (WORD wColor) { return GetRedValue(wColor) << 3; }
		static inline DWORD PixelFromRGB (COLORREF rgb) { return (GetBValue(rgb) >> 3) | ((GetGValue(rgb) >> 2) << 5) | ((GetRValue(rgb) >> 3) << 11); }
		static inline COLORREF RGBFromPixel (WORD wColor) { return RGB(RedValue(wColor), GreenValue(wColor), BlueValue(wColor)); }

	private:
		struct RealPixel
			{
			RealPixelChannel rRed;
			RealPixelChannel rGreen;
			RealPixelChannel rBlue;
			};

		void BresenhamLine (int x1, int y1, 
						    int x2, int y2,
						    WORD wColor);
		void BresenhamLineAA (int x1, int y1, 
							  int x2, int y2,
							  int iWidth,
							  WORD wColor);
		void BresenhamLineAAFade (int x1, int y1, 
								  int x2, int y2,
								  int iWidth,
								  WORD wColor1,
								  WORD wColor2);
		void BresenhamLineAATrans (int x1, int y1, 
							  int x2, int y2,
							  int iWidth,
							  WORD wColor,
							  DWORD dwOpacity);
		void WuLine (int x0, int y0, 
				     int x1, int y1,
				     int iWidth,
					 WORD wColor1);

		void DeleteData (void);
		inline DWORD DoublePixelFromRGB (COLORREF rgb) { return PixelFromRGB(rgb) | (PixelFromRGB(rgb) << 16); }
		inline DWORD *GetPixelDW (DWORD *pRowStart, int x, bool *retbOdd) const { *retbOdd = ((x % 2) == 1); return pRowStart + (x / 2); }
		RealPixel GetRealPixel (const RECT &rcRange, Metric rX, Metric rY, bool *retbBlack = NULL);
		static inline WORD GetBlueValue (WORD wPixel) { return (wPixel & 0x1f); }
		static inline WORD GetGreenValue (WORD wPixel) { return (wPixel & 0x7e0) >> 5; }
		static inline WORD GetRedValue (WORD wPixel) { return (wPixel & 0xf800) >> 11; }
		inline DWORD *GetRowStartDW (int y) const { return m_pRGB + y * m_iRGBRowSize; }
		inline DWORD HighPixelFromRGB (COLORREF rgb) const { return PixelFromRGB(rgb) << 16; }
		inline bool InClipX (int x) const { return (x >= m_rcClip.left && x < m_rcClip.right); }
		inline bool InClipY (int y) const { return (y >= m_rcClip.top && y < m_rcClip.bottom); }
		void InitBMI (BITMAPINFO **retpbi);
		inline DWORD LowPixelFromRGB (COLORREF rgb) const { return PixelFromRGB(rgb); }
		inline WORD MakePixel (WORD wRed, WORD wGreen, WORD wBlue) { return (wRed << 11) | (wGreen << 5) | (wBlue); }
		inline void SetLowPixel (DWORD *pPos, DWORD dwValue) { *pPos = ((*pPos) & 0xFFFF0000) | dwValue; }
		inline void SetHighPixel (DWORD *pPos, DWORD dwValue) { *pPos = ((*pPos) & 0xFFFF) | dwValue; }
		void SetRealPixel (Metric rX, Metric rY, const RealPixel &Value, bool bNotBlack = false);

		int m_cxWidth;				//	Width of the image in pixels
		int m_cyHeight;				//	Height of the image in pixels
		int m_iRGBRowSize;			//	Number of DWORDs in an image row
		int m_iAlphaRowSize;		//	Number of DWORDs in an alpha mask row

		//	The image is structured as an array of 16-bit pixels. Each row
		//	in the image is aligned on a DWORD boundary. The image starts with
		//	the upper left corner. Each pixel is a 5-6-5 RGB pixel.
		DWORD *m_pRGB;

		//	The alpha mask is an array of 8-bit mask values. Each row is
		//	aligned on a DWORD boundary. A value of 0 masks out the image
		//	at that location. A value from 1 to 255 blends the image with
		//	the background.
		DWORD *m_pAlpha;

		WORD m_wBackColor;			//	Back color is transparent
		bool m_bHasMask;			//	TRUE if image has m_pAlpha (or used to have m_pAlpha, but was optimized)

		//	We calculate these transparency tables when using blending
		WORD *m_pRedAlphaTable;
		WORD *m_pGreenAlphaTable;
		WORD *m_pBlueAlphaTable;

		//	Sprite (for optimized blting)
		CG16bitSprite *m_pSprite;

		//	Clipping support
		RECT m_rcClip;				//	Clip rect

		//	DirectDraw
		LPDIRECTDRAWSURFACE7 m_pSurface;
		BITMAPINFO *m_pBMI;			//	Used for blting to a DC
	};

bool CalcBltTransform (Metric rX,
					   Metric rY,
					   Metric rScaleX,
					   Metric rScaleY,
					   Metric rRotation,
					   int xSrc,
					   int ySrc,
					   int cxSrc,
					   int cySrc,
					   CXForm *retSrcToDest, 
					   CXForm *retDestToSrc, 
					   RECT *retrcDest);
void CompositeFilledCircle (CG16bitImage &Dest, int xDest, int yDest, int iRadius, WORD *pColorTable, BYTE *pOpacityTable, bool bStochastic = false);
void CopyBltColorize (CG16bitImage &Dest,
					  int xDest,
					  int yDest,
					  int cxDest,
					  int cyDest,
					  const CG16bitImage &Src,
					  int xSrc,
					  int ySrc,
					  DWORD dwHue,
					  DWORD dwSaturation);
void CopyBltTransformed (CG16bitImage &Dest, 
						 const RECT &rcDest,
						 const CG16bitImage &Src, 
						 const RECT &rcSrc,
						 const CXForm &SrcToDest,
						 const CXForm &DestToSrc,
						 const RECT &rcDestXForm);
void CreateRoundedRectAlpha (int cxWidth, int cyHeight, int iRadius, CG16bitImage *retImage);
void DrawAlphaGradientCircle (CG16bitImage &Dest, 
		int xDest, 
		int yDest, 
		int iRadius,
		WORD wColor);
void DrawArc (CG16bitImage &Dest, int xCenter, int yCenter, int iRadius, int iStartAngle, int iEndAngle, int iLineWidth, WORD wColor);
void DrawBltCircle (CG16bitImage &Dest, 
		int xDest, 
		int yDest, 
		int iRadius,
		const CG16bitImage &Src,
		int xSrc,
		int ySrc,
		int cxSrc,
		int cySrc,
		DWORD byOpacity);
void DrawBltRotated (CG16bitImage &Dest,
					 int xDest,
					 int yDest,
					 int iRotation,
					 const CG16bitImage &Src,
					 int xSrc,
					 int ySrc,
					 int cxSrc,
					 int cySrc);
void DrawBltScaledFast (CG16bitImage &Dest,
						int xDest,
						int yDest,
						int cxDest,
						int cyDest,
						const CG16bitImage &Src,
						int xSrc,
						int ySrc,
						int cxSrc,
						int cySrc);
void DrawBltShimmer (CG16bitImage &Dest,
					 int xDest,
					 int yDest,
					 int cxDest,
					 int cyDest,
					 const CG16bitImage &Src,
					 int xSrc,
					 int ySrc,
					 DWORD byIntensity,
					 DWORD dwSeed = 0);
void DrawBltTransformed (CG16bitImage &Dest, 
						 Metric rX, 
						 Metric rY, 
						 Metric rScaleX, 
						 Metric rScaleY, 
						 Metric rRotation, 
						 const CG16bitImage &Src, 
						 int xSrc, 
						 int ySrc, 
						 int cxSrc, 
						 int cySrc);
void DrawBltTransformedGray (CG16bitImage &Dest, 
						 Metric rX, 
						 Metric rY, 
						 Metric rScaleX, 
						 Metric rScaleY, 
						 Metric rRotation, 
						 const CG16bitImage &Src, 
						 int xSrc, 
						 int ySrc, 
						 int cxSrc, 
						 int cySrc);
void DrawBrokenLine (CG16bitImage &Dest, int xSrc, int ySrc, int xDest, int yDest, int xyBreak, WORD wColor);
void DrawDottedLine (CG16bitImage &Dest, int x1, int y1, int x2, int y2, WORD wColor);
void DrawFilledCircle (CG16bitImage &Dest, int xDest, int yDest, int iRadius, WORD wColor);
void DrawFilledCircle (CG16bitImage &Dest, int xDest, int yDest, int iRadius, WORD *pColorTable, BYTE *pOpacityTable);
void DrawFilledCircleGray (CG16bitImage &Dest, int xDest, int yDest, int iRadius, WORD wColor, DWORD byOpacity);
void DrawFilledCircleTrans (CG16bitImage &Dest, int xDest, int yDest, int iRadius, WORD wColor, DWORD byOpacity);
void DrawGlowRing (CG16bitImage &Dest,
				   int xDest,
				   int yDest,
				   int iRadius,
				   int iRingThickness,
				   WORD wColor,
				   DWORD dwOpacity = 255);
void DrawGlowRing (CG16bitImage &Dest,
				   int xDest,
				   int yDest,
				   int iRadius,
				   int iRingThickness,
				   WORD *wColorRamp,
				   DWORD *byOpacityRamp);
void DrawGradientRectHorz (CG16bitImage &Dest,
		int xDest,
		int yDest,
		int cxDest,
		int cyDest,
		WORD wStartColor,
		WORD wEndColor,
		DWORD dwStartOpacity,
		DWORD dwEndOpacity);
void DrawRectDotted (CG16bitImage &Dest, int x, int y, int cxWidth, int cyHeight, WORD wColor);
void DrawRoundedRect (CG16bitImage &Dest, int x, int y, int cxWidth, int cyHeight, int iRadius, WORD wColor);
void DrawRoundedRectOutline (CG16bitImage &Dest, int x, int y, int cxWidth, int cyHeight, int iRadius, int iLineWidth, WORD wColor);
void DrawRoundedRectTrans (CG16bitImage &Dest, int x, int y, int cxWidth, int cyHeight, int iRadius, WORD wColor, DWORD byOpacity);

enum EffectTypes
	{
	effectShadow =			0x01,
	};

struct SEffectDesc
	{
	EffectTypes iType;
	WORD wColor1;
	WORD wColor2;
	};

class CG16bitFont : public CObject
	{
	public:
		enum Flags
			{
			SmartQuotes =	0x00000001,
			TruncateLine =	0x00000002,

			AlignCenter =	0x00000004,
			AlignRight =	0x00000008,

			MeasureOnly =	0x00000010,
			};

		CG16bitFont (void);

		ALERROR Create (const CString &sTypeface, int iSize, bool bBold = false, bool bItalic = false, bool bUnderline = false);
		ALERROR CreateFromFont (HFONT hFont);
		ALERROR CreateFromResource (HINSTANCE hInst, char *pszRes);
		inline void Destroy (void) { m_FontImage.Destroy(); m_Metrics.RemoveAll(); }

		int BreakText (const CString &sText, int cxWidth, TArray<CString> *retLines, DWORD dwFlags = 0) const;
		inline void DrawText (CG16bitImage &Dest, int x, int y, WORD wColor, const CString &sText, DWORD dwFlags = 0, int *retx = NULL) const
			{ DrawText(Dest, x, y, wColor, 255, sText, dwFlags, retx); }
		inline void DrawText (CG16bitImage &Dest, const RECT &rcRect, WORD wColor, const CString &sText, int iLineAdj = 0, DWORD dwFlags = 0, int *retcyHeight = NULL) const
			{ DrawText(Dest, rcRect, wColor, 255, sText, iLineAdj, dwFlags, retcyHeight); }

		void DrawText (CG16bitImage &Dest, 
					   int x, 
					   int y, 
					   WORD wColor, 
					   DWORD byOpacity,
					   const CString &sText, 
					   DWORD dwFlags = 0, 
					   int *retx = NULL) const;
		void DrawText (CG16bitImage &Dest, 
					   const RECT &rcRect, 
					   WORD wColor, 
					   DWORD byOpacity,
					   const CString &sText, 
					   int iLineAdj = 0, 
					   DWORD dwFlags = 0,
					   int *retcyHeight = NULL) const;
		void DrawTextEffect (CG16bitImage &Dest,
							 int x,
							 int y,
							 WORD wColor,
							 const CString &sText,
							 int iEffectsCount,
							 const SEffectDesc *pEffects,
							 DWORD dwFlags = 0,
							 int *retx = NULL) const;
		inline int GetAscent (void) const { return m_cyAscent; }
		inline int GetAverageWidth (void) const { return m_cxAveWidth; }
		inline int GetHeight (void) const { return m_cyHeight; }
		inline const CString &GetTypeface (void) const { return m_sTypeface; }
		inline bool IsBold (void) const { return m_bBold; }
		inline bool IsEmpty (void) const { return m_FontImage.IsEmpty(); }
		inline bool IsItalic (void) const { return m_bItalic; }
		inline bool IsUnderline (void) const { return m_bUnderline; }
		int MeasureText (const CString &sText, int *retcyHeight = NULL) const;
		static bool ParseFontDesc (const CString &sDesc, CString *retsTypeface, int *retiSize, bool *retbBold, bool *retbItalic);
		ALERROR ReadFromStream (IReadStream *pStream);
		void WriteToStream (IWriteStream *pStream);

	private:
		struct CharMetrics
			{
			int cxWidth;
			int cxAdvance;
			};

		CG16bitImage m_FontImage;

		int m_cyHeight;				//	Height of the font
		int m_cyAscent;				//	Height above baseline
		int m_cxAveWidth;			//	Average width of font
		CStructArray m_Metrics;		//	Metrics for each character

		CString m_sTypeface;		//	Name of font selected
		bool m_bBold;				//	Bold
		bool m_bItalic;				//	Italic
		bool m_bUnderline;			//	Underline
	};

class CG16bitRegion
	{
	public:
		CG16bitRegion (void);
		~CG16bitRegion (void) { CleanUp(); }

		void CleanUp (void);
		void ColorTransBlt (CG16bitImage &Dest, int xDest, int yDest, CG16bitImage &Src, int xSrc, int ySrc, int cxWidth, int cyHeight);
		ALERROR CreateFromMask (const CG16bitImage &Source, int xSrc, int ySrc, int cxWidth, int cyHeight);

	private:
		enum ECodes
			{
			codeMask =			0xC000,
			countMask =			0x3FFF,

			code00 =			0x0000,
			codeRun =			0x4000,
			codeEndOfLine =		0x8000,
			codeFF =			0xC000,
			};

		void AllocCode (DWORD dwNewAlloc);
		void ColorTransBltAlpha (CG16bitImage &Dest, int xDest, int yDest, CG16bitImage &Src, int xSrc, int ySrc, int cxWidth, int cyHeight, int xMask, int yMask);
		void ColorTransBltBackColor (CG16bitImage &Dest, int xDest, int yDest, CG16bitImage &Src, int xSrc, int ySrc, int cxWidth, int cyHeight, int xMask, int yMask);
		void ColorTransBltPlain (CG16bitImage &Dest, int xDest, int yDest, CG16bitImage &Src, int xSrc, int ySrc, int cxWidth, int cyHeight, int xMask, int yMask);
		void WriteCode (WORD *&pCode, DWORD &dwCodeSize, DWORD dwCode, DWORD dwCount);
		void WriteCodeRun (WORD *&pCode, DWORD &dwCodeSize, BYTE *pRun, DWORD dwCount);

		int m_cxWidth;
		int m_cyHeight;
		DWORD m_dwAlloc;
		WORD *m_pCode;
		TArray<DWORD> m_LineIndex;
	};

class CG16bitBinaryRegion
	{
	public:
		struct SRun
			{
			int xStart;
			int xEnd;
			int y;
			};

		CG16bitBinaryRegion (void);
		~CG16bitBinaryRegion (void);

		void CreateFromPolygon (int iVertexCount, SPoint *pVertexList);
		void CreateFromConvexPolygon (int iVertexCount, SPoint *pVertexList);
		void CreateFromSimplePolygon (int iVertexCount, SPoint *pVertexList);
		void Fill (CG16bitImage &Dest, int x, int y, WORD wColor) const;
		void FillTrans (CG16bitImage &Dest, int x, int y, WORD wColor, DWORD byOpacity) const;
		void GetBounds (RECT *retrcRect) const;
		inline int GetCount (void) const { return m_iCount; }
		inline const SRun &GetRun (int iIndex) const { return m_pList[iIndex]; }

	private:
		inline SRun &Run (int iIndex) { return m_pList[iIndex]; }
		void ScanEdge (int X1,
					   int Y1,
					   int X2,
					   int Y2,
					   bool bSetX,
					   int iSkipFirst,
					   SRun **iopRun);

		int m_iCount;
		SRun *m_pList;
	};

bool IsConvexPolygon (int iVertexCount, SPoint *pVertexList);
bool IsSimplePolygon (int iVertexCount, SPoint *pVertexList);
int CreateScanLinesFromPolygon (int iVertexCount, SPoint *pVertexList, CG16bitBinaryRegion::SRun **retpLines);
int CreateScanLinesFromSimplePolygon (int iVertexCount, SPoint *pVertexList, CG16bitBinaryRegion::SRun **retpLines);

inline void CG16bitImage::DrawText (int x, int y, const CG16bitFont &Font, WORD wColor, CString sText, DWORD dwFlags, int *retx)
	{
	Font.DrawText(*this, x, y, wColor, sText, dwFlags, retx);
	}

class CG16bitLinePainter
	{
	public:
		struct SPixelDesc
			{
			WORD *pPos;						//	Pixel to draw on
			DWORD dwOpacity;				//	Opacity
			int x;							//	x-coordinate on image
			int y;							//	y-coordinate on image

			double rV;						//	Distance from start to end along line (0 to 1.0)
			double rW;						//	Distance perpendicular to line axis (0 to 1.0)
			};

		void PaintSolid (CG16bitImage &Image, int x1, int y1, int x2, int y2, int iWidth, WORD wColor);
		void Rasterize (CG16bitImage &Image, int x1, int y1, int x2, int y2, int iWidth, TArray<SPixelDesc> *retPixels);

	private:
		enum ESlopeTypes
			{
			lineNull,						//	No line
			linePoint,						//	Single point
			lineXDominant,
			lineYDominant,
			};

		ESlopeTypes CalcIntermediates (const CG16bitImage &Image, int x1, int y1, int x2, int y2, int iWidth);
		void CalcPixelMapping (int x1, int y1, int x2, int y2, double *retrV, double *retrW);

		inline void NextX (int &x, int &y)
			{
			if (m_d >= 0)
				{
				y = y + m_sy;
				m_d = m_d - m_ax;
				m_rWUp += m_rWUpInc;
				m_rWDown -= m_rWDownDec;
				}

			m_d = m_d + m_ay;
			m_rWUp -= m_rWUpDec;
			m_rWDown += m_rWDownInc;
			}

		inline void NextY (int &x, int &y)
			{
			if (m_d >= 0)
				{
				x = x + m_sx;
				m_d = m_d - m_ay;
				m_rWUp += m_rWUpInc;
				m_rWDown -= m_rWDownDec;
				}

			m_d = m_d + m_ax;
			m_rWUp -= m_rWUpDec;
			m_rWDown += m_rWDownInc;
			}

		inline void NextX (int &x, int &y, double &rV, double &rW)
			{
			if (m_d >= 0)
				{
				y = y + m_sy;
				m_d = m_d - m_ax;
				m_rWUp += m_rWUpInc;
				m_rWDown -= m_rWDownDec;

				rV += m_rVIncY;
				rW += m_rWIncY;
				}

			m_d = m_d + m_ay;
			m_rWUp -= m_rWUpDec;
			m_rWDown += m_rWDownInc;

			rV += m_rVIncX;
			rW += m_rWIncX;
			}

		inline void NextY (int &x, int &y, double &rV, double &rW)
			{
			if (m_d >= 0)
				{
				x = x + m_sx;
				m_d = m_d - m_ay;
				m_rWUp += m_rWUpInc;
				m_rWDown -= m_rWDownDec;

				rV += m_rVIncX;
				rW += m_rWIncX;
				}

			m_d = m_d + m_ax;
			m_rWUp -= m_rWUpDec;
			m_rWDown += m_rWDownInc;

			rV += m_rVIncY;
			rW += m_rWIncY;
			}

		int m_dx;							//	Distance from x1 to x2
		int m_sx;							//	Direction (from x1 to x2)
		int m_ax;							//	absolute distance x (times 2)
		double m_rL;						//	Length of line
		int m_xStart;
		int m_xEnd;

		int m_dy;							//	Distance from y1 to y2
		int m_sy;							//	Direction (from y1 to y2)
		int m_ay;							//	absolute distance y (times 2)
		int m_yStart;
		int m_yEnd;

		int m_d;							//	Discriminator

		double m_rHalfWidth;				//	Half of width;
		int m_iAxisHalfWidth;				//	Half line width aligned on dominant axis
		int m_iAxisWidth;					//	Full width of line aligned on dominant axis

		double m_rWDown;					//	Intermediates for line width
		double m_rWDownDec;
		double m_rWDownInc;
		double m_rWUp;
		double m_rWUpDec;
		double m_rWUpInc;
		int m_wMin;
		int m_wMax;

		int m_iPosRowInc;					//	Image row offset

		double m_rVPerX;					//	Movement along V (line axis) for every X movement
		double m_rVPerY;					//	Movement along V for every Y movement
		double m_rWPerX;					//	Movement along W (perp axis) for every X movement
		double m_rWPerY;					//	Movement along W for every Y movement

		double m_rVIncX;
		double m_rVIncY;
		double m_rWIncX;
		double m_rWIncY;
	};

//	 8-bit drawing functions (for masks) ---------------------------------------

void DrawHorzLine8bit (CG16bitImage &Dest, int x, int y, int cxWidth, BYTE byValue);
void DrawFilledCircle8bit (CG16bitImage &Dest, int x, int y, int iRadius, BYTE byValue);
void DrawFilledRect8bit (CG16bitImage &Dest, int x, int y, int cxWidth, int cyHeight, BYTE byValue);
void DrawGradientCircle8bit (CG16bitImage &Dest, 
							 int xDest, 
							 int yDest, 
							 int iRadius,
							 BYTE byCenter,
							 BYTE byEdge);
void DrawNebulosity8bit (CG16bitImage &Dest, int x, int y, int cxWidth, int cyHeight, int iScale, BYTE byMin, BYTE byMax);
void DrawNoise8bit (CG16bitImage &Dest, int x, int y, int cxWidth, int cyHeight, int iScale, BYTE byMin, BYTE byMax);
void RasterizeQuarterCircle8bit (int iRadius, int *retSolid, BYTE *retEdge, DWORD byOpacity = 255);

//	Noise Functions ------------------------------------------------------------

struct SNoisePos
	{
	int iInteger;
	int iFraction;
	};

class CNoiseGenerator
	{
	public:
		CNoiseGenerator (int iScale);
		~CNoiseGenerator (void);

		inline float GetAt (SNoisePos &x, SNoisePos &y);
		inline void Next (SNoisePos &i) { if (++i.iFraction == m_iScale) { i.iInteger++; i.iFraction = 0; } }
		inline void Reset (SNoisePos &i, int iPos) { i.iInteger = iPos / m_iScale; i.iFraction = iPos % m_iScale; }

	private:
		int m_iScale;
		float *m_Frac;
		float *m_InvFrac;
		float *m_Smooth;
	};

float Noise2D (int x, float fx0, float fx1, float wx, int y, float fy0, float fy1, float wy);
float Noise3D (int x, float fx0, float fx1, float wx, int y, float fy0, float fy1, float wy, int z, float fz0, float fz1, float wz);
void NoiseInit (DWORD dwSeed = 0);
void NoiseReinit (DWORD dwSeed = 0);
float NoiseSmoothStep (float x);

inline float CNoiseGenerator::GetAt (SNoisePos &x, SNoisePos &y)
	{ return Noise2D(x.iInteger, m_Frac[x.iFraction], m_InvFrac[x.iFraction], m_Smooth[x.iFraction], y.iInteger, m_Frac[y.iFraction], m_InvFrac[y.iFraction], m_Smooth[y.iFraction]); }

#ifndef INCL_TEXT_FORMAT
#include "TextFormat.h"
#endif

//	Screen Elements ------------------------------------------------------------

class AGArea;
class AGScreen;

class IScreenController
	{
	public:
		virtual void Action (DWORD dwTag, DWORD dwData = 0) = 0;
	};

class IAreaContainer
	{
	public:
		virtual void GetMousePos (POINT *retpt) { }
		virtual void OnAreaSetRect (void) { }
	};

class AGArea : public CObject
	{
	public:
		AGArea (void);
		virtual ~AGArea (void);

		void AddEffect (const SEffectDesc &Effect);
		void AddShadowEffect (void);
		inline IAreaContainer *GetParent (void) { return m_pParent; }
		inline RECT &GetRect (void) { return m_rcRect; }
		inline const RECT &GetRect (void) const { return m_rcRect; }
		inline AGScreen *GetScreen (void) { return m_pScreen; }
		inline DWORD GetTag (void) { return m_dwTag; }
		inline void Hide (void) { ShowHide(false); }
		ALERROR Init (AGScreen *pScreen, IAreaContainer *pParent, const RECT &rcRect, DWORD dwTag);
		inline void Invalidate (void) { Invalidate(m_rcRect); }
		inline void Invalidate (const RECT &rcInvalid);
		inline bool IsVisible (void) { return m_bVisible; }
		inline void SetController (IScreenController *pController) { m_pController = pController; }
		void SetRect (const RECT &rcRect);
		void SignalAction (DWORD dwData = 0);
		inline void Show (void) { ShowHide(true); }
		void ShowHide (bool bShow);

		//	AGArea virtuals
		virtual int Justify (const RECT &rcRect) { return 0; }
		virtual bool LButtonDoubleClick (int x, int y) { return false; }
		virtual bool LButtonDown (int x, int y) { return false; }
		virtual void LButtonUp (int x, int y) { }
		virtual void MouseEnter (void) { }
		virtual void MouseLeave (void) { }
		virtual void MouseMove (int x, int y) { }
		virtual void Paint (CG16bitImage &Dest, const RECT &rcRect) { }
		virtual void Update (void) { }
		virtual bool WantsMouseOver (void) { return true; }

	protected:
		inline const SEffectDesc &GetEffect (int iIndex) { return m_pEffects[iIndex]; }
		inline int GetEffectCount (void) { return m_iEffectCount; }
		inline const SEffectDesc *GetEffects (void) { return m_pEffects; }
		inline bool HasEffects (void) { return (m_pEffects != NULL); }
		virtual void OnSetRect (void) { }
		virtual void OnShowHide (bool bShow) { }

	private:
		IAreaContainer *m_pParent;
		AGScreen *m_pScreen;
		RECT m_rcRect;
		DWORD m_dwTag;
		IScreenController *m_pController;

		int m_iEffectCount;
		SEffectDesc *m_pEffects;

		bool m_bVisible;						//	TRUE if visible

	friend CObjectClass<AGArea>;
	};

class AGScreen : public CObject, public IAreaContainer
	{
	public:
		AGScreen (HWND hWnd, const RECT &rcRect);
		virtual ~AGScreen (void);

		ALERROR AddArea (AGArea *pArea, const RECT &rcRect, DWORD dwTag, bool bSendToBack = false);
		void DestroyArea (AGArea *pArea);
		AGArea *FindArea (DWORD dwTag);
		inline AGArea *GetArea (int iIndex) { return m_Areas[iIndex]; }
		inline int GetAreaCount (void) { return m_Areas.GetCount(); }
		inline const RECT &GetRect (void) { return m_rcRect; }
		inline IScreenController *GetController (void) { return m_pController; }
		inline void Invalidate (const RECT &rcInvalid) { UnionRect(&m_rcInvalid, &rcInvalid, &m_rcInvalid); }
		inline void SetBackgroundColor (WORD wColor) { m_wBackgroundColor = wColor; }
		inline void SetController (IScreenController *pController) { m_pController = pController; }

		//	IAreaContainer virtuals
		virtual void GetMousePos (POINT *retpt);
		virtual void OnAreaSetRect (void);

		//	These methods are called by the HWND
		void LButtonDoubleClick (int x, int y);
		void LButtonDown (int x, int y);
		void LButtonUp (int x, int y);
		void MouseMove (int x, int y);
		void Paint (CG16bitImage &Dest);
		void Update (void);

	private:
		AGScreen (void);
		void FireMouseMove (const POINT &pt);
		inline int GetAreaIndex (AGArea *pArea) { int iIndex; if (m_Areas.Find(pArea, &iIndex)) return iIndex; else return -1; }
		AGArea *HitTest (const POINT &pt);
		void SetMouseOver (AGArea *pArea);

		HWND m_hWnd;
		RECT m_rcRect;							//	Rect of screen relative to Window
		RECT m_rcInvalid;						//	Invalid rect relative to m_rcRect
		IScreenController *m_pController;		//	Screen controller

		TArray<AGArea *> m_Areas;

		AGArea *m_pMouseCapture;				//	Area that has captured the mouse
		AGArea *m_pMouseOver;					//	Area that the mouse is currently over

		WORD m_wBackgroundColor;

	friend CObjectClass<AGScreen>;
	};

class CGFrameArea : public AGArea, public IAreaContainer
	{
	public:
		CGFrameArea (void);

		ALERROR AddArea (AGArea *pArea, const RECT &rcRect, DWORD dwTag);
		AGArea *FindArea (DWORD dwTag);
		inline AGArea *GetArea (int iIndex) { return (AGArea *)m_Areas.GetObject(iIndex); }
		inline int GetAreaCount (void) { return m_Areas.GetCount(); }

		//	AGArea virtuals
		virtual bool LButtonDoubleClick (int x, int y);
		virtual bool LButtonDown (int x, int y);
		virtual void LButtonUp (int x, int y);
		virtual void MouseMove (int x, int y);
		virtual void Paint (CG16bitImage &Dest, const RECT &rcRect);
		virtual void Update (void);

		//	IAreaContainer virtuals
		virtual void OnAreaSetRect (void);

	private:
		RECT m_rcInvalid;						//	Invalid rect relative to m_rcRect

		CObjectArray m_Areas;					//	Array of areas

		AGArea *m_pMouseCapture;				//	Area that has captured the mouse
		AGArea *m_pMouseOver;					//	Area that the mouse is currently over
	};

class CGButtonArea : public AGArea
	{
	public:
		CGButtonArea (void);

		inline CString GetLabelAccelerator (void) { return (m_iAccelerator != -1 ? CString(m_sLabel.GetASCIIZPointer() + m_iAccelerator, 1) : NULL_STR); }
		inline bool IsDisabled (void) { return m_bDisabled; }
		inline void SetDisabled (bool bDisabled = true) { m_bDisabled = bDisabled; }
		inline void SetLabel (const CString &sText) { m_sLabel = sText; m_iAccelerator = -1; }
		void SetLabelAccelerator (const CString &sKey);
		inline void SetLabelColor (WORD wColor) { m_wLabelColor = wColor; }
		inline void SetLabelFont (const CG16bitFont *pFont) { m_pLabelFont = pFont; }

		//	AGArea virtuals
		virtual void LButtonUp (int x, int y);
		virtual void MouseEnter (void);
		virtual void MouseLeave (void);
		virtual void Paint (CG16bitImage &Dest, const RECT &rcRect);

	protected:
		//	AGArea virtuals
		virtual void OnSetRect (void);

	private:
		CString m_sLabel;
		WORD m_wLabelColor;
		const CG16bitFont *m_pLabelFont;
		int m_iAccelerator;

		bool m_bMouseOver;
		bool m_bDisabled;
	};

class CGImageArea : public AGArea
	{
	public:
		CGImageArea (void);

		inline void SetBackColor (WORD wColor) { m_rgbBackColor = RGB(CG16bitImage::RedValue(wColor), CG16bitImage::GreenValue(wColor), CG16bitImage::BlueValue(wColor)); }
		inline void SetImage (CG16bitImage *pImage, const RECT &rcImage) { m_pImage = pImage; m_rcImage = rcImage; Invalidate(); }
		inline void SetStyles (DWORD dwStyles) { m_dwStyles = dwStyles; Invalidate(); }
		inline void SetTransBackground (bool bTrans = true) { m_bTransBackground = bTrans; }

		//	AGArea virtuals
		virtual void Paint (CG16bitImage &Dest, const RECT &rcRect);
		virtual bool WantsMouseOver (void) { return false; }

	private:
		CG16bitImage *m_pImage;
		RECT m_rcImage;

		COLORREF m_rgbBackColor;
		bool m_bTransBackground;
		DWORD m_dwStyles;
	};

class CGTextArea : public AGArea
	{
	public:
		CGTextArea (void);

		inline const CString &GetText (void) { return m_sText; }
		inline void SetColor (WORD wColor) { m_Color = wColor; }
		inline void SetCursor (int iLine, int iCol = 0) { m_iCursorLine = iLine; m_iCursorPos = iCol; }
		inline void SetEditable (bool bEditable = true) { m_bEditable = bEditable; }
		inline void SetFont (const CG16bitFont *pFont) { m_pFont = pFont; m_cxJustifyWidth = 0; }
		inline void SetFontTable (const IFontTable *pFontTable) { m_pFontTable = pFontTable; }
		inline void SetLineSpacing (int cySpacing) { m_cyLineSpacing = cySpacing; m_cxJustifyWidth = 0; }
		inline void SetRichText (const CString &sRTF) { m_sRTF = sRTF; m_sText = NULL_STR; m_bRTFInvalid = true; Invalidate(); }
		inline void SetStyles (DWORD dwStyles) { m_dwStyles = dwStyles; m_cxJustifyWidth = 0; }
		inline void SetText (const CString &sText) { m_sText = sText; m_sRTF = NULL_STR; m_cxJustifyWidth = 0; Invalidate(); }

		//	AGArea virtuals
		virtual int Justify (const RECT &rcRect);
		virtual void Paint (CG16bitImage &Dest, const RECT &rcRect);
		virtual void Update (void) { m_iTick++; }

	private:
		void FormatRTF (const RECT &rcRect);
		void PaintRTF (CG16bitImage &Dest, const RECT &rcRect);
		void PaintText (CG16bitImage &Dest, const RECT &rcRect);

		CString m_sText;						//	Text text to draw
		CString m_sRTF;							//	Rich text to draw (only if m_sText is blank)

		bool m_bEditable;						//	TRUE if editable
		DWORD m_dwStyles;						//	AlignmentStyles
		int m_cyLineSpacing;					//	Extra spacing between lines
		const CG16bitFont *m_pFont;
		WORD m_Color;

		bool m_bRTFInvalid;						//	TRUE if we need to format rich text
		CTextBlock m_RichText;					//	Rich text to draw (only if m_sText is blank)
		const IFontTable *m_pFontTable;			//	For rich text

		TArray<CString> m_Lines;				//	Justified lines of text
		int m_cxJustifyWidth;					//	Width (in pixels) for which m_Lines
												//		was justified.

		int m_iTick;							//	Cursor tick
		int m_iCursorLine;						//	Cursor position (-1 = no cursor)
		int m_iCursorPos;						//	Position in line
	};

//	Screen Manager ------------------------------------------------------------

struct SScreenMgrOptions
	{
	SScreenMgrOptions (void) :
			m_hWnd(NULL),
			m_bWindowedMode(false),
			m_bMultiMonitorMode(false),
			m_bForceDX(false),
			m_bForceNonDX(false),
			m_bForceExclusiveMode(false),
			m_bForceNonExclusiveMode(false),
			m_bForceScreenSize(false),
			m_cxScreen(1024),
			m_cyScreen(768),
			m_iColorDepth(16),
			m_bDebugVideo(false)
		{ }

	HWND m_hWnd;

	bool m_bWindowedMode;
	bool m_bMultiMonitorMode;			//	If TRUE, screen spans all monitors

	bool m_bForceDX;
	bool m_bForceNonDX;
	bool m_bForceExclusiveMode;
	bool m_bForceNonExclusiveMode;
	bool m_bForceScreenSize;			//	If in exclusive mode, switch to desired res

	int m_cxScreen;						//	Used if exclusive or windowed
	int m_cyScreen;						//	Used if exclusive or windowed
	int m_iColorDepth;					//	Used if exclusive

	bool m_bDebugVideo;
	};

class CScreenMgr
	{
	public:
		CScreenMgr (void);
		~CScreenMgr (void);

		void Blt (void);
		bool CheckIsReady (void);
		void CleanUp (void);
		void ClientToScreen (int x, int y, int *retx, int *rety);
		inline void Flip (void) { if (m_bExclusiveMode && m_pDD) FlipInt(); }
		inline bool GetInvalidRect (RECT *retrcRect) { *retrcRect = m_rcInvalid; return (::IsRectEmpty(&m_rcInvalid) != TRUE); }
		inline CG16bitImage &GetScreen (void) { return m_Screen; }
		void GlobalToLocal (int x, int y, int *retx, int *rety);
		ALERROR Init (SScreenMgrOptions &Options, CString *retsError);
		void Invalidate (void) { SetInvalidAll(&m_rcInvalid); SetInvalidAll(&m_rcInvalidSecondary); }
		void Invalidate (const RECT &rcRect) { ::UnionRect(&m_rcInvalid, &m_rcInvalid, &rcRect); ::UnionRect(&m_rcInvalidSecondary, &m_rcInvalidSecondary, &rcRect); }
		inline bool IsMinimized (void) const { return m_bMinimized; }
		inline bool IsScreenPersistent (void) { return !m_bBackgroundBlt; }
		void LocalToGlobal (int x, int y, int *retx, int *rety);
		void OnWMActivateApp (bool bActivate);
		void OnWMDisplayChange (int iBitDepth, int cxWidth, int cyHeight);
		void OnWMMove (int x, int y);
		void OnWMSize (int cxWidth, int cyHeight, int iSize);
		void ScreenToClient (int x, int y, int *retx, int *rety);
		void StopDX (void);
		void Validate (void) { SetInvalidNone(&m_rcInvalid); }

		void BltToPrimary (CG16bitImage &Image);
		void BltToScreen (CG16bitImage &Image);

	private:
		void BackgroundCleanUp (void);
		void BackgroundInit (void);
		static DWORD WINAPI BackgroundThread (LPVOID pData);

		void DebugOutputStats (void);
		void FlipInt (void);
		void SetInvalidAll (RECT *rcRect) { rcRect->left = 0; rcRect->top = 0; rcRect->right = m_cxScreen; rcRect->bottom = m_cyScreen; }
		void SetInvalidNone (RECT *rcRect) { rcRect->left = 0; rcRect->top = 0; rcRect->right = 0; rcRect->bottom = 0; }

		HWND m_hWnd;

		CG16bitImage m_Screen;			//	This is the image that everyone will write to
		CG16bitImage m_Secondary;		//	This is used to swap, when doing background blts

		RECT m_rcInvalid;				//	Current invalid area (relative to m_Screen)
		RECT m_rcInvalidSecondary;		//	Invalid area of secondary screen

		RECT m_rcScreen;				//	RECT of screen area relative to window client coordinates
		int m_cxScreen;					//	Width of screen area in pixels (if windowed, this is client area)
		int m_cyScreen;					//	Height of screen area in pixels

		bool m_bDX;						//	TRUE if we're using DX
		bool m_bExclusiveMode;			//	TRUE if we're in exclusive mode
		bool m_bWindowedMode;			//	TRUE if we're in a window (as opposed to full screen)
		LPDIRECTDRAW7 m_pDD;
		LPDIRECTDRAWSURFACE7 m_pPrimary;
		CG16bitImage::SurfaceTypes m_PrimaryType;
		LPDIRECTDRAWSURFACE7 m_pBack;
		LPDIRECTDRAWSURFACE7 m_pCurrent;
		LPDIRECTDRAWCLIPPER m_pClipper;
		bool m_bDXReady;				//	TRUE if DX initialized
		int m_iColorDepth;				//	Color depth (if exclusive)
		bool m_bMinimized;				//	TRUE if we're minimized

		bool m_bBackgroundBlt;			//	TRUE if background blt is enabled
		HANDLE m_hBackgroundThread;		//	Background thread
		HANDLE m_hWorkEvent;			//	Blt
		HANDLE m_hQuitEvent;			//	Time to quit

		bool m_bDebugVideo;				//	TRUE if we're in debug video mode
	};

#ifdef LATER
class ISceneSink
	{
	public:
		virtual void OnStart (CGDirector *pDirector) { }
		virtual void OnStop (void) { }

		virtual void OnAnimate (void) { }
		virtual void OnKeyDown (int iVirtKey, DWORD dwKeyData) { }
		virtual void OnKeyUp (int iVirtKey, DWORD dwKeyData) { }
		virtual void OnLButtonDblClick (int x, int y, DWORD dwFlags) { }
		virtual void OnLButtonDown (int x, int y, DWORD dwFlags) { }
		virtual void OnLButtonUp (int x, int y, DWORD dwFlags) { }
		virtual void OnMouseMove (int x, int y, DWORD dwFlags) { }
	};

class CGDirector
	{
	public:
		CDirector (void);
		~CDirector (void);

		void SetScene (IScene *pScene);

		void StartShow (void);
		void EndShow (void);

		void Animate (void);
		void KeyDown (int iVirtKey, DWORD dwKeyData);
		void KeyUp (int iVirtKey, DWORD dwKeyData);
		void LButtonDblClick (int x, int y, DWORD dwFlags);
		void LButtonDown (int x, int y, DWORD dwFlags);
		void LButtonUp (int x, int y, DWORD dwFlags);
		void MouseMove (int x, int y, DWORD dwFlags);

	private:
		IScene *m_pScene;
	};
#endif

//	Inlines

inline void AGArea::Invalidate (const RECT &rcInvalid) { if (m_pScreen) m_pScreen->Invalidate(rcInvalid); }

//	Functions

void *SurfaceLock (LPDIRECTDRAWSURFACE7 pSurface, DDSURFACEDESC2 *retpDesc);
void SurfaceUnlock (LPDIRECTDRAWSURFACE7 pSurface);

void DebugOutputPixelFormat (DDPIXELFORMAT &format);
void DebugOutputSurfaceDesc (DDSURFACEDESC2 &surfacedesc);

//	DirectSound Classes -------------------------------------------------------

class CSoundMgr
	{
	public:
		struct SMusicPlayState
			{
			CString sFilename;
			bool bPlaying;
			bool bPaused;
			int iLength;
			int iPos;
			};

		CSoundMgr (void);
		~CSoundMgr (void);

		ALERROR Init (HWND hWnd);
		void CleanUp (void);

		void Delete (int iChannel);
		ALERROR LoadWaveFile (const CString &sFilename, int *retiChannel);
		ALERROR LoadWaveFromBuffer (IReadBlock &Data, int *retiChannel);
		void Play (int iChannel, int iVolume, int iPan);
		inline void SetWaveVolume (int iVolumeLevel) { m_iSoundVolume = iVolumeLevel; }

		bool CanPlayMusic (const CString &sFilename);
		void GetMusicCatalog (const CString &sMusicFolder, TArray<CString> *retCatalog);
		bool GetMusicPlayState (SMusicPlayState *retState);
		bool PlayMusic (const CString &sFilename, int iPos = 0, CString *retsError = NULL);
		int SetMusicVolume (int iVolumeLevel);
		void StopMusic (void);
		void TogglePlayPaused (void);

	private:
		struct SChannel
			{
			LPDIRECTSOUNDBUFFER pBuffer;
			CString sFilename;
			SChannel *pNext;
			};

		void AddMusicFolder (const CString &sFolder, TArray<CString> *retCatalog);
		int AllocChannel (void);
		void CleanUpChannel (SChannel &Channel);
		inline SChannel *GetChannel (int iChannel) { return &m_Channels[iChannel]; }
		inline int GetChannelCount (void) { return m_Channels.GetCount(); }
		ALERROR LoadWaveFile (HMMIO hFile, int *retiChannel);

		LPDIRECTSOUND m_pDS;
		TArray<SChannel> m_Channels;
		HWND m_hMusic;

		int m_iSoundVolume;
		int m_iMusicVolume;
	};

//	Miscellaneous Classes -----------------------------------------------------

class CBresenhamIterator
	{
	public:
		inline int GetX (void) { return m_x; }
		inline int GetY (void) { return m_y; }
		void Init (int x1, int y1, int x2, int y2);
		bool Step (void);

	private:
		int m_x;
		int m_y;

		int m_x2;
		int m_y2;

		int m_dx;
		int m_dy;
		int m_ax;
		int m_ay;
		int m_sx;
		int m_sy;
		int m_d;

		bool m_bXDominant;
	};

class CFrameRateCounter
	{
	public:
		CFrameRateCounter (int iSize = 128);
		~CFrameRateCounter (void);

		float GetFrameRate (void);
		void MarkFrame (void);

	private:
		int m_iSize;
		DWORD *m_pTimes;
		int m_iCount;
		int m_iPos;
	};

typedef BYTE AlphaArray5 [32];
typedef BYTE AlphaArray6 [64];
extern AlphaArray5 g_Alpha5 [256];
extern AlphaArray6 g_Alpha6 [256];

#endif