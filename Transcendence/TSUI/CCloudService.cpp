//	CCloudService.cpp
//
//	CCloudService class
//	Copyright (c) 2010 by George Moromisato. All Rights Reserved.

#include "stdafx.h"
#include "Internets.h"

#define CMD_SERVICE_ERROR								CONSTLIT("serviceError")
#define CMD_SERVICE_STATUS								CONSTLIT("serviceStatus")
#define CMD_SERVICE_UPGRADE_READY						CONSTLIT("serviceUpgradeReady")

#define PROTOCOL_HTTP									CONSTLIT("http")

#define STR_DOWNLOADING_UPGRADE							CONSTLIT("Downloading upgrade...")
#define STR_DOWNLOADING_UPGRADE_PROGRESS				CONSTLIT("Downloading upgrade...%s")

class CCommsProgress : public IHTTPClientSessionEvents
	{
	public:
		CCommsProgress (CHumanInterface &HI, const CString &sText) :
				m_HI(HI),
				m_sText(sText)
			{ }

		virtual void OnReceiveData (int iBytesReceived, int iBytesLeft);

	private:
		CHumanInterface &m_HI;
		CString m_sText;
	};

CCloudService::~CCloudService (void)

//	CCloudService destructor

	{
	CleanUp();
	}

ALERROR CCloudService::ChangePassword (ITaskProcessor *pProcessor, const CString &sUsername, const CString &sOldPassword, const CString &sNewPassword, CString *retsResult)

//	ChangePassword
//
//	Change the user's password

	{
	int i;

	for (i = 0; i < m_Services.GetCount(); i++)
		if (m_Services[i]->IsEnabled() && m_Services[i]->HasCapability(ICIService::loginUser))
			{
			//	Only one service can sign in at a user
			return m_Services[i]->ChangePassword(pProcessor, sUsername, sOldPassword, sNewPassword, retsResult);
			}

	return NOERROR;
	}

void CCloudService::CleanUp (void)

//	CleanUp
//
//	Cleans up services

	{
	int i;
	for (i = 0; i < m_Services.GetCount(); i++)
		delete m_Services[i];

	m_Services.DeleteAll();
	}

ALERROR CCloudService::DownloadUpgrade (ITaskProcessor *pProcessor, const CString &sDownloadURL, CString *retsResult)

//	DownloadUpgrade
//
//	Download a new game upgrade

	{
	SendServiceStatus(STR_DOWNLOADING_UPGRADE);

	//	Parse the URL to get the host name

	IMediaType *pBody;
	CCommsProgress Progress(*m_pHI, STR_DOWNLOADING_UPGRADE_PROGRESS);
	if (ICIService::DownloadFile(sDownloadURL, &pBody, &Progress, retsResult) != NOERROR)
		{
		SendServiceError(retsResult ? *retsResult : CONSTLIT("Unknown error"));
		return ERR_FAIL;
		}

	//	Return the body up our controller (it takes ownership of it).

	m_pHI->HICommand(CMD_SERVICE_UPGRADE_READY, pBody);

	return NOERROR;
	}

CString CCloudService::GetDefaultUsername (void)

//	GetDefaultUsername
//
//	Returns the username (not necessarily signed in).

	{
	int i;

	//	See if any services can return a username

	CString sUsername;
	for (i = 0; i < m_Services.GetCount(); i++)
		if (m_Services[i]->IsEnabled())
			{
			CString sUsername = m_Services[i]->GetUsername();
			if (!sUsername.IsBlank())
				return sUsername;
			}

	return NULL_STR;
	}

CString CCloudService::GetUsername (void)

//	GetUsername
//
//	Returns the signed in username (or NULL_STR if no one is signed in).

	{
	if (HasCapability(ICIService::canGetUserProfile))
		return GetDefaultUsername();
	else
		return NULL_STR;
	}

bool CCloudService::HasCapability (DWORD dwCapability)

//	HasCapability
//
//	Returns TRUE if any service has the given capability

	{
	int i;

	//	See if any services can post stats

	for (i = 0; i < m_Services.GetCount(); i++)
		if (m_Services[i]->IsEnabled() && m_Services[i]->HasCapability(dwCapability))
			return true;

	return false;
	}

ALERROR CCloudService::Housekeeping (ITaskProcessor *pProcessor)

//	Housekeeping
//
//	Give each service a chance to do some background tasks.

	{
	int i;

	for (i = 0; i < m_Services.GetCount(); i++)
		if (m_Services[i]->IsEnabled())
			m_Services[i]->Housekeeping(pProcessor);

	return NOERROR;
	}

ALERROR CCloudService::InitFromXML (CHumanInterface &HI, CXMLElement *pDesc, bool *retbModified)

//	InitFromXML
//
//	Initializes from XML settings

	{
	ALERROR error;
	int i;

	ASSERT(m_Services.GetCount() == 0);
	m_pHI = &HI;

	//	Get the list of default services

	CHexarcServiceFactory HexarcService;
	CXelerusServiceFactory XelerusService;

	TArray<ICIServiceFactory *> AllServices;
	AllServices.Insert(&HexarcService);
	AllServices.Insert(&XelerusService);

	//	If we don't have any settings, then we use defaults and save out

	bool bModified = (pDesc == NULL);
	*retbModified = bModified;

	//	Initialize all services

	for (i = 0; i < AllServices.GetCount(); i++)
		{
		ICIService *pService = AllServices[i]->Create(HI);
		if (pService)
			{
			if (error = pService->InitFromXML((pDesc ? pDesc->GetContentElementByTag(pService->GetTag()) : NULL), &bModified))
				return error;

			if (bModified)
				*retbModified = true;

			m_Services.Insert(pService);
			}
		}

	//	Done

	return NOERROR;
	}

ALERROR CCloudService::InitPrivateData (void)

//	InitPrivateData
//
//	Allow services to load their private data.

	{
	ALERROR error;
	int i;

	for (i = 0; i < m_Services.GetCount(); i++)
		if (error = m_Services[i]->InitPrivateData())
			return error;

	return NOERROR;
	}

bool CCloudService::IsModified (void)

//	IsModified
//
//	Returns TRUE if any of the services have their settings modified.

	{
	int i;

	for (i = 0; i < m_Services.GetCount(); i++)
		if (m_Services[i]->IsModified())
			return true;

	return false;
	}

ALERROR CCloudService::LoadNews (ITaskProcessor *pProcessor, CMultiverseModel &Multiverse, const SFileVersionInfo &AppVersion, const CString &sCacheFilespec, CString *retsResult)

//	LoadNews
//
//	Loads the news from the multiverse

	{
	int i;

	for (i = 0; i < m_Services.GetCount(); i++)
		if (m_Services[i]->IsEnabled() && m_Services[i]->HasCapability(ICIService::canLoadNews))
			{
			//	For now we only support a single service

			return m_Services[i]->LoadNews(pProcessor, Multiverse, AppVersion, sCacheFilespec, retsResult);
			}

	return NOERROR;
	}

ALERROR CCloudService::LoadUserCollection (ITaskProcessor *pProcessor, CMultiverseModel &Multiverse, CString *retsResult)

//	LoadUserCollection
//
//	Loads the user collection from the cloud

	{
	int i;

	for (i = 0; i < m_Services.GetCount(); i++)
		if (m_Services[i]->IsEnabled() && m_Services[i]->HasCapability(ICIService::canLoadUserCollection))
			{
			//	For now we only support a single service

			return m_Services[i]->LoadUserCollection(pProcessor, Multiverse, retsResult);
			}

	return NOERROR;
	}

ALERROR CCloudService::PostCrashReport (ITaskProcessor *pProcessor, const CString &sCrash, CString *retsResult)

//	PostCrashReport
//
//	Posts a crash report to the cloud

	{
	int i;

	for (i = 0; i < m_Services.GetCount(); i++)
		if (m_Services[i]->IsEnabled() && m_Services[i]->HasCapability(ICIService::canPostCrashReport))
			{
			//	For now we only support a single service

			return m_Services[i]->PostCrashReport(pProcessor, sCrash, retsResult);
			}

	return NOERROR;
	}

ALERROR CCloudService::PostGameRecord (ITaskProcessor *pProcessor, const CGameRecord &Record, const CGameStats &Stats, CString *retsResult)

//	PostGameRecord
//
//	Posts the game record to the cloud

	{
	ALERROR error;
	int i;

	ALERROR firstError = NOERROR;

	for (i = 0; i < m_Services.GetCount(); i++)
		if (m_Services[i]->IsEnabled() && m_Services[i]->HasCapability(ICIService::canPostGameRecord))
			{
			CString sError;
			if (error = m_Services[i]->PostGameRecord(pProcessor, Record, Stats, &sError))
				{
				if (firstError == NOERROR)
					{
					firstError = error;
					*retsResult = sError;
					}
				}
			}

	return firstError;
	}

ALERROR CCloudService::ProcessDownloads (ITaskProcessor *pProcessor, CString *retsResult)

//	ProcessDownloads
//
//	Gives the service a chance to process downloads

	{
	int i;

	for (i = 0; i < m_Services.GetCount(); i++)
		if (m_Services[i]->IsEnabled() && m_Services[i]->HasCapability(ICIService::canDownloadExtension))
			{
			//	For now we only support posting stats to a single service

			return m_Services[i]->ProcessDownloads(pProcessor, retsResult);
			}

	return NOERROR;
	}

ALERROR CCloudService::ReadProfile (ITaskProcessor *pProcessor, CUserProfile *retProfile, CString *retsResult)

//	ReadProfile
//
//	Read the user's profile

	{
	int i;

	for (i = 0; i < m_Services.GetCount(); i++)
		if (m_Services[i]->IsEnabled() && m_Services[i]->HasCapability(ICIService::canGetUserProfile))
			{
			//	For now we only support posting stats to a single service

			return m_Services[i]->ReadProfile(pProcessor, retProfile, retsResult);
			}

	return NOERROR;
	}

ALERROR CCloudService::RegisterUser (ITaskProcessor *pProcessor, const CString &sUsername, const CString &sPassword, const CString &sEmail, bool bAutoSignIn, CString *retsResult)

//	RegisterUser
//
//	Registers a new user

	{
	int i;

	for (i = 0; i < m_Services.GetCount(); i++)
		if (m_Services[i]->IsEnabled() && m_Services[i]->HasCapability(ICIService::registerUser))
			{
			//	Only one service can register a user
			return m_Services[i]->RegisterUser(pProcessor, sUsername, sPassword, sEmail, bAutoSignIn, retsResult);
			}

	return NOERROR;
	}

ALERROR CCloudService::RequestExtensionDownload (const CString &sFilePath, const CString &sFilespec, const CIntegerIP &FileDigest)

//	RequestExtensionDownload
//
//	Download an extension file.

	{
	int i;

	for (i = 0; i < m_Services.GetCount(); i++)
		if (m_Services[i]->IsEnabled() && m_Services[i]->HasCapability(ICIService::canDownloadExtension))
			{
			//	Only one service can register a user
			return m_Services[i]->RequestExtensionDownload(sFilePath, sFilespec, FileDigest);
			}

	return NOERROR;
	}

void CCloudService::SendServiceError (const CString &sStatus)

//	SendServiceError
//
//	Sends current status to the controller.

	{
	if (m_pHI)
		{
		CString *pData = new CString(sStatus);
		m_pHI->HIPostCommand(CMD_SERVICE_ERROR, pData);
		}
	}

void CCloudService::SendServiceStatus (const CString &sStatus)

//	SendServiceStatus
//
//	Sends current status to the controller.

	{
	if (m_pHI)
		{
		CString *pData = new CString(sStatus);
		m_pHI->HIPostCommand(CMD_SERVICE_STATUS, pData);
		}
	}

ALERROR CCloudService::SignInUser (ITaskProcessor *pProcessor, const CString &sUsername, const CString &sPassword, bool bAutoSignIn, CString *retsResult)

//	SignInUser
//
//	Signs in a user

	{
	int i;

	for (i = 0; i < m_Services.GetCount(); i++)
		if (m_Services[i]->IsEnabled() && m_Services[i]->HasCapability(ICIService::loginUser))
			{
			//	Only one service can sign in at a user
			return m_Services[i]->SignInUser(pProcessor, sUsername, sPassword, bAutoSignIn, retsResult);
			}

	return NOERROR;
	}

ALERROR CCloudService::SignOutUser (ITaskProcessor *pProcessor, CString *retsResult)

//	SignOutUser
//
//	Signs out a user

	{
	int i;

	for (i = 0; i < m_Services.GetCount(); i++)
		if (m_Services[i]->IsEnabled() && m_Services[i]->HasCapability(ICIService::canGetUserProfile))
			{
			//	Only one service can sign in at a user
			return m_Services[i]->SignOutUser(pProcessor, retsResult);
			}

	return NOERROR;
	}

ALERROR CCloudService::WriteAsXML (IWriteStream *pOutput)

//	WriteAsXML
//
//	Writes out the XML settings

	{
	ALERROR error;
	int i;

	//	Open tag

	CString sData;
	sData = CONSTLIT("\t<Services>\r\n");
	if (error = pOutput->Write(sData.GetPointer(), sData.GetLength(), NULL))
		return error;

	//	Services

	for (i = 0; i < m_Services.GetCount(); i++)
		if (error = m_Services[i]->WriteAsXML(pOutput))
			return error;

	//	Close tag

	sData = CONSTLIT("\t</Services>\r\n");
	if (error = pOutput->Write(sData.GetPointer(), sData.GetLength(), NULL))
		return error;

	return NOERROR;
	}

ALERROR CCloudService::WritePrivateData (void)

//	WritePrivateData
//
//	Allow services to save their private data.

	{
	ALERROR error;
	int i;

	for (i = 0; i < m_Services.GetCount(); i++)
		if (error = m_Services[i]->WritePrivateData())
			return error;

	return NOERROR;
	}

//	ICIService -----------------------------------------------------------------

ALERROR ICIService::DownloadFile (const CString &sURL, IMediaType **retpBody, IHTTPClientSessionEvents *pEvents, CString *retsResult)

//	DownloadFile
//
//	Downloads a file at the given URL (synchronously). If successful, the 
//	contents of the file are in retResult. Otherwise, retResult is an error
//	string.

	{
	DEBUG_TRY

	CHTTPClientSession Session;

	//	Parse the URL to get the host name

	CString sProtocol;
	CString sHost;
	CString sPath;
	if (!urlParse(sURL.GetASCIIZPointer(), &sProtocol, &sHost, &sPath))
		{
		if (retsResult) *retsResult = strPatternSubst(CONSTLIT("Unable to parse download URL: %s."), sURL);
		return ERR_FAIL;
		}

	if (!strEquals(sProtocol, PROTOCOL_HTTP))
		{
		if (retsResult) *retsResult = strPatternSubst(CONSTLIT("Unsupported protocol in download URL: %s."), sURL);
		return ERR_FAIL;
		}

	//	Connect to the host

	if (Session.Connect(sHost, CONSTLIT("80")) != inetsOK)
		{
		if (retsResult) *retsResult = strPatternSubst(CONSTLIT("Unable to connect to server: %s."), sURL);
		return ERR_FAIL;
		}

	//	Now issue a GET

	CHTTPMessage Request;
	Request.InitRequest(CONSTLIT("GET"), sPath);
	Request.AddHeader(CONSTLIT("Host"), sHost);
	Request.AddHeader(CONSTLIT("User-Agent"), CONSTLIT("TranscendenceClient/1.0"));
	Request.AddHeader(CONSTLIT("Accept-Language"), CONSTLIT("en-US"));

	//	Send the request and wait for response

	CHTTPMessage Response;
	if (Session.Send(Request, &Response, pEvents) != inetsOK)
		{
		if (retsResult) *retsResult = strPatternSubst(CONSTLIT("%s: Error sending to server."), sURL);
		return ERR_FAIL;
		}

	//	If we get an error, return

	if (Response.GetStatusCode() != 200)
		{
		if (retsResult) *retsResult = strPatternSubst(CONSTLIT("%s: %s"), sURL, Response.GetStatusMsg());
		return ERR_FAIL;
		}

	//	Return the body

	if (retpBody)
		*retpBody = Response.GetBodyHandoff();

	return NOERROR;

	DEBUG_CATCH
	}

ALERROR ICIService::DownloadFile (const CString &sURL, const CString &sDestFilespec, IHTTPClientSessionEvents *pEvents, CString *retsResult)

//	DownloadFile
//
//	Downloads the file and writes it to the given filespec

	{
	DEBUG_TRY

	ALERROR error;

	//	Download first (otherwise we have to delete the file on error).

	IMediaType *pBody;
	if (DownloadFile(sURL, &pBody, pEvents, retsResult) != NOERROR)
		return ERR_FAIL;

	//	Open the file for writing

	CFileWriteStream DestFile(sDestFilespec);
	if (DestFile.Create() != NOERROR)
		{
		delete pBody;
		if (retsResult) *retsResult = strPatternSubst(CONSTLIT("Unable to create file: %s"), sDestFilespec);
		return ERR_FAIL;
		}

	//	Write the body out

	error = DestFile.Write(pBody->GetMediaBuffer().GetPointer(), pBody->GetMediaLength());
	delete pBody;
	if (error)
		{
		if (retsResult) *retsResult = strPatternSubst(CONSTLIT("Unable to write to file: %s"), sDestFilespec);
		return ERR_FAIL;
		}

	//	Done

	return NOERROR;

	DEBUG_CATCH
	}

void ICIService::SendServiceError (const CString &sStatus)

//	SendServiceError
//
//	Sends current status to the controller.

	{
	CString *pData = new CString(sStatus);
	m_HI.HIPostCommand(CMD_SERVICE_ERROR, pData);
	}

void ICIService::SendServiceStatus (const CString &sStatus)

//	SendServiceStatus
//
//	Sends current status to the controller.

	{
	CString *pData = new CString(sStatus);
	m_HI.HIPostCommand(CMD_SERVICE_STATUS, pData);
	}

//	CCommsProgress -------------------------------------------------------------

void CCommsProgress::OnReceiveData (int iBytesReceived, int iBytesLeft)

//	OnReceiveData
//
//	Display progress

	{
	CString *pData = new CString(strPatternSubst(m_sText, strFormatBytes(iBytesReceived)));
	m_HI.HIPostCommand(CMD_SERVICE_STATUS, pData);
	}
