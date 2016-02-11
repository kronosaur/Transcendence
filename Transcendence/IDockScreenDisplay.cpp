//	IDockScreenDisplay.cpp
//
//	IDockScreenDisplay class

#include "PreComp.h"
#include "Transcendence.h"

#define LIST_TAG					CONSTLIT("List")
#define LIST_OPTIONS_TAG			CONSTLIT("ListOptions")

#define BACKGROUND_ID_ATTRIB		CONSTLIT("backgroundID")
#define DATA_FROM_ATTRIB			CONSTLIT("dataFrom")
#define HEIGHT_ATTRIB				CONSTLIT("height")
#define INITIAL_ITEM_ATTRIB			CONSTLIT("initialItem")
#define LIST_ATTRIB					CONSTLIT("list")
#define POS_X_ATTRIB				CONSTLIT("posX")
#define POS_Y_ATTRIB				CONSTLIT("posY")
#define ROW_HEIGHT_ATTRIB			CONSTLIT("rowHeight")
#define WIDTH_ATTRIB				CONSTLIT("width")

#define DATA_FROM_PLAYER			CONSTLIT("player")
#define DATA_FROM_SOURCE			CONSTLIT("source")
#define DATA_FROM_STATION			CONSTLIT("station")

#define FIELD_IMAGE					CONSTLIT("image")
#define FIELD_OBJ					CONSTLIT("obj")
#define FIELD_TYPE					CONSTLIT("type")

#define TYPE_IMAGE					CONSTLIT("image")
#define TYPE_NONE					CONSTLIT("none")
#define TYPE_OBJECT					CONSTLIT("object")

bool IDockScreenDisplay::GetDisplayOptions (SInitCtx &Ctx, SDisplayOptions *retOptions, CString *retsError)

//	GetDisplayOptions
//
//	Initializes the display options structure, which is used by list and 
//	selector displays.

	{
	//	Initialize background image options

	CString sBackgroundID;
	if (Ctx.pDesc->FindAttribute(BACKGROUND_ID_ATTRIB, &sBackgroundID))
		{
		//	If the attribute exists, but is empty (or equals "none") then
		//	we don't have a background

		if (sBackgroundID.IsBlank() || strEquals(sBackgroundID, CONSTLIT("none")))
			retOptions->BackgroundDesc.iType = backgroundNone;

		//	If the ID is "object" then we should ask the object

		else if (strEquals(sBackgroundID, CONSTLIT("object")))
			{
			retOptions->BackgroundDesc.iType = backgroundObj;
			retOptions->BackgroundDesc.pObj = Ctx.pLocation;
			}

		//	If the ID is "player" then we should ask the player ship

		else if (strEquals(sBackgroundID, DATA_FROM_PLAYER))
			{
			CSpaceObject *pPlayer = g_pUniverse->GetPlayerShip();
			if (pPlayer)
				{
				retOptions->BackgroundDesc.iType = backgroundObj;
				retOptions->BackgroundDesc.pObj = pPlayer;
				}
			}

		//	Otherwise, we expect an integer

		else
			{
			retOptions->BackgroundDesc.iType = backgroundImage;
			retOptions->BackgroundDesc.dwImageID = strToInt(sBackgroundID, 0);
			}
		}

	//	Initialize control rect. If we have a background, then initialize to
	//	backwards compatible position. Otherwise, we take up the full range.

	if (retOptions->BackgroundDesc.iType != backgroundDefault)
		{
		retOptions->rcControl.left = 4;
		retOptions->rcControl.top = 12;
		retOptions->rcControl.right = 548;
		retOptions->rcControl.bottom = 396;
		}
	else
		{
		retOptions->rcControl.left = 0;
		retOptions->rcControl.top = 23;
		retOptions->rcControl.right = 600;
		retOptions->rcControl.bottom = 482;
		}

	//	There are a couple of different ways to get options (for backwards
	//	compatibility).

	CXMLElement *pOptions;
	if ((pOptions = Ctx.pDesc->GetContentElementByTag(LIST_OPTIONS_TAG)) == NULL
			&& (pOptions = Ctx.pDesc->GetContentElementByTag(LIST_TAG)) == NULL)
		return true;

	//	Read from the element

	retOptions->sDataFrom = pOptions->GetAttribute(DATA_FROM_ATTRIB);
	retOptions->sItemCriteria = pOptions->GetAttribute(LIST_ATTRIB);
	retOptions->sCode = pOptions->GetContentText(0);
	retOptions->sInitialItemCode = pOptions->GetAttribute(INITIAL_ITEM_ATTRIB);
	retOptions->sRowHeightCode = pOptions->GetAttribute(ROW_HEIGHT_ATTRIB);

	//	See if we have control position

	if (pOptions->FindAttributeInteger(POS_X_ATTRIB, (int *)&retOptions->rcControl.left))
		{
		retOptions->rcControl.top = pOptions->GetAttributeIntegerBounded(POS_Y_ATTRIB, 0, -1);
		retOptions->rcControl.right = retOptions->rcControl.left + pOptions->GetAttributeIntegerBounded(WIDTH_ATTRIB, 0, -1);
		retOptions->rcControl.bottom = retOptions->rcControl.top + pOptions->GetAttributeIntegerBounded(HEIGHT_ATTRIB, 0, -1);
		}

	return true;
	}

bool IDockScreenDisplay::EvalBool (const CString &sCode, bool *retbResult, CString *retsError)

//	EvalBool
//
//	Evaluates the given string

	{
	CCodeChainCtx Ctx;
	Ctx.SetScreen(m_pDockScreen);
	Ctx.SaveAndDefineSourceVar(m_pLocation);
	Ctx.SaveAndDefineDataVar(m_pData);

	char *pPos = sCode.GetPointer();
	ICCItem *pExp = Ctx.Link(sCode, 1, NULL);

	ICCItem *pResult = Ctx.Run(pExp);	//	LATER:Event
	Ctx.Discard(pExp);

	if (pResult->IsError())
		{
		*retsError = pResult->GetStringValue();
		Ctx.Discard(pResult);
		return false;
		}

	*retbResult = !pResult->IsNil();
	Ctx.Discard(pResult);

	return true;
	}

CSpaceObject *IDockScreenDisplay::EvalListSource (const CString &sString, CString *retsError)

//	EvalListSource
//
//	Returns the object from which we should display items

	{
	char *pPos = sString.GetPointer();

	//	See if we need to evaluate

	if (*pPos == '=')
		{
		CCodeChainCtx Ctx;
		Ctx.SetScreen(this);
		Ctx.SaveAndDefineSourceVar(m_pLocation);
		Ctx.SaveAndDefineDataVar(m_pData);

		ICCItem *pExp = Ctx.Link(sString, 1, NULL);

		ICCItem *pResult = Ctx.Run(pExp);	//	LATER:Event
		Ctx.Discard(pExp);

		if (pResult->IsError())
			{
			*retsError = pResult->GetStringValue();
			Ctx.Discard(pResult);
			return NULL;
			}

		//	Convert to an object pointer

		CSpaceObject *pSource;
		if (strEquals(pResult->GetStringValue(), DATA_FROM_PLAYER))
			pSource = m_pPlayer->GetShip();
		else if (strEquals(pResult->GetStringValue(), DATA_FROM_STATION)
				|| strEquals(pResult->GetStringValue(), DATA_FROM_SOURCE))
			pSource = m_pLocation;
		else
			pSource = Ctx.AsSpaceObject(pResult);

		Ctx.Discard(pResult);
		return pSource;
		}

	//	Otherwise, compare to constants

	else if (strEquals(sString, DATA_FROM_PLAYER))
		return m_pPlayer->GetShip();
	else
		return m_pLocation;
	}

bool IDockScreenDisplay::EvalString (const CString &sString, bool bPlain, ECodeChainEvents iEvent, CString *retsResult)

//	EvalString
//
//	Evaluates the given string.

	{
	CCodeChainCtx Ctx;
	Ctx.SetEvent(iEvent);
	Ctx.SetScreen(m_pDockScreen);
	Ctx.SaveAndDefineSourceVar(m_pLocation);
	Ctx.SaveAndDefineDataVar(m_pData);

	return Ctx.RunEvalString(sString, bPlain, retsResult);
	}

bool IDockScreenDisplay::GetDefaultBackground (SBackgroundDesc *retDesc)

//	GetDefaultBackground
//
//	Sets up any default backgrounds from the display.
	
	{
	return OnGetDefaultBackground(retDesc);
	}

ALERROR IDockScreenDisplay::Init (SInitCtx &Ctx, const SDisplayOptions &Options, CString *retsError)

//	Init
//
//	Initialize

	{
	m_pDockScreen = Ctx.pDockScreen;
	m_pPlayer = Ctx.pPlayer;
	m_pLocation = Ctx.pLocation;
	m_pData = Ctx.pData;

	return OnInit(Ctx, Options, retsError); 
	}

void IDockScreenDisplay::OnShowPane (bool bNoListNavigation) 

//	OnShowPane
//
//	Pane has been shown. This is normally overridden by subclasses, but by
//	default we clear the armor selection.
	
	{
	g_pTrans->SelectArmor(-1); 
	}

bool IDockScreenDisplay::ParseBackgrounDesc (ICCItem *pDesc, SBackgroundDesc *retDesc)

//	ParseBackroundDesc
//
//	Parses a descriptor. Returns TRUE if successful.

	{
	CCodeChain &CC = g_pUniverse->GetCC();

	//	Nil means no default value

	if (pDesc->IsNil())
		retDesc->iType = backgroundDefault;

	//	If we have a struct, we expect a certain format

	else if (pDesc->IsSymbolTable())
		{
		CString sType = pDesc->GetStringAt(FIELD_TYPE);
		if (sType.IsBlank() || strEquals(sType, TYPE_NONE))
			retDesc->iType = backgroundNone;

		else if (strEquals(sType, TYPE_IMAGE))
			{
			retDesc->iType = backgroundImage;

			ICCItem *pImage = pDesc->GetElement(FIELD_IMAGE);
			if (pImage == NULL)
				return false;

			else if (pImage->IsInteger())
				retDesc->dwImageID = pImage->GetIntegerValue();

			else
				return false;
			}
		else if (strEquals(sType, TYPE_OBJECT))
			{
			retDesc->iType = backgroundObj;
			retDesc->pObj = CreateObjFromItem(CC, pDesc->GetElement(FIELD_OBJ));
			if (retDesc->pObj == NULL)
				return false;
			}
		else
			return false;
		}

	//	Otherwise, we can't parse.
	//
	//	LATER: We should eventually handle a list-based image descriptor, but we
	//	would need to enhance SBackgroundDesc for that.

	else
		return false;

	//	Success
	
	return true;
	}
