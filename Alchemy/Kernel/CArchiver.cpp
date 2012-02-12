//	CArchiver.cpp
//
//	Implements CArchiver object
//
//	The structure of the archive is:
//
//		ARCHIVEHEADER
//		[external refs]
//		[objects]

#include "Kernel.h"
#include "KernelObjID.h"

#define ARCHIVE_SIGNATURE					'ALOA'
#define ARCHIVE_VERSION						1

typedef struct
	{
	DWORD dwSignature;									//	Always 'ALOA'
	DWORD dwVersion;									//	Archive version
	DWORD dwCount;										//	Number of objects in the archive
	DWORD dwReferences;									//	Number of references
	DWORD dwUserVersion;								//	Version of data
	DWORD dwSpare[7];
	} ARCHIVEHEADER, *PARCHIVEHEADER;

static CObjectClass<CArchiver>g_ArchiverClass(OBJID_CARCHIVER, NULL);
static CObjectClass<CUnarchiver>g_UnarchiverClass(OBJID_CUNARCHIVER, NULL);

CArchiver::CArchiver (void) :
		CObject(&g_ArchiverClass)

//	CArchiver constructor

	{
	}

CArchiver::CArchiver (IWriteStream *pStream) :
		CObject(&g_ArchiverClass),
		m_pStream(pStream),
		m_List(FALSE),
		m_ExternalReferences(FALSE, TRUE),
		m_dwVersion(0)

//	CArchiver constructor

	{
	}

CArchiver::~CArchiver (void)

//	CArchiver destructor

	{
	}

ALERROR CArchiver::AddExternalReference (CString sTag, void *pReference)

//	AddExternalReference
//
//	Adds an external reference to the archive. This must only
//	be called after BeingArchive and before EndArchive.

	{
	ALERROR error;
	int iID;

	//	Convert to ID

	if (error = Reference2ID(pReference, &iID))
		return error;

	//	Store

	if (error = m_ExternalReferences.AddEntry(sTag, (CObject *)iID))
		return error;

	return NOERROR;
	}

ALERROR CArchiver::AddObject (CObject *pObject)

//	AddObject
//
//	Adds an object to the archive. BeginArchive must be called
//	first. Note that any objects passed in to this must
//	remain valid until the call to EndArchive. The archiver does
//	not own the objects; the caller must free them.

	{
	ALERROR error;
	int iID;

	//	Assign a reference ID to this pointer

	if (error = Reference2ID(pObject, &iID))
		return error;

	return m_List.AppendObject(pObject, NULL);
	}

ALERROR CArchiver::BeginArchive (void)

//	BeginArchive
//
//	This must be called before any call to AddObject. Call
//	EndArchive to finish.

	{
	//	Initialize reference IDs

	m_iNextID = 0;
	return NOERROR;
	}

ALERROR CArchiver::EndArchive (void)

//	EndArchiver
//
//	This is called to finish archiving.

	{
	ALERROR error;
	ARCHIVEHEADER archiveheader;
	int i;

	//	Open up the file

	if (error = m_pStream->Create())
		return error;

	utlMemSet(&archiveheader, sizeof(archiveheader), 0);
	archiveheader.dwSignature = ARCHIVE_SIGNATURE;
	archiveheader.dwVersion = ARCHIVE_VERSION;
	archiveheader.dwCount = m_List.GetCount();
	archiveheader.dwReferences = m_ReferenceList.GetCount();
	archiveheader.dwUserVersion = m_dwVersion;

	//	IDs must be allocated sequentially

	ASSERT(archiveheader.dwReferences == (DWORD)m_iNextID);

	//	Write out the header

	if (error = m_pStream->Write((char *)&archiveheader, sizeof(archiveheader), NULL))
		{
		m_pStream->Close();
		return error;
		}

	//	Write out the external references symbol table

	if (error = m_ExternalReferences.Save(this))
		{
		m_pStream->Close();
		return error;
		}

	//	Write out each of the objects

	for (i = 0; i < m_List.GetCount(); i++)
		{
		CObject *pObject = m_List.GetObject(i);

		if (error = SaveObject(pObject))
			{
			m_pStream->Close();
			return error;
			}
		}

	//	Done

	if (error = m_pStream->Close())
		return error;

	return NOERROR;
	}

ALERROR CArchiver::Reference2ID (void *pReference, int *retiID)

//	Reference2ID
//
//	Converts a reference to an ID that can be stored on disk

	{
	ALERROR error;
	BOOL bFound;
	int iID;

	//	We always map NULL to -1

	if (pReference == NULL)
		{
		*retiID = -1;
		return NOERROR;
		}

	//	Look for the pointer in our table

	if (error = m_ReferenceList.FindOrAdd((int)pReference, m_iNextID, &bFound, &iID))
		return error;

	//	If we found it, then return the value. Otherwise, m_iNextID
	//	got added as the new ID

	if (bFound)
		*retiID = iID;
	else
		{
		*retiID = m_iNextID;
		m_iNextID++;
		}

	return NOERROR;
	}

ALERROR CArchiver::SaveObject (CObject *pObject)

//	SaveObject
//
//	Saves an object to the stream

	{
	ALERROR error;
	int iID;

	//	Write out the object ID

	if (error = Reference2ID(pObject, &iID))
		return error;

	if (error = m_pStream->Write((char *)&iID, sizeof(iID), NULL))
		return error;

	//	We let the object save itself to our stream.

	if (error = pObject->Save(this))
		return error;

	return NOERROR;
	}

ALERROR CArchiver::WriteData (char *pData, int iLength)

//	WriteData
//
//	This method is used by objects to store their own data. It must only
//	be called by an object's Save method

	{
	return m_pStream->Write(pData, iLength, NULL);
	}

//	Unarchiver class ----------------------------------------------------------

CUnarchiver::CUnarchiver (void) :
		CObject(&g_UnarchiverClass)

//	CUnarchiver constructor

	{
	}

CUnarchiver::CUnarchiver (IReadStream *pStream) :
		CObject(&g_UnarchiverClass),
		m_pStream(pStream),
		m_List(FALSE),
		m_pExternalReferences(NULL),
		m_dwMinVersion(0),
		m_dwVersion(0)

//	CUnarchiver constructor

	{
	}

CUnarchiver::~CUnarchiver (void)

//	CUnarchiver destructor

	{
	if (m_pExternalReferences)
		delete m_pExternalReferences;
	}

ALERROR CUnarchiver::BeginUnarchive (void)

//	BeginUnarchive
//
//	Loads all the objects from the archive

	{
	ALERROR error;
	ARCHIVEHEADER archiveheader;
	int i;
	BOOL bCloseStreamOnError = FALSE;
	DWORD dwDummy;

	//	Open up the file

	if (error = m_pStream->Open())
		goto Fail;

	bCloseStreamOnError = TRUE;

	//	Read the header

	if (error = m_pStream->Read((char *)&archiveheader, sizeof(archiveheader), NULL))
		goto Fail;

	//	If this is not the right signature, fail

	if (archiveheader.dwSignature != ARCHIVE_SIGNATURE)
		{
		error = ERR_FAIL;
		goto Fail;
		}

	//	If this is not the right version, fail

	if (archiveheader.dwVersion != ARCHIVE_VERSION)
		{
		error = ERR_FAIL;
		goto Fail;
		}

	//	If this is not the right user version, fail

	if (archiveheader.dwVersion < m_dwMinVersion)
		{
		error = ERR_FAIL;
		goto Fail;
		}

	//	Make room in the reference table. Note that this is not guaranteed
	//	to be all the references in the file, just a good hint.

	m_ReferenceList.RemoveAll();
	if (error = m_ReferenceList.ExpandArray(0, (int)archiveheader.dwReferences))
		goto Fail;

	for (i = 0; i < m_ReferenceList.GetCount(); i++)
		m_ReferenceList.ReplaceElement(i, -1);

	//	Load the external reference table

	if (m_pExternalReferences)
		delete m_pExternalReferences;

	m_pExternalReferences = new CSymbolTable;
	if (m_pExternalReferences == NULL)
		{
		error = ERR_MEMORY;
		goto Fail;
		}

	//	Skip over the object's class ID because we know that
	//	this is an external reference object

	if (error = m_pStream->Read((char *)&dwDummy, sizeof(DWORD), NULL))
		goto Fail;

	if (error = m_pExternalReferences->Load(this))
		goto Fail;

	//	Load the number of objects given

	for (i = 0; i < (int)archiveheader.dwCount; i++)
		{
		CObject *pObject;

		if (error = LoadObject(&pObject))
			goto Fail;

		//	Add the object to our list

		if (error = m_List.AppendObject(pObject, NULL))
			{
			delete pObject;
			goto Fail;
			}
		}

	m_pStream->Close();

	return NOERROR;

Fail:

	if (bCloseStreamOnError)
		m_pStream->Close();

	return error;
	}

ALERROR CUnarchiver::EndUnarchive (void)

//	EndArchive
//
//	Fixup references

	{
	ALERROR error;
	int i;

	for (i = 0; i < m_FixupTable.GetCount(); i += 2)
		{
		void **pReferenceDest;
		int iID, iRef;

		pReferenceDest = (void **)m_FixupTable.GetElement(i);
		iID = m_FixupTable.GetElement(i + 1);

		//	Look for this ID

		iRef = m_ReferenceList.GetElement(iID);
		if (iRef == -1)
			return ERR_FAIL;

		*pReferenceDest = (void *)iRef;
		}

	//	Let each object know that we're done loading

	for (i = 0; i < m_List.GetCount(); i++)
		{
		CObject *pObject = m_List.GetObject(i);

		if (error = pObject->LoadDone())
			return error;
		}

	return NOERROR;
	}

CObject *CUnarchiver::GetObject (int iIndex)

//	GetObject
//
//	Returns the nth object loaded by the unarchiver. If there is
//	no object at that index, it returns NULL

	{
	if (iIndex < 0 || iIndex >= m_List.GetCount())
		return NULL;

	return m_List.GetObject(iIndex);
	}

ALERROR CUnarchiver::LoadObject (CObject **retpObject)

//	LoadObject
//
//	Loads the object from the stream

	{
	ALERROR error;
	DWORD dwID, dwReferenceID;
	IObjectClass *pClass;
	CObject *pObject;

	//	Load the object's instance ID

	if (error = m_pStream->Read((char *)&dwReferenceID, sizeof(DWORD), NULL))
		return error;

	//	Load the object's class ID

	if (error = m_pStream->Read((char *)&dwID, sizeof(DWORD), NULL))
		return error;

	//	Create a new object with this ID

	pClass = CObjectClassFactory::GetClass(dwID);
	if (pClass == NULL)
		return ERR_CLASSNOTFOUND;

	pObject = pClass->Instantiate();
	if (pObject == NULL)
		return ERR_FAIL;

	//	Load the object

	if (error = pObject->Load(this))
		{
		delete pObject;
		return error;
		}

	//	Now that we've got the object, associate the ID with the
	//	pointer. Note that we may need to grow the reference table

	if (dwReferenceID >= (DWORD)m_ReferenceList.GetCount())
		{
		int iGrow = 1 + ((int)dwReferenceID - m_ReferenceList.GetCount());
		int iPos = m_ReferenceList.GetCount();
		int i;

		if (error = m_ReferenceList.ExpandArray(iPos, iGrow))
			{
			delete pObject;
			return error;
			}

		//	Fill new part of table with -1

		for (i = iPos; i < iPos + iGrow; i++)
			m_ReferenceList.ReplaceElement(i, -1);
		}

	m_ReferenceList.ReplaceElement((int)dwReferenceID, (int)pObject);

	//	Done

	*retpObject = pObject;

	return NOERROR;
	}

ALERROR CUnarchiver::ReadData (char *pData, int iLength)

//	ReadData
//
//	This method is used by objects to read their own data. It must only
//	be called by an object's Load method.

	{
	return m_pStream->Read(pData, iLength, NULL);
	}

ALERROR CUnarchiver::ResolveReference (int iID, void **pReferenceDest)

//	ResolveReference
//
//	Converts from an ID to a reference. If the reference
//	has not been resolved yet, we add an entry to the fixup table

	{
	ALERROR error;
	int iRef;

	//	An ID of -1 is always a NULL

	if (iID == -1)
		{
		*pReferenceDest = NULL;
		return NOERROR;
		}

	//	Make sure we're not out of range

	if (iID < 0)
		return ERR_FAIL;

	//	Look up the ID. If we found it, then we're done

	if (iID < m_ReferenceList.GetCount())
		{
		iRef = m_ReferenceList.GetElement(iID);
		if (iRef != -1)
			{
			*pReferenceDest = (void *)iRef;
			return NOERROR;
			}
		}

	//	If we could not find it, add it to our fixup table

	if (error = m_FixupTable.AppendElement((int)pReferenceDest, NULL))
		return error;

	if (error = m_FixupTable.AppendElement(iID, NULL))
		return error;

	return NOERROR;
	}

ALERROR CUnarchiver::ResolveExternalReference (CString sTag, void *pReference)

//	ResolveExternalReference
//
//	Associates a pointer to a saved reference

	{
	ALERROR error;
	CObject *pValue;
	int iID;

	//	Look for the reference in our symbol table

	if (error = m_pExternalReferences->Lookup(sTag, &pValue))
		return error;

	//	Add the reference

	iID = (int)pValue;
	if (iID < 0 || iID >= m_ReferenceList.GetCount())
		return ERR_FAIL;

	m_ReferenceList.ReplaceElement(iID, (int)pReference);

	return NOERROR;
	}

