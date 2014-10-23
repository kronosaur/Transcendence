//	CMission.cpp
//
//	CMission class
//	Copyright (c) 2012 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"

static CObjectClass<CMission>g_MissionClass(OBJID_CMISSION, NULL);

#define EVENT_ON_ACCEPTED						CONSTLIT("OnAccepted")
#define EVENT_ON_COMPLETED						CONSTLIT("OnCompleted")
#define EVENT_ON_DECLINED						CONSTLIT("OnDeclined")
#define EVENT_ON_REWARD							CONSTLIT("OnReward")
#define EVENT_ON_SET_PLAYER_TARGET				CONSTLIT("OnSetPlayerTarget")
#define EVENT_ON_STARTED						CONSTLIT("OnStarted")

#define PROPERTY_ACCEPTED_ON					CONSTLIT("acceptedOn")
#define PROPERTY_CAN_BE_DELETED					CONSTLIT("canBeDeleted")
#define PROPERTY_DEBRIEFER_ID					CONSTLIT("debrieferID")
#define PROPERTY_FORCE_UNDOCK_AFTER_DEBRIEF		CONSTLIT("forceUndockAfterDebrief")
#define PROPERTY_IS_ACTIVE						CONSTLIT("isActive")
#define PROPERTY_IS_COMPLETED					CONSTLIT("isCompleted")
#define PROPERTY_IS_DEBRIEFED					CONSTLIT("isDebriefed")
#define PROPERTY_IS_DECLINED					CONSTLIT("isDeclined")
#define PROPERTY_IS_FAILURE						CONSTLIT("isFailure")
#define PROPERTY_IS_INTRO_SHOWN					CONSTLIT("isIntroShown")
#define PROPERTY_IS_OPEN						CONSTLIT("isOpen")
#define PROPERTY_IS_RECORDED					CONSTLIT("isRecorded")
#define PROPERTY_IS_SUCCESS						CONSTLIT("isSuccess")
#define PROPERTY_IS_UNAVAILABLE					CONSTLIT("isUnavailable")
#define PROPERTY_NAME							CONSTLIT("name")
#define PROPERTY_NODE_ID						CONSTLIT("nodeID")
#define PROPERTY_OWNER_ID						CONSTLIT("ownerID")
#define PROPERTY_SUMMARY						CONSTLIT("summary")
#define PROPERTY_UNID							CONSTLIT("unid")

#define REASON_ACCEPTED							CONSTLIT("accepted")
#define REASON_DEBRIEFED						CONSTLIT("debriefed")
#define REASON_DESTROYED						CONSTLIT("destroyed")
#define REASON_FAILURE							CONSTLIT("failure")
#define REASON_IN_PROGRESS						CONSTLIT("inProgress")
#define REASON_NEW_SYSTEM						CONSTLIT("newSystem")
#define REASON_SUCCESS							CONSTLIT("success")

#define SPECIAL_OWNER_ID						CONSTLIT("ownerID:")

#define STATUS_OPEN								CONSTLIT("open")
#define STATUS_CLOSED							CONSTLIT("closed")
#define STATUS_ACCEPTED							CONSTLIT("accepted")
#define STATUS_PLAYER_SUCCESS					CONSTLIT("playerSuccess")
#define STATUS_PLAYER_FAILURE					CONSTLIT("playerFailure")
#define STATUS_SUCCESS							CONSTLIT("success")
#define STATUS_FAILURE							CONSTLIT("failure")

#define STR_A_REASON							CONSTLIT("aReason")

CMission::CMission (void) : CSpaceObject(&g_MissionClass)

//	CMission constructor

	{
	}

void CMission::CloseMission (void)

//	CloseMission
//
//	Closes a mission

	{
	//	Remove all subscriptions

	CSystem *pSystem = g_pUniverse->GetCurrentSystem();
	if (pSystem)
		RemoveAllEventSubscriptions(pSystem);

	//	Cancel all timer events

	g_pUniverse->CancelEvent(this);

	//	If this is a player mission then refresh another player mission

	if (m_fAcceptedByPlayer)
		g_pUniverse->RefreshCurrentMission();
	}

void CMission::CompleteMission (ECompletedReasons iReason)

//	CompleteMission
//
//	Complete the mission

	{
	if (IsCompleted())
		return;

	bool bIsPlayerMission = (m_iStatus == statusAccepted);

	//	Handle player missions differently

	if (bIsPlayerMission)
		{
		//	Mission failure

		if (iReason == completeFailure || iReason == completeDestroyed)
			{
			m_iStatus = statusPlayerFailure;

			//	Tell the player that we failed

			CSpaceObject *pPlayer = g_pUniverse->GetPlayer();
			if (pPlayer)
				{
				CString sMessage;
				if (!Translate(CONSTLIT("FailureMsg"), NULL, &sMessage))
					sMessage = CONSTLIT("Mission failed!");

				pPlayer->SendMessage(NULL, sMessage);
				}

			//	Set the player target (mission usually sets the target back to the 
			//	station that gave the mission).

			FireOnSetPlayerTarget(REASON_FAILURE);

			//	Let the player record the mission failure

			pPlayer->OnMissionCompleted(this, false);
			}

		//	Mission success

		else if (iReason == completeSuccess)
			{
			m_iStatus = statusPlayerSuccess;

			//	Tell the player that we succeeded

			CSpaceObject *pPlayer = g_pUniverse->GetPlayer();
			if (pPlayer)
				{
				CString sMessage;
				if (!Translate(CONSTLIT("SuccessMsg"), NULL, &sMessage))
					sMessage = CONSTLIT("Mission complete!");

				pPlayer->SendMessage(NULL, sMessage);
				}

			//	Set the player target (mission usually sets the target back to the 
			//	station that gave the mission).

			FireOnSetPlayerTarget(REASON_SUCCESS);

			//	Let the player record the mission success

			if (pPlayer)
				pPlayer->OnMissionCompleted(this, true);
			}

		//	If there is no debrief, then we close the mission

		if (!m_pType->HasDebrief())
			{
			m_fDebriefed = true;

			FireOnSetPlayerTarget(REASON_DEBRIEFED);
			CloseMission();
			}
		}

	//	Set status for non-player missions

	else
		{
		if (iReason == completeFailure || iReason == completeDestroyed)
			m_iStatus = statusFailure;
		else if (iReason == completeSuccess)
			m_iStatus = statusSuccess;

		//	For non-player missions we can close now. (For players we wait until
		//	debrief.)

		CloseMission();
		}
	}

ALERROR CMission::Create (CMissionType *pType,
						  CSpaceObject *pOwner,
						  ICCItem *pCreateData,
					      CMission **retpMission,
						  CString *retsError)

//	Create
//
//	Creates a new mission object. We return ERR_NOTFOUND if the mission could 
//	not be created because conditions do not allow it.

	{
	CMission *pMission;

	//	If we cannot encounter this mission, then we fail

	if (!pType->CanBeEncountered())
		return ERR_NOTFOUND;

	//	Create the new object

	pMission = new CMission;
	if (pMission == NULL)
		{
		*retsError = CONSTLIT("Out of memory");
		return ERR_MEMORY;
		}

	pMission->m_pType = pType;

	//	Initialize

	pMission->m_iStatus = statusOpen;
	pMission->m_fIntroShown = false;
	pMission->m_fDeclined = false;
	pMission->m_fDebriefed = false;
	pMission->m_fAcceptedByPlayer = false;
	pMission->m_pOwner = pOwner;
	pMission->m_pDebriefer = NULL;

	//	NodeID

	CTopologyNode *pNode = NULL;
	CSystem *pSystem = NULL;
	if ((pSystem = (pOwner ? pOwner->GetSystem() : g_pUniverse->GetCurrentSystem()))
			&& (pNode = pSystem->GetTopology()))
		pMission->m_sNodeID = pNode->GetID();

	pMission->m_dwCreatedOn = g_pUniverse->GetTicks();
	pMission->m_fInMissionSystem = true;
	pMission->m_dwAcceptedOn = 0;
	pMission->m_dwLeftSystemOn = 0;

	//	Set flags so we know which events we have

	pMission->SetEventFlags();

	//	Fire OnCreate

	pMission->m_fInOnCreate = true;

	CSpaceObject::SOnCreate OnCreate;
	OnCreate.pData = pCreateData;
	OnCreate.pOwnerObj = pOwner;
	pMission->FireOnCreate(OnCreate);

	pMission->m_fInOnCreate = false;

	//	If OnCreate destroyed the object then it means that the mission was not
	//	suitable. We return ERR_NOTFOUND

	if (pMission->IsDestroyed())
		{
		delete pMission;
		return ERR_NOTFOUND;
		}

	//	If we haven't subscribed to the owner, do it now

	if (pOwner && !pOwner->FindEventSubscriber(pMission))
		pOwner->AddEventSubscriber(pMission);

	//	Done

	if (retpMission)
		*retpMission = pMission;

	return NOERROR;
	}

void CMission::FireCustomEvent (const CString &sEvent, ICCItem *pData)

//	FireCustomEvent
//
//	Fires a custom timed event

	{
	SEventHandlerDesc Event;

	if (FindEventHandler(sEvent, &Event))
		{
		CCodeChainCtx Ctx;

		Ctx.SetEvent(eventDoEvent);
		Ctx.SaveAndDefineSourceVar(this);
		Ctx.SaveAndDefineDataVar(pData);

		ICCItem *pResult = Ctx.Run(Event);
		if (pResult->IsError())
			ReportEventError(sEvent, pResult);
		Ctx.Discard(pResult);
		}
	}

void CMission::FireOnAccepted (void)

//	FireOnAccepted
//
//	Fire <OnAccepted>

	{
	SEventHandlerDesc Event;

	if (FindEventHandler(EVENT_ON_ACCEPTED, &Event))
		{
		CCodeChainCtx Ctx;

		Ctx.SaveAndDefineSourceVar(this);

		ICCItem *pResult = Ctx.Run(Event);
		if (pResult->IsError())
			ReportEventError(EVENT_ON_ACCEPTED, pResult);
		Ctx.Discard(pResult);
		}
	}

ICCItem *CMission::FireOnDeclined (void)

//	FireOnDeclined
//
//	Fire <OnDeclined>. We return the result of the event, which might contain
//	instructions for the mission screen.
//
//	Callers are responsible for discarding the result, if not NULL.

	{
	SEventHandlerDesc Event;

	if (FindEventHandler(EVENT_ON_DECLINED, &Event))
		{
		CCodeChainCtx Ctx;

		Ctx.SaveAndDefineSourceVar(this);

		ICCItem *pResult = Ctx.Run(Event);
		if (pResult->IsError())
			{
			ReportEventError(EVENT_ON_DECLINED, pResult);
			Ctx.Discard(pResult);
			return NULL;
			}

		return pResult;
		}

	return NULL;
	}

ICCItem *CMission::FireOnReward (ICCItem *pData)

//	FireOnReward
//
//	Fire <OnReward>
//
//	Callers are responsible for discarding the result, if not NULL.

	{
	SEventHandlerDesc Event;

	if (FindEventHandler(EVENT_ON_REWARD, &Event))
		{
		CCodeChainCtx Ctx;

		Ctx.SaveAndDefineSourceVar(this);
		Ctx.SaveAndDefineDataVar(pData);

		ICCItem *pResult = Ctx.Run(Event);
		if (pResult->IsError())
			{
			ReportEventError(EVENT_ON_REWARD, pResult);
			Ctx.Discard(pResult);
			return NULL;
			}

		return pResult;
		}

	return NULL;
	}

void CMission::FireOnSetPlayerTarget (const CString &sReason)

//	FireOnSetPlayerTarget
//
//	Fire <OnSetPlayerTarget>

	{
	SEventHandlerDesc Event;

	if (FindEventHandler(EVENT_ON_SET_PLAYER_TARGET, &Event))
		{
		CCodeChainCtx Ctx;

		Ctx.SaveAndDefineSourceVar(this);
		Ctx.DefineString(STR_A_REASON, sReason);

		ICCItem *pResult = Ctx.Run(Event);
		if (pResult->IsError())
			ReportEventError(EVENT_ON_SET_PLAYER_TARGET, pResult);
		Ctx.Discard(pResult);
		}
	}

void CMission::FireOnStart (void)

//	FireOnStart
//
//	Fire <OnStarted>

	{
	SEventHandlerDesc Event;

	if (FindEventHandler(EVENT_ON_STARTED, &Event))
		{
		CCodeChainCtx Ctx;

		Ctx.SaveAndDefineSourceVar(this);

		ICCItem *pResult = Ctx.Run(Event);
		if (pResult->IsError())
			ReportEventError(EVENT_ON_STARTED, pResult);
		Ctx.Discard(pResult);
		}
	}

void CMission::FireOnStop (const CString &sReason, ICCItem *pData)

//	FireOnStop
//
//	Fire <OnCompleted>

	{
	SEventHandlerDesc Event;

	if (FindEventHandler(EVENT_ON_COMPLETED, &Event))
		{
		CCodeChainCtx Ctx;

		Ctx.SaveAndDefineSourceVar(this);
		Ctx.SaveAndDefineDataVar(pData);
		Ctx.DefineString(STR_A_REASON, sReason);

		ICCItem *pResult = Ctx.Run(Event);
		if (pResult->IsError())
			ReportEventError(EVENT_ON_COMPLETED, pResult);
		Ctx.Discard(pResult);
		}
	}

ICCItem *CMission::GetProperty (const CString &sName)

//	GetProperty
//
//	Returns a property

	{
	CCodeChain &CC = g_pUniverse->GetCC();

	if (strEquals(sName, PROPERTY_ACCEPTED_ON))
		return (m_fAcceptedByPlayer ? CC.CreateInteger(m_dwAcceptedOn) : CC.CreateNil());

	else if (strEquals(sName, PROPERTY_CAN_BE_DELETED))
		return CC.CreateBool(m_pType->CanBeDeleted());

	else if (strEquals(sName, PROPERTY_DEBRIEFER_ID))
		{
		if (m_pDebriefer.GetID() != OBJID_NULL)
			return CC.CreateInteger(m_pDebriefer.GetID());
		else if (m_pOwner.GetID() != OBJID_NULL)
			return CC.CreateInteger(m_pOwner.GetID());
		else
			return CC.CreateNil();
		}

	else if (strEquals(sName, PROPERTY_FORCE_UNDOCK_AFTER_DEBRIEF))
		return CC.CreateBool(m_pType->ForceUndockAfterDebrief());

	else if (strEquals(sName, PROPERTY_IS_ACTIVE))
		return CC.CreateBool(IsActive());

	else if (strEquals(sName, PROPERTY_IS_COMPLETED))
		return CC.CreateBool(IsCompleted());

	else if (strEquals(sName, PROPERTY_IS_DEBRIEFED))
		return CC.CreateBool(m_fDebriefed);

	else if (strEquals(sName, PROPERTY_IS_DECLINED))
		return CC.CreateBool(m_fDeclined);

	else if (strEquals(sName, PROPERTY_IS_FAILURE))
		return CC.CreateBool(IsFailure());

	else if (strEquals(sName, PROPERTY_IS_INTRO_SHOWN))
		return CC.CreateBool(m_fIntroShown);

	else if (strEquals(sName, PROPERTY_IS_OPEN))
		return CC.CreateBool(m_iStatus == statusOpen);

	else if (strEquals(sName, PROPERTY_IS_RECORDED))
		return CC.CreateBool(IsRecorded());

	else if (strEquals(sName, PROPERTY_IS_SUCCESS))
		return CC.CreateBool(IsSuccess());

	else if (strEquals(sName, PROPERTY_IS_UNAVAILABLE))
		return CC.CreateBool(IsUnavailable());

	else if (strEquals(sName, PROPERTY_NAME))
		return CC.CreateString(m_sTitle);

	else if (strEquals(sName, PROPERTY_NODE_ID))
		return (m_sNodeID.IsBlank() ? CC.CreateNil() : CC.CreateString(m_sNodeID));

	else if (strEquals(sName, PROPERTY_OWNER_ID))
		{
		if (m_pOwner.GetID() == OBJID_NULL)
			return CC.CreateNil();
		else
			return CC.CreateInteger(m_pOwner.GetID());
		}

	else if (strEquals(sName, PROPERTY_SUMMARY))
		return CC.CreateString(m_sInstructions);

	else if (strEquals(sName, PROPERTY_UNID))
		return CC.CreateInteger(m_pType->GetUNID());

	else
		return CSpaceObject::GetProperty(sName);
	}

bool CMission::HasSpecialAttribute (const CString &sAttrib) const

//	HasSpecialAttribute
//
//	Returns TRUE if object has the special attribute
//
//	NOTE: Subclasses may override this, but they must call the
//	base class if they do not handle the attribute.

	{
	if (strStartsWith(sAttrib, SPECIAL_OWNER_ID))
		{
		DWORD dwOwnerID = strToInt(strSubString(sAttrib, SPECIAL_OWNER_ID.GetLength()), 0);
		return (dwOwnerID == m_pOwner.GetID());
		}
	else
		return CSpaceObject::HasSpecialAttribute(sAttrib);
	}

bool CMission::MatchesCriteria (CSpaceObject *pSource, const SCriteria &Criteria)

//	MatchesCriteria
//
//	Returns TRUE if the given mission matches the criteria

	{
	int i;

	//	By status

	if (!(Criteria.bIncludeActive && IsActive())
			&& !(Criteria.bIncludeOpen && IsOpen())
			&& !(Criteria.bIncludeRecorded && IsRecorded())
			&& !(Criteria.bIncludeUnavailable && IsUnavailable()))
		return false;
			
	//	Owned by source

	if (Criteria.bOnlySourceOwner)
		{
		if (pSource)
			{
			if (pSource->GetID() != m_pOwner.GetID())
				return false;
			}
		else
			{
			if (m_pOwner.GetID() != OBJID_NULL)
				return false;
			}
		}

	if (Criteria.bOnlySourceDebriefer)
		{
		if (m_pDebriefer.GetID() != OBJID_NULL)
			{
			if (pSource == NULL || pSource->GetID() != m_pDebriefer.GetID())
				return false;
			}
		else
			{
			if (pSource)
				{
				if (pSource->GetID() != m_pOwner.GetID())
					return false;
				}
			else
				{
				if (m_pOwner.GetID() != OBJID_NULL)
					return false;
				}
			}
		}

	//	Check required attributes

	for (i = 0; i < Criteria.AttribsRequired.GetCount(); i++)
		if (!HasAttribute(Criteria.AttribsRequired[i]))
			return false;

	//	Check attributes not allowed

	for (i = 0; i < Criteria.AttribsNotAllowed.GetCount(); i++)
		if (HasAttribute(Criteria.AttribsNotAllowed[i]))
			return false;

	//	Check special attribs required

	for (i = 0; i < Criteria.SpecialRequired.GetCount(); i++)
		if (!HasSpecialAttribute(Criteria.SpecialRequired[i]))
			return false;

	//	Check special attribs not allowed

	for (i = 0; i < Criteria.SpecialNotAllowed.GetCount(); i++)
		if (HasSpecialAttribute(Criteria.SpecialNotAllowed[i]))
			return false;

	//	Match

	return true;
	}

void CMission::OnDestroyed (SDestroyCtx &Ctx)

//	OnDestroyed
//
//	Mission is destroyed

	{
	DEBUG_TRY

	if (m_fInOnCreate)
		return;

	//	If the mission is running then we need to stop

	if (m_iStatus == statusClosed || m_iStatus == statusAccepted)
		{
		FireOnStop(REASON_DESTROYED, NULL);

		CSpaceObject *pOwner = m_pOwner.GetObj();
		if (pOwner)
			pOwner->FireOnMissionCompleted(this, REASON_DESTROYED);
		}

	//	Make sure the mission is completed

	CompleteMission(completeDestroyed);

	//	Destroy the mission

	FireOnDestroy(Ctx);

	DEBUG_CATCH
	}

void CMission::OnNewSystem (CSystem *pSystem)

//	OnNewSystem
//
//	We are in a new system.
//
//	NOTE: pSystem can be NULL in some cases, particularly at the end of a game
//	(when we're resetting things).

	{
	//	Ignore any closed missions (completed missions)

	if (IsClosed())
		return;

	//	Resolve owner

	m_pOwner.Resolve();
	m_pDebriefer.Resolve();

	//	Clear any object references (because they might belong to a different
	//	system).

	ClearObjReferences();

	//	Keep track to see if we're leaving the mission system.

	CTopologyNode *pNode;
	if (!m_sNodeID.IsBlank()
			&& pSystem
			&& (pNode = pSystem->GetTopology()))
		{
		if (strEquals(m_sNodeID, pNode->GetID()))
			{
			//	Back in our system

			m_fInMissionSystem = true;
			m_dwLeftSystemOn = 0;
			}
		else
			{
			//	If mission fails when we leave the system, fail now

			if (m_pType->GetOutOfSystemTimeOut() == 0)
				SetFailure(NULL);

			//	If required, close the mission

			if (!m_fDebriefed 
					&& IsCompleted() 
					&& m_pType->CloseIfOutOfSystem())
				{
				m_fDebriefed = true;

				FireOnSetPlayerTarget(REASON_DEBRIEFED);
				CloseMission();
				return;
				}

			//	Left the system. If we used to be in our system, then keep 
			//	track of when we left.

			if (m_fInMissionSystem)
				m_dwLeftSystemOn = g_pUniverse->GetTicks();

			m_fInMissionSystem = false;
			}
		}
	}

void CMission::OnObjDestroyedNotify (SDestroyCtx &Ctx)

//	OnObjDestroyedNotify
//
//	An object that we subscribe to has been destroyed

	{
	//	Fire events

	FireOnObjDestroyed(Ctx);

	//	If this is the owner then the mission fails

	if (Ctx.pObj->GetID() == m_pOwner.GetID())
		{
		//	Mission fails

		if (m_pType->FailureWhenOwnerDestroyed())
			{
			SetFailure(NULL);

			//	Since the owner is dead, we do not require a debrief

			if (IsActive())
				{
				m_fDebriefed = true;

				FireOnSetPlayerTarget(REASON_DEBRIEFED);
				CloseMission();
				}
			}

		//	Clear out owner pointer (unless we left a wreck)

		if (Ctx.pWreck == NULL)
			m_pOwner.CleanUp();
		else if (Ctx.pWreck->GetID() != m_pOwner.GetID())
			m_pOwner = Ctx.pWreck;
		}

	if (Ctx.pObj->GetID() == m_pDebriefer.GetID())
		{
		//	If our debriefer has been destroyed, then remove it.
		//	(But only if it didn't leave a wreck).

		if (Ctx.pWreck == NULL)
			m_pDebriefer.CleanUp();
		else if (Ctx.pWreck->GetID() != m_pDebriefer.GetID())
			m_pDebriefer = Ctx.pWreck;
		}
	}

void CMission::OnPlayerEnteredSystem (CSpaceObject *pPlayer)

//	OnPlayerEnteredSystem
//
//	Player has entered the system

	{
	//	Fire event

	FireOnPlayerEnteredSystem(pPlayer);

	//	For active missions, fire event to reset player targets.

	FireOnSetPlayerTarget(REASON_NEW_SYSTEM);
	}

void CMission::OnReadFromStream (SLoadCtx &Ctx)

//	OnReadFromStream
//
//	Read from stream
//
//	DWORD		Mission type UNID
//	DWORD		Mission status
//	CGlobalSpaceObject	m_pOwner
//	CGlobalSpaceObject	m_pDebriefer
//	CString		m_sNodeID
//	DWORD		m_dwCreatedOn
//	DWORD		m_dwLeftSystemOn
//	DWORD		m_dwAcceptedOn
//	CString		m_sTitle
//	CString		m_sInstructions
//	DWORD		Flags

	{
	DWORD dwLoad;

	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	m_pType = CMissionType::AsType(g_pUniverse->FindDesignType(dwLoad));
	if (m_pType == NULL)
		throw CException(ERR_FAIL);

	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	m_iStatus = (EStatus)dwLoad;

	m_pOwner.ReadFromStream(Ctx);
	if (Ctx.dwVersion >= 89)
		m_pDebriefer.ReadFromStream(Ctx);

	m_sNodeID.ReadFromStream(Ctx.pStream);

	if (Ctx.dwVersion >= 85)
		Ctx.pStream->Read((char *)&m_dwCreatedOn, sizeof(DWORD));
	else
		m_dwCreatedOn = 0;

	if (Ctx.dwVersion >= 84)
		Ctx.pStream->Read((char *)&m_dwLeftSystemOn, sizeof(DWORD));
	else
		m_dwLeftSystemOn = 0;

	if (Ctx.dwVersion >= 86)
		Ctx.pStream->Read((char *)&m_dwAcceptedOn, sizeof(DWORD));
	else
		m_dwAcceptedOn = 0;

	if (Ctx.dwVersion >= 86)
		{
		m_sTitle.ReadFromStream(Ctx.pStream);
		m_sInstructions.ReadFromStream(Ctx.pStream);
		}

	//	Flags

	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	m_fIntroShown =			((dwLoad & 0x00000001) ? true : false);
	m_fDeclined	=			((dwLoad & 0x00000002) ? true : false);
	m_fDebriefed =			((dwLoad & 0x00000004) ? true : false);
	m_fInMissionSystem =	((dwLoad & 0x00000008) ? true : false);
	m_fAcceptedByPlayer =	((dwLoad & 0x00000010) ? true : false);
	m_fInOnCreate =		false;

	//	For backwards compatibility

	if (Ctx.dwVersion < 84)
		m_fInMissionSystem = true;

#ifdef DEBUG
	SetEventFlags();
#endif
	}

void CMission::OnUpdate (SUpdateCtx &Ctx, Metric rSecondsPerTick)

//	OnUpdate
//
//	Active missions update every tick.

	{
	ASSERT(IsActive());

	//	If we're out of the system then see if we've failed the mission.
	//	NOTE: It is OK to leave the system if we've completed the mission
	//	but not yet been debriefed.

	int iTimeout;
	if (!m_fInMissionSystem
			&& !IsCompleted()
			&& (iTimeout = m_pType->GetOutOfSystemTimeOut()) != -1)
		{
		if (m_dwLeftSystemOn + iTimeout < (DWORD)g_pUniverse->GetTicks())
			{
			SetFailure(NULL);

			//	If required, close the mission

			if (!m_fDebriefed && m_pType->CloseIfOutOfSystem())
				{
				m_fDebriefed = true;

				FireOnSetPlayerTarget(REASON_DEBRIEFED);
				CloseMission();
				}
			}
		}
	}

void CMission::OnWriteToStream (IWriteStream *pStream)

//	OnWriteToStream
//
//	Write to stream
//
//	DWORD		Mission type UNID
//	DWORD		Mission status
//	DWORD		Player status
//	CGlobalSpaceObject	m_pOwner
//	CGlobalSpaceObject	m_pDebriefer
//	CString		m_sNodeID
//	DWORD		m_dwCreatedOn
//	DWORD		m_dwLeftSystemOn
//	DWORD		m_dwAcceptedOn
//	CString		m_sTitle
//	CString		m_sInstructions
//	DWORD		Flags

	{
	DWORD dwSave;

	dwSave = m_pType->GetUNID();
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	dwSave = (DWORD)m_iStatus;
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	m_pOwner.WriteToStream(pStream);
	m_pDebriefer.WriteToStream(pStream);
	m_sNodeID.WriteToStream(pStream);
	pStream->Write((char *)&m_dwCreatedOn, sizeof(DWORD));
	pStream->Write((char *)&m_dwLeftSystemOn, sizeof(DWORD));
	pStream->Write((char *)&m_dwAcceptedOn, sizeof(DWORD));

	m_sTitle.WriteToStream(pStream);
	m_sInstructions.WriteToStream(pStream);

	//	Flags

	dwSave = 0;
	dwSave |= (m_fIntroShown ?			0x00000001 : 0);
	dwSave |= (m_fDeclined ?			0x00000002 : 0);
	dwSave |= (m_fDebriefed ?			0x00000004 : 0);
	dwSave |= (m_fInMissionSystem ?		0x00000008 : 0);
	dwSave |= (m_fAcceptedByPlayer ?	0x00000010 : 0);
	pStream->Write((char *)&dwSave, sizeof(DWORD));
	}

bool CMission::ParseCriteria (const CString &sCriteria, SCriteria *retCriteria)

//	ParseCriteria
//
//	Parses criteria. Returns TRUE if successful.

	{
	//	Initialize

	*retCriteria = SCriteria();

	//	Parse

	char *pPos = sCriteria.GetPointer();
	while (*pPos != '\0')
		{
		switch (*pPos)
			{
			case '*':
				retCriteria->bIncludeOpen = true;
				retCriteria->bIncludeUnavailable = true;
				retCriteria->bIncludeActive = true;
				retCriteria->bIncludeRecorded = true;
				break;

			case 'a':
				retCriteria->bIncludeActive = true;
				break;

			case 'o':
				retCriteria->bIncludeOpen = true;
				break;

			case 'r':
				retCriteria->bIncludeRecorded = true;
				break;

			case 'u':
				retCriteria->bIncludeUnavailable = true;
				break;

			case 'D':
				retCriteria->bOnlySourceDebriefer = true;
				break;

			case 'S':
				retCriteria->bOnlySourceOwner = true;
				break;

			case '+':
			case '-':
				{
				bool bRequired = (*pPos == '+');
				bool bBinaryParam;
				CString sParam = ParseCriteriaParam(&pPos, false, &bBinaryParam);

				if (bRequired)
					{
					if (bBinaryParam)
						retCriteria->SpecialRequired.Insert(sParam);
					else
						retCriteria->AttribsRequired.Insert(sParam);
					}
				else
					{
					if (bBinaryParam)
						retCriteria->SpecialNotAllowed.Insert(sParam);
					else
						retCriteria->AttribsNotAllowed.Insert(sParam);
					}
				break;
				}
			}

		pPos++;
		}

	//	Make sure we include some missions

	if (!retCriteria->bIncludeUnavailable
			&& !retCriteria->bIncludeActive
			&& !retCriteria->bIncludeRecorded
			&& !retCriteria->bIncludeOpen)
		{
		retCriteria->bIncludeUnavailable = true;
		retCriteria->bIncludeActive = true;
		retCriteria->bIncludeRecorded = true;
		retCriteria->bIncludeOpen = true;
		}

	return true;
	}

bool CMission::Reward (ICCItem *pData, ICCItem **retpResult)

//	Reward
//
//	Reward the player for a mission success

	{
	//	If we haven't yet called success, then do it now

	if (!IsCompleted())
		SetSuccess(NULL);

	//	Reward

	ICCItem *pResult = FireOnReward(pData);
	if (retpResult == NULL || (pResult && !pResult->IsSymbolTable()))
		{
		pResult->Discard(&g_pUniverse->GetCC());
		pResult = NULL;
		}

	//	Set debriefed to true as a convenience

	m_fDebriefed = true;
	FireOnSetPlayerTarget(REASON_DEBRIEFED);
	CloseMission();

	//	Done

	if (retpResult)
		*retpResult = pResult;

	return true;
	}

bool CMission::SetAccepted (void)

//	SetAccepted
//
//	Player accepts a mission

	{
	//	Must be available to player.

	if (m_iStatus != statusOpen)
		return false;

	//	Remember that we accepted

	m_fAcceptedByPlayer = true;
	m_dwAcceptedOn = g_pUniverse->GetTicks();
	m_pType->IncAccepted();

	//	Player accepts the mission

	FireOnAccepted();

	CSpaceObject *pOwner = m_pOwner.GetObj();
	if (pOwner)
		{
		//	Track mission accept stats

		if (KeepsStats())
			g_pUniverse->GetObjStatsActual(pOwner->GetID()).iPlayerMissionsGiven++;

		//	Let the mission given know

		pOwner->FireOnMissionAccepted(this);
		}

	//	If the above call changed anything, then we're done

	if (m_iStatus != statusOpen)
		return false;

	//	Player has accepted

	m_iStatus = statusAccepted;

	//	Start the mission

	FireOnStart();

	//	Set the player target

	FireOnSetPlayerTarget(REASON_ACCEPTED);

	//	Get the mission title and description (we remember these because we may
	//	need to access them outside of the system).

	if (!Translate(CONSTLIT("Name"), NULL, &m_sTitle))
		m_sTitle = m_pType->GetName();

	if (!Translate(CONSTLIT("Summary"), NULL, &m_sInstructions))
		m_sInstructions = NULL_STR;

	return true;
	}

bool CMission::SetDeclined (ICCItem **retpResult)

//	SetDeclined
//
//	Mission declined by player. Optionally returns the result of <OnDeclined>,
//	which the caller is responsible for discarding.

	{
	//	Must be available to player.

	if (m_iStatus != statusOpen)
		{
		if (retpResult)
			*retpResult = NULL;
		return false;
		}

	//	Player declines the mission

	ICCItem *pResult = FireOnDeclined();
	if (retpResult == NULL || (pResult && !pResult->IsSymbolTable()))
		{
		pResult->Discard(&g_pUniverse->GetCC());
		pResult = NULL;
		}

	//	Set flag

	m_fDeclined = true;

	//	Done

	if (retpResult)
		*retpResult = pResult;

	return true;
	}

bool CMission::SetFailure (ICCItem *pData)

//	SetFailure
//
//	Mission failed

	{
	//	Must be in the right state

	if (m_iStatus != statusAccepted && m_iStatus != statusClosed && m_iStatus != statusOpen)
		return false;

	//	Stop the mission

	if (m_iStatus != statusOpen)
		{
		FireOnStop(REASON_FAILURE, pData);

		CSpaceObject *pOwner = m_pOwner.GetObj();
		if (pOwner)
			pOwner->FireOnMissionCompleted(this, REASON_FAILURE);
		}

	//	Done

	CompleteMission(completeFailure);

	return true;
	}

bool CMission::SetSuccess (ICCItem *pData)

//	SetSuccess
//
//	Mission succeeded

	{
	//	Must be in the right state

	if (m_iStatus != statusAccepted && m_iStatus != statusClosed && m_iStatus != statusOpen)
		return false;

	//	Stop the mission

	if (m_iStatus != statusOpen)
		{
		FireOnStop(REASON_SUCCESS, pData);

		CSpaceObject *pOwner = m_pOwner.GetObj();
		if (pOwner)
			pOwner->FireOnMissionCompleted(this, REASON_SUCCESS);
		}

	//	Done

	CompleteMission(completeSuccess);

	return true;
	}

bool CMission::SetUnavailable (void)

//	SetUnavailable
//
//	Mission starting without player

	{
	//	Must be open

	if (m_iStatus == statusOpen)
		return false;

	//	No player

	m_iStatus = statusClosed;

	//	Start the mission

	FireOnStart();

	return true;
	}

bool CMission::SetPlayerTarget (void)

//	SetPlayerTarget
//
//	Calls <OnSetPlayerTarget>

	{
	if (!IsPlayerMission() || !IsActive())
		return false;

	//	Call OnSetPlayerTarget

	FireOnSetPlayerTarget(REASON_IN_PROGRESS);

	//	Done

	return true;
	}

bool CMission::SetProperty (const CString &sName, ICCItem *pValue, CString *retsError)

//	SetProperty
//
//	Sets an object property

	{
	if (strEquals(sName, PROPERTY_IS_DEBRIEFED))
		{
		if (m_iStatus == statusPlayerSuccess || m_iStatus == statusPlayerFailure)
			{
			if (!pValue->IsNil())
				{
				if (!m_fDebriefed)
					{
					m_fDebriefed = true;

					FireOnSetPlayerTarget(REASON_DEBRIEFED);
					CloseMission();
					}
				}
			else
				m_fDebriefed = false;
			}
		}

	else if (strEquals(sName, PROPERTY_DEBRIEFER_ID))
		{
		if (pValue->IsNil())
			m_pDebriefer.SetID(OBJID_NULL);
		else
			m_pDebriefer.SetID(pValue->GetIntegerValue());
		}

	else if (strEquals(sName, PROPERTY_IS_DECLINED))
		{
		if (m_iStatus == statusOpen)
			m_fDeclined = !pValue->IsNil();
		}

	else if (strEquals(sName, PROPERTY_IS_INTRO_SHOWN))
		{
		if (m_iStatus == statusOpen)
			m_fIntroShown = !pValue->IsNil();
		}

	else if (strEquals(sName, PROPERTY_NAME))
		{
		if (IsActive())
			m_sTitle = pValue->GetStringValue();
		}

	else if (strEquals(sName, PROPERTY_SUMMARY))
		{
		if (IsActive())
			m_sInstructions = pValue->GetStringValue();
		}

	else
		return CSpaceObject::SetProperty(sName, pValue, retsError);

	return true;
	}

void CMission::UpdateExpiration (int iTick)

//	UpdateExpiration
//
//	Check to see if open mission has expired

	{
	ASSERT(IsOpen());

	int iExpireTime = m_pType->GetExpireTime();
	if (iExpireTime != -1
			&& (DWORD)iTick > (m_dwCreatedOn + iExpireTime))
		{
		//	But only if the player is not docked with the mission object

		CSpaceObject *pPlayer = g_pUniverse->GetPlayer();
		if (pPlayer && !m_pOwner.IsEmpty() && m_pOwner->IsObjDocked(pPlayer))
			return;

		//	Destroy

		CDamageSource DamageSource(NULL, removedFromSystem);
		Destroy(DamageSource.GetCause(), DamageSource);
		}
	}
