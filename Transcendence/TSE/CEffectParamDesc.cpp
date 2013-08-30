//	CEffectParamDesc.cpp
//
//	CEffectParamDesc class
//	Copyright (c) 2013 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"

CEffectParamDesc::~CEffectParamDesc (void)

//	CEffectParamDesc destructor

	{
	CleanUp();
	}

void CEffectParamDesc::CleanUp (void)

//	CleanUp
//
//	Clean up value

	{
	if (m_pItem)
		{
		m_pItem->Discard(&g_pUniverse->GetCC());
		m_pItem = NULL;
		}

	m_iType = typeNull;
	}

WORD CEffectParamDesc::EvalColor (CCreatePainterCtx &Ctx) const

//	EvalColor
//
//	Returns the color

	{
	switch (m_iType)
		{
		case typeColorConstant:
			return (WORD)m_dwData;

		default:
			return 0;
		}
	}

int CEffectParamDesc::EvalInteger (CCreatePainterCtx &Ctx) const

//	EvalInteger
//
//	Returns the integer

	{
	switch (m_iType)
		{
		case typeIntegerConstant:
			return m_dwData;

		case typeIntegerDiceRange:
			return m_DiceRange.Roll();

		default:
			return 0;
		}
	}

int CEffectParamDesc::EvalIntegerBounded (CCreatePainterCtx &Ctx, int iMin, int iMax, int iDefault) const

//	EvalIntegerBounded
//
//	Returns the integer within a certain range

	{
	int iValue;

	switch (m_iType)
		{
		case typeIntegerConstant:
			iValue = (int)m_dwData;
			break;

		case typeIntegerDiceRange:
			iValue = m_DiceRange.Roll();
			break;

		default:
			return iDefault;
		}

	if (iValue == iDefault)
		return iDefault;

	else if (iMax < iMin)
		return Max(iValue, iMin);

	else
		return Max(Min(iValue, iMax), iMin);
	}

ALERROR CEffectParamDesc::InitColorFromXML (SDesignLoadCtx &Ctx, const CString &sValue)

//	InitColorFromXML
//
//	Initializes a color value

	{
	ASSERT(m_iType == typeNull);

	if (!sValue.IsBlank())
		{
		m_iType = typeColorConstant;
		m_dwData = ::LoadRGBColor(sValue);
		}

	return NOERROR;
	}

ALERROR CEffectParamDesc::InitIdentifierFromXML (SDesignLoadCtx &Ctx, const CString &sValue, LPSTR *pIDMap)

//	InitIdentifierFromXML
//
//	Initializes an identifier

	{
	ASSERT(m_iType == typeNull);

	//	If the value is blank, then we leave as defaults

	if (sValue.IsBlank())
		{
		m_iType = typeNull;
		return NOERROR;
		}

	//	Loop through the ID table looking for the identifier

	DWORD dwID = 0;
	LPSTR *pID = pIDMap;
	while (*pID != NULL)
		{
		if ((*pID)[0] != '\0' && strEquals(sValue, CString(*pID, -1, TRUE)))
			{
			m_iType = typeIntegerConstant;
			m_dwData = dwID;
			return NOERROR;
			}

		//	Next string in the table

		dwID++;
		pID++;
		}

	//	If we get this far then we didn't find the identifier in the table, 
	//	which is an error.

	Ctx.sError = strPatternSubst(CONSTLIT("Invalid effect param identifier: %s"), sValue);
	return ERR_FAIL;
	}

ALERROR CEffectParamDesc::InitIntegerFromXML (SDesignLoadCtx &Ctx, const CString &sValue)

//	InitIntegerFromXML
//
//	Initializes an integer

	{
	ASSERT(m_iType == typeNull);

	if (!sValue.IsBlank())
		{
		if (m_DiceRange.LoadFromXML(sValue) != NOERROR)
			{
			Ctx.sError = strPatternSubst(CONSTLIT("Unable to parse dice range: %s"), sValue);
			return ERR_FAIL;
			}

		//	If the dice range is constant, then just use that

		if (m_DiceRange.IsConstant())
			{
			m_iType = typeIntegerConstant;
			m_dwData = m_DiceRange.Roll();
			}

		//	Otherwise we are variant

		else
			m_iType = typeIntegerDiceRange;
		}

	return NOERROR;
	}

ALERROR CEffectParamDesc::InitStringFromXML (SDesignLoadCtx &Ctx, const CString &sValue)

//	InitStringFromXML
//
//	Initializes a string

	{
	ASSERT(m_iType == typeNull);

	if (!sValue.IsBlank())
		{
		m_iType = typeStringConstant;
		m_sData = sValue;
		}

	return NOERROR;
	}

bool CEffectParamDesc::IsConstant (void)

//	IsConstant
//
//	Returns TRUE if the value is a constant

	{
	switch (m_iType)
		{
		case typeNull:
		case typeColorConstant:
		case typeIntegerConstant:
		case typeStringConstant:
			return true;

		default:
			return false;
		}
	}

void CEffectParamDesc::ReadFromStream (SLoadCtx &Ctx)

//	ReadFromStream
//
//	Read the parameter. NOTE: This only works for constant values.

	{
	CleanUp();

	DWORD dwLoad;
	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	m_iType = (EDataTypes)dwLoad;

	switch (m_iType)
		{
		case typeNull:
			break;

		case typeColorConstant:
		case typeIntegerConstant:
			Ctx.pStream->Read((char *)&m_dwData, sizeof(DWORD));
			break;

		case typeStringConstant:
			m_sData.ReadFromStream(Ctx.pStream);
			break;
		}
	}

void CEffectParamDesc::WriteToStream (IWriteStream *pStream)

//	WriteToStream
//
//	Write parameter
//
//	DWORD			m_iType
//
//	DWORD
//	or
//	CString

	{
	DWORD dwSave = (DWORD)m_iType;
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	switch (m_iType)
		{
		case typeNull:
			break;

		case typeColorConstant:
		case typeIntegerConstant:
			pStream->Write((char *)&m_dwData, sizeof(DWORD));
			break;

		case typeStringConstant:
			m_sData.WriteToStream(pStream);
			break;
		}
	}
