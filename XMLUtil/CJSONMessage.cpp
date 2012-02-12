//	CJSONMessage.cpp
//
//	CJSONMessage class
//	Copyright (c) 2011 by Kronosaur Productions, LLC. All Rights Reserved.

#include <windows.h>
#include "Alchemy.h"
#include "JSONUtil.h"

CJSONMessage::CJSONMessage (CJSONValue &Value)

//	CJSONMessage contructor

	{
	m_Value.TakeHandoff(Value);
	}

ALERROR CJSONMessage::DecodeFromBuffer (const CString &sMediaType, const CString &sBuffer)

//	DecodeFromBuffer
//
//	Decode from a buffer

	{
	ALERROR error;

	//	Only support application/json

	if (!strEquals(sMediaType, GetMediaType()))
		return ERR_FAIL;

	//	Parse

	CString sError;
	if (error = CJSONValue::Deserialize(sBuffer, &m_Value, &sError))
		return error;

	//	Done

	return NOERROR;
	}

ALERROR CJSONMessage::EncodeToBuffer (IWriteStream *pOutput) const

//	EncodeToBuffer
//
//	Write out to buffer

	{
	Serialize();
	pOutput->Write(m_Serialized.GetPointer(), m_Serialized.GetLength());
	return NOERROR;
	}

DWORD CJSONMessage::GetMediaLength (void) const

//	GetMediaLength
//
//	Returns the length of the encoded media

	{
	Serialize();
	return m_Serialized.GetLength();
	}

void CJSONMessage::Serialize (void) const

//	Serialize
//
//	Initializes m_Serialized

	{
	if (m_Serialized.GetLength() == 0)
		{
		m_Serialized.Create();
		m_Value.Serialize(&m_Serialized);
		}
	}
