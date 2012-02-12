//	CAniProperty.cpp
//
//	CAniProperty class

#include <windows.h>
#include <ddraw.h>
#include "Alchemy.h"
#include "DirectXUtil.h"
#include <math.h>
#include <stdio.h>

#include "Reanimator.h"

CAniProperty::CAniProperty (void) :
		m_iType(typeNone)

//	CAniProperty constructor

	{
	}

CAniProperty::CAniProperty (int iValue)

//	CAniProperty constructor

	{
	m_iType = typeInteger;
	m_Value.dwValue = iValue;
	}

CAniProperty::CAniProperty (Metric rValue)

//	CAniProperty constructor

	{
	m_iType = typeMetric;
	m_Value.rValue = rValue;
	}

CAniProperty::CAniProperty (const CVector &vValue)

//	CAniProperty constructor

	{
	m_iType = typeVector;

	CVector *pVector = (CVector *)&m_Value.vValue;
	*pVector = vValue;
	}

CAniProperty::~CAniProperty (void)

//	CAniProperty destructor

	{
	Clear();
	}

CAniProperty &CAniProperty::operator= (const CAniProperty &Obj)

//	CAniProperty operator =

	{
	Clear();

	m_iType = Obj.m_iType;

	switch (m_iType)
		{
		case typeString:
			m_Value.pValue = CString::INTGetStorage(Obj.GetString());
			break;

		default:
			m_Value = Obj.m_Value;
			break;
		}

	return *this;
	}

void CAniProperty::Clear (void)

//	Clear
//
//	Clear value

	{
	switch (m_iType)
		{
		case typeString:
			CString::INTFreeStorage(m_Value.pValue);
			break;
		}

	m_iType = typeNone;
	}

CString CAniProperty::GetString (void) const

//	GetString
//
//	Returns a string value

	{
	if (m_iType != typeString)
		return NULL_STR;

	CString sString;
	CString::INTSetStorage(sString, m_Value.pValue);
	return sString;
	}

const CVector &CAniProperty::GetVector (void) const

//	GetVector
//
//	Returns a vector value

	{
	if (m_iType != typeVector)
		return NullVector;

	CVector *pVector = (CVector *)&m_Value.vValue;
	return *pVector;
	}

void CAniProperty::Set (Types iType, bool bValue)

//	Set
//
//	Set boolean value

	{
	Clear();

	ASSERT(iType == typeBool);
	m_iType = iType;
	m_Value.dwValue = (bValue ? 1 : 0);
	}

void CAniProperty::Set (Types iType, const CG16bitFont *pFont)

//	Set
//
//	Set font value

	{
	Clear();

	ASSERT(iType == typeFont);
	m_iType = iType;
	m_Value.pValue = (void *)pFont;
	}

void CAniProperty::Set (Types iType, int iValue)

//	Set
//
//	Set integer value
	
	{
	Clear();

	ASSERT(iType == typeInteger);
	m_iType = iType;
	m_Value.dwValue = iValue;
	}

void CAniProperty::Set (Types iType, DWORD dwValue)

//	Set
//
//	Set DWORD value

	{
	Clear();

	ASSERT(iType == typeColor || iType == typeOpacity || iType == typeInteger);
	m_iType = iType;
	m_Value.dwValue = dwValue;
	}

void CAniProperty::Set (Types iType, Metric rValue)

//	Set
//
//	Set Metric value

	{
	Clear();

	ASSERT(iType == typeMetric);
	m_iType = iType;
	m_Value.rValue = rValue;
	}

void CAniProperty::Set (Types iType, const CString &sValue)

//	Set
//
//	Set string value

	{
	Clear();

	ASSERT(iType == typeString);
	m_iType = iType;
	m_Value.pValue = CString::INTGetStorage(sValue);
	}

void CAniProperty::Set (Types iType, const CVector &vValue)

//	Set
//
//	Set vector value

	{
	Clear();

	ASSERT(iType == typeVector);
	m_iType = iType;
	CVector *pVector = (CVector *)&m_Value.vValue;
	*pVector = vValue;
	}
