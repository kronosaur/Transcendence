//	CStaticEffect.cpp
//
//	CStaticEffect class

#include "PreComp.h"


static CObjectClass<CStaticEffect>g_Class(OBJID_CSTATICEFFECT, NULL);

CStaticEffect::CStaticEffect (void) : CSpaceObject(&g_Class), m_pPainter(NULL)

//	CStaticEffect constructor

	{
	}

CStaticEffect::~CStaticEffect (void)

//	CStaticEffect destructor

	{
	if (m_pPainter)
		m_pPainter->Delete();
	}

ALERROR CStaticEffect::Create (CEffectCreator *pType,
				CSystem *pSystem,
				const CVector &vPos,
				CStaticEffect **retpEffect)

//	Create
//
//	Creates a new effects object

	{
	ALERROR error;
	CStaticEffect *pEffect;

	pEffect = new CStaticEffect;
	if (pEffect == NULL)
		return ERR_MEMORY;

	pEffect->Place(vPos);
	pEffect->SetCannotMove();

	ASSERT(pType);
	pEffect->m_pPainter = pType->CreatePainter(CCreatePainterCtx());

	//	Set the size of the object

	if (pEffect->m_pPainter)
		pEffect->SetBounds(pEffect->m_pPainter);

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
	}

void CStaticEffect::OnPaint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx)

//	OnPaint
//
//	Paints the effect

	{
	Ctx.iTick = 0;
	Ctx.iVariant = GetDestiny();
	Ctx.iDestiny = GetDestiny();
	m_pPainter->Paint(Dest, x, y, Ctx);
	}

void CStaticEffect::OnReadFromStream (SLoadCtx &Ctx)

//	OnReadFromStream
//
//	Read object data from a stream
//
//	DWORD		IEffectPainter

	{
#ifdef DEBUG_LOAD
	::OutputDebugString("CStaticEffect::OnReadFromStream\n");
#endif
	m_pPainter = CEffectCreator::CreatePainterFromStream(Ctx);
	}

void CStaticEffect::OnWriteToStream (IWriteStream *pStream)

//	OnWriteToStream
//
//	Write the object's data to stream
//
//	DWORD		IEffectPainter

	{
	CEffectCreator::WritePainterToStream(pStream, m_pPainter);
	}

