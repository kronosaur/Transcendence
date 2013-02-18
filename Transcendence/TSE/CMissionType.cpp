//	CMissionType.cpp
//
//	CMissionType class

#include "PreComp.h"

#define EXPIRE_TIME_ATTRIB						CONSTLIT("expireTime")
#define FAILURE_AFTER_OUT_OF_SYSTEM_ATTRIB		CONSTLIT("failureAfterOutOfSystem")
#define NAME_ATTRIB								CONSTLIT("name")
#define NO_DEBRIEF_ATTRIB						CONSTLIT("noDebrief")
#define NO_FAILURE_ON_OWNER_DESTROYED_ATTRIB	CONSTLIT("noFailureOnOwnerDestroyed")
#define PRIORITY_ATTRIB							CONSTLIT("priority")

#define FIELD_NAME								CONSTLIT("name")

bool CMissionType::FindDataField (const CString &sField, CString *retsValue)

//	FindDataField
//
//	Returns the data field.

	{
	if (strEquals(sField, FIELD_NAME))
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
	m_sName = pDesc->GetAttribute(NAME_ATTRIB);
	m_iPriority = pDesc->GetAttributeIntegerBounded(PRIORITY_ATTRIB, 0, -1, 1);
	m_iExpireTime = pDesc->GetAttributeIntegerBounded(EXPIRE_TIME_ATTRIB, 1, -1, -1);
	m_iFailIfOutOfSystem = pDesc->GetAttributeIntegerBounded(FAILURE_AFTER_OUT_OF_SYSTEM_ATTRIB, 0, -1, -1);
	m_fNoFailureOnOwnerDestroyed = pDesc->GetAttributeBool(NO_FAILURE_ON_OWNER_DESTROYED_ATTRIB);
	m_fNoDebrief = pDesc->GetAttributeBool(NO_DEBRIEF_ATTRIB);

	return NOERROR;
	}
