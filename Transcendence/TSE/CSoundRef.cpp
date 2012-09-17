//	CSoundRef.cpp
//
//	CSoundRef class

#include "PreComp.h"

ALERROR CSoundRef::Bind (SDesignLoadCtx &Ctx)

//	Bind
//
//	Binds the design (looking up the actual sound file)

	{
	if (Ctx.bNoResources || g_pUniverse->GetSoundMgr() == NULL)
		{
		m_iSound = -1;
		return NOERROR;
		}

	if (m_dwUNID)
		{
		m_iSound = g_pUniverse->FindSound(m_dwUNID);
		if (m_iSound == -1 && Ctx.GetAPIVersion() >= 12)
			{
			Ctx.sError = strPatternSubst(CONSTLIT("Unable to find sound: %x."), m_dwUNID);
			return ERR_FAIL;
			}
		}
	else
		m_iSound = -1;

	return NOERROR;
	}

ALERROR CSoundRef::LoadUNID (SDesignLoadCtx &Ctx, const CString &sAttrib)

//	LoadUNID
//
//	Loads the sound file UNID

	{
	return ::LoadUNID(Ctx, sAttrib, &m_dwUNID);
	}

void CSoundRef::PlaySound (CSpaceObject *pSource)

//	PlaySound
//
//	Plays the sound

	{
	if (m_iSound != -1)
		g_pUniverse->PlaySound(pSource, m_iSound);
	}
