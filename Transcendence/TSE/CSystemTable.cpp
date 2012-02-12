//	CSystemTable.cpp
//
//	CSystemTable class

#include "PreComp.h"

CSystemTable::~CSystemTable (void)

//	CSystemTable destructor

	{
	if (m_pTable)
		delete m_pTable;
	}

