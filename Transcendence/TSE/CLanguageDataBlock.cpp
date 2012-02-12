//	CLanguageDataBlock.cpp
//
//	CLanguageDataBlock class

#include "PreComp.h"

#define MESSAGE_TAG								CONSTLIT("Message")
#define TEXT_TAG								CONSTLIT("Text")

#define ID_ATTRIB								CONSTLIT("id")
#define TEXT_ATTRIB								CONSTLIT("text")

CLanguageDataBlock::~CLanguageDataBlock (void)

//	CLanguageDataBlock destructor

	{
	DeleteAll();
	}

void CLanguageDataBlock::AddEntry (const CString &sID, const CString &sText)

//	AddEntry
//
//	Adds an entry to the data block

	{
	bool bIsNew;
	SEntry *pEntry = m_Data.Insert(sID, &bIsNew);

	//	If not new then we need to free the code block

	if (!bIsNew && pEntry->pCode)
		pEntry->pCode->Discard(&g_pUniverse->GetCC());

	//	Init

	pEntry->sText = sText;
	pEntry->pCode = NULL;
	}

void CLanguageDataBlock::DeleteAll (void)

//	DeleteAll
//
//	Deletes all entries

	{
	int i;
	CCodeChain *pCC = &g_pUniverse->GetCC();

	for (i = 0; i < m_Data.GetCount(); i++)
		if (m_Data[i].pCode)
			m_Data[i].pCode->Discard(pCC);

	m_Data.DeleteAll();
	}

ALERROR CLanguageDataBlock::InitFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	InitFromXML
//
//	Initializes from an XML block

	{
	int i;

	for (i = 0; i < pDesc->GetContentElementCount(); i++)
		{
		CXMLElement *pItem = pDesc->GetContentElement(i);
		CString sID = pItem->GetAttribute(ID_ATTRIB);
		if (sID.IsBlank())
			{
			Ctx.sError = strPatternSubst(CONSTLIT("Invalid id in <Language> block"));
			return ERR_FAIL;
			}

		if (strEquals(pItem->GetTag(), TEXT_TAG))
			{
			//	Link the code

			CCodeChainCtx CCCtx;
			ICCItem *pCode = CCCtx.Link(pItem->GetContentText(0), 0, NULL);
			if (pCode->IsError())
				{
				Ctx.sError = strPatternSubst(CONSTLIT("Language id: %s : %s"), sID, pCode->GetStringValue());
				return ERR_FAIL;
				}

			//	Add an entry

			bool bIsNew;
			SEntry *pEntry = m_Data.Insert(sID, &bIsNew);
			if (!bIsNew)
				{
				Ctx.sError = strPatternSubst(CONSTLIT("Duplicate <Language> element: %s"), sID);
				return ERR_FAIL;
				}

			//	If pCode is a string and not an identifier, then we can just
			//	store it directly.

			if (pCode->IsIdentifier() && pCode->IsQuoted())
				{
				pEntry->pCode = NULL;
				pEntry->sText = pCode->GetStringValue();
				}

			//	Otherwise we store the code

			else
				pEntry->pCode = pCode->Reference();

			//	Done

			CCCtx.Discard(pCode);
			}
		else if (strEquals(pItem->GetTag(), MESSAGE_TAG))
			{
			//	Add an entry

			bool bIsNew;
			SEntry *pEntry = m_Data.Insert(sID, &bIsNew);
			if (!bIsNew)
				{
				Ctx.sError = strPatternSubst(CONSTLIT("Duplicate <Language> element: %s"), sID);
				return ERR_FAIL;
				}

			//	Set the text

			pEntry->pCode = NULL;
			pEntry->sText = pItem->GetAttribute(TEXT_ATTRIB);
			}
		else
			{
			Ctx.sError = strPatternSubst(CONSTLIT("Invalid element in <Language> block: <%s>"), pItem->GetTag());
			return ERR_FAIL;
			}
		}

	return NOERROR;
	}

void CLanguageDataBlock::MergeFrom (const CLanguageDataBlock &Source)

//	MergeFrom
//
//	Merges entries from Source that we do not already have.

	{
	int i;

	for (i = 0; i < Source.m_Data.GetCount(); i++)
		{
		if (m_Data.GetAt(Source.m_Data.GetKey(i)) == NULL)
			{
			SEntry *pEntry = m_Data.Insert(Source.m_Data.GetKey(i));

			if (Source.m_Data[i].pCode)
				pEntry->pCode = Source.m_Data[i].pCode->Reference();
			else
				{
				pEntry->pCode = NULL;
				pEntry->sText = Source.m_Data[i].sText;
				}
			}
		}
	}

bool CLanguageDataBlock::Translate (CSpaceObject *pObj, const CString &sID, CString *retsText) const

//	Translate
//
//	Translates to text. If not found, returns FALSE.

	{
	//	If we can't find this ID then we can't translate

	SEntry *pEntry = m_Data.GetAt(sID);
	if (pEntry == NULL)
		return false;

	//	If we don't want the text back then all we need to do is return that we
	//	have the text.

	if (retsText == NULL)
		return true;

	//	If we don't need to run code then we just return the string.

	if (pEntry->pCode == NULL)
		{
		*retsText = pEntry->sText;
		return true;
		}

	//	Otherwise we have to run some code

	CCodeChainCtx Ctx;
	Ctx.SaveAndDefineSourceVar(pObj);
	Ctx.DefineString(CONSTLIT("aTextID"), sID);
	
	ICCItem *pResult = Ctx.Run(pEntry->pCode);
	*retsText = pResult->GetStringValue();
	Ctx.Discard(pResult);

	return true;
	}
