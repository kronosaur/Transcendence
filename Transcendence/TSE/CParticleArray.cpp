//	CParticleArray.cpp
//
//	CParticleArray object

#include "PreComp.h"

const int FIXED_POINT =							256;
const WORD FLAME_CORE_COLOR =					CG16bitImage::RGBValue(255,241,230);
const WORD FLAME_MIDDLE_COLOR =					CG16bitImage::RGBValue(255,208,0);
const WORD FLAME_OUTER_COLOR =					CG16bitImage::RGBValue(255,59,27);
//const WORD FLAME_MIDDLE_COLOR =					CG16bitImage::RGBValue(248,200,12);
//const WORD FLAME_OUTER_COLOR =					CG16bitImage::RGBValue(189,30,0);

#define PAINT_GASEOUS_PARTICLE(Dest,x,y,iWidth,wColor,iFade,iFade2)	\
	{	\
	switch (iWidth)	\
		{	\
		case 0:	\
			Dest.DrawPixelTrans((x), (y), (wColor), (iFade));	\
			break;	\
\
		case 1:	\
			Dest.DrawPixelTrans((x), (y), (wColor), (iFade));	\
			Dest.DrawPixelTrans((x) + 1, (y), (wColor), (iFade2));	\
			Dest.DrawPixelTrans((x), (y) + 1, (wColor), (iFade2));	\
			break;	\
\
		case 2:	\
			Dest.DrawPixelTrans((x), (y), (wColor), (iFade));	\
			Dest.DrawPixelTrans((x) + 1, (y), (wColor), (iFade2));	\
			Dest.DrawPixelTrans((x), (y) + 1, (wColor), (iFade2));	\
			Dest.DrawPixelTrans((x) - 1, (y), (wColor), (iFade2));	\
			Dest.DrawPixelTrans((x), (y) - 1, (wColor), (iFade2));	\
			break;	\
\
		case 3:	\
			Dest.DrawPixelTrans((x), (y), (wColor), (iFade));	\
			Dest.DrawPixelTrans((x) + 1, (y), (wColor), (iFade));	\
			Dest.DrawPixelTrans((x), (y) + 1, (wColor), (iFade));	\
			Dest.DrawPixelTrans((x) - 1, (y), (wColor), (iFade));	\
			Dest.DrawPixelTrans((x), (y) - 1, (wColor), (iFade));	\
			Dest.DrawPixelTrans((x) + 1, (y) + 1, (wColor), (iFade2));	\
			Dest.DrawPixelTrans((x) + 1, (y) - 1, (wColor), (iFade2));	\
			Dest.DrawPixelTrans((x) - 1, (y) + 1, (wColor), (iFade2));	\
			Dest.DrawPixelTrans((x) - 1, (y) - 1, (wColor), (iFade2));	\
			break;	\
\
		default:	\
			if (CG16bitImage::IsGrayscaleValue(wColor))	\
				DrawFilledCircleGray(Dest, (x), (y), ((iWidth) + 1) / 2, (wColor), (iFade));	\
			else	\
				DrawFilledCircleTrans(Dest, (x), (y), ((iWidth) + 1) / 2, (wColor), (iFade));	\
			break;	\
		}	\
	}

CParticleArray::CParticleArray (void) :
		m_iCount(0),
		m_pArray(NULL),
		m_iLastAdded(-1),
		m_bUseRealCoords(false)

//	CParticleArray constructor

	{
	m_rcBounds.left = 0;
	m_rcBounds.top = 0;
	m_rcBounds.right = 0;
	m_rcBounds.bottom = 0;
	}

CParticleArray::~CParticleArray (void)

//	CParticleArray destructor

	{
	if (m_pArray)
		delete [] m_pArray;
	}

void CParticleArray::AddParticle (const CVector &vPos, const CVector &vVel, int iLifeLeft, int iRotation, int iDestiny, DWORD dwData)

//	AddParticle
//
//	Adds a new particle to the array

	{
	ASSERT(iDestiny == -1 || (iDestiny >= 0 && iDestiny < g_DestinyRange));

	if (m_iCount == 0)
		return;

	//	Infinite loop without this.

	if (m_iLastAdded == -1)
		m_iLastAdded = m_iCount - 1;

	//	Look for an open slot

	int iSlot = (m_iLastAdded + 1) % m_iCount;
	while (iSlot != m_iLastAdded && m_pArray[iSlot].fAlive)
		iSlot = (iSlot + 1) % m_iCount;

	//	If we're out of room, can't add

	if (iSlot == m_iLastAdded)
		return;

	//	Add the particle at the slot

	SParticle *pParticle = &m_pArray[iSlot];

	if (m_bUseRealCoords)
		{
		pParticle->Pos = vPos;
		pParticle->Vel = (vVel * g_SecondsPerUpdate);
		PosToXY(vPos, &pParticle->x, &pParticle->y);
		//	xVel and yVel are ignored if using real coords
		}
	else
		{
		PosToXY(vPos, &pParticle->x, &pParticle->y);
		PosToXY(vVel * g_SecondsPerUpdate, &pParticle->xVel, &pParticle->yVel);
		}

	pParticle->iLifeLeft = iLifeLeft;
	pParticle->iDestiny = (iDestiny == -1 ? mathRandom(0, g_DestinyRange - 1) : iDestiny);
	pParticle->iRotation = iRotation;
	pParticle->dwData = dwData;

	pParticle->fAlive = true;

	m_iLastAdded = iSlot;
	}

void CParticleArray::CleanUp (void)

//	CleanUp
//
//	Deletes the array

	{
	if (m_pArray)
		{
		delete [] m_pArray;
		m_pArray = NULL;
		}

	m_iCount = 0;
	}

void CParticleArray::GetBounds (CVector *retvUR, CVector *retvLL)

//	GetBounds
//
//	Returns the bounds of the particles

	{
	if (m_bUseRealCoords)
		{
		*retvUR = m_vOrigin + m_vUR;
		*retvLL = m_vOrigin + m_vLL;
		}
	else
		{
		*retvUR = m_vOrigin + CVector(m_rcBounds.right * g_KlicksPerPixel, -m_rcBounds.top * g_KlicksPerPixel);
		*retvLL = m_vOrigin + CVector(m_rcBounds.left * g_KlicksPerPixel, -m_rcBounds.bottom * g_KlicksPerPixel);
		}
	}

void CParticleArray::Init (int iMaxCount, const CVector &vOrigin)

//	Init
//
//	Initializes the particle array

	{
	CleanUp();

	if (iMaxCount > 0)
		{
		m_pArray = new SParticle [iMaxCount];
		utlMemSet(m_pArray, sizeof(SParticle) * iMaxCount, 0);

		m_iCount = iMaxCount;
		}

	m_vOrigin = vOrigin;
	}

void CParticleArray::Move (const CVector &vMove)

//	Move
//
//	Moves all particles by the given vector

	{
	UseRealCoords();

	SParticle *pParticle = m_pArray;
	SParticle *pEnd = pParticle + m_iCount;
	int iParticleCount = 0;
	CVector vTotalPos;

	//	Keep track of bounds

	Metric xLeft = g_InfiniteDistance;
	Metric xRight = -g_InfiniteDistance;
	Metric yTop = -g_InfiniteDistance;
	Metric yBottom = g_InfiniteDistance;

	//	Loop

	while (pParticle < pEnd)
		{
		if (pParticle->fAlive)
			{
			iParticleCount++;

			//	Update position

			pParticle->Pos = pParticle->Pos + vMove;

			//	Convert to integer
			//	NOTE: If we're using real coords we always ignore integer
			//	velocity.

			PosToXY(pParticle->Pos, &pParticle->x, &pParticle->y);

			//	Update the bounding box

			if (pParticle->Pos.GetX() > xRight)
				xRight = pParticle->Pos.GetX();
			if (pParticle->Pos.GetX() < xLeft)
				xLeft = pParticle->Pos.GetX();

			if (pParticle->Pos.GetY() < yBottom)
				yBottom = pParticle->Pos.GetY();
			if (pParticle->Pos.GetY() > yTop)
				yTop = pParticle->Pos.GetY();

			//	Update center of mass

			vTotalPos = vTotalPos + pParticle->Pos;
			}

		//	Next

		pParticle++;
		}

	//	Set bounds

	m_vUR = CVector(xRight, yTop);
	m_vLL = CVector(xLeft, yBottom);

	int xiRight, xiLeft, yiTop, yiBottom;
	PosToXY(m_vUR, &xiRight, &yiTop);
	PosToXY(m_vLL, &xiLeft, &yiBottom);

	m_rcBounds.left = xiLeft / FIXED_POINT;
	m_rcBounds.top = yiTop / FIXED_POINT;
	m_rcBounds.right = xiRight / FIXED_POINT;
	m_rcBounds.bottom = yiBottom / FIXED_POINT;

	//	Center of mass

	m_vCenterOfMass = (iParticleCount > 0 ? vTotalPos / (Metric)iParticleCount : NullVector);
	}

void CParticleArray::Paint (CG16bitImage &Dest,
							int xPos,
							int yPos,
							SViewportPaintCtx &Ctx,
							SParticlePaintDesc &Desc)

//	Paint
//
//	Paint all particles

	{
	//	Paint based on style

	switch (Desc.iStyle)
		{
		case paintFlame:
			{
			int iCore = Desc.iMaxLifetime / 6;
			int iFlame = Desc.iMaxLifetime / 2;
			int iSmoke = 3 * Desc.iMaxLifetime / 4;
			int iSmokeBrightness = 80;

			PaintFireAndSmoke(Dest, 
					xPos, yPos, 
					Ctx, 
					Desc.iMaxLifetime,
					Desc.iMinWidth,
					Desc.iMaxWidth, 
					iCore, 
					iFlame, 
					iSmoke, 
					iSmokeBrightness);
			break;
			}

		case paintImage:
			PaintImage(Dest, xPos, yPos, Ctx, Desc);
			break;

		case paintLine:
			PaintLine(Dest, xPos, yPos, Ctx, Desc.wPrimaryColor);
			break;

		case paintSmoke:
			{
			int iCore = 1;
			int iFlame = Desc.iMaxLifetime / 6;
			int iSmoke = Desc.iMaxLifetime / 4;
			int iSmokeBrightness = 60;

			PaintFireAndSmoke(Dest, 
					xPos, yPos, 
					Ctx, 
					Desc.iMaxLifetime,
					Desc.iMinWidth,
					Desc.iMaxWidth, 
					iCore, 
					iFlame, 
					iSmoke, 
					iSmokeBrightness);
			break;
			}

		default:
			PaintGaseous(Dest,
					xPos, yPos,
					Ctx,
					Desc.iMaxLifetime,
					Desc.iMinWidth,
					Desc.iMaxWidth,
					Desc.wPrimaryColor,
					Desc.wSecondaryColor);
			break;
		}
	}

void CParticleArray::Paint (CG16bitImage &Dest,
							int xPos,
							int yPos,
							SViewportPaintCtx &Ctx,
							IEffectPainter *pPainter)

//	Paint
//
//	Paint using a painter for each particle

	{
	int iSavedDestiny = Ctx.iDestiny;
	int iSavedRotation = Ctx.iRotation;

	SParticle *pParticle = m_pArray;
	SParticle *pEnd = pParticle + m_iCount;

	while (pParticle < pEnd)
		{
		if (pParticle->fAlive)
			{
			//	Compute the position of the particle

			int x = xPos + pParticle->x / FIXED_POINT;
			int y = yPos + pParticle->y / FIXED_POINT;

			//	Paint the particle

			Ctx.iDestiny = pParticle->iDestiny;
			Ctx.iRotation = pParticle->iRotation;
			pPainter->Paint(Dest, x, y, Ctx);
			}

		//	Next

		pParticle++;
		}

	Ctx.iDestiny = iSavedDestiny;
	Ctx.iRotation = iSavedRotation;
	}

void CParticleArray::PaintFireAndSmoke (CG16bitImage &Dest, 
										int xPos, 
										int yPos, 
										SViewportPaintCtx &Ctx, 
										int iLifetime,
										int iMinWidth,
										int iMaxWidth,
										int iCore,
										int iFlame,
										int iSmoke,
										int iSmokeBrightness)

//	PaintFireAndSmoke
//
//	Paints each particle as fire/smoke particle. 
//
//	iLifetime is the point at which the particle fades completely
//	iWidth is the maximum width of the particle

	{
	//	We don't support infinite lifetime here
	ASSERT(iLifetime >= 0);

	iCore = Max(iCore, 1);
	iFlame = Max(iFlame, iCore + 1);
	iSmoke = Max(iSmoke, iFlame + 1);

	iMinWidth = Max(1, iMinWidth);
	iMaxWidth = Max(iMinWidth, iMaxWidth);
	int iWidthRange = (iMaxWidth - iMinWidth) + 1;

	SParticle *pParticle = m_pArray;
	SParticle *pEnd = pParticle + m_iCount;

	while (pParticle < pEnd)
		{
		if (pParticle->fAlive)
			{
			int iLifeLeft = (pParticle->iLifeLeft == -1 ? iLifetime : Min(pParticle->iLifeLeft, iLifetime));
			int iAge = iLifetime - iLifeLeft;

			//	Compute properties of the particle based on its life

			WORD wColor = 0;
			int iFade = 0;
			int iFade2 = 0;
			int iWidth = 0;

			if (iLifetime > 0)
				{
				//	Particle fades out over time

				iFade = Max(20, Min(255, (255 * iLifeLeft / iLifetime)));
				iFade2 = iFade / 2;

				//	Particle grows over time

				iWidth = iMinWidth + (iWidthRange * iAge / iLifetime);

				//	Smoke color

				int iDarkness = Min(255, iSmokeBrightness + (2 * (pParticle->iDestiny % 25)));
				WORD wSmokeColor = CG16bitImage::GrayscaleValue(iDarkness);

				//	Some particles are gray

				WORD wFadeColor;
				if ((pParticle->iDestiny % 4) != 0)
					wFadeColor = FLAME_OUTER_COLOR;
				else
					wFadeColor = wSmokeColor;

				//	Particle color changes over time

				if (iAge <= iCore)
					wColor = CG16bitImage::FadeColor(FLAME_CORE_COLOR,
							FLAME_MIDDLE_COLOR,
							100 * iAge / iCore);
				else if (iAge <= iFlame)
					wColor = CG16bitImage::FadeColor(FLAME_MIDDLE_COLOR,
							wFadeColor,
							100 * (iAge - iCore) / (iFlame - iCore));
				else if (iAge <= iSmoke)
					wColor = CG16bitImage::FadeColor(wFadeColor,
							wSmokeColor,
							100 * (iAge - iFlame) / (iSmoke - iFlame));
				else
					wColor = wSmokeColor;
				}

			//	Compute the position of the particle

			int x = xPos + pParticle->x / FIXED_POINT;
			int y = yPos + pParticle->y / FIXED_POINT;

			//	Paint the particle

			PAINT_GASEOUS_PARTICLE(Dest, x, y, iWidth, wColor, iFade, iFade2);
			}

		//	Next

		pParticle++;
		}
	}

void CParticleArray::PaintGaseous (CG16bitImage &Dest,
								   int xPos,
								   int yPos,
								   SViewportPaintCtx &Ctx,
								   int iMaxLifetime,
								   int iMinWidth,
								   int iMaxWidth,
								   WORD wPrimaryColor,
								   WORD wSecondaryColor)

//	PaintGaseous
//
//	Paints gaseous particles that fade from primary color to secondary color

	{
	ASSERT(iMaxLifetime >= 0);

	iMinWidth = Max(1, iMinWidth);
	iMaxWidth = Max(iMinWidth, iMaxWidth);
	int iWidthRange = (iMaxWidth - iMinWidth) + 1;

	SParticle *pParticle = m_pArray;
	SParticle *pEnd = pParticle + m_iCount;

	while (pParticle < pEnd)
		{
		if (pParticle->fAlive)
			{
			int iLifeLeft = (pParticle->iLifeLeft == -1 ? iMaxLifetime : Min(pParticle->iLifeLeft, iMaxLifetime));
			int iAge = iMaxLifetime - iLifeLeft;

			//	Compute properties of the particle based on its life

			WORD wColor = 0;
			int iFade = 0;
			int iFade2 = 0;
			int iWidth = 0;

			if (iMaxLifetime > 0)
				{
				//	Particle fades out over time

				iFade = Max(20, Min(255, (255 * iLifeLeft / iMaxLifetime)));
				iFade2 = iFade / 2;

				//	Particle grows over time

				iWidth = iMinWidth + (iWidthRange * iAge / iMaxLifetime);

				//	Particle color fades from primary to secondary

				wColor = CG16bitImage::FadeColor(wPrimaryColor, wSecondaryColor, 100 * iAge / iMaxLifetime);
				}

			//	Compute the position of the particle

			int x = xPos + pParticle->x / FIXED_POINT;
			int y = yPos + pParticle->y / FIXED_POINT;

			//	Paint the particle

			PAINT_GASEOUS_PARTICLE(Dest, x, y, iWidth, wColor, iFade, iFade2);
			}

		//	Next

		pParticle++;
		}
	}

void CParticleArray::PaintImage (CG16bitImage &Dest, int xPos, int yPos, SViewportPaintCtx &Ctx, SParticlePaintDesc &Desc)

//	Paint
//
//	Paints particle as an image

	{
	int iRotationFrame = 0;
	if (Desc.bDirectional)
		iRotationFrame = Angle2Direction(Ctx.iRotation, Desc.iVariants);

	SParticle *pParticle = m_pArray;
	SParticle *pEnd = pParticle + m_iCount;

	while (pParticle < pEnd)
		{
		if (pParticle->fAlive)
			{
			//	Figure out the animation frame to paint

			int iTick;
			if (Desc.bRandomStartFrame)
				iTick = Ctx.iTick + pParticle->iDestiny;
			else
				iTick = Ctx.iTick;

			//	Figure out the rotation or variant to paint

			int iFrame = (Desc.bDirectional ? iRotationFrame : (pParticle->iDestiny % Desc.iVariants));

			//	Compute the position of the particle

			int x = xPos + pParticle->x / FIXED_POINT;
			int y = yPos + pParticle->y / FIXED_POINT;

			//	Paint the particle

			Desc.pImage->PaintImage(Dest, x, y, iTick, iFrame);
			}

		//	Next

		pParticle++;
		}
	}

void CParticleArray::PaintLine (CG16bitImage &Dest,
								int xPos,
								int yPos,
								SViewportPaintCtx &Ctx,
								WORD wPrimaryColor)

//	PaintLine
//
//	Paints particle as a line

	{
	//	Figure out the velocity of our object

	int xVel = 0;
	int yVel = 0;
	if (Ctx.pObj)
		PosToXY(Ctx.pObj->GetVel(), &xVel, &yVel);

	//	Paint all the particles

	SParticle *pParticle = m_pArray;
	SParticle *pEnd = pParticle + m_iCount;

	while (pParticle < pEnd)
		{
		if (pParticle->fAlive)
			{
			//	Compute the position of the particle

			int xFrom = xPos + pParticle->x / FIXED_POINT;
			int yFrom = yPos + pParticle->y / FIXED_POINT;

			int xTo = xFrom - (xVel + pParticle->xVel) / FIXED_POINT;
			int yTo = yFrom - (yVel + pParticle->yVel) / FIXED_POINT;

			//	Paint the particle

			Dest.DrawLine(xFrom, yFrom,
					xTo, yTo,
					1,
					wPrimaryColor);
			}

		//	Next

		pParticle++;
		}
	}

void CParticleArray::PosToXY (const CVector &xy, int *retx, int *rety)

//	PosToXY
//
//	Convert from system coordinates to particle coordinates

	{
	*retx = (int)(FIXED_POINT * xy.GetX() / g_KlicksPerPixel);
	*rety = -(int)(FIXED_POINT * xy.GetY() / g_KlicksPerPixel);
	}

void CParticleArray::ReadFromStream (SLoadCtx &Ctx)

//	ReadFromStream
//
//	Reads from a stream
//
//	DWORD		Particle count
//	CVector		m_vOrigin
//	CVector		m_vCenterOfMass
//	CVector		m_vUR
//	CVector		m_vLL
//	DWORD		Flags
//	Array of particles

	{
	int i;
	DWORD dwLoad;

	CleanUp();

	//	Load the count

	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	if (dwLoad > 0x00100000)
		return;

	m_iCount = dwLoad;

	//	Origin

	if (Ctx.dwVersion >= 67)
		{
		Ctx.pStream->Read((char *)&m_vOrigin, sizeof(CVector));
		Ctx.pStream->Read((char *)&m_vCenterOfMass, sizeof(CVector));
		Ctx.pStream->Read((char *)&m_vUR, sizeof(CVector));
		Ctx.pStream->Read((char *)&m_vLL, sizeof(CVector));
		}

	//	Load flags

	if (Ctx.dwVersion >= 64)
		{
		Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
		m_bUseRealCoords =			((dwLoad & 0x00000001) ? true : false);
		}
	else
		{
		m_bUseRealCoords = false;
		}

	//	If no particles, then we're done

	if (m_iCount == 0)
		{
		m_pArray = NULL;
		return;
		}

	//	Load the particles

	m_pArray = new SParticle [m_iCount];
	
	//	Previous version didn't have everything

	if (Ctx.dwVersion < 64)
		{
		for (i = 0; i < m_iCount; i++)
			{
			Ctx.pStream->Read((char *)&m_pArray[i].x, sizeof(DWORD));
			Ctx.pStream->Read((char *)&m_pArray[i].y, sizeof(DWORD));
			Ctx.pStream->Read((char *)&m_pArray[i].xVel, sizeof(DWORD));
			Ctx.pStream->Read((char *)&m_pArray[i].yVel, sizeof(DWORD));
			Ctx.pStream->Read((char *)&m_pArray[i].iLifeLeft, sizeof(DWORD));
			Ctx.pStream->Read((char *)&m_pArray[i].iDestiny, sizeof(DWORD));
			Ctx.pStream->Read((char *)&m_pArray[i].iRotation, sizeof(DWORD));
			Ctx.pStream->Read((char *)&m_pArray[i].dwData, sizeof(DWORD));

			Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
			m_pArray[i].fAlive = ((dwLoad & 0x00000001) ? true : false);
			m_pArray[i].dwSpare = 0;

			//	See if we need to compute real coords

			if (m_bUseRealCoords)
				{
				m_pArray[i].Pos = XYToPos(m_pArray[i].x, m_pArray[i].y);
				m_pArray[i].Vel = XYToPos(m_pArray[i].xVel, m_pArray[i].yVel);
				}
			}
		}
	else
		Ctx.pStream->Read((char *)m_pArray, sizeof(SParticle) * m_iCount);
	}

void CParticleArray::Update (SEffectUpdateCtx &Ctx)

//	Update
//
//	Updates the array based on the context

	{
	int i;

	//	We need real coordinates for this

	UseRealCoords();

	//	Compute some values

	int iSplashChance = (Ctx.pDamageDesc ? Ctx.pDamageDesc->GetParticleSplashChance() : 0);
	int iGhostChance = (Ctx.pDamageDesc ? Ctx.pDamageDesc->GetParticleMissChance() : 0);

	//	Compute the velocity of the effect object in Km/tick

	CVector vEffectVel;
	if (Ctx.iWakePotential)
		vEffectVel = Ctx.pObj->GetVel() * g_SecondsPerUpdate;

	//	Compute bounds

	CVector vUR;
	CVector vLL;
	GetBounds(&vUR, &vLL);

	//	Loop over all objects in the system

	for (i = 0; i < Ctx.pSystem->GetObjectCount(); i++)
		{
		CSpaceObject *pObj = Ctx.pSystem->GetObject(i);

		//	If the object is in the bounding box then remember
		//	it so that we can do a more accurate calculation.

		if (pObj 
				&& Ctx.pObj->CanHit(pObj) 
				&& pObj->CanBeHit() 
				&& pObj->InBox(vUR, vLL)
				&& !pObj->IsDestroyed()
				&& pObj != Ctx.pObj)
			{

			//	See if this object can be hit by the damage

			bool bDoDamage = (Ctx.pDamageDesc && pObj->CanBeHitBy(Ctx.pDamageDesc->m_Damage));

			//	Keep track of some values if we're doing damage

			int iHitCount = 0;
			CVector vTotalHitPos;

			//	Compute some values if we're affecting particles

			CVector vVelN;
			CVector vVelT;
			Metric rObjVel;
			Metric rWakeFactor;
			int iObjMotionAngle;
			if (Ctx.iWakePotential)
				{
				//	Compute the velocity components of the object that will
				//	affect the particles

				Metric rSpeed = pObj->GetVel().Length();
				vVelN = (rSpeed == 0.0 ? CVector(1.0f, 0) : pObj->GetVel() / rSpeed);
				vVelT = vVelN.Perpendicular();

				//	Object speed in Km/tick

				rObjVel = rSpeed * g_SecondsPerUpdate;

				//	How much does velocity of object affect particle

				rWakeFactor = (Metric)Ctx.iWakePotential / 100.0f;

				//	Compute the objects direction

				iObjMotionAngle = VectorToPolar(vVelN);
				}

			//	Initialize context for hit testing

			SPointInObjectCtx PIOCtx;
			pObj->PointInObjectInit(PIOCtx);

			//	Loop over all particles

			SParticle *pParticle = m_pArray;
			SParticle *pEnd = pParticle + m_iCount;
			bool bNoParticlesLeft = true;

			while (pParticle < pEnd)
				{
				if (pParticle->fAlive)
					{
					bool bHit = false;

					//	There is at least one particle alive

					bNoParticlesLeft = false;

					//	If we're doing a true hit test then we need a pretty accurate test

					if (bDoDamage)
						{
						//	Compute the current position of the particle and the
						//	half-way position of the particle

						CVector vCurPos = m_vOrigin + pParticle->Pos;
						CVector vHalfPos = vCurPos - (pParticle->Vel / 2.0f);

						//	First check to see if the new position hit the object

						if (pObj->PointInObject(PIOCtx, pObj->GetPos(), vCurPos))
							{
							bHit = true;
							vTotalHitPos = vTotalHitPos + vCurPos;
							iHitCount++;
							}

						//	Otherwise, check the half-way point

						else if (pObj->PointInObject(PIOCtx, pObj->GetPos(), vHalfPos))
							{
							bHit = true;
							vTotalHitPos = vTotalHitPos + vHalfPos;
							iHitCount++;
							}
						}

					//	Otherwise we just check bounds.

					else
						{
						if (pObj->PointInObject(PIOCtx, pObj->GetPos(), m_vOrigin + pParticle->Pos))
							bHit = true;
						}


					//	If we hit, deal with it

					if (bHit)
						{
						//	Some particles hit the object

						if (bDoDamage && (iGhostChance == 0 || mathRandom(1, 100) <= iGhostChance))
							{
							//	Some of the particles bounce

							if (iSplashChance && mathRandom(1, 100) <= iSplashChance)
								{
								Metric rSpeed;
								int iDir = VectorToPolar(pParticle->Vel, &rSpeed);

								iDir = (iDir + 180 + mathRandom(-60, 60) + mathRandom(-60, 60)) % 360;
								CVector vNewVel = PolarToVector(iDir, mathRandom(5, 30) * rSpeed / 100.0);
								pParticle->Vel = vNewVel;

								if (pParticle->iLifeLeft != -1)
									pParticle->iLifeLeft = Max(1, pParticle->iLifeLeft - mathRandom(2, 5));
								}
							else
								pParticle->fAlive = false;
							}

						//	Surviving particles make be influenced

						else if (Ctx.iWakePotential)
							{
							//	Compute whether the particle is to the left or right of the object
							//	along the object's line of motion.
							//
							//	We start by computing the particle's position relative to object

							CVector vRelPos = (m_vOrigin + pParticle->Pos) - pObj->GetPos();
							int iRelAngle = VectorToPolar(vRelPos);

							//	Compute the bearing (>0 is left; <0 is right)

							int iBearing = AngleBearing(iObjMotionAngle, iRelAngle);

							//	Decompose the particle's velocity along the object's motion

							CVector vParticleVel = vEffectVel + pParticle->Vel;
							Metric rParticleVelLine = vParticleVel.Dot(vVelN);
							Metric rParticleVelPerp = vParticleVel.Dot(vVelT);

							//	Compute the maximum speed of the particle

							Metric rMaxSpeed = Max(rWakeFactor * rObjVel, pParticle->Vel.Length());

							//	Figure out how we affect the particle speed along the object's motion

							Metric rNewVelLine = rParticleVelLine + (rWakeFactor * rObjVel);
							Metric rNewVelPerp = rParticleVelPerp + (rWakeFactor * rObjVel);
							if (iBearing > 0)
								rNewVelPerp = -rNewVelPerp;

							//	Generate the new velocity

							CVector vNewVel = (rNewVelLine * vVelN) + (rNewVelPerp * vVelT);
							vNewVel.Clip(rMaxSpeed);

							//	Set the particle velocity

							pParticle->Vel = vNewVel - vEffectVel;
							}
						}
					}

				//	Next

				pParticle++;
				}

			//	If we hit the object, then add to the list

			if (bDoDamage && iHitCount > 0 && Ctx.iTotalParticleCount > 0)
				{
				//	Add to the list

				SEffectHitDesc *pHit = Ctx.Hits.Insert();
				pHit->pObjHit = pObj;
				pHit->vHitPos = (vTotalHitPos / iHitCount);
				pHit->iHitStrength = iHitCount;

				//	Compute damage

				int iTotal = Ctx.pDamageDesc->m_Damage.RollDamage() * pHit->iHitStrength;
				int iDamage = iTotal / Ctx.iTotalParticleCount;
				if (mathRandom(1, 100) <= (iTotal % Ctx.iTotalParticleCount) * 100 / Ctx.iTotalParticleCount)
					iDamage++;

				//	Do damage

				SDamageCtx DamageCtx;
				DamageCtx.pObj = pObj;
				DamageCtx.pDesc = Ctx.pDamageDesc;
				DamageCtx.Damage = Ctx.pDamageDesc->m_Damage;
				DamageCtx.Damage.SetDamage(iDamage);
				DamageCtx.Damage.AddEnhancements(Ctx.pEnhancements);
				DamageCtx.Damage.SetCause(Ctx.iCause);
				if (Ctx.bAutomatedWeapon)
					DamageCtx.Damage.SetAutomatedWeapon();
				DamageCtx.vHitPos = pHit->vHitPos;
				DamageCtx.iDirection = VectorToPolar(DamageCtx.vHitPos - pObj->GetPos());
				DamageCtx.pCause = Ctx.pObj;
				DamageCtx.Attacker = Ctx.Attacker;

				pObj->Damage(DamageCtx);
				}

			//	If there are no particles left, then nothing more will happen
			
			if (bNoParticlesLeft)
				{
				Ctx.bDestroy = true;
				return;
				}
			}
		}
	}

void CParticleArray::UpdateMotionLinear (bool *retbAlive, CVector *retvAveragePos)

//	UpdateMotionLinear
//
//	Updates the position of all particles

	{
	//	If we've been asked for the average position, then we
	//	need to use real coordinates

	if (retvAveragePos)
		UseRealCoords();

	//	See if all particles are dead

	bool bAllParticlesDead = true;				//	If true, all dead after processing

	//	Keep track of center of mass

	bool bCalcCenterOfMass = m_bUseRealCoords;
	int iParticleCount = 0;
	CVector vTotalPos;

	//	Loop

	SParticle *pParticle = m_pArray;
	SParticle *pEnd = pParticle + m_iCount;

	//	Different loops depending on whether we are using
	//	real coordinates or not.

	if (m_bUseRealCoords)
		{
		//	Keep track of bounds

		Metric xLeft = g_InfiniteDistance;
		Metric xRight = -g_InfiniteDistance;
		Metric yTop = -g_InfiniteDistance;
		Metric yBottom = g_InfiniteDistance;

		//	Loop

		while (pParticle < pEnd)
			{
			if (pParticle->fAlive)
				{
				iParticleCount++;

				//	Update position

				pParticle->Pos = pParticle->Pos + pParticle->Vel;

				//	Convert to integer
				//	NOTE: If we're using real coords we always ignore integer
				//	velocity.

				PosToXY(pParticle->Pos, &pParticle->x, &pParticle->y);

				//	Update the bounding box

				if (pParticle->Pos.GetX() > xRight)
					xRight = pParticle->Pos.GetX();
				if (pParticle->Pos.GetX() < xLeft)
					xLeft = pParticle->Pos.GetX();

				if (pParticle->Pos.GetY() < yBottom)
					yBottom = pParticle->Pos.GetY();
				if (pParticle->Pos.GetY() > yTop)
					yTop = pParticle->Pos.GetY();

				//	Update lifetime

				if (pParticle->iLifeLeft > 0)
					{
					pParticle->iLifeLeft--;
					bAllParticlesDead = false;
					}

				//	If we hit 0, then we're dead

				else if (pParticle->iLifeLeft == 0)
					pParticle->fAlive = false;

				//	Otherwise, LifeLeft is -1 and we're immortal

				else
					bAllParticlesDead = false;

				//	Update center of mass

				if (bCalcCenterOfMass)
					vTotalPos = vTotalPos + pParticle->Pos;
				}

			//	Next

			pParticle++;
			}

		//	Set bounds

		m_vUR = CVector(xRight, yTop);
		m_vLL = CVector(xLeft, yBottom);

		int xiRight, xiLeft, yiTop, yiBottom;
		PosToXY(m_vUR, &xiRight, &yiTop);
		PosToXY(m_vLL, &xiLeft, &yiBottom);

		m_rcBounds.left = xiLeft / FIXED_POINT;
		m_rcBounds.top = yiTop / FIXED_POINT;
		m_rcBounds.right = xiRight / FIXED_POINT;
		m_rcBounds.bottom = yiBottom / FIXED_POINT;

		//	Center of mass

		m_vCenterOfMass = (iParticleCount > 0 ? vTotalPos / (Metric)iParticleCount : NullVector);
		if (retvAveragePos)
			*retvAveragePos = m_vCenterOfMass;
		}
	else
		{
		//	Keep track of our bounds

		int xLeft = 0;
		int xRight = 0;
		int yTop = 0;
		int yBottom = 0;

		//	Loop

		while (pParticle < pEnd)
			{
			if (pParticle->fAlive)
				{
				//	Update position

				pParticle->x += pParticle->xVel;
				pParticle->y += pParticle->yVel;

				//	Update the bounding box

				if (pParticle->x > xRight)
					xRight = pParticle->x;
				if (pParticle->x < xLeft)
					xLeft = pParticle->x;

				if (pParticle->y > yBottom)
					yBottom = pParticle->y;
				if (pParticle->y < yTop)
					yTop = pParticle->y;

				//	Update lifetime

				if (pParticle->iLifeLeft > 0)
					{
					pParticle->iLifeLeft--;
					bAllParticlesDead = false;
					}

				//	If we hit 0, then we're dead

				else if (pParticle->iLifeLeft == 0)
					pParticle->fAlive = false;

				//	Otherwise, LifeLeft is -1 and we're immortal

				else
					bAllParticlesDead = false;
				}

			//	Next

			pParticle++;
			}

		//	Set the bounding rect

		m_rcBounds.left = xLeft / FIXED_POINT;
		m_rcBounds.top = yTop / FIXED_POINT;
		m_rcBounds.right = xRight / FIXED_POINT;
		m_rcBounds.bottom = yBottom / FIXED_POINT;
		}

	//	Any particles left?

	if (retbAlive)
		*retbAlive = !bAllParticlesDead;
	}

void CParticleArray::UpdateRingCohesion (Metric rRadius, Metric rMinRadius, Metric rMaxRadius, int iCohesion, int iResistance)

//	UpdateRingCohesion
//
//	Accelerates all particles so that they stay formed along a ring of the
//	given dimensions

	{
	//	We need to use real coordinates instead of fixed point

	UseRealCoords();

	//	Compute some constants

	Metric rOuterRange = rMaxRadius - rRadius;
	Metric rInnerRange = rRadius - rMinRadius;
	Metric *pNormalDist = GetDestinyToBellCurveArray();

	//	Loop over all particles

	SParticle *pParticle = m_pArray;
	SParticle *pEnd = pParticle + m_iCount;

	while (pParticle < pEnd)
		{
		if (pParticle->fAlive)
			{
			//	Compute this particle's distance from the center

			Metric rParticleRadius = pParticle->Pos.Length();
			
			//	See if the particle is outside of its bounds
			//	(we calculate a different boundary for each individual
			//	particle [based on destiny] so that we can have a "normal"
			//	distribution)

			bool bOutside;
			if (rParticleRadius > rRadius)
				{
				Metric rMax = rRadius + rOuterRange * Absolute(pNormalDist[pParticle->iDestiny]);
				bOutside = (rParticleRadius > rMax);
				}
			else
				{
				Metric rMin = rRadius - rInnerRange * Absolute(pNormalDist[pParticle->iDestiny]);
				bOutside = (rParticleRadius < rMin);
				}

			//	If we're outside our bounds, then we need to accelerate
			//	towards the center

			if (bOutside)
				{
				//	Compute some constants

				Metric rAccelerationFactor = (iCohesion * iCohesion) * g_KlicksPerPixel / 10000.0;

				//	Compute a normal vector pointing away from the center

				CVector vNormal = pParticle->Pos / rParticleRadius;

				//	Accelerate towards the center

				if (rParticleRadius > rRadius)
					pParticle->Vel = pParticle->Vel - (vNormal * rAccelerationFactor);
				else
					pParticle->Vel = pParticle->Vel + (vNormal * rAccelerationFactor);
				}

			//	Otherwise, internal resistance slows us down

			else
				{
				if (iResistance > 0)
					{
					Metric rDragFactor = (10000 - iResistance * iResistance) / 10000.0f;
					pParticle->Vel = pParticle->Vel * rDragFactor;
					}
				}
			}

		//	Next

		pParticle++;
		}
	}

void CParticleArray::UpdateTrackTarget (CSpaceObject *pTarget, int iManeuverRate, Metric rMaxSpeed)

//	UpdateTrackTarget
//
//	Change particle velocities to track the given target

	{
	//	We need to use real coordinates instead of fixed point

	UseRealCoords();

	//	Skip if we don't yet have a center of mass calculated

	if (m_vCenterOfMass.IsNull())
		return;

	//	Compute the intercept position of the target

	CVector vTarget = pTarget->GetPos() - (m_vOrigin + m_vCenterOfMass);
	CVector vTargetVel = pTarget->GetVel();
	Metric rTimeToIntercept = CalcInterceptTime(vTarget, vTargetVel, rMaxSpeed);
	
	CVector vAimPos;
	if (rTimeToIntercept > 0.0)
		vAimPos = (pTarget->GetPos() + pTarget->GetVel() * rTimeToIntercept) - m_vOrigin;
	else
		vAimPos = pTarget->GetPos() - m_vOrigin;

	//	Loop over all particles

	SParticle *pParticle = m_pArray;
	SParticle *pEnd = pParticle + m_iCount;

	while (pParticle < pEnd)
		{
		if (pParticle->fAlive)
			{
			//	Compute the particles current direction of motion

			Metric rCurSpeed;
			int iCurDir = VectorToPolar(pParticle->Vel, &rCurSpeed);

			//	Compute desired direction

			int iTargetDir = VectorToPolar(vAimPos - pParticle->Pos);

			//	Turn to desired direction.

			if (!AreAnglesAligned(iTargetDir, iCurDir, 1))
				{
				int iTurn = (iTargetDir + 360 - iCurDir) % 360;

				int iNewDir;
				if (iTurn >= 180)
					{
					int iTurnAngle = Min((360 - iTurn), iManeuverRate);
					iNewDir = (iCurDir + 360 - iTurnAngle) % 360;
					}
				else
					{
					int iTurnAngle = Min(iTurn, iManeuverRate);
					iNewDir = (iCurDir + iTurnAngle) % 360;
					}

				//	Turn

				pParticle->Vel = PolarToVector(iNewDir, rCurSpeed);
				}
			}

		//	Next

		pParticle++;
		}
	}

void CParticleArray::UseRealCoords (void)

//	UseRealCoords
//
//	Switches to using real coordinates (instead of int)

	{
	if (!m_bUseRealCoords)
		{
		SParticle *pParticle = m_pArray;
		SParticle *pEnd = pParticle + m_iCount;

		while (pParticle < pEnd)
			{
			if (pParticle->fAlive)
				{
				pParticle->Pos = XYToPos(pParticle->x, pParticle->y);
				pParticle->Vel = XYToPos(pParticle->xVel, pParticle->yVel);
				}

			//	Next

			pParticle++;
			}

		m_bUseRealCoords = true;
		}
	}

void CParticleArray::WriteToStream (IWriteStream *pStream) const

//	WriteToStream
//
//	Writes the array to a stream
//
//	DWORD			count of particles
//	CVector			m_vOrigin
//	CVector			m_vCenterOfMass
//	CVector			m_vUR
//	CVector			m_vLL
//	DWORD			flags
//	SParticle[]		array of particles

	{
	DWORD dwSave;

	pStream->Write((char *)&m_iCount, sizeof(DWORD));
	pStream->Write((char *)&m_vOrigin, sizeof(CVector));
	pStream->Write((char *)&m_vCenterOfMass, sizeof(CVector));
	pStream->Write((char *)&m_vUR, sizeof(CVector));
	pStream->Write((char *)&m_vLL, sizeof(CVector));

	//	Flags

	dwSave = 0;
	dwSave |= (m_bUseRealCoords ?		0x00000001 : 0);
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	//	Array

	pStream->Write((char *)m_pArray, sizeof(SParticle) * m_iCount);
	}

CVector CParticleArray::XYToPos (int x, int y)

//	XYToPos
//
//	Converts from particle coordinates to screen coordinates

	{
	return CVector((x * g_KlicksPerPixel / FIXED_POINT), -(y * g_KlicksPerPixel / FIXED_POINT));
	}
