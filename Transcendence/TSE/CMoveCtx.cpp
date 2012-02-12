//	CMoveCtx.cpp
//
//	CMoveCtx class

#include "PreComp.h"

CMoveCtx::CMoveCtx (int iAlloc)

//	CMoveCtx constructor

	{
	ASSERT(iAlloc >= 0);

	m_iAlloc = iAlloc;
	m_pArray = (iAlloc > 0 ? new SEntry [iAlloc] : NULL);
	m_iCount = 0;
	}

CMoveCtx::~CMoveCtx (void)

//	CMoveCtx destrutor

	{
	if (m_pArray)
		delete [] m_pArray;
	}

void CMoveCtx::AddObj (CSpaceObject *pObj)

//	AddObj
//
//	Add the object to the move context

	{
	if (m_iCount < m_iAlloc)
		{
		SEntry *pEntry = &m_pArray[m_iCount++];

		pEntry->pObj = pObj;
		pEntry->vOldPos = pObj->GetPos();

		pEntry->bMobile = (pObj->IsMobile() && !pObj->IsTimeStopped());
		pEntry->bBarrier = pObj->IsBarrier();

		//	If we moved this tick, then compute bounds around the whole motion
		//	Otherwise, we just compute static bounds

		CVector vBoundsDiag = pObj->GetBoundsDiag();
		CVector vVel = pObj->GetVel();
		if (pEntry->bMobile && !vVel.IsNull())
			{
			pEntry->bHasMoved = true;
			pEntry->vNewPos = pEntry->vOldPos + (vVel * g_SecondsPerUpdate);

			if (pEntry->vOldPos.GetX() < pEntry->vNewPos.GetX())
				{
				pEntry->vBoundsLL.SetX(pEntry->vOldPos.GetX() - vBoundsDiag.GetX());
				pEntry->vBoundsUR.SetX(pEntry->vNewPos.GetX() + vBoundsDiag.GetX());
				}
			else
				{
				pEntry->vBoundsLL.SetX(pEntry->vNewPos.GetX() - vBoundsDiag.GetX());
				pEntry->vBoundsUR.SetX(pEntry->vOldPos.GetX() + vBoundsDiag.GetX());
				}

			if (pEntry->vOldPos.GetY() < pEntry->vNewPos.GetY())
				{
				pEntry->vBoundsLL.SetY(pEntry->vOldPos.GetY() - vBoundsDiag.GetY());
				pEntry->vBoundsUR.SetY(pEntry->vNewPos.GetY() + vBoundsDiag.GetY());
				}
			else
				{
				pEntry->vBoundsLL.SetY(pEntry->vNewPos.GetY() - vBoundsDiag.GetY());
				pEntry->vBoundsUR.SetY(pEntry->vOldPos.GetY() + vBoundsDiag.GetY());
				}
			}
		else
			{
			pEntry->bHasMoved = false;
			pEntry->vNewPos = pEntry->vOldPos;
			pEntry->vBoundsUR = pEntry->vOldPos + vBoundsDiag;
			pEntry->vBoundsLL = pEntry->vOldPos - vBoundsDiag;
			}
		}
	}

void CMoveCtx::BounceObj (int iIndex)

//	BounceObj
//
//	Figure out if the object bounced off a barrier. If so, adjust the new position
//	and the object velocity

	{
#if 0
	int i;

	SEntry *pEntry = &m_pArray[iIndex];
	if (pEntry->bHasMoved)
		{
		bool bBlocked = false;

		//	Loop over all barriers and see if we intersect any of them

		for (i = 0; i < m_iCount; i++)
			{
			SEntry *pTarget = &m_pArray[i];
			if (pTarget->bBarrier 
					&& i != iIndex
					&& pTarget->pObj->CanBlock(pEntry->pObj)
					&& IntersectRect(pEntry->vBoundsUR, pEntry->vBoundsLL, pTarget->vBoundsUR, pTarget->vBoundsLL)
					&& pEntry->pObj->ObjectInObject(pEntry->vNewPos, pTarget->pObj, pTarget->vNewPos))
				{
				//	Compute the resulting velocities depending
				//	on whether the barrier moves or not

				if (pTarget->bMobile)
					{
					//	For a head-on elastic collision where
					//	the second object has velocity 0, the equations are:
					//
					//		  (m1 - m2)
					//	v1' = --------- v1
					//		  (m1 + m2)
					//
					//		    2m1
					//	v2' = --------- v1
					//		  (m1 + m2)
					
					Metric rObjMass = pEntry->pObj->GetMass();
					Metric rTargetMass = pTarget->pObj->GetMass();
					Metric rInvM1plusM2 = g_BounceCoefficient / (rObjMass + rTargetMass);
					Metric rM1minusM2 = rObjMass - rTargetMass;
					Metric r2M1 = 2.0 * rObjMass;
					CVector vVel = pEntry->pObj->GetVel();

					pEntry->vNewPos = pEntry->vOldPos;

					pEntry->pObj->SetVel(rM1minusM2 * rInvM1plusM2 * vVel);
					pTarget->pObj->SetVel(r2M1 * rInvM1plusM2 * vVel);
					}
				else
					{
					//	If we've already been blocked, then make sure that we are not inside
					//	the second barrier. If we are, then revert the position

					if (bBlocked)
						{
						if (pTarget->pObj->PointInObject(pTarget->vNewPos, pEntry->vNewPos))
							pEntry->vNewPos = pEntry->vOldPos;
						}

					//	Otherwise, deal with the first barrier

					else
						{
						//	Revert the position to before the move

						pEntry->vNewPos = pEntry->vOldPos;

						//	If the old position is not blocked, then bounce and carry on

						if (!pEntry->pObj->ObjectInObject(pEntry->vNewPos, pTarget->pObj, pTarget->vNewPos))
							pEntry->pObj->SetVel(-g_BounceCoefficient * pEntry->pObj->GetVel());

						//	Otherwise, move slowly towards the new position, but make sure that we never
						//	move the center of the object inside the barrier.

						else
							{
							CVector vNewPos = pEntry->vNewPos + (g_KlicksPerPixel * m_vVel.Normal());
							if (!pTarget->pObj->PointInObject(pTarget->vNewPos, vNewPos))
								pEntry->vNewPos = vNewPos;

							ClipSpeed(0.01 * LIGHT_SPEED);
							}
						}
					}

				//	Tell the barrier

				pTarget->pObj->OnObjBounce(pEntry->pObj, pEntry->vNewPos);

				//	Remember that we already dealt with one barrier

				bBlocked = true;
				}
			}
		}
#endif
	}
