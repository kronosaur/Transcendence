//	SFXVariants.cpp
//
//	CEffectVariantCreator class
//	Copyright (c) 2012 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"

#define MAX_VALUE_ATTRIB						CONSTLIT("maxValue")

class CEffectVariantPainter : public IEffectPainter
	{
	public:
		CEffectVariantPainter (CEffectVariantCreator *pCreator);
		~CEffectVariantPainter (void);

		//	IEffectPainter virtuals
		virtual CEffectCreator *GetCreator (void) { return m_pCreator; }
		virtual void Paint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx);

	private:
		CEffectVariantCreator *m_pCreator;
		TArray<CEffectPainterRef> m_Cache;
	};

CEffectVariantCreator::CEffectVariantCreator (void)

//	CEffectVariantCreator constructor

	{
	}

CEffectVariantCreator::~CEffectVariantCreator (void)

//	CEffectVariantCreator destructor

	{
	int i;

	for (i = 0; i < m_Effects.GetCount(); i++)
		delete m_Effects[i].pEffect;
	}

CEffectVariantCreator::SEntry *CEffectVariantCreator::ChooseVariant (int iVariantValue, int *retiIndex)

//	ChooseVariant
//
//	Choose a variant

	{
	int i;

	for (i = 0; i < m_Effects.GetCount() - 1; i++)
		{
		//	If the maxValue property for this effect is less than or equal to
		//	the variant	value, then we choose this effect.

		if (iVariantValue <= m_Effects[i].iMaxValue)
			{
			if (retiIndex)
				*retiIndex = i;
			return &m_Effects[i];
			}
		}

	//	If we get this far then we choose the last effect as a default

	if (retiIndex)
		*retiIndex = m_Effects.GetCount() - 1;

	return &m_Effects[m_Effects.GetCount() - 1];
	}

ALERROR CEffectVariantCreator::CreateEffect (CSystem *pSystem,
										   CSpaceObject *pAnchor,
										   const CVector &vPos,
										   const CVector &vVel,
										   int iRotation,
										   int iVariant,
										   CSpaceObject **retpEffect)

//	CreateEffect
//
//	Creates an effect object

	{
	return ChooseVariant(iVariant)->pEffect->CreateEffect(pSystem, pAnchor, vPos, vVel, iRotation, iVariant, retpEffect);
	}

IEffectPainter *CEffectVariantCreator::CreatePainter (CCreatePainterCtx &Ctx)

//	CreatePainter
//
//	Creates a painter

	{
	return new CEffectVariantPainter(this);
	}

int CEffectVariantCreator::GetLifetime (void)

//	GetLifetime
//
//	Returns the lifetime of the effect
	
	{
	int iTotalLifetime = 0;
	for (int i = 0; i < m_Effects.GetCount(); i++)
		{
		int iLifetime = m_Effects[i].pEffect->GetLifetime();
		if (iLifetime == -1)
			{
			iTotalLifetime = -1;
			break;
			}
		else if (iLifetime > iTotalLifetime)
			iTotalLifetime = iLifetime;
		}

	return iTotalLifetime;
	}

ALERROR CEffectVariantCreator::OnEffectCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, const CString &sUNID)

//	OnEffectCreateFromXML
//
//	Creates from XML

	{
	ALERROR error;
	int i;

	//	Allocate the creator array

	int iCount = pDesc->GetContentElementCount();
	if (iCount == 0)
		{
		Ctx.sError = CONSTLIT("<Variants> effect must have at least one sub-element.");
		return ERR_FAIL;
		}

	m_Effects.InsertEmpty(iCount);

	for (i = 0; i < iCount; i++)
		{
		CString sSubUNID = strPatternSubst(CONSTLIT("%s/%d"), sUNID, i);

		CXMLElement *pCreatorDesc = pDesc->GetContentElement(i);
		if (error = CEffectCreator::CreateFromXML(Ctx, pCreatorDesc, sSubUNID, &m_Effects[i].pEffect))
			return error;

		m_Effects[i].iMaxValue = pCreatorDesc->GetAttributeInteger(MAX_VALUE_ATTRIB);
		}

	return NOERROR;
	}

ALERROR CEffectVariantCreator::OnEffectBindDesign (SDesignLoadCtx &Ctx)

//	OnEffectBindDesign
//
//	Resolve loading

	{
	ALERROR error;

	for (int i = 0; i < m_Effects.GetCount(); i++)
		if (error = m_Effects[i].pEffect->BindDesign(Ctx))
			return error;

	return NOERROR;
	}

void CEffectVariantCreator::OnMarkImages (void)

//	OnMarkImages
//
//	Mark images used by this effect

	{
	for (int i = 0; i < m_Effects.GetCount(); i++)
		m_Effects[i].pEffect->MarkImages();
	}

void CEffectVariantCreator::SetLifetime (int iLifetime)

//	SetLifetime
//
//	Sets the lifetime

	{
	for (int i = 0; i < m_Effects.GetCount(); i++)
		m_Effects[i].pEffect->SetLifetime(iLifetime);
	}

void CEffectVariantCreator::SetVariants (int iVariants)

//	SetVariants
//
//	Sets the variants

	{
	for (int i = 0; i < m_Effects.GetCount(); i++)
		m_Effects[i].pEffect->SetVariants(iVariants);
	}

//	CEffectVariantPainter object -----------------------------------------------

CEffectVariantPainter::CEffectVariantPainter (CEffectVariantCreator *pCreator) : m_pCreator(pCreator)

//	CEffectVariantPainter constructor
	
	{
	m_Cache.InsertEmpty(pCreator->GetVariantCount());
	}

CEffectVariantPainter::~CEffectVariantPainter (void)

//	CEffectVariantPainter destructor

	{
	int i;

	for (i = 0; i < m_Cache.GetCount(); i++)
		m_Cache[i].Delete();
	}

void CEffectVariantPainter::Paint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx)

//	Paint
//
//	Paint the effect

	{
	//	Find the appropriate effect

	int iIndex = m_pCreator->GetVariantCreatorIndex(Ctx.iVariant);
	if (m_Cache[iIndex].IsEmpty())
		{
		CEffectCreator *pCreator = m_pCreator->GetVariantCreator(iIndex);
		m_Cache[iIndex].Set(pCreator->CreatePainter(CCreatePainterCtx()));
		}

	//	Paint

	m_Cache[iIndex]->Paint(Dest, x, y, Ctx);
	}
