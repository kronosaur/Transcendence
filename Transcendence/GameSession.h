//	GameSession.h
//
//	Transcendence session classes
//	Copyright (c) 2016 by Kronosaur Productions, LLC. All Rights Reserved.

#pragma once

class CHeadsUpDisplay
    {
    public:
        CHeadsUpDisplay (CHumanInterface &HI, CTranscendenceModel &Model) :
                m_HI(HI),
                m_Model(Model),
                m_pArmorPainter(NULL),
                m_pShieldsPainter(NULL),
                m_iSelection(-1),
                m_pReactorPainter(NULL),
                m_pWeaponsPainter(NULL)
            { }

        inline ~CHeadsUpDisplay (void) { CleanUp(); }

        void CleanUp (void);
        void GetClearHorzRect (RECT *retrcRect) const;
        bool Init (const RECT &rcRect);
        void Invalidate (EHUDTypes iHUD = hudNone);
        void Paint (CG32bitImage &Screen, bool bInDockScreen = false);
        void SetArmorSelection (int iSelection);
        void Update (int iTick);

    private:
        CHumanInterface &m_HI;
        CTranscendenceModel &m_Model;
        RECT m_rcScreen;

        //  Armor/Shields Display

		IHUDPainter *m_pArmorPainter;
		IHUDPainter *m_pShieldsPainter;
        int m_iSelection;                   //  Selected armor seg (or -1)

        //  Reactor Display

        IHUDPainter *m_pReactorPainter;

        //  Weapons Display

        IHUDPainter *m_pWeaponsPainter;
    };

class CSystemMapDisplay
    {
    public:
        CSystemMapDisplay (CHumanInterface &HI, CTranscendenceModel &Model, CHeadsUpDisplay &HUD);

        bool HandleKeyDown (int iVirtKey, DWORD dwKeyData);
        bool HandleMouseWheel (int iDelta, int x, int y, DWORD dwFlags);
        bool Init (const RECT &rcRect);
        void OnHideMap (void);
        void OnShowMap (void);
        void Paint (CG32bitImage &Screen);

    private:
        enum EConstants
            {
            MAP_SCALE_COUNT = 4,
            };

        Metric GetScaleKlicksPerPixel (int iScale) const;

        CHumanInterface &m_HI;
        CTranscendenceModel &m_Model;
        CHeadsUpDisplay &m_HUD;
        RECT m_rcScreen;

        CMapScaleCounter m_Scale;           //  Track current map scale
        CMapLegendPainter m_HelpPainter;
    };

class CGameSession : public IHISession
	{
	public:
		CGameSession (CHumanInterface &HI, 
					  CGameSettings &Settings,
                      CTranscendenceModel &Model,
					  CSoundtrackManager &Soundtrack) : IHISession(HI),
				m_Settings(Settings),
                m_Model(Model),
				m_Soundtrack(Soundtrack),
                m_HUD(HI, Model),
                m_bShowingSystemMap(false),
                m_SystemMap(HI, Model, m_HUD),
        		m_iDamageFlash(0)
			{ }

		void ExecuteCommand (CPlayerShipController *pPlayer, CGameKeys::Keys iCommand);
		void ExecuteCommandEnd (CPlayerShipController *pPlayer, CGameKeys::Keys iCommand);
        void ShowSystemMap (bool bShow = true);

        //  Notifications from player ship, etc.

        inline void OnArmorDamaged (int iSegment) { m_HUD.Invalidate(hudArmor); }
        inline void OnArmorRepaired (int iSegment) { m_HUD.Invalidate(hudArmor); }
        inline void OnArmorSelected (int iSelection) { m_HUD.SetArmorSelection(iSelection); }
		inline void OnDamageFlash (void) { m_iDamageFlash = Min(2, m_iDamageFlash + 2); }
        inline void OnPlayerChangedShips (CSpaceObject *pOldShip) { m_HUD.Init(m_rcScreen); g_pTrans->InitDisplays(); }
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

        CGalacticMapSession::SOptions &GetGalacticMapSettings (void) { return m_GalacticMapSettings; }
		void PaintInfoText (CG32bitImage &Dest, const CString &sTitle, const TArray<CString> &Body, bool bAboveTargeting = true);
		void PaintSoundtrackTitles (CG32bitImage &Dest);
        inline bool ShowingSystemMap (void) const { return m_bShowingSystemMap; }

	private:
		void PaintSRS (CG32bitImage &Screen);

		CGameSettings &m_Settings;
        CTranscendenceModel &m_Model;
		CSoundtrackManager &m_Soundtrack;
		RECT m_rcScreen;					//	RECT of main screen within window.

        CHeadsUpDisplay m_HUD;              //  Paint the HUD
        bool m_bShowingSystemMap;           //  If TRUE, show map
        CSystemMapDisplay m_SystemMap;      //  Helps to paint the system map
        CGalacticMapSession::SOptions m_GalacticMapSettings;
		int m_iDamageFlash;					//	0 = no flash; odd = recover; even = flash;
	};
