//	CTransSaverModel.cpp
//
//	CTransSaverModel class
//	Copyright (c) 2010 by George Moromisato. All Rights Reserved.

#include "PreComp.h"

void CTransSaverModel::CleanUp (void)

//	CleanUp
//
//	Clean up

	{
	}

ALERROR CTransSaverModel::Init (void)

//	Init
//
//	Initialize

	{
	fileGetVersionInfo(NULL_STR, &m_Version);

	m_Universe.SetSoundMgr(&m_HI.GetSoundMgr());
	m_Universe.SetDebugMode(false);
	m_Universe.SetHost(NULL);
	m_Universe.SetSound(false);

	return NOERROR;
	}

ALERROR CTransSaverModel::LoadUniverse (CString *retsError)

//	LoadUniverse
//
//	Load the universe

	{
	ALERROR error;

	//	Load

	CUniverse::SInitDesc Ctx;
	Ctx.sFilespec = pathGetResourcePath("DEFAULTDB");
	Ctx.bDefaultExtensions = true;

	if (error = m_Universe.Init(Ctx, retsError))
		return error;

	return NOERROR;
	}
