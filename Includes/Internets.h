//	Internets.h
//
//	Basic classes for Internet protocols

#ifndef INCL_INTERNETS
#define INCL_INTERNETS

enum EInetsErrors
	{
	inetsOK =						0,	//	Success
	inetsDNSError,						//	Error resolving DNS address
	inetsInvalidProtocol,				//	Error finding protocol (e.g., "tcp" or "udp")
	inetsUnableToCreateSocket,			//	Failed creating a socket
	inetsCannotConnect,					//	Could not establish connection
	inetsUnableToWrite,					//	Failed writing
	inetsUnableToRead,					//	Failed reading
	inetsOutOfMemory,					//	Client is out of memory
	inetsInvalidMessage,				//	Could not stream a message
	};

class IMediaType
	{
	public:
		virtual ALERROR DecodeFromBuffer (const CString &sMediaType, const CString &sBuffer) = 0;
		virtual ALERROR EncodeToBuffer (IWriteStream *pOutput) const = 0;
		virtual const CString &GetMediaBuffer (void) const { return NULL_STR; }
		virtual DWORD GetMediaLength (void) const = 0;
		virtual CString GetMediaType (void) const = 0;
	};

class CHTTPMessage
	{
	public:
		CHTTPMessage (void);
		~CHTTPMessage (void);

		void AddHeader (const CString &sField, const CString &sValue);
		bool FindHeader (const CString &sField, CString *retsValue = NULL) const;
		inline const CString &GetBodyBuffer (void) const { if (m_pBody) return m_pBody->GetMediaBuffer(); else return NULL_STR; }
		inline DWORD GetStatusCode (void) const { return m_dwStatusCode; }
		inline const CString &GetStatusMsg (void) const { return m_sStatusMsg; }
		inline const CString &GetURL (void) const { return m_sURL; }
		ALERROR InitFromBuffer (const CString &sBuffer, bool bNoBody = false);
		ALERROR InitRequest (const CString &sMethod, const CString &sURL);
		ALERROR InitResponse (DWORD dwStatusCode, const CString &sStatusMsg);
		bool IsMessageComplete (void) const { return m_iState == stateDone; }
		inline void SetBody (IMediaType *pBody) { if (m_pBody) delete m_pBody; m_pBody = pBody; }
		ALERROR WriteToBuffer (IWriteStream *pOutput) const;

	private:
		enum States
			{
			stateStart,					//	Haven't parsed anything
			stateHeaders,				//	Parse the headers
			stateBody,					//	Parse the body
			stateChunk,					//	In the middle of a chunk
			stateDone,					//	Found end of message
			};

		enum MessageTypes
			{
			typeUnknown,
			typeRequest,
			typeResponse,
			};

		struct SHeader
			{
			CString sField;
			CString sValue;
			};

		void CleanUp (void);

		MessageTypes m_iType;
		CString m_sMethod;				//	If method is blank, then this is a response
		CString m_sURL;
		CString m_sVersion;
		DWORD m_dwStatusCode;
		CString m_sStatusMsg;
		TArray<SHeader> m_Headers;
		IMediaType *m_pBody;

		//	Parse state
		States m_iState;
		int m_iPos;
		CString m_sBody;				//	Temporary store while we parse
		int m_iChunkLeft;				//	Amount left to read
	};

class CHTTPClientSession
	{
	public:
		enum EStatuses
			{
			notConnected,				//	No connection to server
			inConnect,					//	We're inside a call to Connect
			dnsLookup,					//	We're in a DNS lookup call
			connecting,					//	Inside call to connect
			connected,					//	Connected to server and waiting for commands
			};

		CHTTPClientSession (void);
		~CHTTPClientSession (void);

		EInetsErrors Connect (const CString &sHost, const CString &sPort = NULL_STR);
		EInetsErrors Disconnect (void);
		inline const CString &GetHost (void) { return m_sHost; }
		inline DWORD GetTicksSinceLastActivity (void) { return (m_dwLastActivity == 0 ? 0xffffffff : (::GetTickCount() - m_dwLastActivity)); }
		bool IsConnected (void);
		EInetsErrors Send (const CHTTPMessage &Request, CHTTPMessage *retResponse);
		inline void SetStopEvent (HANDLE hEvent) { m_hStop = hEvent; }

	private:
		static bool IsResponseComplete (CMemoryWriteStream &Response);
		bool ReadBuffer (void *pBuffer, DWORD dwLen, DWORD *retdwRead = NULL);
		bool WaitForTransfer (OVERLAPPED &oOp, DWORD *retdwBytesTransfered = NULL);
		bool WriteBuffer (void *pBuffer, DWORD dwLen, DWORD *retdwWritten = NULL);

		CCriticalSection m_cs;
		EInetsErrors m_iLastError;

		CString m_sHost;
		bool m_bConnected;
		SOCKET m_Socket;
		HANDLE m_hReadDone;
		HANDLE m_hWriteDone;
		HANDLE m_hStop;

		EStatuses m_iStatus;
		DWORD m_dwLastActivity;			//	Tick on which we last used the connection
	};

//	Media Types ----------------------------------------------------------------

class CRawMediaType : public IMediaType
	{
	public:
		//	IMediaType
		virtual ALERROR DecodeFromBuffer (const CString &sMediaType, const CString &sBuffer) { m_sMediaType = sMediaType; m_sBody = sBuffer; return NOERROR; }
		virtual ALERROR EncodeToBuffer (IWriteStream *pOutput) const { return pOutput->Write(m_sBody.GetASCIIZPointer(), m_sBody.GetLength()); }
		virtual const CString &GetMediaBuffer (void) const { return m_sBody; }
		virtual DWORD GetMediaLength (void) const { return m_sBody.GetLength(); }
		virtual CString GetMediaType (void) const { return m_sMediaType; }

	private:
		CString m_sMediaType;
		CString m_sBody;
	};

class CHTMLForm : public IMediaType
	{
	public:
		void AddField (const CString &sKey, const CString &sValue);

		//	IMediaType
		virtual ALERROR DecodeFromBuffer (const CString &sMediaType, const CString &sBuffer);
		virtual ALERROR EncodeToBuffer (IWriteStream *pOutput) const;
		virtual DWORD GetMediaLength (void) const;
		virtual CString GetMediaType (void) const { return CONSTLIT("application/x-www-form-urlencoded"); }

	private:
		struct SField
			{
			CString sKey;
			CString sValue;
			};

		ALERROR EncodeText (IWriteStream *pOutput, const CString &sText) const;
		DWORD GetEncodedTextLength (const CString &sText) const;

		TArray<SField> m_Fields;
	};

//	Base64 Encoding ------------------------------------------------------------

class CBase64Decoder : public IReadStream
	{
	public:
		CBase64Decoder (IReadStream *pInput, DWORD dwFlags = 0);

		//	IReadStream
		virtual ALERROR Close (void) { return NOERROR; }
		virtual ALERROR Open (void) { return NOERROR; }
		virtual ALERROR Read (char *pData, int iLength, int *retiBytesRead = NULL);

	private:
		BYTE CharToByte (char chChar);

		IReadStream *m_pStream;

		int m_iBufferPos;
		BYTE m_chBuffer[3];
	};

class CBase64Encoder : public IWriteStream
	{
	public:
		CBase64Encoder (IWriteStream *pOutput, DWORD dwFlags = 0);
		~CBase64Encoder (void) { Close(); }

		//	IWriteStream
		virtual ALERROR Close (void);
		virtual ALERROR Create (void) { return NOERROR; }
		virtual ALERROR Write (char *pData, int iLength, int *retiBytesWritten = NULL);

	private:
		int WriteTriplet (void *pData);

		IWriteStream *m_pStream;

		int m_iBufferLen;
		BYTE m_chBuffer[3];
	};

//	Utilities ------------------------------------------------------------------

void strParseHostspec (const CString &sHostspec, CString *retsHost, CString *retsPort);
CString urlDecode (const CString &sURL, DWORD dwFlags = 0);
CString urlEncode (const CString &sText, DWORD dwFlags = 0);

//	Winsock.h doesn't define these constants so we need to do it here.
//	Winsock2.h DOES define them. If we ever switch to that, we won't need this.

#ifndef SD_BOTH
#define SD_BOTH 2
#endif

#endif