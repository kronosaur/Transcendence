//	SFXParticleExplosion.cpp
//
//	Particle explosion effect

#include "PreComp.h"

#define IMAGE_TAG								(CONSTLIT("Image"))

#define PARTICLE_COUNT_ATTRIB					(CONSTLIT("particleCount"))
#define PARTICLE_SPEED_ATTRIB					(CONSTLIT("particleSpeed"))
#define PARTICLE_LIFETIME_ATTRIB				(CONSTLIT("particleLifetime"))
#define LIFETIME_ATTRIB							(CONSTLIT("lifetime"))

ALERROR CParticleExplosionEffectCreator::CreateEffect (CSystem *pSystem,
													   CSpaceObject *pAnchor,
													   const CVector &vPos,
													   const CVector &vVel,
													   int iRotation,
													   int iVariant,
													   CSpaceObject **retpEffect)

//	CreateEffect
//
//	Creates the effect object

	{
	ALERROR error;

	//	Create the effect

	CParticleEffect *pObj;
	if (error = CParticleEffect::CreateExplosion(pSystem,
			pAnchor,
			vPos,
			vVel,
			m_ParticleCount.Roll(),
			m_rParticleSpeed,
			m_Lifetime.Roll(),
			m_iParticleLifetime,
			m_Image,
			&pObj))
		return error;

	//	Play Sound

	PlaySound(pObj);

	//	Done

	if (retpEffect)
		*retpEffect = pObj;

	return NOERROR;
	}

ALERROR CParticleExplosionEffectCreator::OnEffectCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, const CString &sUNID)

//	OnEffectCreateFromXML
//
//	Load from XML

	{
	ALERROR error;

	if (error = m_Lifetime.LoadFromXML(pDesc->GetAttribute(LIFETIME_ATTRIB)))
		return error;

	if (error = m_ParticleCount.LoadFromXML(pDesc->GetAttribute(PARTICLE_COUNT_ATTRIB)))
		return error;

	m_rParticleSpeed = LIGHT_SPEED * pDesc->GetAttributeInteger(PARTICLE_SPEED_ATTRIB) / 100.0;
	m_iParticleLifetime = pDesc->GetAttributeInteger(PARTICLE_LIFETIME_ATTRIB);

	CXMLElement *pImage = pDesc->GetContentElementByTag(IMAGE_TAG);
	if (pImage)
		{
		if (error = m_Image.InitFromXML(Ctx, pImage))
			return error;
		}

	return NOERROR;
	}

ALERROR CParticleExplosionEffectCreator::OnEffectBindDesign (SDesignLoadCtx &Ctx)

//	OnEffectBindDesign
//
//	Resolve loading

	{
	ALERROR error;

	if (error = m_Image.OnDesignLoadComplete(Ctx))
		return error;

	return NOERROR;
	}

