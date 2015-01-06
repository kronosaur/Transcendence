//	CEffectCreator.cpp
//
//	CEffectCreator class

#include "PreComp.h"

#define BOLT_TAG								CONSTLIT("Bolt")
#define DAMAGE_TAG								CONSTLIT("Damage")
#define EFFECT_TAG								CONSTLIT("Effect")
#define EVENTS_TAG								CONSTLIT("Events")
#define FLARE_TAG								CONSTLIT("Flare")
#define IMAGE_TAG								CONSTLIT("Image")
#define IMAGE_AND_TAIL_TAG						CONSTLIT("ImageAndTail")
#define MOLTEN_BOLT_TAG							CONSTLIT("MoltenBolt")
#define NULL_TAG								CONSTLIT("Null")
#define PARTICLE_TAG							CONSTLIT("Particle")
#define PARTICLE_COMET_TAG						CONSTLIT("ParticleComet")
#define PARTICLE_EXPLOSION_TAG					CONSTLIT("ParticleExplosion")
#define PLASMA_SPHERE_TAG						CONSTLIT("PlasmaSphere")
#define POLYFLASH_TAG							CONSTLIT("Polyflash")
#define SHAPE_TAG								CONSTLIT("Shape")
#define SHOCKWAVE_TAG							CONSTLIT("Shockwave")
#define SMOKE_TRAIL_TAG							CONSTLIT("SmokeTrail")
#define STARBURST_TAG							CONSTLIT("Starburst")

#define INSTANCE_ATTRIB							CONSTLIT("instance")
#define SOUND_ATTRIB							CONSTLIT("sound")
#define UNID_ATTRIB								CONSTLIT("UNID")

#define INSTANCE_CREATOR						CONSTLIT("creator")
#define INSTANCE_GAME							CONSTLIT("game")
#define INSTANCE_OWNER							CONSTLIT("owner")

#define STR_NO_UNID								CONSTLIT("(no UNID)")

static char *CACHED_EVENTS[CEffectCreator::evtCount] =
	{
		"GetParameters",
	};

CEffectCreator::CEffectCreator (void) : m_pDamage(NULL)

//	CEffectCreator constructor

	{
	utlMemSet(m_CachedEvents, sizeof(m_CachedEvents), 0);
	}

CEffectCreator::~CEffectCreator (void)

//	CEffectCreator destructor

	{
	if (m_pDamage)
		delete m_pDamage;
	}

ALERROR CEffectCreator::CreateBeamEffect (SDesignLoadCtx &Ctx, CXMLElement *pDesc, const CString &sUNID, CEffectCreator **retpCreator)

//	CreateBeamEffect
//
//	Create a beam effect

	{
	ALERROR error;

	//	Create the effect

	CEffectCreator *pCreator = new CBeamEffectCreator;
	pCreator->m_sUNID = sUNID;

	//	Type-specific creation

	if (error = pCreator->OnEffectCreateFromXML(Ctx, pDesc, sUNID))
		return error;

	//	Done

	*retpCreator = pCreator;

	return NOERROR;
	}

ALERROR CEffectCreator::CreateEffect (CSystem *pSystem,
									  CSpaceObject *pAnchor,
									  const CVector &vPos,
									  const CVector &vVel,
									  int iRotation,
									  int iVariant,
									  CSpaceObject **retpEffect)

//	CreateEffect
//
//	Default creation of effect (using CEffect)

	{
	ALERROR error;
	CEffect *pEffect;

	if (error = CEffect::Create(this,
			pSystem,
			pAnchor,
			vPos,
			vVel,
			iRotation,
			&pEffect))
		return error;

	if (retpEffect)
		*retpEffect = pEffect;

	return NOERROR;
	}

ALERROR CEffectCreator::CreateFromTag (const CString &sTag, CEffectCreator **retpCreator)

//	CreateFromTag
//
//	Creates effect creator from a tag

	{
	CEffectCreator *pCreator;

	if (strEquals(sTag, CImageEffectCreator::GetClassTag()))
		pCreator = new CImageEffectCreator;
	else if (strEquals(sTag, CRayEffectCreator::GetClassTag()))
		pCreator = new CRayEffectCreator;
	else if (strEquals(sTag, CParticleJetEffectCreator::GetClassTag()))
		pCreator = new CParticleJetEffectCreator;
	else if (strEquals(sTag, COrbEffectCreator::GetClassTag()))
		pCreator = new COrbEffectCreator;
	else if (strEquals(sTag, CLightningStormEffectCreator::GetClassTag()))
		pCreator = new CLightningStormEffectCreator;
	else if (strEquals(sTag, CPolyflashEffectCreator::GetClassTag()))
		pCreator = new CPolyflashEffectCreator;
	else if (strEquals(sTag, CShockwaveEffectCreator::GetClassTag()))
		pCreator = new CShockwaveEffectCreator;
	else if (strEquals(sTag, CStarburstEffectCreator::GetClassTag()))
		pCreator = new CStarburstEffectCreator;
	else if (strEquals(sTag, CPlasmaSphereEffectCreator::GetClassTag()))
		pCreator = new CPlasmaSphereEffectCreator;
	else if (strEquals(sTag, CBeamEffectCreator::GetClassTag()))
		pCreator = new CBeamEffectCreator;
	else if (strEquals(sTag, CBoltEffectCreator::GetClassTag()))
		pCreator = new CBoltEffectCreator;
	else if (strEquals(sTag, CMoltenBoltEffectCreator::GetClassTag()))
		pCreator = new CMoltenBoltEffectCreator;
	else if (strEquals(sTag, CParticleExplosionEffectCreator::GetClassTag()))
		pCreator = new CParticleExplosionEffectCreator;
	else if (strEquals(sTag, CImageAndTailEffectCreator::GetClassTag()))
		pCreator = new CImageAndTailEffectCreator;
	else if (strEquals(sTag, CFlareEffectCreator::GetClassTag()))
		pCreator = new CFlareEffectCreator;
	else if (strEquals(sTag, CParticleCometEffectCreator::GetClassTag()))
		pCreator = new CParticleCometEffectCreator;
	else if (strEquals(sTag, CSmokeTrailEffectCreator::GetClassTag()))
		pCreator = new CSmokeTrailEffectCreator;
	else if (strEquals(sTag, CSingleParticleEffectCreator::GetClassTag()))
		pCreator = new CSingleParticleEffectCreator;
	else if (strEquals(sTag, CShapeEffectCreator::GetClassTag()))
		pCreator = new CShapeEffectCreator;
	else if (strEquals(sTag, CImageFractureEffectCreator::GetClassTag()))
		pCreator = new CImageFractureEffectCreator;
	else if (strEquals(sTag, CEffectSequencerCreator::GetClassTag()))
		pCreator = new CEffectSequencerCreator;
	else if (strEquals(sTag, CEffectGroupCreator::GetClassTag()))
		pCreator = new CEffectGroupCreator;
	else if (strEquals(sTag, CNullEffectCreator::GetClassTag()))
		pCreator = new CNullEffectCreator;
	else if (strEquals(sTag, CParticleCloudEffectCreator::GetClassTag()))
		pCreator = new CParticleCloudEffectCreator;
	else if (strEquals(sTag, CTextEffectCreator::GetClassTag()))
		pCreator = new CTextEffectCreator;
	else if (strEquals(sTag, CEllipseEffectCreator::GetClassTag()))
		pCreator = new CEllipseEffectCreator;
	else if (strEquals(sTag, CEffectVariantCreator::GetClassTag()))
		pCreator = new CEffectVariantCreator;
	else
		return ERR_FAIL;

	//	Done

	if (retpCreator)
		*retpCreator = pCreator;

	return NOERROR;
	}

ALERROR CEffectCreator::CreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, const CString &sUNID, CEffectCreator **retpCreator)

//	CreateFromXML
//
//	Creates the creator from an XML element

	{
	ALERROR error;
	CEffectCreator *pCreator;

	//	Basic info

	CString sEffectUNID = sUNID;
	if (sEffectUNID.IsBlank())
		{
		DWORD dwUNID = pDesc->GetAttributeInteger(UNID_ATTRIB);
		if (dwUNID)
			sEffectUNID = strFromInt(dwUNID, false);
		else
			sEffectUNID = STR_NO_UNID;
		}

	//	Create the effect based on the child tag

	if (pDesc->GetContentElementCount() == 0)
		{
		*retpCreator = NULL;
		return NOERROR;
		}
	else if (pDesc->GetContentElementCount() == 1)
		{
		if (error = CreateSimpleFromXML(Ctx, pDesc->GetContentElement(0), sEffectUNID, &pCreator))
			return error;
		}
	else
		{
		pCreator = new CEffectGroupCreator;
		if (pCreator == NULL)
			return ERR_MEMORY;

		pCreator->m_sUNID = sEffectUNID;

		//	Type-specific creation

		if (error = pCreator->OnEffectCreateFromXML(Ctx, pDesc, sEffectUNID))
			return error;
		}

	//	Initialize basic parameters

	if (error = pCreator->InitBasicsFromXML(Ctx, pDesc))
		return error;

	//	Done

	*retpCreator = pCreator;

	return NOERROR;
	}

ALERROR CEffectCreator::CreateSimpleFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, const CString &sUNID, CEffectCreator **retpCreator)

//	CreateSimpleFromXML
//
//	Creates the creator from an XML element

	{
	ALERROR error;
	CEffectCreator *pCreator;

	//	Create the effect based on the tag

	if (error = CreateFromTag(pDesc->GetTag(), &pCreator))
		{
		Ctx.sError = strPatternSubst(CONSTLIT("Invalid painter tag: %s"), pDesc->GetTag());
		return error;
		}

	if (pCreator == NULL)
		return ERR_MEMORY;

	pCreator->m_sUNID = sUNID;
	pCreator->m_iInstance = instCreator;

	//	Load events

	CXMLElement *pEventsDesc = pDesc->GetContentElementByTag(EVENTS_TAG);
	if (pEventsDesc)
		{
		if (error = pCreator->m_Events.InitFromXML(Ctx, pEventsDesc))
			return error;
		}

	//	Type-specific creation

	if (error = pCreator->OnEffectCreateFromXML(Ctx, pDesc, sUNID))
		return error;

	//	Done

	*retpCreator = pCreator;

	return NOERROR;
	}

ALERROR CEffectCreator::CreateTypeFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, CEffectCreator **retpCreator)

//	CreateTypeFromXML
//
//	Creates the class only. This is used by the main Design load code when we use
//	the full <EffectType> definition.

	{
	ALERROR error;
	CEffectCreator *pCreator;

	//	Create the effect based on the child tag

	CXMLElement *pEffect = pDesc->GetContentElementByTag(EFFECT_TAG);
	if (pEffect == NULL)
		{
		Ctx.sError = CONSTLIT("<EffectType> must have an <Effect> sub-element.");
		return ERR_FAIL;
		}

	//	If we've got no sub elements, then its a null creator

	if (pEffect->GetContentElementCount() == 0)
		pCreator = new CNullEffectCreator;

	//	If we've got a single element, then we create a simple creator

	else if (pEffect->GetContentElementCount() == 1)
		{
		const CString &sTag = pEffect->GetContentElement(0)->GetTag();

		if (error = CreateFromTag(sTag, &pCreator))
			{
			Ctx.sError = strPatternSubst(CONSTLIT("Invalid effect tag: %s"), sTag);
			return error;
			}
		}

	//	Otherwise we have a group

	else
		pCreator = new CEffectGroupCreator;

	//	Done

	*retpCreator = pCreator;

	return NOERROR;
	}

IEffectPainter *CEffectCreator::CreatePainterFromStream (SLoadCtx &Ctx, bool bNullCreator)

//	CreatePainterFromStream
//
//	Load a painter from a stream

	{
	CEffectCreator *pCreator;

	//	For previous versions, we only stored UNID if we had a creator

	if (Ctx.dwVersion < 43 && bNullCreator)
		return NULL;

	//	At version 15 we started saving versions as string UNIDs. We need to do this
	//	because sometimes the effect creator is inside a weapon fire desc
	//	structure (also identified by string UNIDs).

	if (Ctx.dwVersion >= 15)
		{
		CString sUNID;
		sUNID.ReadFromStream(Ctx.pStream);

		pCreator = (sUNID.IsBlank() ? NULL : CEffectCreator::FindEffectCreator(sUNID));

		//	Load the creator class that saved the painter

		if (IEffectPainter::ValidateClass(Ctx, (pCreator ? pCreator->GetTag() : NULL_STR)) != NOERROR)
			return NULL;

		//	Error

		if (pCreator == NULL)
			{
			if (!sUNID.IsBlank())
				kernelDebugLogMessage("Invalid painter creator: %s", sUNID);
			return NULL;
			}
		}

	//	Old style uses DWORD UNIDs

	else
		{
		//	The first DWORD is the UNID of the creator

		DWORD dwUNID;
		Ctx.pStream->Read((char *)&dwUNID, sizeof(DWORD));
		if (dwUNID == 0)
			return NULL;

		pCreator = g_pUniverse->FindEffectType(dwUNID);

		//	Error

		if (pCreator == NULL)
			{
			kernelDebugLogMessage("Invalid painter creator: %x", dwUNID);
			return NULL;
			}
		}

	//	Let the creator create the object

	ELoadStates iOldLoadState = Ctx.iLoadState;
	Ctx.iLoadState = loadStateEffect;
	Ctx.sEffectUNID = pCreator->GetUNIDString();

	IEffectPainter *pPainter = pCreator->CreatePainter(CCreatePainterCtx());

	//	Load it

	pPainter->ReadFromStream(Ctx);

	//	Done

	Ctx.iLoadState = iOldLoadState;
	return pPainter;
	}

IEffectPainter *CEffectCreator::CreatePainterFromStreamAndCreator (SLoadCtx &Ctx, CEffectCreator *pCreator)

//	CreatePainterFromStreamAndCreator
//
//	Load a painter from a stream given a creator

	{
	//	Older versions did not save UNID or class if there was no creator

	if (Ctx.dwVersion < 43 && pCreator == NULL)
		return NULL;

	//	The UNID is ignored (because it is the UNID of the creator)

	IEffectPainter::ReadUNID(Ctx);

	//	Validate the class

	if (IEffectPainter::ValidateClass(Ctx, pCreator ? pCreator->GetTag() : NULL_STR) != NOERROR)
		return NULL;

	//	Load it

	if (pCreator == NULL)
		return NULL;

	IEffectPainter *pPainter = pCreator->CreatePainter(CCreatePainterCtx());
	pPainter->ReadFromStream(Ctx);

	//	Done

	return pPainter;
	}

ALERROR CEffectCreator::InitBasicsFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	InitBasicsFromXML
//
//	Initialize some basic parameters.

	{
	ALERROR error;

	//	Initialize sound.

	if (error = m_Sound.LoadUNID(Ctx, pDesc->GetAttribute(SOUND_ATTRIB)))
		return error;

	//	Figure out what kind of instancing model we use

	CString sAttrib;
	if (pDesc->FindAttribute(INSTANCE_ATTRIB, &sAttrib))
		{
		if (strEquals(sAttrib, INSTANCE_GAME))
			m_iInstance = instGame;
		else if (strEquals(sAttrib, INSTANCE_OWNER))
			m_iInstance = instOwner;
		else if (strEquals(sAttrib, INSTANCE_CREATOR))
			m_iInstance = instCreator;
		else
			{
			Ctx.sError = strPatternSubst(CONSTLIT("Invalid instance type: %s"), sAttrib);
			return ERR_FAIL;
			}
		}
	else
		m_iInstance = instCreator;

	//	Done

	return NOERROR;
	}

void CEffectCreator::InitPainterParameters (CCreatePainterCtx &Ctx, IEffectPainter *pPainter)

//	InitPainterParameters
//
//	Initialize painter parameters

	{
	SEventHandlerDesc Event;
	if (FindEventHandlerEffectType(evtGetParameters, &Event))
		{
		CCodeChainCtx CCCtx;

		CCCtx.SaveAndDefineDataVar(Ctx.GetData());

		ICCItem *pResult = CCCtx.Run(Event);
		if (pResult->IsError())
			::kernelDebugLogMessage(CONSTLIT("EffectType %x GetParameters: %s"), GetUNID(), (LPSTR)pResult->GetStringValue());
		else if (pResult->IsSymbolTable())
			{
			int i;
			CCSymbolTable *pTable = (CCSymbolTable *)pResult;

			for (i = 0; i < pTable->GetCount(); i++)
				{
				CString sParam = pTable->GetKey(i);
				ICCItem *pValue = pTable->GetElement(i);
				CEffectParamDesc Value;

				if (pValue->IsNil())
					Value.InitNull();
				else if (pValue->IsInteger())
					Value.InitInteger(pValue->GetIntegerValue());
				else if (pValue->IsIdentifier())
					{
					CString sValue = pValue->GetStringValue();
					char *pPos = sValue.GetASCIIZPointer();

					//	If this is a color, parse it

					if (*pPos == '#')
						Value.InitColor(::LoadRGBColor(sValue));

					//	Otherwise, a string

					else
						Value.InitString(sValue);
					}

				pPainter->SetParam(Ctx, sParam, Value);
				}
			}
		else
			::kernelDebugLogMessage(CONSTLIT("EffectType %x GetParameters: Expected struct result."), GetUNID());

		CCCtx.Discard(pResult);
		}
	}

bool CEffectCreator::IsValidUNID (void)

//	IsValidUNID
//
//	Returns TRUE if UNID is valid. If not valid, the creator must be known
//	before loading.

	{
	return !strEquals(m_sUNID, STR_NO_UNID);
	}

CEffectCreator *CEffectCreator::FindEffectCreator (const CString &sUNID)

//	FindEffectCreator
//
//	Finds the effect creator from a complex UNID (or NULL if not found)

	{
	//	A complex UNID looks like this:
	//
	//	For effects:
	//
	//	{unid}
	//
	//	"12345"					= Effect UNID 12345
	//	"12345/0"				= Effect UNID 12345; variant 0
	//	"12345/d:h"				= Effect UNID 12345; damage; hit effect
	//
	//	For overlays:
	//
	//	{unid}:e
	//	{unid}:h
	//
	//	For shields:
	//
	//	{unid}:h
	//	"12345:h"				= Shield UNID 12345; Hit effect
	//
	//	For system maps:
	//
	//	{unid}/{nodeID}
	//	{unid}/{nodeID}[/{nodeID}]
	//
	//	For weapons:
	//
	//	{unid}/{var}[/f{frag}]:(e | h | f)
	//
	//	"12345/0:e"				= Weapon UNID 12345; variant 0; Bullet effect
	//	"12345/0/f0:h"			= Weapon UNID 12345; variant 0; fragment 0; Hit Effect
	//

	//	First we parse the UNID

	char *pPos = sUNID.GetASCIIZPointer();
	DWORD dwUNID = strParseInt(pPos, 0, &pPos);

	//	Look for the design type

	CDesignType *pDesign = g_pUniverse->FindDesignType(dwUNID);
	if (pDesign == NULL)
		return NULL;

	//	Allow the design type to parse the remainder of the UNID

	return pDesign->FindEffectCreatorInType(CString(pPos));
	}

ALERROR CEffectCreator::OnBindDesign (SDesignLoadCtx &Ctx)

//	OnBindDesign
//
//	Bind the design

	{
	try
		{
		ALERROR error;

		//	Load sounds

		if (error = m_Sound.Bind(Ctx))
			return error;

		//	Cache some events

		InitCachedEvents(evtCount, CACHED_EVENTS, m_CachedEvents);

		//	Load our descendants

		if (error = OnEffectBindDesign(Ctx))
			return error;

		return NOERROR;
		}
	catch (...)
		{
		::kernelDebugLogMessage("Crash in CEffectCreator::OnBindDesign. [m_sUNID = %s]", m_sUNID);
		throw;
		}
	}

ALERROR CEffectCreator::OnCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	OnCreateFromXML
//
//	Load from XML. This is only called if we go through the EffectType path
//	(as opposed to plain Effect types).

	{
	ALERROR error;

	//	Basic info

	m_sUNID = strFromInt(GetUNID(), false);

	if (error = InitBasicsFromXML(Ctx, pDesc))
		return error;

	//	Allow our subclass to initialize based on the effect
	//	(We know we have one because we couldn't have gotten this far
	//	without one. See CreateTypeFromXML.)

	CXMLElement *pEffect = pDesc->GetContentElementByTag(EFFECT_TAG);
	ASSERT(pEffect);

	//	Continue

	if (pEffect->GetContentElementCount() == 1)
		{
		CXMLElement *pEffectDesc = pEffect->GetContentElement(0);

		//	Load events for this effect, in case they're here.

		CXMLElement *pEventsDesc = pEffectDesc->GetContentElementByTag(EVENTS_TAG);
		if (pEventsDesc)
			{
			if (error = m_Events.InitFromXML(Ctx, pEventsDesc))
				return error;
			}

		//	Load the single effect

		error = OnEffectCreateFromXML(Ctx, pEffectDesc, m_sUNID);
		}
	else
		error = OnEffectCreateFromXML(Ctx, pEffect, m_sUNID);

	if (error)
		return error;

	//	Load damage descriptors

	CXMLElement *pDamageDesc = pDesc->GetContentElementByTag(DAMAGE_TAG);
	if (pDamageDesc)
		{
		m_pDamage = new CWeaponFireDesc;

		CString sUNID = strPatternSubst(CONSTLIT("%d/d"), GetUNID());
		if (error = m_pDamage->InitFromXML(Ctx, pDamageDesc, sUNID, true))
			return error;
		}
	
	return NOERROR;
	}

void CEffectCreator::OnEffectPlaySound (CSpaceObject *pSource)

//	OnEffectPlaySound
//
//	Play a sound

	{
	m_Sound.PlaySound(pSource);
	}

CEffectCreator *CEffectCreator::OnFindEffectCreator (const CString &sUNID)

//	OnFindEffectCreator
//
//	Finds the effect creator by UNID

	{
	char *pPos = sUNID.GetASCIIZPointer();

	//	If we're done, then we mean ourselves

	if (*pPos == '\0')
		return this;

	if (*pPos++ != '/')
		return NULL;

	//	Damage selector?

	if (*pPos == 'd')
		{
		pPos++;

		if (m_pDamage == NULL)
			return NULL;

		return m_pDamage->FindEffectCreator(CString(pPos));
		}

	//	Sub effect

	int iSubEffect = strParseInt(pPos, -1, 0, &pPos);
	if (iSubEffect == -1)
		return NULL;

	CEffectCreator *pSubEffect = GetSubEffect(iSubEffect);
	if (pSubEffect == NULL)
		return NULL;

	return pSubEffect->FindEffectCreatorInType(CString(pPos));
	}

bool CEffectCreator::OnFindEventHandler (const CString &sEvent, SEventHandlerDesc *retEvent) const

//	OnFindEventHandler
//
//	Looks for an event handler.

	{
	ICCItem *pCode;
	if (m_Events.FindEvent(sEvent, &pCode))
		{
		if (retEvent)
			{
			retEvent->pExtension = GetExtension();
			retEvent->pCode = pCode;
			}

		return true;
		}

	//	Not found

	return false;
	}

void CEffectCreator::PlaySound (CSpaceObject *pSource)

//	PlaySound
//
//	Play the sound effect

	{
	OnEffectPlaySound(pSource);
	}

void CEffectCreator::WritePainterToStream (IWriteStream *pStream, IEffectPainter *pPainter)

//	WritePainterToStream
//
//	Writes the painter to the stream (pPainter may be NULL)

	{
	if (pPainter)
		pPainter->WriteToStream(pStream);
	else
		{
		//	Write the UNID

		CString sNull;
		sNull.WriteToStream(pStream);

		//	Write the Class

		sNull.WriteToStream(pStream);
		}
	}

//	IEffectPainter object

void IEffectPainter::GetBounds (RECT *retRect)
	{
	//	CSpaceObject cannot handle asymmetric bounds, so we make sure
	//	that the box is centered.

	RECT rcRect;
	GetRect(&rcRect);

	retRect->left = -Max(-rcRect.left, rcRect.right);
	retRect->right = -retRect->left;
	retRect->top = -Max(-rcRect.top, rcRect.bottom);
	retRect->bottom = -retRect->top;
	}

void IEffectPainter::GetBounds (const CVector &vPos, CVector *retvUR, CVector *retvLL)
	{
	RECT rcRect;
	GetRect(&rcRect);

	*retvUR = vPos + CVector(rcRect.right * g_KlicksPerPixel, -rcRect.top * g_KlicksPerPixel);
	*retvLL = vPos + CVector(rcRect.left * g_KlicksPerPixel, -rcRect.bottom * g_KlicksPerPixel);
	}

int IEffectPainter::GetInitialLifetime (void)
	{
	return GetCreator()->GetLifetime();
	}

Metric IEffectPainter::GetRadius (void) const
	{
	RECT rcRect;
	GetRect(&rcRect);

	int cxWidth = RectWidth(rcRect);
	int cyHeight = RectHeight(rcRect);

	return g_KlicksPerPixel * Max(cxWidth / 2, cyHeight / 2);
	}

void IEffectPainter::GetRect (RECT *retRect) const
	{
	retRect->left = 0; 
	retRect->top = 0;
	retRect->bottom = 0;
	retRect->right = 0;
	}

void IEffectPainter::OnReadFromStream (SLoadCtx &Ctx)

//	OnReadFromStream
//
//	This is the default for reading painter parameters. We ask the painter for
//	a list of its parameters and read those.

	{
	int i;

	//	Read parameters

	if (Ctx.dwVersion >= 90)
		{
		//	Read the number of parameters

		DWORD dwCount;
		Ctx.pStream->Read((char *)&dwCount, sizeof(DWORD));

		for (i = 0; i < (int)dwCount; i++)
			{
			//	Read the parameter name

			CString sParam;
			sParam.ReadFromStream(Ctx.pStream);

			//	Read the value

			CEffectParamDesc Value;
			Value.ReadFromStream(Ctx);

			//	Set it

			SetParam(CCreatePainterCtx(), sParam, Value);
			}
		}
	}

void IEffectPainter::OnWriteToStream (IWriteStream *pStream)

//	OnWriteToStream
//
//	Writes list of parameters from the painter.

	{
	int i;

	TArray<CString> Params;
	if (!m_bSingleton && GetParamList(&Params))
		{
		//	Write the count of parameters

		DWORD dwSave = Params.GetCount();
		pStream->Write((char *)&dwSave, sizeof(DWORD));

		//	Write each parameter (name and value)

		for (i = 0; i < Params.GetCount(); i++)
			{
			//	Write the parameter value

			Params[i].WriteToStream(pStream);

			//	Write the value

			CEffectParamDesc Value;
			GetParam(Params[i], &Value);

			Value.WriteToStream(pStream);
			}
		}
	else
		{
		DWORD dwSave = 0;
		pStream->Write((char *)&dwSave, sizeof(DWORD));
		}
	}

CString IEffectPainter::ReadUNID (SLoadCtx &Ctx)

//	ReadUNID
//
//	Returns the UNID saved to a stream

	{
	CString sUNID;

	if (Ctx.dwVersion >= 15)
		sUNID.ReadFromStream(Ctx.pStream);
	else
		{
		DWORD dwUNID;
		Ctx.pStream->Read((char *)&dwUNID, sizeof(DWORD));
		sUNID = strFromInt(dwUNID, false);
		}

	return sUNID;
	}

ALERROR IEffectPainter::ValidateClass (SLoadCtx &Ctx, const CString &sOriginalClass)

//	ValidateClass
//
//	Reads the class string. If the class does not match the current painter,
//	we read the old data and return ERR_FAIL.

	{
	if (Ctx.dwVersion >= 40)
		{
		CString sClass;
		sClass.ReadFromStream(Ctx.pStream);

		//	If the original class doesn't match the current one, then it means
		//	that the design changed. In that case, we load the painter using the
		//	old class.

		if (!strEquals(sClass, sOriginalClass))
			{
			//	If sClass is blank, then it means that the original did not have
			//	an effect painter (but the current design does)

			if (!sClass.IsBlank())
				{
				//	Get the original creator

				CEffectCreator *pOriginalCreator;
				if (CEffectCreator::CreateFromTag(sClass, &pOriginalCreator) != NOERROR)
					{
					kernelDebugLogMessage("Unable to find original effect creator: %s", sClass);
					return ERR_FAIL;
					}

				//	Load the original painter

				IEffectPainter *pOriginalPainter = pOriginalCreator->CreatePainter(CCreatePainterCtx());
				pOriginalPainter->ReadFromStream(Ctx);

				//	Discard

				pOriginalPainter->Delete();
				delete pOriginalCreator;
				}

			//	Done

			return ERR_FAIL;
			}
		}

	return NOERROR;
	}

void IEffectPainter::WriteToStream (IWriteStream *pStream)
	{
	CString sUNID = GetCreator()->GetUNIDString();
	sUNID.WriteToStream(pStream);

	CString sClass = GetCreator()->GetTag();
	sClass.WriteToStream(pStream);

	OnWriteToStream(pStream);
	}

//	CNullEffectCreator ---------------------------------------------------------

