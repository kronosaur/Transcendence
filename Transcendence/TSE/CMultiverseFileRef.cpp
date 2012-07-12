//	CMultiverseFileRef.cpp
//
//	CMultiverseFileRef Class
//	Copyright (c) 2012 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"

#define FIELD_FILE_PATH							CONSTLIT("filePath")
#define FIELD_UPLOADED_ON						CONSTLIT("uploadedOn")

#define ERR_INVALID_FILE_REF					CONSTLIT("Invalid file reference for catalog entry.")

ALERROR CMultiverseFileRef::InitFromJSON (const CJSONValue &Desc, CString *retsResult)

//	InitFromJSON
//
//	Initialize from JSON value.

	{
	m_sFilePath = Desc.GetElement(FIELD_FILE_PATH).AsString();
	CString sUploadedOn = Desc.GetElement(FIELD_UPLOADED_ON).AsString();

	//	Initialize

	m_sFilespec = NULL_STR;
	m_dwSize = 0;

	//	Done

	return NOERROR;
	}

