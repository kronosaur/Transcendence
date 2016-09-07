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

const int CONTROL_HEIGHT =			420;

ALERROR CDockScreenSubjugate::OnInit (SInitCtx &Ctx, const SDisplayOptions &Options, CString *retsError)

//	OnInit
//
//	Initialize

	{
	//	Create the control

	m_pControl = new CGSubjugateArea(*Ctx.pVI);
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
