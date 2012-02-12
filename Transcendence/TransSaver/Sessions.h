//	Sessions.h
//
//	Sessions
//	Copyright (c) 2010 by George Moromisato. All Rights Reserved.

class CLoadingSession : public IHISession
	{
	public:
		CLoadingSession (CHumanInterface &HI, CTransSaverModel &Model);

		//	IHISession virtuals
		virtual void OnCleanUp (void);
		virtual ALERROR OnInit (CString *retsError);
		virtual void OnPaint (CG16bitImage &Screen, const RECT &rcInvalid);
		virtual void OnSize (int cxWidth, int cyHeight);
		virtual void OnUpdate (void);

	private:
		CTransSaverModel &m_Model;

		int m_iTick;

		CG16bitImage m_StargateImage;
		RECT m_rcScreen;
		RECT m_rcStargate;
	};

class CBattlesSession : public IHISession
	{
	public:
		CBattlesSession (CHumanInterface &HI, CTransSaverModel &Model) : IHISession(HI),
				m_Model(Model), m_pSystem(NULL), m_iLastChange(0) { }

		//	IHISession virtuals
		virtual void OnCleanUp (void);
		virtual ALERROR OnInit (CString *retsError);
		virtual void OnPaint (CG16bitImage &Screen, const RECT &rcInvalid);
		virtual void OnSize (int cxWidth, int cyHeight);
		virtual void OnUpdate (void);

	private:
		enum Flags
			{
			FLAG_FORCE_MULTIPLE_SHIPS = 0x00000001,
			};

		enum States
			{
			stateNormal,
			stateShowingLogo,
			};

		void CreateBattle (DWORD dwFlags = 0, DWORD dwNewShipClass = 0, DWORD dwSovereign = 0, CSpaceObject *pShipDestroyed = NULL);
		void CreateLogoAnimation (IAnimatron **retpAnimatron);
		void CreateRandomShips (DWORD dwClass, CSovereign *pSovereign, DWORD dwFlags, CShip **retpShip);
		void CreateShipDescAnimation (CShip *pShip, IAnimatron **retpAnimatron);
		void OnPOVSet (CSpaceObject *pObj);
		void ShowShipDescAnimation (CShip *pShip = NULL);

		CTransSaverModel &m_Model;

		States m_iState;
		CSystem *m_pSystem;

		int m_iLastChange;					//	Tick on which ships changed
		int m_iLastShipDesc;				//	Tick on which ship desc last showed up
	};
