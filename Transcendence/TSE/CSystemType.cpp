//	CSystemType.cpp
//
//	CSystemType class

#include "PreComp.h"

#define TABLES_TAG								CONSTLIT("Tables")

#define ON_CREATE_EVENT							CONSTLIT("OnCreate")
#define ON_OBJ_JUMP_POS_ADJ_EVENT				CONSTLIT("OnObjJumpPosAdj")

CSystemType::CSystemType (void) : 
		m_pDesc(NULL)

//	CSystemType constructor

	{
	}

CSystemType::~CSystemType (void)

//	CSystemType destructor

	{
	if (m_pDesc)
		delete m_pDesc;
	}

ALERROR CSystemType::FireOnCreate (SSystemCreateCtx &SysCreateCtx, CString *retsError)

//	FireOnCreate
//
//	Fire OnCreate event

	{
	ICCItem *pCode;
	if (FindEventHandler(ON_CREATE_EVENT, &pCode))
		{
		CCodeChainCtx Ctx;
		Ctx.SetSystemCreateCtx(&SysCreateCtx);

		ICCItem *pResult = Ctx.Run(pCode);
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
	if (m_pOnObjJumpPosAdjEvent)
		{
		CCodeChainCtx Ctx;

		Ctx.SaveAndDefineSourceVar(pObj);
		Ctx.DefineVector(CONSTLIT("aJumpPos"), *iovPos);

		ICCItem *pResult = Ctx.Run(m_pOnObjJumpPosAdjEvent);

		if (pResult->IsError())
			{
			kernelDebugLogMessage("System OnObjJumpPosAdj: %s", pResult->GetStringValue().GetASCIIZPointer());
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

ALERROR CSystemType::FireSystemCreateCode (SSystemCreateCtx &SysCreateCtx, ICCItem *pCode, CString *retsError)

//	FireSystemCreateCode
//
//	Runs a bit of code at system create time

	{
	CCodeChainCtx Ctx;
	Ctx.SetSystemCreateCtx(&SysCreateCtx);

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

CXMLElement *CSystemType::GetLocalSystemTables (void)

//	GetLocalSystemTables
//
//	Returns the local tables

	{
	return m_pDesc->GetContentElementByTag(TABLES_TAG);
	}

ALERROR CSystemType::OnBindDesign (SDesignLoadCtx &Ctx)

//	OnBindDesign
//
//	Bind design

	{
	m_pOnObjJumpPosAdjEvent = GetEventHandler(ON_OBJ_JUMP_POS_ADJ_EVENT);

	return NOERROR;
	}

ALERROR CSystemType::OnCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	OnCreateFromXML
//
//	Create from XML

	{
	m_pDesc = pDesc->OrphanCopy();
	return NOERROR;
	}
