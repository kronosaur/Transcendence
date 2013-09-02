//	Sound.cpp
//
//	Functions to help with DirectSound

#include <windows.h>
#include <dsound.h>
#include "Alchemy.h"
#include "DirectXUtil.h"

const int MAX_VOLUME = 0;
const int MIN_VOLUME = -10000;
const int MAX_VOLUME_LEVEL = 10;
const int VOLUME_STEP = 250;
const int MUSIC_LEVEL_FACTOR = 100;

CSoundMgr::CSoundMgr (void) : m_pDS(NULL), 
		m_hMusic(NULL), 
		m_iSoundVolume(MAX_VOLUME_LEVEL), 
		m_iMusicVolume(MAX_VOLUME_LEVEL)

//	CSoundMgr constructor

	{
	}

CSoundMgr::~CSoundMgr (void)

//	CSoundMgr destructor

	{
	CleanUp();
	}

void CSoundMgr::AddMusicFolder (const CString &sFolder, TArray<CString> *retCatalog)

//	AddMusicFolder
//
//	This is a recursive function that adds all the music files in sFolder (and
//	any subfolders) to retCatalog

	{
	CFileDirectory Dir(pathAddComponent(sFolder, CONSTLIT("*.*")));
	while (Dir.HasMore())
		{
		SFileDesc FileDesc;
		Dir.GetNextDesc(&FileDesc);

		//	Skip hidden or system files

		if (FileDesc.bHiddenFile || FileDesc.bSystemFile)
			continue;

		//	Skip any file or directory that starts with a dot

		if (*FileDesc.sFilename.GetASCIIZPointer() == '.')
			continue;

		//	Get the full filename

		CString sFilepath = pathAddComponent(sFolder, FileDesc.sFilename);

		//	If this is a folder then recurse

		if (FileDesc.bFolder)
			AddMusicFolder(sFilepath, retCatalog);

		//	Otherwise add the filespec

		else
			retCatalog->Insert(sFilepath);
		}
	}

int CSoundMgr::AllocChannel (void)

//	AllocChannel
//
//	Allocate a channel (note that the buffer will be completely
//	uninitialized).

	{
	int i;

	ASSERT(m_pDS);

	//	Look for an empty channel

	for (i = 0; i < m_Channels.GetCount(); i++)
		if (m_Channels[i].pBuffer == NULL)
			return i;

	//	Otherwise, allocate a new one

	int iIndex = m_Channels.GetCount();
	m_Channels.Insert();

	return iIndex;
	}

bool CSoundMgr::CanPlayMusic (const CString &sFilename)

//	CanPlayMusic
//
//	Returns TRUE if we can play the music

	{
	if (m_hMusic == NULL)
		return false;

	//	Open the device

	if (MCIWndOpen(m_hMusic, sFilename.GetASCIIZPointer(), 0) != 0)
		return false;

	//	Can play it?

	if (!MCIWndCanPlay(m_hMusic))
		return false;

	//	Done

	return true;
	}

void CSoundMgr::CleanUp (void)

//	CleanUp
//
//	Clean up sound manager

	{
	int i;

	if (m_pDS)
		{
		//	Stop and delete all buffers

		for (i = 0; i < GetChannelCount(); i++)
			CleanUpChannel(m_Channels[i]);

		m_Channels.DeleteAll();

		//	Kill the DirectSound object

		m_pDS->Release();
		m_pDS = NULL;
		}

	if (m_hMusic)
		{
		::DestroyWindow(m_hMusic);
		m_hMusic = NULL;
		}
	}

void CSoundMgr::CleanUpChannel (SChannel &Channel)

//	CleanUpChannel
//
//	Clean up the given channel

	{
	if (Channel.pBuffer)
		{
		Channel.pBuffer->Stop();
		Channel.pBuffer->Release();
		Channel.pBuffer = NULL;

		//	Clean up duplicate bufferse

		SChannel *pNext = Channel.pNext;
		while (pNext)
			{
			pNext->pBuffer->Stop();
			pNext->pBuffer->Release();

			SChannel *pDelete = pNext;
			pNext = pNext->pNext;
			delete pDelete;
			}

		Channel.pNext = NULL;
		}
	}

void CSoundMgr::Delete (int iChannel)

//	Delete
//
//	Deletes the given channel

	{
	CleanUpChannel(m_Channels[iChannel]);
	}

void CSoundMgr::GetMusicCatalog (const CString &sMusicFolder, TArray<CString> *retCatalog)

//	GetMusicCatalog
//
//	Returns a list of filenames for all songs in the given folder.
//	If sMusicFolder is NULL then we return the files in the Windows Music folder

	{
	AddMusicFolder((sMusicFolder.IsBlank() ? pathGetSpecialFolder(folderMusic) : sMusicFolder), retCatalog);
	}

bool CSoundMgr::GetMusicPlayState (SMusicPlayState *retState)

//	GetMusicPlayState
//
//	Returns the current play state. Returns FALSE if not playing

	{
	if (m_hMusic == NULL)
		{
		retState->bPlaying = false;
		retState->iLength = 0;
		retState->iPos = 0;
		retState->sFilename = NULL_STR;
		return false;
		}

	int iMode = MCIWndGetMode(m_hMusic, NULL, 0);
	retState->bPlaying = (iMode == MCI_MODE_PLAY);
	retState->bPaused = (iMode == MCI_MODE_PAUSE);
	retState->iLength = MCIWndGetLength(m_hMusic);
	retState->iPos = MCIWndGetPosition(m_hMusic);
	
	char *pDest = retState->sFilename.GetWritePointer(MAX_PATH);
	MCIWndGetFileName(m_hMusic, pDest, retState->sFilename.GetLength());
	retState->sFilename.Truncate(lstrlen(pDest));

	return retState->bPlaying;
	}

ALERROR CSoundMgr::Init (HWND hWnd)

//	Init
//
//	Initialize sound manager

	{
	HRESULT hr;

	ASSERT(m_pDS == NULL);

	//	Initialize DirectSound

	hr = ::DirectSoundCreate(NULL, &m_pDS, NULL);
	if (hr != DS_OK)
		return ERR_FAIL;

	hr = m_pDS->SetCooperativeLevel(hWnd, DSSCL_NORMAL);
	if (hr != DS_OK)
		{
		m_pDS->Release();
		m_pDS = NULL;
		return ERR_FAIL;
		}

	//	Initialize MCI

	m_hMusic = ::MCIWndCreate(hWnd, 
			(HINSTANCE)::GetWindowLong(hWnd, GWL_HINSTANCE),
			WS_OVERLAPPED | WS_CHILD | MCIWNDF_NOERRORDLG | MCIWNDF_NOMENU | MCIWNDF_NOPLAYBAR | MCIWNDF_NOTIFYALL,
			NULL);
	//::ShowWindow(m_hMusic, SW_HIDE);
	if (m_hMusic == NULL)
		return ERR_FAIL;

	//	Set the time format to milliseconds

	MCIWndUseTime(m_hMusic);

	return NOERROR;
	}

ALERROR CSoundMgr::LoadWaveFile (const CString &sFilename, int *retiChannel)

//	LoadWaveFile
//
//	Creates a sound buffer from a WAV file

	{
	ALERROR error;
	HMMIO hFile;

	if (m_pDS == NULL)
		{
		*retiChannel = 0;
		return NOERROR;
		}

	hFile = mmioOpen(sFilename.GetASCIIZPointer(), NULL, MMIO_READ | MMIO_ALLOCBUF);
	if (hFile == NULL)
		return ERR_FAIL;

	if (error = LoadWaveFile(hFile, retiChannel))
		return error;

	SChannel *pChannel = GetChannel(*retiChannel);
	pChannel->sFilename = sFilename;
	return NOERROR;
	}

ALERROR CSoundMgr::LoadWaveFile (HMMIO hFile, int *retiChannel)

//	LoadWaveFile
//
//	Creates a sound buffer from an open MMIO file

	{
	ASSERT(m_pDS);

	MMCKINFO parent, child;
	::ZeroMemory(&parent, sizeof(parent));
	::ZeroMemory(&child, sizeof(child));

	//	Descend into the RIFF

	parent.fccType = mmioFOURCC('W', 'A', 'V', 'E');
	if (mmioDescend(hFile, &parent, NULL, MMIO_FINDRIFF))
		{
		mmioClose(hFile, 0);
		return ERR_FAIL;
		}

	//	Descend to the wave format

	child.ckid = mmioFOURCC('f', 'm', 't', ' ');
	if (mmioDescend(hFile, &child, &parent, 0))
		{
		mmioClose(hFile, 0);
		return ERR_FAIL;
		}

	//	Allocate a block large enough to hold the format size

	DWORD dwFmtSize = child.cksize;
	DWORD dwAllocSize = max(dwFmtSize, sizeof(WAVEFORMATEX));
	WAVEFORMATEX *pFormat = (WAVEFORMATEX *)new char [dwAllocSize];
	::ZeroMemory(pFormat, dwAllocSize);

	if (mmioRead(hFile, (char *)pFormat, dwFmtSize) != (LONG)dwFmtSize)
		{
		delete [] (char *)pFormat;
		mmioClose(hFile, 0);
		return ERR_FAIL;
		}

	//	Now descend to the data

	if (mmioAscend(hFile, &child, 0))
		{
		delete [] (char *)pFormat;
		mmioClose(hFile, 0);
		return ERR_FAIL;
		}

	child.ckid = mmioFOURCC('d', 'a', 't', 'a');
	if (mmioDescend(hFile, &child, &parent, MMIO_FINDCHUNK))
		{
		delete [] (char *)pFormat;
		mmioClose(hFile, 0);
		return ERR_FAIL;
		}

	DWORD dwDataSize = child.cksize;

	//	Create the DirectSound buffer

	DSBUFFERDESC dsbdesc;
	LPDIRECTSOUNDBUFFER pBuffer;

	::ZeroMemory(&dsbdesc, sizeof(dsbdesc));
	dsbdesc.dwSize = sizeof(dsbdesc);
	dsbdesc.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPAN;
	dsbdesc.dwBufferBytes = dwDataSize;
	dsbdesc.lpwfxFormat = pFormat;

	if (FAILED(m_pDS->CreateSoundBuffer(&dsbdesc, &pBuffer, NULL)))
		{
		//	If we can't create the sound buffer, try creating the same buffer
		//	but with no volume or pan controls.

		dsbdesc.dwFlags = 0;
		if (FAILED(m_pDS->CreateSoundBuffer(&dsbdesc, &pBuffer, NULL)))
			{
			delete [] (char *)pFormat;
			return ERR_FAIL;
			}
		}

	delete [] (char *)pFormat;

	//	Now read into the buffer

	LPVOID pDest;
	DWORD dwDestSize;
	if (FAILED(pBuffer->Lock(0, 0, &pDest, &dwDestSize, NULL, NULL, DSBLOCK_ENTIREBUFFER)))
		{
		pBuffer->Release();
		mmioClose(hFile, 0);
		return ERR_FAIL;
		}

	if (mmioRead(hFile, (char *)pDest, dwDestSize) != (LONG)dwDataSize)
		{
		pBuffer->Release();
		mmioClose(hFile, 0);
		return ERR_FAIL;
		}

	//	Done with the wave file

	mmioClose(hFile, 0);

	//	Add to the channel

	int iChannel = AllocChannel();
	SChannel *pChannel = GetChannel(iChannel);
	pChannel->pBuffer = pBuffer;
	pChannel->pNext = NULL;

	//	Done

	if (retiChannel)
		*retiChannel = iChannel;

	return NOERROR;
	}

ALERROR CSoundMgr::LoadWaveFromBuffer (IReadBlock &Data, int *retiChannel)

//	LoadWaveFromBuffer
//
//	Loads a wave file from a buffer

	{
	ALERROR error;
	HMMIO hFile;
	MMIOINFO info;

	if (m_pDS == NULL)
		{
		*retiChannel = 0;
		return NOERROR;
		}

	::ZeroMemory(&info, sizeof(info));
	info.pchBuffer = Data.GetPointer(0, -1);
	info.fccIOProc = FOURCC_MEM;
	info.cchBuffer = Data.GetLength();

	hFile = mmioOpen(NULL, &info, MMIO_READ);
	if (hFile == NULL)
		return ERR_FAIL;

	if (error = LoadWaveFile(hFile, retiChannel))
		return error;

	return NOERROR;
	}

void CSoundMgr::Play (int iChannel, int iVolume, int iPan)

//	Play
//
//	Plays a channel.
//
//	iVolume = 0 for maximum volume.
//	iVolume = -10,000 for minimum volume.

	{
	if (m_pDS == NULL || m_iSoundVolume == 0)
		return;

	SChannel *pChannel = GetChannel(iChannel);
	if (pChannel->pBuffer == NULL)
		return;

	//	If the buffer is lost, then we need to restore it

	DWORD dwStatus;
	pChannel->pBuffer->GetStatus(&dwStatus);
	if (dwStatus & DSBSTATUS_BUFFERLOST)
		{
		if (FAILED(pChannel->pBuffer->Restore()))
			return;

		if (pChannel->sFilename.IsBlank())
			return;

		//	Open the file

		MMCKINFO parent, child;
		::ZeroMemory(&parent, sizeof(parent));
		::ZeroMemory(&child, sizeof(child));

		HMMIO hFile = mmioOpen(pChannel->sFilename.GetASCIIZPointer(), NULL, MMIO_READ | MMIO_ALLOCBUF);
		if (hFile == NULL)
			return;

		//	Descend into the RIFF

		parent.fccType = mmioFOURCC('W', 'A', 'V', 'E');
		if (mmioDescend(hFile, &parent, NULL, MMIO_FINDRIFF))
			{
			mmioClose(hFile, 0);
			return;
			}

		child.ckid = mmioFOURCC('d', 'a', 't', 'a');
		if (mmioDescend(hFile, &child, &parent, MMIO_FINDCHUNK))
			{
			mmioClose(hFile, 0);
			return;
			}

		DWORD dwDataSize = child.cksize;

		//	Now read into the buffer

		LPVOID pDest;
		DWORD dwDestSize;
		if (FAILED(pChannel->pBuffer->Lock(0, 0, &pDest, &dwDestSize, NULL, NULL, DSBLOCK_ENTIREBUFFER)))
			{
			mmioClose(hFile, 0);
			return;
			}

		if (mmioRead(hFile, (char *)pDest, dwDestSize) != (LONG)dwDataSize)
			{
			mmioClose(hFile, 0);
			return;
			}

		//	Done with the wave file

		mmioClose(hFile, 0);
		}

	//	Check to see if the channel is busy. If it is, then we may need to create
	//	a second buffer.

	else if (dwStatus & DSBSTATUS_PLAYING)
		{
		SChannel *pLastChannel = pChannel;
		pChannel = pChannel->pNext;
		while (pChannel)
			{
			pChannel->pBuffer->GetStatus(&dwStatus);
			if (!(dwStatus & DSBSTATUS_BUFFERLOST)
					&& !(dwStatus & DSBSTATUS_PLAYING))
				break;

			pLastChannel = pChannel;
			pChannel = pChannel->pNext;
			}

		//	If we couldn't find another channel, then duplicate the original

		if (pChannel == NULL)
			{
			pChannel = GetChannel(iChannel);
			LPDIRECTSOUNDBUFFER pNewBuffer = NULL;

			if (FAILED(m_pDS->DuplicateSoundBuffer(pChannel->pBuffer, &pNewBuffer)))
				return;

			SChannel *pNewChannel = new SChannel;
			pLastChannel->pNext = pNewChannel;
			pNewChannel->pBuffer = pNewBuffer;
			pNewChannel->pNext = NULL;

			pChannel = pNewChannel;
			}
		}

	//	Adjust volume

	int iVolumeAdj = MAX_VOLUME_LEVEL - m_iSoundVolume;
	int iMaxVolume = -(iVolumeAdj * VOLUME_STEP);
	int iVolumeRange = iMaxVolume - MIN_VOLUME;
	iVolume = iMaxVolume - ((iVolumeRange * iVolume) / MIN_VOLUME);

	//	pChannel now points to a valid buffer. Play it!

	pChannel->pBuffer->SetVolume(iVolume);
	pChannel->pBuffer->SetPan(iPan);
	pChannel->pBuffer->Play(0, 0, 0);

	//	Clean-up any channels after us that are done playing

	SChannel *pLastChannel = pChannel;
	pChannel = pChannel->pNext;
	while (pChannel)
		{
		pChannel->pBuffer->GetStatus(&dwStatus);
		if (!(dwStatus & DSBSTATUS_PLAYING))
			{
			pChannel->pBuffer->Release();
			pLastChannel->pNext = pChannel->pNext;
			delete pChannel;
			pChannel = pLastChannel->pNext;
			}
		else
			{
			pLastChannel = pChannel;
			pChannel = pChannel->pNext;
			}
		}
	}

bool CSoundMgr::PlayMusic (const CString &sFilename, int iPos, CString *retsError)

//	PlayMusic
//
//	Starts playing the given MP3 file

	{
	if (m_hMusic == NULL)
		return false;

	//	Figure out our current state

	SMusicPlayState State;
	GetMusicPlayState(&State);

	//	If we're not already playing this file, open it

	if (!strEquals(sFilename, State.sFilename))
		{
		//	Stop playing first

		if (State.bPlaying || State.bPaused)
			MCIWndStop(m_hMusic);

		//	Open new file

		if (MCIWndOpen(m_hMusic, sFilename.GetASCIIZPointer(), 0) != 0)
			{
			if (retsError)
				{
				char *pDest = retsError->GetWritePointer(1024);
				MCIWndGetError(m_hMusic, pDest, retsError->GetLength());
				retsError->Truncate(lstrlen(pDest));
				}
			return false;
			}
		}

	//	Seek to the proper position

	MCIWndSeek(m_hMusic, Max(0, Min(iPos, State.iLength)));

	//	Play it

	if (MCIWndPlay(m_hMusic) != 0)
		return false;

	//	Done

	return true;
	}

int CSoundMgr::SetMusicVolume (int iVolumeLevel)

//	SetMusicVolume
//
//	Sets the current music volume

	{
	m_iMusicVolume = iVolumeLevel;
	MCIWndSetVolume(m_hMusic, MUSIC_LEVEL_FACTOR * iVolumeLevel);
	return m_iMusicVolume;
	}

void CSoundMgr::StopMusic (void)

//	StopMusic
//
//	Stops the music playback (if it is still playing)

	{
	if (m_hMusic)
		MCIWndStop(m_hMusic);
	}

void CSoundMgr::TogglePlayPaused (void)

//	TogglePlayPaused
//
//	Play/Pause

	{
	int iMode = MCIWndGetMode(m_hMusic, 0, NULL);
	if (iMode == MCI_MODE_PLAY)
		MCIWndPause(m_hMusic);
	else if (iMode == MCI_MODE_PAUSE)
		MCIWndPlay(m_hMusic);
	}
