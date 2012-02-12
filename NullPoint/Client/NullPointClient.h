//	NullPointApp.h
//
//	Defines for null point client

#ifndef INCL_NULLPOINTCLIENT
#define INCL_NULLPOINTCLIENT

#ifndef INCL_NPRPC
#include "NPRPC.h"
#endif

class CMapView : public CFView
	{
	public:
		enum Events
			{
			ViewObject =	1000,	//	dwData1 = UNID of object to view
			UnviewObject =	1001,	//	dwData1 = UNID of object to unview
			};

		enum Messages
			{
			SetCenter =		1000,	//	dwData1 = UNID of map center
			SetNullPoint =	1001,	//	dwData1 = CNPNullPointView *
			};

		static ALERROR Create (CFView **retpView);

		virtual DWORD Message (int iMsg, DWORD dwData1, const CString &sData2);

	protected:
		virtual void OnCreate (CXMLElement *pDesc);
		virtual void OnPaint (HDC hDC, const RECT &rcUpdateRect);

	private:
		int m_xCenter;					//	Cordinates of map center
		int m_yCenter;
		CIDTable m_NullPoints;			//	(CNPNullPointView *) indexed by UNID
	};

class CNullPointApp : public INPEvents,
		public IFlatlandEvents,
		public IFController
	{
	public:
		CNullPointApp (void);

		//	INPEvents
		virtual void OnConnect (const CNPGameDesc &Desc);
		virtual void OnError (const NPClientError iError, const CString &sError);
		virtual void OnCmdFail (int iCmd, const CString &sError);
		virtual void OnCmdSuccess (int iCmd);
		virtual void OnLogin (DWORD dwSovereignUNID);
		virtual void OnViewFleet (CNPFleetView &Fleet);
		virtual void OnViewNullPoint (CNPNullPointView &NullPoint);
		virtual void OnViewSovereign (CNPSovereignView &Sovereign);
		virtual void OnViewWorld (CNPWorldView &World);

		//	IFlatlandEvents
		virtual ALERROR OnStartup (void);
		virtual void OnShutdown (void);

		//	IFController
		virtual void OnEvent (CFView *pView, int iID, int iEvent, DWORD dwData1, const CString &sData2);
		virtual void OnViewLoad (CFView *pView, int iID);

	private:
		enum States
			{
			Start,
			ShowFatalError,
			PasswordOK_WaitingForConnect,
			ConnectOK_WaitingForPassword,
			WaitingForLogin,
			PlayingGame,
			};

		bool CheckUniverseExistence (CNPGameDesc *pGameDesc);
		void ExitNP (void);
		void GetUserNameAndPassword (CFView *pLoginPane);
		void HandleMapPaneEvents(CFView *pView, int iEvent, DWORD dwData1, const CString &sData2);
		void Login (void);
		void ShowMessage (const CString &sString);

		CFWindow *m_pMainWindow;
		CNPClient m_NP;

		States m_iState;

		//	WelcomeScreen
		CString m_sUserName;
		CString m_sPassword;
	};

ALERROR RegisterCode (void);

#endif