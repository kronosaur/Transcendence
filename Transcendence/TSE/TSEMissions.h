//	TSEMissions.h
//
//	CMission implementation
//	Copyright (c) 2012 by Kronosaur Productions, LLC. All Rights Reserved.

#ifndef INCL_TSE_MISSIONS
#define INCL_TSE_MISSIONS

class CMission : public CSpaceObject
	{
	public:
		enum EStatus
			{
			statusOpen,						//	Mission is open, ready for acceptance
			statusClosed,					//	Mission started without player
			statusAccepted,					//	Mission accepted by player
			statusPlayerSuccess,			//	Player completed mission
			statusPlayerFailure,			//	Player failed mission
			statusSuccess,					//	Mission succeeded without player
			statusFailure,					//	Mission failed without player
			};

		struct SCriteria
			{
			SCriteria (void) :
					bIncludeOpen(false),
					bIncludeUnavailable(false),
					bIncludeActive(false),
					bIncludeRecorded(false),
					bOnlySourceOwner(false),
					bOnlySourceDebriefer(false)
				{ }

			bool bIncludeOpen;					//	Include open missions
			bool bIncludeUnavailable;			//	Include unavailable missions
			bool bIncludeActive;				//	Include active missions
			bool bIncludeRecorded;				//	Include recorded missions

			bool bOnlySourceOwner;				//	Source must be owner
			bool bOnlySourceDebriefer;			//	Source must be debriefer

			TArray<CString> AttribsRequired;	//	Required attributes
			TArray<CString> AttribsNotAllowed;	//	Exclude objects with these attributes
			TArray<CString> SpecialRequired;	//	Special required attributes
			TArray<CString> SpecialNotAllowed;	//	Special excluding attributes
			};

		static ALERROR Create (CMissionType *pType,
							   CSpaceObject *pOwner,
							   ICCItem *pCreateData,
							   CMission **retpMission,
							   CString *retsError);
		void FireCustomEvent (const CString &sEvent, ICCItem *pData);
		inline DWORD GetAcceptedOn (void) const { return m_dwAcceptedOn; }
		inline bool IsActive (void) const { return (m_iStatus == statusAccepted || (!m_fDebriefed && (m_iStatus == statusPlayerSuccess || m_iStatus == statusPlayerFailure))); }
		inline bool IsClosed (void) const { return (!IsActive() && IsCompleted()); }
		inline bool IsCompleted (void) const { return (m_iStatus == statusPlayerSuccess || m_iStatus == statusPlayerFailure || m_iStatus == statusSuccess || m_iStatus == statusFailure); }
		inline bool IsCompletedNonPlayer (void) const { return (m_iStatus == statusSuccess || m_iStatus == statusFailure); }
		inline bool IsFailure (void) const { return (m_iStatus == statusFailure || m_iStatus == statusPlayerFailure); }
		inline bool IsOpen (void) const { return (m_iStatus == statusOpen); }
		inline bool IsPlayerMission (void) const { return m_fAcceptedByPlayer; }
		inline bool IsRecorded (void) const { return (m_fDebriefed && (m_iStatus == statusPlayerSuccess || m_iStatus == statusPlayerFailure)); }
		inline bool IsSuccess (void) const { return (m_iStatus == statusSuccess || m_iStatus == statusPlayerSuccess); }
		inline bool IsUnavailable (void) const { return (m_iStatus == statusClosed || m_iStatus == statusSuccess || m_iStatus == statusPlayerSuccess); }
		inline bool KeepsStats (void) const { return m_pType->KeepsStats(); }
		bool MatchesCriteria (CSpaceObject *pSource, const SCriteria &Criteria);
		void OnPlayerEnteredSystem (CSpaceObject *pPlayer);
		bool Reward (ICCItem *pData, ICCItem **retpResult = NULL);
		bool SetAccepted (void);
		bool SetDeclined (ICCItem **retpResult = NULL);
		bool SetFailure (ICCItem *pData);
		bool SetPlayerTarget (void);
		bool SetSuccess (ICCItem *pData);
		bool SetUnavailable (void);
		void UpdateExpiration (int iTick);

		static bool ParseCriteria (const CString &sCriteria, SCriteria *retCriteria);

		//	CSpaceObject virtuals

		virtual CMission *AsMission (void) { return this; }
		virtual CString GetName (DWORD *retdwFlags = NULL) { if (retdwFlags) *retdwFlags = 0; return m_pType->GetName(); }
		virtual ICCItem *GetProperty (const CString &sName);
		virtual CDesignType *GetType (void) const { return m_pType; }
		virtual bool HasAttribute (const CString &sAttribute) const { return m_pType->HasLiteralAttribute(sAttribute); }
		virtual bool HasSpecialAttribute (const CString &sAttrib) const;
		virtual bool IsMission (void) { return true; }
		virtual bool IsNonSystemObj (void) { return true; }
		virtual void OnNewSystem (CSystem *pSystem);
		virtual bool SetProperty (const CString &sName, ICCItem *pValue, CString *retsError);

	protected:
		//	CSpaceObject virtuals

		virtual CString GetObjClassName (void) { return CONSTLIT("CMission"); }
		virtual void OnDestroyed (SDestroyCtx &Ctx);
		virtual void OnObjDestroyedNotify (SDestroyCtx &Ctx);
		virtual void OnReadFromStream (SLoadCtx &Ctx);
		virtual void OnUpdate (SUpdateCtx &Ctx, Metric rSecondsPerTick);
		virtual void OnWriteToStream (IWriteStream *pStream);

	private:
		enum ECompletedReasons
			{
			completeSuccess,
			completeFailure,
			completeDestroyed,
			};

		CMission (void);

		void CloseMission (void);
		void CompleteMission (ECompletedReasons iReason);
		void FireOnAccepted (void);
		ICCItem *FireOnDeclined (void);
		ICCItem *FireOnReward (ICCItem *pData);
		void FireOnSetPlayerTarget (const CString &sReason);
		void FireOnStart (void);
		void FireOnStop (const CString &sReason, ICCItem *pData);

		CMissionType *m_pType;				//	Mission type
		EStatus m_iStatus;					//	Current mission status
		CGlobalSpaceObject m_pOwner;		//	Mission owner (may be NULL)
		CGlobalSpaceObject m_pDebriefer;	//	Object at which player debriefs (may be NULL,
											//		in which case we debrief at the owner obj).
		CString m_sNodeID;					//	NodeID of owner
		DWORD m_dwCreatedOn;				//	Mission created on (used to check expiration)
		DWORD m_dwAcceptedOn;				//	Tick on which mission was accepted
		DWORD m_dwLeftSystemOn;				//	Left the system on this tick (only used if the mission
											//		times out when out of the system).

		CString m_sTitle;					//	Mission title
		CString m_sInstructions;			//	Current instructions

		DWORD m_fIntroShown:1;				//	TRUE if player has seen intro
		DWORD m_fDeclined:1;				//	TRUE if player has declined at least once
		DWORD m_fDebriefed:1;				//	TRUE if player has been debriefed
		DWORD m_fInOnCreate:1;				//	TRUE if we're inside OnCreate
		DWORD m_fInMissionSystem:1;			//	TRUE if player is in the proper mission system
		DWORD m_fAcceptedByPlayer:1;		//	TRUE if this is a player mission
		DWORD m_fSpare7:1;
		DWORD m_fSpare8:1;
		DWORD m_dwSpare:24;

	friend CObjectClass<CMission>;
	};

class CMissionList
	{
	public:
		CMissionList (bool bFree = false) : m_bFree(bFree)
			{ }

		~CMissionList (void) { DeleteAll(); }

		void Delete (int iIndex);
		void Delete (CMission *pMission);
		void DeleteAll (void);
		inline int GetCount (void) const { return m_List.GetCount(); }
		inline CMission *GetMission (int iIndex) const { return m_List[iIndex]; }
		CMission *GetMissionByID (DWORD dwID) const;
		void Insert (CMission *pMission);
		ALERROR ReadFromStream (SLoadCtx &Ctx, CString *retsError);
		ALERROR WriteToStream (IWriteStream *pStream, CString *retsError);

	private:
		TArray<CMission *> m_List;
		bool m_bFree;						//	If TRUE, free missions when removed
	};

class CTimedMissionEvent : public CTimedEvent
	{
	public:
		CTimedMissionEvent (void) { }	//	Used only for loading
		CTimedMissionEvent (int iTick,
							int iInterval,
							CMission *pMission,
							const CString &sEvent);

		virtual CString DebugCrashInfo (void);
		virtual void DoEvent (DWORD dwTick, CSystem *pSystem);
		virtual CString GetEventHandlerName (void) { return m_sEvent; }
		virtual CSpaceObject *GetEventHandlerObj (void) { return m_pMission; }

	protected:
		virtual void OnReadFromStream (SLoadCtx &Ctx);
		virtual void OnWriteClassToStream (IWriteStream *pStream);
		virtual void OnWriteToStream (CSystem *pSystem, IWriteStream *pStream);

	private:
		int m_iInterval;			//	0 = not recurring
		CMission *m_pMission;
		CString m_sEvent;
	};

#endif
