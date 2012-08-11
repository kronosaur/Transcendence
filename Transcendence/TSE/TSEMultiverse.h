//	TSEMultiverse.h
//
//	Transcendence Multiverse Classes
//	Copyright 2012 by Kronosaur Productions, LLC. All Rights Reserved.

#ifndef INCL_TSE_MULTIVERSE
#define INCL_TSE_MULTIVERSE

class CMultiverseFileRef
	{
	public:
		inline const CString &GetFilePath (void) const { return m_sFilePath; }
		ALERROR InitFromJSON (const CJSONValue &Desc, CString *retsResult);
		inline bool IsEmpty (void) const { return m_sFilePath.IsBlank(); }

	private:
		CString m_sFilePath;				//	FilePath on service
		CTimeDate m_ModifiedTime;
		DWORD m_dwSize;

		CString m_sFilespec;				//	Filespec on local disk
	};

class CMultiverseCatalogEntry
	{
	public:
		CMultiverseCatalogEntry (const CMultiverseCatalogEntry &Src);
		static ALERROR CreateFromJSON (const CJSONValue &Entry, CMultiverseCatalogEntry **retpEntry, CString *retsResult);

		inline DWORD GetRelease (void) const { return m_dwRelease; }
		inline const CMultiverseFileRef &GetTDBFileRef (void) const { return m_TDBFile; }
		inline DWORD GetUNID (void) const { return m_dwUNID; }
		bool IsValid (void);

	private:
		CMultiverseCatalogEntry (void) { }

		CString m_sUNID;					//	Fully qualified UNID
		DWORD m_dwRelease;					//	Catalog entry release
		DWORD m_dwVersion;					//	Catalog entry version

		DWORD m_dwUNID;						//	UNID

		CMultiverseFileRef m_TDBFile;		//	Reference to TDB file.
	};

class CMultiverseCollection
	{
	public:
		~CMultiverseCollection (void) { DeleteAll(); }

		void DeleteAll (void);
		inline int GetCount (void) const { return m_List.GetCount(); }
		inline CMultiverseCatalogEntry *GetEntry (int iIndex) const { return m_List[iIndex]; }
		inline void Insert (CMultiverseCatalogEntry *pEntry) { m_List.Insert(pEntry); }

	private:
		TArray<CMultiverseCatalogEntry *> m_List;
	};

class CMultiverseModel
	{
	public:
		enum EOnlineStates
			{
			stateDisabled,					//	Multiverse disabled; can't sign in
			stateNoUser,					//	No user set
			stateOffline,					//	User set, but not signed in.
			stateOnline,					//	User signed in to Multiverse.
			};

		CMultiverseModel (void);

		ALERROR GetCollection (CMultiverseCollection *retCollection) const;
		EOnlineStates GetOnlineState (CString *retsUsername = NULL) const;
		inline const CString &GetServiceStatus (void) { return m_sLastStatus; }
		bool IsLoadCollectionNeeded (void) const;
		void OnCollectionLoading (void);
		void OnCollectionLoadFailed (void);
		void OnUserSignedIn (const CString &sUsername);
		void OnUserSignedOut (void);
		ALERROR SetCollection (const CJSONValue &Data, CString *retsResult);
		void SetDisabled (void);
		void SetServiceStatus (const CString &sStatus) { m_sLastStatus = sStatus; }
		void SetUsername (const CString &sUsername);

	private:
		void DeleteCollection (void);

		mutable CCriticalSection m_cs;		//	Protects access to all data
		CString m_sUsername;				//	User that we're representing
		CMultiverseCollection m_Collection;	//	User's collection of registered extensions

		CString m_sLastStatus;				//	Most recent service status

		DWORD m_fUserSignedIn:1;			//	TRUE if user is currently signed in.
		DWORD m_fCollectionLoaded:1;		//	TRUE if collection is loaded.
		DWORD m_fDisabled:1;				//	TRUE if we don't have multiverse support.
		DWORD m_fLoadingCollection:1;		//	TRUE if we're currently loading the collection.

		DWORD m_dwSpare:28;
	};

//	CHexarc --------------------------------------------------------------------

class CHexarc
	{
	public:
		static bool ConvertIPIntegerToString (const CJSONValue &Value, CString *retsValue);
		static bool ConvertToDigest (const CJSONValue &Value, CDigest *retDigest);
		static bool ConvertToIntegerIP (const CJSONValue &Value, CIntegerIP *retValue);
		static bool ConvertToJSON (const CIntegerIP &Value, CJSONValue *retValue);
		static bool CreateCredentials (const CString &sUsername, const CString &sPassword, CJSONValue *retValue);
		static bool CreateCredentials (const CString &sUsername, const CString &sPassword, CString *retsValue);
		static CString GetFilenameFromFilePath (const CString &sFilePath);
		static bool HasSpecialAeonChars (const CString &sValue);
		static bool IsBinary (const CJSONValue &Value, CString *retsData = NULL);
		static bool IsError (const CJSONValue &Value, CString *retsError = NULL, CString *retsDesc = NULL);
		static void WriteAsAeon (const CJSONValue &Value, IWriteStream &Stream);
		static bool ValidatePasswordComplexity (const CString &sPassword, CString *retsResult = NULL);
	};

class CHexarcSession
	{
	public:
		CHexarcSession (const CString &sHost = NULL_STR, const CString &sPort = NULL_STR, const CString &sRootURL = NULL_STR);

		CString GetClientVersion (void) const;
		inline const CString &GetRootURL (void) const { return m_sRootURL; }
		inline void SetClientID (const CString &sClientID) { m_sClientID = sClientID; }
		inline CString GetHostspec (void) const { return (m_sPort.IsBlank() ? m_sHost : strPatternSubst("%s:%s", m_sHost, m_sPort)); }
		void Housekeeping (void);
		ALERROR ServerCommand (const CString &sMethod, const CString &sFunc, CJSONValue &Payload, CJSONValue *retResult);
		inline void SetUsername (const CString &sUsername) { m_sUsername = sUsername; }
		void SetServer (const CString &sHost, const CString &sPort, const CString &sRootURL);

	private:
		bool Connect (CString *retsError);
		bool GetJSONResponse (CHTTPMessage &Response, CJSONValue *retValue, CString *retsError);
		void InitRequest (const CString &sMethod, const CString &sFunction, CHTTPMessage *retMessage);
		ALERROR ServerCommand (const CHTTPMessage &Request, CJSONValue *retResult);

		CString m_sHost;
		CString m_sPort;
		CString m_sRootURL;

		CString m_sClientID;
		CString m_sUsername;

		CHTTPClientSession m_Session;		//	Connection
	};

class CHexarcDownloader
	{
	public:
		struct SStatus
			{
			CString sFilespec;				//	Filespec to which we're downloading
			int iProgress;					//	0-100. If 100, then file is ready.
			};

		CHexarcDownloader (void) : m_Requests(1000), m_pCurrent(NULL), m_dwChunkSize(100000) { }
		~CHexarcDownloader (void);

		void AddRequest (const CString &sAPI,
						 const CString &sFilePath,
						 const CJSONValue &AuthToken,
						 const CString &sFilespec);
		void GetStatus (SStatus *retStatus);
		ALERROR Update (CHexarcSession &Session, SStatus *retStatus, CString *retsError);

	private:
		struct SRequest
			{
			CString sAPI;					//	API URL for download
			CString sFilePath;				//	FilePath of the file to download
			CJSONValue AuthToken;			//	AuthToken to use
			CString sFilespec;				//	Destination filespec

			DWORD dwTotalLen;				//	Initialized after first download
			DWORD dwDownload;				//	Total downloaded so far
			};

		void CleanUp (void);
		void UpdateCurrent (void);

		CCriticalSection m_cs;
		TQueue<SRequest *> m_Requests;
		SRequest *m_pCurrent;

		DWORD m_dwChunkSize;
	};

#endif
