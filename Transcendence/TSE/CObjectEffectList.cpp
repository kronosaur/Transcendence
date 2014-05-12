//	CObjectEffectList.cpp
//
//	CObjectEffectList class
//	Copyright (c) 2014 by George Moromisato. All Rights Reserved.

#include "PreComp.h"

CObjectEffectList::~CObjectEffectList (void)

//	CObjectEffectList destructor

	{
	CleanUp();
	}

void CObjectEffectList::AccumulateBounds (CSpaceObject *pObj, const CObjectEffectDesc &Desc, int iRotation, RECT *ioBounds)

//	AccumulateBounds
//
//	Increases ioBounds to account for effects.

	{
	int i;

	for (i = 0; i < m_FixedEffects.GetCount(); i++)
		{
		const CObjectEffectDesc::SEffectDesc &EffectDesc = Desc.GetEffectDesc(i);

		if (m_FixedEffects[i].pPainter)
			{
			//	Figure out where to paint the effect

			int xPainter, yPainter;
			EffectDesc.PosCalc.GetCoord(iRotation, &xPainter, &yPainter);

			//	Get the painter bounds

			RECT rcRect;
			m_FixedEffects[i].pPainter->GetBounds(&rcRect);

			//	Offset the painter rect based on position

			::OffsetRect(&rcRect, xPainter, yPainter);

			//	Adjust resulting RECT to include painter rect

			::UnionRect(ioBounds, ioBounds, &rcRect);
			}
		}
	}

void CObjectEffectList::CleanUp (void)

//	CleanUp
//
//	Clean up effects

	{
	int i;

	for (i = 0; i < m_FixedEffects.GetCount(); i++)
		if (m_FixedEffects[i].pPainter)
			m_FixedEffects[i].pPainter->Delete();

	m_FixedEffects.DeleteAll();
	}

void CObjectEffectList::Init (const CObjectEffectDesc &Desc, const TArray<IEffectPainter *> &Painters)

//	Init
//
//	Initialize the effect list

	{
	int i;
	ASSERT(Desc.GetEffectCount() == Painters.GetCount());

	CleanUp();
	
	m_FixedEffects.InsertEmpty(Desc.GetEffectCount());
	for (i = 0; i < m_FixedEffects.GetCount(); i++)
		m_FixedEffects[i].pPainter = Painters[i];
	}

void CObjectEffectList::Move (CSpaceObject *pObj, const CVector &vOldPos, bool *retbBoundsChanged)

//	Move
//
//	Effects move

	{
	int i;
	bool bBoundsChanged = false;

	SEffectMoveCtx MoveCtx;
	MoveCtx.pObj = pObj;
	MoveCtx.vOldPos = vOldPos;

	for (i = 0; i < m_FixedEffects.GetCount(); i++)
		if (m_FixedEffects[i].pPainter)
			{
			bool bChanged;
			m_FixedEffects[i].pPainter->OnMove(MoveCtx, &bChanged);
			if (bChanged)
				bBoundsChanged = true;
			}

	if (retbBoundsChanged)
		*retbBoundsChanged = bBoundsChanged;
	}

void CObjectEffectList::Paint (SViewportPaintCtx &Ctx, const CObjectEffectDesc &Desc, DWORD dwEffects, CG16bitImage &Dest, int x, int y)

//	Paint
//
//	Paint the effects in the dwEffects mask

	{
	int i;
	int iObjRotation = Ctx.iRotation;

	for (i = 0; i < m_FixedEffects.GetCount(); i++)
		{
		const CObjectEffectDesc::SEffectDesc &EffectDesc = Desc.GetEffectDesc(i);

		//	Skip effects that are not in the right plane (in front or behind) OR
		//	that don't have a painter.

		if (m_FixedEffects[i].pPainter == NULL
				|| EffectDesc.PosCalc.PaintFirst(Ctx.iVariant) == Ctx.bInFront)
			continue;

		//	Skip maneuvering effects unless we want them

		if (!Ctx.fShowManeuverEffects && CObjectEffectDesc::IsManeuverEffect(EffectDesc))
			continue;

		//	Figure out where to paint the effect

		int xPainter, yPainter;
		EffectDesc.PosCalc.GetCoordFromDir(Ctx.iVariant, &xPainter, &yPainter);

		//	Compute the rotation (180 for thruster effects)

		Ctx.iRotation = AngleMod(iObjRotation + EffectDesc.iRotation + 180);

		//	Paint

		if (dwEffects & EffectDesc.iType)
			m_FixedEffects[i].pPainter->Paint(Dest,
					x + xPainter,
					y + yPainter,
					Ctx);
		else
			m_FixedEffects[i].pPainter->PaintFade(Dest,
					x + xPainter,
					y + yPainter,
					Ctx);
		}

	Ctx.iRotation = iObjRotation;
	}

void CObjectEffectList::PaintAll (SViewportPaintCtx &Ctx, const CObjectEffectDesc &Desc, CG16bitImage &Dest, int x, int y)

//	PaintAll
//
//	Paint all effects

	{
	DWORD dwAll = CObjectEffectDesc::effectThrustLeft
			| CObjectEffectDesc::effectThrustRight
			| CObjectEffectDesc::effectThrustMain;

	//	Set tick to 1 so we don't blink

	int iOldTick = Ctx.iTick;
	Ctx.iTick = 1;

	//	Paint

	Paint(Ctx, Desc, dwAll,	Dest, x, y);

	//	Done

	Ctx.iTick = iOldTick;
	}

void CObjectEffectList::Update (CSpaceObject *pObj, const CObjectEffectDesc &Desc, int iRotation, DWORD dwEffects)

//	Update
//
//	Update effects

	{
	int i;
	SEffectUpdateCtx PainterCtx;
	PainterCtx.pObj = pObj;

	for (i = 0; i < m_FixedEffects.GetCount(); i++)
		if (m_FixedEffects[i].pPainter)
			{
			//	If bFade is TRUE then it means that we don't emit new particles
			//	out of particle effects (other effects are hidden).

			PainterCtx.bFade = ((dwEffects & Desc.GetEffectDesc(i).iType) == 0);

			//	Compute the position of the effect (relative to the object center)
			//	so we can emit at that location. We need this because some effects
			//	(particle effects) are centered on the object, not the emit position.

			int xEmit;
			int yEmit;
			Desc.GetEffectDesc(i).PosCalc.GetCoord(iRotation, &xEmit, &yEmit);
			PainterCtx.vEmitPos = CVector(xEmit * g_KlicksPerPixel, -yEmit * g_KlicksPerPixel);

			//	Compute the rotation (180 for thruster effects)

			PainterCtx.iRotation = AngleMod(iRotation + Desc.GetEffectDesc(i).iRotation + 180);

			//	Update

			m_FixedEffects[i].pPainter->OnUpdate(PainterCtx);
			}
	}
