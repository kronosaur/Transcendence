//	CMultiverseCatalogEntry.cpp
//
//	CMultiverseCatalogEntry Class
//	Copyright (c) 2012 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"

#define FIELD_DESCRIPTION						CONSTLIT("description")
#define FIELD_FILE_TDB							CONSTLIT("fileTDB")
#define FIELD_LICENSE_TYPE						CONSTLIT("licenseType")
#define FIELD_NAME								CONSTLIT("name")
#define FIELD_RELEASE							CONSTLIT("release")
#define FIELD_RESOURCES							CONSTLIT("resources")
#define FIELD_TYPE								CONSTLIT("type")
#define FIELD_UNID								CONSTLIT("unid")
#define FIELD_VERSION							CONSTLIT("$version")

#define LICENSE_AUTO							CONSTLIT("auto")
#define LICENSE_CORE							CONSTLIT("core")
#define LICENSE_FREE							CONSTLIT("free")
#define LICENSE_PAID							CONSTLIT("paid")

#define STR_UNID_PREFIX							CONSTLIT("Transcendence:")

#define TYPE_ADVENTURE							CONSTLIT("transcendenceAdventure")
#define TYPE_LIBRARY							CONSTLIT("transcendenceLibrary")

#define ERR_INVALID_UNID						CONSTLIT("Invalid UNID for catalog entry.")

CMultiverseCatalogEntry::CMultiverseCatalogEntry (const CMultiverseCatalogEntry &Src)

//	CMultiverseCatalogEntry constructor

	{
	m_sUNID = Src.m_sUNID;
	m_dwRelease = Src.m_dwRelease;
	m_dwVersion = Src.m_dwVersion;
	m_iType = Src.m_iType;

	m_sName = Src.m_sName;
	m_sDesc = Src.m_sDesc;
	m_TDBFile = Src.m_TDBFile;
	m_iLicenseType = Src.m_iLicenseType;

	m_dwUNID = Src.m_dwUNID;
	m_iStatus = Src.m_iStatus;

	//	This is initialized in a call to CExtensionCollection, so we don't bother
	//	copying it here.

	m_pIcon = NULL;
	}

CMultiverseCatalogEntry::~CMultiverseCatalogEntry (void)

//	CMultiverseCatalogEntry destructor

	{
	if (m_pIcon)
		delete m_pIcon;
	}

ALERROR CMultiverseCatalogEntry::CreateFromJSON (const CJSONValue &Entry, CMultiverseCatalogEntry **retpEntry, CString *retsResult)

//	CreateFromJSON
//
//	Creates a new entry from a JSON value.

	{
	int i;

	CMultiverseCatalogEntry *pNewEntry = new CMultiverseCatalogEntry;

	pNewEntry->m_sUNID = Entry.GetElement(FIELD_UNID).AsString();
	if (pNewEntry->m_sUNID.IsBlank())
		{
		delete pNewEntry;
		*retsResult = ERR_INVALID_UNID;
		return ERR_FAIL;
		}

	pNewEntry->m_dwRelease = (DWORD)Entry.GetElement(FIELD_RELEASE).AsInt32();
	pNewEntry->m_dwVersion = (DWORD)Entry.GetElement(FIELD_VERSION).AsInt32();

	pNewEntry->m_sName = Entry.GetElement(FIELD_NAME).AsString();
	pNewEntry->m_sDesc = Entry.GetElement(FIELD_DESCRIPTION).AsString();

	//	Parse the fully qualified UNID and get just the hex UNID.

	if (strStartsWith(pNewEntry->m_sUNID, STR_UNID_PREFIX))
		{
		pNewEntry->m_dwUNID = strParseIntOfBase(pNewEntry->m_sUNID.GetASCIIZPointer() + STR_UNID_PREFIX.GetLength(), 16, 0);
		if (pNewEntry->m_dwUNID == 0)
			{
			delete pNewEntry;
			*retsResult = ERR_INVALID_UNID;
			return ERR_FAIL;
			}
		}
	else
		{
		delete pNewEntry;
		*retsResult = ERR_INVALID_UNID;
		return ERR_FAIL;
		}

	//	Get the type

	CString sType = Entry.GetElement(FIELD_TYPE).AsString();
	if (strEquals(sType, TYPE_ADVENTURE))
		pNewEntry->m_iType = extAdventure;
	else if (strEquals(sType, TYPE_LIBRARY))
		pNewEntry->m_iType = extLibrary;
	else
		pNewEntry->m_iType = extExtension;

	//	Get the license type

	CString sLicense = Entry.GetElement(FIELD_LICENSE_TYPE).AsString();
	if (strEquals(sLicense, LICENSE_AUTO))
		pNewEntry->m_iLicenseType = licenseAuto;
	else if (strEquals(sLicense, LICENSE_CORE))
		pNewEntry->m_iLicenseType = licenseCore;
	else if (strEquals(sLicense, LICENSE_FREE))
		pNewEntry->m_iLicenseType = licenseFree;
	else if (strEquals(sLicense, LICENSE_PAID))
		pNewEntry->m_iLicenseType = licensePaid;
	else
		pNewEntry->m_iLicenseType = licenseUnknown;

	//	Get the TDB file (only if not core)

	if (pNewEntry->m_iLicenseType != licenseCore)
		{
		if (pNewEntry->m_TDBFile.InitFromJSON(Entry.GetElement(FIELD_FILE_TDB), retsResult) != NOERROR)
			{
			delete pNewEntry;
			return ERR_FAIL;
			}
		}

	//	Get the resources

	const CJSONValue &Resources = Entry.GetElement(FIELD_RESOURCES);
	if (Resources.GetType() == CJSONValue::typeArray)
		{
		pNewEntry->m_Resources.InsertEmpty(Resources.GetCount());

		for (i = 0; i < Resources.GetCount(); i++)
			{
			if (pNewEntry->m_Resources[i].InitFromJSON(Resources.GetElement(i), retsResult) != NOERROR)
				{
				delete pNewEntry;
				return ERR_FAIL;
				}
			}
		}

	//	Initialize status

	pNewEntry->m_iStatus = statusUnknown;
	pNewEntry->m_pIcon = NULL;

	//	Done

	*retpEntry = pNewEntry;
	return NOERROR;
	}

bool CMultiverseCatalogEntry::IsValid (void)

//	IsValid
//
//	Returns TRUE if the entry is a valid entry (has all necessary fields).

	{
	//	We must have a TDB file (unless this is a core entry)

	if (m_TDBFile.IsEmpty()
			&& m_iLicenseType != licenseCore)
		return false;

	return true;
	}

void CMultiverseCatalogEntry::SetIcon (CG16bitImage *pImage)

//	SetIcon
//
//	Sets the icon. We take ownership of the image

	{
	if (m_pIcon)
		delete m_pIcon;

	m_pIcon = pImage;
	}
