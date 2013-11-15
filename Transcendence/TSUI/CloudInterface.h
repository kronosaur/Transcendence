//	CloudInterface.h
//
//	Classes and methods for accessing Transcendence cloud services
//	Copyright (c) 2010 by George Moromisato. All Rights Reserved.

#pragma once

#ifndef INCL_INTERNETS
#include "Internets.h"
#endif

#ifndef INCL_CRYPTO
#include "Crypto.h"
#endif

#ifndef INCL_JSONUTIL
#include "JSONUtil.h"
#endif

struct SUserInfo
	{
	CString sUsername;
	CString sService;
	};

class ICIService
	{
	public:
		enum Capabilities
			{
			autoLoginUser =				0x00000001,	//	Implies that we have cached username and credentials
			cachedUser =				0x00000002,	//	We know the username, but may not have credentials
			canGetUserProfile =			0x00000004,	//	Implies that we are signed in (we have an authToken)
			loginUser =					0x00000008,	//	We have the ability to sign in (always TRUE for Hexarc)
			registerUser =				0x00000010, //	We have the ability to register (always TRUE for Hexarc)
			canPostGameRecord =			0x00000020,	//	We can post a game record
			userProfile =				0x00000040,	//	We can show user records (TRUE even if not signed in)
			modExchange =				0x00000080,	//	We can show mod exchange (TRUE even if not signed in)
			canLoadUserCollection =		0x00000100,	//	We can retrieve the user's collection
			canDownloadExtension =		0x00000200,	//	We can download an extension from the multiverse
			canPostCrashReport =		0x00000400,	//	We can post a crash report
			canLoadNews =				0x00000800,	//	We can load news from the multiverse
			};

		ICIService (CHumanInterface &HI) : m_HI(HI), m_bEnabled(false), m_bModified(false) { }
		virtual ~ICIService (void) { }

		inline bool IsEnabled (void) { return m_bEnabled; }
		inline bool IsModified (void) { return m_bModified; }
		inline void SetEnabled (bool bValue = true) { m_bEnabled = bValue; }

		virtual ALERROR ChangePassword (ITaskProcessor *pProcessor, const CString &sUsername, const CString &sOldPassword, const CString &sNewPassword, CString *retsResult = NULL) { return NOERROR; }
		virtual CString GetTag (void) = 0;
		virtual CString GetUsername (void) { return NULL_STR; }
		virtual bool HasCapability (DWORD dwCapability) { return false; }
		virtual ALERROR Housekeeping (ITaskProcessor *pProcessor) { return NOERROR; }
		virtual ALERROR InitFromXML (CXMLElement *pDesc, bool *retbModified) { *retbModified = false; return NOERROR; }
		virtual ALERROR InitPrivateData (void) { return NOERROR; }
		virtual ALERROR LoadNews (ITaskProcessor *pProcessor, CMultiverseModel &Multiverse, const SFileVersionInfo &AppVersion, const CString &sCacheFilespec, CString *retsResult = NULL) { return NOERROR; }
		virtual ALERROR LoadUserCollection (ITaskProcessor *pProcessor, CMultiverseModel &Multiverse, CString *retsResult = NULL) { return NOERROR; }
		virtual ALERROR PostCrashReport (ITaskProcessor *pProcessor, const CString &sCrash, CString *retsResult = NULL) { return NOERROR; }
		virtual ALERROR PostGameRecord (ITaskProcessor *pProcessor, const CGameRecord &Record, const CGameStats &Stats, CString *retsResult = NULL) { return NOERROR; }
		virtual ALERROR ProcessDownloads (ITaskProcessor *pProcessor, CString *retsResult = NULL) { return NOERROR; }
		virtual ALERROR ReadProfile (ITaskProcessor *pProcessor, CUserProfile *retProfile, CString *retsResult = NULL) { return NOERROR; }
		virtual ALERROR RegisterUser (ITaskProcessor *pProcessor, const CString &sUsername, const CString &sPassword, const CString &sEmail, bool bAutoSignIn, CString *retsResult = NULL) { return NOERROR; }
		virtual ALERROR RequestExtensionDownload (const CString &sFilePath, const CString &sFilespec, const CIntegerIP &FileDigest) { return NOERROR; }
		virtual ALERROR SignInUser (ITaskProcessor *pProcessor, const CString &sUsername, const CString &sPassword, bool bAutoSignIn, CString *retsResult = NULL) { return NOERROR; }
		virtual ALERROR SignOutUser (ITaskProcessor *pProcessor, CString *retsError = NULL) { return NOERROR; }
		virtual ALERROR WriteAsXML (IWriteStream *pOutput) { return NOERROR; }
		virtual ALERROR WritePrivateData (void) { return NOERROR; }

		static ALERROR DownloadFile (const CString &sURL, IMediaType **retpBody, IHTTPClientSessionEvents *pEvents = NULL, CString *retsResult = NULL);
		static ALERROR DownloadFile (const CString &sURL, const CString &sDestFilespec, IHTTPClientSessionEvents *pEvents = NULL, CString *retsResult = NULL);

	protected:
		void SendServiceError (const CString &sStatus);
		void SendServiceStatus (const CString &sStatus);
		inline void SetModified (bool bValue = true) { m_bModified = bValue; }

		CHumanInterface &m_HI;
		bool m_bEnabled;
		bool m_bModified;
	};

class ICIServiceFactory
	{
	public:
		virtual ICIService *Create (CHumanInterface &HI) { return NULL; }
	};

class CCloudService
	{
	public:
		CCloudService (void) : m_pHI(NULL)
			{ }

		~CCloudService (void);

		void CleanUp (void);
		CString GetDefaultUsername (void);
		CString GetUsername (void);
		bool HasCapability (DWORD dwCapability);
		ALERROR InitFromXML (CHumanInterface &HI, CXMLElement *pDesc, bool *retbModified);
		ALERROR InitPrivateData (void);
		bool IsEmpty (void) { return (m_Services.GetCount() == 0); }
		bool IsModified (void);
		ALERROR WriteAsXML (IWriteStream *pOutput);
		ALERROR WritePrivateData (void);

		ALERROR ChangePassword (ITaskProcessor *pProcessor, const CString &sUsername, const CString &sOldPassword, const CString &sNewPassword, CString *retsResult = NULL);
		ALERROR DownloadUpgrade (ITaskProcessor *pProcessor, const CString &sUpgradeURL, CString *retsResult = NULL);
		ALERROR Housekeeping (ITaskProcessor *pProcessor);
		ALERROR LoadNews (ITaskProcessor *pProcessor, CMultiverseModel &Multiverse, const SFileVersionInfo &AppVersion, const CString &sCacheFilespec, CString *retsResult = NULL);
		ALERROR LoadUserCollection (ITaskProcessor *pProcessor, CMultiverseModel &Multiverse, CString *retsResult = NULL);
		ALERROR PostCrashReport (ITaskProcessor *pProcessor, const CString &sCrash, CString *retsResult = NULL);
		ALERROR PostGameRecord (ITaskProcessor *pProcessor, const CGameRecord &Record, const CGameStats &Stats, CString *retsResult = NULL);
		ALERROR ProcessDownloads (ITaskProcessor *pProcessor, CString *retsResult = NULL);
		ALERROR ReadProfile (ITaskProcessor *pProcessor, CUserProfile *retProfile, CString *retsResult = NULL);
		ALERROR RegisterUser (ITaskProcessor *pProcessor, const CString &sUsername, const CString &sPassword, const CString &sEmail, bool bAutoSignIn, CString *retsResult = NULL);
		ALERROR RequestExtensionDownload (const CString &sFilePath, const CString &sFilespec, const CIntegerIP &FileDigest);
		ALERROR SignInUser (ITaskProcessor *pProcessor, const CString &sUsername, const CString &sPassword, bool bAutoSignIn, CString *retsResult = NULL);
		ALERROR SignOutUser (ITaskProcessor *pProcessor, CString *retsError = NULL);

	private:
		void SendServiceError (const CString &sStatus);
		void SendServiceStatus (const CString &sStatus);

		CHumanInterface *m_pHI;
		TArray<ICIService *> m_Services;
	};

//	Default Services -----------------------------------------------------------

class CHexarcServiceFactory : public ICIServiceFactory
	{
	public:
		virtual ICIService *Create (CHumanInterface &HI);
	};

class CXelerusServiceFactory : public ICIServiceFactory
	{
	public:
		virtual ICIService *Create (CHumanInterface &HI);
	};

