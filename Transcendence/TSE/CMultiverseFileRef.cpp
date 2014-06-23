//	CMultiverseFileRef.cpp
//
//	CMultiverseFileRef Class
//	Copyright (c) 2012 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"

#define FIELD_DIGEST							CONSTLIT("digest")
#define FIELD_FILE_PATH							CONSTLIT("filePath")
#define FIELD_ORIGINAL_FILENAME					CONSTLIT("originalFilename")
#define FIELD_UPLOADED_ON						CONSTLIT("uploadedOn")

#define ERR_INVALID_DIGEST						CONSTLIT("Unable to read file digest.")
#define ERR_INVALID_FILE_REF					CONSTLIT("Invalid file reference for catalog entry.")

ALERROR CMultiverseFileRef::InitFromJSON (const CJSONValue &Desc, CString *retsResult)

//	InitFromJSON
//
//	Initialize from JSON value.

	{
	m_sFilePath = Desc.GetElement(FIELD_FILE_PATH).AsString();
	m_sOriginalFilename = Desc.GetElement(FIELD_ORIGINAL_FILENAME).AsString();
	CString sUploadedOn = Desc.GetElement(FIELD_UPLOADED_ON).AsString();

	const CJSONValue &Digest = Desc.GetElement(FIELD_DIGEST);
	if (!Digest.IsNull())
		{
		if (!CHexarc::ConvertToIntegerIP(Digest, &m_Digest))
			{
			if (retsResult)
				*retsResult = ERR_INVALID_DIGEST;
			return ERR_FAIL;
			}
		}

	//	Initialize

	m_sFilespec = NULL_STR;
	m_dwSize = 0;

	//	Done

	return NOERROR;
	}

