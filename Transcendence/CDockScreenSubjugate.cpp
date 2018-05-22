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
//
//		<OnCompleted>
//			;	Event called when minigame is done
//		</OnCompleted>
//	</Display>
//
//	DEFINITION STRUCTURE
//
//	{
//		intelligence: ...			; artifact intelligence 1-24
//		willpower: ...				; artifact willpower 1-24
//		ego: ...					; artifact ego 1-24
//		countermeasures: ...		; countermeasure definitions
//
//		daimons: ...				; daimon definitions
//		}

#include "PreComp.h"
#include "Transcendence.h"

#define FIELD_COUNTERMEASURES		CONSTLIT("countermeasures")
#define FIELD_DAIMONS				CONSTLIT("daimons")
#define FIELD_EGO					CONSTLIT("ego")
#define FIELD_INTELLIGENCE			CONSTLIT("intelligence")
#define FIELD_WILLPOWER				CONSTLIT("willpower")

#define ON_COMPLETED_EVENT			CONSTLIT("OnCompleted")
#define ON_STARTED_EVENT			CONSTLIT("OnStarted")

#define PARAM_REASON				CONSTLIT("aReason")

#define PROPERTY_AWAKENING_STATUS	CONSTLIT("awakeningStatus")

#define REASON_FAILURE				CONSTLIT("failure")
#define REASON_SUCCESS				CONSTLIT("success")

const int MAX_STATISTIC =			24;
const int CONTROL_HEIGHT =			420;

CDockScreenSubjugate::CDockScreenSubjugate (CDockScreen &DockScreen) : IDockScreenDisplay(DockScreen)

//	CDockScreenSubjugate constructor

	{
	}

void CDockScreenSubjugate::FireOnCompleted (const CString &sReason)

//	FireOnCompleted
//
//	Fire OnCompleted event

	{
	ICCItem *pCode;
	if (!m_Events.FindEvent(ON_COMPLETED_EVENT, &pCode))
		return;

	CCodeChainCtx Ctx;
	Ctx.SetScreen(&m_DockScreen);
	Ctx.SaveAndDefineSourceVar(m_pLocation);
	Ctx.SaveAndDefineDataVar(m_pData);
	Ctx.DefineString(PARAM_REASON, sReason);

	ICCItem *pResult = Ctx.Run(pCode);	//	LATER:Event
	if (pResult->IsError())
		::kernelDebugLogPattern("<OnCompleted>: %s", pResult->GetStringValue());

	Ctx.Discard(pResult);
	}

void CDockScreenSubjugate::OnCompleted (bool bSuccess)

//	OnCompleted
//
//	Done with attempt.

	{
	FireOnCompleted((bSuccess ? REASON_SUCCESS : REASON_FAILURE));

	//	NOTE: After we fire this event, we might be destroyed, so do not try to
	//	access any member variables.
	}

ICCItemPtr CDockScreenSubjugate::OnGetProperty (const CString &sProperty) const

//	OnGetProperty
//
//	Returns the given property

	{
	CCodeChain &CC = g_pUniverse->GetCC();

	if (strEquals(sProperty, PROPERTY_AWAKENING_STATUS))
		{
		switch (m_Artifact.GetStatus())
			{
			case CArtifactAwakening::resultNone:
				return ICCItemPtr(CC.CreateString(CONSTLIT("notStarted")));

			case CArtifactAwakening::resultArtifactSubdued:
				return ICCItemPtr(CC.CreateString(CONSTLIT("subdued")));

			case CArtifactAwakening::resultPlayerFailed:
				return ICCItemPtr(CC.CreateString(CONSTLIT("failed")));

			case CArtifactAwakening::resultBattleContinues:
				return ICCItemPtr(CC.CreateString(CONSTLIT("inProgress")));

			default:
				return ICCItemPtr(CC.CreateString(CONSTLIT("unknown")));
			}
		}
	else
		return ICCItemPtr(CC.CreateNil());
	}

IDockScreenDisplay::EResults CDockScreenSubjugate::OnHandleKeyDown (int iVirtKey)

//	OnHandleKeyDown
//
//	Handle key down

	{
	switch (iVirtKey)
		{
		case VK_DOWN:
			m_pControl->Command(CGSubjugateArea::cmdSelectNextDaimon);
			return resultHandled;

		case VK_RETURN:
			m_pControl->Command(CGSubjugateArea::cmdDeployDaimon);
			return resultHandled;

		case VK_UP:
			m_pControl->Command(CGSubjugateArea::cmdSelectPrevDaimon);
			return resultHandled;

		default:
			return resultNone;
		}
	}

ALERROR CDockScreenSubjugate::OnInit (SInitCtx &Ctx, const SDisplayOptions &Options, CString *retsError)

//	OnInit
//
//	Initialize

	{
	int i;
	ALERROR error;

	//	Get the options

	if (Options.pOptions)
		{
		if (error = m_Events.AddEvent(Options.pOptions->GetContentElementByTag(ON_COMPLETED_EVENT), retsError))
			return error;

		if (error = m_Events.AddEvent(Options.pOptions->GetContentElementByTag(ON_STARTED_EVENT), retsError))
			return error;
		}

	//	Initialize with data

	if (m_pData)
		{
		CArtifactAwakening::SCreateDesc CreateDesc;
		ICCItem *pValue;

		pValue = m_pData->GetElement(FIELD_EGO);
		CreateDesc.Stat[CArtifactStat::statEgo] = (pValue ? Max(1, Min(pValue->GetIntegerValue(), MAX_STATISTIC)) : 1);

		pValue = m_pData->GetElement(FIELD_INTELLIGENCE);
		CreateDesc.Stat[CArtifactStat::statIntelligence] = (pValue ? Max(1, Min(pValue->GetIntegerValue(), MAX_STATISTIC)) : 1);

		pValue = m_pData->GetElement(FIELD_WILLPOWER);
		CreateDesc.Stat[CArtifactStat::statWillpower] = (pValue ? Max(1, Min(pValue->GetIntegerValue(), MAX_STATISTIC)) : 1);

		//	Add all countermeasures

		pValue = m_pData->GetElement(FIELD_COUNTERMEASURES);
		if (pValue)
			{
			for (i = 0; i < pValue->GetCount(); i++)
				{
				ICCItem *pUNID = pValue->GetElement(i);
				CItemType *pItem = g_pUniverse->FindItemType(pUNID->GetIntegerValue());
				if (pItem == NULL)
					{
					::kernelDebugLogPattern("Artifact Awaken: Unable to find item: %08x", pUNID->GetIntegerValue());
					continue;
					}

				CreateDesc.Countermeasures.Insert(pItem);
				}
			}

		//	Add all daimons

		pValue = m_pData->GetElement(FIELD_DAIMONS);
		if (pValue)
			{
			for (i = 0; i < pValue->GetCount(); i++)
				{
				ICCItem *pUNID = pValue->GetElement(i);
				CItemType *pItem = g_pUniverse->FindItemType(pUNID->GetIntegerValue());
				if (pItem == NULL)
					{
					::kernelDebugLogPattern("Artifact Awaken: Unable to find item: %08x", pUNID->GetIntegerValue());
					continue;
					}

				CreateDesc.Daimons.Insert(pItem);
				}
			}

		//	Initialize

		if (!m_Artifact.Init(CreateDesc, retsError))
			return ERR_MEMORY;
		}

	//	Create the control

	m_pControl = new CGSubjugateArea(*Ctx.pVI, *this, m_Artifact);
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

	return NOERROR;
	}
