//	GameSessionDisplays.h
//
//	Transcendence session classes
//	Copyright (c) 2017 by Kronosaur Productions, LLC. All Rights Reserved.

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

