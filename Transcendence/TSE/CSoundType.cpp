//	CSoundType.cpp
//
//	CSoundType class
//	Copyright (c) 2013 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"

#define FILENAME_ATTRIB						CONSTLIT("filename")
#define LOCATION_CRITERIA_ATTRIB			CONSTLIT("locationCriteria")
#define PRIORITY_ATTRIB						CONSTLIT("priority")

bool CSoundType::FindDataField (const CString &sField, CString *retsValue)

//	FindDataField
//
//	Returns a data field

	{
	return false;
	}

CString CSoundType::GetFilespec (void) const

//	GetFilespec
//
//	Returns the filespec of the sound file.

	{
	return m_sFilespec;
	}

ALERROR CSoundType::OnCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	OnCreateFromXML
//
//	Initialize from XML

	{
	ALERROR error;

	CString sFilename = pDesc->GetAttribute(FILENAME_ATTRIB);
	if (!sFilename.IsBlank())
		m_sFilespec = Ctx.pResDb->ResolveFilespec(Ctx.sFolder, pDesc->GetAttribute(FILENAME_ATTRIB));

	//	Location criteria

	m_iPriority = pDesc->GetAttributeIntegerBounded(PRIORITY_ATTRIB, 0, -1, 0);

	if (error = m_LocationCriteria.Parse(pDesc->GetAttribute(LOCATION_CRITERIA_ATTRIB), 0, &Ctx.sError))
		return ComposeLoadError(Ctx, Ctx.sError);

	return NOERROR;
	}
