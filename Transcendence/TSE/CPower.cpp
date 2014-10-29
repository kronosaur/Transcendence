//	CPower.cpp
//
//	CPower class

#include "PreComp.h"

#define INVOKE_TAG								CONSTLIT("Invoke")
#define ON_SHOW_TAG								CONSTLIT("OnShow")
#define ON_INVOKED_BY_PLAYER_TAG				CONSTLIT("OnInvokedByPlayer")
#define ON_DESTROY_CHECK_TAG					CONSTLIT("OnDestroyCheck")

#define UNID_ATTRIB								CONSTLIT("UNID")
#define COST_ATTRIB								CONSTLIT("cost")
#define KEY_ATTRIB								CONSTLIT("key")
#define NAME_ATTRIB								CONSTLIT("name")

#define STR_A_CAUSE								CONSTLIT("aCause")
#define STR_A_DESTROYER							CONSTLIT("aDestroyer")

CPower::CPower (void) : 
		m_pCode(NULL),
		m_pOnShow(NULL),
		m_pOnDestroyCheck(NULL),
		m_pOnInvokedByPlayer(NULL)

//	CPower constructor

	{
	}

CPower::~CPower (void)

//	CPower destructor

	{
	CCodeChain &CC = g_pUniverse->GetCC();

	if (m_pCode)
		m_pCode->Discard(&CC);

	if (m_pOnShow)
		m_pOnShow->Discard(&CC);

	if (m_pOnInvokedByPlayer)
		m_pOnInvokedByPlayer->Discard(&CC);

	if (m_pOnDestroyCheck)
		m_pOnDestroyCheck->Discard(&CC);
	}

void CPower::Invoke (CSpaceObject *pSource, CSpaceObject *pTarget, CString *retsError)

//	Invoke
//
//	Invoke the power

	{
	CCodeChainCtx Ctx;

	Ctx.SetExtension(GetExtension());
	Ctx.SaveAndDefineSourceVar(pSource);
	Ctx.DefineSpaceObject(CONSTLIT("gTarget"), pTarget);

	ICCItem *pResult = Ctx.Run(GetCode());
	if (retsError)
		{
		if (pResult->IsError())
			*retsError = pResult->GetStringValue();
		else
			*retsError = NULL_STR;
		}

	Ctx.Discard(pResult);
	}

void CPower::InvokeByPlayer (CSpaceObject *pSource, CSpaceObject *pTarget, CString *retsError)

//	Invoke
//
//	Invoke the power

	{
	CCodeChainCtx Ctx;

	if (retsError)
		*retsError = NULL_STR;

	Ctx.SetExtension(GetExtension());
	Ctx.SaveAndDefineSourceVar(pSource);
	Ctx.DefineSpaceObject(CONSTLIT("gTarget"), pTarget);

	//	First handle the player portion

	ICCItem *pResult;
	ICCItem *pCode = GetOnInvokedByPlayer();
	if (pCode)
		{
		pResult = Ctx.Run(pCode);

		bool bCancelInvocation = false;
		if (pResult->IsError())
			{
			if (retsError)
				*retsError = pResult->GetStringValue();

			bCancelInvocation = true;
			}

		//	If OnInvokedByPlayer returns Nil, then we do not invoke

		else if (pResult->IsNil())
			bCancelInvocation = true;

		//	Continue?

		Ctx.Discard(pResult);
		if (bCancelInvocation)
			return;
		}

	//	Invoke

	pCode = GetCode();
	if (pCode)
		{
		pResult = Ctx.Run(pCode);
		if (pResult->IsError())
			{
			if (retsError)
				*retsError = pResult->GetStringValue();
			}

		Ctx.Discard(pResult);
		}
	}

ALERROR CPower::OnCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	OnCreateFromXML
//
//	Create from XML

	{
	ALERROR error;
	int i;

	//	Load basic stuff

	m_sName = pDesc->GetAttribute(NAME_ATTRIB);
	m_iInvokeCost = pDesc->GetAttributeInteger(COST_ATTRIB);
	m_sInvokeKey = pDesc->GetAttribute(KEY_ATTRIB);

	//	Load various code blocks

	m_pCode = NULL;
	m_pOnShow = NULL;
	m_pOnInvokedByPlayer = NULL;
	m_pOnDestroyCheck = NULL;
	for (i = 0; i < pDesc->GetContentElementCount(); i++)
		{
		CXMLElement *pBlock = pDesc->GetContentElement(i);

		if (strEquals(pBlock->GetTag(), INVOKE_TAG))
			{
			if (error = LoadCodeBlock(pBlock->GetContentText(0), &m_pCode, &Ctx.sError))
				return error;
			}
		else if (strEquals(pBlock->GetTag(), ON_SHOW_TAG))
			{
			if (error = LoadCodeBlock(pBlock->GetContentText(0), &m_pOnShow, &Ctx.sError))
				return error;
			}
		else if (strEquals(pBlock->GetTag(), ON_INVOKED_BY_PLAYER_TAG))
			{
			if (error = LoadCodeBlock(pBlock->GetContentText(0), &m_pOnInvokedByPlayer, &Ctx.sError))
				return error;
			}
		else if (strEquals(pBlock->GetTag(), ON_DESTROY_CHECK_TAG))
			{
			if (error = LoadCodeBlock(pBlock->GetContentText(0), &m_pOnDestroyCheck, &Ctx.sError))
				return error;
			}
		else if (IsValidLoadXML(pBlock->GetTag()))
			;
		else
			{
			return ComposeLoadError(Ctx, strPatternSubst(CONSTLIT("Unknown element: <%s>"), pBlock->GetTag()));
			}
		}

	//	Done

	return NOERROR;
	}

bool CPower::OnDestroyCheck (CSpaceObject *pSource, DestructionTypes iCause, const CDamageSource &Attacker)

//	OnDestroyCheck
//
//	Returns TRUE if ship can be destroyed; otherwise, FALSE

	{
	if (m_pOnDestroyCheck)
		{
		CCodeChainCtx Ctx;

		//	Set up parameters

		Ctx.SetExtension(GetExtension());
		CSpaceObject *pAttacker = Attacker.GetObj();
		Ctx.SaveAndDefineSourceVar(pSource);
		Ctx.DefineSpaceObject(STR_A_DESTROYER, pAttacker);
		Ctx.DefineInteger(STR_A_CAUSE, iCause);

		//	Invoke

		ICCItem *pResult = Ctx.Run(m_pOnDestroyCheck);
		bool bResult = (pResult->IsNil() ? false : true);
		Ctx.Discard(pResult);

		return bResult;
		}
	else
		return true;
	}

bool CPower::OnShow (CSpaceObject *pSource, CSpaceObject *pTarget, CString *retsError)

//	OnShow
//
//	Returns TRUE if we should show this power on the menu

	{
	ICCItem *pCode = GetOnShow();
	if (pCode)
		{
		CCodeChainCtx Ctx;

		Ctx.SetExtension(GetExtension());
		Ctx.SaveAndDefineSourceVar(pSource);
		Ctx.DefineSpaceObject(CONSTLIT("gTarget"), pTarget);

		ICCItem *pResult = Ctx.Run(GetOnShow());
		if (retsError)
			{
			if (pResult->IsError())
				*retsError = pResult->GetStringValue();
			else
				*retsError = NULL_STR;
			}

		bool bResult = !pResult->IsNil();
		Ctx.Discard(pResult);

		return bResult;
		}
	else
		return true;
	}
