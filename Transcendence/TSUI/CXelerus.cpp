//	CXelerus.cpp
//
//	CXelerus class
//	Copyright (c) 2010 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

#define XELERUS_TAG								CONSTLIT("Xelerus")

#define ENABLED_ATTRIB							CONSTLIT("enabled")
#define HOST_ATTRIB								CONSTLIT("host")
#define POST_STATS_URL_ATTRIB					CONSTLIT("postStatsURL")

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

CString CXelerus::GetTag (void)

//	GetTag
//
//	Returns the element tag used to save settings

	{
	return XELERUS_TAG;
	}

ALERROR CXelerus::InitFromXML (CXMLElement *pDesc, bool *retbModified)

//	InitFromXML
//
//	Initializes from XML settings

	{
	*retbModified = false;

	if (pDesc == NULL)
		{
		SetEnabled(false);
		*retbModified = true;
		return NOERROR;
		}

	SetEnabled(pDesc->GetAttributeBool(ENABLED_ATTRIB));
	m_sHost = pDesc->GetAttribute(HOST_ATTRIB);
	if (m_sHost.IsBlank())
		m_sHost = CONSTLIT("xelerus.de");

	m_sPostStatsURL = pDesc->GetAttribute(POST_STATS_URL_ATTRIB);
	if (m_sPostStatsURL.IsBlank())
		m_sPostStatsURL = CONSTLIT("/index.php?s=stats_submit");

	return NOERROR;
	}

ALERROR CXelerus::PostGameStats (ITaskProcessor *pProcessor, const CGameStats &Stats, CString *retsResult)

//	PostGameStats
//
//	Posts game stats to the cloud

	{
	EInetsErrors iError;

	pProcessor->SetProgress(CONSTLIT("Posting game statistics to Xelerus..."));

	//	Generate a stats text buffer

	CMemoryWriteStream StatsBuffer;
	if (StatsBuffer.Create() != NOERROR)
		{
		*retsResult = CONSTLIT("Unable to post stats to Xelerus: Out of memory.");
		return inetsOutOfMemory;
		}

	if (Stats.WriteAsText(&StatsBuffer) != NOERROR)
		{
		*retsResult = CONSTLIT("Unable to post stats to Xelerus: Out of memory.");
		return inetsOutOfMemory;
		}

	//	Compose the HTML form

	CHTMLForm *pForm = new CHTMLForm;
	pForm->AddField(CONSTLIT("op"), CONSTLIT("contact"));
	pForm->AddField(CONSTLIT("via"), CONSTLIT("game"));
	pForm->AddField(CONSTLIT("secondname"), Stats.GetDefaultSectionName());
	pForm->AddField(CONSTLIT("statistics"), CString(StatsBuffer.GetPointer(), StatsBuffer.GetLength(), true));

	//	Compose a post request

	CHTTPMessage Request;
	Request.InitRequest(CONSTLIT("POST"), m_sPostStatsURL);
	Request.AddHeader(CONSTLIT("Host"), m_sHost);
	Request.AddHeader(CONSTLIT("User-Agent"), CONSTLIT("TranscendenceClient/1.0"));
	Request.AddHeader(CONSTLIT("Accept-Language"), CONSTLIT("en-US"));
	Request.AddHeader(CONSTLIT("Connection"), CONSTLIT("close"));
	Request.SetBody(pForm);

	//	Establish a session

	CHTTPClientSession Session;
	Session.SetStopEvent(pProcessor->GetStopEvent());
	if (iError = Session.Connect(m_sHost))
		{
		if (iError == inetsDNSError)
			*retsResult = strPatternSubst(CONSTLIT("Unable to find http://%s."), m_sHost);
		else
			*retsResult = strPatternSubst(CONSTLIT("Unable to connect to http://%s."), m_sHost);
		return ERR_FAIL;
		}

	//	Send the request and wait for response

	CHTTPMessage Response;
	if (iError = Session.Send(Request, &Response))
		{
		*retsResult = strPatternSubst(CONSTLIT("Unable to upload to http://%s/%s."), m_sHost, m_sPostStatsURL);
		return ERR_FAIL;
		}

	//	Done

	Session.Disconnect();
	*retsResult = CONSTLIT("Game statistics posted to Xelerus.");
	return NOERROR;
	}

ALERROR CXelerus::WriteAsXML (IWriteStream *pOutput)

//	WriteAsXML
//
//	Writes settings to XML file

	{
	ALERROR error;

	CString sData = strPatternSubst(CONSTLIT("\t\t<Xelerus enabled=\"%s\"/>\r\n"), 
			(IsEnabled() ? CONSTLIT("true") : CONSTLIT("false")));
	if (error = pOutput->Write(sData.GetPointer(), sData.GetLength(), NULL))
		return error;

	return NOERROR;
	}

//	CXelerusServiceFactory -----------------------------------------------------

ICIService *CXelerusServiceFactory::Create (CHumanInterface &HI)
	{
	return new CXelerus;
	}

