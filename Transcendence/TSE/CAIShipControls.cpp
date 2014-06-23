//	CAIShipControls.cpp
//
//	CAIShipControls class
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "PreComp.h"

CAIShipControls::CAIShipControls (void) : 
		m_iManeuver(NoRotation),
		m_iThrustDir(constNeverThrust)

//	CAIShipControls constructor

	{
	}

void CAIShipControls::ReadFromStream (SLoadCtx &Ctx)

//	ReadFromStream
//
//	Load from stream

	{
	Ctx.pStream->Read((char *)&m_iManeuver, sizeof(DWORD));
	Ctx.pStream->Read((char *)&m_iThrustDir, sizeof(DWORD));
	}

void CAIShipControls::WriteToStream (CSystem *pSystem, IWriteStream *pStream)

//	WriteToStream
//
//	Write to stream

	{
	pStream->Write((char *)&m_iManeuver, sizeof(DWORD));
	pStream->Write((char *)&m_iThrustDir, sizeof(DWORD));
	}
