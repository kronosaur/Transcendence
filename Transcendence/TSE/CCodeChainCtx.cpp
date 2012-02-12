//	CCodeChainCtx.cpp
//
//	CCodeChainCtx object

#include "PreComp.h"

#define STR_G_ITEM								CONSTLIT("gItem")
#define STR_G_SOURCE							CONSTLIT("gSource")

CCodeChainCtx::CCodeChainCtx (void) :
		m_CC(g_pUniverse->GetCC()),
		m_iEvent(eventNone),
		m_pScreen(NULL),
		m_pCanvas(NULL),
		m_pItemType(NULL),
		m_pScreensRoot(NULL),
		m_pSysCreateCtx(NULL),
		m_pOldSource(NULL),
		m_pOldItem (NULL)

//	CCodeChainCtx constructor

	{
	}

CCodeChainCtx::~CCodeChainCtx (void)

//	CCodeChainCtx destructor

	{
	RestoreVars();
	}

CSpaceObject *CCodeChainCtx::AsSpaceObject (ICCItem *pItem)

//	AsSpaceObject
//
//	Convert item to CSpaceObject

	{
	CSpaceObject *pObj;
	try
		{
		pObj = dynamic_cast<CSpaceObject *>((CObject *)pItem->GetIntegerValue());
		}
	catch (...)
		{
		pObj = NULL;
		}
	return pObj;
	}

CVector CCodeChainCtx::AsVector (ICCItem *pItem)

//	AsVector
//
//	Convert item to vector

	{
	return CreateVectorFromList(m_CC, pItem);
	}

void CCodeChainCtx::DefineDamageEffects (const CString &sVar, SDamageCtx &Ctx)

//	DefineDamageEffects
//
//	Sets a variable to damage effects

	{
	ICCItem *pItem = CreateItemFromDamageEffects(m_CC, Ctx);
	m_CC.DefineGlobal(sVar, pItem);
	pItem->Discard(&m_CC);
	}

void CCodeChainCtx::DefineItem (const CString &sVar, CItemCtx &ItemCtx)

//	DefineItem
//
//	Defines a CItem variable

	{
	ICCItem *pItem = ItemCtx.CreateItemVariable(m_CC);
	m_CC.DefineGlobal(sVar, pItem);
	pItem->Discard(&m_CC);
	}

void CCodeChainCtx::DefineItem (const CItem &Item)

//	DefineItem
//
//	Sets gItem

	{
	DefineItem(STR_G_ITEM, Item);
	}

void CCodeChainCtx::DefineItem (const CString &sVar, const CItem &Item)

//	DefineItem
//
//	Defines a CItem variable

	{
	if (Item.GetType())
		{
		ICCItem *pItem = CreateListFromItem(m_CC, Item);
		m_CC.DefineGlobal(sVar, pItem);
		pItem->Discard(&m_CC);
		}
	else
		m_CC.DefineGlobal(sVar, m_CC.CreateNil());
	}

void CCodeChainCtx::DefineItemType (const CString &sVar, CItemType *pType)

//	DefineItemType
//
//	Defines a CItemType variable

	{
	if (pType)
		m_CC.DefineGlobalInteger(sVar, pType->GetUNID());
	else
		m_CC.DefineGlobal(sVar, m_CC.CreateNil());
	}

void CCodeChainCtx::DefineSource (CSpaceObject *pSource)

//	DefineSource
//
//	Sets gSource

	{
	DefineGlobalSpaceObject(m_CC, STR_G_SOURCE, pSource);
	}

void CCodeChainCtx::DefineSpaceObject (const CString &sVar, CSpaceObject *pObj)

//	DefineSpaceObject
//
//	Defines a global SpaceObject variable

	{
	if (pObj)
		m_CC.DefineGlobalInteger(sVar, (int)pObj);
	else
		{
		ICCItem *pValue = m_CC.CreateNil();
		m_CC.DefineGlobal(sVar, pValue);
		pValue->Discard(&m_CC);
		}
	}

void CCodeChainCtx::DefineVector (const CString &sVar, const CVector &vVector)

//	DefineVector
//
//	Defines a global CVector variable

	{
	ICCItem *pValue = CreateListFromVector(m_CC, vVector);
	m_CC.DefineGlobal(sVar, pValue);
	pValue->Discard(&m_CC);
	}

void CCodeChainCtx::RestoreVars (void)

//	RestoreVars
//
//	Restores all saved variables

	{
	if (m_pOldItem)
		{
		m_CC.DefineGlobal(STR_G_ITEM, m_pOldItem);
		m_pOldItem->Discard(&m_CC);
		m_pOldItem = NULL;
		}

	if (m_pOldSource)
		{
		m_CC.DefineGlobal(STR_G_SOURCE, m_pOldSource);
		m_pOldSource->Discard(&m_CC);
		m_pOldSource = NULL;
		}
	}

ICCItem *CCodeChainCtx::Run (ICCItem *pCode)

//	Run
//
//	Runs the given piece of code and returns a result
//	(which must be discarded by the caller)

	{
	return m_CC.TopLevel(pCode, this);
	}

ICCItem *CCodeChainCtx::RunLambda (ICCItem *pCode)

//	RunLambda
//
//	Runs a piece of code or a lambda expression
//	and returns a result (which must be discarded by the caller)

	{
	//	If this is a lambda expression, then eval as if
	//	it were an expression with no arguments

	if (pCode->IsFunction())
		return m_CC.Apply(pCode, m_CC.CreateNil(), this);
	else
		return m_CC.TopLevel(pCode, this);
	}

void CCodeChainCtx::SaveAndDefineSourceVar (CSpaceObject *pSource)

//	SaveAndDefineSourceVar
//
//	Saves and sets gSource

	{
	if (m_pOldSource == NULL)
		m_pOldSource = m_CC.LookupGlobal(STR_G_SOURCE, this);

	DefineGlobalSpaceObject(m_CC, STR_G_SOURCE, pSource);
	}

void CCodeChainCtx::SaveItemVar (void)

//	SaveItemVar
//
//	Saves gItem if not already saved

	{
	if (m_pOldItem == NULL)
		m_pOldItem = m_CC.LookupGlobal(STR_G_ITEM, this);
	}

void CCodeChainCtx::SaveSourceVar (void)

//	SaveSourceVar
//
//	Saves gSource if not already saved

	{
	if (m_pOldSource == NULL)
		m_pOldSource = m_CC.LookupGlobal(STR_G_SOURCE, this);
	}
