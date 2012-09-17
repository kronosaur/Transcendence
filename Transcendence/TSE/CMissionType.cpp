//	CMissionType.cpp
//
//	CMissionType class

#include "PreComp.h"

#define NAME_ATTRIB								CONSTLIT("name")
#define NO_FAILURE_ON_OWNER_DESTROYED_ATTRIB	CONSTLIT("noFailureOnOwnerDestroyed")

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
	m_fNoFailureOnOwnerDestroyed = pDesc->GetAttributeBool(NO_FAILURE_ON_OWNER_DESTROYED_ATTRIB);

	return NOERROR;
	}
