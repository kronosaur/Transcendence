//	CCShell.h
//
//	CodeChain command shell
//	Copyright (c) 2010 by George Moromisato

#ifndef INCL_CCSHELL
#define INCL_CCSHELL

struct SExecuteCtx
	{
	bool bQuit;
	};

ALERROR RegisterShellPrimitives (CCodeChain &CC);

#endif