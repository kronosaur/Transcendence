//	TransExec.h
//
//	Include file for TranExec app
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#ifndef INCL_TRANSEXEC
#define INCL_TRANSEXEC

//	Globals

extern CCodeChain g_CC;

//	Utilities

ALERROR dbgLibraryInit (CCodeChain &CC);
ALERROR osFunctionsInit (void);
ALERROR RegisterPrimitives (PRIMITIVEPROCDEF *pTable, int iCount);

#endif
