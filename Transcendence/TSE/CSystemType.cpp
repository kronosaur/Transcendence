//	CSystemType.cpp
//
//	CSystemType class

#include "PreComp.h"

#define SYSTEM_GROUP_TAG						CONSTLIT("SystemGroup")
#define TABLES_TAG								CONSTLIT("Tables")

#define BACKGROUND_ID_ATTRIB					CONSTLIT("backgroundID")
#define NO_EXTRA_ENCOUNTERS_ATTRIB				CONSTLIT("noExtraEncounters")
#define NO_RANDOM_ENCOUNTERS_ATTRIB				CONSTLIT("noRandomEncounters")
#define SPACE_SCALE_ATTRIB						CONSTLIT("spaceScale")
#define TIME_SCALE_ATTRIB						CONSTLIT("timeScale")

#define ON_CREATE_EVENT							CONSTLIT("OnCreate")

static char *CACHED_EVENTS[CSystemType::evtCount] =
	{
		"OnObjJumpPosAdj",
	};

CSystemType::CSystemType (void) : 
		m_pDesc(NULL),
		m_pLocalTables(NULL)

//	CSystemType constructor

	{
	}

CSystemType::~CSystemType (void)

//	CSystemType destructor

	{
	if (m_pDesc)
		delete m_pDesc;

	if (m_pLocalTables)
		delete m_pLocalTables;
	}

ALERROR CSystemType::FireOnCreate (SSystemCreateCtx &SysCreateCtx, CString *retsError)

//	FireOnCreate
//
//	Fire OnCreate event

	{
	SEventHandlerDesc Event;
	if (FindEventHandler(ON_CREATE_EVENT, &Event))
		{
		CCodeChainCtx Ctx;
		Ctx.SetSystemCreateCtx(&SysCreateCtx);

		ICCItem *pResult = Ctx.Run(Event);
		if (pResult->IsError())
			{
			if (retsError)
				*retsError = strPatternSubst(CONSTLIT("System OnCreate: %s"), pResult->GetStringValue());
			return ERR_FAIL;
			}

		Ctx.Discard(pResult);
		}

	return NOERROR;
	}

bool CSystemType::FireOnObjJumpPosAdj (CSpaceObject *pObj, CVector *iovPos)

//	FireOnObjJumpPosAdj
//
//	Fires a system event to adjust the position of an object that jumped
//	Returns TRUE if the event adjusted the position

	{
	SEventHandlerDesc Event;
	if (FindEventHandlerSystemType(evtOnObjJumpPosAdj, &Event))
		{
		CCodeChainCtx Ctx;

		Ctx.SaveAndDefineSourceVar(pObj);
		Ctx.DefineVector(CONSTLIT("aJumpPos"), *iovPos);

		ICCItem *pResult = Ctx.Run(Event);

		if (pResult->IsError())
			{
			kernelDebugLogMessage("System OnObjJumpPosAdj: %s", pResult->GetStringValue());
			Ctx.Discard(pResult);
			return false;
			}
		else if (pResult->IsNil())
			{
			Ctx.Discard(pResult);
			return false;
			}
		else
			{
			CVector vNewPos = Ctx.AsVector(pResult);
			Ctx.Discard(pResult);

			if (vNewPos == *iovPos)
				return false;

			*iovPos = vNewPos;
			return true;
			}
		}

	return false;
	}

ALERROR CSystemType::FireSystemCreateCode (SSystemCreateCtx &SysCreateCtx, ICCItem *pCode, const COrbit &OrbitDesc, CString *retsError)

//	FireSystemCreateCode
//
//	Runs a bit of code at system create time.

	{
	CCodeChainCtx Ctx;
	Ctx.SetExtension(GetExtension());	//	This code always comes from the SystemType (never inherited).
	Ctx.SetSystemCreateCtx(&SysCreateCtx);
	Ctx.DefineOrbit(CONSTLIT("aOrbit"), OrbitDesc);

	ICCItem *pResult = Ctx.Run(pCode);
	if (pResult->IsError())
		{
		if (retsError)
			*retsError = strPatternSubst(CONSTLIT("System: %s"), pResult->GetStringValue());
		return ERR_FAIL;
		}

	Ctx.Discard(pResult);
	return NOERROR;
	}

ALERROR CSystemType::OnBindDesign (SDesignLoadCtx &Ctx)

//	OnBindDesign
//
//	Bind design

	{
	InitCachedEvents(evtCount, CACHED_EVENTS, m_CachedEvents);

	return NOERROR;
	}

ALERROR CSystemType::OnCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	OnCreateFromXML
//
//	Create from XML

	{
	ALERROR error;

	//	Load the background image UNID

	if (error = ::LoadUNID(Ctx, pDesc->GetAttribute(BACKGROUND_ID_ATTRIB), &m_dwBackgroundUNID))
		return error;

	//	Options

	m_bNoExtraEncounters = pDesc->GetAttributeBool(NO_EXTRA_ENCOUNTERS_ATTRIB);
	m_bNoRandomEncounters = pDesc->GetAttributeBool(NO_RANDOM_ENCOUNTERS_ATTRIB);

	if (!pDesc->FindAttributeDouble(SPACE_SCALE_ATTRIB, &m_rSpaceScale) || m_rSpaceScale <= 0.0)
		m_rSpaceScale = KLICKS_PER_PIXEL;

	if (!pDesc->FindAttributeDouble(TIME_SCALE_ATTRIB, &m_rTimeScale) || m_rTimeScale <= 0.0)
		m_rTimeScale = TIME_SCALE;

	//	We keep the XML around for system definitions.

	CXMLElement *pRoot = pDesc->GetContentElementByTag(SYSTEM_GROUP_TAG);
	if (pRoot == NULL)
		return ComposeLoadError(Ctx, CONSTLIT("Unable to find <SystemGroup> element."));

	m_pDesc = pRoot->OrphanCopy();

	//	We also need to keep the local tables

	CXMLElement *pLocalTables = pDesc->GetContentElementByTag(TABLES_TAG);
	if (pLocalTables)
		m_pLocalTables = pLocalTables->OrphanCopy();

	return NOERROR;
	}
