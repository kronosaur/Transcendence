//	CHexarc.cpp
//
//	CHexarc class
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.
//
//	INTERFACE
//
//	1.	After initializing everything, check for autoLoginUser capability. If
//		available, call AutoSignInUser

#include "stdafx.h"

#define HEXARC_TAG								CONSTLIT("Hexarc")

#define ENABLED_ATTRIB							CONSTLIT("enabled")
#define HOST_ATTRIB								CONSTLIT("host")
#define ROOT_URL_ATTRIB							CONSTLIT("rootURL")
#define USERNAME_ATTRIB							CONSTLIT("username")

#define DEFAULT_HOST							CONSTLIT("transcendence.hierosteel.com")
#define DEFAULT_URL								CONSTLIT("/api/")

#define AUTH_TYPE_SHA1							CONSTLIT("SHA1")

#define CLIENT_TYPE								CONSTLIT("Transcendence/Windows")

#define ERR_CANNOT_DIGEST_PASSWORD				CONSTLIT("Error creating a password hash.")
#define ERR_FROM_SERVER							CONSTLIT("Error response from server at %s: %s")
#define ERR_INVALID_RESPONSE					CONSTLIT("Invalid response from server at %s.")
#define ERR_INVALID_JSON						CONSTLIT("Invalid response from server at %s: %s")
#define ERR_NOT_CONNECTED						CONSTLIT("Lost connection to server.")
#define ERR_CONNECT_ERROR						CONSTLIT("No response from server at %s.")
#define ERR_REQUEST_FAILED						CONSTLIT("Unable to communicate with %s.")
#define ERR_INTERNAL							CONSTLIT("Unable to connect to %s.")
#define ERR_DNS_ERROR							CONSTLIT("Unable to resolve address for %s.")
#define ERR_CANNOT_SEND							CONSTLIT("Unable to send command to server.")
#define ERR_UNKNOWN_RESPONSE_TYPE				CONSTLIT("Unknown response content type.")

#define FIELD_ACTUAL							CONSTLIT("actual")
#define FIELD_ACTUAL_REQUIRED					CONSTLIT("actualRequired")
#define FIELD_CHALLENGE							CONSTLIT("challenge")
#define FIELD_CHALLENGE_CREDENTIALS				CONSTLIT("challengeCredentials")
#define FIELD_CLIENT_ID							CONSTLIT("clientID")
#define FIELD_CLIENT_VERSION					CONSTLIT("clientVersion")
#define FIELD_CREDENTIALS						CONSTLIT("credentials")
#define FIELD_PROTOCOL_VERSION					CONSTLIT("protocolVersion")
#define FIELD_SCOPED_AUTH_TOKEN					CONSTLIT("scopedAuthToken")
#define FIELD_SCOPED_CREDENTIALS				CONSTLIT("scopedCredentials")
#define FIELD_TYPE								CONSTLIT("type")
#define FIELD_USERNAME							CONSTLIT("username")
#define FIELD_USER_TOKEN						CONSTLIT("userToken")

#define FUNC_CONNECT							CONSTLIT("connect")
#define FUNC_LOGIN								CONSTLIT("login")
#define FUNC_REGISTER							CONSTLIT("register")
#define FUNC_REQUEST_LOGIN						CONSTLIT("requestLogin")

#define METHOD_POST								CONSTLIT("POST")

#define REGISTRY_COMPANY_NAME					CONSTLIT("Kronosaur Productions")
#define REGISTRY_PRODUCT_NAME					CONSTLIT("Transcendence")
#define REGISTRY_CLIENT_ID_OPTION				CONSTLIT("ClientID")

#define TYPE_AEON_ERROR							CONSTLIT("AEON2011:hexeError:v1")
#define TYPE_AEON_IPINTEGER						CONSTLIT("AEON2011:ipInteger:v1")

const int TRANS_SERVICE_VERSION =				1;

const DWORD MAX_CONNECTION_IDLE_TIME =			0;//3 * 60 * 1000;

bool CHexarc::Connect (CString *retsResult)

//	Connect
//
//	Connect to the server. Returns FALSE if we failed.

	{
	if (m_Session.IsConnected())
		return true;

	EInetsErrors iError = m_Session.Connect(m_sHost, m_sPort);
	if (iError != inetsOK)
		{
		switch (iError)
			{
			case inetsDNSError:
				*retsResult = strPatternSubst(ERR_DNS_ERROR, GetHostspec());
				return false;

			case inetsCannotConnect:
				*retsResult = strPatternSubst(ERR_CONNECT_ERROR, GetHostspec());
				return false;

			default:
				*retsResult = strPatternSubst(ERR_INTERNAL, GetHostspec());
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
	InitRequest(METHOD_POST, FUNC_CONNECT, &Request);
	Request.SetBody(new CJSONMessage(Payload));

	//	Send the request and wait for response

	CHTTPMessage Response;
	if (iError = m_Session.Send(Request, &Response))
		{
		m_Session.Disconnect();
		*retsResult = strPatternSubst(ERR_REQUEST_FAILED, GetHostspec());
		return false;
		}

	//	If we got a 301 redirect then we try the whole thing again

	if (Response.GetStatusCode() == 301)
		{
		m_Session.Disconnect();

		//	LATER: Set up the new host and recurse
		//	LATER: Keep track of recursion count, in case of ping-pong redirects
		*retsResult = CONSTLIT("LATER: Redirect");
		return false;
		}

	//	If we get an error, return

	else if (Response.GetStatusCode() != 200)
		{
		m_Session.Disconnect();
		*retsResult = strPatternSubst(ERR_FROM_SERVER, GetHostspec(), Response.GetStatusMsg());
		return false;
		}

	//	Get the JSON response

	CJSONValue ResponseData;
	if (!GetJSONResponse(Response, &ResponseData, retsResult))
		{
		m_Session.Disconnect();
		*retsResult = strPatternSubst(ERR_INVALID_JSON, GetHostspec(), *retsResult);
		return false;
		}

	//	Get some values

	m_bActualRequired = ResponseData.GetElement(FIELD_ACTUAL_REQUIRED).IsNotFalse();
	m_Challenge = ResponseData.GetElement(FIELD_CHALLENGE);

	//	Done

	return true;
	}

bool CHexarc::ConvertIPIntegerToString (const CJSONValue &Value, CString *retsValue)

//	ConvertIPIntegerToString
//
//	Converts an AEON CIPInteger encoded in JSON to a string buffer of raw bytes.

	{
	if (!strEquals(Value.GetElement(0).AsString(), TYPE_AEON_IPINTEGER))
		return false;

	const CString &sEncoded = Value.GetElement(1).AsString();
	CMemoryReadStream Stream(sEncoded.GetASCIIZPointer(), sEncoded.GetLength());
	if (Stream.Open() != NOERROR)
		return false;

	CBase64Decoder Decoder(&Stream);
	if (Decoder.Open() != NOERROR)
		return false;
	
	//	The first DWORD is a signature (which we can ignore)

	DWORD dwLoad;
	Decoder.Read((char *)&dwLoad, sizeof(DWORD));

	//	Next is the size

	DWORD dwSize;
	Decoder.Read((char *)&dwSize, sizeof(DWORD));

	//	Allocate enough room on the string

	char *pDest = retsValue->GetWritePointer(dwSize);

	//	Read

	Decoder.Read(pDest, dwSize);

	//	Done

	return true;
	}

bool CHexarc::ConvertToDigest (const CJSONValue &Value, CDigest *retDigest)

//	ConvertToDigest
//
//	Converts an AEON CIPInteger encoded in JSON to a CDigest object.

	{
	CIntegerIP Result;
	if (!ConvertToIntegerIP(Value, &Result))
		return false;

	//	Make sure we are the right size

	if (Result.GetLength() != 20)
		return false;

	//	Move to digest

	retDigest->TakeHandoff(Result);

	//	Done

	return true;
	}

bool CHexarc::ConvertToIntegerIP (const CJSONValue &Value, CIntegerIP *retValue)

//	ConvertToIntegerIP
//
//	Converts an AEON CIPInteger encoded in JSON to a CIntegerIP object.

	{
	if (!strEquals(Value.GetElement(0).AsString(), TYPE_AEON_IPINTEGER))
		return false;

	const CString &sEncoded = Value.GetElement(1).AsString();
	CMemoryReadStream Stream(sEncoded.GetASCIIZPointer(), sEncoded.GetLength());
	if (Stream.Open() != NOERROR)
		return false;

	CBase64Decoder Decoder(&Stream);
	if (Decoder.Open() != NOERROR)
		return false;
	
	//	The first DWORD is a signature (which we can ignore)

	DWORD dwLoad;
	Decoder.Read((char *)&dwLoad, sizeof(DWORD));

	//	Next is the size

	DWORD dwSize;
	Decoder.Read((char *)&dwSize, sizeof(DWORD));

	//	Load the bytes

	CIntegerIP Result(dwSize);
	Decoder.Read((char *)Result.GetBytes(), dwSize);

	//	Return

	retValue->TakeHandoff(Result);

	//	Done

	return true;
	}

bool CHexarc::ConvertToJSON (const CIntegerIP &Value, CJSONValue *retValue)

//	ConvertToJSON
//
//	Converts a CIntegerIP to an AEON CIPInteger encoded in JSON

	{
	//	First we generate the encoded CIPInteger

	CMemoryWriteStream Buffer;
	if (Buffer.Create() != NOERROR)
		return false;

	CBase64Encoder Encoder(&Buffer);
	if (Encoder.Create() != NOERROR)
		return false;

	//	Signature

	DWORD dwSave = 'IP1+';
	Encoder.Write((char *)&dwSave, sizeof(DWORD));

	//	Size

	dwSave = Value.GetLength();
	Encoder.Write((char *)&dwSave, sizeof(DWORD));

	//	Bytes

	Encoder.Write((char *)Value.GetBytes(), dwSave);

	//	Done

	Encoder.Close();

	//	Create the array

	CJSONValue IPInteger(CJSONValue::typeArray);
	IPInteger.InsertHandoff(CJSONValue(TYPE_AEON_IPINTEGER));
	IPInteger.InsertHandoff(CJSONValue(CString(Buffer.GetPointer(), Buffer.GetLength())));

	//	Done

	retValue->TakeHandoff(IPInteger);
	return true;
	}

CString CHexarc::GetClientVersion (void) const

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

bool CHexarc::GetJSONResponse (CHTTPMessage &Response, CJSONValue *retValue, CString *retsError)

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

CString CHexarc::GetTag (void)

//	GetTag
//
//	Returns the element tag used to save settings

	{
	return HEXARC_TAG;
	}

bool CHexarc::HasCapability (DWORD dwCapability)

//	HasCapability
//
//	Returns TRUE if the service has the given capability

	{
	switch (dwCapability)
		{
		case loginUser:
		case registerUser:
			return true;

		case autoLoginUser:
			return HasCachedCredentials();

		case cachedUser:
			return HasCachedUsername();

		case getUserProfile:
		case postGameStats:
			return IsLoggedIn();

		default:
			return false;
		}
	}

ALERROR CHexarc::Housekeeping (ITaskProcessor *pProcessor)

//	Housekeeping
//
//	Do periodic tasks. This function is called roughly once per minute.

	{
	//	If our session is connected and we haven't had any activity in a certain
	//	period of time, disconnect.

	if (m_Session.IsConnected() 
			&& m_Session.GetTicksSinceLastActivity() > MAX_CONNECTION_IDLE_TIME)
		m_Session.Disconnect();

	//	Done

	return NOERROR;
	}

ALERROR CHexarc::InitFromXML (CXMLElement *pDesc, bool *retbModified)

//	InitFromXML
//
//	Initializes from XML settings

	{
	*retbModified = false;

	if (pDesc == NULL)
		{
		SetEnabled(true);
		m_sPort = NULL_STR;
		m_sHost = DEFAULT_HOST;
		m_sRootURL = DEFAULT_URL;
		*retbModified = true;
		return NOERROR;
		}

	SetEnabled(pDesc->GetAttributeBool(ENABLED_ATTRIB));

	CString sHostspec = pDesc->GetAttribute(HOST_ATTRIB);
	strParseHostspec(sHostspec, &m_sHost, &m_sPort);
	if (m_sHost.IsBlank())
		{
		m_sPort = NULL_STR;
		m_sHost = DEFAULT_HOST;
		}

	m_sRootURL = pDesc->GetAttribute(ROOT_URL_ATTRIB);
	if (m_sRootURL.IsBlank())
		m_sRootURL = DEFAULT_URL;

	m_sUsername = pDesc->GetAttribute(USERNAME_ATTRIB);

	return NOERROR;
	}

ALERROR CHexarc::InitPrivateData (void)

//	InitPrivateData
//
//	Loads private data

	{
	//	Load the clientID from the registry.

	CRegKey Key;
	if (CRegKey::OpenUserAppKey(REGISTRY_COMPANY_NAME,
			REGISTRY_PRODUCT_NAME,
			&Key) == NOERROR)
		{
		Key.FindStringValue(REGISTRY_CLIENT_ID_OPTION, &m_sClientID);
		}

	//	If we could not find the clientID, then generate a new one.

	if (m_sClientID.IsBlank())
		{
		CIntegerIP RandomID;
		cryptoRandom(32, &RandomID);
		m_sClientID = RandomID.AsBase64();

		//	Remember to save it out.

		SetModified();
		}

	//	Done

	return NOERROR;
	}

void CHexarc::InitRequest (const CString &sMethod, const CString &sFunction, CHTTPMessage *retMessage)

//	InitRequest
//
//	Initializes a request

	{
	retMessage->InitRequest(sMethod, strPatternSubst(CONSTLIT("%s%s"), m_sRootURL, sFunction));
	retMessage->AddHeader(CONSTLIT("Host"), m_sHost);
	retMessage->AddHeader(CONSTLIT("User-Agent"), CONSTLIT("TranscendenceClient/1.0"));
	retMessage->AddHeader(CONSTLIT("Accept-Language"), CONSTLIT("en-US"));
	retMessage->AddHeader(CONSTLIT("Connection"), CONSTLIT("close"));
	}

bool CHexarc::IsError (const CJSONValue &Value, CString *retsError, CString *retsDesc)

//	IsError
//
//	Returns TRUE if the JSON value is an error

	{
	if (!strEquals(Value.GetElement(0).AsString(), TYPE_AEON_ERROR))
		return false;

	const CString &sEncoded = Value.GetElement(1).AsString();
	CMemoryReadStream Stream(sEncoded.GetASCIIZPointer(), sEncoded.GetLength());
	if (Stream.Open() != NOERROR)
		return false;

	CBase64Decoder Decoder(&Stream);
	if (Decoder.Open() != NOERROR)
		return false;

	//	Read the strings

	if (retsError)
		retsError->ReadFromStream(&Decoder);

	if (retsDesc)
		{
		if (retsError == NULL)
			{
			CString sDummy;
			sDummy.ReadFromStream(&Decoder);
			}

		retsDesc->ReadFromStream(&Decoder);
		}

	//	Done

	return true;
	}

ALERROR CHexarc::PostGameStats (ITaskProcessor *pProcessor, const CGameStats &Stats, CString *retsResult)

//	PostGameStats
//
//	Posts game stats to the cloud

	{
//	EInetsErrors iError;

	pProcessor->SetProgress(CONSTLIT("Posting statistics to Transcendence Multiverse..."));

	//	Generate a stats text buffer

	CMemoryWriteStream StatsBuffer;
	if (StatsBuffer.Create() != NOERROR)
		{
		*retsResult = CONSTLIT("Unable to post stats: Out of memory.");
		return inetsOutOfMemory;
		}

	if (Stats.WriteAsText(&StatsBuffer) != NOERROR)
		{
		*retsResult = CONSTLIT("Unable to post stats: Out of memory.");
		return inetsOutOfMemory;
		}

	//	Compose the HTML form

#ifdef LATER
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
			*retsResult = strPatternSubst(CONSTLIT("Unable to find http://%s."), GetHostspec());
		else
			*retsResult = strPatternSubst(CONSTLIT("Unable to connect to http://%s."), GetHostspec());
		return ERR_FAIL;
		}

	//	Send the request and wait for response

	CHTTPMessage Response;
	if (iError = Session.Send(Request, &Response))
		{
		*retsResult = strPatternSubst(CONSTLIT("Unable to upload to http://%s/%s."), GetHostspec(), m_sPostStatsURL);
		return ERR_FAIL;
		}

	//	Done

	Session.Disconnect();
	*retsResult = CONSTLIT("Game statistics posted to Xelerus.");
#endif

	return NOERROR;
	}

ALERROR CHexarc::ReadProfile (ITaskProcessor *pProcessor, CUserProfile *retProfile, CString *retsResult)

//	ReadProfile
//
//	Reads the user profile from the server

	{
	::Sleep(1000);

	retProfile->Init(m_sUsername);

	CAdventureRecord &Record1 = retProfile->InsertAdventureRecord(0x00009301, 10);

	CGameRecord &Game1 = Record1.GetRecordAt(CAdventureRecord::personalBest);
	Game1.SetUsername(CONSTLIT("Saint Leibowitz"));
	Game1.SetScore(12300);
	Game1.SetPlayerName(CONSTLIT("Loki"));
	Game1.SetPlayerGenome(genomeHumanMale);
	Game1.SetShipClass(0x00003801);
	Game1.SetEndGameEpitaph(CONSTLIT("killed by a really nasty monster"));
	Game1.SetResurrectCount(1);

	CGameRecord &Game2 = Record1.GetRecordAt(CAdventureRecord::mostRecent);
	Game2.SetUsername(CONSTLIT("Saint Leibowitz"));
	Game2.SetScore(2200);
	Game2.SetPlayerName(CONSTLIT("Loki"));
	Game2.SetPlayerGenome(genomeHumanFemale);
	Game2.SetShipClass(0x00003802);
	Game2.SetEndGameEpitaph(CONSTLIT("killed by flying around in circles and then exploding suddenly"));
	Game2.SetResurrectCount(0);

	CAdventureRecord &Record2 = retProfile->InsertAdventureRecord(0x00300001, 10);

	CGameRecord &Game3 = Record2.GetRecordAt(CAdventureRecord::personalBest);
	Game3.SetUsername(CONSTLIT("Saint Leibowitz"));
	Game3.SetScore(12300);
	Game3.SetPlayerName(CONSTLIT("Loki"));
	Game3.SetPlayerGenome(genomeHumanMale);
	Game3.SetShipClass(0x00003801);
	Game3.SetEndGameEpitaph(CONSTLIT("killed by a really nasty monster"));
	Game3.SetResurrectCount(1);

	CGameRecord &Game4 = Record2.GetRecordAt(CAdventureRecord::mostRecent);
	Game4.SetUsername(CONSTLIT("Saint Leibowitz"));
	Game4.SetScore(2200);
	Game4.SetPlayerName(CONSTLIT("Loki"));
	Game4.SetPlayerGenome(genomeHumanFemale);
	Game4.SetShipClass(0x00003802);
	Game4.SetEndGameEpitaph(CONSTLIT("killed by flying around in circles and then exploding suddenly"));
	Game4.SetResurrectCount(0);

	//	Done

	return NOERROR;
	}

ALERROR CHexarc::RegisterUser (ITaskProcessor *pProcessor, const CString &sUsername, const CString &sPassword, CString *retsResult)

//	RegisterUser
//
//	Register a new user

	{
	//	Connect to the host

	if (!Connect(retsResult))
		return ERR_FAIL;

	//	Generate a hash of the username and password

	CJSONValue PasswordHash;
	if (!ConvertToJSON(CDigest(CBufferReadBlock(strPatternSubst("%s:%s", sUsername, sPassword))), &PasswordHash))
		{
		*retsResult = ERR_CANNOT_DIGEST_PASSWORD;
		return ERR_FAIL;
		}

	//	Put together a payload for registration

	CJSONValue Result;
	CJSONValue Payload(CJSONValue::typeObject);
	Payload.InsertHandoff(FIELD_USERNAME, CJSONValue(sUsername));
	Payload.InsertHandoff(FIELD_CREDENTIALS, PasswordHash);
	if (!ServerCommand(FUNC_REGISTER, Payload, &Result))
		{
		*retsResult = Result.AsString();
		return ERR_FAIL;
		}

	//	If successful, then we initialize our data

	m_sUsername = sUsername;
	m_Credentials = Result.GetElement(FIELD_SCOPED_CREDENTIALS);
	m_UserToken = Result.GetElement(FIELD_SCOPED_AUTH_TOKEN);
	SetModified();

	return NOERROR;
	}

ALERROR CHexarc::SignInUser (ITaskProcessor *pProcessor, const CString &sUsername, const CString &sPassword, CString *retsResult)

//	SignInUser
//
//	Signs in a user

	{
	//	Connect to the host

	if (!Connect(retsResult))
		return ERR_FAIL;

	//	Ask for the challenge

	CJSONValue Result;
	CJSONValue GetChallenge(CJSONValue::typeObject);
	GetChallenge.InsertHandoff(FIELD_USERNAME, CJSONValue(sUsername));
	if (!ServerCommand(FUNC_REQUEST_LOGIN, GetChallenge, &Result))
		{
		*retsResult = Result.AsString();
		return ERR_FAIL;
		}

	//	Convert the challenge to a string

	CString sChallenge;
	if (!ConvertIPIntegerToString(Result, &sChallenge))
		{
		*retsResult = strPatternSubst(ERR_INVALID_RESPONSE, GetHostspec());
		return ERR_FAIL;
		}

	//	Hash the password

	CDigest PasswordHash(CBufferReadBlock(strPatternSubst("%s:%s", sUsername, sPassword)));
	CString sPasswordHash((char *)PasswordHash.GetBytes(), PasswordHash.GetLength());

	//	Generate the proper response

	CJSONValue ChallengeCredentials;
	if (!ConvertToJSON(CDigest(CBufferReadBlock(strPatternSubst("%s:%s", sPasswordHash, sChallenge))), &ChallengeCredentials))
		{
		*retsResult = ERR_CANNOT_DIGEST_PASSWORD;
		return ERR_FAIL;
		}

	//	Login

	CJSONValue Login(CJSONValue::typeObject);
	Login.InsertHandoff(FIELD_ACTUAL, CJSONValue(CJSONValue::typeTrue));
	Login.InsertHandoff(FIELD_USERNAME, CJSONValue(sUsername));
	Login.InsertHandoff(FIELD_CHALLENGE_CREDENTIALS, ChallengeCredentials);
	if (!ServerCommand(FUNC_LOGIN, Login, &Result))
		{
		*retsResult = Result.AsString();
		return ERR_FAIL;
		}

	//	We get back an auth token

	m_sUsername = sUsername;
	m_UserToken = Result;
	SetModified();

	return NOERROR;
	}

bool CHexarc::ServerCommand (const CString &sFunc, CJSONValue &Payload, CJSONValue *retResult)

//	ServerCommand
//
//	Sends a command to the server and gets back a result. We assume that we are
//	connected.
//
//	NOTE: We clobber the Payload.

	{
	EInetsErrors iError;

	if (!m_Session.IsConnected())
		{
		*retResult = CJSONValue(ERR_NOT_CONNECTED);
		return false;
		}

	//	Now issue the connect command

	CHTTPMessage Request;
	InitRequest(METHOD_POST, sFunc, &Request);
	Request.SetBody(new CJSONMessage(Payload));

	//	Send the request and wait for response

	CHTTPMessage Response;
	if (iError = m_Session.Send(Request, &Response))
		{
		*retResult = CJSONValue(ERR_CANNOT_SEND);
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
		*retResult = CJSONValue(strPatternSubst(ERR_INVALID_JSON, GetHostspec(), sError));
		return false;
		}

	//	See if it is a server error code

	CString sErrorCode;
	CString sErrorDesc;
	if (IsError(*retResult, &sErrorCode, &sErrorDesc))
		{
		*retResult = CJSONValue(sErrorDesc);
		return false;
		}

	//	Done

	return true;
	}

ALERROR CHexarc::WriteAsXML (IWriteStream *pOutput)

//	WriteAsXML
//
//	Writes settings to XML file

	{
	ALERROR error;

	CString sData = strPatternSubst(CONSTLIT(
				"\t\t<Hexarc enabled=\"%s\"\r\n"
				"\t\t\tusername=\"%s\"\r\n"
				"\t\t\thost=\"%s\"\r\n"
				"\t\t\trootURL=\"%s\"\r\n"
				"\t\t\t/>\r\n"), 
			(IsEnabled() ? CONSTLIT("true") : CONSTLIT("false")),
			m_sUsername,
			GetHostspec(),
			m_sRootURL
			);
	if (error = pOutput->Write(sData.GetPointer(), sData.GetLength(), NULL))
		return error;

	return NOERROR;
	}

ALERROR CHexarc::WritePrivateData (void)

//	WritePrivateData
//
//	Writes private data.

	{
	ALERROR error;

	//	Write the clientID to the registry.

	CRegKey Key;
	if (error = CRegKey::OpenUserAppKey(REGISTRY_COMPANY_NAME,
			REGISTRY_PRODUCT_NAME,
			&Key))
		return error;

	//	Save

	Key.SetStringValue(REGISTRY_CLIENT_ID_OPTION, m_sClientID);

	//	Done

	return NOERROR;
	}
