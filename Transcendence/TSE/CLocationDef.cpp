//	CLocationDef.cpp
//
//	CLocationDef class
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "PreComp.h"

CLocationDef::CLocationDef (void) : 
		m_dwObjID(0),
		m_bBlocked(false)

//	CLocationDef constructor

	{
	}

void CLocationDef::ReadFromStream (SLoadCtx &Ctx)

//	ReadFromStream
//
//	CString			m_sID
//	COrbit			m_OrbitDesc
//	CString			m_sAttributes
//	DWORD			m_dwObjID
//	DWORD			flags

	{
	DWORD dwLoad;

	m_sID.ReadFromStream(Ctx.pStream);
	Ctx.pStream->Read((char *)&m_OrbitDesc, sizeof(COrbit));
	m_sAttributes.ReadFromStream(Ctx.pStream);

	Ctx.pStream->Read((char *)&m_dwObjID, sizeof(DWORD));

	//	Flags

	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	m_bBlocked = ((dwLoad & 0x00000001) ? true : false);
	}

void CLocationDef::WriteToStream (IWriteStream *pStream)

//	WriteToStream
//
//	CString			m_sID
//	COrbit			m_OrbitDesc
//	CString			m_sAttributes
//	DWORD			m_dwObjID
//	DWORD			flags

	{
	DWORD dwSave;

	m_sID.WriteToStream(pStream);
	pStream->Write((char *)&m_OrbitDesc, sizeof(COrbit));
	m_sAttributes.WriteToStream(pStream);

	pStream->Write((char *)&m_dwObjID, sizeof(DWORD));

	//	Flags

	dwSave = 0;
	dwSave |= (m_bBlocked ? 0x00000001 : 0);
	pStream->Write((char *)&dwSave, sizeof(DWORD));
	}
