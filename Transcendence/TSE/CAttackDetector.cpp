//	CAttackDetector.cpp
//
//	CAttackDetector class

#include "PreComp.h"

CAttackDetector::CAttackDetector (void) : m_iCounter(0), m_iLastHit(0)

//	CAttackDetector constructor

	{
	}

bool CAttackDetector::Hit (int iTick)

//	Hit
//
//	Register a hit. Returns TRUE if there have been enough hits that we should
//	count the attack as deliberate.

	{
	if (IsBlacklisted())
		return true;

	//	If we were hit recently, then ignore the hit because it is probably
	//	from a multi-shot weapon (or a fragmentation weapon)

	if (iTick - m_iLastHit > MULTI_HIT_WINDOW)
		{
		m_iCounter++;
		m_iLastHit = iTick;

		if (m_iCounter > HIT_LIMIT)
			{
			m_iCounter = -1;
			return true;
			}
		}

	return false;
	}

void CAttackDetector::OnUpdate (void)

//	OnUpdate
//
//	Update structure. This is called once per DECAY_RATE.

	{
	if (m_iCounter > 0)
		m_iCounter--;
	}

void CAttackDetector::ReadFromStream (SLoadCtx &Ctx)

//	ReadFromStream
//
//	Read structure

	{
	Ctx.pStream->Read((char *)&m_iCounter, sizeof(DWORD));
	Ctx.pStream->Read((char *)&m_iLastHit, sizeof(DWORD));
	}

void CAttackDetector::WriteToStream (IWriteStream *pStream)

//	WriteToStream
//
//	Write the structure

	{
	pStream->Write((char *)&m_iCounter, sizeof(DWORD));
	pStream->Write((char *)&m_iLastHit, sizeof(DWORD));
	}
