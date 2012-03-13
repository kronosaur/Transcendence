//	CCloudService.cpp
//
//	CCloudService class
//	Copyright (c) 2010 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

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
	if (HasCapability(ICIService::getUserProfile))
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

ALERROR CCloudService::PostGameRecord (ITaskProcessor *pProcessor, const CGameRecord &Record, const CGameStats &Stats, CString *retsResult)

//	PostGameRecord
//
//	Posts the game record to the cloud

	{
	int i;

	for (i = 0; i < m_Services.GetCount(); i++)
		if (m_Services[i]->IsEnabled() && m_Services[i]->HasCapability(ICIService::postGameRecord))
			{
			//	For now we only support posting stats to a single service

			return m_Services[i]->PostGameRecord(pProcessor, Record, Stats, retsResult);
			}

	return NOERROR;
	}

ALERROR CCloudService::PostGameStats (ITaskProcessor *pProcessor, const CGameStats &Stats, CString *retsResult)

//	PostGameStats
//
//	Posts the game stats to the cloud

	{
	int i;

	for (i = 0; i < m_Services.GetCount(); i++)
		if (m_Services[i]->IsEnabled() && m_Services[i]->HasCapability(ICIService::postGameStats))
			{
			//	For now we only support posting stats to a single service

			return m_Services[i]->PostGameStats(pProcessor, Stats, retsResult);
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
		if (m_Services[i]->IsEnabled() && m_Services[i]->HasCapability(ICIService::getUserProfile))
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
		if (m_Services[i]->IsEnabled() && m_Services[i]->HasCapability(ICIService::getUserProfile))
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

