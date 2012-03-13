//	CCurrencyBlock.cpp
//
//	CCurrencyBlock class

#include "PreComp.h"

CurrencyValue CCurrencyBlock::GetCredits (DWORD dwEconomyUNID)

//	GetCredits
//
//	Returns the number of credits available in the given currency

	{
	if (dwEconomyUNID == 0)
		dwEconomyUNID = DEFAULT_ECONOMY_UNID;

	SEntry *pEntry = m_Block.GetAt(dwEconomyUNID);
	if (pEntry == NULL)
		return 0;

	return pEntry->iValue;
	}

CurrencyValue CCurrencyBlock::GetCredits (const CString &sCurrency)

//	GetCredits
//
//	Returns the number of credits available in the given currency

	{
	ASSERT(!sCurrency.IsBlank());

	CEconomyType *pEcon = g_pUniverse->FindEconomyType(sCurrency);
	if (pEcon == NULL)
		return 0;

	return GetCredits(pEcon->GetUNID());
	}

CurrencyValue CCurrencyBlock::IncCredits (DWORD dwEconomyUNID, CurrencyValue iInc)

//	IncCredits
//
//	Increments/decrements credits

	{
	if (dwEconomyUNID == 0)
		dwEconomyUNID = DEFAULT_ECONOMY_UNID;

	bool bInserted;
	SEntry *pEntry = m_Block.SetAt(dwEconomyUNID, &bInserted);
	if (bInserted)
		pEntry->iValue = 0;

	pEntry->iValue += iInc;

	return pEntry->iValue;
	}

CurrencyValue CCurrencyBlock::IncCredits (const CString &sCurrency, CurrencyValue iInc)

//	IncCredits
//
//	Increments/decrements credits

	{
	ASSERT(!sCurrency.IsBlank());

	CEconomyType *pEcon = g_pUniverse->FindEconomyType(sCurrency);
	if (pEcon == NULL)
		return 0;

	return IncCredits(pEcon->GetUNID(), iInc);
	}

void CCurrencyBlock::ReadFromStream (SLoadCtx &Ctx)

//	ReadFromStream
//
//	DWORD		Count of currencies
//	DWORD			CEconomyType UNID
//	LONGLONG		value

	{
	int i;

	DWORD dwCount;
	Ctx.pStream->Read((char *)&dwCount, sizeof(DWORD));

	for (i = 0; i < (int)dwCount; i++)
		{
		DWORD dwUNID;
		if (Ctx.dwVersion >= 62)
			{
			Ctx.pStream->Read((char *)&dwUNID, sizeof(DWORD));
			}
		else
			{
			CString sCurrency;
			sCurrency.ReadFromStream(Ctx.pStream);

			//	Previous to 62 we never stored any currency
			//	other than credits

			dwUNID = DEFAULT_ECONOMY_UNID;
			}

		SEntry *pEntry = m_Block.Insert(dwUNID);
		Ctx.pStream->Read((char *)&pEntry->iValue, sizeof(CurrencyValue));
		}
	}

void CCurrencyBlock::SetCredits (DWORD dwEconomyUNID, CurrencyValue iValue)

//	SetCredits
//
//	Sets the credit value for the given currency

	{
	if (dwEconomyUNID == 0)
		dwEconomyUNID = DEFAULT_ECONOMY_UNID;

	SEntry *pEntry = m_Block.Insert(dwEconomyUNID);
	pEntry->iValue = iValue;
	}

void CCurrencyBlock::SetCredits (const CString &sCurrency, CurrencyValue iValue)

//	SetCredits
//
//	Sets the credit value for the given currency

	{
	ASSERT(!sCurrency.IsBlank());

	CEconomyType *pEcon = g_pUniverse->FindEconomyType(sCurrency);
	if (pEcon == NULL)
		return;

	SetCredits(pEcon->GetUNID(), iValue);
	}

void CCurrencyBlock::WriteToStream (IWriteStream *pStream)

//	WriteToStream
//
//	DWORD		Count of currencies
//	DWORD			CEconomyType UNID
//	LONGLONG		value

	{
	DWORD dwSave;
	int i;

	dwSave = m_Block.GetCount();
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	for (i = 0; i < m_Block.GetCount(); i++)
		{
		DWORD dwUNID = m_Block.GetKey(i);
		pStream->Write((char *)&dwUNID, sizeof(DWORD));

		pStream->Write((char *)&m_Block[i].iValue, sizeof(CurrencyValue));
		}
	}
