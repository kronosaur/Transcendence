//	CDamageSource.cpp
//
//	CDamageSource class

#include "PreComp.h"

CDamageSource::CDamageSource (CSpaceObject *pSource, DestructionTypes iCause, CSpaceObject *pSecondarySource, const CString &sSourceName, DWORD dwSourceFlags) : 
		m_iCause(iCause),
		m_dwFlags(0),
		m_sSourceName(sSourceName),
		m_dwSourceNameFlags(dwSourceFlags)

//	CDamageSource constructor

	{
	SetObj(pSource);

	m_pSecondarySource = (pSecondarySource && !pSecondarySource->IsDestroyed() ? pSecondarySource : NULL);
	}

CString CDamageSource::GetDamageCauseNounPhrase (DWORD dwFlags)

//	GetDamageCauseNounPhrase
//
//	Returns the name of the damage source

	{
	if (m_pSource)
		return m_pSource->GetDamageCauseNounPhrase(dwFlags);
	else if (!m_sSourceName.IsBlank())
		return ::ComposeNounPhrase(m_sSourceName, 1, NULL_STR, m_dwSourceNameFlags, dwFlags);
	else
		return CONSTLIT("damage");
	}

CSpaceObject *CDamageSource::GetObj (void) const

//	GetObj
//
//	Returns the source object

	{
	CSpaceObject *pOrderGiver;

	//	If the source is the player then always return the player
	//	object (regardless of m_pSource). We do this in case
	//	the player changes ships.

	if (m_dwFlags & FLAG_IS_PLAYER)
		{
		CSystem *pSystem = g_pUniverse->GetCurrentSystem();
		return (pSystem ? pSystem->GetPlayer() : NULL);
		}

	//	Otherwise, if we're a subordinate and our order giver
	//	has changed, switch back to the player.

	else if ((m_dwFlags & FLAG_IS_PLAYER_SUBORDINATE)
			&& (m_pSource == NULL
				|| (pOrderGiver = m_pSource->GetOrderGiver()) == NULL
				|| !pOrderGiver->IsPlayer()))
		{
		CSystem *pSystem = g_pUniverse->GetCurrentSystem();
		return (pSystem ? pSystem->GetPlayer() : NULL);
		}

	//	Otherwise, return the source (even if NULL)

	else
		return m_pSource;
	}

CSovereign *CDamageSource::GetSovereign (void) const

//	GetSovereign
//
//	Returns the sovereign of the source

	{
	if (IsPlayer())
		return g_pUniverse->GetPlayerSovereign();
	else if (m_pSource)
		return m_pSource->GetSovereign();
	else
		return NULL;
	}

DWORD CDamageSource::GetSovereignUNID (void) const

//	GetSovereignUNID
//
//	Returns the sovereign

	{
	CSovereign *pSovereign = GetSovereign();
	if (pSovereign == NULL)
		return 0;

	return pSovereign->GetUNID();
	}

bool CDamageSource::IsCausedByEnemyOf (CSpaceObject *pObj) const

//	IsCausedByEnemyOf
//
//	Returns TRUE if the damage is caused by an object that is
//	an enemy of pObj.

	{
	if (IsCausedByPlayer())
		{
		CSovereign *pSovereign = pObj->GetSovereign();
		return (pSovereign ? pSovereign->IsEnemy(g_pUniverse->GetPlayerSovereign()) : false);
		}
	else
		return (m_pSource && m_pSource->CanAttack() && pObj->IsEnemy(m_pSource));
	}

bool CDamageSource::IsCausedByFriendOf (CSpaceObject *pObj) const

//	IsCausedByFriendOf
//
//	Returns TRUE if the damage is caused by an object that is
//	friends of pObj.

	{
	if (IsCausedByPlayer())
		{
		CSovereign *pSovereign = pObj->GetSovereign();
		return (pSovereign ? pSovereign->IsFriend(g_pUniverse->GetPlayerSovereign()) : false);
		}
	else
		return (m_pSource && m_pSource->CanAttack() && pObj->IsFriend(m_pSource));
	}

bool CDamageSource::IsCausedByNonFriendOf (CSpaceObject *pObj) const

//	IsCausedByNonFriendOf
//
//	Returns TRUE if the damage is caused by an object that is
//	not a friend of pObj and that can attack.

	{
	if (IsCausedByPlayer())
		{
		CSovereign *pSovereign = pObj->GetSovereign();
		return (pSovereign ? !pSovereign->IsFriend(g_pUniverse->GetPlayerSovereign()) : false);
		}
	else
		return (m_pSource && m_pSource->CanAttack() && !pObj->IsFriend(m_pSource));
	}

void CDamageSource::OnObjDestroyed (CSpaceObject *pObjDestroyed)

//	OnObjDestroyed
//
//	An object was destroyed

	{
	if (pObjDestroyed == m_pSource)
		{
		m_sSourceName = m_pSource->GetName(&m_dwSourceNameFlags);
		m_pSource = NULL;
		}

	if (pObjDestroyed == m_pSecondarySource)
		m_pSecondarySource = NULL;
	}

void CDamageSource::ReadFromStream (SLoadCtx &Ctx)

//	ReadFromStream
//
//	Read data from a stream
//
//	DWORD		m_pSource
//	CString		m_sSourceName
//	DWORD		m_dwSourceNameFlags
//	DWORD		m_pSecondarySource
//	DWORD		m_iCause
//	DWORD		m_dwFlags

	{
	DWORD dwLoad;

	CSystem::ReadObjRefFromStream(Ctx, &m_pSource);
	if (Ctx.dwVersion >= 30)
		{
		m_sSourceName.ReadFromStream(Ctx.pStream);
		Ctx.pStream->Read((char *)&m_dwSourceNameFlags, sizeof(DWORD));
		}

	if (Ctx.dwVersion >= 32)
		CSystem::ReadObjRefFromStream(Ctx, &m_pSecondarySource);

	if (Ctx.dwVersion >= 45)
		{
		Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
		m_iCause = (DestructionTypes)dwLoad;
		}
	else
		m_iCause = killedByDamage;

	if (Ctx.dwVersion >= 55)
		Ctx.pStream->Read((char *)&m_dwFlags, sizeof(DWORD));
	else
		m_dwFlags = 0;
	}

void CDamageSource::SetObj (CSpaceObject *pSource)

//	SetObj
//
//	Sets the damage source
	
	{
	//	If this is the player, remember it in case we lose the
	//	source later.

	if (pSource)
		{
		CSpaceObject *pOrderGiver;

		if (pSource->IsPlayer())
			{
			m_dwFlags |= FLAG_IS_PLAYER;
			m_dwFlags |= FLAG_IS_PLAYER_CAUSED;
			}
		else if ((pOrderGiver = pSource->GetOrderGiver()) && pOrderGiver->IsPlayer())
			{
			m_dwFlags |= FLAG_IS_PLAYER_SUBORDINATE;
			m_dwFlags |= FLAG_IS_PLAYER_CAUSED;
			}
		}
	else
		m_dwFlags &= ~(FLAG_IS_PLAYER | FLAG_IS_PLAYER_SUBORDINATE | FLAG_IS_PLAYER_CAUSED);

	//	If the source is already destroyed, then don't store it--just get the
	//	name of the source.

	if (pSource && pSource->IsDestroyed())
		{
		m_sSourceName = pSource->GetName(&m_dwSourceNameFlags);
		m_pSource = NULL;
		}

	//	Otherwise, remember the source

	else
		m_pSource = pSource;
	}

void CDamageSource::WriteToStream (CSystem *pSystem, IWriteStream *pStream)

//	WriteToStream
//
//	Write data to a stream
//
//	DWORD		m_pSource
//	CString		m_sSourceName
//	DWORD		m_dwSourceNameFlags
//	DWORD		m_pSecondarySource
//	DWORD		m_iCause
//	DWORD		m_dwFlags

	{
	DWORD dwSave;

	pSystem->WriteObjRefToStream(m_pSource, pStream);
	m_sSourceName.WriteToStream(pStream);
	pStream->Write((char *)&m_dwSourceNameFlags, sizeof(DWORD));
	pSystem->WriteObjRefToStream(m_pSecondarySource, pStream);

	dwSave = m_iCause;
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	pStream->Write((char *)&m_dwFlags, sizeof(DWORD));
	}
