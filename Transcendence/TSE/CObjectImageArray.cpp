//	CObjectImageArray.cpp
//
//	CObjectImageArray

#include "PreComp.h"

static CObjectClass<CObjectImageArray>g_Class(OBJID_COBJECTIMAGEARRAY, NULL);

#define FLASH_TICKS_ATTRIB				CONSTLIT("flashTicks")
#define ROTATE_OFFSET_ATTRIB			CONSTLIT("rotationOffset")
#define BLENDING_ATTRIB					CONSTLIT("blend")
#define ROTATION_COLUMNS_ATTRIB			CONSTLIT("rotationColumns")
#define ROTATION_COUNT_ATTRIB			CONSTLIT("rotationCount")
#define VIEWPORT_SIZE_ATTRIB			CONSTLIT("viewportSize")
#define X_OFFSET_ATTRIB					CONSTLIT("xOffset")
#define Y_OFFSET_ATTRIB					CONSTLIT("yOffset")

#define LIGHTEN_BLENDING				CONSTLIT("brighten")

#define	GLOW_SIZE						4
#define FILTER_SIZE						5
#define FIXED_POINT						65536

static int g_FilterOffset[FILTER_SIZE] =
	{
	-GLOW_SIZE,
	-(GLOW_SIZE / 2),
	0,
	+(GLOW_SIZE / 2),
	+GLOW_SIZE
	};

static int g_Filter[FILTER_SIZE][FILTER_SIZE] =
	{
		{	   57,   454,   907,   454,    57	},
		{	  454,  3628,  7257,  3628,   454	},
		{	  907,  7257, 14513,  7257,   907	},
		{	  454,  3628,  7257,  3628,   454	},
		{	   57,   454,   907,   454,    57	},
	};

static char g_ImageIDAttrib[] = "imageID";
static char g_ImageXAttrib[] = "imageX";
static char g_ImageYAttrib[] = "imageY";
static char g_ImageWidthAttrib[] = "imageWidth";
static char g_ImageHeightAttrib[] = "imageHeight";
static char g_ImageFrameCountAttrib[] = "imageFrameCount";
static char g_ImageTicksPerFrameAttrib[] = "imageTicksPerFrame";

CObjectImageArray::CObjectImageArray (void) : CObject(&g_Class),
		m_pImage(NULL),
		m_pRotationOffset(NULL),
		m_pGlowImages(NULL),
		m_pScaledImages(NULL),
		m_dwBitmapUNID(0)

//	CObjectImageArray constructor

	{
	}

CObjectImageArray::CObjectImageArray (const CObjectImageArray &Source) : CObject(&g_Class)

//	CObjectImageArray copy constructor

	{
	CopyFrom(Source);
	}

CObjectImageArray::~CObjectImageArray (void)

//	CObjectImageArray destructor

	{
	CleanUp();
	}

CObjectImageArray &CObjectImageArray::operator= (const CObjectImageArray &Source)

//	Operator =

	{
	CleanUp();
	CopyFrom(Source);
	return *this;
	}

void CObjectImageArray::CleanUp (void)

//	CleanUp
//
//	Cleans up the image to free resources

	{
	if (m_pRotationOffset)
		{
		delete [] m_pRotationOffset;
		m_pRotationOffset = NULL;
		}

	if (m_pGlowImages)
		{
		delete [] m_pGlowImages;
		m_pGlowImages = NULL;
		}

	if (m_pScaledImages)
		{
		delete [] m_pScaledImages;
		m_pScaledImages = NULL;
		}

	if (m_pImage && m_dwBitmapUNID == 0)
		{
		delete m_pImage;
		m_pImage = NULL;
		}
	}

void CObjectImageArray::ComputeSourceXY (int iTick, int iRotation, int *retxSrc, int *retySrc) const

//	ComputeSourceXY
//
//	Computes the frame source position, based on tick and rotation.

	{
	//	If we're animating, then we expect a set of columns with one frame
	//	per column.

	if (m_iFrameCount > 0 && m_iTicksPerFrame > 0)
		{
		int iFrame;

		//	Compute the frame number.

		if (m_iFlashTicks > 0)
			{
			int iTotal = m_iFlashTicks + m_iTicksPerFrame;
			iFrame = (((iTick % iTotal) < m_iFlashTicks) ? 1 : 0);
			}
		else
			iFrame = ((iTick / m_iTicksPerFrame) % m_iFrameCount);

		//	If we've got multi-column rotations, then we need to deal with that.

		if (m_iRotationCount != m_iFramesPerColumn)
			{
			int iColsPerFrame = (m_iRotationCount + m_iFramesPerColumn - 1) / m_iFramesPerColumn;
			int iRotationCol = (iRotation / m_iFramesPerColumn);
			int iRotationRow = (iRotation % m_iFramesPerColumn);

			*retxSrc = m_rcImage.left + (((iFrame * iColsPerFrame) + iRotationCol) * RectWidth(m_rcImage));
			*retySrc = m_rcImage.top + (iRotationRow * RectHeight(m_rcImage));
			}

		//	Otherwise, we expect a single column per frame

		else
			{
			*retxSrc = m_rcImage.left + (iFrame * RectWidth(m_rcImage));
			*retySrc = m_rcImage.top + (iRotation * RectHeight(m_rcImage));
			}
		}

	//	If we've got multi-column rotations but no animations, then we just
	//	compute the proper column

	else if (m_iRotationCount != m_iFramesPerColumn)
		{
		int iRotationCol = (iRotation / m_iFramesPerColumn);
		int iRotationRow = (iRotation % m_iFramesPerColumn);

		*retxSrc = m_rcImage.left + (iRotationCol * RectWidth(m_rcImage));
		*retySrc = m_rcImage.top + (iRotationRow * RectHeight(m_rcImage));
		}
	
	//	Otherwise, a single columne

	else if (iRotation > 0)
		{
		*retxSrc = m_rcImage.left;
		*retySrc = m_rcImage.top + (iRotation * RectWidth(m_rcImage));
		}

	//	Otherwise, it's simple

	else
		{
		*retxSrc = m_rcImage.left;
		*retySrc = m_rcImage.top;
		}
	}

void CObjectImageArray::ComputeRotationOffsets (void)

//	ComputeRotationOffsets
//
//	Compute rotation offsets

	{
	if (m_iRotationOffset != 0 && m_iRotationCount > 0)
		{
		if (m_pRotationOffset)
			delete [] m_pRotationOffset;

		m_pRotationOffset = new OffsetStruct[m_iRotationCount];
		for (int i = 0; i < m_iRotationCount; i++)
			{
			int iRotationAngle = 360 / m_iRotationCount;
			int iAngleAdj = (m_iRotationCount / 4) + 1;
			int iAngle = iRotationAngle * (((m_iRotationCount - (i+1)) + iAngleAdj) % m_iRotationCount);
			CVector vOffset = PolarToVector(iAngle, (Metric)m_iRotationOffset);
			m_pRotationOffset[i].x = (int)vOffset.GetX();
			m_pRotationOffset[i].y = (int)vOffset.GetY();
			}
		}
	}

void CObjectImageArray::ComputeRotationOffsets (int xOffset, int yOffset)

//	ComputeRotationOffsets
//
//	Computer rotation offsets from a fixed offset

	{
	if (m_iRotationCount != 0)
		{
		if (m_pRotationOffset)
			delete [] m_pRotationOffset;

		m_pRotationOffset = new OffsetStruct[m_iRotationCount];
		for (int i = 0; i < m_iRotationCount; i++)
			{
			m_pRotationOffset[i].x = xOffset;
			m_pRotationOffset[i].y = yOffset;
			}
		}
	}

void CObjectImageArray::CopyFrom (const CObjectImageArray &Source)

//	CopyFrom
//
//	Copy image

	{
	m_dwBitmapUNID = Source.m_dwBitmapUNID;
	if (m_dwBitmapUNID || Source.m_pImage == NULL)
		m_pImage = Source.m_pImage;
	else
		m_pImage = new CObjectImage(Source.m_pImage->GetImage(NULL_STR));
	m_rcImage = Source.m_rcImage;
	m_iFrameCount = Source.m_iFrameCount;
	m_iRotationCount = Source.m_iRotationCount;
	m_iFramesPerColumn = Source.m_iFramesPerColumn;
	m_iTicksPerFrame = Source.m_iTicksPerFrame;
	m_iFlashTicks = Source.m_iFlashTicks;
	m_iBlending = Source.m_iBlending;
	m_iViewportSize = Source.m_iViewportSize;
	m_pGlowImages = NULL;
	m_pScaledImages = NULL;

	m_iRotationOffset = Source.m_iRotationOffset;
	if (Source.m_pRotationOffset)
		{
		m_pRotationOffset = new OffsetStruct[m_iRotationCount];
		for (int i = 0; i < m_iRotationCount; i++)
			m_pRotationOffset[i] = Source.m_pRotationOffset[i];
		}
	else
		m_pRotationOffset = NULL;
	}

void CObjectImageArray::CopyImage (CG16bitImage &Dest, int x, int y, int iFrame, int iRotation) const

//	CopyImage
//
//	Copies entire image to the destination

	{
	if (m_pImage)
		{
		CG16bitImage *pSource = m_pImage->GetImage(NULL_STR);
		if (pSource == NULL)
			return;

		int xSrc;
		int ySrc;
		ComputeSourceXY(iFrame, iRotation, &xSrc, &ySrc);

		Dest.Blt(xSrc,
				ySrc,
				RectWidth(m_rcImage),
				RectHeight(m_rcImage),
				*pSource,
				x,
				y);

		Dest.CopyAlpha(xSrc,
				ySrc,
				RectWidth(m_rcImage),
				RectHeight(m_rcImage),
				*pSource,
				x,
				y);
		}
	}

void CObjectImageArray::GenerateGlowImage (int iRotation) const

//	GenerateGlowImage
//
//	Generates a mask that looks like a glow. The mask is 0 for all image pixels
//	and for all pixels where there is no glow (thus we can optimize painting
//	of the glow by ignoring 0 values)

	{
	ASSERT(iRotation >= 0 && iRotation < m_iRotationCount);

	//	Source

	if (m_pImage == NULL)
		return;

	CG16bitImage *pSource = m_pImage->GetImage(NULL_STR);
	if (pSource == NULL)
		return;

	WORD wBackColor = pSource->GetBackColor();

	//	Allocate the array of images (if not already allocated)

	if (m_pGlowImages == NULL)
		m_pGlowImages = new CG16bitImage [m_iRotationCount];

	//	If the image for this rotation has already been initialized, then
	//	we're done

	if (m_pGlowImages[iRotation].HasAlpha())
		return;

	//	Otherwise we need to create the glow mask. The glow image is larger
	//	than the object image (by GLOW_SIZE)

	int cxSrcWidth = RectWidth(m_rcImage);
	int cySrcHeight = RectHeight(m_rcImage);
	int cxGlowWidth = cxSrcWidth + 2 * GLOW_SIZE;
	int cyGlowHeight = cySrcHeight + 2 * GLOW_SIZE;
	m_pGlowImages[iRotation].CreateBlankAlpha(cxGlowWidth, cyGlowHeight);

	//	Get the extent of the source image

	RECT rcSrc;
	ComputeSourceXY(0, iRotation, &rcSrc.left, &rcSrc.top);
	rcSrc.right = rcSrc.left + cxSrcWidth;
	rcSrc.bottom = rcSrc.top + cySrcHeight;

	//	Loop over every pixel of the destination

	BYTE *pDestRow = m_pGlowImages[iRotation].GetAlphaRow(0);
	BYTE *pDestRowEnd = m_pGlowImages[iRotation].GetAlphaRow(cyGlowHeight);
	int ySrc = rcSrc.top - GLOW_SIZE;
	while (pDestRow < pDestRowEnd)
		{
		BYTE *pDest = pDestRow;
		BYTE *pDestEnd = pDest + cxGlowWidth;
		int xSrc = rcSrc.left - GLOW_SIZE;
		while (pDest < pDestEnd)
			{
			//	If the source image is using this pixel then we don't
			//	do anything.

			WORD wValue;
			if ((xSrc >= rcSrc.left && xSrc < rcSrc.right && ySrc >= rcSrc.top && ySrc < rcSrc.bottom)
					&& ((wValue = *(pSource->GetRowStart(ySrc) + xSrc)) != wBackColor))
				{
				DWORD dwGray = (CG16bitImage::RedValue(wValue)
						+ CG16bitImage::GreenValue(wValue)
						+ CG16bitImage::BlueValue(wValue)) / 3;
				if (dwGray < 0x40)
					*pDest = 0x60;
				else
					*pDest = 0x00;
				}

			//	Otherwise we process the pixel

			else
				{
				int xStart = (rcSrc.left > (xSrc - GLOW_SIZE) ? ((rcSrc.left - (xSrc - GLOW_SIZE) + (GLOW_SIZE / 2 - 1)) / (GLOW_SIZE / 2)) : 0);
				int xEnd = ((xSrc + GLOW_SIZE) >= rcSrc.right ? (FILTER_SIZE - (((GLOW_SIZE / 2 + 1) + xSrc + GLOW_SIZE - rcSrc.right) / (GLOW_SIZE / 2))) : FILTER_SIZE);
				int yStart = (rcSrc.top > (ySrc - GLOW_SIZE) ? ((rcSrc.top - (ySrc - GLOW_SIZE) + (GLOW_SIZE / 2 - 1)) / (GLOW_SIZE / 2)) : 0);
				int yEnd = ((ySrc + GLOW_SIZE) >= rcSrc.bottom ? (FILTER_SIZE - (((GLOW_SIZE / 2 + 1) + ySrc + GLOW_SIZE - rcSrc.bottom) / (GLOW_SIZE / 2))) : FILTER_SIZE);

				int iTotal = 0;
				for (int i = yStart; i < yEnd; i++)
					for (int j = xStart; j < xEnd; j++)
						if (*(pSource->GetRowStart(ySrc + g_FilterOffset[i]) + xSrc + g_FilterOffset[j]) != wBackColor)
							iTotal += g_Filter[i][j];

				int iValue = (512 * iTotal / FIXED_POINT);
				*pDest = (iValue > 0xf8 ? 0xf8 : (BYTE)iValue);
				}

			//	Next

			pDest++;
			xSrc++;
			}

		pDestRow = m_pGlowImages[iRotation].NextAlphaRow(pDestRow);
		ySrc++;
		}
	}

void CObjectImageArray::GenerateScaledImages (int iRotation, int cxWidth, int cyHeight) const

//	GenerateScaledImages
//
//	Generate scaled images

	{
	ASSERT(iRotation >= 0 && iRotation < m_iRotationCount);

	//	Allocate the array of images (if not already allocated)

	if (m_pScaledImages == NULL)
		m_pScaledImages = new CG16bitImage [m_iRotationCount];

	//	If the image for this rotation has already been initialized, then
	//	we're done

	else if (m_pScaledImages[iRotation].HasRGB())
		return;

	//	Get the extent of the source image

	int cxSrcWidth = RectWidth(m_rcImage);
	int cySrcHeight = RectHeight(m_rcImage);

	CG16bitImage *pSource = m_pImage->GetImage(NULL_STR);
	if (pSource == NULL || cxSrcWidth == 0 || cySrcHeight == 0)
		return;

	RECT rcSrc;
	ComputeSourceXY(0, iRotation, &rcSrc.left, &rcSrc.top);
	rcSrc.right = rcSrc.left + cxSrcWidth;
	rcSrc.bottom = rcSrc.top + cySrcHeight;

	//	Scale

	m_pScaledImages[iRotation].CreateFromImageTransformed(*pSource,
			rcSrc.left,
			rcSrc.top,
			cxSrcWidth,
			cySrcHeight,
			(Metric)cxWidth / (Metric)cxSrcWidth,
			(Metric)cyHeight / (Metric)cySrcHeight,
			0.0);
	}

CString CObjectImageArray::GetFilename (void) const

//	GetFilename
//
//	Returns the filename used for the image

	{
	if (m_pImage == NULL)
		return NULL_STR;

	return m_pImage->GetImageFilename();
	}

bool CObjectImageArray::GetImageOffset (int iTick, int iRotation, int *retx, int *rety) const

//	GetImageOffset
//
//	Returns the image offset. Also returns FALSE if there is no image offset.
//	(But the return variables are always initialized.)

	{
	if (m_pRotationOffset)
		{
		*retx = m_pRotationOffset[iRotation % m_iRotationCount].x;
		*rety = -m_pRotationOffset[iRotation % m_iRotationCount].y;
		return true;
		}
	else
		{
		*retx = 0;
		*rety = 0;
		return false;
		}
	}

RECT CObjectImageArray::GetImageRect (int iTick, int iRotation, int *retxCenter, int *retyCenter) const

//	GetImageRect
//
//	Returns the rect of the image

	{
	RECT rcRect;

	ComputeSourceXY(iTick, iRotation, &rcRect.left, &rcRect.top);
	rcRect.right = rcRect.left + RectWidth(m_rcImage);
	rcRect.bottom = rcRect.top + RectHeight(m_rcImage);

	if (retxCenter)
		*retxCenter = rcRect.left + ((RectWidth(m_rcImage) / 2) - (m_pRotationOffset ? m_pRotationOffset[iRotation % m_iRotationCount].x : 0));

	if (retyCenter)
		*retyCenter = rcRect.top + ((RectHeight(m_rcImage) / 2) + (m_pRotationOffset ? m_pRotationOffset[iRotation % m_iRotationCount].y : 0));

	return rcRect;
	}

RECT CObjectImageArray::GetImageRectAtPoint (int x, int y) const

//	GetImageRectAtPoint
//
//	Returns the rect of the image centered at the given coordinates

	{
	RECT rcRect;
	int cxWidth = RectWidth(m_rcImage);
	int cyHeight = RectHeight(m_rcImage);

	rcRect.left = x - (cxWidth / 2);
	rcRect.right = rcRect.left + cxWidth;
	rcRect.top = y - (cyHeight / 2);
	rcRect.bottom = rcRect.top + cyHeight;

	return rcRect;
	}

int CObjectImageArray::GetImageViewportSize (void) const

//	GetImageViewportSize
//
//	Returns the size of the image viewport (in pixels). This is used to compute
//	perspective distortion.

	{
	return m_iViewportSize;
	}

bool CObjectImageArray::ImagesIntersect (int iTick, int iRotation, int x, int y, const CObjectImageArray &Image2, int iTick2, int iRotation2) const

//	ImagesIntersect
//
//	Returns TRUE if the given image intersects with this image

	{
	if (m_pImage == NULL || Image2.m_pImage == NULL)
		return false;

	//	Compute the rectangle of image1

	RECT rcRect;
	int cxWidth = RectWidth(m_rcImage);
	int cyHeight = RectHeight(m_rcImage);
	ComputeSourceXY(iTick, iRotation, &rcRect.left, &rcRect.top);
	rcRect.right = rcRect.left + cxWidth;
	rcRect.bottom = rcRect.top + cyHeight;

	//	Compute the rectangle of image2

	RECT rcRect2;
	int cxWidth2 = RectWidth(Image2.m_rcImage);
	int cyHeight2 = RectHeight(Image2.m_rcImage);
	Image2.ComputeSourceXY(iTick2, iRotation2, &rcRect2.left, &rcRect2.top);
	rcRect2.right = rcRect2.left + cxWidth2;
	rcRect2.bottom = rcRect2.top + cyHeight2;
	
	//	Now figure out the position of image2 on the image1 coordinate space

	int xCenter = rcRect.left + (cxWidth / 2);
	int yCenter = rcRect.top + (cyHeight / 2);

	RECT rcImage2On1;
	rcImage2On1.left = xCenter + x - (cxWidth2 / 2);
	rcImage2On1.top = yCenter + y - (cyHeight2 / 2);

	if (m_pRotationOffset)
		{
		rcImage2On1.left -= m_pRotationOffset[iRotation % m_iRotationCount].x;
		rcImage2On1.top += m_pRotationOffset[iRotation % m_iRotationCount].y;
		}

	if (Image2.m_pRotationOffset)
		{
		rcImage2On1.left += Image2.m_pRotationOffset[iRotation2 % Image2.m_iRotationCount].x;
		rcImage2On1.top -= Image2.m_pRotationOffset[iRotation2 % Image2.m_iRotationCount].y;
		}

	rcImage2On1.right = rcImage2On1.left + cxWidth2;
	rcImage2On1.bottom = rcImage2On1.top + cyHeight2;

	//	Intersect the rectangles

	RECT rcRectInt;
	if (!::IntersectRect(&rcRectInt, &rcRect, &rcImage2On1))
		return false;

	//	Figure out the position of image1 on the image2 coordinate space

	int xOffset = rcRect.left - rcImage2On1.left;
	int yOffset = rcRect.top - rcImage2On1.top;

	RECT rcImage1On2;
	rcImage1On2.left = rcRect2.left + xOffset;
	rcImage1On2.top = rcRect2.top + yOffset;
	rcImage1On2.right = rcImage1On2.left + cxWidth;
	rcImage1On2.bottom = rcImage1On2.top + cyHeight;

	//	Intersect the rectangles

	RECT rcRectInt2;
	::IntersectRect(&rcRectInt2, &rcRect2, &rcImage1On2);
	ASSERT(RectWidth(rcRectInt) == RectWidth(rcRectInt2));
	ASSERT(RectHeight(rcRectInt) == RectHeight(rcRectInt2));

	//	Images

	CG16bitImage *pSrc1 = m_pImage->GetImage(NULL_STR);
	CG16bitImage *pSrc2 = Image2.m_pImage->GetImage(NULL_STR);
	if (pSrc1 == NULL || pSrc2 == NULL)
		return false;

	//	If both rectangles have a mask

	if (pSrc1->HasAlpha() && pSrc2->HasAlpha())
		{
		//	Now iterate over the intersection area and see if there
		//	are any pixels in common.

		BYTE *pRow = pSrc1->GetAlphaRow(rcRectInt.top) + rcRectInt.left;
		BYTE *pRowEnd = pSrc1->GetAlphaRow(rcRectInt.bottom) + rcRectInt.left;
		BYTE *pRow2 = pSrc2->GetAlphaRow(rcRectInt2.top) + rcRectInt2.left;

		int cxWidthInt = RectWidth(rcRectInt);
		while (pRow < pRowEnd)
			{
			BYTE *pPos = pRow;
			BYTE *pEnd = pPos + cxWidthInt;
			BYTE *pPos2 = pRow2;

			while (pPos < pEnd)
				{
				if (*pPos && *pPos2)
					return true;

				pPos++;
				pPos2++;
				}

			pRow = pSrc1->NextAlphaRow(pRow);
			pRow2 = pSrc2->NextAlphaRow(pRow2);
			}
		}

	//	If neither has a mask

	else if (!pSrc1->HasAlpha() && !pSrc2->HasAlpha())
		{
		//	Now iterate over the intersection area and see if there
		//	are any pixels in common.

		WORD *pRow = pSrc1->GetRowStart(rcRectInt.top) + rcRectInt.left;
		WORD *pRowEnd = pSrc1->GetRowStart(rcRectInt.bottom) + rcRectInt.left;
		WORD *pRow2 = pSrc2->GetRowStart(rcRectInt2.top) + rcRectInt2.left;
		WORD wBackColor1 = pSrc1->GetBackColor();
		WORD wBackColor2 = pSrc2->GetBackColor();

		int cxWidthInt = RectWidth(rcRectInt);
		while (pRow < pRowEnd)
			{
			WORD *pPos = pRow;
			WORD *pEnd = pPos + cxWidthInt;
			WORD *pPos2 = pRow2;

			while (pPos < pEnd)
				{
				if (*pPos != wBackColor1 && *pPos2 != wBackColor2)
					return true;

				pPos++;
				pPos2++;
				}

			pRow = pSrc1->NextRow(pRow);
			pRow2 = pSrc2->NextRow(pRow2);
			}
		}

	//	If src1 has a mask and src2 does not

	else if (pSrc1->HasAlpha() && !pSrc2->HasAlpha())
		{
		//	Now iterate over the intersection area and see if there
		//	are any pixels in common.

		BYTE *pRow = pSrc1->GetAlphaRow(rcRectInt.top) + rcRectInt.left;
		BYTE *pRowEnd = pSrc1->GetAlphaRow(rcRectInt.bottom) + rcRectInt.left;
		WORD *pRow2 = pSrc2->GetRowStart(rcRectInt2.top) + rcRectInt2.left;
		WORD wBackColor2 = pSrc2->GetBackColor();

		int cxWidthInt = RectWidth(rcRectInt);
		while (pRow < pRowEnd)
			{
			BYTE *pPos = pRow;
			BYTE *pEnd = pPos + cxWidthInt;
			WORD *pPos2 = pRow2;

			while (pPos < pEnd)
				{
				if (*pPos && *pPos2 != wBackColor2)
					return true;

				pPos++;
				pPos2++;
				}

			pRow = pSrc1->NextAlphaRow(pRow);
			pRow2 = pSrc2->NextRow(pRow2);
			}
		}

	//	If src1 has no mask and src2 does

	else if (!pSrc1->HasAlpha() && pSrc2->HasAlpha())
		{
		//	Now iterate over the intersection area and see if there
		//	are any pixels in common.

		WORD *pRow = pSrc1->GetRowStart(rcRectInt.top) + rcRectInt.left;
		WORD *pRowEnd = pSrc1->GetRowStart(rcRectInt.bottom) + rcRectInt.left;
		BYTE *pRow2 = pSrc2->GetAlphaRow(rcRectInt2.top) + rcRectInt2.left;
		WORD wBackColor1 = pSrc1->GetBackColor();

		int cxWidthInt = RectWidth(rcRectInt);
		while (pRow < pRowEnd)
			{
			WORD *pPos = pRow;
			WORD *pEnd = pPos + cxWidthInt;
			BYTE *pPos2 = pRow2;

			while (pPos < pEnd)
				{
				if (*pPos != wBackColor1 && *pPos2)
					return true;

				pPos++;
				pPos2++;
				}

			pRow = pSrc1->NextRow(pRow);
			pRow2 = pSrc2->NextAlphaRow(pRow2);
			}
		}

	//	Can't happen

	else
		return false;

	//	If we get this far then we did not intersect

	return false;
	}

ALERROR CObjectImageArray::Init (CG16bitImage *pBitmap, const RECT &rcImage, int iFrameCount, int iTicksPerFrame, bool bFreeBitmap)

//	Init
//
//	Create from parameters

	{
	CleanUp();

	//	Initialize basic info

	m_dwBitmapUNID = 0;
	m_pImage = new CObjectImage(pBitmap, bFreeBitmap);
	m_rcImage = rcImage;
	m_iFrameCount = iFrameCount;
	m_iRotationCount = STD_ROTATION_COUNT;
	m_iFramesPerColumn = m_iRotationCount;
	m_iTicksPerFrame = iTicksPerFrame;
	m_iFlashTicks = 0;
	m_iRotationOffset = 0;
	m_pRotationOffset = NULL;
	m_iBlending = blendNormal;
	m_iViewportSize = RectWidth(rcImage);

	return NOERROR;
	}

ALERROR CObjectImageArray::Init (DWORD dwBitmapUNID, const RECT &rcImage, int iFrameCount, int iTicksPerFrame)

//	Init
//
//	Create from parameters

	{
	CleanUp();

	//	Initialize basic info

	m_dwBitmapUNID = dwBitmapUNID;
	m_pImage = g_pUniverse->FindLibraryImage(m_dwBitmapUNID);
	m_rcImage = rcImage;
	m_iFrameCount = iFrameCount;
	m_iRotationCount = STD_ROTATION_COUNT;
	m_iFramesPerColumn = m_iRotationCount;
	m_iTicksPerFrame = iTicksPerFrame;
	m_iFlashTicks = 0;
	m_iRotationOffset = 0;
	m_pRotationOffset = NULL;
	m_iBlending = blendNormal;
	m_iViewportSize = RectWidth(rcImage);

	return NOERROR;
	}

ALERROR CObjectImageArray::InitFromXML (CXMLElement *pDesc)

//	InitFromXML

	{
	SDesignLoadCtx Ctx;

	return InitFromXML(Ctx, pDesc, true);
	}

ALERROR CObjectImageArray::InitFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, bool bResolveNow, int iDefaultRotationCount)

//	InitFromXML
//
//	Create from XML description

	{
	ALERROR error;

	//	Initialize basic info

	m_rcImage.left = pDesc->GetAttributeInteger(CONSTLIT(g_ImageXAttrib));
	m_rcImage.top = pDesc->GetAttributeInteger(CONSTLIT(g_ImageYAttrib));
	m_rcImage.right = m_rcImage.left + pDesc->GetAttributeInteger(CONSTLIT(g_ImageWidthAttrib));
	m_rcImage.bottom = m_rcImage.top + pDesc->GetAttributeInteger(CONSTLIT(g_ImageHeightAttrib));
	m_iFrameCount = pDesc->GetAttributeInteger(CONSTLIT(g_ImageFrameCountAttrib));
	m_iRotationCount = pDesc->GetAttributeInteger(ROTATION_COUNT_ATTRIB);
	if (m_iRotationCount <= 0)
		m_iRotationCount = iDefaultRotationCount;

	int iRotationCols = pDesc->GetAttributeIntegerBounded(ROTATION_COLUMNS_ATTRIB, 0, m_iRotationCount, 0);
	if (iRotationCols > 0)
		m_iFramesPerColumn = (m_iRotationCount + iRotationCols - 1) / iRotationCols;
	else
		m_iFramesPerColumn = m_iRotationCount;

	m_iTicksPerFrame = pDesc->GetAttributeInteger(CONSTLIT(g_ImageTicksPerFrameAttrib));
	if (m_iTicksPerFrame <= 0 && m_iFrameCount > 1)
		m_iTicksPerFrame = 1;
	m_iFlashTicks = pDesc->GetAttributeInteger(FLASH_TICKS_ATTRIB);

	CString sBlending = pDesc->GetAttribute(BLENDING_ATTRIB);
	if (strEquals(sBlending, LIGHTEN_BLENDING))
		m_iBlending = blendLighten;
	else
		m_iBlending = blendNormal;

	//	Viewport

	if (pDesc->FindAttributeInteger(VIEWPORT_SIZE_ATTRIB, &m_iViewportSize))
		m_iViewportSize = Max(1, m_iViewportSize);
	else
		m_iViewportSize = RectWidth(m_rcImage);

	//	Compute rotation offsets

	m_iRotationOffset = pDesc->GetAttributeInteger(ROTATE_OFFSET_ATTRIB);
	if (m_iRotationOffset)
		ComputeRotationOffsets();
	else
		{
		int xOffset = pDesc->GetAttributeInteger(X_OFFSET_ATTRIB);
		int yOffset = pDesc->GetAttributeInteger(Y_OFFSET_ATTRIB);
		if (xOffset != 0 || yOffset != 0)
			ComputeRotationOffsets(xOffset, yOffset);
		}

	//	Get the image from the universe

	if (error = LoadUNID(Ctx, pDesc->GetAttribute(CONSTLIT(g_ImageIDAttrib)), &m_dwBitmapUNID))
		return error;

	if (bResolveNow)
		m_pImage = g_pUniverse->FindLibraryImage(m_dwBitmapUNID);
	else
		m_pImage = NULL;

	return NOERROR;
	}

void CObjectImageArray::MarkImage (void)

//	MarkImage
//
//	Mark image so that the sweeper knows that it is in use

	{
	if (m_pImage)
		m_pImage->Mark();
	}

ALERROR CObjectImageArray::OnDesignLoadComplete (SDesignLoadCtx &Ctx)

//	OnDesignLoadComplete
//
//	All design elements have been loaded

	{
	DEBUG_TRY

#ifdef NO_RESOURCES
	if (Ctx.bNoResources)
		return NOERROR;
#endif

	if (m_dwBitmapUNID)
		{
		m_pImage = g_pUniverse->FindLibraryImage(m_dwBitmapUNID);

		if (m_pImage == NULL)
			{
			Ctx.sError = strPatternSubst(CONSTLIT("Unknown image: %x"), m_dwBitmapUNID);
			return ERR_FAIL;
			}
		}

	return NOERROR;

	DEBUG_CATCH
	}

void CObjectImageArray::PaintImage (CG16bitImage &Dest, int x, int y, int iTick, int iRotation, bool bComposite) const

//	PaintImage
//
//	Paints the image on the destination

	{
	if (m_pImage)
		{
		CG16bitImage *pSource = m_pImage->GetImage(NULL_STR);
		if (pSource == NULL)
			return;

		int xSrc;
		int ySrc;
		ComputeSourceXY(iTick, iRotation, &xSrc, &ySrc);

		if (m_pRotationOffset)
			{
			x += m_pRotationOffset[iRotation % m_iRotationCount].x;
			y -= m_pRotationOffset[iRotation % m_iRotationCount].y;
			}

		if (bComposite)
			{
			Dest.CompositeTransBlt(xSrc,
					ySrc,
					RectWidth(m_rcImage),
					RectHeight(m_rcImage),
					255,
					*pSource,
					x - (RectWidth(m_rcImage) / 2),
					y - (RectHeight(m_rcImage) / 2));
			}
		else if (m_iBlending == blendLighten)
			{
			Dest.BltLighten(xSrc,
					ySrc,
					RectWidth(m_rcImage),
					RectHeight(m_rcImage),
					255,
					*pSource,
					x - (RectWidth(m_rcImage) / 2),
					y - (RectHeight(m_rcImage) / 2));
			}
		else
			{
			Dest.ColorTransBlt(xSrc,
					ySrc,
					RectWidth(m_rcImage),
					RectHeight(m_rcImage),
					255,
					*pSource,
					x - (RectWidth(m_rcImage) / 2),
					y - (RectHeight(m_rcImage) / 2));
			}
		}
	}

void CObjectImageArray::PaintImageShimmering (CG16bitImage &Dest, int x, int y, int iTick, int iRotation, DWORD byOpacity) const

//	PaintImageShimmering
//
//	Paint a distorted/invisible image.
//
//	byOpacity = 0 means the image is invisible.
//	byOpacity = 256 means the image is fully visible.

	{
	if (m_pImage)
		{
		CG16bitImage *pSource = m_pImage->GetImage(NULL_STR);
		if (pSource == NULL)
			return;

		int xSrc;
		int ySrc;
		ComputeSourceXY(iTick, iRotation, &xSrc, &ySrc);

		if (m_pRotationOffset)
			{
			x += m_pRotationOffset[iRotation % m_iRotationCount].x;
			y -= m_pRotationOffset[iRotation % m_iRotationCount].y;
			}

		DrawBltShimmer(Dest,
				x - (RectWidth(m_rcImage) / 2),
				y - (RectHeight(m_rcImage) / 2),
				RectWidth(m_rcImage),
				RectHeight(m_rcImage),
				*pSource,
				xSrc,
				ySrc,
				byOpacity,
				iTick);
		}
	}

void CObjectImageArray::PaintImageGrayed (CG16bitImage &Dest, int x, int y, int iTick, int iRotation) const

//	PaintImageGrayed
//
//	Paints the image on the destination

	{
	if (m_pImage)
		{
		CG16bitImage *pSource = m_pImage->GetImage(NULL_STR);
		if (pSource == NULL)
			return;

		int xSrc;
		int ySrc;
		ComputeSourceXY(iTick, iRotation, &xSrc, &ySrc);

		if (m_pRotationOffset)
			{
			x += m_pRotationOffset[iRotation % m_iRotationCount].x;
			y -= m_pRotationOffset[iRotation % m_iRotationCount].y;
			}

		Dest.BltGray(xSrc,
				ySrc,
				RectWidth(m_rcImage),
				RectHeight(m_rcImage),
				128,
				*pSource,
				x - (RectWidth(m_rcImage) / 2),
				y - (RectHeight(m_rcImage) / 2));
		}
	}

void CObjectImageArray::PaintImageUL (CG16bitImage &Dest, int x, int y, int iTick, int iRotation) const

//	PaintImageUL
//
//	Paints the image. x,y is the upper-left corner of the destination
//
//	Note: This should not use the rotation offsets

	{
	if (m_pImage)
		{
		CG16bitImage *pSource = m_pImage->GetImage(NULL_STR);
		if (pSource == NULL)
			return;

		int xSrc;
		int ySrc;
		ComputeSourceXY(iTick, iRotation, &xSrc, &ySrc);

		if (m_iBlending == blendLighten)
			{
			Dest.BltLighten(xSrc,
					ySrc,
					RectWidth(m_rcImage),
					RectHeight(m_rcImage),
					255,
					*pSource,
					x,
					y);
			}
		else
			{
			Dest.ColorTransBlt(xSrc,
					ySrc,
					RectWidth(m_rcImage),
					RectHeight(m_rcImage),
					255,
					*pSource,
					x,
					y);
			}
		}
	}

void CObjectImageArray::PaintImageWithGlow (CG16bitImage &Dest,
											int x,
											int y,
											int iTick,
											int iRotation,
											COLORREF rgbGlowColor) const

//	PaintImageWithGlow
//
//	Paints the image on the destination with a pulsating glow around
//	it of the specified color.
//
//	This effect does not work with blending modes.

	{
	//	Paint the image

	PaintImage(Dest, x, y, iTick, iRotation);

	if (m_pRotationOffset)
		{
		x += m_pRotationOffset[iRotation % m_iRotationCount].x;
		y -= m_pRotationOffset[iRotation % m_iRotationCount].y;
		}

	//	Make sure we have the glow image

	GenerateGlowImage(iRotation);

	//	Glow strength

	int iStrength = 64 + (4 * Absolute((iTick % 65) - 32));
	if (iStrength > 255)
		iStrength = 255;

	//	Paint the glow

	Dest.FillMask(0,
			0,
			RectWidth(m_rcImage) + 2 * GLOW_SIZE,
			RectHeight(m_rcImage) + 2 * GLOW_SIZE,
			m_pGlowImages[iRotation],
			(WORD)CG16bitImage::PixelFromRGB(rgbGlowColor),
			x - (RectWidth(m_rcImage) / 2) - GLOW_SIZE,
			y - (RectHeight(m_rcImage) / 2) - GLOW_SIZE,
			(BYTE)iStrength);
	}

void CObjectImageArray::PaintRotatedImage (CG16bitImage &Dest,
										   int x,
										   int y,
										   int iTick,
										   int iRotation,
										   bool bComposite) const

//	PaintRotatedImage
//
//	Paint rotated image

	{
	if (m_pImage == NULL)
		return;

	CG16bitImage *pSource = m_pImage->GetImage(NULL_STR);
	if (pSource == NULL)
		return;

	int xSrc;
	int ySrc;
	ComputeSourceXY(iTick, 0, &xSrc, &ySrc);
	int cxSrc = RectWidth(m_rcImage);
	int cySrc = RectHeight(m_rcImage);

	//	Calculate the position of the upper-left corner of
	//	the rotated image.

#if 0
	CVector vUL(-cxSrc / 2, cySrc / 2);
	vUL = vUL.Rotate(iRotation);
#endif

	//	LATER: Since we have an actual rotation angle, we can calculate
	//	any rotation offset directly (instead of of using m_pRotationOffset)
	//	(But we are too lazy to do that now)

	ASSERT(m_pRotationOffset == NULL);

	//	Blt

	DrawBltRotated(Dest,
			x,
			y,
			iRotation,
			*pSource,
			xSrc,
			ySrc,
			cxSrc,
			cySrc);
	}

void CObjectImageArray::PaintScaledImage (CG16bitImage &Dest,
										  int x,
										  int y,
										  int iTick,
										  int iRotation,
										  int cxWidth,
										  int cyHeight,
										  bool bComposite) const

//	PaintScaledImage
//
//	Paint scaled image

	{
	//	Make sure we have the scaled image

	GenerateScaledImages(iRotation, cxWidth, cyHeight);

	//	Paint the image

	Dest.ColorTransBlt(0,
			0,
			cxWidth,
			cyHeight,
			255,
			m_pScaledImages[iRotation],
			x - (cxWidth / 2),
			y - (cyHeight / 2));
	}

void CObjectImageArray::PaintSilhoutte (CG16bitImage &Dest,
										int x,
										int y,
										int iTick,
										int iRotation,
										WORD wColor) const

//	PaintSilhouette
//
//	Paints a silhouette of the object

	{
	if (m_pImage)
		{
		CG16bitImage *pSource = m_pImage->GetImage(NULL_STR);
		if (pSource == NULL)
			return;

		int xSrc;
		int ySrc;
		ComputeSourceXY(iTick, iRotation, &xSrc, &ySrc);

		if (m_pRotationOffset)
			{
			x += m_pRotationOffset[iRotation % m_iRotationCount].x;
			y -= m_pRotationOffset[iRotation % m_iRotationCount].y;
			}

		Dest.FillMask(xSrc,
				ySrc,
				RectWidth(m_rcImage),
				RectHeight(m_rcImage),
				*pSource,
				wColor,
				x - (RectWidth(m_rcImage) / 2),
				y - (RectHeight(m_rcImage) / 2));
		}
	}

bool CObjectImageArray::PointInImage (int x, int y, int iTick, int iRotation) const

//	PointInImage
//
//	Returns TRUE if the given point is inside the masked part of the image
//	x, y is relative to the center of the image (GDI directions)

	{
	if (m_pImage)
		{
		CG16bitImage *pSource = m_pImage->GetImage(NULL_STR);
		if (pSource == NULL)
			return false;

		//	Compute the position of the frame

		int cxWidth = RectWidth(m_rcImage);
		int cyHeight = RectHeight(m_rcImage);
		int xSrc;
		int ySrc;
		ComputeSourceXY(iTick, iRotation, &xSrc, &ySrc);

		//	Adjust the point so that it is relative to the
		//	frame origin (upper left)

		x = xSrc + x + (cxWidth / 2);
		y = ySrc + y + (cyHeight / 2);

		//	Adjust for rotation

		if (m_pRotationOffset)
			{
			x -= m_pRotationOffset[iRotation % m_iRotationCount].x;
			y += m_pRotationOffset[iRotation % m_iRotationCount].y;
			}

		//	Check bounds

		if (x < xSrc || y < ySrc || x >= (xSrc + cxWidth) || y >= (ySrc + cyHeight))
			return false;

		//	Check to see if the point is inside or outside the mask

		return (pSource->GetPixelAlpha(x, y) != 0);
		}
	else
		return false;
	}

bool CObjectImageArray::PointInImage (SPointInObjectCtx &Ctx, int x, int y) const

//	PointInImage
//
//	Returns TRUE if the given point is in the image

	{
	if (Ctx.pImage)
		{
		//	Adjust to image coords

		x = x + Ctx.xImageOffset;
		y = y + Ctx.yImageOffset;

		//	Check bounds

		if (x < Ctx.rcImage.left || y < Ctx.rcImage.top || x >= Ctx.rcImage.right || y >= Ctx.rcImage.bottom)
			return false;

		//	Check to see if the point is inside or outside the mask

		return (Ctx.pImage->GetPixelAlpha(x, y) != 0);
		}
	else
		return false;
	}

void CObjectImageArray::PointInImageInit (SPointInObjectCtx &Ctx, int iTick, int iRotation) const

//	PointInImageInit
//
//	Initializes the context to optimize the PointInImage call

	{
	if (m_pImage)
		{
		Ctx.pImage = m_pImage->GetImage(NULL_STR);
		if (Ctx.pImage == NULL)
			return;

		//	Compute the position of the frame

		int cxWidth = RectWidth(m_rcImage);
		int cyHeight = RectHeight(m_rcImage);
		ComputeSourceXY(iTick, iRotation, &Ctx.rcImage.left, &Ctx.rcImage.top);
		Ctx.rcImage.right = Ctx.rcImage.left + cxWidth;
		Ctx.rcImage.bottom = Ctx.rcImage.top + cyHeight;

		//	Compute the offset required to convert to image coordinates

		Ctx.xImageOffset = Ctx.rcImage.left + (cxWidth / 2);
		Ctx.yImageOffset = Ctx.rcImage.top + (cyHeight / 2);

		//	Adjust for rotation

		if (m_pRotationOffset)
			{
			Ctx.xImageOffset -= m_pRotationOffset[iRotation % m_iRotationCount].x;
			Ctx.yImageOffset += m_pRotationOffset[iRotation % m_iRotationCount].y;
			}
		}
	}

void CObjectImageArray::ReadFromStream (SLoadCtx &Ctx)

//	ReadFromStream
//
//	Read the object from a stream

	{
	Ctx.pStream->Read((char *)&m_dwBitmapUNID, sizeof(DWORD));
	if (m_dwBitmapUNID)
		m_pImage = g_pUniverse->FindLibraryImage(m_dwBitmapUNID);
	else
		m_pImage = NULL;
	Ctx.pStream->Read((char *)&m_rcImage, sizeof(RECT));
	Ctx.pStream->Read((char *)&m_iFrameCount, sizeof(DWORD));
	Ctx.pStream->Read((char *)&m_iTicksPerFrame, sizeof(DWORD));
	Ctx.pStream->Read((char *)&m_iFlashTicks, sizeof(DWORD));
	Ctx.pStream->Read((char *)&m_iBlending, sizeof(DWORD));

	if (Ctx.dwVersion >= 17)
		Ctx.pStream->Read((char *)&m_iRotationCount, sizeof(DWORD));
	else
		m_iRotationCount = STD_ROTATION_COUNT;

	if (Ctx.dwVersion >= 104)
		Ctx.pStream->Read((char *)&m_iFramesPerColumn, sizeof(DWORD));
	else
		m_iFramesPerColumn = m_iRotationCount;

	if (Ctx.dwVersion >= 90)
		Ctx.pStream->Read((char *)&m_iViewportSize, sizeof(DWORD));
	else
		m_iViewportSize = RectWidth(m_rcImage);

	DWORD dwLoad;
	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	if (dwLoad)
		{
		m_pRotationOffset = new OffsetStruct [dwLoad];
		Ctx.pStream->Read((char *)m_pRotationOffset, dwLoad * sizeof(OffsetStruct));
		}
	}

void CObjectImageArray::SetRotationCount (int iRotationCount)

//	SetRotationCount
//
//	Sets the rotation count

	{
	if (iRotationCount != m_iRotationCount)
		{
		m_iRotationCount = iRotationCount;
		m_iFramesPerColumn = iRotationCount;

		ComputeRotationOffsets();

		if (m_pGlowImages)
			{
			delete [] m_pGlowImages;
			m_pGlowImages = NULL;
			}

		if (m_pScaledImages)
			{
			delete [] m_pScaledImages;
			m_pScaledImages = NULL;
			}
		}
	}

void CObjectImageArray::TakeHandoff (CObjectImageArray &Source)

//	TakeHandoff
//
//	Take ownership of the bitmap (and clear out the source)

	{
	CleanUp();

	//	Take ownership

	m_pImage = Source.m_pImage;
	Source.m_pImage = NULL;

	m_pGlowImages = Source.m_pGlowImages;
	Source.m_pGlowImages = NULL;

	m_pScaledImages = Source.m_pScaledImages;
	Source.m_pScaledImages = NULL;

	m_pRotationOffset = Source.m_pRotationOffset;
	Source.m_pRotationOffset = NULL;

	//	Copy the remainder

	m_dwBitmapUNID = Source.m_dwBitmapUNID;
	m_rcImage = Source.m_rcImage;
	m_iFrameCount = Source.m_iFrameCount;
	m_iRotationCount = Source.m_iRotationCount;
	m_iFramesPerColumn = Source.m_iFramesPerColumn;
	m_iTicksPerFrame = Source.m_iTicksPerFrame;
	m_iFlashTicks = Source.m_iFlashTicks;
	m_iBlending = Source.m_iBlending;
	m_iViewportSize = Source.m_iViewportSize;
	m_iRotationOffset = Source.m_iRotationOffset;
	}

void CObjectImageArray::WriteToStream (IWriteStream *pStream) const

//	WriteToStream
//
//	Writes the object to a stream
//
//	DWORD		m_dwBitmapUNID
//	Rect		m_rcImage
//	DWORD		m_iFrameCount
//	DWORD		m_iTicksPerFrame
//	DWORD		m_iFlashTicks
//	DWORD		m_iBlending
//	DWORD		m_iRotationCount
//	DWORD		m_iFramesPerColumns
//	DWORD		m_iViewportSize

//	DWORD		No of rotation offsets
//	DWORD		x
//	DWORD		y
//				...

	{
	pStream->Write((char *)&m_dwBitmapUNID, sizeof(DWORD));
	pStream->Write((char *)&m_rcImage, sizeof(m_rcImage));
	pStream->Write((char *)&m_iFrameCount, sizeof(DWORD));
	pStream->Write((char *)&m_iTicksPerFrame, sizeof(DWORD));
	pStream->Write((char *)&m_iFlashTicks, sizeof(DWORD));
	pStream->Write((char *)&m_iBlending, sizeof(DWORD));
	pStream->Write((char *)&m_iRotationCount, sizeof(DWORD));
	pStream->Write((char *)&m_iFramesPerColumn, sizeof(DWORD));
	pStream->Write((char *)&m_iViewportSize, sizeof(DWORD));

	if (m_pRotationOffset)
		{
		DWORD dwSave = m_iRotationCount;
		pStream->Write((char *)&dwSave, sizeof(DWORD));

		pStream->Write((char *)m_pRotationOffset, m_iRotationCount * sizeof(OffsetStruct));
		}
	else
		{
		DWORD dwSave = 0;
		pStream->Write((char *)&dwSave, sizeof(DWORD));
		}
	}