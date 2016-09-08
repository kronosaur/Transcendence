//	CDockScreenSubjugate.cpp
//
//	CDockScreenSubjugate class
//	Copyright (c) 2016 by Kronosaur Productions, LLC. All Rights Reserved.
//
//	USAGE
//
//	<Display type="subjugateMinigame">
//		<Data>
//			;	Expression should evaluate to a definition struct
//			;	(see below)
//		</Data>
//	</Display>
//
//	DEFINITION STRUCTURE
//
//	{
//		daimons: ...				; daimon definitions
//		countermeasures: ...		; countermeasure definitions
//		}

#include "PreComp.h"
#include "Transcendence.h"

#define ON_COMPLETED_EVENT			CONSTLIT("OnCompleted")

const int CONTROL_HEIGHT =			420;

CDockScreenSubjugate::CDockScreenSubjugate (void)

//	CDockScreenSubjugate constructor

	{
	}

void CDockScreenSubjugate::FireOnCompleted (bool bSuccess)

//	FireOnCompleted
//
//	Fire OnCompleted event

	{
	ICCItem *pCode;
	if (!m_Events.FindEvent(ON_COMPLETED_EVENT, &pCode))
		return;

	CCodeChainCtx Ctx;
	Ctx.SetScreen(m_pDockScreen);
	Ctx.SaveAndDefineSourceVar(m_pLocation);
	Ctx.SaveAndDefineDataVar(m_pData);

	ICCItem *pResult = Ctx.Run(pCode);	//	LATER:Event
	if (pResult->IsError())
		::kernelDebugLogMessage("<OnCompleted>: %s", pResult->GetStringValue());

	Ctx.Discard(pResult);
	}

void CDockScreenSubjugate::OnCompleted (bool bSuccess)

//	OnCompleted
//
//	Done with attempt.

	{
	FireOnCompleted(bSuccess);

	//	NOTE: After we fire this event, we might be destroyed, so do not try to
	//	access any member variables.
	}

ALERROR CDockScreenSubjugate::OnInit (SInitCtx &Ctx, const SDisplayOptions &Options, CString *retsError)

//	OnInit
//
//	Initialize

	{
	ALERROR error;

	//	Get the options

	if (Options.pOptions)
		{
		if (error = m_Events.AddEvent(Options.pOptions->GetContentElementByTag(ON_COMPLETED_EVENT), retsError))
			return error;
		}

	//	Create the control

	m_pControl = new CGSubjugateArea(*Ctx.pVI, *this);
	if (m_pControl == NULL)
		{
		*retsError = CONSTLIT("Out of memory.");
		return ERR_MEMORY;
		}

	//	Create. NOTE: Once we add it to the screen, it takes ownership of it. 
	//	We do not have to free it.

	m_dwID = Ctx.dwFirstID;
	RECT rcControl = Ctx.rcScreen;
	rcControl.bottom = rcControl.top + CONTROL_HEIGHT;
	Ctx.pScreen->AddArea(m_pControl, rcControl, m_dwID);

	//	Done

	return true;
	}
