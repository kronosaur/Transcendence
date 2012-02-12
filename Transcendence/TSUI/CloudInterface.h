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
			getUserProfile =			0x00000004,	//	Implies that we are signed in (we have an authToken)
			loginUser =					0x00000008,	//	We have the ability to sign in (always TRUE for Hexarc)
			postGameStats =				0x00000010,	//	We can post game states
			registerUser =				0x00000020, //	We have the ability to register (always TRUE for Hexarc)
			postGameRecord =			0x00000040,	//	We can post a game record
			};

		ICIService (void) : m_bEnabled(false), m_bModified(false) { }

		inline bool IsEnabled (void) { return m_bEnabled; }
		inline bool IsModified (void) { return m_bModified; }
		inline void SetEnabled (bool bValue = true) { m_bEnabled = bValue; }

		virtual ALERROR ChangePassword (ITaskProcessor *pProcessor, const CString &sUsername, const CString &sOldPassword, const CString &sNewPassword, CString *retsResult = NULL) { return NOERROR; }
		virtual CString GetTag (void) = 0;
		virtual CString GetUsername (void) { return NULL_STR; }
		virtual bool HasCapability (DWORD dwCapability) { return false; }
		virtual ALERROR Housekeeping (ITaskProcessor *pProcessor) { return NOERROR; }
		virtual ALERROR InitFromXML (CXMLElement *pDesc, bool *retbModified) { return NOERROR; }
		virtual ALERROR InitPrivateData (void) { return NOERROR; }
		virtual ALERROR PostGameRecord (ITaskProcessor *pProcessor, const CGameRecord &Record, CString *retsResult = NULL) { return NOERROR; }
		virtual ALERROR PostGameStats (ITaskProcessor *pProcessor, const CGameStats &Stats, CString *retsResult = NULL) { return NOERROR; }
		virtual ALERROR ReadProfile (ITaskProcessor *pProcessor, CUserProfile *retProfile, CString *retsResult = NULL) { return NOERROR; }
		virtual ALERROR RegisterUser (ITaskProcessor *pProcessor, const CString &sUsername, const CString &sPassword, CString *retsResult = NULL) { return NOERROR; }
		virtual ALERROR SignInUser (ITaskProcessor *pProcessor, const CString &sUsername, const CString &sPassword, CString *retsResult = NULL) { return NOERROR; }
		virtual ALERROR SignOutUser (ITaskProcessor *pProcessor, CString *retsError = NULL) { return NOERROR; }
		virtual ALERROR WriteAsXML (IWriteStream *pOutput) { return NOERROR; }
		virtual ALERROR WritePrivateData (void) { return NOERROR; }

	protected:
		inline void SetModified (bool bValue = true) { m_bModified = bValue; }

		bool m_bEnabled;
		bool m_bModified;
	};

class CCloudService
	{
	public:
		~CCloudService (void);

		void CleanUp (void);
		CString GetUsername (void);
		bool HasCapability (DWORD dwCapability);
		ALERROR InitFromXML (CHumanInterface &HI, CXMLElement *pDesc, bool *retbModified);
		ALERROR InitPrivateData (void);
		bool IsEmpty (void) { return (m_Services.GetCount() == 0); }
		bool IsModified (void);
		ALERROR WriteAsXML (IWriteStream *pOutput);
		ALERROR WritePrivateData (void);

		ALERROR ChangePassword (ITaskProcessor *pProcessor, const CString &sUsername, const CString &sOldPassword, const CString &sNewPassword, CString *retsResult = NULL);
		ALERROR Housekeeping (ITaskProcessor *pProcessor);
		ALERROR PostGameRecord (ITaskProcessor *pProcessor, const CGameRecord &Record, CString *retsResult = NULL);
		ALERROR PostGameStats (ITaskProcessor *pProcessor, const CGameStats &Stats, CString *retsResult = NULL);
		ALERROR ReadProfile (ITaskProcessor *pProcessor, CUserProfile *retProfile, CString *retsResult = NULL);
		ALERROR RegisterUser (ITaskProcessor *pProcessor, const CString &sUsername, const CString &sPassword, CString *retsResult = NULL);
		ALERROR SignInUser (ITaskProcessor *pProcessor, const CString &sUsername, const CString &sPassword, CString *retsResult = NULL);
		ALERROR SignOutUser (ITaskProcessor *pProcessor, CString *retsError = NULL);

	private:
		TArray<ICIService *> m_Services;
	};

//	Service Implementations

class CHexarcService : public ICIService
	{
	public:
		CHexarcService (CHumanInterface &HI) : m_HI(HI) { }

		virtual ALERROR ChangePassword (ITaskProcessor *pProcessor, const CString &sUsername, const CString &sOldPassword, const CString &sNewPassword, CString *retsResult = NULL);
		virtual CString GetTag (void);
		virtual CString GetUsername (void) { return m_sUsername; }
		virtual bool HasCapability (DWORD dwCapability);
		virtual ALERROR Housekeeping (ITaskProcessor *pProcessor);
		virtual ALERROR InitFromXML (CXMLElement *pDesc, bool *retbModified);
		virtual ALERROR InitPrivateData (void);
		virtual ALERROR PostGameRecord (ITaskProcessor *pProcessor, const CGameRecord &Record, CString *retsResult = NULL);
		virtual ALERROR PostGameStats (ITaskProcessor *pProcessor, const CGameStats &Stats, CString *retsResult = NULL);
		virtual ALERROR ReadProfile (ITaskProcessor *pProcessor, CUserProfile *retProfile, CString *retsResult = NULL);
		virtual ALERROR RegisterUser (ITaskProcessor *pProcessor, const CString &sUsername, const CString &sPassword, CString *retsResult = NULL);
		virtual ALERROR SignInUser (ITaskProcessor *pProcessor, const CString &sUsername, const CString &sPassword, CString *retsResult = NULL);
		virtual ALERROR SignOutUser (ITaskProcessor *pProcessor, CString *retsError = NULL);
		virtual ALERROR WriteAsXML (IWriteStream *pOutput);
		virtual ALERROR WritePrivateData (void);

	private:
		bool Connect (CString *retsResult);
		CString GetClientVersion (void) const;
		CString GetHostspec (void) const { return (m_sPort.IsBlank() ? m_sHost : strPatternSubst("%s:%s", m_sHost, m_sPort)); }
		bool GetJSONResponse (CHTTPMessage &Response, CJSONValue *retValue, CString *retsError);
		void InitRequest (const CString &sMethod, const CString &sFunction, CHTTPMessage *retMessage);
		bool IsLoggedIn (void) { return !m_UserToken.IsNull(); }
		bool HasCachedUsername (void) { return !m_sUsername.IsBlank(); }
		bool HasCachedCredentials (void) { return !m_Credentials.IsNull(); }
		bool ServerCommand (const CString &sMethod, const CString &sFunc, CJSONValue &Payload, CJSONValue *retResult);

		CHumanInterface &m_HI;
		CString m_sHost;
		CString m_sPort;
		CString m_sRootURL;

		CString m_sClientID;						//	Unique clientID
		CString m_sUsername;						//	Username
		CJSONValue m_Credentials;					//	Saved service password (not the user password)
		CJSONValue m_UserToken;						//	Obtained after register/login

		bool m_bActualRequired;						//	Require actual username password
		CJSONValue m_Challenge;						//	Challenge

		CHTTPClientSession m_Session;				//	Connection
	};

class CXelerus : public ICIService
	{
	public:
		virtual CString GetTag (void);
		virtual bool HasCapability (DWORD dwCapability) { return (dwCapability == ICIService::postGameStats); }
		virtual ALERROR InitFromXML (CXMLElement *pDesc, bool *retbModified);
		virtual ALERROR PostGameStats (ITaskProcessor *pProcessor, const CGameStats &Stats, CString *retsResult = NULL);
		virtual ALERROR WriteAsXML (IWriteStream *pOutput);

	private:
		CString m_sHost;
		CString m_sPostStatsURL;
	};
