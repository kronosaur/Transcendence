//	CGameFile.cpp
//
//	CGameFile class
//	Copyright (c) 2012 by Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

#define MIN_GAME_FILE_VERSION					5
#define GAME_FILE_VERSION						8

CGameFile::CGameFile (void) : 
		m_pFile(NULL),
		m_iRefCount(0),
		m_SystemMap(FALSE, TRUE)

//	CGameFile constructor

	{
	}

CGameFile::~CGameFile (void)

//	CGameFile destructor

	{
	if (m_iRefCount)
		{
		m_iRefCount = 1;
		Close();
		}
	}

void CGameFile::Close (void)

//	Close
//
//	Close the game file

	{
	ASSERT(m_iRefCount > 0);

	if (--m_iRefCount == 0)
		{
		ASSERT(m_pFile);

		m_pFile->Close();
		delete m_pFile;
		m_pFile = NULL;
		}
	}

ALERROR CGameFile::Create (const CString &sFilename, const CString &sUsername)

//	Create
//
//	Create the game file

	{
	ALERROR error;

	ASSERT(m_pFile == NULL);
	ASSERT(m_iRefCount == 0);

	//	Create and open the file

	if (error = CDataFile::Create(sFilename, 4096, 8))
		return error;

	m_pFile = new CDataFile(sFilename);
	if (m_pFile == NULL)
		return ERR_MEMORY;

	if (error = m_pFile->Open())
		{
		delete m_pFile;
		m_pFile = NULL;
		return error;
		}

	//	Universe not yet saved

	::ZeroMemory(&m_Header, sizeof(m_Header));

	m_Header.dwVersion = GAME_FILE_VERSION;
	m_Header.dwCreateVersion = fileGetProductVersion();
	m_Header.dwUniverse = INVALID_ENTRY;
	m_Header.dwGameStats = INVALID_ENTRY;
	m_Header.dwResurrectCount = 0;
	m_Header.dwFlags = 0;

	//	If we have a username then we treat this as a regulation game.

	if (!sUsername.IsBlank())
		{
		lstrcpyn(m_Header.szUsername, sUsername.GetASCIIZPointer(), sizeof(m_Header.szUsername));

		//	Generate a game ID

		CIntegerIP RandomID;
		cryptoRandom(32, &RandomID);
		CString sGameID = RandomID.AsBase64();
		utlMemCopy(sGameID.GetASCIIZPointer(), m_Header.szGameID, sGameID.GetLength() + 1);
		}

	//	Save out an empty system map

	m_SystemMap.RemoveAll();
	CString sData;
	SaveSystemMapToStream(&sData);

	if (error = m_pFile->AddEntry(sData, (int *)&m_Header.dwSystemMap))
		{
		delete m_pFile;
		m_pFile = NULL;
		return error;
		}

	//	Save out the game header information

	sData = CString((char *)&m_Header, sizeof(m_Header));
	if (error = m_pFile->AddEntry(sData, &m_iHeaderID))
		{
		delete m_pFile;
		m_pFile = NULL;
		return error;
		}

	m_pFile->SetDefaultEntry(m_iHeaderID);
	m_pFile->Flush();

	//	Done

	m_iRefCount = 1;

	return NOERROR;
	}

CString CGameFile::GenerateFilename (const CString &sName)

//	GenerateFilename
//
//	Generates a filename from a name

	{
	CString sFilespec;

	do
		{
		sFilespec = strPatternSubst(CONSTLIT("%s-%04d.sav"), sName, mathRandom(1, 9999));
		pathValidateFilename(sFilespec, &sFilespec);
		}
	while (pathExists(sFilespec));

	return sFilespec;
	}

CString CGameFile::GetPlayerName (void) const

//	GetPlayerName
//
//	Returns the player name
	
	{
	CString sName = CString((char *)m_Header.szPlayerName);

	//	If we don't have a player name saved, then get it from the filename

	if (sName.IsBlank())
		sName = pathStripExtension(pathGetFilename(m_pFile->GetFilename()));

	return sName;
	}

CString CGameFile::GetSystemName (void) const

//	GetSystemName
//
//	Returns the system name where we saved.
	
	{
	CString sName = CString((char *)m_Header.szSystemName);

	//	Default to something if blank

	if (sName.IsBlank())
		sName = CONSTLIT("Unknown");

	return sName;
	}

ALERROR CGameFile::LoadGameHeader (SGameHeader *retHeader)

//	LoadGameHeader
//
//	Load the game header

	{
	ALERROR error;

	CString sHeader;
	if (error = m_pFile->ReadEntry(m_iHeaderID, &sHeader))
		return error;

	//	Read a previous version

	if (sHeader.GetLength() == sizeof(SGameHeader8))
		{
		utlMemCopy(sHeader.GetPointer(), (char *)retHeader, sizeof(SGameHeader8));

		//	Initialize additional data

		retHeader->szUsername[0] = '\0';
		retHeader->szGameID[0] = '\0';
		retHeader->dwAdventure = 0;
		retHeader->szPlayerName[0] = '\0';
		retHeader->dwGenome = 0;
		retHeader->dwPlayerShip = 0;
		retHeader->dwScore = 0;
		retHeader->szEpitaph[0] = '\0';
		}

	//	Read current version

	else
		{
		if (sHeader.GetLength() < sizeof(SGameHeader))
			return ERR_FAIL;

		utlMemCopy(sHeader.GetPointer(), (char *)retHeader, sizeof(SGameHeader));
		}

	return NOERROR;
	}

ALERROR CGameFile::LoadGameStats (CGameStats *retStats)

//	LoadGameStats
//
//	Loads game stats

	{
	ALERROR error;

	if (m_Header.dwGameStats == 0 || m_Header.dwGameStats == INVALID_ENTRY)
		return ERR_NOTFOUND;

	CString sData;
	if (error = m_pFile->ReadEntry(m_Header.dwGameStats, &sData))
		return error;

	CMemoryReadStream Stream(sData.GetPointer(), sData.GetLength());
	if (error = Stream.Open())
		return error;

	if (error = retStats->LoadFromStream(&Stream))
		return error;

	if (error = Stream.Close())
		return error;

	return NOERROR;
	}

ALERROR CGameFile::LoadSystem (DWORD dwUNID, 
							   CSystem **retpSystem, 
							   DWORD dwObjID,
							   CSpaceObject **retpObj,
							   CSpaceObject *pPlayerShip)

//	LoadSystem
//
//	Load a star system from the game file

	{
	ALERROR error;

	ASSERT(m_pFile);

	//	Get the entry where this system is stored. If we can't find it,
	//	then this must be a new system.

	DWORD dwEntry;
	if (m_SystemMap.Lookup(dwUNID, (CObject **)&dwEntry) != NOERROR)
		{
		kernelDebugLogMessage("Unable to find system ID: %x", dwUNID);
		return ERR_FAIL;
		}

	//	If the IN_STARGATE flag is set then it means that we crashed after we saved
	//	the system but before we could save the new system.

	if (m_Header.dwFlags & GAME_FLAG_IN_STARGATE)
		{
		kernelDebugLogMessage("Recovering from corrupt save file system: %x", dwEntry);

		//	Read the version history. We should have a previous version

		TArray<CDataFile::SVersionInfo> History;
		if (error = m_pFile->ReadHistory(dwEntry, &History))
			{
			kernelDebugLogMessage("Unable to read entry history: %x", dwEntry);
			return error;
			}

		//	If we have a previous version, delete the current one and clear the
		//	flag.

		if (History.GetCount() > 1)
			{
			if (error = m_pFile->DeleteEntry(dwEntry))
				{
				kernelDebugLogMessage("Unable to delete entry: %x", dwEntry);
				return error;
				}

			//	Clear the flag now that we have recovered

			m_Header.dwFlags &= ~GAME_FLAG_IN_STARGATE;
			if (error = SaveGameHeader(m_Header))
				{
				kernelDebugLogMessage("Unable to save header");
				return error;
				}
			}
		}

	//	Read the system data

	CString sData;
	if (error = m_pFile->ReadEntry(dwEntry, &sData))
		{
		kernelDebugLogMessage("Unable to read system data entry: %x", dwEntry);
		return error;
		}

	//	Convert to a stream

	CMemoryReadStream Stream(sData.GetPointer(), sData.GetLength());
	if (error = Stream.Open())
		{
		kernelDebugLogMessage("Unable to open data stream for system entry: %x", dwEntry);
		return error;
		}

	//	Load the system from the stream

	if (error = CSystem::CreateFromStream(g_pUniverse, 
			&Stream, 
			retpSystem,
			dwObjID,
			retpObj,
			pPlayerShip))
		{
		Stream.Close();
		kernelDebugLogMessage("Unable to load system: %x", dwEntry);
		return error;
		}

	//	Tell the universe

	if (error = g_pUniverse->AddStarSystem((*retpSystem)->GetTopology(), *retpSystem))
		{
		delete *retpSystem;
		Stream.Close();
		kernelDebugLogMessage("Unable to add system to topology: %x", dwEntry);
		return error;
		}

	//	Done

	if (error = Stream.Close())
		return error;

	return NOERROR;
	}

void CGameFile::LoadSystemMapFromStream (DWORD dwVersion, const CString &sStream)

//	LoadSystemMapFromStream
//
//	Loads the system map

	{
	int i;
	DWORD *pPos = (DWORD *)sStream.GetASCIIZPointer();
	DWORD *pPosEnd = pPos + (sStream.GetLength() / sizeof(DWORD));
	bool bLoadV6;

	//	Because of a bug in version 7 we don't actually know whether
	//	version 6 is the new version or the old version. Try to
	//	heuristically determine it here.

	int iCount = (int)*pPos++;

	if (dwVersion < 7)
		bLoadV6 = (iCount != ((sStream.GetLength() / sizeof(DWORD)) - 1) / 2);
	else
		bLoadV6 = false;

	//	For the older version, we need to do some processing

	if (bLoadV6)
		{
		m_SystemMap.RemoveAll();
		for (i = 0; i < iCount && pPos < pPosEnd; i++)
			{
			DWORD dwValue = *pPos++;
			if (dwValue)
				m_SystemMap.AddEntry(i + 1, (CObject *)dwValue);
			}
		}

	//	The newer version stores a compact ID table

	else
		{
		m_SystemMap.RemoveAll();
		for (i = 0; i < iCount && pPos < pPosEnd; i++)
			{
			DWORD dwKey = *pPos++;
			DWORD dwValue = *pPos++;

			m_SystemMap.AddEntry(dwKey, (CObject *)dwValue);
			}
		}
	}

ALERROR CGameFile::LoadUniverse (CUniverse &Univ, DWORD *retdwSystemID, DWORD *retdwPlayerID, CString *retsError)

//	LoadUniverse
//
//	Load the universe

	{
	ALERROR error;

	ASSERT(m_pFile);
	if (m_Header.dwUniverse == INVALID_ENTRY)
		{
		*retsError = CONSTLIT("Invalid save file: can't find universe entry.");
		return ERR_FAIL;
		}

	//	Read the data

	CString sData;
	if (error = m_pFile->ReadEntry(m_Header.dwUniverse, &sData))
		{
		*retsError = CONSTLIT("Invalid save file: error loading universe entry.");
		return error;
		}

	//	Convert to a stream

	CMemoryReadStream Stream(sData.GetPointer(), sData.GetLength());
	if (error = Stream.Open())
		{
		*retsError = CONSTLIT("Invalid save file: unable to open universe stream.");
		return error;
		}

	//	Set debug (we need to do this before we load)

	Univ.SetDebugMode((m_Header.dwFlags & GAME_FLAG_DEBUG) ? true : false);

	//	Load the universe from the stream

	if (error = Univ.LoadFromStream(&Stream, retdwSystemID, retdwPlayerID, retsError))
		{
		Stream.Close();
		return error;
		}

	//	Done

	Stream.Close();

	return NOERROR;
	}

ALERROR CGameFile::Open (const CString &sFilename)

//	Open
//
//	Open an existing game file

	{
	ALERROR error;

	if (m_iRefCount == 0)
		{
		ASSERT(m_pFile == NULL);

		//	Open the file

		m_pFile = new CDataFile(sFilename);
		if (m_pFile == NULL)
			return ERR_MEMORY;

		if (error = m_pFile->Open())
			goto Fail;

		//	Load the header

		m_iHeaderID = m_pFile->GetDefaultEntry();
		if (error = LoadGameHeader(&m_Header))
			goto Fail;

		//	Check the version

		if (m_Header.dwVersion < MIN_GAME_FILE_VERSION
				|| m_Header.dwVersion > GAME_FILE_VERSION)
			{
			error = ERR_FAIL;
			goto Fail;
			}

		//	Load the system map

		CString sSystemMap;
		if (error = m_pFile->ReadEntry(m_Header.dwSystemMap, &sSystemMap))
			goto Fail;

		LoadSystemMapFromStream(m_Header.dwVersion, sSystemMap);

		//	If this is a previous version, upgrade to the latest

		bool bUpgrade = false;
		if (m_Header.dwVersion < 8)
			{
			//	Save out the system map because we changed the format in version 7

			SaveSystemMapToStream(&sSystemMap);
			if (error = m_pFile->WriteEntry(m_Header.dwSystemMap, sSystemMap))
				goto Fail;

			bUpgrade = true;
			}

		if (bUpgrade)
			{
			m_Header.dwVersion = GAME_FILE_VERSION;
			if (error = SaveGameHeader(m_Header))
				goto Fail;

			m_pFile->Flush();
			}
		}

	m_iRefCount++;

	return NOERROR;

Fail:

	delete m_pFile;
	m_pFile = NULL;
	return error;
	}

ALERROR CGameFile::SaveGameHeader (SGameHeader &Header)

//	SaveGameHeader
//
//	Save the game header

	{
	ALERROR error;

	CString sData = CString((char *)&Header, sizeof(Header));

	if (error = m_pFile->WriteEntry(m_iHeaderID, sData))
		{
		kernelDebugLogMessage("Unable to write game header");
		return error;
		}

	return NOERROR;
	}

ALERROR CGameFile::SaveGameStats (const CGameStats &Stats)

//	SaveGameStats
//
//	Saves the game stats block

	{
	ALERROR error;

	//	Save the stats to a stream

	CMemoryWriteStream Stream;
	if (error = Stream.Create())
		return error;

	if (error = Stats.SaveToStream(&Stream))
		return error;

	if (error = Stream.Close())
		return error;

	CString sStream(Stream.GetPointer(), Stream.GetLength(), true);

	//	If we haven't yet saved game stats, add it to the save file

	if (m_Header.dwGameStats == 0 || m_Header.dwGameStats == INVALID_ENTRY)
		{
		if (error = m_pFile->AddEntry(sStream, (int *)&m_Header.dwGameStats))
			return error;

		if (error = SaveGameHeader(m_Header))
			return error;
		}

	//	Otherwise, save to the current entry

	else
		{
		if (error = m_pFile->WriteEntry(m_Header.dwGameStats, sStream))
			return error;
		}

	//	Done

	m_pFile->Flush();

	return NOERROR;
	}

ALERROR CGameFile::SaveSystem (DWORD dwUNID, CSystem *pSystem, DWORD dwFlags)

//	SaveSystem
//
//	Save a star system

	{
	ALERROR error;
	DWORD *pNewMap = NULL;

	ASSERT(m_pFile);

	//	Figure out the limit for the stream (1MB is too small for some
	//	systems with lots of objects)

	int iMaxSize = 2048 * Max(pSystem->GetObjectCount(), 512);

	//	Save the system to a stream

	CMemoryWriteStream Stream(iMaxSize);
	if (error = Stream.Create())
		return error;

	if (error = pSystem->SaveToStream(&Stream))
		{
		kernelDebugLogMessage("Unable to save system to stream");
		return error;
		}

	if (error = Stream.Close())
		return error;

	CString sStream(Stream.GetPointer(), Stream.GetLength(), true);

	//	See if this system has already been saved

	DWORD dwEntry;
	if (m_SystemMap.Lookup(dwUNID, (CObject **)&dwEntry) == NOERROR)
		{
		//	If we're entering a stargate, then we save the system with
		//	versioning so that we can revert it if saving fails later.

		if (dwFlags & FLAG_ENTER_GATE)
			{
			ASSERT(!(m_Header.dwFlags & GAME_FLAG_IN_STARGATE));

			//	Save the system as a new version

			if (error = m_pFile->WriteVersion(dwEntry, sStream))
				{
				kernelDebugLogMessage("Unable to write system version: %x", dwUNID);
				return error;
				}

			//	Mark the fact that we are in the middle of changing system

			m_Header.dwFlags |= GAME_FLAG_IN_STARGATE;
			m_Header.dwPartialSave = dwEntry;
			if (error = SaveGameHeader(m_Header))
				{
				kernelDebugLogMessage("Unable to write game header");
				return error;
				}
			}

		//	Otherwise, just save the system

		else
			{
			//	Save the system

			if (error = m_pFile->WriteEntry(dwEntry, sStream))
				{
				kernelDebugLogMessage("Unable to write system: %x", dwUNID);
				return error;
				}
			}
		}

	//	Otherwise, we add the system

	else
		{
		//	Save the system

		if (error = m_pFile->AddEntry(sStream, (int *)&dwEntry))
			{
			kernelDebugLogMessage("Unable to add system: %x", dwUNID);
			return error;
			}

		//	Add to the map

		m_SystemMap.AddEntry(dwUNID, (CObject *)dwEntry);

		//	Save the map

		CString sData;
		SaveSystemMapToStream(&sData);

		if (error = m_pFile->WriteEntry(m_Header.dwSystemMap, sData))
			{
			kernelDebugLogMessage("Unable to write system map");
			return error;
			}
		}

	//	Done

	m_pFile->Flush();

	return NOERROR;
	}

void CGameFile::SaveSystemMapToStream (CString *retsStream)

//	SaveSystemMapToStream
//
//	Saves out the system map

	{
	int i;

	//	The system map is a DWORD length followed by n pair of DWORDs

	int iTotalLen = sizeof(DWORD) + m_SystemMap.GetCount() * 2 * sizeof(DWORD);
	CString sOutput;
	DWORD *pPos = (DWORD *)sOutput.GetWritePointer(iTotalLen);

	//	Write out the length

	DWORD dwSave = m_SystemMap.GetCount();
	*pPos++ = dwSave;

	//	Write out each mapping

	for (i = 0; i < m_SystemMap.GetCount(); i++)
		{
		dwSave = (DWORD)m_SystemMap.GetKey(i);
		*pPos++ = dwSave;

		dwSave = (DWORD)m_SystemMap.GetValue(i);
		*pPos++ = dwSave;
		}

	//	Done

	*retsStream = sOutput;
	}

ALERROR CGameFile::SaveUniverse (CUniverse &Univ, DWORD dwFlags)

//	SaveUniverse
//
//	Saves the universe

	{
	ALERROR error;

	//	Get the universe to stream itself out (note that we save
	//	systems separately)

	CMemoryWriteStream Stream;
	if (error = Stream.Create())
		return error;

	if (error = Univ.SaveToStream(&Stream))
		return error;

	if (error = Stream.Close())
		return error;

	CString sStream(Stream.GetPointer(), Stream.GetLength(), true);

	//	Keep track to see if we need to update the header

	bool bUpdateHeader = false;

	//	Figure out the name of the system that the player is at

	CSystem *pCurSystem = Univ.GetCurrentSystem();
	if (pCurSystem)
		{
		CString sSystemName = pCurSystem->GetName();
		ASSERT(!sSystemName.IsBlank());
		if (!strEquals(sSystemName, GetSystemName()))
			{
			lstrcpyn(m_Header.szSystemName, sSystemName.GetASCIIZPointer(), sizeof(m_Header.szSystemName));
			bUpdateHeader = true;
			}
		}
	else
		ASSERT(false);

	//	If we don't have an adventure, then it means that this is the first time
	//	that we've saved, so we need to set the adventure and other data in the
	//	header.

	CSpaceObject *pPlayerObj = Univ.GetPlayer();
	CShip *pPlayerShip = (pPlayerObj ? pPlayerObj->AsShip() : NULL);
	IShipController *pPlayerController = (pPlayerShip ? pPlayerShip->GetController() : NULL);

	if (m_Header.dwAdventure == 0)
		{
		m_Header.dwAdventure = Univ.GetCurrentAdventureDesc()->GetExtensionUNID();

		if (pPlayerController)
			{
			CString sPlayerName = pPlayerController->GetPlayerName();
			lstrcpyn(m_Header.szPlayerName, pPlayerController->GetPlayerName().GetASCIIZPointer(), sizeof(m_Header.szPlayerName));
			}

		bUpdateHeader = true;
		}

	//	Save the genome and player ship in the header

	if (pPlayerObj && pPlayerObj->GetType()->GetUNID() != m_Header.dwPlayerShip)
		{
		m_Header.dwPlayerShip = pPlayerObj->GetType()->GetUNID();
		bUpdateHeader = true;
		}

	if (pPlayerController && (DWORD)pPlayerController->GetPlayerGenome() != m_Header.dwGenome)
		{
		m_Header.dwGenome = (DWORD)pPlayerController->GetPlayerGenome();
		bUpdateHeader = true;
		}

	//	Set the flags. The resurrect flag says that we should increase
	//	the resurrect count when loading.

	DWORD dwNewFlags = m_Header.dwFlags;
	dwNewFlags &= ~GAME_FLAG_RESURRECT;
	if (dwFlags & FLAG_CHECKPOINT)
		dwNewFlags |= GAME_FLAG_RESURRECT;

	//	Set the registered flag

	if (Univ.IsRegistered())
		dwNewFlags |= GAME_FLAG_REGISTERED;

	//	Set the debug flag

	if (Univ.InDebugMode())
		dwNewFlags |= GAME_FLAG_DEBUG;

	//	Clear the "in stargate" flag if we're exiting the gate. This tells
	//	us that we've saved the universe successfully.

	if ((dwFlags & FLAG_EXIT_GATE) && (dwNewFlags & GAME_FLAG_IN_STARGATE))
		{
		dwNewFlags &= ~GAME_FLAG_IN_STARGATE;

		//	We also need to delete the old version of the system. Get the
		//	version history.

		TArray<CDataFile::SVersionInfo> History;
		if (error = m_pFile->ReadHistory(m_Header.dwPartialSave, &History))
			{
			kernelDebugLogMessage("Unable to read entry history: %x", m_Header.dwPartialSave);
			return error;
			}

		//	Delete the old entry

		if (History.GetCount() >= 2)
			{
			ASSERT(History.GetCount() == 2);
			ASSERT(m_Header.dwPartialSave != (DWORD)History[1].iEntry);

			if (error = m_pFile->DeleteEntry(History[1].iEntry))
				{
				kernelDebugLogMessage("Unable to delete old system version: %x", m_Header.dwPartialSave);
				return error;
				}
			}
		else
			{
			ASSERT(false);
			kernelDebugLogMessage("Unable to find previous version for system: %x", m_Header.dwPartialSave);
			}
		}

	//	If flags have changed, save the header

	if (dwNewFlags != m_Header.dwFlags)
		{
		m_Header.dwFlags = dwNewFlags;
		bUpdateHeader = true;
		}

	//	If the universe has already been saved before then just
	//	save to the existing entry

	if (m_Header.dwUniverse != INVALID_ENTRY)
		{
		if (error = m_pFile->WriteEntry(m_Header.dwUniverse, sStream))
			return error;
		}

	//	Otherwise we need to create a new entry and update the header

	else
		{
		if (error = m_pFile->AddEntry(sStream, (int *)&m_Header.dwUniverse))
			return error;

		//	Update the header

		bUpdateHeader = true;
		}

	//	Save the header, if necessary

	if (bUpdateHeader)
		{
		if (error = SaveGameHeader(m_Header))
			return error;
		}

	//	Done

	m_pFile->Flush();

	return NOERROR;
	}

ALERROR CGameFile::SetGameResurrect (void)

//	SetGameResurrect
//
//	Sets the resurrect flag in the game (if not already set). This
//	Should only be called when we're loading a game.

	{
	ALERROR error;

	ASSERT(m_pFile);

	//	If we're about to start playing a game that has been
	//	resurrected, then increment our resurrect count (and save it)

	if (IsGameResurrect())
		m_Header.dwResurrectCount++;

	//	Otherwise, set the flag

	else
		m_Header.dwFlags |= GAME_FLAG_RESURRECT;

	//	Save the header

	if (error = SaveGameHeader(m_Header))
		return error;

	return NOERROR;
	}

ALERROR CGameFile::SetGameStatus (int iScore, const CString &sEpitaph)

//	SetGameStatus
//
//	Sets the score and epitaph in the header

	{
	ALERROR error;

	ASSERT(m_pFile);

	m_Header.dwScore = iScore;
	lstrcpyn(m_Header.szEpitaph, sEpitaph.GetASCIIZPointer(), sizeof(m_Header.szEpitaph));

	//	Save the header

	if (error = SaveGameHeader(m_Header))
		return error;

	return NOERROR;
	}
