//	CFractureEffect.cpp
//
//	CFractureEffect object

#include "PreComp.h"

#define ALLOC_GRANULARITY						64
#define FIXED_POINT								256

static CObjectClass<CFractureEffect>g_Class(OBJID_CFRACTUREEFFECT, NULL);

CFractureEffect::CFractureEffect (void) : CSpaceObject(&g_Class),
		m_iParticleCount(0),
		m_pParticles(NULL),
		m_pAttractor(NULL)

//	CFractureEffect constructor

	{
	}

CFractureEffect::~CFractureEffect (void)

//	CFractureEffect destructor

	{
	if (m_pParticles)
		delete [] m_pParticles;
	}

ALERROR CFractureEffect::Create (CSystem *pSystem,
				const CVector &vPos,
				const CVector &vVel,
				const CObjectImageArray &Image,
				int iImageTick,
				int iImageRotation,
				int iStyle,
				CFractureEffect **retpEffect)

//	Create
//
//	Creates a new effects object

	{
	DEBUG_TRY

	ALERROR error;
	CFractureEffect *pEffect;

	pEffect = new CFractureEffect;
	if (pEffect == NULL)
		return ERR_MEMORY;

	pEffect->Place(vPos, vVel);
	pEffect->SetObjectDestructionHook();

	//	Cannot be hit; otherwise, explosion ejecta will hit the effect
	pEffect->SetCannotBeHit();

	pEffect->m_Image = Image;
	pEffect->m_iImageTick = iImageTick;
	pEffect->m_iImageRotation = iImageRotation;
	pEffect->m_iTick = 0;
	pEffect->m_iLifeTime = 300;

	pEffect->m_iStyle = iStyle;
	pEffect->m_iCellSize = 4;

	pEffect->InitParticleArray();

	//	Set large bounds

	pEffect->SetBounds(g_KlicksPerPixel * 1024.0);

	//	Add to system

	if (error = pEffect->AddToSystem(pSystem))
		{
		delete pEffect;
		return error;
		}

	//	Done

	if (retpEffect)
		*retpEffect = pEffect;

	return NOERROR;

	DEBUG_CATCH
	}

void CFractureEffect::InitParticleArray (void)

//	InitParticleArray
//
//	Initializes the particle array based on cell size and the image

	{
	ASSERT(m_pParticles == NULL);
	ASSERT(m_iCellSize >= 1);

	//	Get the source image and metrics

	CG16bitImage &Source = m_Image.GetImage(NULL_STR);
	int xCenter, yCenter;
	RECT rcSource = m_Image.GetImageRect(m_iImageTick, m_iImageRotation, &xCenter, &yCenter);

	//	Allocate a structure for particles

	int iAlloc = ALLOC_GRANULARITY;
	m_pParticles = new SParticle [iAlloc];
	m_iParticleCount = 0;

	int y = rcSource.top;
	while (y + m_iCellSize <= rcSource.bottom)
		{
		int x = rcSource.left;
		while (x + m_iCellSize <= rcSource.right)
			{
			//	Initialize the entry

			if (true)
				{
				//	Add an entry to the array

				if (m_iParticleCount + 1 == iAlloc)
					{
					iAlloc += ALLOC_GRANULARITY;
					SParticle *pNewArray = new SParticle [iAlloc];
					utlMemCopy((char *)m_pParticles, (char *)pNewArray, sizeof(SParticle) * m_iParticleCount);
					delete [] m_pParticles;
					m_pParticles = pNewArray;
					}

				SParticle *pNewParticle = &m_pParticles[m_iParticleCount++];

				//	Initialize position

				pNewParticle->x = (x - xCenter) * FIXED_POINT;
				pNewParticle->y = (y - yCenter) * FIXED_POINT;

				//	Velocity of each particle is away from the center

				CVector vAway(pNewParticle->x, pNewParticle->y);
				vAway = vAway.Normal() * (Metric)mathRandom(0, FIXED_POINT * 4);
				pNewParticle->xV = (int)vAway.GetX();
				pNewParticle->yV = (int)vAway.GetY();

				//	Other

				pNewParticle->iTicks = 0;

				pNewParticle->xSrc = x;
				pNewParticle->ySrc = y;
				pNewParticle->iShape = 0;
				}

			x += m_iCellSize;
			}

		y += m_iCellSize;
		}
	}

void CFractureEffect::OnMove (const CVector &vOldPos, Metric rSeconds)

//	OnMove
//
//	Move hook

	{
	}

void CFractureEffect::ObjectDestroyedHook (const SDestroyCtx &Ctx)

//	ObjectDestroyedHook
//
//	Handle it when another object is destroyed

	{
	if (m_pAttractor == Ctx.pObj)
		m_pAttractor = NULL;
	}

void CFractureEffect::OnPaint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx)

//	OnPaint
//
//	Paints the effect

	{
	if (m_pParticles)
		{
		//	Get the source image and metrics

		CG16bitImage &Source = m_Image.GetImage(NULL_STR);
		//int xCenter, yCenter;
		//RECT rcSource = m_Image.GetImageRect(m_iImageTick, m_iImageRotation, &xCenter, &yCenter);

		//	Loop

		SParticle *pParticle = m_pParticles;
		SParticle *pParticleEnd = pParticle + m_iParticleCount;
		while (pParticle < pParticleEnd)
			{
			if (pParticle->iShape != -1)
				{
				Dest.ColorTransBlt(pParticle->xSrc,
						pParticle->ySrc,
						m_iCellSize,
						m_iCellSize,
						255,
						Source,
						x + (pParticle->x / FIXED_POINT),
						y + (pParticle->y / FIXED_POINT));
				}

			pParticle++;
			}
		}
	}

void CFractureEffect::OnReadFromStream (SLoadCtx &Ctx)

//	OnReadFromStream
//
//	Read object data from a stream
//
//	Image		m_Image
//	DWORD		m_iImageTick
//	DWORD		m_iImageRotation
//	DWORD		m_iTick

	{
#ifdef DEBUG_LOAD
	::OutputDebugString("CFractureEffect::OnReadFromStream\n");
#endif
	m_Image.ReadFromStream(Ctx);
	Ctx.pStream->Read((char *)&m_iImageTick, sizeof(DWORD));
	Ctx.pStream->Read((char *)&m_iImageRotation, sizeof(DWORD));
	Ctx.pStream->Read((char *)&m_iTick, sizeof(DWORD));

	//	We don't bother loading this object (since it is just an effect and it
	//	is hard to save the required bitmaps)

	DWORD dwLoad;
	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));

	m_pParticles = NULL;
	m_iParticleCount = 0;
	m_iLifeTime = 0;
	}

void CFractureEffect::OnUpdate (SUpdateCtx &Ctx, Metric rSecondsPerTick)

//	OnUpdate
//
//	Update the effect

	{
	//	See if the effect has faded out

	m_iTick++;
	if (m_iTick >= m_iLifeTime)
		{
		Destroy(removedFromSystem, CDamageSource());
		return;
		}

	//	Update particles

	if (m_pParticles)
		{
		SParticle *pParticle = m_pParticles;
		SParticle *pParticleEnd = pParticle + m_iParticleCount;

		//	If we have an attractor, accelerate all particles towards it

		if (m_pAttractor)
			{
			bool bParticlesLeft = false;

			//	Compute the position of the attractor in the particle
			//	coordinate system.

			CVector vDist = m_pAttractor->GetPos() - GetPos();
			int xAttract = FIXED_POINT * (int)(vDist.GetX() / g_KlicksPerPixel);
			int yAttract = -FIXED_POINT * (int)(vDist.GetY() / g_KlicksPerPixel);

			//	If the attractor is a gravity source, then handle it differently

			if (m_pAttractor->HasGravity())
				{
				int iMinRadius = 50 * FIXED_POINT;
				int iMaxAccel = 5 * FIXED_POINT;
				Metric rMinRadius2 = (Metric)iMinRadius * (Metric)iMinRadius;

				while (pParticle < pParticleEnd)
					{
					if (pParticle->iShape == -1)
						{
						pParticle++;
						continue;
						}

					int xDelta = xAttract - pParticle->x;
					int yDelta = yAttract - pParticle->y;
					int xDist = abs(xDelta);
					int yDist = abs(yDelta);
					int iDist = (int)sqrt((Metric)xDist * (Metric)xDist + (Metric)yDist * (Metric)yDist);

					//	If within threshold, then particles is destroyed

					if (iDist < iMinRadius)
						pParticle->iShape = -1;

					//	Otherwise, move towards attractor

					else
						{
						Metric rDist2 = (Metric)iDist * (Metric)iDist;
						int iAccel = (int)(iDist > iMinRadius ? rMinRadius2 * iMaxAccel / rDist2 : iMaxAccel);
						int xA = iAccel * xDelta / iDist;
						int yA = iAccel * yDelta / iDist;

						if (xA == 0 && yA == 0)
							{
							xA = Sign(xDelta);
							yA = Sign(yDelta);
							}

						pParticle->xV += xA;
						pParticle->yV += yA;

						pParticle->x += pParticle->xV;
						pParticle->y += pParticle->yV;

						bParticlesLeft = true;
						}

					pParticle++;
					}
				}

			//	Otherwise, this is an attractor like a Gaian terraformer

			else
				{
				while (pParticle < pParticleEnd)
					{
					if (pParticle->iShape == -1)
						{
						pParticle++;
						continue;
						}

					int xDelta = xAttract - pParticle->x;
					int yDelta = yAttract - pParticle->y;
					int xDist = abs(xDelta);
					int yDist = abs(yDelta);

					//	If within threshold, then particles do not move

					if (xDist < (8 * FIXED_POINT) && yDist < (8 * FIXED_POINT))
						pParticle->iShape = -1;

					//	Otherwise, move towards attractor

					else
						{
						int iDist = Max(xDist, yDist);
						int iSpeed = 256 * 256 * FIXED_POINT / iDist;
						iSpeed = Min(iSpeed, 4 * FIXED_POINT);

						int iFade = (m_iTick > 100 ? 0 : (100 - m_iTick));
						int xV = (iSpeed * xDelta / iDist) + (iFade * pParticle->xV / 100);
						int yV = (iSpeed * yDelta / iDist) + (iFade * pParticle->yV / 100);

						pParticle->x += xV;
						pParticle->y += yV;

						bParticlesLeft = true;
						}

					pParticle++;
					}
				}

			//	If no particles left, destroy the effect

			if (!bParticlesLeft)
				{
				Destroy(removedFromSystem, CDamageSource());
				return;
				}
			}

		//	Otherwise, follow normal trajectory

		else
			{
			while (pParticle < pParticleEnd)
				{
				pParticle->x += pParticle->xV;
				pParticle->y += pParticle->yV;

				pParticle++;
				}
			}
		}
	else
		{
		Destroy(removedFromSystem, CDamageSource());
		return;
		}

	//	If we're moving, slow down

	SetVel(CVector(GetVel().GetX() * g_SpaceDragFactor, GetVel().GetY() * g_SpaceDragFactor));
	}

void CFractureEffect::OnUpdateExtended (const CTimeSpan &ExtraTime)

//	OnUpdateExtended
//
//	Update a prolonged time

	{
	//	Updates don't last for an extended period of time

	Destroy(removedFromSystem, CDamageSource());
	}

void CFractureEffect::OnWriteToStream (IWriteStream *pStream)

//	OnWriteToStream
//
//	Write the object's data to stream
//
//	Image		m_Image
//	DWORD		m_iImageTick
//	DWORD		m_iImageRotation
//	DWORD		m_iTick

	{
	m_Image.WriteToStream(pStream);
	pStream->Write((char *)&m_iImageTick, sizeof(DWORD));
	pStream->Write((char *)&m_iImageRotation, sizeof(DWORD));
	pStream->Write((char *)&m_iTick, sizeof(DWORD));

	//	We don't bother loading this object (since it is just an effect and it
	//	is hard to save the required bitmaps)

	DWORD dwSave = 0;
	pStream->Write((char *)&dwSave, sizeof(DWORD));
	}

void CFractureEffect::SetAttractor (CSpaceObject *pObj)

//	SetAttractor
//
//	Sets the point towards which the particles will move

	{ 
	m_pAttractor = pObj;

	//	In attractor mode it takes us longer, so we increase our lifespan
	//	(We will automatically die when all our particles are gone anyway).

	m_iLifeTime = 1200;
	}
