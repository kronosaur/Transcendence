//	CHexarcSession.cpp
//
//	CHexarcSession class
//	Copyright (c) 2012 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"

#define CLIENT_TYPE								CONSTLIT("Transcendence/Windows")

#define FIELD_CLIENT_ID							CONSTLIT("clientID")
#define FIELD_CLIENT_VERSION					CONSTLIT("clientVersion")
#define FIELD_PROTOCOL_VERSION					CONSTLIT("protocolVersion")
#define FIELD_USERNAME							CONSTLIT("username")

#define FUNC_CONNECT_CLIENT						CONSTLIT("connectClient")

#define METHOD_POST								CONSTLIT("POST")

#define MSG_ERROR_OUT_OF_DATE					CONSTLIT("Error.outOfDate")

#define ERR_FROM_SERVER							CONSTLIT("Error response from server at %s: %s")
#define ERR_INVALID_JSON						CONSTLIT("Invalid response from server at %s: %s")
#define ERR_CONNECT_ERROR						CONSTLIT("No response from server at %s.")
#define ERR_REQUEST_FAILED						CONSTLIT("Unable to communicate with %s.")
#define ERR_NO_SERVER_DEFINED					CONSTLIT("Unable to connect: no server or root URL specified.")
#define ERR_INTERNAL							CONSTLIT("Unable to connect to %s.")
#define ERR_DNS_ERROR							CONSTLIT("Unable to resolve address for %s.")
#define ERR_CANNOT_SEND							CONSTLIT("Unable to send command to server.")
#define ERR_UNKNOWN_RESPONSE_TYPE				CONSTLIT("Unknown response content type.")
#define ERR_NO_INTERNET							CONSTLIT("You are not connected to the Internet.")

const DWORD MAX_CONNECTION_IDLE_TIME =			5 * 60 * 1000;

const int TRANS_SERVICE_VERSION =				2;

CHexarcSession::CHexarcSession (const CString &sHost, const CString &sPort, const CString &sRootURL) :
		m_sHost(sHost),
		m_sPort(sPort),
		m_sRootURL(sRootURL)

//	CHexarcSession constructor

	{
	}

bool CHexarcSession::Connect (CString *retsError)

//	Connect
//
//	Connect to the server (if not already connected)

	{
	//	Already connected?

	if (m_Session.IsConnected())
		return true;

	//	Need host and root URL

	if (m_sHost.IsBlank() || m_sRootURL.IsBlank())
		{
		*retsError = ERR_NO_SERVER_DEFINED;
		return false;
		}

	//	Connect to host

	EInetsErrors iError = m_Session.Connect(m_sHost, m_sPort);
	if (iError != inetsOK)
		{
		switch (iError)
			{
			case inetsDNSError:
				*retsError = strPatternSubst(ERR_DNS_ERROR, GetHostspec());
				kernelDebugLogMessage(*retsError);
				return false;

			case inetsCannotConnect:
				*retsError = strPatternSubst(ERR_CONNECT_ERROR, GetHostspec());
				kernelDebugLogMessage(*retsError);
				return false;

			default:
				*retsError = strPatternSubst(ERR_INTERNAL, GetHostspec());
				kernelDebugLogMessage(*retsError);
				return false;
			}
		}

	//	Compose a connect payload

	CJSONValue Payload(CJSONValue::typeObject);
	Payload.InsertHandoff(FIELD_CLIENT_ID, CJSONValue(m_sClientID));
	Payload.InsertHandoff(FIELD_CLIENT_VERSION, CJSONValue(GetClientVersion()));
	Payload.InsertHandoff(FIELD_PROTOCOL_VERSION, CJSONValue(TRANS_SERVICE_VERSION));
	if (!m_sUsername.IsBlank())
		Payload.InsertHandoff(FIELD_USERNAME, CJSONValue(m_sUsername));

	//	Now issue the connect command

	CHTTPMessage Request;
	InitRequest(METHOD_POST, FUNC_CONNECT_CLIENT, &Request);
	Request.SetBody(new CJSONMessage(Payload));

	//	Send the request and wait for response

	CHTTPMessage Response;
	if (iError = m_Session.Send(Request, &Response))
		{
		m_Session.Disconnect();
		*retsError = strPatternSubst(ERR_REQUEST_FAILED, GetHostspec());
		kernelDebugLogMessage(*retsError);
		return false;
		}

	//	If we got a 301 redirect then we try the whole thing again

	if (Response.GetStatusCode() == 301)
		{
		m_Session.Disconnect();

		//	LATER: Set up the new host and recurse
		//	LATER: Keep track of recursion count, in case of ping-pong redirects
		*retsError = CONSTLIT("LATER: Redirect");
		kernelDebugLogMessage(*retsError);
		return false;
		}

	//	If we get an error, return

	else if (Response.GetStatusCode() != 200)
		{
		m_Session.Disconnect();
		*retsError = strPatternSubst(ERR_FROM_SERVER, GetHostspec(), Response.GetStatusMsg());
		kernelDebugLogMessage(*retsError);
		return false;
		}

	//	Get the JSON response

	CJSONValue ResponseData;
	if (!GetJSONResponse(Response, &ResponseData, retsError))
		{
		m_Session.Disconnect();
		*retsError = strPatternSubst(ERR_INVALID_JSON, GetHostspec(), *retsError);
		return false;
		}

	//	If this is an error, then return it

	if (CHexarc::IsError(ResponseData, NULL, retsError))
		{
		m_Session.Disconnect();
		kernelDebugLogMessage(*retsError);
		return false;
		}

	//	LATER: Process return value (challenge)

	//	Done

	return true;
	}

CString CHexarcSession::GetClientVersion (void) const

//	GetClientVersion
//
//	Returns the client type and version in the form:
//
//	Transcendence/Windows/1.1a

	{
	SFileVersionInfo VerInfo;
	::fileGetVersionInfo(NULL_STR, &VerInfo);

	return strPatternSubst("%s/%s", CLIENT_TYPE, VerInfo.sProductVersion);
	}

bool CHexarcSession::GetJSONResponse (CHTTPMessage &Response, CJSONValue *retValue, CString *retsError)

//	GetJSONResponse
//
//	Returns the JSON payload, or false if there is an error.

	{
	//	Make sure it is the proper type

	CString sMediaType;
	if (!Response.FindHeader(CONSTLIT("Content-Type"), &sMediaType) || !strEquals(sMediaType, CONSTLIT("application/json")))
		{
		*retsError = ERR_UNKNOWN_RESPONSE_TYPE;
		return false;
		}

	//	Deserialized

	if (CJSONValue::Deserialize(Response.GetBodyBuffer(), retValue, retsError) != NOERROR)
		return false;

	//	Done

	return true;
	}

void CHexarcSession::Housekeeping (void)

//	Housekeeping
//
//	Call every minute or so.

	{
	//	If our session is connected and we haven't had any activity in a certain
	//	period of time, disconnect.

	if (m_Session.IsConnected() 
			&& m_Session.GetTicksSinceLastActivity() > MAX_CONNECTION_IDLE_TIME)
		m_Session.Disconnect();
	}

void CHexarcSession::InitRequest (const CString &sMethod, const CString &sFunction, CHTTPMessage *retMessage)

//	InitRequest
//
//	Initializes a request

	{
	retMessage->InitRequest(sMethod, strPatternSubst(CONSTLIT("%s%s"), m_sRootURL, sFunction));
	retMessage->AddHeader(CONSTLIT("Host"), m_sHost);
	retMessage->AddHeader(CONSTLIT("User-Agent"), CONSTLIT("TranscendenceClient/1.0"));
	retMessage->AddHeader(CONSTLIT("Accept-Language"), CONSTLIT("en-US"));
	}

ALERROR CHexarcSession::ServerCommand (const CHTTPMessage &Request, CJSONValue *retResult)

//	ServerCommand
//
//	Sends a command to the server and gets back a result.

	{
	//	If we're currently connected (or think we are) do a reconnect on failure.

	bool bReconnectOnFailure = m_Session.IsConnected();

	//	Make sure we're connected

	CString sError;
	if (!Connect(&sError))
		{
		if (!m_Session.IsInternetAvailable())
			{
			*retResult = CJSONValue(ERR_NO_INTERNET);
			kernelDebugLogMessage("Unable to verify connection to the Internet.");
			}
		else
			*retResult = CJSONValue(sError);
		return ERR_FAIL;
		}

	//	Send the request and wait for response

	EInetsErrors iError;
	CHTTPMessage Response;
	if (iError = m_Session.Send(Request, &Response))
		{
		//	Reconnect, if necessary

		if (bReconnectOnFailure)
			{
			m_Session.Disconnect();
			return ServerCommand(Request, retResult);
			}

		//	Check to see if we have internet access. If we don't then that
		//	explains the problem.

		else if (!m_Session.IsInternetAvailable())
			{
			*retResult = CJSONValue(ERR_NO_INTERNET);
			kernelDebugLogMessage("Unable to verify connection to the Internet.");
			return ERR_FAIL;
			}

		//	Otherwise, error

		else
			{
			*retResult = CJSONValue(ERR_CANNOT_SEND);
			kernelDebugLogMessage(strPatternSubst(CONSTLIT("%s: Unable to send command to server."), Request.GetURL()));
			return ERR_FAIL;
			}
		}

	//	If we get an error, return

	if (Response.GetStatusCode() != 200)
		{
		*retResult = CJSONValue(Response.GetStatusMsg());
		kernelDebugLogMessage(strPatternSubst(CONSTLIT("%s: %s."), Request.GetURL(), Response.GetStatusMsg()));
		return ERR_FAIL;
		}

	//	Get the JSON response

	if (!GetJSONResponse(Response, retResult, &sError))
		{
		sError = strPatternSubst(ERR_INVALID_JSON, GetHostspec(), sError);
		*retResult = CJSONValue(sError);
		kernelDebugLogMessage(strPatternSubst(CONSTLIT("%s: %s"), Request.GetURL(), sError));
		return ERR_FAIL;
		}

	//	See if it is a server error code

	CString sErrorCode;
	CString sErrorDesc;
	if (CHexarc::IsError(*retResult, &sErrorCode, &sErrorDesc))
		{
		*retResult = CJSONValue(sErrorDesc);
		kernelDebugLogMessage(strPatternSubst(CONSTLIT("%s: Server returned error: %s"), Request.GetURL(), sErrorDesc));

		//	If this is Error.outOfDate then we return a different error code

		if (strEquals(sErrorCode, MSG_ERROR_OUT_OF_DATE))
			return ERR_OUTOFDATE;
		else
			return ERR_FAIL;
		}

	//	Done

	return NOERROR;
	}

ALERROR CHexarcSession::ServerCommand (const CString &sMethod, const CString &sFunc, CJSONValue &Payload, CJSONValue *retResult)

//	ServerCommand
//
//	Sends a command to the server and gets back a result. We assume that we are
//	connected.
//
//	NOTE: We clobber the Payload.

	{
	//	Create the message

	CHTTPMessage Request;
	InitRequest(sMethod, sFunc, &Request);
	Request.SetBody(new CJSONMessage(Payload));

	//	Send

	return ServerCommand(Request, retResult);
	}

void CHexarcSession::SetServer (const CString &sHost, const CString &sPort, const CString &sRootURL)

//	SetServer
//
//	Sets server parameters. Cannot be called once we are connected.

	{
	if (m_Session.IsConnected())
		return;

	m_sHost = sHost;
	m_sPort = sPort;
	m_sRootURL = sRootURL;
	}
