//	IDockScreenDisplay.cpp
//
//	IDockScreenDisplay class

#include "PreComp.h"
#include "Transcendence.h"

#define DATA_FROM_PLAYER			CONSTLIT("player")
#define DATA_FROM_SOURCE			CONSTLIT("source")
#define DATA_FROM_STATION			CONSTLIT("station")

bool IDockScreenDisplay::EvalBool (const CString &sCode, bool *retbResult, CString *retsError)

//	EvalBool
//
//	Evaluates the given string

	{
	CCodeChainCtx Ctx;
	Ctx.SetScreen(m_pDockScreen);
	Ctx.SaveAndDefineSourceVar(m_pLocation);
	Ctx.SaveAndDefineDataVar(m_pData);

	char *pPos = sCode.GetPointer();
	ICCItem *pExp = Ctx.Link(sCode, 1, NULL);

	ICCItem *pResult = Ctx.Run(pExp);	//	LATER:Event
	Ctx.Discard(pExp);

	if (pResult->IsError())
		{
		*retsError = pResult->GetStringValue();
		Ctx.Discard(pResult);
		return false;
		}

	*retbResult = !pResult->IsNil();
	Ctx.Discard(pResult);

	return true;
	}

CSpaceObject *IDockScreenDisplay::EvalListSource (const CString &sString, CString *retsError)

//	EvalListSource
//
//	Returns the object from which we should display items

	{
	char *pPos = sString.GetPointer();

	//	See if we need to evaluate

	if (*pPos == '=')
		{
		CCodeChainCtx Ctx;
		Ctx.SetScreen(this);
		Ctx.SaveAndDefineSourceVar(m_pLocation);
		Ctx.SaveAndDefineDataVar(m_pData);

		ICCItem *pExp = Ctx.Link(sString, 1, NULL);

		ICCItem *pResult = Ctx.Run(pExp);	//	LATER:Event
		Ctx.Discard(pExp);

		if (pResult->IsError())
			{
			*retsError = pResult->GetStringValue();
			Ctx.Discard(pResult);
			return NULL;
			}

		//	Convert to an object pointer

		CSpaceObject *pSource;
		if (strEquals(pResult->GetStringValue(), DATA_FROM_PLAYER))
			pSource = m_pPlayer->GetShip();
		else if (strEquals(pResult->GetStringValue(), DATA_FROM_STATION)
				|| strEquals(pResult->GetStringValue(), DATA_FROM_SOURCE))
			pSource = m_pLocation;
		else
			pSource = Ctx.AsSpaceObject(pResult);

		Ctx.Discard(pResult);
		return pSource;
		}

	//	Otherwise, compare to constants

	else if (strEquals(sString, DATA_FROM_PLAYER))
		return m_pPlayer->GetShip();
	else
		return m_pLocation;
	}

bool IDockScreenDisplay::EvalString (const CString &sString, bool bPlain, ECodeChainEvents iEvent, CString *retsResult)

//	EvalString
//
//	Evaluates the given string.

	{
	CCodeChainCtx Ctx;
	Ctx.SetEvent(iEvent);
	Ctx.SetScreen(m_pDockScreen);
	Ctx.SaveAndDefineSourceVar(m_pLocation);
	Ctx.SaveAndDefineDataVar(m_pData);

	return Ctx.RunEvalString(sString, bPlain, retsResult);
	}

ALERROR IDockScreenDisplay::Init (SInitCtx &Ctx, CString *retsError)

//	Init
//
//	Initialize

	{
	m_pDockScreen = Ctx.pDockScreen;
	m_pPlayer = Ctx.pPlayer;
	m_pLocation = Ctx.pLocation;
	m_pData = Ctx.pData;

	return OnInit(Ctx, retsError); 
	}

void IDockScreenDisplay::OnShowPane (bool bNoListNavigation) 

//	OnShowPane
//
//	Pane has been shown. This is normally overridden by subclasses, but by
//	default we clear the armor selection.
	
	{
	g_pTrans->SelectArmor(-1); 
	}
