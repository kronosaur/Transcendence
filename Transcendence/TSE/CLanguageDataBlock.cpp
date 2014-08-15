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

CLanguageDataBlock &CLanguageDataBlock::operator= (const CLanguageDataBlock &Src)

//	CLanguageDataBlock equals operator

	{
	int i;

	//	Copy the data

	DeleteAll();
	m_Data = Src.m_Data;

	//	Add a reference to every item

	for (i = 0; i < m_Data.GetCount(); i++)
		{
		if (m_Data[i].pCode)
			m_Data[i].pCode = m_Data[i].pCode->Reference();
		}

	return *this;
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

ICCItem *CLanguageDataBlock::ComposeCCItem (CCodeChain &CC, ICCItem *pValue, const CString &sPlayerName, GenomeTypes iPlayerGenome) const

//	ComposeCCItem
//
//	Recursively parse all strings for player string escape codes (e.g., %name%). 
//	Callers must discard the result.

	{
	int i;

	if (pValue->IsSymbolTable())
		{
		ICCItem *pResult = CC.CreateSymbolTable();

		CCSymbolTable *pSource = (CCSymbolTable *)pValue;
		CCSymbolTable *pTable = (CCSymbolTable *)pResult;
		for (i = 0; i < pValue->GetCount(); i++)
			{
			ICCItem *pElement = ComposeCCItem(CC, pSource->GetElement(i), sPlayerName, iPlayerGenome);
			ICCItem *pKey = CC.CreateString(pSource->GetKey(i));
			pTable->AddEntry(&CC, pKey, pElement);
			pElement->Discard(&CC);
			pKey->Discard(&CC);
			}

		return pResult;
		}

	else if (pValue->IsIdentifier())
		return CC.CreateString(::ComposePlayerNameString(pValue->GetStringValue(), sPlayerName, iPlayerGenome));

	else if (pValue->IsList())
		{
		ICCItem *pResult = CC.CreateLinkedList();

		CCLinkedList *pList = (CCLinkedList *)(pResult);
		for (i = 0; i < pValue->GetCount(); i++)
			{
			ICCItem *pElement = ComposeCCItem(CC, pValue->GetElement(i), sPlayerName, iPlayerGenome);
			pList->Append(&CC, pElement);
			pElement->Discard(&CC);
			}

		return pResult;
		}

	else
		return pValue->Reference();
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
	DEBUG_TRY

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

	DEBUG_CATCH
	}

CLanguageDataBlock::ETranslateResult CLanguageDataBlock::Translate (CSpaceObject *pObj, const CString &sID, ICCItem *pData, TArray<CString> *retText, CString *retsText, ICCItem **retpResult) const

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
	Ctx.SaveAndDefineDataVar(pData);
	
	ICCItem *pResult = Ctx.Run(pEntry->pCode);	//	LATER:Event
	ETranslateResult iResult;

	//	Nil

	if (pResult->IsNil())
		iResult = resultNotFound;

	//	String

	else if (pResult->GetValueType() == ICCItem::String)
		{
		if (retsText)
			{
			*retsText = ::ComposePlayerNameString(pResult->GetStringValue(), g_pUniverse->GetPlayerName(), g_pUniverse->GetPlayerGenome());
			iResult = resultString;
			}
		else
			iResult = resultFound;
		}

	//	Array of strings

	else if (pResult->GetValueType() == ICCItem::List && pResult->GetCount() > 0 && pResult->GetElement(0)->GetValueType() == ICCItem::String)
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

	//	Otherwise we return the naked value

	else
		{
		if (retpResult)
			{
			*retpResult = pResult->Reference();
			iResult = resultCCItem;
			}
		else
			iResult = resultFound;
		}

	//	Done

	Ctx.Discard(pResult);

	return iResult;
	}

bool CLanguageDataBlock::Translate (CSpaceObject *pObj, const CString &sID, ICCItem *pData, ICCItem **retpResult) const

//	Translate
//
//	Translates to an item. The caller is responsible for discarding.

	{
	int i;

	TArray<CString> List;
	CString sText;
	ICCItem *pResult;
	ETranslateResult iResult = Translate(pObj, sID, pData, &List, &sText, &pResult);

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

		case resultCCItem:
			{
			CString sPlayerName = g_pUniverse->GetPlayerName();
			GenomeTypes iPlayerGenome = g_pUniverse->GetPlayerGenome();

			*retpResult = ComposeCCItem(g_pUniverse->GetCC(), pResult, sPlayerName, iPlayerGenome);
			pResult->Discard(&g_pUniverse->GetCC());
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

bool CLanguageDataBlock::Translate (CSpaceObject *pObj, const CString &sID, ICCItem *pData, TArray<CString> *retText) const

//	Translate
//
//	Translates to an array of text. If not found, return FALSE.

	{
	CString sText;

	ETranslateResult iResult = Translate(pObj, sID, pData, retText, &sText);

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

bool CLanguageDataBlock::Translate (CSpaceObject *pObj, const CString &sID, ICCItem *pData, CString *retsText) const

//	Translate
//
//	Translates to text. If not found, returns FALSE.

	{
	TArray<CString> List;

	ETranslateResult iResult = Translate(pObj, sID, pData, &List, retsText);

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
