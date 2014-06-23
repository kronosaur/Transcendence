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

bool CEffectParamDesc::EvalBool (CCreatePainterCtx &Ctx) const

//	EvalBool
//
//	Returns a boolean

	{
	switch (m_iType)
		{
		case typeBoolConstant:
		case typeIntegerConstant:
			return (m_dwData != 0);

		case typeIntegerDiceRange:
			return (m_DiceRange.Roll() != 0);

		case typeStringConstant:
			return strEquals(m_sData, CONSTLIT("true"));

		default:
			return false;
		}
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

DiceRange CEffectParamDesc::EvalDiceRange (CCreatePainterCtx &Ctx, int iDefault) const

//	EvalDiceRange
//
//	Evaluates the value as a dice range.

	{
	switch (m_iType)
		{
		case typeNull:
			return DiceRange(0, 0, iDefault);

		case typeIntegerConstant:
			return DiceRange(0, 0, (int)m_dwData);

		case typeIntegerDiceRange:
			return m_DiceRange;

		case typeStringConstant:
			{
			DiceRange Temp;
			if (Temp.LoadFromXML(m_sData) != NOERROR)
				Temp = DiceRange();

			return Temp;
			}

		default:
			return DiceRange(0, 0, iDefault);
		}
	}

int CEffectParamDesc::EvalIdentifier (CCreatePainterCtx &Ctx, LPSTR *pIDMap, int iMax, int iDefault) const

//	EvalIdentifier
//
//	Evalues the value as an identifer (either a constant integer or a string,
//	which is looked up in the ID map).

	{
	switch (m_iType)
		{
		case typeIntegerConstant:
			return Max(0, Min((int)m_dwData, iMax));

		case typeIntegerDiceRange:
			return Max(0, Min((int)m_DiceRange.Roll(), iMax));

		case typeStringConstant:
			{
			DWORD dwID;
			if (!FindIdentifier(m_sData, pIDMap, &dwID))
				return iDefault;

			return (int)dwID;
			}

		default:
			return iDefault;
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
		case typeBoolConstant:
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

CString CEffectParamDesc::EvalString (CCreatePainterCtx &Ctx) const

//	EvalString
//
//	Returns a string

	{
	switch (m_iType)
		{
		case typeStringConstant:
			return m_sData;

		default:
			return NULL_STR;
		}
	}

CVector CEffectParamDesc::EvalVector (CCreatePainterCtx &Ctx) const

//	EvalVector
//
//	Returns a vector

	{
	switch (m_iType)
		{
		case typeVectorConstant:
			return m_vVector;

		default:
			return CVector();
		}
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

bool CEffectParamDesc::FindIdentifier (const CString &sValue, LPSTR *pIDMap, DWORD *retdwID) const

//	FindIdentifier
//
//	Finds the identifier in the table

	{
	//	Loop through the ID table looking for the identifier

	DWORD dwID = 0;
	LPSTR *pID = pIDMap;
	while (*pID != NULL)
		{
		if ((*pID)[0] != '\0' && strEquals(sValue, CString(*pID, -1, TRUE)))
			{
			if (retdwID)
				*retdwID = dwID;

			return true;
			}

		//	Next string in the table

		dwID++;
		pID++;
		}

	//	Not found

	return false;
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

	if (!FindIdentifier(sValue, pIDMap, &m_dwData))
		{
		Ctx.sError = strPatternSubst(CONSTLIT("Invalid effect param identifier: %s"), sValue);
		return ERR_FAIL;
		}

	m_iType = typeIntegerConstant;

	return NOERROR;
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
		case typeBoolConstant:
		case typeColorConstant:
		case typeIntegerConstant:
		case typeStringConstant:
		case typeVectorConstant:
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

		case typeBoolConstant:
			if (Ctx.dwVersion >= 99)
				Ctx.pStream->Read((char *)&m_dwData, sizeof(DWORD));
			else
				m_dwData = 0;
			break;

		case typeColorConstant:
		case typeIntegerConstant:
			Ctx.pStream->Read((char *)&m_dwData, sizeof(DWORD));
			break;

		case typeIntegerDiceRange:
			if (Ctx.dwVersion >= 99)
				m_DiceRange.ReadFromStream(Ctx);
			break;

		case typeStringConstant:
			m_sData.ReadFromStream(Ctx.pStream);
			break;

		case typeVectorConstant:
			Ctx.pStream->Read((char *)&m_vVector, sizeof(CVector));
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

		case typeBoolConstant:
		case typeColorConstant:
		case typeIntegerConstant:
			pStream->Write((char *)&m_dwData, sizeof(DWORD));
			break;

		case typeIntegerDiceRange:
			m_DiceRange.WriteToStream(pStream);
			break;

		case typeStringConstant:
			m_sData.WriteToStream(pStream);
			break;

		case typeVectorConstant:
			pStream->Write((char *)&m_vVector, sizeof(CVector));
			break;
		}
	}
