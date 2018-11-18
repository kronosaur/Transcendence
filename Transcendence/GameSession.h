//	GameSession.h
//
//	Transcendence session classes
//	Copyright (c) 2016 by Kronosaur Productions, LLC. All Rights Reserved.

#pragma once

#include "GameSessionDisplays.h"

class CGameSession : public IHISession
	{
	public:
		enum EMenuTypes
			{
			menuNone,

			menuComms,						//	Choose message to send
			menuCommsSquadron,				//	Choose message for squadron
			menuCommsTarget,				//	Choose target for comms
			menuDebugConsole,				//	Debug console
			menuEnableDevice,				//	Choose device to enable/disable
			menuGame,						//	Game menu
			menuInvoke,						//	Invoke power
			menuSelfDestructConfirm,		//	Confirm self-destruct
			menuUseItem,					//	Choose item to use
			};

		CGameSession (STranscendenceSessionCtx &CreateCtx);

		void ExecuteCommand (CPlayerShipController *pPlayer, CGameKeys::Keys iCommand);
		void ExecuteCommandEnd (CPlayerShipController *pPlayer, CGameKeys::Keys iCommand);
        void ShowSystemMap (bool bShow = true);

        //  Notifications from player ship, etc.

        inline void OnArmorDamaged (int iSegment) { m_HUD.Invalidate(hudArmor); }
        inline void OnArmorRepaired (int iSegment) { m_HUD.Invalidate(hudArmor); }
        inline void OnArmorSelected (int iSelection) { m_HUD.SetArmorSelection(iSelection); }
		inline void OnDamageFlash (void) { m_iDamageFlash = Min(2, m_iDamageFlash + 2); }
		void OnObjDestroyed (const SDestroyCtx &Ctx);
        inline void OnPlayerChangedShips (CSpaceObject *pOldShip) { InitUI(); m_HUD.Init(m_rcScreen); g_pTrans->InitDisplays(); }
        void OnPlayerDestroyed (SDestroyCtx &Ctx, const CString &sEpitaph);
        void OnPlayerEnteredStargate (CTopologyNode *pNode);
        void OnShowDockScreen (bool bShow);
        inline void OnTargetChanged (CSpaceObject *pTarget) { m_HUD.Invalidate(hudTargeting); }
        inline void OnWeaponStatusChanged (void) { m_HUD.Invalidate(hudTargeting); }

		//	IHISession virtuals

		virtual CReanimator &GetReanimator (void) override { return g_pTrans->GetReanimator(); }
        virtual void OnAnimate (CG32bitImage &Screen, bool bTopMost) override;
        virtual void OnChar (char chChar, DWORD dwKeyData) override;
        virtual void OnCleanUp (void);
		virtual ALERROR OnCommand (const CString &sCmd, void *pData = NULL) override { return NOERROR; }
        virtual ALERROR OnInit (CString *retsError) override;
        virtual void OnKeyDown (int iVirtKey, DWORD dwKeyData) override;
        virtual void OnKeyUp (int iVirtKey, DWORD dwKeyData) override;
		virtual void OnLButtonDblClick (int x, int y, DWORD dwFlags) override;
		virtual void OnLButtonDown (int x, int y, DWORD dwFlags, bool *retbCapture) override;
		virtual void OnLButtonUp (int x, int y, DWORD dwFlags) override;
		virtual void OnMButtonDown (int x, int y, DWORD dwFlags, bool *retbCapture) override;
		virtual void OnMButtonUp (int x, int y, DWORD dwFlags) override;
		virtual void OnMouseMove (int x, int y, DWORD dwFlags) override;
        virtual void OnMouseWheel (int iDelta, int x, int y, DWORD dwFlags) override;
		virtual void OnMove (int x, int y) override { g_pTrans->WMMove(x, y); }
		virtual void OnRButtonDblClick (int x, int y, DWORD dwFlags) override;
		virtual void OnRButtonDown (int x, int y, DWORD dwFlags) override;
		virtual void OnRButtonUp (int x, int y, DWORD dwFlags) override;
		virtual void OnReportHardCrash (CString *retsMessage) override { *retsMessage = g_pTrans->GetCrashInfo(); }
		virtual void OnSize (int cxWidth, int cyHeight) override { g_pTrans->WMSize(cxWidth, cyHeight, 0); }

		//	Helpers

		CDockScreen &GetDockScreen (void) { return m_CurrentDock; }
        CGalacticMapSession::SOptions &GetGalacticMapSettings (void) { return m_GalacticMapSettings; }
		CGameSettings &GetGameSettings (void) { return m_Settings; }
		IPlayerController::EUIMode GetUIMode (void) const;
		inline bool InMenu (void) { return (m_CurrentMenu != menuNone); }
        inline bool InSystemMap (void) const { return m_bShowingSystemMap; }
		void PaintInfoText (CG32bitImage &Dest, const CString &sTitle, const TArray<CString> &Body, bool bAboveTargeting = true);
		void PaintSoundtrackTitles (CG32bitImage &Dest);

	private:
		void DismissMenu (void);
		void HideMenu (void);
		void InitUI (void);
        inline bool IsMouseAimEnabled (void) const { return m_bMouseAim; }
		void PaintSRS (CG32bitImage &Screen);
        void SetMouseAimEnabled (bool bEnabled = true);
		bool ShowMenu (EMenuTypes iMenu);
		void SyncMouseToPlayerShip (void);

		CGameSettings &m_Settings;
        CTranscendenceModel &m_Model;
		CCommandLineDisplay &m_DebugConsole;
		CSoundtrackManager &m_Soundtrack;
		RECT m_rcScreen;					//	RECT of main screen within window.

		//	UI state

		EUITypes m_iUI;						//	Basic flying UI
		bool m_bMouseAim;					//	Player aims with the mouse

		//	HUD state

        CHeadsUpDisplay m_HUD;              //  Paint the HUD
		EMenuTypes m_CurrentMenu;			//	Current menu being displayed
		CSpaceObject *m_pCurrentComms;		//	Object that we're currently communicating with
		int m_iDamageFlash;					//	0 = no flash; odd = recover; even = flash;
		bool m_bIgnoreButtonUp;				//	If we closed a menu on button down, ignore button up
		bool m_bIgnoreMouseMove;			//	Ignore mouse move, for purposes of enabling mouse flight

		//	Map state

        bool m_bShowingSystemMap;           //  If TRUE, show map
        CSystemMapDisplay m_SystemMap;      //  Helps to paint the system map
        CGalacticMapSession::SOptions m_GalacticMapSettings;

		//	Dock screen state

		CDockScreen m_CurrentDock;			//	Current dock screen
	};
