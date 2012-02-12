//	ConstantsUtilities.cpp
//
//	Utilities to translate constants from strings and back

#include "PreComp.h"

CString ConstantToString (SConstString *pTable, int iTableSize, int iValue)
	{
	if (iValue < 0 || iValue >= iTableSize)
		return NULL_STR;

	return CONSTUSE(pTable[iValue]);
	}

int StringToConstant (SConstString *pTable, int iTableSize, const CString &sValue)
	{
	int i;

	for (i = 0; i < iTableSize; i++)
		if (strEquals(sValue, CONSTUSE(pTable[i])))
			return i;

	return -1;
	}

struct SConstStringTuple
	{
	int iLen;
	char *pszString;
	DWORD dwData;
	};

CString ConstantToString (SConstStringTuple *pTable, int iTableSize, DWORD dwValue)
	{
	int i;

	for (i = 0; i < iTableSize; i++)
		if (pTable[i].dwData == dwValue)
			return CONSTUSE(pTable[i]);

	return NULL_STR;
	}

DWORD StringToConstant (SConstStringTuple *pTable, int iTableSize, const CString &sValue)
	{
	int i;

	for (i = 0; i < iTableSize; i++)
		if (strEquals(sValue, CONSTUSE(pTable[i])))
			return pTable[i].dwData;

	return pTable[0].dwData;
	}

//	CSpaceObject abilities -----------------------------------------------------

static SConstString AbilityTable[] =
	{
		{	CONSTDEF("SRS") },
		{	CONSTDEF("LRS") },
		{	CONSTDEF("SystemMap") },
		{	CONSTDEF("Autopilot") },
		{	CONSTDEF("SRSEnhancer") },
		{	CONSTDEF("TargetingComputer") },
		{	CONSTDEF("GalacticMap") },
	};

const int AbilityTableSize = (sizeof(AbilityTable) / sizeof(AbilityTable[0]));

static SConstString AbilityModificationTable[] =
	{
		{	CONSTDEF("install") },
		{	CONSTDEF("remove") },
		{	CONSTDEF("damage") },
		{	CONSTDEF("repair") },
	};

const int AbilityModificationTableSize = (sizeof(AbilityModificationTable) / sizeof(AbilityModificationTable[0]));

static SConstStringTuple AbilityModificationOptionTable[] =
	{
		{	CONSTDEF("[unknown]"),			ablOptionUnknown },
		{	CONSTDEF("noMessage"),			ablOptionNoMessage },
	};

const int AbilityModificationOptionTableSize = (sizeof(AbilityModificationOptionTable) / sizeof(AbilityModificationOptionTable[0]));

static SConstString AbilityStatusTable[] =
	{
		{	CONSTDEF("notInstalled") },
		{	CONSTDEF("ready") },
		{	CONSTDEF("damaged") },
	};

const int AbilityStatusTableSize = (sizeof(AbilityStatusTable) / sizeof(AbilityStatusTable[0]));

Abilities AbilityDecode (const CString &sString) { return (Abilities)StringToConstant(AbilityTable, AbilityTableSize, sString); }
AbilityModifications AbilityModificationDecode (const CString &sString) { return (AbilityModifications)StringToConstant(AbilityModificationTable, AbilityModificationTableSize, sString); }
AbilityModificationOptions AbilityModificationOptionDecode (const CString &sString) { return (AbilityModificationOptions)StringToConstant(AbilityModificationOptionTable, AbilityModificationOptionTableSize, sString); }
AbilityStatus AbilityStatusDecode (const CString &sString) { return (AbilityStatus)StringToConstant(AbilityStatusTable, AbilityStatusTableSize, sString); }

CString AbilityEncode (Abilities iValue) { return ConstantToString(AbilityTable, AbilityTableSize, iValue); }
CString AbilityModificationEncode (AbilityModifications iValue) { return ConstantToString(AbilityModificationTable, AbilityModificationTableSize, iValue); }
CString AbilityModificationOptionEncode (AbilityModificationOptions iValue) { return ConstantToString(AbilityModificationOptionTable, AbilityModificationOptionTableSize, iValue); }
CString AbilityStatusEncode (AbilityStatus iValue) { return ConstantToString(AbilityStatusTable, AbilityStatusTableSize, iValue); }
