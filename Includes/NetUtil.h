//	NetUtil.h
//
//	Implements basic networking services
//
//	Requires
//
//		Alchemy
//
//	Usage:
//
//		1. Call NetBoot at the beginning of a process. Call
//			NetShutdown when done.
//
//		2. All CNetClient methods are asynchronous. Notifications
//			reveal result.
//
//		2. Notifications are called on a random thread. User
//			must do her own synchronization.
//
//	INetServerEvents
//
//		OnClientConnect (DWORD dwClientID) -> CString
//			This is called whenever a client connects to the server. The
//			server can return data before the client sends anything or it
//			can return an empty string if it wants to wait for the client
//			to send something.
//
//		OnClientError (DWORD dwClientID)
//			This client has experienced an error
//
//		OnDataReceived (DWORD dwClientID, CString sData, LPVOID pCtx) -> CString
//			The client has sent some data. pCtx is a user-defined LPVOID that
//			was setup for this client using SetClientData. The server may return
//			data to respond.
//
//		OnClientDisconnect (DWORD dwClientID, LPVOID pCtx)
//			Client has been disconnected. This is guaranteed to be called once
//			for every OnClientConnect call.

#ifndef INCL_NETUTIL
#define INCL_NETUTIL

#include <winsock.h>

class INetClientEvents;
class INetServerEvents;

//	Classes

enum NetErrors
	{
	GeneralError,
	HostNotFound,
	BadProtocol,
	PortNotFound,
	ConnectionLost,
	};

enum DataPackItemTypes
	{
	dpiInteger,
	dpiString,
	dpiStruct
	};

class CDataPackStruct
	{
	public:
		~CDataPackStruct (void);

		static ALERROR CreateFromBuffer (IReadBlock *pBuffer, CDataPackStruct **retpPack);
		ALERROR SerializeToBuffer (IWriteStream *pStream) const;
		CString SerializeToString (void) const;

		inline int GetCount (void) const { return m_Data.GetCount() / 2; }
		inline DataPackItemTypes GetItemType (int iIndex) const { return (DataPackItemTypes)m_Data.GetElement(iIndex * 2); }
		inline int GetIntegerItem (int iIndex) const { return GetItemData(iIndex); }
		CString GetStringItem (int iIndex) const;
		inline CDataPackStruct *GetStructItem (int iIndex) const { return (CDataPackStruct *)GetItemData(iIndex); }

		void AppendIntArray (const CIntArray &Array);
		void AppendInteger (int iData);
		void AppendString (const CString &sData);
		void AppendNewStruct (CDataPackStruct **retpStruct);

	private:
		inline int GetItemData (int iIndex) const { return m_Data.GetElement(iIndex * 2 + 1); }
		BYTE GetSerializedStructType (void) const;
		static ALERROR SerializeInteger (IWriteStream *pStream, int iValue);
		static ALERROR SerializeString (IWriteStream *pStream, const CString &sValue);
		static ALERROR SerializeStruct (IWriteStream *pStream, const CDataPackStruct * const pData);
		static ALERROR UnpackInteger (BYTE **pBuffer, BYTE *pEnd, int *retiValue);
		static ALERROR UnpackIntegerStruct (BYTE **pBuffer, BYTE *pEnd, CDataPackStruct *pStruct);
		static ALERROR UnpackString (BYTE **pBuffer, BYTE *pEnd, CString *retsValue);
		static ALERROR UnpackStruct (BYTE **pBuffer, BYTE *pEnd, CDataPackStruct *pStruct);

		CIntArray m_Data;
	};

class CNetClient
	{
	public:
		CNetClient (void);
		~CNetClient (void);

		ALERROR Connect (const CString &sHost, const CString &sService, const CString &sProtocol, INetClientEvents *pNotify);
		ALERROR SendData (const CString &sData);
		void Disconnect (void);

	private:
		enum NetState
			{
			stateDisconnected,
			stateHostLookup,
			stateConnecting,
			stateConnected,
			stateSending,
			stateReceiving,
			};

		enum WorkTypes
			{
			workSend,
			};

		struct WorkStruct
			{
			WorkTypes Type;
			CString sData;
			};

		static DWORD WINAPI ClientThreadThunk (LPVOID pThis);
		DWORD ClientThread (void);

		INetClientEvents *m_pNotify;		//	NULL if not connected

		//	Connection information
		CString m_sHost;
		CString m_sService;
		CString m_sProtocol;

		//	Thread stuff
		HANDLE m_hThread;					//	Thread handle
		HANDLE m_hQuit;						//	Quit event
		HANDLE m_hWriteDone;				//	Write done event
		HANDLE m_hReadDone;					//	Read done event

		//	State information
		NetState m_State;					//	Only modified by thread
		CSharedObjectQueue m_Queue;			//	Queue of work to do

		//	Socket information
		SOCKET m_Socket;
	};

class INetClientEvents
	{
	public:
		virtual void OnConnect (void) { }
		virtual void OnError (NetErrors Error, const CString &sErrorString) { }
		virtual void OnDataReady (const CString &sData) { }
		virtual void OnWriteComplete (void) { }
		virtual void OnDisconnect (void) { }
	};

class CNetServer
	{
	public:
		CNetServer (void);
		~CNetServer (void);

		ALERROR Boot (const CString &sService, const CString &sProtocol, INetServerEvents *pNotify);
		void DropClient (DWORD dwClientID);
		void SetClientData (DWORD dwClientID, LPVOID pData);
		void Shutdown (void);

	private:
		struct ClientStruct
			{
			CNetServer *pServer;			//	Back pointer to server
			DWORD dwID;						//	Client ID
			HANDLE hThread;					//	Thread handling client
			SOCKET Socket;					//	Client socket
			LPVOID pData;					//	Custom data

			HANDLE hQuit;					//	Client should quit
			};

		static DWORD WINAPI ClientThreadThunk (LPVOID pvClient);
		DWORD ClientThread (ClientStruct *pClient);
		static DWORD WINAPI ListenerThreadThunk (LPVOID pThis);
		DWORD ListenerThread (void);

		INetServerEvents *m_pNotify;		//	NULL if not booted

		//	Connection information
		CString m_sService;
		CString m_sProtocol;

		//	Thread stuff
		HANDLE m_hQuit;						//	Tells all threads to quit

		//	Listener thread
		HANDLE m_hListener;					//	Listener thread
		SOCKET m_ListenerSocket;

		//	Client array
		CRITICAL_SECTION m_csClients;
		CStructArray m_Clients;
	};

class INetServerEvents
	{
	public:
		virtual CString OnClientConnect (DWORD dwClientID) { return LITERAL(""); }
		virtual void OnClientError (DWORD dwClientID, NetErrors Error, const CString &sErrorString) { }
		virtual void OnError (NetErrors Error, const CString &sErrorString) { }
		virtual CString OnDataReceived (DWORD dwClientID, const CString &sData, LPVOID pCtx) { return LITERAL(""); }
		virtual void OnClientDisconnect (DWORD dwClientID, LPVOID pCtx) { }
	};

//	Functions

ALERROR netBoot (void);
void netShutdown (void);

#endif