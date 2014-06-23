//	CUIMessageController.cpp
//
//	CUIMessageController class

#include "PreComp.h"
#include "Transcendence.h"

const DWORD FLAG_IS_HINT =				0x00000001;

struct SUIMessageData
	{
	int iNameLen;
	char *pszName;
	DWORD dwFlags;
	};

static SUIMessageData g_MessageData[uimsgCount] =
	{
		{	CONSTDEF("allMessages"),		0,	},
		{	CONSTDEF("allHints"),			0,	},

		{	CONSTDEF("commsHint"),			FLAG_IS_HINT,	},
		{	CONSTDEF("dockHint"),			FLAG_IS_HINT,	},
		{	CONSTDEF("mapHint"),			FLAG_IS_HINT,	},
		{	CONSTDEF("autopilotHint"),		FLAG_IS_HINT,	},
		{	CONSTDEF("gateHint"),			FLAG_IS_HINT,	},
		{	CONSTDEF("useItemHint"),		FLAG_IS_HINT,	},
		{	CONSTDEF("refuelHint"),			FLAG_IS_HINT,	},
		{	CONSTDEF("enableDeviceHint"),	FLAG_IS_HINT,	},
		{	CONSTDEF("switchMissileHint"),	FLAG_IS_HINT,	},
		{	CONSTDEF("fireMissileHint"),	FLAG_IS_HINT,	},
		{	CONSTDEF("galacticMapHint"),	FLAG_IS_HINT,	},
	};

CUIMessageController::CUIMessageController (void)

//	CUIMessageController constructor

	{
	utlMemSet(m_bMsgEnabled, sizeof(m_bMsgEnabled), 0);
	SetEnabled(uimsgAllHints);
	}

UIMessageTypes CUIMessageController::Find (const CString &sMessageName)

//	Find
//
//	Find the message by name. Returns uimsgUnknown if not found.

	{
	int i;

	for (i = 0; i < uimsgCount; i++)
		if (strEquals(sMessageName, CString(g_MessageData[i].pszName, g_MessageData[i].iNameLen, true)))
			return (UIMessageTypes)i;

	if (strEquals(sMessageName, CONSTLIT("enabledHints")))
		return uimsgEnabledHints;
	else
		return uimsgUnknown;
	}

bool CUIMessageController::IsHint (UIMessageTypes iMsg)

//	IsHint
//
//	Returns TRUE if the message is a hint

	{
	if (iMsg < 0)
		return false;

	return ((g_MessageData[iMsg].dwFlags & FLAG_IS_HINT) ? true : false);
	}

void CUIMessageController::ReadFromStream (SLoadCtx &Ctx)

//	ReadFromStream
//
//	Read from a save file

	{
	int i;

	//	Read the number of bytes stored

	DWORD dwLoad;
	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));

	//	Load the array of bytes
	//	(We assume it is DWORD aligned)

	int iArraySize = AlignUp((int)dwLoad, sizeof(DWORD));
	BYTE *pLoad = new BYTE [iArraySize];
	Ctx.pStream->Read((char *)pLoad, iArraySize);

	//	Each byte is an enabled state
	//	(We don't use SetEnabled because that has special meaning for
	//	certain message).

	for (i = 0; i < Min((int)uimsgCount, (int)dwLoad); i++)
		m_bMsgEnabled[i] = (pLoad[i] ? true : false);

	//	NOTE: If uimsgCount increases between version, that's OK because
	//	we initialize m_bMsgEnabled properly in the constructor.

	//	Done

	delete pLoad;
	}

void CUIMessageController::SetEnabled (UIMessageTypes iMsg, bool bEnabled)

//	SetEnabled
//
//	Enable or disable a message

	{
	int i;

	if (iMsg == uimsgAllMessages)
		{
		for (i = 0; i < uimsgCount; i++)
			m_bMsgEnabled[i] = bEnabled;
		}
	else if (iMsg == uimsgAllHints)
		{
		for (i = 0; i < uimsgCount; i++)
			if (IsHint((UIMessageTypes)i))
				m_bMsgEnabled[i] = bEnabled;

		//	We keep track of whether any hint is enabled

		m_bMsgEnabled[uimsgAllHints] = bEnabled;
		}
	else if (iMsg == uimsgEnabledHints)
		{
		if (bEnabled)
			{
			//	Re-enable hints, but only if we have some hints left.

			for (i = 0; i < uimsgCount; i++)
				if (IsHint((UIMessageTypes)i) && m_bMsgEnabled[i])
					{
					m_bMsgEnabled[uimsgAllHints] = true;
					break;
					}
			}
		else
			m_bMsgEnabled[uimsgAllHints] = false;
		}
	else
		{
		m_bMsgEnabled[iMsg] = bEnabled;

		//	We keep track of whether any hint is enabled

		if (IsHint(iMsg))
			{
			if (bEnabled)
				m_bMsgEnabled[uimsgAllHints] = true;
			else if (m_bMsgEnabled[uimsgAllHints])
				{
				m_bMsgEnabled[uimsgAllHints] = false;
				for (i = 0; i < uimsgCount; i++)
					if (IsHint((UIMessageTypes)i) && m_bMsgEnabled[i])
						{
						m_bMsgEnabled[uimsgAllHints] = true;
						break;
						}
				}
			}
		}
	}

void CUIMessageController::WriteToStream (IWriteStream *pStream)

//	WriteToStream
//
//	Write out to the save file

	{
	int i;

	//	Write the count of messages

	DWORD dwSave = uimsgCount;
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	//	Create an array with a byte for each message state
	//	(We make sure that it is DWORD aligned)

	int iArraySize = AlignUp(uimsgCount, sizeof(DWORD));
	BYTE *pSave = new BYTE [iArraySize];
	for (i = 0; i < uimsgCount; i++)
		pSave[i] = (m_bMsgEnabled[i] ? 1 : 0);

	//	Save

	pStream->Write((char *)pSave, iArraySize);
	}
