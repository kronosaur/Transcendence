//	CMissionType.cpp
//
//	CMissionType class

#include "PreComp.h"

#define ALLOW_PLAYER_DELETE_ATTRIB				CONSTLIT("allowPlayerDelete")
#define DEBRIEF_AFTER_OUT_OF_SYSTEM_ATTRIB		CONSTLIT("debriefAfterOutOfSystem")
#define EXPIRE_TIME_ATTRIB						CONSTLIT("expireTime")
#define FAILURE_AFTER_OUT_OF_SYSTEM_ATTRIB		CONSTLIT("failureAfterOutOfSystem")
#define FORCE_UNDOCK_AFTER_DEBRIEF_ATTRIB		CONSTLIT("forceUndockAfterDebrief")
#define LEVEL_ATTRIB							CONSTLIT("level")
#define MAX_APPEARING_ATTRIB					CONSTLIT("maxAppearing")
#define NAME_ATTRIB								CONSTLIT("name")
#define NO_DEBRIEF_ATTRIB						CONSTLIT("noDebrief")
#define NO_FAILURE_ON_OWNER_DESTROYED_ATTRIB	CONSTLIT("noFailureOnOwnerDestroyed")
#define NO_STATS_ATTRIB							CONSTLIT("noStats")
#define PRIORITY_ATTRIB							CONSTLIT("priority")

#define FIELD_LEVEL								CONSTLIT("level")
#define FIELD_MAX_LEVEL							CONSTLIT("maxLevel")
#define FIELD_MIN_LEVEL							CONSTLIT("minLevel")
#define FIELD_NAME								CONSTLIT("name")

bool CMissionType::FindDataField (const CString &sField, CString *retsValue)

//	FindDataField
//
//	Returns the data field.

	{
	if (strEquals(sField, FIELD_LEVEL))
		*retsValue = strFromInt(GetLevel());

	else if (strEquals(sField, FIELD_MAX_LEVEL))
		*retsValue = strFromInt(m_iMaxLevel);

	else if (strEquals(sField, FIELD_MIN_LEVEL))
		*retsValue = strFromInt(m_iMinLevel);

	else if (strEquals(sField, FIELD_NAME))
		*retsValue = m_sName;
	else
		return false;

	return true;
	}

ALERROR CMissionType::OnBindDesign (SDesignLoadCtx &Ctx)

//	OnBindDesign
//
//	Bind design

	{
	return NOERROR;
	}

ALERROR CMissionType::OnCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	OnCreateFromXML
//
//	Create from XML

	{
	ALERROR error;

	m_sName = pDesc->GetAttribute(NAME_ATTRIB);
	m_iPriority = pDesc->GetAttributeIntegerBounded(PRIORITY_ATTRIB, 0, -1, 1);
	m_iExpireTime = pDesc->GetAttributeIntegerBounded(EXPIRE_TIME_ATTRIB, 1, -1, -1);
	m_iFailIfOutOfSystem = pDesc->GetAttributeIntegerBounded(FAILURE_AFTER_OUT_OF_SYSTEM_ATTRIB, 0, -1, -1);
	m_fNoFailureOnOwnerDestroyed = pDesc->GetAttributeBool(NO_FAILURE_ON_OWNER_DESTROYED_ATTRIB);
	m_fNoDebrief = pDesc->GetAttributeBool(NO_DEBRIEF_ATTRIB);
	m_fNoStats = pDesc->GetAttributeBool(NO_STATS_ATTRIB);
	m_fCloseIfOutOfSystem = pDesc->GetAttributeBool(DEBRIEF_AFTER_OUT_OF_SYSTEM_ATTRIB);
	m_fForceUndockAfterDebrief = pDesc->GetAttributeBool(FORCE_UNDOCK_AFTER_DEBRIEF_ATTRIB);
	m_fAllowDelete = pDesc->GetAttributeBool(ALLOW_PLAYER_DELETE_ATTRIB);

	CString sAttrib;
	if (pDesc->FindAttribute(MAX_APPEARING_ATTRIB, &sAttrib))
		{
		if (error = m_MaxAppearing.LoadFromXML(sAttrib))
			{
			Ctx.sError = CONSTLIT("Invalid maxAppearing parameter.");
			return ERR_FAIL;
			}
		}

	m_iMaxAppearing = (m_MaxAppearing.IsEmpty() ? -1 : m_MaxAppearing.Roll());
	m_iAccepted = 0;

	//	Level

	if (pDesc->FindAttribute(LEVEL_ATTRIB, &sAttrib))
		{
		//	Parse this value

		char *pPos = sAttrib.GetASCIIZPointer();
		m_iMinLevel = Max(1, Min(strParseInt(pPos, 1, &pPos), MAX_SYSTEM_LEVEL));

		while (*pPos == ' ')
			pPos++;

		if (*pPos == '-')
			{
			pPos++;
			m_iMaxLevel = Max(m_iMinLevel, Min(strParseInt(pPos, MAX_SYSTEM_LEVEL), MAX_SYSTEM_LEVEL));
			}
		else
			m_iMaxLevel = m_iMinLevel;
		}
	else
		{
		m_iMinLevel = 1;
		m_iMaxLevel = MAX_SYSTEM_LEVEL;
		}

	return NOERROR;
	}

void CMissionType::OnReadFromStream (SUniverseLoadCtx &Ctx)

//	OnReadFromStream
//
//	Read mission type data

	{
	if (Ctx.dwVersion >= 22)
		{
		Ctx.pStream->Read((char *)&m_iMaxAppearing, sizeof(DWORD));
		Ctx.pStream->Read((char *)&m_iAccepted, sizeof(DWORD));
		}
	}

void CMissionType::OnReinit (void)

//	OnReinit
//
//	Reinitialize

	{
	m_iMaxAppearing = (m_MaxAppearing.IsEmpty() ? -1 : m_MaxAppearing.Roll());
	m_iAccepted = 0;
	}

void CMissionType::OnWriteToStream (IWriteStream *pStream)

//	OnWriteToStream
//
//	Write mission type data

	{
	pStream->Write((char *)&m_iMaxAppearing, sizeof(DWORD));
	pStream->Write((char *)&m_iAccepted, sizeof(DWORD));
	}
