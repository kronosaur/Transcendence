//	TSEMultiverse.h
//
//	Transcendence Multiverse Classes
//	Copyright 2012 by Kronosaur Productions, LLC. All Rights Reserved.

#ifndef INCL_TSE_MULTIVERSE
#define INCL_TSE_MULTIVERSE

class CMultiverseFileRef
	{
	public:
		inline const CIntegerIP &GetDigest (void) const { return m_Digest; }
		inline const CString &GetFilePath (void) const { return m_sFilePath; }
		inline const CString &GetFilespec (void) const { return m_sFilespec; }
		inline const CString &GetOriginalFilename (void) const { return m_sOriginalFilename; }
		ALERROR InitFromJSON (const CJSONValue &Desc, CString *retsResult);
		inline bool IsEmpty (void) const { return m_sFilePath.IsBlank(); }
		inline void SetFilespec (const CString &sFilespec) { m_sFilespec = sFilespec; }

	private:
		CString m_sFilePath;				//	FilePath on service
		CString m_sOriginalFilename;		//	Original filename
		CTimeDate m_ModifiedTime;
		DWORD m_dwSize;
		CIntegerIP m_Digest;				//	File data digest (according to server)

		CString m_sFilespec;				//	Filespec on local disk
	};

class CMultiverseCatalogEntry
	{
	public:
		enum ELicenseTypes
			{
			licenseUnknown,
			licenseAuto,
			licenseCore,
			licenseFree,
			licensePaid,
			};

		enum ELocalStatus
			{
			statusUnknown,					//	Don't know whether we have it
			statusNotAvailable,				//	Not on this computer
			statusDownloadInProgress,		//	Currently downloading the extension
			statusLoaded,					//	Available and ready for use
			statusCorrupt,					//	Loaded, but digest did not match

			statusError,					//	Could not load for some reason
			};

		CMultiverseCatalogEntry (const CMultiverseCatalogEntry &Src);
		~CMultiverseCatalogEntry (void);
		static ALERROR CreateFromJSON (const CJSONValue &Entry, CMultiverseCatalogEntry **retpEntry, CString *retsResult);

		inline const CString &GetDesc (void) const { return m_sDesc; }
		inline CG16bitImage *GetIcon (void) const { return m_pIcon; }
		inline CG16bitImage *GetIconHandoff (void) { CG16bitImage *pIcon = m_pIcon; m_pIcon = NULL; return pIcon; }
		inline ELicenseTypes GetLicenseType (void) const { return m_iLicenseType; }
		inline const CString &GetName (void) const { return m_sName; }
		inline DWORD GetRelease (void) const { return m_dwRelease; }
		inline int GetResourceCount (void) const { return m_Resources.GetCount(); }
		inline const CMultiverseFileRef &GetResourceRef (int iIndex) const { return m_Resources[iIndex]; }
		inline ELocalStatus GetStatus (void) const { return m_iStatus; }
		inline const CString &GetStatusText (void) const { return m_sStatus; }
		inline const CMultiverseFileRef &GetTDBFileRef (void) const { return m_TDBFile; }
		inline EExtensionTypes GetType (void) const { return m_iType; }
		inline DWORD GetUNID (void) const { return m_dwUNID; }
		inline const CString &GetVersion (void) const { return m_sVersion; }
		bool IsValid (void);
		void SetIcon (CG16bitImage *pImage);
		inline void SetStatus (ELocalStatus iStatus, const CString &sStatus = NULL_STR) { m_iStatus = iStatus; m_sStatus = sStatus; }
		inline void SetVersion (const CString &sVersion) { m_sVersion = sVersion; }

	private:
		CMultiverseCatalogEntry (void) : 
				m_iLicenseType(licenseUnknown),
				m_iStatus(statusUnknown),
				m_pIcon(NULL)
			{ }

		CString m_sUNID;					//	Fully qualified UNID
		DWORD m_dwRelease;					//	Catalog entry release
		DWORD m_dwVersion;					//	Catalog entry version
		CString m_sVersion;					//	User-visible version
		EExtensionTypes m_iType;			//	Type of extension

		CString m_sName;					//	Name of extension
		CString m_sDesc;					//	Description
		CMultiverseFileRef m_TDBFile;		//	Reference to TDB file.
		ELicenseTypes m_iLicenseType;		//	Type of license

		TArray<CMultiverseFileRef> m_Resources;

		DWORD m_dwUNID;						//	UNID
		ELocalStatus m_iStatus;				//	Current status
		CString m_sStatus;					//	Status message
		CG16bitImage *m_pIcon;				//	200x100 image
	};

class CMultiverseCollection
	{
	public:
		~CMultiverseCollection (void) { DeleteAll(); }

		void DeleteAll (void);
		inline int GetCount (void) const { return m_List.GetCount(); }
		inline CMultiverseCatalogEntry *GetEntry (int iIndex) const { return m_List[iIndex]; }
		bool HasAllUNIDs (const TArray<DWORD> &UNIDList) const;
		bool HasAnyUNID (const TArray<DWORD> &UNIDList) const;
		bool HasUNID (DWORD dwUNID) const;
		inline void Insert (CMultiverseCatalogEntry *pEntry) { m_List.Insert(pEntry); }

	private:
		TArray<CMultiverseCatalogEntry *> m_List;
	};

class CMultiverseNewsEntry
	{
	public:
		CMultiverseNewsEntry (const CMultiverseNewsEntry &Src);
		~CMultiverseNewsEntry (void);

		static ALERROR CreateFromJSON (const CJSONValue &Entry, CMultiverseNewsEntry **retpEntry, CString *retsResult);

		void FindImages (const CString &sCacheFilespec, TSortMap<CString, CString> *retDownloads);
		inline const CString &GetBody (void) const { return m_sBody; }
		inline const CString &GetCallToActionText (void) const { return m_sCallToActionText; }
		inline const CString &GetCallToActionURL (void) const { return m_sCallToActionURL; }
		inline const CString &GetID (void) const { return m_sID; }
		inline const TArray<DWORD> &GetExcludedUNIDs (void) const { return m_ExcludedUNIDs; }
		inline const TArray<DWORD> &GetRequiredUNIDs (void) const { return m_RequiredUNIDs; }
		inline const CString &GetTitle (void) const { return m_sTitle; }
		inline bool IsShown (void) const { return m_bShown; }
		CG16bitImage *LoadImage (void);
		CG16bitImage *LoadImageHandoff (void);
		inline void SetShown (bool bShown = true) { m_bShown = bShown; }

	private:
		CMultiverseNewsEntry (void) :
				m_bShown(false)
			{ }

		void FindImage (const CString &sImageURL, 
						const CString &sImageFilename,
						const CString &sCacheFilespec,
						CString *retsImageFilespec,
						TSortMap<CString, CString> *retDownloads);

		CString m_sID;						//	Multiverse ID of this entry
		CString m_sTitle;					//	Title
		CString m_sBody;					//	Body (plain text)
		CString m_sCallToActionText;		//	Footer text
		CString m_sCallToActionURL;			//	URL to navigate to when clicked.
		CString m_sImageURL;
		CString m_sImageMaskURL;
		TArray<DWORD> m_RequiredUNIDs;		//	List of UNIDs that we must have to show entry
		TArray<DWORD> m_ExcludedUNIDs;		//	List of UNIDs that we CANNOT have to show entry

		//	We store the image here after loading

		CString m_sImageFilespec;			//	Local path to image (blank if download needed)
		CString m_sImageMaskFilespec;		//	Local path to mask (blank if download needed)
		CG16bitImage m_Image;

		//	News entry state

		bool m_bShown;						//	TRUE if we've shown the news this session
	};

class CMultiverseNews
	{
	public:
		~CMultiverseNews (void) { DeleteAll(); }

		void DeleteAll (void);
		inline int GetCount (void) const { return m_List.GetCount(); }
		inline CMultiverseNewsEntry *GetEntry (int iIndex) const { return m_List[iIndex]; }
		ALERROR Save (const CString &sCacheFilespec, CString *retsResult);
		ALERROR SetNews (const CJSONValue &Data, const CString &sCacheFilespec, TSortMap<CString, CString> *retDownloads, CString *retsResult);
		void ShowNews (CMultiverseNewsEntry *pEntry);

	private:
		struct SUserReadState
			{
			SUserReadState (void) :
					iReadCount(0)
				{ }

			CTimeDate LastRead;				//	Last time we showed the news
			int iReadCount;					//	Number of times we shown the news lately.
			};

		ALERROR LoadReadState (const CString &sFilespec, CString *retsError);
		inline void Insert (CMultiverseNewsEntry *pEntry) { m_List.Insert(pEntry); }
		ALERROR SaveReadState (const CString &sFilespec, CString *retsError);

		TArray<CMultiverseNewsEntry *> m_List;
		TSortMap<CString, SUserReadState> m_ReadState;
		bool m_bReadStateModified;
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
		ALERROR GetEntry (DWORD dwUNID, DWORD dwRelease, CMultiverseCollection *retCollection) const;
		CMultiverseNewsEntry *GetNextNewsEntry (void);
		EOnlineStates GetOnlineState (CString *retsUsername = NULL) const;
		bool GetResourceFileRefs (const TArray<CString> &Filespecs, TArray<CMultiverseFileRef> *retFileRefs) const;
		inline const CString &GetServiceStatus (void) { return m_sLastStatus; }
		const CString &GetUpgradeURL (void) const { return m_sUpgradeURL; }
		inline ULONG64 GetUpgradeVersion (void) const { return m_UpgradeVersion.dwProductVersion; }
		bool IsLoadCollectionNeeded (void) const;
		bool IsLoadNewsNeeded (void) const;
		void OnCollectionLoading (void);
		void OnCollectionLoadFailed (void);
		void OnUserSignedIn (const CString &sUsername);
		void OnUserSignedOut (void);
		ALERROR Save (const CString &sCacheFilespec, CString *retsResult = NULL) { return m_News.Save(sCacheFilespec, retsResult); }
		ALERROR SetCollection (const CJSONValue &Data, CString *retsResult);
		void SetDisabled (void);
		ALERROR SetNews (const CJSONValue &Data, const CString &sCacheFilespec, TSortMap<CString, CString> *retDownloads, CString *retsResult);
		void SetServiceStatus (const CString &sStatus) { m_sLastStatus = sStatus; }
		void SetUsername (const CString &sUsername);

	private:
		struct SResourceDesc
			{
			CString sFilename;				//	Local filename
			CString sFilePath;				//	Multiverse filePath

			const CMultiverseCatalogEntry *pEntry;
			int iIndex;
			};

		void AddResources (const CMultiverseCatalogEntry &Entry);
		void DeleteCollection (void);
		void SetUpgradeVersion (const CJSONValue &Entry);

		mutable CCriticalSection m_cs;		//	Protects access to all data
		CString m_sUsername;				//	User that we're representing
		CMultiverseCollection m_Collection;	//	User's collection of registered extensions
		CMultiverseNews m_News;				//	News from the Multiverse
		SFileVersionInfo m_UpgradeVersion;	//	This is the engine version available on the Multiverse
		CString m_sUpgradeURL;				//	The URL where we can find an upgrade, if necessary
		TSortMap<CString, SResourceDesc> m_Resources;	//	Resources, indexed by local filename

		CString m_sLastStatus;				//	Most recent service status

		DWORD m_fUserSignedIn:1;			//	TRUE if user is currently signed in.
		DWORD m_fCollectionLoaded:1;		//	TRUE if collection is loaded.
		DWORD m_fDisabled:1;				//	TRUE if we don't have multiverse support.
		DWORD m_fLoadingCollection:1;		//	TRUE if we're currently loading the collection.
		DWORD m_fNewsLoaded:1;				//	TRUE if news is loaded.

		DWORD m_dwSpare:27;
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
			SStatus (void) :
					iProgress(0)
				{ }

			SStatus (const SStatus &Src)
				{
				sFilespec = Src.sFilespec;
				iProgress = Src.iProgress;
				FileDigest = Src.FileDigest;
				}

			CString sFilespec;				//	Filespec to which we're downloading
			int iProgress;					//	0-100. If 100, then file is ready.
			CIntegerIP FileDigest;			//	Desired digest
			};

		CHexarcDownloader (void) : m_Requests(1000), m_pCurrent(NULL), m_dwChunkSize(100000) { }
		~CHexarcDownloader (void);

		void AddRequest (const CString &sAPI,
						 const CString &sFilePath,
						 const CJSONValue &AuthToken,
						 const CString &sFilespec,
						 const CIntegerIP &FileDigest);
		void GetStatus (SStatus *retStatus);
		ALERROR Update (CHexarcSession &Session, SStatus *retStatus, CString *retsError);

	private:
		struct SRequest
			{
			CString sAPI;					//	API URL for download
			CString sFilePath;				//	FilePath of the file to download
			CJSONValue AuthToken;			//	AuthToken to use
			CString sFilespec;				//	Destination filespec
			CIntegerIP FileDigest;			//	File digest

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
