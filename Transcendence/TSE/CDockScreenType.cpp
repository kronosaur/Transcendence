//	CDockScreenType.cpp
//
//	CDockScreenType class

#include "PreComp.h"

CDockScreenType::CDockScreenType (void) : 
		m_pDesc(NULL)

//	CDockScreenType constructor

	{
	}

CDockScreenType::~CDockScreenType (void)

//	CDockScreenType destructor

	{
	if (m_pDesc)
		delete m_pDesc;
	}

CString CDockScreenType::GetStringUNID (CDesignType *pRoot, const CString &sScreen)

//	GetStringUNID
//
//	Returns a screen UNID of the form:
//
//	uuuuu
//	uuuuu/localName

	{
	if (pRoot == NULL)
		return sScreen;
	else if (pRoot->GetType() == designDockScreen)
		return strPatternSubst("0x%08x", pRoot->GetUNID());
	else
		return strPatternSubst("0x%08x/%s", pRoot->GetUNID(), sScreen);
	}

ALERROR CDockScreenType::OnCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	OnCreateFromXML
//
//	Create from XML

	{
	m_pDesc = pDesc->OrphanCopy();
	return NOERROR;
	}

CDesignType *CDockScreenType::ResolveScreen (CDesignType *pLocalScreen, const CString &sScreen, CString *retsScreenActual, bool *retbIsLocal)

//	ResolveScreen
//
//	Resolves a string screen name.
//
//	If the screen name is an UNID then we return the docks screen type
//	Otherwise, we return pLocalScreen and the screen name.
//
//	NULL means that the screen was not found.

	{
	bool bNotANumber;
	DWORD dwUNID = (DWORD)strToInt(sScreen, 0, &bNotANumber);

	if (bNotANumber)
		{
		if (retbIsLocal)
			*retbIsLocal = true;

		if (retsScreenActual)
			*retsScreenActual = sScreen;

		return pLocalScreen;
		}
	else
		{
		if (retbIsLocal)
			*retbIsLocal = false;

		if (retsScreenActual)
			*retsScreenActual = NULL_STR;

		return g_pUniverse->FindSharedDockScreen(dwUNID);
		}
	}

//	CDockScreenTypeRef --------------------------------------------------------

ALERROR CDockScreenTypeRef::Bind (CXMLElement *pLocalScreens)
	{
	SDesignLoadCtx Dummy;
	return Bind(Dummy, pLocalScreens);
	}

ALERROR CDockScreenTypeRef::Bind (SDesignLoadCtx &Ctx, CXMLElement *pLocalScreens)
	{
	if (!m_sUNID.IsBlank())
		{
		bool bNotANumber;
		DWORD dwUNID = (DWORD)strToInt(m_sUNID, 0, &bNotANumber);

		//	If the UNID is not a number, then we're looking for a local screen

		if (bNotANumber)
			{
			if (pLocalScreens == NULL)
				{
				Ctx.sError = strPatternSubst(CONSTLIT("No local screen found: %s"), m_sUNID);
				return ERR_FAIL;
				}

			m_pLocal = pLocalScreens->GetContentElementByTag(m_sUNID);
			if (m_pLocal == NULL)
				{
				Ctx.sError = strPatternSubst(CONSTLIT("No local screen found: %s"), m_sUNID);
				return ERR_FAIL;
				}
			}

		//	Otherwise, look for a global screen

		else
			{
			CDesignType *pBaseType = g_pUniverse->FindDesignType(dwUNID);
			if (pBaseType == NULL)
				{
				Ctx.sError = strPatternSubst(CONSTLIT("Unknown dock screen design type: %x"), dwUNID);
				return ERR_FAIL;
				}

			m_pType = CDockScreenType::AsType(pBaseType);
			if (m_pType == NULL)
				{
				Ctx.sError = strPatternSubst(CONSTLIT("Dock screen type expected: %x"), dwUNID);
				return ERR_FAIL;
				}
			}
		}

	return NOERROR;
	}

CXMLElement *CDockScreenTypeRef::GetDesc (void) const
	{
	if (m_pLocal)
		return m_pLocal;
	else if (m_pType)
		return m_pType->GetDesc();
	else
		return NULL;
	}

CDesignType *CDockScreenTypeRef::GetDockScreen (CDesignType *pRoot, CString *retsName) const

//	GetDockScreen
//
//	Returns a root type and a screen name.
//
//	If the root type is a dock screen, then retsName is blank
//	Otherwise, retsName is a screen in the local screens of the root type

	{
	if (m_pLocal)
		{
		if (retsName)
			*retsName = m_sUNID;
		return pRoot;
		}
	else if (m_pType)
		{
		if (retsName)
			*retsName = NULL_STR;
		return m_pType;
		}
	else
		return NULL;
	}

CString CDockScreenTypeRef::GetStringUNID (CDesignType *pRoot) const

//	GetStringUNID
//
//	Returns an UNID of the form:
//
//	uuuuu
//	uuuuu/localName

	{
	CString sScreen;
	CDesignType *pScreen = GetDockScreen(pRoot, &sScreen);
	return CDockScreenType::GetStringUNID(pScreen, sScreen);
	}

void CDockScreenTypeRef::LoadUNID (SDesignLoadCtx &Ctx, const CString &sUNID)
	{
	if (Ctx.pExtension)
		{
		char *pPos = sUNID.GetASCIIZPointer();
		if (*pPos == '@')
			{
			WORD wLow = LOWORD(Ctx.pExtension->GetUNID()) + (WORD)strParseIntOfBase(pPos+1, 16, 0);
			m_sUNID = strFromInt(MAKELONG(wLow, HIWORD(Ctx.pExtension->GetUNID())));
			}
		else
			m_sUNID = sUNID;
		}
	else
		m_sUNID = sUNID;
	}

