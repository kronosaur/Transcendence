//	CEconomyType.cpp
//
//	CEconomyType class

#include "PreComp.h"

#define CURRENCY_ATTRIB							CONSTLIT("currency")
#define CONVERSION_ATTRIB						CONSTLIT("conversion")
#define ID_ATTRIB								CONSTLIT("id")

#define FIELD_NAME								CONSTLIT("name")

static CEconomyType *g_pDefaultEconomy = NULL;

CurrencyValue CEconomyType::Exchange (CEconomyType *pFrom, CurrencyValue iAmount)

//	Exchange
//
//	Exchange from the given currency to this one

	{
	ASSERT(sizeof(CurrencyValue) == sizeof(LONGLONG));

	//	If we're the same rate, then done

	if (pFrom->m_iCreditConversion == m_iCreditConversion)
		return iAmount;

	//	If converting from credits, then it is simpler

	else if (pFrom->m_iCreditConversion == 100)
		return 100 * iAmount / m_iCreditConversion;

	//	If converting to credits, then it is simpler

	else if (m_iCreditConversion == 100)
		return pFrom->m_iCreditConversion * iAmount / 100;

	//	Otherwise we need to do both conversion

	else
		{
		//	First convert to Commonwealth credits

		CurrencyValue iCredits = pFrom->m_iCreditConversion * iAmount / 100;

		//	Now convert back to this currency

		return 100 * iCredits / m_iCreditConversion;
		}
	}

CurrencyValue CEconomyType::ExchangeToCredits (CEconomyType *pFrom, CurrencyValue iAmount)

//	ExchangeToCredits
//
//	Converts the given amount to credits.

	{
	return pFrom->m_iCreditConversion * iAmount / 100;
	}

bool CEconomyType::FindDataField (const CString &sField, CString *retsValue)

//	FindDataField
//
//	Get a data field

	{
	if (strEquals(sField, FIELD_NAME))
		*retsValue = m_sCurrencySingular;
	else
		return false;

	return true;
	}

ALERROR CEconomyType::OnBindDesign (SDesignLoadCtx &Ctx)

//	OnBindDesign
//
//	Bind design

	{
	return NOERROR;
	}

ALERROR CEconomyType::OnCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	OnCreateFromXML
//
//	Load from XML

	{
	m_sSID = pDesc->GetAttribute(ID_ATTRIB);
	if (m_sSID.IsBlank())
		return ComposeLoadError(Ctx, CONSTLIT("Invalid ID"));

	//	Parse the currency name

	m_sCurrencyName = pDesc->GetAttribute(CURRENCY_ATTRIB);
	if (m_sCurrencyName.IsBlank())
		return ComposeLoadError(Ctx, CONSTLIT("Invalid currency name"));

	m_sCurrencySingular = ParseNounForm(m_sCurrencyName, 0, false, true);
	if (m_sCurrencySingular.IsBlank())
		return ComposeLoadError(Ctx, CONSTLIT("Invalid singular form of currency name"));

	m_sCurrencyPlural = ParseNounForm(m_sCurrencyName, 0, true, true);
	if (m_sCurrencyPlural.IsBlank())
		return ComposeLoadError(Ctx, CONSTLIT("Invalid plural form of currency name"));

	//	Get the conversion rate

	m_iCreditConversion = pDesc->GetAttributeIntegerBounded(CONVERSION_ATTRIB, 1, -1, 100);

	return NOERROR;
	}

CString CEconomyType::RinHackGet (CSpaceObject *pObj)

//	RinHackGet
//
//	In previous version we used to store rin as object data on the player ship
//	This hack returns it from the proper place

	{
	CShip *pPlayerShip = pObj->AsShip();
	IShipController *pController = (pPlayerShip ? pPlayerShip->GetController() : NULL);
	CCurrencyBlock *pMoney = (pController ? pController->GetCurrencyBlock() : NULL);
	return (pMoney ? strFromInt((int)pMoney->GetCredits(CONSTLIT("rin"))) : NULL_STR);
	}

CurrencyValue CEconomyType::RinHackInc (CSpaceObject *pObj, CurrencyValue iInc)

//	RinHackInc
//
//	Increment rin

	{
	CShip *pPlayerShip = pObj->AsShip();
	IShipController *pController = (pPlayerShip ? pPlayerShip->GetController() : NULL);
	CCurrencyBlock *pMoney = (pController ? pController->GetCurrencyBlock() : NULL);
	return (pMoney ? pMoney->IncCredits(CONSTLIT("rin"), iInc) : 0);
	}

void CEconomyType::RinHackSet (CSpaceObject *pObj, const CString &sData)

//	RinHackSet
//
//	Sets rin

	{
	CShip *pPlayerShip = pObj->AsShip();
	IShipController *pController = (pPlayerShip ? pPlayerShip->GetController() : NULL);
	CCurrencyBlock *pMoney = (pController ? pController->GetCurrencyBlock() : NULL);
	if (pMoney)
		pMoney->SetCredits(CONSTLIT("rin"), strToInt(sData, 0));
	}

//	CEconomyTypeRef ------------------------------------------------------------

ALERROR CEconomyTypeRef::Bind (SDesignLoadCtx &Ctx)

//	Bind
//
//	Bind the design

	{
	if (m_sUNID.IsBlank())
		{
		if (g_pDefaultEconomy == NULL)
			g_pDefaultEconomy = CEconomyType::AsType(g_pUniverse->FindDesignType(DEFAULT_ECONOMY_UNID));

		m_pType = g_pDefaultEconomy;
		}
	else
		{
		m_pType = GetEconomyTypeFromString(m_sUNID);
		if (m_pType == NULL)
			{
			Ctx.sError = strPatternSubst(CONSTLIT("Unable to find economy type: %s"), m_sUNID);
			return ERR_FAIL;
			}
		}

	return NOERROR;
	}

void CEconomyTypeRef::Set (DWORD dwUNID)

//	Set
//
//	Set

	{
	m_pType = CEconomyType::AsType(g_pUniverse->FindDesignType(dwUNID));
	}

//	CCurrencyAndValue ----------------------------------------------------------

ALERROR CCurrencyAndValue::Bind (SDesignLoadCtx &Ctx)

//	Bind
//
//	Bind design

	{
	return m_pCurrency.Bind(Ctx);
	}

ALERROR CCurrencyAndValue::InitFromXML (SDesignLoadCtx &Ctx, const CString &sDesc)

//	InitFromXML
//
//	Initialies from a string

	{
	//	If blank, default to 0 credits

	if (sDesc.IsBlank())
		{
		//	If m_pCurrency is blank we default to credits (at Bind time)
		m_pCurrency.LoadUNID(NULL_STR);
		m_iValue = 0;
		return NOERROR;
		}

	//	Look for a colon separator

	char *pPos = sDesc.GetASCIIZPointer();
	char *pStart = pPos;
	while (*pPos != '\0' && *pPos != ':')
		pPos++;

	//	If found, then take the first part as the currency
	//	and the second part as value.

	CString sCurrency;
	CString sValue;
	if (*pPos == ':')
		{
		sCurrency = CString(pStart, pPos - pStart);
		sValue = CString(pPos + 1);
		}

	//	Otherwise, assume credits (blank string means credits)

	else
		sValue = sDesc;

	//	Load the currency type

	m_pCurrency.LoadUNID(sCurrency);

	//	Load the value

	bool bFailed;
	m_iValue = strToInt(sValue, 0, &bFailed);
	if (bFailed)
		{
		Ctx.sError = strPatternSubst(CONSTLIT("Invalid currency value: %s"), sValue);
		return ERR_FAIL;
		}

	//	Done

	return NOERROR;
	}

//	CCurrencyAndRange ----------------------------------------------------------

ALERROR CCurrencyAndRange::Bind (SDesignLoadCtx &Ctx)

//	Bind
//
//	Bind

	{
	return m_pCurrency.Bind(Ctx);
	}

ALERROR CCurrencyAndRange::InitFromXML (SDesignLoadCtx &Ctx, const CString &sDesc)

//	InitFromXML
//
//	Initialize from XML

	{
	ALERROR error;

	//	Handle blank

	if (sDesc.IsBlank())
		{
		m_pCurrency.LoadUNID(NULL_STR);
		m_Value.SetConstant(0);
		return NOERROR;
		}

	//	Look for a colon separator

	char *pPos = sDesc.GetASCIIZPointer();
	char *pStart = pPos;
	while (*pPos != '\0' && *pPos != ':')
		pPos++;

	//	If found, then take the first part as the currency
	//	and the second part as value.

	CString sCurrency;
	CString sValue;
	if (*pPos == ':')
		{
		sCurrency = CString(pStart, pPos - pStart);
		sValue = CString(pPos + 1);
		}

	//	Otherwise, assume credits (blank string means credits)

	else
		sValue = sDesc;

	//	Load the currency type

	m_pCurrency.LoadUNID(sCurrency);

	//	Load the range

	if (error = m_Value.LoadFromXML(sValue))
		{
		Ctx.sError = strPatternSubst(CONSTLIT("Invalid dice range: %s"), sValue);
		return ERR_FAIL;
		}

	//	Done

	return NOERROR;
	}
