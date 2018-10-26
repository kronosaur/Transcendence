//	CGNeurohackArea.cpp
//
//	CGNeurohackArea class

#include "PreComp.h"
#include "Transcendence.h"

const CG32bitPixel RGB_BACKGROUND =					CG32bitPixel(0,0,0,0);
const CG32bitPixel RGB_LINE =						CG32bitPixel(255,255,255);
const CG32bitPixel RGB_LIGHTNING =					CG32bitPixel(192,192,255);

CGNeurohackArea::CGNeurohackArea (void) :
		m_pFonts(NULL),
		m_pNetwork(NULL),
		m_pRootNodes(NULL),
		m_pTerminalNodes(NULL),
		m_pActiveNodes(NULL),
		m_iWillpower(10000),
		m_iDamage(0)

//	CGNeurohackArea constructor

	{
	}

CGNeurohackArea::~CGNeurohackArea (void)

//	CGNeurohackArea destructor

	{
	CleanUp();
	}

void CGNeurohackArea::CleanUp (void)

//	CleanUp
//
//	Delete all structures

	{
	if (m_pNetwork)
		{
		delete m_pNetwork;
		m_pNetwork = NULL;

		delete m_pRootNodes;
		m_pRootNodes = NULL;

		delete m_pTerminalNodes;
		m_pTerminalNodes = NULL;

		delete m_pActiveNodes;
		m_pActiveNodes = NULL;
		}
	}

void CGNeurohackArea::CreateBranch (SNode *pParent, int iDirection, int iGeneration, int iWidth, const RECT &rcRect)

//	CreateBranch
//
//	Creates a branch that expands in the given direction

	{
	int i;
	int iBranchLength = 0;

	//	Calculate how long we will go before splitting

	int iSplitPoint = Max(3, 6 - iGeneration);

	//	Create nodes until we're done

	bool bTerminal = false;
	int iCurve = 0;
	int iCurveInc = (mathRandom(1, 2) == 1 ? mathRandom(-10, -5) : mathRandom(5, 10));
	while (!bTerminal && iBranchLength < iSplitPoint)
		{
		//	Compute the length of this segment

		int iLength = 2 * Max(0, (6 - iGeneration)) + mathRandom(3, 6);

		//	Generate coordinates of new node

		int x, y;
		IntPolarToVector((360 + iDirection + iCurve) % 360, iLength, &x, &y);
		iCurve += iCurveInc;
		x = pParent->x + x;
		y = pParent->y + y;

		//	If the new coordinates are outside the rect, then we're done

		if (x >= rcRect.right || x < rcRect.left || y >= rcRect.bottom || y < rcRect.top)
			{
			bTerminal = true;
			break;
			}

		//	Create the new node

		SNode *pNewNode = CreateNode(pParent, x, y);
		if (pNewNode == NULL)
			{
			bTerminal = true;
			break;
			}

		pNewNode->iWidth = iWidth;
		if (iWidth > 1)
			iWidth--;

		//	Next

		pParent = pNewNode;
		iBranchLength++;
		}

	//	If we've reached the end, then add us to the list of endpoint

	if (bTerminal)
		{
		if (m_iTerminalCount < m_iTerminalAlloc)
			m_pTerminalNodes[m_iTerminalCount++] = pParent;
		}

	//	Otherwise, we split into sub-branches

	else if (iGeneration < 6)
		{
		if (mathRandom(1, 16) == 1)
			pParent->iSphereSize = 6 * (6 - iGeneration);

		int iSplitCount = mathRandom(2, 3);
		int iSep = mathRandom(15, 45);
		int iOffset = -(iSplitCount * iSep / 2) + mathRandom(-15, 15);
		for (i = 0; i < iSplitCount; i++)
			{
			int iSplitDir = (iDirection + iCurve + (i * iSep) + iOffset + 360) % 360;
			CreateBranch(pParent, iSplitDir, iGeneration + 1, iWidth, rcRect);
			}
		}
	}

void CGNeurohackArea::CreateNetwork (const RECT &rcRect)

//	CreateNetwork
//
//	Create a network of nodes

	{
	int i;

	//	Allocate the network

	ASSERT(m_pNetwork == NULL);

	m_iNodeCount = 0;
	m_iNodeAlloc = 20000;
	m_pNetwork = new SNode [m_iNodeAlloc];

	m_iRootCount = 0;
	m_iRootAlloc = 20;
	m_pRootNodes = new SNode * [m_iRootAlloc];

	m_iTerminalCount = 0;
	m_iTerminalAlloc = 200;
	m_pTerminalNodes = new SNode * [m_iTerminalAlloc];

	m_iActiveCount = 0;
	m_iActiveAlloc = 50;
	m_pActiveNodes = new SNode * [m_iActiveAlloc];

	//	Create a number of branches starting at the center and
	//	expanding outward in a random direction

	int xCenter = rcRect.left + RectWidth(rcRect) / 2;
	int yCenter = rcRect.top + RectHeight(rcRect) / 2;

	int iBranchCount = mathRandom(15, 20);
	int iBranchAngle10 = 3600 / iBranchCount;
	int iAngle = mathRandom(0, 359);

	for (i = 0; i < iBranchCount && m_iRootCount < m_iRootAlloc; i++)
		{
		SNode *pNode = CreateNode(NULL, xCenter, yCenter);
		if (pNode == NULL)
			break;

		if (i == 0)
			pNode->iSphereSize = 60;

		//	Add to root

		m_pRootNodes[m_iRootCount++] = pNode;

		//	Create a branch

		CreateBranch(pNode, 
				(iAngle + mathRandom(-12, 12) + 360 + (i * iBranchAngle10) / 10) % 360,
				0, 
				25, 
				rcRect);
		}
	}

CGNeurohackArea::SNode *CGNeurohackArea::CreateNode (SNode *pParent, int x, int y)

//	CreateNode
//
//	Creates a new node

	{
	if (m_iNodeCount == m_iNodeAlloc)
		return NULL;

	SNode *pNode = &m_pNetwork[m_iNodeCount++];
	pNode->x = x;
	pNode->y = y;
	pNode->iSphereSize = 0;
	pNode->pParent = pParent;
	pNode->pFirstChild = NULL;
	if (pParent)
		{
		pNode->pNext = pParent->pFirstChild;
		if (pParent->pFirstChild)
			pParent->pFirstChild->pPrev = pNode;
		pParent->pFirstChild = pNode;
		pNode->pPrev = NULL;

		pNode->iRootDist = pParent->iRootDist + 1;
		}
	else
		{
		pNode->pNext = NULL;
		pNode->pPrev = NULL;

		pNode->iRootDist = 0;
		}

	pNode->iDamageLevel = Max(1, (30 - pNode->iRootDist) + mathRandom(-1, 1));

	return pNode;
	}

void CGNeurohackArea::Paint (CG32bitImage &Dest, const RECT &rcRect)

//	Paint
//
//	Paint event

	{
	DEBUG_TRY

	int i;

	//	Erase background

	Dest.Fill(rcRect.left,
			rcRect.top,
			RectWidth(rcRect),
			RectHeight(rcRect),
			RGB_BACKGROUND);

	//	Create the network, if necessary

	if (m_pNetwork == NULL)
		CreateNetwork(rcRect);

	//	Paint all the branches

	for (i = 0; i < m_iRootCount; i++)
		PaintBranch(Dest, m_pRootNodes[i]);

	//	Paint neural lightning

	for (i = 0; i < m_iActiveCount; i++)
		{
		SNode *pEnd = m_pActiveNodes[i];
		SNode *pStart = pEnd->pParent;
		if (pStart && m_iDamage <= pEnd->iDamageLevel)
			DrawLightning(Dest, pStart->x, pStart->y, pEnd->x, pEnd->y, RGB_LIGHTNING, 8, 0.5);
		}

	//	Paint central sphere

	int xCenter = rcRect.left + RectWidth(rcRect) / 2;
	int yCenter = rcRect.top + RectHeight(rcRect) / 2;
	CG32bitPixel rgbColor = (m_iWillpower > 0 ? CG32bitPixel(255,255,255) : CG32bitPixel(128,255,128));
	PaintSphere(Dest, xCenter, yCenter, 40, rgbColor);

	DEBUG_CATCH
	}

void CGNeurohackArea::PaintBranch (CG32bitImage &Dest, SNode *pNode, SNode *pNext)

//	PaintBranch
//
//	Paints the branch

	{
	SNode *pStart = pNode;
	SNode *pEnd = (pNext ? pNext : pNode->pFirstChild);

	while (pEnd && (pEnd->pNext == NULL || pEnd == pNext))
		{
		BYTE byLevel = (BYTE)(pEnd->iWidth >= 10 ? 255 : (WORD)(255 * pEnd->iWidth / 10));
		CG32bitPixel rgbColor = (m_iDamage > pEnd->iDamageLevel ? CG32bitPixel(byLevel, 0, 0) : CG32bitPixel(byLevel, byLevel, byLevel));

		Dest.DrawLine(pStart->x, pStart->y, pEnd->x, pEnd->y, pEnd->iWidth / 10, rgbColor);

		pStart = pEnd;
		pEnd = pEnd->pFirstChild;
		}

	//	Recurse

	SNode *pBranch = pEnd;
	while (pBranch)
		{
		PaintBranch(Dest, pStart, pBranch);
		pBranch = pBranch->pNext;
		}

	if (pEnd && pStart->iSphereSize)
		{
		CG32bitPixel rgbColor = (m_iWillpower > pStart->iRootDist ? CG32bitPixel(255,255,255) : CG32bitPixel(128,255,128));
		PaintSphere(Dest, pStart->x, pStart->y, pStart->iSphereSize, rgbColor);
		}
	}

void CGNeurohackArea::PaintSphere (CG32bitImage &Dest, int x, int y, int iRadius, CG32bitPixel rgbGlowColor)

//	PaintSphere
//
//	Paints the sphere

	{
	CG32bitPixel rgbFadeColor = CG32bitPixel::Blend(0, rgbGlowColor, (BYTE)192);
	CGDraw::RingGlowing(Dest, x, y, iRadius + (iRadius / 6), iRadius / 4, rgbFadeColor);
	CGDraw::Circle(Dest, x, y, iRadius, CG32bitPixel(25,25,25));
	CGDraw::CircleGradient(Dest, x + (iRadius / 3), y - (iRadius / 3), iRadius / 2, rgbFadeColor);
	CGDraw::CircleGradient(Dest, x + (iRadius / 3), y - (iRadius / 3), iRadius / 4, rgbGlowColor);

	if (x % 2)
		CGDraw::CircleGradient(Dest, x - (iRadius / 2), y - (iRadius / 2), iRadius / 8, rgbGlowColor);

	if ((y % 2) == 0)
		CGDraw::CircleGradient(Dest, x - (iRadius / 2) - (iRadius / 4), y + (iRadius / 5), iRadius / 6, rgbFadeColor);
	}

void CGNeurohackArea::SetData (int iWillpower, int iDamage)

//	SetData
//
//	Sets the data

	{
	m_iWillpower = mathSqrt(iWillpower / 8);
	m_iDamage = mathSqrt(8 * iDamage);
	}

void CGNeurohackArea::Update (void)

//	Update
//
//	Update event

	{
	DEBUG_TRY

	int i;

	if (m_pNetwork)
		{
		//	Loop over all active nodes

		for (i = 0; i < m_iActiveCount; i++)
			{
			//	Count the number of children for this node

			int iChildren = 0;
			SNode *pChild = m_pActiveNodes[i]->pFirstChild;
			while (pChild)
				{
				iChildren++;
				pChild = pChild->pNext;
				}

			//	If we have 0 children, then we're at the end of the line;
			//	We reset to the root

			if (iChildren == 0)
				m_pActiveNodes[i] = m_pRootNodes[mathRandom(0, m_iRootCount-1)];

			//	Otherwise, move along to a child

			else
				{
				int iChild = mathRandom(0, iChildren-1);
				SNode *pChild = m_pActiveNodes[i]->pFirstChild;
				while (iChild--)
					pChild = pChild->pNext;

				m_pActiveNodes[i] = pChild;
				}
			}

		//	Start a new active node at the root

		if (m_iActiveCount < m_iActiveAlloc)
			m_pActiveNodes[m_iActiveCount++] = m_pRootNodes[mathRandom(0, m_iRootCount-1)];
		}

	DEBUG_CATCH
	}
