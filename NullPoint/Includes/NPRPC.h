//	NPRPC.h
//
//	Implements the basic client/server functionality
//
//	Adding a New Command:
//
//	1.	Add a new command method to CNPClient class
//	2.	Add entry to switch statement in CNPServer::OnDataReceived
//	3.	If custom reply is required, add virtual method to INPEvents
//
//	Protocol Examples:
//
//	The create universe protocol looks something like this:
//
//		Client							Server
//		------							------
//		Connect() ------------------->
//					<------------------	OnConnect()
//		CreateUniverse() ------------>
//					<------------------ OnCmdSuccess()
//		Disconnect() ---------------->
//					<------------------ OnDisconnect()
//
//	Joining a game looks like this:
//
//		Connect() ------------------->
//					<------------------	OnConnect()
//		NewPlayer() ----------------->
//					<------------------ OnCmdSuccess()
//		Disconnect() ---------------->
//					<------------------ OnDisconnect()
//
//	Playing looks like this:
//
//		Connect() ------------------->
//					<------------------	OnConnect()
//		Login() --------------------->
//					<------------------ OnLogin(SovereignUNID)
//		Disconnect() ---------------->
//					<------------------ OnDisconnect()
//

#ifndef INCL_NPCLIENT
#define INCL_NPCLIENT

#ifndef INCL_NETUTIL
#include "NetUtil.h"
#endif

#define SERVER_PORT							LITERAL("5673")
#define SERVER_PROTOCOL						LITERAL("tcp")
#define SERVER_MAJOR_VERSION				1
#define SERVER_MINOR_VERSION				0

#ifndef INCL_NPPROTOCOL
#include "NPProtocol.h"
#endif

#ifndef INCL_NPENGINE
#include "NPEngine.h"
#endif

enum NPClientStatus
	{
	npsDisconnected,					//	No connection
	npsWaitingForConnection,			//	Connect called; waiting for response
	npsWaitingForGameDesc,				//	Waiting to receive game description
	npsConnected,
	npsConnectedAsOverlord,
	npsCreatingUniverse,
	npsDownloadingUniverse,
	npsInGameAsOverlord,
	npsInGameAsPlayer,
	npsInGameAsSpectator
	};

enum NPClientError
	{
	npeSuccess,
	npeUnableToConnect,
	npeLostConnection
	};

struct CNPGameDesc
	{
	DWORD m_dwUNID;						//	Game UNID (0 = no game in progress)
	DWORD m_dwVersion;					//	HIWORD = major; LOWORD = minor
	CString m_sName;					//	Name of this game
	int m_iTurns;						//	Number of turns
	int m_iNullPoints;					//	Number of null points
	int m_iPlayers;						//	Number of players playing
	bool m_bOpen;						//	New players may join
	};

class CNPFleetView
	{
	public:
		inline DWORD GetUNID (void) { return m_dwUNID; }
		inline DWORD GetLocation (void) { return m_dwLocation; }
		inline DWORD GetSovereign (void) { return m_dwSovereign; }
		inline DWORD GetDest (void) { return m_dwDest; }
		inline int GetInTransit (void) { return m_iInTransit; }
		inline const CNPAssetList &GetAssetList (void) { return m_Assets; }

		ALERROR InitFromData (const CDataPackStruct &Data);

	private:
		DWORD m_dwUNID;
		DWORD m_dwSovereign;
		DWORD m_dwLocation;
		DWORD m_dwDest;
		int m_iInTransit;
		CNPAssetList m_Assets;
	};

class CNPNullPointView
	{
	public:
		inline DWORD GetUNID (void) { return m_dwUNID; }
		inline DWORD GetWorld (void) { return m_dwWorld; }
		inline int GetX (void) { return m_x; }
		inline int GetY (void) { return m_y; }
		inline int GetLinkCount (void) { return m_Links.GetCount(); }
		inline DWORD GetLinkDest (int iIndex) { return m_Links.GetElement(iIndex); }
		inline int GetFleetCount (void) { return m_Fleets.GetCount(); }
		inline DWORD GetFleet (int iIndex) { return (DWORD)m_Fleets.GetElement(iIndex); }

		ALERROR InitFromData (const CDataPackStruct &Data);

	private:
		DWORD m_dwUNID;
		int m_x, m_y;
		DWORD m_dwWorld;
		CIntArray m_Links;
		CIntArray m_Fleets;
	};

class CNPSovereignView
	{
	public:
		inline DWORD GetCapital (void) { return m_dwCapital; }
		inline const CString &GetName (void) { return m_sName; }
		inline DWORD GetUNID (void) { return m_dwUNID; }

		ALERROR InitFromData (const CDataPackStruct &Data);

	private:
		DWORD m_dwUNID;
		CString m_sName;
		DWORD m_dwCapital;
	};

class CNPWorldView
	{
	public:
		inline DWORD GetUNID (void) { return m_dwUNID; }
		inline DWORD GetLocation (void) { return m_dwLocation; }
		inline DWORD GetSovereign (void) { return m_dwSovereign; }
		inline int GetPopulation (void) { return m_iPopulation; }
		inline NPTechLevels GetTechLevel (void) { return m_iTechLevel; }
		inline int GetEfficiency (void) { return m_iEfficiency; }
		inline int GetTraitCount (void) { return m_Traits.GetCount(); }
		inline int GetTrait (int iIndex) { return m_Traits.GetElement(iIndex); }

		ALERROR InitFromData (const CDataPackStruct &Data);

	private:
		DWORD m_dwUNID;
		DWORD m_dwLocation;
		DWORD m_dwSovereign;
		int m_iPopulation;
		NPTechLevels m_iTechLevel;
		int m_iEfficiency;
		CIntArray m_Traits;
	};

class INPEvents
	{
	public:
		virtual void OnConnect (const CNPGameDesc &Desc) { }
		virtual void OnCmdFail (int iCmd, const CString &sError) { }
		virtual void OnCmdSuccess (int iCmd) { }
		virtual void OnDisconnect (void) { }
		virtual void OnError (const NPClientError iError, const CString &sError) { }
		virtual void OnLogin (DWORD dwSovereignUNID) { }
		virtual void OnViewFleet (CNPFleetView &Fleet) { }
		virtual void OnViewNullPoint (CNPNullPointView &NullPoint) { }
		virtual void OnViewSovereign (CNPSovereignView &Sovereign) { }
		virtual void OnViewWorld (CNPWorldView &World) { }
	};

class CNPClient : public INetClientEvents
	{
	public:
		CNPClient (void);
		~CNPClient (void);

		ALERROR Connect (const CString &sServer, INPEvents *pEvents);
		ALERROR Disconnect (void);

		//	Commands
		ALERROR CreateUniverse (CString sPassword);
		ALERROR FleetDest (DWORD dwFleet, DWORD dwDest);
		ALERROR Login (CString sName, CString sPassword);
		ALERROR NewPlayer (CString sEmperorName, CString sPassword, CString sEmpireName);
		ALERROR ViewObject (DWORD dwUNID);
		void UnviewObject (DWORD dwUNID);

		//	Status
		inline NPClientStatus GetClientStatus (void) { return m_iStatus; }
		inline const CString &GetEmpireName (void) { return m_sEmpireName; }
		inline const CString &GetEmperorName (void) { return m_sEmperorName; }
		inline int GetStardate (void) { return m_iStardate; }

		//	INetClientEvents virtuals
		virtual void OnConnect (void);
		virtual void OnError (NetErrors Error, const CString &sErrorString);
		virtual void OnWriteComplete (void);
		virtual void OnDataReady (const CString &sData);
		virtual void OnDisconnect (void);

	private:
		struct ViewedObject
			{
			int iRefCount;					//	RefCount of view requests
			void *pObj;						//	The actual object
			};

		void OnResConnect (const CDataPackStruct &Data);
		void OnResError (const CDataPackStruct &Data);
		void OnResFail (const CDataPackStruct &Data);
		void OnResLogin (const CDataPackStruct &Data);
		void OnResSuccess (const CDataPackStruct &Data);
		void OnResViewFleet (const CDataPackStruct &Data);
		void OnResViewNullPoint (const CDataPackStruct &Data);
		void OnResViewSovereign (const CDataPackStruct &Data);
		void OnResViewWorld (const CDataPackStruct &Data);

		void SendData (const CDataPackStruct &Data);

		inline void Lock (void) { ::EnterCriticalSection(&m_csClient); }
		inline void Unlock (void) { ::LeaveCriticalSection(&m_csClient); }

		CRITICAL_SECTION m_csClient;
		CNetClient m_Connection;
		INPEvents *m_pHandler;
		bool m_bConnected;					//	true if connected to server

		NPClientStatus m_iStatus;
		HANDLE m_hWritesComplete;			//	Set when all writes have completed
		int m_iPendingWrites;				//	Number of writes outstanding

		//	Universe state
		int m_iStardate;					//	Current year (in hundreths)
		CString m_sEmperorName;				//	Our name
		CString m_sEmpireName;				//	Name of the empire
		DWORD m_dwSovereignUNID;			//	Our empire

		CIDTable m_Objects;					//	ViewedObject indexed by UNID
	};

class INPServerEvents
	{
	public:
		virtual void OnError (const CString &sError) = 0;
	};

class CNPHumanPlayer : public CObject
	{
	public:
		CNPHumanPlayer (void);

		static ALERROR Create (CNPUniverse *pUniv, CString sName, CString sPassword, CString sEmpireName, CNPWorld *pCapital, CNPHumanPlayer **retpPlayer);

		inline const CString &GetName (void) { return m_sName; }
		inline const CString &GetPassword (void) { return m_sPassword; }
		inline DWORD GetSovereignID (void) { return m_dwSovereignUNID; }

	private:
		CString m_sName;
		CString m_sPassword;
		DWORD m_dwSovereignUNID;
	};

class CNPServer : public INetServerEvents
	{
	public:
		CNPServer (void);

		ALERROR Start (INPServerEvents *pHandler);
		void Run (void);
		void Stop (void);

		//	INetServerEvents
		virtual CString OnClientConnect (DWORD dwClientID);
		virtual void OnClientError (DWORD dwClientID, NetErrors Error, const CString &sErrorString);
		virtual void OnError (NetErrors Error, const CString &sErrorString);
		virtual CString OnDataReceived (DWORD dwClientID, const CString &sData, LPVOID pData);
		virtual void OnClientDisconnect (DWORD dwClientID, LPVOID pCtx);

	private:
		enum ServerStates
			{
			ssNotStarted,
			ssStart,
			ssUniverseCreated,
			ssGameInProgress,
			};

		enum AccessLevel
			{
			aclNoLogin		= 0,
			aclGuest		= 1,
			aclPlayer		= 2,
			aclOverlord		= 3,
			};

		struct ClientCtx
			{
			CString sName;					//	User name
			DWORD dwClientID;				//	ClientID
			int iRPCVersion;				//	RPC protocol version

			AccessLevel ACL;				//	Access level
			CNPSovereign *pSovereign;		//	Sovereign (may be NULL)
			};

		//	Commands
		CString CmdCreateUniverse (ClientCtx *pClient, const CDataPackStruct &Data);
		CString CmdFleetDest (ClientCtx *pClient, const CDataPackStruct &Data);
		CString CmdLogin (ClientCtx *pClient, const CDataPackStruct &Data);
		CString CmdNewPlayer (ClientCtx *pClient, const CDataPackStruct &Data);
		CString CmdRequestConnect (ClientCtx *pClient, const CDataPackStruct &Data);
		CString CmdView (ClientCtx *pClient, const CDataPackStruct &Data);

		CString ComposeCmdError (ClientCtx *pClient, int iCmd, ALERROR error);
		CString ComposeCmdFail (ClientCtx *pClient, int iCmd, CString sMessage);
		CString ComposeCmdSuccess (ClientCtx *pClient, int iCmd);
		inline void Lock (void) { EnterCriticalSection(&m_csServer); }
		void Log (char *pszLine, ...);
		inline void Unlock (void) { LeaveCriticalSection(&m_csServer); }
		bool ValidAccess (ClientCtx *pClient, DWORD dwUNID);
		bool ValidFleetAccess (ClientCtx *pClient, DWORD dwFleet, bool bOrders, CNPFleet **retpFleet, CString *retsError);
		bool ValidNullPointAccess (ClientCtx *pClient, DWORD dwNullPoint, bool bAdjacent, CNPNullPoint **retpNP, CString *retsError);

		CRITICAL_SECTION m_csServer;		//	Protects access to member variables
		CNetServer m_Server;				//	Network server code
		INPServerEvents *m_pHandler;		//	Callback
		ServerStates m_iState;				//	Current state
		HANDLE m_hQuitEvent;				//	quit event

		DWORD m_dwGameUNID;					//	UNID for this game (0 if no universe)
		CNPUniverse *m_pUniv;				//	universe
		CString m_sOverlordPassword;		//	Overlord password
		bool m_bOpenAccess;					//	TRUE if player's can join by themselves
		CSymbolTable m_Players;				//	CNPHumanPlayer (indexed by name)

		bool m_bLog;						//	TRUE if we're logging
	};

#endif