//	CGameRecord.cpp
//
//	CGameRecord class
//	Copyright (c) 2012 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"

#define SCORE_ATTRIB						CONSTLIT("score")
#define NAME_ATTRIB							CONSTLIT("name")
#define EPITAPH_ATTRIB						CONSTLIT("epitaph")
#define TIME_ATTRIB							CONSTLIT("time")
#define RESURRECT_COUNT_ATTRIB				CONSTLIT("resurrectCount")
#define GENOME_ATTRIB						CONSTLIT("genome")
#define DEBUG_ATTRIB						CONSTLIT("debug")
#define SHIP_CLASS_ATTRIB					CONSTLIT("shipClass")
#define ADVENTURE_ATTRIB					CONSTLIT("adventureUNID")
#define ADVENTURE_NAME_ATTRIB				CONSTLIT("adventureName")

#define FIELD_ADVENTURE						CONSTLIT("adventure")
#define FIELD_ADVENTURE_ID					CONSTLIT("adventureID")
#define FIELD_CHARACTER_GENOME				CONSTLIT("characterGenome")
#define FIELD_CHARACTER_NAME				CONSTLIT("characterName")
#define FIELD_DEBUG							CONSTLIT("debug")
#define FIELD_DURATION						CONSTLIT("duration")
#define FIELD_END_GAME						CONSTLIT("endGame")
#define FIELD_EPITAPH						CONSTLIT("epitaph")
#define FIELD_EXTENSIONS					CONSTLIT("extensions")
#define FIELD_GAME_ID						CONSTLIT("gameID")
#define FIELD_LEAGUE						CONSTLIT("league")
#define FIELD_RESURRECT_COUNT				CONSTLIT("resurrectCount")
#define FIELD_SCORE							CONSTLIT("score")
#define FIELD_SHIP_CLASS					CONSTLIT("shipClass")
#define FIELD_SHIP_CLASS_NAME				CONSTLIT("shipClassName")
#define FIELD_SYSTEM						CONSTLIT("system")
#define FIELD_SYSTEM_NAME					CONSTLIT("systemName")
#define FIELD_USERNAME						CONSTLIT("username")

CGameRecord::CGameRecord (void) :
		m_dwAdventure(0),
		m_iGenome(genomeUnknown),
		m_dwShipClass(0),
		m_bDebugGame(false),
		m_iScore(0),
		m_iResurrectCount(0)

//	CGameRecord constructor

	{
	}

CString CGameRecord::GetAdventureID (void) const

//	GetAdventureID
//
//	Returns a unique string for the adventure and all included extensions.

	{
	int i;
	CMemoryWriteStream Buffer;
	if (Buffer.Create() != NOERROR)
		return NULL_STR;

	CString sUNID = strPatternSubst(CONSTLIT("%08x"), m_dwAdventure);
	Buffer.Write(sUNID.GetASCIIZPointer(), sUNID.GetLength());

	for (i = 0; i < m_Extensions.GetCount(); i++)
		{
		sUNID = strPatternSubst(CONSTLIT("-%08x"), m_Extensions[i]);
		Buffer.Write(sUNID.GetASCIIZPointer(), sUNID.GetLength());
		}

	return CString(Buffer.GetPointer(), Buffer.GetLength());
	}

CString CGameRecord::GetDescription (DWORD dwParts) const

//	GetDescription
//
//	Returns a description of the record

	{
	CMemoryWriteStream Output(4096);
	if (Output.Create() != NOERROR)
		return NULL_STR;

	//	Adjust depending on what data we've got

	if (m_sName.IsBlank())
		dwParts &= ~descCharacter;

	if (m_sShipClass.IsBlank())
		dwParts &= ~descShip;

	if (m_sEpitaph.IsBlank())
		dwParts &= ~descEpitaph;

	if (m_Duration.Milliseconds() == 0)
		dwParts &= ~descPlayTime;

	if (m_iResurrectCount == 0)
		dwParts &= ~descResurrectCount;

	//	The first line shows the character and ship class

	bool bNewLineNeeded = false;
	if (dwParts & (descCharacter | descShip))
		{
		bool bDashNeeded = false;

		if (dwParts & descCharacter)
			{
			CString sLine = strPatternSubst(CONSTLIT("%s — %s"), m_sName, GetGenomeName(m_iGenome));
			Output.Write(sLine.GetASCIIZPointer(), sLine.GetLength());

			bDashNeeded = true;
			}

		if (dwParts & descShip)
			{
			if (bDashNeeded)
				Output.Write(" — ", 3);

			Output.Write(m_sShipClass.GetASCIIZPointer(), m_sShipClass.GetLength());
			}

		bNewLineNeeded = true;
		}

	//	Next line is the epitaph

	if (dwParts & descEpitaph)
		{
		if (bNewLineNeeded)
			Output.Write("\n", 1);

		CString sLine = strCapitalize(GetEndGameEpitaph(FLAG_OMIT_WAS));
		Output.Write(sLine.GetASCIIZPointer(), sLine.GetLength());

		bNewLineNeeded = true;
		}

	//	Next line is the play time

	bool bSpaceNeeded = false;
	if (dwParts & descPlayTime)
		{
		if (bNewLineNeeded)
			{
			Output.Write("\n", 1);
			bNewLineNeeded = false;
			}

		Output.Write("Played for ", 11);
		CString sTime = m_Duration.Format(NULL_STR);
		Output.Write(sTime.GetASCIIZPointer(), sTime.GetLength());

		bSpaceNeeded = true;
		}

	//	Write the resurrection count

	if (dwParts & descResurrectCount)
		{
		if (bNewLineNeeded)
			Output.Write("\n", 1);

		if (bSpaceNeeded)
			{
			Output.Write(" ", 1);
			bSpaceNeeded = false;
			}

		if (m_iResurrectCount == 1)
			Output.Write("(resurrected once)", 18);
		else
			{
			CString sLine = strPatternSubst(CONSTLIT("(resurrected %d times)"), m_iResurrectCount);
			Output.Write(sLine.GetASCIIZPointer(), sLine.GetLength());
			}

		bNewLineNeeded = true;
		}

	//	Done

	return CString(Output.GetPointer(), Output.GetLength());
	}

CString CGameRecord::GetEndGameEpitaph (DWORD dwFlags) const

//	GetEndGameEpitaph
//
//	Returns the epitaph
	
	{
	if (dwFlags & FLAG_OMIT_WAS)
		{
		if (strStartsWith(m_sEpitaph, CONSTLIT("was")))
			return strSubString(m_sEpitaph, 4, -1);
		else
			return m_sEpitaph;
		}
	else
		return m_sEpitaph; 
	}

ALERROR CGameRecord::InitFromJSON (const CJSONValue &Value)

//	InitFromJSON
//
//	Initializes from a JSON structure.

	{
	int i;

	m_sUsername = Value.GetElement(FIELD_USERNAME).AsString();
	m_sGameID = Value.GetElement(FIELD_GAME_ID).AsString();
	m_dwAdventure = (DWORD)Value.GetElement(FIELD_ADVENTURE).AsInt32();

	const CJSONValue &Extensions = Value.GetElement(FIELD_EXTENSIONS);
	m_Extensions.DeleteAll();
	m_Extensions.InsertEmpty(Extensions.GetCount());
	for (i = 0; i < Extensions.GetCount(); i++)
		m_Extensions[i] = Extensions.GetElement(i).AsInt32();

	m_sName = Value.GetElement(FIELD_CHARACTER_NAME).AsString();
	m_iGenome = ParseGenomeID(Value.GetElement(FIELD_CHARACTER_GENOME).AsString());

	m_dwShipClass = (DWORD)Value.GetElement(FIELD_SHIP_CLASS).AsInt32();
	m_sShipClass = Value.GetElement(FIELD_SHIP_CLASS_NAME).AsString();
	m_sSystem = Value.GetElement(FIELD_SYSTEM).AsString();
	m_sSystemName = Value.GetElement(FIELD_SYSTEM_NAME).AsString();

	m_Duration = CTimeSpan(Value.GetElement(FIELD_DURATION).AsInt32() * 1000);
	m_bDebugGame = Value.GetElement(FIELD_DEBUG).IsNotFalse();

	m_iScore = Value.GetElement(FIELD_SCORE).AsInt32();
	m_iResurrectCount = Value.GetElement(FIELD_RESURRECT_COUNT).AsInt32();

	m_sEndGameReason = Value.GetElement(FIELD_END_GAME).AsString();
	m_sEpitaph = Value.GetElement(FIELD_EPITAPH).AsString();

	return NOERROR;
	}

ALERROR CGameRecord::InitFromXML (CXMLElement *pDesc)

//	InitFromXML
//
//	Initialize the record from an XML entry.
//	NOTE: Not all fields are saved in XML.

	{
	m_sGameID = NULL_STR;
	m_sName = pDesc->GetAttribute(NAME_ATTRIB);
	m_sShipClass = pDesc->GetAttribute(SHIP_CLASS_ATTRIB);
	m_iGenome = LoadGenome(pDesc->GetAttribute(GENOME_ATTRIB));
	m_bDebugGame = (pDesc->GetAttributeInteger(DEBUG_ATTRIB) ? true : false);
	m_dwAdventure = pDesc->GetAttributeInteger(ADVENTURE_ATTRIB);
	if (m_dwAdventure == 0)
		m_dwAdventure = DEFAULT_ADVENTURE_EXTENSION_UNID;
	if (!CTimeSpan::Parse(pDesc->GetAttribute(TIME_ATTRIB), &m_Duration))
		m_Duration = CTimeSpan();
	m_iScore = pDesc->GetAttributeInteger(SCORE_ATTRIB);
	m_iResurrectCount = pDesc->GetAttributeInteger(RESURRECT_COUNT_ATTRIB);
	m_sEndGameReason = NULL_STR;
	m_sEpitaph = pDesc->GetAttribute(EPITAPH_ATTRIB);

	return NOERROR;
	}

GenomeTypes CGameRecord::LoadGenome (const CString &sAttrib)

//	LoadGenome
//
//	Load genome value

	{
	int iGenome = strToInt(sAttrib, genomeUnknown, NULL);

	switch (iGenome)
		{
		case genomeHumanMale:
		case genomeHumanFemale:
			return (GenomeTypes)iGenome;

		default:
			return genomeUnknown;
		}
	}

void CGameRecord::SaveToJSON (CJSONValue *retOutput) const

//	SaveToJSON
//
//	Save a record to a JSON value

	{
	int i;

	*retOutput = CJSONValue(CJSONValue::typeObject);

	retOutput->InsertHandoff(FIELD_USERNAME, CJSONValue(m_sUsername));
	retOutput->InsertHandoff(FIELD_GAME_ID, CJSONValue(m_sGameID));
	retOutput->InsertHandoff(FIELD_ADVENTURE, CJSONValue((int)m_dwAdventure));
	retOutput->InsertHandoff(FIELD_ADVENTURE_ID, CJSONValue(GetAdventureID()));

	if (IsRegistered())
		retOutput->InsertHandoff(FIELD_LEAGUE, CJSONValue(CONSTLIT("registered")));
	else
		retOutput->InsertHandoff(FIELD_LEAGUE, CJSONValue(CONSTLIT("unregistered")));

	CJSONValue Extensions(CJSONValue::typeArray);
	for (i = 0; i < m_Extensions.GetCount(); i++)
		Extensions.Insert(CJSONValue((int)m_Extensions[i]));
	retOutput->InsertHandoff(FIELD_EXTENSIONS, Extensions);

	retOutput->InsertHandoff(FIELD_CHARACTER_NAME, CJSONValue(m_sName));
	retOutput->InsertHandoff(FIELD_CHARACTER_GENOME, CJSONValue(GetGenomeID(m_iGenome)));

	retOutput->InsertHandoff(FIELD_SHIP_CLASS, CJSONValue((int)m_dwShipClass));
	retOutput->InsertHandoff(FIELD_SHIP_CLASS_NAME, CJSONValue(m_sShipClass));
	retOutput->InsertHandoff(FIELD_SYSTEM, CJSONValue(m_sSystem));
	retOutput->InsertHandoff(FIELD_SYSTEM_NAME, CJSONValue(m_sSystemName));

	retOutput->InsertHandoff(FIELD_DURATION, CJSONValue((int)m_Duration.Seconds()));
	if (m_bDebugGame)
		retOutput->InsertHandoff(FIELD_DEBUG, CJSONValue(CJSONValue::typeTrue));

	retOutput->InsertHandoff(FIELD_SCORE, CJSONValue(m_iScore));
	retOutput->InsertHandoff(FIELD_RESURRECT_COUNT, CJSONValue(m_iResurrectCount));

	retOutput->InsertHandoff(FIELD_END_GAME, CJSONValue(m_sEndGameReason));
	retOutput->InsertHandoff(FIELD_EPITAPH, CJSONValue(m_sEpitaph));
	}

void CGameRecord::SetShipClass (DWORD dwUNID)

//	SetShipClass
//
//	Sets the ship class being used by the player

	{
	m_dwShipClass = dwUNID;

	CShipClass *pClass = g_pUniverse->FindShipClass(dwUNID);
	if (pClass)
		m_sShipClass = pClass->GetName();
	else
		m_sShipClass = CONSTLIT("Unknown");
	}

void CGameRecord::SetSystem (CSystem *pSystem)

//	SetSystem
//
//	Sets the current system

	{
	if (pSystem)
		{
		CTopologyNode *pNode = pSystem->GetTopology();
		m_sSystem = (pNode ? pNode->GetID() : NULL_STR);
		m_sSystemName = pSystem->GetName();
		}
	else
		{
		m_sSystem = NULL_STR;
		m_sSystemName = NULL_STR;
		}
	}

ALERROR CGameRecord::WriteToXML (IWriteStream &Stream)

//	WriteToXML
//
//	Writes the record to an XML entry

	{
	ALERROR error;

	CString sData = strPatternSubst(CONSTLIT("\t<Score name=\"%s\"\tshipClass=\"%s\"\tgenome=\"%d\"\tscore=\"%d\"\tepitaph=\"%s\"\ttime=\"%s\"\tresurrectCount=\"%d\"\tadventureUNID=\"%x\"\tdebug=\"%d\"/>\r\n"),
			CXMLElement::MakeAttribute(m_sName),
			CXMLElement::MakeAttribute(m_sShipClass),
			m_iGenome,
			m_iScore,
			CXMLElement::MakeAttribute(m_sEpitaph),
			m_Duration.Encode(),
			m_iResurrectCount,
			m_dwAdventure,
			(m_bDebugGame ? 1 : 0));

	if (error = Stream.Write(sData.GetPointer(), sData.GetLength(), NULL))
		return error;

	//	Done

	return NOERROR;
	}

