//	osFunctions.cpp
//
//	Registers and implements os functions
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

#if 0
static PRIMITIVEPROCDEF g_Extensions[] =
	{
		{	"armGetName",					fnArmGet,		FN_ARM_NAME,
			"(armGetName type) -> Name of the armor",
			"v",	0,	},
	};

ALERROR osFunctionsInit (void)
	{
	return RegisterPrimitives(g_Extensions, sizeof(g_Extensions) / sizeof(g_Extension[0]));
	}

ALERROR RegisterPrimitives (PRIMITIVEPROCDEF *pTable, int iCount)

//	osFunctionsInit
//
//	Registers extensions

	{
	ALERROR error;
	int i;

	for (i = 0; i < iCount; i++)
		if (error = g_CC.RegisterPrimitive(&pTable[i]))
			return error;

	return NOERROR;
	}

#endif