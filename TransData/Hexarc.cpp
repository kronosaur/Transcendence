//	Hexarc.cpp
//
//	Hexarc function tests

#include <stdio.h>

#include <windows.h>
#include "Alchemy.h"
#include "XMLUtil.h"
#include "TransData.h"

#include "Internets.h"
#include "Crypto.h"
#include "JSONUtil.h"

#define FIELD_ADVENTURE							CONSTLIT("adventure")
#define FIELD_ACTUAL							CONSTLIT("actual")
#define FIELD_ACTUAL_REQUIRED					CONSTLIT("actualRequired")
#define FIELD_AUTH_TOKEN						CONSTLIT("authToken")
#define FIELD_CHALLENGE							CONSTLIT("challenge")
#define FIELD_CHALLENGE_CREDENTIALS				CONSTLIT("challengeCredentials")
#define FIELD_CLIENT_ID							CONSTLIT("clientID")
#define FIELD_CLIENT_VERSION					CONSTLIT("clientVersion")
#define FIELD_CREDENTIALS						CONSTLIT("credentials")
#define FIELD_GAME_ID							CONSTLIT("gameID")
#define FIELD_GAME_RECORD						CONSTLIT("gameRecord")
#define FIELD_MAX_GAMES							CONSTLIT("maxGames")
#define FIELD_NEW_CREDENTIALS					CONSTLIT("newCredentials")
#define FIELD_OLD_CREDENTIALS					CONSTLIT("oldCredentials")
#define FIELD_PROTOCOL_VERSION					CONSTLIT("protocolVersion")
#define FIELD_SCOPED_AUTH_TOKEN					CONSTLIT("scopedAuthToken")
#define FIELD_SCOPED_CREDENTIALS				CONSTLIT("scopedCredentials")
#define FIELD_SIGNATURE							CONSTLIT("signature")
#define FIELD_TYPE								CONSTLIT("type")
#define FIELD_USERNAME							CONSTLIT("username")

#define FUNC_CHANGE_PASSWORD					CONSTLIT("changePassword")
#define FUNC_CONNECT							CONSTLIT("connect")
#define FUNC_HIGH_SCORE_GAMES					CONSTLIT("highScoreGames")
#define FUNC_LOGIN								CONSTLIT("login")
#define FUNC_POST_GAME							CONSTLIT("postGame")
#define FUNC_RECENT_GAMES						CONSTLIT("recentGames")
#define FUNC_REGISTER							CONSTLIT("register")
#define FUNC_REQUEST_LOGIN						CONSTLIT("requestLogin")

#define METHOD_OPTIONS							CONSTLIT("OPTIONS")
#define METHOD_POST								CONSTLIT("POST")

#define OPTION_HOST								CONSTLIT("host")

#define CLIENT_TYPE								CONSTLIT("TransData/Windows")
#define HOSTSPEC_DEFAULT						CONSTLIT("transcendence.hierosteel.com:8080")
#define ROOT_URL_DEFAULT						CONSTLIT("/api/")

const int TRANS_SERVICE_VERSION =				1;

bool Connect (const CString &sHostspec, CHTTPClientSession &Session);
bool GetJSONResponse (CHTTPMessage &Response, CJSONValue *retValue, CString *retsError);
void InitRequest (const CString &sHost, const CString &sMethod, const CString &sFunction, CHTTPMessage *retMessage);
bool ServerCommand (CHTTPClientSession &Session, const CString &sMethod, const CString &sFunc, CJSONValue &Payload, CJSONValue *retResult);

void HexarcTest (CUniverse &Universe, CXMLElement *pCmdLine)
	{
	int i, j;

	CString sHostspec = pCmdLine->GetAttribute(OPTION_HOST);
	if (sHostspec.IsBlank())
		sHostspec = HOSTSPEC_DEFAULT;

	CHTTPClientSession Session;

	if (!Connect(sHostspec, Session))
		return;

	//	Read the list of high scores

	for (j = 0; j < 5; j++)
		{
		CJSONValue Payload = CJSONValue(CJSONValue::typeObject);
		Payload.InsertHandoff(FIELD_MAX_GAMES, CJSONValue(100));
		CJSONValue Result;
		if (!ServerCommand(Session, METHOD_OPTIONS, FUNC_HIGH_SCORE_GAMES, Payload, &Result))
			{
			printf("%s\n", Result.AsString().GetASCIIZPointer());
			return;
			}

		//	For each adventure, print high score

		if (Result.GetCount() == 0)
			printf("No game records.\n");
		else
			{
			for (i = 0; i < Result.GetCount(); i++)
				{
				const CJSONValue &Record = Result.GetElement(i);
				DWORD dwAdventure = (DWORD)Record.GetElement(FIELD_ADVENTURE).AsInt32();
				if (dwAdventure == 0)
					continue;

				CGameRecord GameRecord;
				if (GameRecord.InitFromJSON(Record) != NOERROR)
					{
					printf("Unable to parse JSON record.\n");
					continue;
					}

				DWORD dwAdventureUNID = GameRecord.GetAdventureUNID();
				CString sUsername = GameRecord.GetUsername();
				int iScore = GameRecord.GetScore();

				printf("%x %s %d\n", dwAdventureUNID, sUsername.GetASCIIZPointer(), iScore);
				}
			}
		}
	}

bool Connect (const CString &sHostspec, CHTTPClientSession &Session)
	{
	CString sHost;
	CString sPort;
	urlParseHostspec(sHostspec, &sHost, &sPort);

	EInetsErrors iError = Session.Connect(sHost, sPort);
	if (iError != inetsOK)
		{
		switch (iError)
			{
			case inetsDNSError:
				printf("Unable to resolve IP address for: %s\n", sHost.GetASCIIZPointer());
				return false;

			case inetsCannotConnect:
				printf("Unable to connect to: %s\n", sHostspec.GetASCIIZPointer());
				return false;

			default:
				printf("Unknown error connecting to: %s\n", sHostspec.GetASCIIZPointer());
				return false;
			}
		}

	//	Use a constant clientID (representing TransData)

	BYTE ClientIDBytes[4] = { 0, 1, 0, 1 };
	CIntegerIP ClientID(4, ClientIDBytes);
	CString sClientID = ClientID.AsBase64();

	//	Compose a connect payload

	CJSONValue Payload(CJSONValue::typeObject);
	Payload.InsertHandoff(FIELD_CLIENT_ID, CJSONValue(sClientID));
	Payload.InsertHandoff(FIELD_CLIENT_VERSION, CJSONValue(CLIENT_TYPE));
	Payload.InsertHandoff(FIELD_PROTOCOL_VERSION, CJSONValue(TRANS_SERVICE_VERSION));

	//	Now issue the connect command

	CHTTPMessage Request;
	InitRequest(sHost, METHOD_POST, FUNC_CONNECT, &Request);
	Request.SetBody(new CJSONMessage(Payload));

	//	Send the request and wait for response

	CHTTPMessage Response;
	if (iError = Session.Send(Request, &Response))
		{
		Session.Disconnect();
		printf("Unable to send to: %s\n", sHostspec.GetASCIIZPointer());
		return false;
		}

	//	If we got a 301 redirect then we try the whole thing again

	if (Response.GetStatusCode() == 301)
		{
		Session.Disconnect();

		//	LATER: Set up the new host and recurse
		//	LATER: Keep track of recursion count, in case of ping-pong redirects
		printf("REDIRECT\n");
		return false;
		}

	//	If we get an error, return

	else if (Response.GetStatusCode() != 200)
		{
		Session.Disconnect();
		printf("[%d] %s\n", Response.GetStatusCode(), Response.GetStatusMsg().GetASCIIZPointer());
		return false;
		}

	//	Get the JSON response

	CString sError;
	CJSONValue ResponseData;
	if (!GetJSONResponse(Response, &ResponseData, &sError))
		{
		Session.Disconnect();
		printf("Unable to parse JSON reply: %s\n", sError.GetASCIIZPointer());
		return false;
		}

	//	Done

	printf("Connected to %s\n", sHostspec.GetASCIIZPointer());
	return true;
	}

bool GetJSONResponse (CHTTPMessage &Response, CJSONValue *retValue, CString *retsError)

//	GetJSONResponse
//
//	Returns the JSON payload, or false if there is an error.

	{
	//	Make sure it is the proper type

	CString sMediaType;
	if (!Response.FindHeader(CONSTLIT("Content-Type"), &sMediaType) || !strEquals(sMediaType, CONSTLIT("application/json")))
		{
		*retsError = CONSTLIT("Unknown response content type");
		return false;
		}

	//	Deserialized

	if (CJSONValue::Deserialize(Response.GetBodyBuffer(), retValue, retsError) != NOERROR)
		return false;

	//	Done

	return true;
	}

void InitRequest (const CString &sHost, const CString &sMethod, const CString &sFunction, CHTTPMessage *retMessage)

//	InitRequest
//
//	Initializes a request

	{
	retMessage->InitRequest(sMethod, strPatternSubst(CONSTLIT("%s%s"), ROOT_URL_DEFAULT, sFunction));
	retMessage->AddHeader(CONSTLIT("Host"), sHost);
	retMessage->AddHeader(CONSTLIT("User-Agent"), CONSTLIT("TransData/1.0"));
	retMessage->AddHeader(CONSTLIT("Accept-Language"), CONSTLIT("en-US"));
	}

bool ServerCommand (CHTTPClientSession &Session, const CString &sMethod, const CString &sFunc, CJSONValue &Payload, CJSONValue *retResult)

//	ServerCommand
//
//	Sends a command to the server and gets back a result. We assume that we are
//	connected.
//
//	NOTE: We clobber the Payload.

	{
	EInetsErrors iError;

	if (!Session.IsConnected())
		{
		*retResult = CJSONValue(CONSTLIT("Not connected."));
		return false;
		}

	//	Now issue the connect command

	CHTTPMessage Request;
	InitRequest(Session.GetHost(), sMethod, sFunc, &Request);
	Request.SetBody(new CJSONMessage(Payload));

	//	Send the request and wait for response

	CHTTPMessage Response;
	if (iError = Session.Send(Request, &Response))
		{
		*retResult = CJSONValue(CONSTLIT("Unable to send."));
		return false;
		}

	//	If we get an error, return

	if (Response.GetStatusCode() != 200)
		{
		*retResult = CJSONValue(Response.GetStatusMsg());
		return false;
		}

	//	Get the JSON response

	CString sError;
	if (!GetJSONResponse(Response, retResult, &sError))
		{
		*retResult = CJSONValue(strPatternSubst(CONSTLIT("Unable to parse response: %s"), sError));
		return false;
		}

	//	See if it is a server error code

	CString sErrorCode;
	CString sErrorDesc;
	if (CHexarc::IsError(*retResult, &sErrorCode, &sErrorDesc))
		{
		*retResult = CJSONValue(sErrorDesc);
		return false;
		}

	//	Done

	return true;
	}

