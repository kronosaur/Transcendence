//	CGameTimeKeeper.cpp
//
//	CGameTimeKeeper class

#include "PreComp.h"

const DWORD VERSION = 1;

void CGameTimeKeeper::AddDiscontinuity (int iTick, const CTimeSpan &Duration)

//	AddDiscontinuity
//
//	Adds a discontinuity to the time stream

	{
	SDiscontinuity *pDisc = m_Discontinuities.Insert();
	pDisc->iTick = iTick;
	pDisc->iDuration = ((LONGLONG)Duration.Days() * (LONGLONG)SECONDS_PER_DAY) + Duration.MillisecondsSinceMidnight() / 1000;
	}

CTimeSpan CGameTimeKeeper::GetElapsedTimeAt (int iTick)

//	GetElapsedTimeAt
//
//	Returns the elapsed game time from the beginning of the game
//	to the given tick.

	{
	int i;

	//	First compute the elapsed time (accounting for
	//	discontinuities) in seconds.

	LONGLONG iTime = (LONGLONG)(iTick * g_TimeScale / g_TicksPerSecond);
	for (i = 0; i < m_Discontinuities.GetCount(); i++)
		if (iTick > m_Discontinuities[i].iTick)
			iTime += m_Discontinuities[i].iDuration;

	//	Split into days and seconds

	LONGLONG iDays = iTime / (LONGLONG)SECONDS_PER_DAY;
	LONGLONG iSeconds = iTime % (LONGLONG)SECONDS_PER_DAY;

	//	Return a time span

	return CTimeSpan((int)iDays, (int)iSeconds * 1000);
	}

void CGameTimeKeeper::ReadFromStream (IReadStream *pStream)

//	ReadFromStream
//
//	DWORD		version
//	DWORD		count of discontinuities
//	LONGLONG		iDuration
//	DWORD			iTick

	{
	int i;

	DWORD dwVersion;
	pStream->Read((char *)&dwVersion, sizeof(DWORD));

	DWORD dwCount;
	pStream->Read((char *)&dwCount, sizeof(DWORD));

	m_Discontinuities.InsertEmpty(dwCount);
	for (i = 0; i < (int)dwCount; i++)
		{
		pStream->Read((char *)&m_Discontinuities[i].iDuration, sizeof(LONGLONG));
		pStream->Read((char *)&m_Discontinuities[i].iTick, sizeof(DWORD));
		}
	}

void CGameTimeKeeper::WriteToStream (IWriteStream *pStream)

//	WriteToStream
//	
//	DWORD		version
//	DWORD		count of discontinuities
//	LONGLONG		iDuration
//	DWORD			iTick

	{
	int i;
	DWORD dwSave;

	dwSave = VERSION;
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	dwSave = m_Discontinuities.GetCount();
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	for (i = 0; i < m_Discontinuities.GetCount(); i++)
		{
		pStream->Write((char *)&m_Discontinuities[i].iDuration, sizeof(LONGLONG));
		pStream->Write((char *)&m_Discontinuities[i].iTick, sizeof(DWORD));
		}
	}
