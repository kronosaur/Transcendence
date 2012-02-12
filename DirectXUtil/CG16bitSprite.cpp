//	CG16bitSprite.cpp
//
//	Implementation of raw 16-bit sprite object

#include <windows.h>
#include <ddraw.h>
#include "Alchemy.h"
#include "DirectXUtil.h"

CG16bitSprite::CG16bitSprite (void) :
		m_pCode(NULL),
		m_pLineIndex(NULL),
		m_cxWidth(0),
		m_cyHeight(0)

//	CG16bitSprite constructor

	{
	}

CG16bitSprite::~CG16bitSprite (void)

//	CG16bitSprite destructor

	{
	DeleteData();
	}

void CG16bitSprite::ColorTransBlt (CG16bitImage &Dest, int xDest, int yDest, int xSrc, int ySrc, int cxWidth, int cyHeight)

//	ColorTransBlt
//
//	Blt the sprite

	{
	//	Make sure we're in bounds

	if (!Dest.AdjustCoords(&xSrc, &ySrc, m_cxWidth, m_cyHeight, 
			&xDest, &yDest,
			&cxWidth, &cyHeight))
		return;

	//	Blt each line

	WORD *pDstRow = Dest.GetPixel(Dest.GetRowStart(yDest), xDest);

	int iLine;
	int iLineEnd = ySrc + cyHeight;
	for (iLine = ySrc; iLine < iLineEnd; iLine++)
		{
		WORD *pCode = m_pLineIndex[iLine];
		int cxOffset = xSrc;
		int cxBlt = cxWidth;

		WORD *pDstPos = pDstRow;

		while (cxBlt > 0)
			{
			switch (*pCode)
				{
				case codeSkip:
					{
					pCode++;
					int cxCount = (int)(DWORD)(*pCode);
					pCode++;

					//	Deal with offset

					if (cxOffset)
						{
						int cxConsume = Min(cxCount, cxOffset);
						cxCount -= cxConsume;
						cxOffset -= cxConsume;
						}

					ASSERT(cxCount == 0 || cxOffset == 0);

					//	Skip any remaining

					if (cxCount > 0)
						{
						int cxConsume = Min(cxCount, cxBlt);
						cxBlt -= cxConsume;
						pDstPos += cxConsume;
						}

					break;
					}

				case codeRun:
					{
					pCode++;
					int cxCount = (int)(DWORD)(*pCode);
					pCode++;

					//	Deal with offset

					if (cxOffset)
						{
						int cxConsume = Min(cxCount, cxOffset);
						cxCount -= cxConsume;
						cxOffset -= cxConsume;
						pCode += cxConsume;
						}

					ASSERT(cxCount == 0 || cxOffset == 0);

					//	Blt any remaining

					if (cxCount > 0)
						{
						int cxConsume = Min(cxCount, cxBlt);

						WORD *pCodeEnd = pCode + cxConsume;
						while (pCode < pCodeEnd)
							*pDstPos++ = *pCode++;

						cxBlt -= cxConsume;

						ASSERT(cxBlt == 0 || cxConsume == cxCount);
						}

					break;
					}

				default:
					ASSERT(false);
				}
			}

		//	Next row

		pDstRow = Dest.NextRow(pDstRow);
		}
	}

ALERROR CG16bitSprite::CreateFromImage (const CG16bitImage &Source)

//	CreateFromImage
//
//	Creates the sprite from an image

	{
	ALERROR error;
	int i;

	//	There are many images types that we don't handle

	if (Source.HasAlpha() 
		|| Source.IsTransparent()
		|| Source.GetWidth() <= 0
		|| Source.GetHeight() <= 0)
		{
		ASSERT(false);
		return ERR_FAIL;
		}

	//	States

	enum States
		{
		stateStart,
		stateInSkip,
		stateInRun,
		};

	//	Initialize

	int cxWidth = Source.GetWidth();
	int cyHeight = Source.GetHeight();
	WORD wBackColor = Source.GetBackColor();

	//	Allocate line index and a temp code block

	CMemoryWriteStream Code;
	if (error = Code.Create())
		return ERR_FAIL;

	int *pLineIndex = new int [cyHeight];

	//	Init state

	int iState = stateStart;
	int iLine = 0;

	//	Traverse the image

	WORD *pSrcRow = Source.GetRowStart(0);
	WORD *pSrcRowEnd = Source.GetRowStart(cyHeight);

	while (pSrcRow < pSrcRowEnd)
		{
		WORD *pSrcPos = pSrcRow;
		WORD *pSrcPosEnd = pSrcRow + cxWidth;
		WORD *pStart;

		pLineIndex[iLine++] = (Code.GetLength() / sizeof(WORD));

		while (pSrcPos < pSrcPosEnd)
			{
			switch (iState)
				{
				case stateStart:
					{
					if (*pSrcPos == wBackColor)
						{
						pStart = pSrcPos;
						iState = stateInSkip;
						}
					else
						{
						pStart = pSrcPos;
						iState = stateInRun;
						}
					break;
					}

				case stateInSkip:
					{
					if (*pSrcPos != wBackColor)
						{
						WORD wCode = codeSkip;
						Code.Write((char *)&wCode, sizeof(WORD));
						WORD wCount = (WORD)(DWORD)(pSrcPos - pStart);
						Code.Write((char *)&wCount, sizeof(WORD));

						pStart = pSrcPos;
						iState = stateInRun;
						}
					break;
					}

				case stateInRun:
					{
					if (*pSrcPos == wBackColor)
						{
						WORD wCode = codeRun;
						Code.Write((char *)&wCode, sizeof(WORD));
						WORD wCount = (WORD)(DWORD)(pSrcPos - pStart);
						Code.Write((char *)&wCount, sizeof(WORD));
						Code.Write((char *)pStart, wCount * sizeof(WORD));

						pStart = pSrcPos;
						iState = stateInSkip;
						}
					break;
					}
				}

			pSrcPos++;
			}

		//	Done with row, so end code

		if (iState == stateInSkip)
			{
			WORD wCode = codeSkip;
			Code.Write((char *)&wCode, sizeof(WORD));
			WORD wCount = (WORD)(DWORD)(pSrcPos - pStart);
			Code.Write((char *)&wCount, sizeof(WORD));
			}
		else if (iState == stateInRun)
			{
			WORD wCode = codeRun;
			Code.Write((char *)&wCode, sizeof(WORD));
			WORD wCount = (WORD)(DWORD)(pSrcPos - pStart);
			Code.Write((char *)&wCount, sizeof(WORD));
			Code.Write((char *)pStart, wCount * sizeof(WORD));
			}

		iState = stateStart;

		//	Next

		pSrcRow = Source.NextRow(pSrcRow);
		}

	//	Convert to compact format

	DeleteData();

	int iCodeSize = Code.GetLength() / sizeof(WORD);
	m_pCode = new WORD [iCodeSize];
	utlMemCopy((char *)Code.GetPointer(), (char *)m_pCode, iCodeSize * sizeof(WORD));

	m_pLineIndex = new WORD *[cyHeight];
	for (i = 0; i < cyHeight; i++)
		m_pLineIndex[i] = m_pCode + pLineIndex[i];

	m_cxWidth = cxWidth;
	m_cyHeight = cyHeight;

	//	Done

	delete [] pLineIndex;

	return NOERROR;
	}

void CG16bitSprite::DeleteData (void)

//	DeleteData
//
//	Delete all data

	{
	if (m_pCode)
		{
		delete m_pCode;
		m_pCode = NULL;
		}

	if (m_pLineIndex)
		{
		delete m_pLineIndex;
		m_pLineIndex = NULL;
		}

	m_cxWidth = 0;
	m_cyHeight = 0;
	}
