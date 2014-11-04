//	CAttributeDataBlock.cpp
//
//	CAttributeDataBlock class

#include "PreComp.h"

#define DATA_ATTRIB								CONSTLIT("data")
#define ID_ATTRIB								CONSTLIT("id")

CAttributeDataBlock::CAttributeDataBlock (void) :
		m_pData(NULL),
		m_pObjRefData(NULL)

//	CAttributeDataBlock constructor

	{
	}

CAttributeDataBlock::CAttributeDataBlock (const CAttributeDataBlock &Src) :
		m_pData(NULL),
		m_pObjRefData(NULL)

//	CAttributeDataBlock constructor

	{
	Copy(Src);
	}

CAttributeDataBlock::~CAttributeDataBlock (void)

//	CAttributeDataBlock destructor

	{
	CleanUp();
	}

CAttributeDataBlock &CAttributeDataBlock::operator= (const CAttributeDataBlock &Src)

//	CAttributeDataBlock equals operator

	{
	Copy(Src);
	return *this;
	}

void CAttributeDataBlock::CleanUp (void)

//	CleanUp
//
//	Destroy all entries

	{
	if (m_pData)
		{
		delete m_pData;
		m_pData = NULL;
		}

	SObjRefEntry *pNext = m_pObjRefData;
	while (pNext)
		{
		SObjRefEntry *pDeleteMe = pNext;
		pNext = pNext->pNext;
		delete pDeleteMe;
		}

	m_pObjRefData = NULL;
	}

void CAttributeDataBlock::Copy (const CAttributeDataBlock &Copy)

//	Copy
//
//	Copies

	{
	CleanUp();

	//	Copy data

	if (Copy.m_pData)
		{
		CString sSave;
		CObject::Flatten(Copy.m_pData, &sSave);

		if (!sSave.IsBlank())
			CObject::Unflatten(sSave, (CObject **)&m_pData);
		}

	//	Copy object references

	SObjRefEntry *pSrcNext = Copy.m_pObjRefData;
	SObjRefEntry *pDest = NULL;
	while (pSrcNext)
		{
		SObjRefEntry *pNew = new SObjRefEntry;
		pNew->sName = pSrcNext->sName;
		pNew->pObj = pSrcNext->pObj;
		pNew->dwObjID = pSrcNext->dwObjID;
		pNew->pNext = NULL;

		if (pDest == NULL)
			m_pObjRefData = pNew;
		else
			pDest->pNext = pNew;

		pDest = pNew;
		pSrcNext = pSrcNext->pNext;
		}
	}

bool CAttributeDataBlock::FindData (const CString &sAttrib, const CString **retsData) const

//	FindData
//
//	Finds string data

	{
	if (m_pData)
		{
		CString *pData;
		if (m_pData->Lookup(sAttrib, (CObject **)&pData) == NOERROR)
			{
			if (retsData)
				*retsData = pData;

			return true;
			}
		else
			return false;
		}
	else
		return false;
	}

bool CAttributeDataBlock::FindObjRefData (CSpaceObject *pObj, CString *retsAttrib) const

//	FindObjRefData
//
//	Find the attribute that has the given object

	{
	SObjRefEntry *pNext = m_pObjRefData;
	while (pNext)
		{
		if (pNext->pObj == pObj)
			{
			if (retsAttrib)
				*retsAttrib = pNext->sName;
			return true;
			}
		pNext = pNext->pNext;
		}

	return false;
	}

const CString &CAttributeDataBlock::GetData (const CString &sAttrib) const

//	GetData
//
//	Returns string data associated with attribute

	{
	if (m_pData)
		{
		CString *pData;
		if (m_pData->Lookup(sAttrib, (CObject **)&pData) == NOERROR)
			return *pData;
		else
			return NULL_STR;
		}
	else
		return NULL_STR;
	}

CSpaceObject *CAttributeDataBlock::GetObjRefData (const CString &sAttrib) const

//	GetObjRefData
//
//	Returns CSpaceObject data associated with attribute

	{
	SObjRefEntry *pEntry = m_pObjRefData;
	while (pEntry)
		{
		if (strEquals(sAttrib, pEntry->sName))
			{
			if (pEntry->pObj == NULL && pEntry->dwObjID != OBJID_NULL)
				pEntry->pObj = g_pUniverse->FindObject(pEntry->dwObjID);

			return pEntry->pObj;
			}

		pEntry = pEntry->pNext;
		}

	return NULL;
	}

bool CAttributeDataBlock::IsEqual (const CAttributeDataBlock &Src)

//	IsEqual
//
//	Returns TRUE if the two data blocks are the same

	{
	int i;

	if ((m_pData == NULL) != (Src.m_pData == NULL))
		return false;

	if (m_pData)
		{
		if (m_pData->GetCount() != Src.m_pData->GetCount())
			return false;

		for (i = 0; i < m_pData->GetCount(); i++)
			{
			if (!strEquals(m_pData->GetKey(i), Src.m_pData->GetKey(i)))
				return false;

			CString *pDest = (CString *)m_pData->GetValue(i);
			CString *pSrc = (CString *)Src.m_pData->GetValue(i);

			if (!strEquals(*pDest, *pSrc))
				return false;
			}
		}

	SObjRefEntry *pDest = m_pObjRefData;
	SObjRefEntry *pSrc = Src.m_pObjRefData;
	while (pDest && pSrc)
		{
		if (!strEquals(pDest->sName, pSrc->sName))
			return false;

		if (pDest->dwObjID != pSrc->dwObjID)
			return false;

		pDest = pDest->pNext;
		pSrc = pSrc->pNext;
		}

	return (pDest == NULL && pSrc == NULL);
	}

bool CAttributeDataBlock::IsXMLText (const CString &sData) const

//	IsXMLText
//
//	Returns TRUE if the given data value is an XML source stream.

	{
	char *pPos = sData.GetASCIIZPointer();

	while (strIsWhitespace(pPos))
		pPos++;

	return (*pPos == '<');
	}

void CAttributeDataBlock::LoadObjReferences (CSystem *pSystem)

//	LoadObjReferences
//
//	Load all the object references now that it's safe

	{
	SObjRefEntry *pEntry = m_pObjRefData;
	while (pEntry)
		{
		if (pEntry->pObj == NULL && pEntry->dwObjID != OBJID_NULL)
			pEntry->pObj = pSystem->FindObject(pEntry->dwObjID);

		pEntry = pEntry->pNext;
		}
	}

void CAttributeDataBlock::MergeFrom (const CAttributeDataBlock &Src)

//	MergeFrom
//
//	Merges data from source

	{
	int i;

	//	Plain data

	if (Src.m_pData)
		{
		for (i = 0; i < Src.m_pData->GetCount(); i++)
			{
			CString *pValue = (CString *)Src.m_pData->GetValue(i);
			SetData(Src.m_pData->GetKey(i), *pValue);
			}
		}

	//	Object reference

	SObjRefEntry *pEntry = Src.m_pObjRefData;
	while (pEntry)
		{
		SetObjRefData(pEntry->sName, pEntry->pObj);

		pEntry = pEntry->pNext;
		}
	}

void CAttributeDataBlock::OnObjDestroyed (CSpaceObject *pObj)

//	OnObjDestroyed
//
//	Object has been destroyed

	{
	DWORD dwObjID = pObj->GetID();
	SObjRefEntry *pEntry = m_pObjRefData;
	while (pEntry)
		{
		if (pEntry->dwObjID == dwObjID || pObj == pEntry->pObj)
			{
			pEntry->dwObjID = OBJID_NULL;
			pEntry->pObj = NULL;
			}

		pEntry = pEntry->pNext;
		}
	}

void CAttributeDataBlock::OnSystemChanged (CSystem *pSystem)

//	OnSystemChanged
//
//	We are now in a different system, so we need to clear out any pointers that
//	we might have to old system objects.

	{
	SObjRefEntry *pEntry = m_pObjRefData;
	while (pEntry)
		{
		pEntry->pObj = NULL;
		pEntry = pEntry->pNext;
		}
	}

void CAttributeDataBlock::ReadFromStream (SLoadCtx &Ctx)

//	ReadFromStream
//
//	Reads from a stream
//
//	CString		m_pData (flattened)
//
//	DWORD		No of obj references
//	CString		ref: name
//	DWORD		ref: pointer (CSpaceObject ref)

	{
	DeleteAll();

	//	Load the opaque data table

	CString sData;
	sData.ReadFromStream(Ctx.pStream);
	if (!sData.IsBlank())
		CObject::Unflatten(sData, (CObject **)&m_pData);

	//	Load object reference

	DWORD dwCount;
	Ctx.pStream->Read((char *)&dwCount, sizeof(DWORD));
	if (dwCount)
		{
		SObjRefEntry *pPrev = NULL;
		for (int i = 0; i < (int)dwCount; i++)
			{
			SObjRefEntry *pEntry = new SObjRefEntry;
			if (pPrev)
				pPrev->pNext = pEntry;
			else
				m_pObjRefData = pEntry;

			pEntry->sName.ReadFromStream(Ctx.pStream);

			//	For later versions don't bind the object until someone asks for it

			if (Ctx.dwVersion >= 41)
				{
				Ctx.pStream->Read((char *)&pEntry->dwObjID, sizeof(DWORD));
				pEntry->pObj = NULL;
				}
			else
				{
				CSystem::ReadObjRefFromStream(Ctx, &pEntry->pObj);
				pEntry->dwObjID = OBJID_NULL;
				}

			pEntry->pNext = NULL;

			pPrev = pEntry;
			}
		}
	}

void CAttributeDataBlock::ReadFromStream (IReadStream *pStream)

//	ReadFromStream
//
//	Reads from a stream
//
//	CString		m_pData (flattened)
//
//	DWORD		No of obj references
//	CString		ref: name
//	DWORD		ref: pointer (CSpaceObject ref)

	{
	DeleteAll();

	//	Load the opaque data table

	CString sData;
	sData.ReadFromStream(pStream);
	if (!sData.IsBlank())
		CObject::Unflatten(sData, (CObject **)&m_pData);

	//	Load object reference (since we don't have a system,
	//	we ignore all references)

	DWORD dwCount;
	pStream->Read((char *)&dwCount, sizeof(DWORD));
	if (dwCount)
		{
		ASSERT(false);

		for (int i = 0; i < (int)dwCount; i++)
			{
			CString sDummy;
			DWORD dwDummy;

			sDummy.ReadFromStream(pStream);
			pStream->Read((char *)&dwDummy, sizeof(DWORD));
			}
		}
	}

void CAttributeDataBlock::SetData (const CString &sAttrib, const CString &sData)

//	SetData
//
//	Sets string data associated with attribute

	{
	if (m_pData == NULL)
		m_pData = new CSymbolTable(TRUE, FALSE);

	CString *pData = new CString(sData);
	m_pData->ReplaceEntry(sAttrib, pData, TRUE, NULL);
	}

void CAttributeDataBlock::SetFromXML (CXMLElement *pData)

//	SetFromXML
//
//	Initializes data from an XML element

	{
	if (pData)
		{
		for (int i = 0; i < pData->GetContentElementCount(); i++)
			{
			CXMLElement *pItem = pData->GetContentElement(i);
			CString sID;
			CString sData;

			//	ID of data is either in id= attrib or element tag.

			if (!pItem->FindAttribute(ID_ATTRIB, &sID))
				sID = pItem->GetTag();

			//	Dats is either in data= attrib or content text

			sData = pItem->GetAttribute(DATA_ATTRIB);
			if (sData.IsBlank())
				{
				sData = pItem->GetContentText(0);

				//	If this item begins with '<' then we quote it (and escape it) so that it
				//	can be a TLisp string.

				if (IsXMLText(sData))
					sData = CCString::Print(sData);
				}

			//	Store

			SetData(sID, sData);
			}
		}
	}

void CAttributeDataBlock::SetObjRefData (const CString &sAttrib, CSpaceObject *pObj)

//	SetObjRefData
//
//	Sets CSpaceObject data associated with attribute

	{
	DWORD dwObjID = (pObj ? pObj->GetID() : OBJID_NULL);

	SObjRefEntry *pEntry = m_pObjRefData;
	while (pEntry)
		{
		if (strEquals(sAttrib, pEntry->sName))
			{
			pEntry->pObj = pObj;
			pEntry->dwObjID = dwObjID;
			return;
			}

		pEntry = pEntry->pNext;
		}

	pEntry = new SObjRefEntry;
	pEntry->sName = sAttrib;
	pEntry->pObj = pObj;
	pEntry->dwObjID = dwObjID;
	pEntry->pNext = m_pObjRefData;
	m_pObjRefData = pEntry;
	}

void CAttributeDataBlock::WriteToStream (IWriteStream *pStream, CSystem *pSystem)

//	WriteToStream
//
//	Write the object to a stream
//
//	CString		m_pData (flattened)
//
//	DWORD		No of obj references
//	CString		ref: name
//	DWORD		ref: pointer (CSpaceObject ref)

	{
	//	Write out the opaque data

	CString sSave;
	if (m_pData)
		CObject::Flatten(m_pData, &sSave);
	sSave.WriteToStream(pStream);

	//	Write out object references

	DWORD dwCount = 0;

	if (pSystem)
		{
		SObjRefEntry *pEntry = m_pObjRefData;
		while (pEntry)
			{
			dwCount++;
			pEntry = pEntry->pNext;
			}

		pStream->Write((char *)&dwCount, sizeof(DWORD));

		pEntry = m_pObjRefData;
		while (pEntry)
			{
			pEntry->sName.WriteToStream(pStream);

			//	For previous versions we sometimes load the object and don't
			//	yet have the ID

			if (pEntry->pObj && pEntry->dwObjID == OBJID_NULL)
				pEntry->dwObjID = pEntry->pObj->GetID();

			pStream->Write((char *)&pEntry->dwObjID, sizeof(DWORD));
			pEntry = pEntry->pNext;
			}
		}
	else
		pStream->Write((char *)&dwCount, sizeof(DWORD));
	}
