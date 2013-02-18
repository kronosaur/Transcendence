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
	SEntry *pEntry = m_Data.SetAt(sID, &bIsNew);

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
			SEntry *pEntry = m_Data.SetAt(sID, &bIsNew);
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
			SEntry *pEntry = m_Data.SetAt(sID, &bIsNew);
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

CLanguageDataBlock::ETranslateResult CLanguageDataBlock::Translate (CSpaceObject *pObj, const CString &sID, TArray<CString> *retText, CString *retsText) const

//	Translate
//
//	Translates an entry to either a string or an array of strings.

	{
	int i;

	//	If we can't find this ID then we can't translate

	SEntry *pEntry = m_Data.GetAt(sID);
	if (pEntry == NULL)
		return resultNotFound;

	//	If we don't want the text back then all we need to do is return that we
	//	have the text.

	if (retText == NULL && retsText == NULL)
		return resultFound;

	//	If we don't need to run code then we just return the string.

	if (pEntry->pCode == NULL)
		{
		if (retsText)
			{
			*retsText = ::ComposePlayerNameString(pEntry->sText, g_pUniverse->GetPlayerName(), g_pUniverse->GetPlayerGenome());
			return resultString;
			}
		else
			return resultFound;
		}

	//	Otherwise we have to run some code

	CCodeChainCtx Ctx;
	Ctx.SaveAndDefineSourceVar(pObj);
	Ctx.DefineString(CONSTLIT("aTextID"), sID);
	
	ICCItem *pResult = Ctx.Run(pEntry->pCode);	//	LATER:Event
	ETranslateResult iResult;

	//	Nil

	if (pResult->IsNil())
		iResult = resultNotFound;

	//	List of strings

	else if (pResult->GetCount() > 1)
		{
		if (retText)
			{
			CString sPlayerName = g_pUniverse->GetPlayerName();
			GenomeTypes iPlayerGenome = g_pUniverse->GetPlayerGenome();

			retText->DeleteAll();

			retText->InsertEmpty(pResult->GetCount());
			for (i = 0; i < pResult->GetCount(); i++)
				retText->GetAt(i) = ::ComposePlayerNameString(pResult->GetElement(i)->GetStringValue(), sPlayerName, iPlayerGenome);

			iResult = resultArray;
			}
		else
			iResult = resultFound;
		}

	//	String

	else
		{
		if (retsText)
			{
			*retsText = ::ComposePlayerNameString(pResult->GetStringValue(), g_pUniverse->GetPlayerName(), g_pUniverse->GetPlayerGenome());
			iResult = resultString;
			}
		else
			iResult = resultFound;
		}

	//	Done

	Ctx.Discard(pResult);

	return iResult;
	}

bool CLanguageDataBlock::Translate (CSpaceObject *pObj, const CString &sID, ICCItem **retpResult) const

//	Translate
//
//	Translates to an item. The caller is responsible for discarding.

	{
	int i;

	TArray<CString> List;
	CString sText;
	ETranslateResult iResult = Translate(pObj, sID, &List, &sText);

	switch (iResult)
		{
		case resultNotFound:
			return false;

		case resultFound:
			//	This only happens if the caller passed in NULL for retText.
			//	It means that we don't need the result.
			return true;

		case resultArray:
			{
			*retpResult = g_pUniverse->GetCC().CreateLinkedList();

			CCLinkedList *pList = (CCLinkedList *)(*retpResult);
			for (i = 0; i < List.GetCount(); i++)
				pList->AppendStringValue(&g_pUniverse->GetCC(), List[i]);

			return true;
			}

		case resultString:
			{
			*retpResult = g_pUniverse->GetCC().CreateString(sText);
			return true;
			}

		default:
			ASSERT(false);
			return false;
		}
	}

bool CLanguageDataBlock::Translate (CSpaceObject *pObj, const CString &sID, TArray<CString> *retText) const

//	Translate
//
//	Translates to an array of text. If not found, return FALSE.

	{
	CString sText;

	ETranslateResult iResult = Translate(pObj, sID, retText, &sText);

	switch (iResult)
		{
		case resultNotFound:
			return false;

		case resultFound:
			//	This only happens if the caller passed in NULL for retText.
			//	It means that we don't need the result.
			return true;

		case resultArray:
			return true;

		case resultString:
			{
			if (retText)
				{
				retText->DeleteAll();
				retText->Insert(sText);
				}
			return true;
			}

		default:
			ASSERT(false);
			return false;
		}
	}

bool CLanguageDataBlock::Translate (CSpaceObject *pObj, const CString &sID, CString *retsText) const

//	Translate
//
//	Translates to text. If not found, returns FALSE.

	{
	TArray<CString> List;

	ETranslateResult iResult = Translate(pObj, sID, &List, retsText);

	switch (iResult)
		{
		case resultNotFound:
			return false;

		case resultFound:
			//	This only happens if the caller passed in NULL for retsText.
			//	It means that we don't need the result.
			return true;

		case resultArray:
			{
			if (List.GetCount() > 0)
				{
				if (retsText)
					*retsText = List[0];
				return true;
				}
			else
				return false;
			}

		case resultString:
			return true;

		default:
			ASSERT(false);
			return false;
		}
	}
