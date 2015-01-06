//	CExplorerModel.cpp
//
//	CExplorerModel class
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

ALERROR CExplorerModel::Init (void)

//	Init
//
//	Initialize the model

	{
	fileGetVersionInfo(NULL_STR, &m_Version);

	m_Universe.SetSoundMgr(&m_HI.GetSoundMgr());
	m_Universe.SetDebugMode(false);
	m_Universe.SetSound(false);

	return NOERROR;
	}

ALERROR CExplorerModel::LoadUniverse (CString *retsError)

//	LoadUniverse
//
//	Load the universe

	{
	ALERROR error;

	//	Load

	CUniverse::SInitDesc Ctx;
	Ctx.bDefaultExtensions = true;

	if (error = m_Universe.Init(Ctx, retsError))
		return error;

	return NOERROR;
	}
