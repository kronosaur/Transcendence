//	CCUtil.cpp
//
//	CodeChain utilities

#include "PreComp.h"

#define CLASS_CORBIT					CONSTLIT("COrbit")

#define DISP_NEUTRAL					CONSTLIT("neutral")
#define DISP_ENEMY						CONSTLIT("enemy")
#define DISP_FRIEND						CONSTLIT("friend")

#define FIELD_GAUSSIAN					CONSTLIT("gaussian")

Metric CalcRandomMetric (CCodeChain &CC, ICCItem *pItem)

//	CalcRandomMetric
//
//	Parses an item to generate a Metric. The item may have one of the following
//	formats:
//
//	integer
//		We treat as a distance in light-seconds.
//
//	('gaussian min center max)
//		We return a random distance between min and max with center as the 
//		average, using a Gaussian random distribution. All values are expressed
//		in light-seconds, but we can return random numbers anywhere in between.

	{
	if (pItem == NULL || pItem->IsNil())
		return 0.0;
	else if (pItem->IsList())
		{
		if (pItem->GetCount() >= 4 && strEquals(pItem->GetElement(0)->GetStringValue(), FIELD_GAUSSIAN))
			{
			//	Get the parameters

			double rLow = pItem->GetElement(1)->GetIntegerValue();
			double rMid = pItem->GetElement(2)->GetIntegerValue();
			double rHigh = pItem->GetElement(3)->GetIntegerValue();
			if (rLow >= rMid || rLow >= rHigh || rMid >= rHigh)
				return 0.0;

			//	Compute some ranges

			double rHighRange = rHigh - rMid;
			double rLowRange = rMid - rLow;

			//	Generate a gaussian, but clip out after 3 standard deviations

			double rMaxStdDev = 3.0;
			double rValue;
			do
				{
				rValue = mathRandomGaussian();
				}
			while (rValue > rMaxStdDev || rValue < -rMaxStdDev);

			rValue = rValue / rMaxStdDev;

			//	Scale to proper value

			if (rValue >= 0.0)
				return (rMid + rValue * rHighRange) * LIGHT_SECOND;
			else
				return (rMid + rValue * rLowRange) * LIGHT_SECOND;
			}
		else
			return 0.0;
		}
	else
		return (pItem->GetIntegerValue() * LIGHT_SECOND);
	}

bool CreateBinaryFromList (CCodeChain &CC, const CString &sClass, ICCItem *pList, void *pvDest)

//	CreateBinaryFromList
//
//	Initializes binary structure from a list

	{
	int iStart = 0;

	//	Check the class, if provided

	if (!sClass.IsBlank())
		{
		if (pList->GetCount() < 1)
			return false;

		if (!strEquals(pList->GetElement(0)->GetStringValue(), sClass))
			return false;

		iStart++;
		}

	//	Load the binary data

	DWORD *pDest = (DWORD *)pvDest;
	for (int i = iStart; i < pList->GetCount(); i++)
		*pDest++ = (DWORD)pList->GetElement(i)->GetIntegerValue();

	return true;
	}

CString CreateDataFieldFromItemList (const TArray<CItem> &List)

//	CreateDataFieldFromItemList
//
//	Creates a data field string from a list of items

	{
	int i;
	CCodeChain &CC = g_pUniverse->GetCC();

	CMemoryWriteStream Output(10000);
	if (Output.Create() != NOERROR)
		return NULL_STR;

	Output.Write("='(", 3);
	for (i = 0; i < List.GetCount(); i++)
		{
		ICCItem *pItem = List[i].WriteToCCItem(CC);
		if (pItem->IsError())
			{
			pItem->Discard(&CC);
			return NULL_STR;
			}

		CString sItem = pItem->Print(&CC);
		Output.Write(sItem.GetASCIIZPointer(), sItem.GetLength());
		Output.Write(" ", 1);

		pItem->Discard(&CC);
		}
	Output.Write(")", 1);
	Output.Close();

	return CString(Output.GetPointer(), Output.GetLength());
	}

CString CreateDataFromItem (CCodeChain &CC, ICCItem *pItem)

//	CreateDataFromItem
//
//	Encodes the item in a string suitable for storage.
//	(Use Link to reverse).

	{
	//	Set quoted before we unlink
	//	Note: This might be a hack...it probably makes more sense to mark
	//	all function return values as 'quoted'

	bool bOldQuoted = (pItem->IsQuoted() ? true : false);
	pItem->SetQuoted();
	CString sData = CC.Unlink(pItem);
	if (!bOldQuoted)
		pItem->ClearQuoted();

	return sData;
	}

CItem CreateItemFromList (CCodeChain &CC, ICCItem *pList)

//	CreateItemFromList
//
//	Creates an item from a code chain list

	{
	CItem NewItem;
	NewItem.ReadFromCCItem(CC, pList);
	return NewItem;
	}

ICCItem *CreateListFromBinary (CCodeChain &CC, const CString &sClass, void const *pvSource, int iLengthBytes)

//	CreateListFromBinary
//
//	Creates a code chain list from a block of memory

	{
	ICCItem *pResult = CC.CreateLinkedList();
	if (pResult->IsError())
		return pResult;

	CCLinkedList *pList = (CCLinkedList *)pResult;

	//	Add a class, if provided

	if (!sClass.IsBlank())
		pList->AppendStringValue(&CC, sClass);

	//	Add binary bytes in DWORD chunks.

	DWORD *pSource = (DWORD *)pvSource;
	int iCount = AlignUp(iLengthBytes, sizeof(DWORD)) / sizeof(DWORD);

	for (int i = 0; i < iCount; i++)
		{
		ICCItem *pInt = CC.CreateInteger(*pSource++);
		pList->Append(&CC, pInt, NULL);
		pInt->Discard(&CC);
		}

	return pResult;
	}

ICCItem *CreateListFromImage (CCodeChain &CC, const CObjectImageArray &Image, int iRotation)

//	CreateListFromImage
//
//	Creates an imageDesc from an image

	{
	ICCItem *pResult = CC.CreateLinkedList();
	if (pResult->IsError())
		return pResult;

	CCLinkedList *pList = (CCLinkedList *)pResult;

	//	Add the bitmap UNID

	ICCItem *pValue = CC.CreateInteger(Image.GetBitmapUNID());
	pList->Append(&CC, pValue, NULL);
	pValue->Discard(&CC);

	//	Get the rect

	RECT rcRect = Image.GetImageRect(0, iRotation);

	//	Add the x coordinate

	pValue = CC.CreateInteger(rcRect.left);
	pList->Append(&CC, pValue, NULL);
	pValue->Discard(&CC);

	//	Add the y coordinate

	pValue = CC.CreateInteger(rcRect.top);
	pList->Append(&CC, pValue, NULL);
	pValue->Discard(&CC);

	//	Add width

	pValue = CC.CreateInteger(RectWidth(rcRect));
	pList->Append(&CC, pValue, NULL);
	pValue->Discard(&CC);

	//	Add height

	pValue = CC.CreateInteger(RectHeight(rcRect));
	pList->Append(&CC, pValue, NULL);
	pValue->Discard(&CC);

	//	Done

	return pResult;
	}

ICCItem *CreateListFromItem (CCodeChain &CC, const CItem &Item)

//	CreateListFromItem
//
//	Creates a code chain list from an item

	{
	if (Item.GetType() == NULL)
		return CC.CreateNil();

	return Item.WriteToCCItem(CC);
	}

ICCItem *CreateListFromOrbit (CCodeChain &CC, const COrbit &OrbitDesc)

//	CreateListFromOrbit
//
//	Encodes a COrbit object into a code chain list.

	{
	return CreateListFromBinary(CC, CLASS_CORBIT, &OrbitDesc, sizeof(OrbitDesc));
	}

ICCItem *CreateListFromVector (CCodeChain &CC, const CVector &vVector)

//	CreateListFromVector
//
//	Creates a code chain list from a vector

	{
	return CreateListFromBinary(CC, NULL_STR, &vVector, sizeof(vVector));
	}

CSpaceObject *CreateObjFromItem (CCodeChain &CC, ICCItem *pItem)
	{
	int iArg = pItem->GetIntegerValue();
	if (iArg == 0)
		return NULL;

	CSpaceObject *pObj;
	try
		{
		pObj = dynamic_cast<CSpaceObject *>((CObject *)iArg);
		}
	catch (...)
		{
		pObj = NULL;
		}

	return pObj;
	}

bool CreateOrbitFromList (CCodeChain &CC, ICCItem *pList, COrbit *retOrbitDesc)

//	CreateOrbitFromList
//
//	Creates an orbit from a list.

	{
	//	Nil means default orbit

	if (pList->IsNil())
		{
		*retOrbitDesc = COrbit();
		return true;
		}

	//	Must be a list

	else if (!pList->IsList())
		return false;

	//	Load binary from list and check the class

	else
		{
		if (!CreateBinaryFromList(CC, CLASS_CORBIT, pList, retOrbitDesc))
			return false;

		return true;
		}
	}

ICCItem *CreateResultFromDataField (CCodeChain &CC, const CString &sValue)

//	CreateResultFromDataField
//
//	Converts a data field result into either an integer or a string

	{
	//	Blank values are Nil

	if (sValue.IsBlank())
		return CC.CreateNil();

	//	If prefixed with = sign, then this is a CodeChain expression

	else if (*sValue.GetASCIIZPointer() == '=')
		return CC.Link(sValue, 1, NULL);

	//	Handle some special constants

	else if (strEquals(sValue, CONSTLIT("true")))
		return CC.CreateTrue();
	else if (strEquals(sValue, CONSTLIT("nil")) 
			|| strEquals(sValue, CONSTLIT("false"))
			|| strEquals(sValue, CONSTLIT("none")))
		return CC.CreateNil();

	//	Otherwise, see if it is a string or an integer

	else
		{
		char *pPos = sValue.GetASCIIZPointer();
		char *pEnd;
		bool bNull;
		int iResult = strParseInt(pPos, 0, &pEnd, &bNull);
		if (!bNull && *pEnd == '\0')
			return CC.CreateInteger(iResult);
		else
			return CC.CreateString(sValue);
		}
	}

CShip *CreateShipObjFromItem (CCodeChain &CC, ICCItem *pArg) 
	{
	CSpaceObject *pObj = CreateObjFromItem(CC, pArg);
	return (pObj ? pObj->AsShip() : NULL);
	}

CStation *CreateStationObjFromItem (CCodeChain &CC, ICCItem *pArg) 
	{
	CSpaceObject *pObj = CreateObjFromItem(CC, pArg);
	return (pObj ? pObj->AsStation() : NULL);
	}

CVector CreateVectorFromList (CCodeChain &CC, ICCItem *pList)

//	CreateVectorFromList
//
//	Creates a vector from a code chain list

	{
	CVector vVec;

	if (pList->IsList())
		CreateBinaryFromList(CC, NULL_STR, pList, &vVec);
	else if (pList->IsInteger())
		{
		CSpaceObject *pObj = CreateObjFromItem(CC, pList);
		if (pObj)
			vVec = pObj->GetPos();
		}

	return vVec;
	}

CCXMLWrapper *CreateXMLElementFromItem (CCodeChain &CC, ICCItem *pItem)
	{
	if (!strEquals(pItem->GetTypeOf(), CONSTLIT("xmlElement")))
		return NULL;

	CCXMLWrapper *pWrapper = dynamic_cast<CCXMLWrapper *>(pItem);
	if (pItem == NULL)
		return NULL;

	return pWrapper;
	}

ICCItem *CreateDisposition (CCodeChain &CC, CSovereign::Disposition iDisp)

//	CreateDisposition
//
//	Converts a disposition to a string

	{
	switch (iDisp)
		{
		case CSovereign::dispNeutral:
			return CC.CreateString(DISP_NEUTRAL);

		case CSovereign::dispEnemy:
			return CC.CreateString(DISP_ENEMY);

		case CSovereign::dispFriend:
			return CC.CreateString(DISP_FRIEND);

		default:
			ASSERT(false);
			return CC.CreateNil();
		}
	}

CInstalledArmor *GetArmorSectionArg (CCodeChain &CC, ICCItem *pArg, CSpaceObject *pObj)

//	GetArmorSectionArg
//
//	Returns an armor section from a section number or an item struct
//	Returns NULL if invalid.

	{
	//	Get the ship 

	CShip *pShip = pObj->AsShip();
	if (pShip == NULL)
		return NULL;

	//	Set the armor segment

	int iArmorSeg;
	if (pArg->IsList())
		{
		CItem Item = CreateItemFromList(CC, pArg);
		if (Item.GetType() && Item.GetType()->GetArmorClass() && Item.IsInstalled())
			iArmorSeg = Item.GetInstalled();
		else
			return NULL;
		}
	else
		iArmorSeg = pArg->GetIntegerValue();

	//	Some error checking

	if (iArmorSeg < 0 || iArmorSeg >= pShip->GetArmorSectionCount())
		return NULL;

	//	Done

	return pShip->GetArmorSection(iArmorSeg);
	}

CDamageSource GetDamageSourceArg (CCodeChain &CC, ICCItem *pArg)

//	GetDamageSourceArg
//
//	Returns a CDamageSource

	{
	if (pArg->IsNil())
		return CDamageSource(NULL, killedByDamage);

	//	Killed by the given object (explosion)

	else if (pArg->IsInteger())
		{
		CSpaceObject *pSource = CreateObjFromItem(CC, pArg);
		return CDamageSource(pSource, killedByDamage);
		}

	//	Custom death; string describes the cause of death (no explosion)

	else if (pArg->IsIdentifier())
		{
		CString sCause = pArg->GetElement(0)->GetStringValue();

		//	If the cause happens to be a destruction cause ID, then use that.

		DestructionTypes iCause = ::GetDestructionCause(sCause);
		if (iCause != killedNone)
			return CDamageSource(NULL, iCause, NULL, NULL_STR, 0);

		//	Otherwise, this is a custom death

		return CDamageSource(NULL, killedByOther, NULL, sCause, 0);
		}

	//	Killed by a custom cause (explosion)

	else if (pArg->IsList() && pArg->GetCount() == 2 && pArg->GetElement(0)->IsIdentifier())
		{
		CString sSourceName = pArg->GetElement(0)->GetStringValue();
		DWORD dwSourceFlags = (DWORD)pArg->GetElement(1)->GetIntegerValue();
		return CDamageSource(NULL, killedByDamage, NULL, sSourceName, dwSourceFlags);
		}

	//	Full control over death

	else if (pArg->IsList())
		{
		CSpaceObject *pSource = NULL;
		CSpaceObject *pSecondarySource = NULL;
		CString sSourceName;
		DWORD dwSourceFlags = 0;
		DestructionTypes iCause = killedByDamage;

		if (pArg->GetCount() >= 1)
			pSource = CreateObjFromItem(CC, pArg->GetElement(0));

		if (pArg->GetCount() >= 2)
			iCause = ::GetDestructionCause(pArg->GetElement(1)->GetStringValue());

		if (pArg->GetCount() >= 3)
			pSecondarySource = CreateObjFromItem(CC, pArg->GetElement(2));

		if (pArg->GetCount() >= 4)
			sSourceName = pArg->GetElement(3)->GetStringValue();

		if (pArg->GetCount() >= 5)
			dwSourceFlags = (DWORD)pArg->GetElement(4)->GetIntegerValue();

		return CDamageSource(pSource, iCause, pSecondarySource, sSourceName, dwSourceFlags);
		}
	else
		return CDamageSource();
	}

DamageTypes GetDamageTypeFromArg (CCodeChain &CC, ICCItem *pArg)

//	GetDamageTypeFromArg
//
//	Returns the damage type from an argument

	{
	if (pArg->IsInteger())
		{
		int iDamage = pArg->GetIntegerValue();
		if (iDamage < damageGeneric || iDamage >= damageCount)
			return damageError;

		return (DamageTypes)iDamage;
		}
	else
		return LoadDamageTypeFromXML(pArg->GetStringValue());
	}

CInstalledDevice *GetDeviceFromItem (CCodeChain &CC, CSpaceObject *pObj, ICCItem *pArg)

//	GetDeviceFromItem
//
//	Returns a device struct from an item struct (or NULL if not found)

	{
	//	Get the item

	CItem Item(CreateItemFromList(CC, pArg));

	//	Make sure the item is on the object

	CItemListManipulator ItemList(pObj->GetItemList());
	if (!ItemList.SetCursorAtItem(Item))
		return NULL;

	//	Get the installed device from the item

	CInstalledDevice *pDevice = pObj->FindDevice(Item);
	return pDevice;
	}

CItemType *GetItemTypeFromArg (CCodeChain &CC, ICCItem *pArg)

//	GetItemTypeFromArg
//
//	Get an item type

	{
	//	If this is a list, then expect an item

	if (pArg->IsList())
		{
		CItem Item(CreateItemFromList(CC, pArg));
		return Item.GetType();
		}

	//	Otherwise, expect an UNID

	else if (pArg->IsInteger())
		return g_pUniverse->FindItemType((DWORD)pArg->GetIntegerValue());

	//	Otherwise, we don't know

	else
		return NULL;
	}

CEconomyType *GetEconomyTypeFromString (const CString &sCurrency)
	{
	//	If we have an UNID, then look up

	DWORD dwUNID = strToInt(sCurrency, 0);
	if (dwUNID)
		return CEconomyType::AsType(g_pUniverse->FindDesignType(dwUNID));

	//	If we have a currency name, look that up

	return g_pUniverse->FindEconomyType(sCurrency);
	}

CEconomyType *GetEconomyTypeFromItem (CCodeChain &CC, ICCItem *pItem)
	{
	if (pItem == NULL || pItem->IsNil())
		return CEconomyType::AsType(g_pUniverse->FindDesignType(DEFAULT_ECONOMY_UNID));

	if (pItem->IsInteger())
		return CEconomyType::AsType(g_pUniverse->FindDesignType(pItem->GetIntegerValue()));

	return g_pUniverse->FindEconomyType(pItem->GetStringValue());
	}

ALERROR GetEconomyUNIDOrDefault (CCodeChain &CC, ICCItem *pItem, DWORD *retdwUNID)
	{
	if (pItem == NULL || pItem->IsNil())
		{
		if (retdwUNID)
			*retdwUNID = 0;
		}
	else if (pItem->IsInteger())
		{
		CDesignType *pType = g_pUniverse->FindDesignType(pItem->GetIntegerValue());
		if (pType == NULL)
			return ERR_FAIL;

		if (retdwUNID)
			*retdwUNID = pType->GetUNID();
		}
	else
		{
		CEconomyType *pEconomy = g_pUniverse->FindEconomyType(pItem->GetStringValue());
		if (pEconomy == NULL)
			return ERR_FAIL;

		if (retdwUNID)
			*retdwUNID = pEconomy->GetUNID();
		}

	return NOERROR;
	}

void GetImageDescFromList (CCodeChain &CC, ICCItem *pList, CG16bitImage **retpBitmap, RECT *retrcRect)

//	GetImageDescFromList
//
//	Returns an image

	{
	*retpBitmap = NULL;

	if (pList->GetCount() < 5)
		return;

	*retpBitmap = g_pUniverse->GetLibraryBitmap(pList->GetElement(0)->GetIntegerValue());
	if (*retpBitmap == NULL)
		return;

	retrcRect->left = pList->GetElement(1)->GetIntegerValue();
	retrcRect->top = pList->GetElement(2)->GetIntegerValue();
	retrcRect->right = retrcRect->left + pList->GetElement(3)->GetIntegerValue();
	retrcRect->bottom = retrcRect->top + pList->GetElement(4)->GetIntegerValue();
	}

CItem GetItemFromArg (CCodeChain &CC, ICCItem *pArg)

//	GetItemFromArg
//
//	Arg can be an UNID or an item

	{
	if (pArg->IsList())
		return CreateItemFromList(CC, pArg);
	else if (pArg->IsInteger())
		{
		CItemType *pType = g_pUniverse->FindItemType(pArg->GetIntegerValue());
		if (pType == NULL)
			return CItem();

		return CItem(pType, 1);
		}
	else
		return CItem();
	}

bool GetLinkedFireOptions (ICCItem *pArg, DWORD *retdwOptions, CString *retsError)

//	GetLinkedFireOptions
//
//	Parses a list of linked fire options

	{
	int i;

	DWORD dwOptions = 0;

	if (pArg && !pArg->IsNil())
		{
		SDesignLoadCtx LoadCtx;

		DWORD dwOption;
		for (i = 0; i < pArg->GetCount(); i++)
			{
			ICCItem *pOption = pArg->GetElement(i);

			if (CDeviceClass::ParseLinkedFireOptions(LoadCtx, pOption->GetStringValue(), &dwOption) != NOERROR)
				{
				*retsError = LoadCtx.sError;
				return false;
				}

			dwOptions |= dwOption;
			}
		}

	//	Done

	*retdwOptions = dwOptions;
	return true;
	}

bool GetPosOrObject (CEvalContext *pEvalCtx, 
					 ICCItem *pArg, 
					 CVector *retvPos, 
					 CSpaceObject **retpObj,
					 int *retiLocID)

//	GetPosOrObject
//
//	pArg is either a position or an object. We return a position and/or the object.

	{
	CCodeChain &CC(*pEvalCtx->pCC);

	CVector vPos;
	CSpaceObject *pObj = NULL;
	int iLocID = -1;

	if (pArg->IsNil())
		NULL;
	else if (pArg->IsList())
		{
		//	Is this a location criteria?

		CString sTag = pArg->GetElement(0)->GetStringValue();
		if (strEquals(sTag, CONSTLIT("location")))
			{
			CSystem *pSystem = g_pUniverse->GetCurrentSystem();
			if (pSystem == NULL)
				return false;

			//	Get the criteria and parse it

			CString sCriteria = (pArg->GetCount() > 1 ? pArg->GetElement(1)->GetStringValue() : NULL_STR);
			if (sCriteria.IsBlank())
				return false;

			SLocationCriteria Criteria;
			if (Criteria.AttribCriteria.Parse(sCriteria) != NOERROR)
				return false;

			//	Get a random location

			if (!pSystem->FindRandomLocation(Criteria, 0, COrbit(), NULL, &iLocID))
				return false;

			//	Return the position

			CLocationDef *pLoc = pSystem->GetLocation(iLocID);
			vPos = pLoc->GetOrbit().GetObjectPos();
			}

		//	Otherwise, we assume a vector

		else
			vPos = CreateVectorFromList(CC, pArg);
		}
	else
		{
		pObj = CreateObjFromItem(CC, pArg);
		if (pObj)
			vPos = pObj->GetPos();
		}

	//	Done

	if (retvPos)
		*retvPos = vPos;

	if (retpObj)
		*retpObj = pObj;

	if (retiLocID)
		*retiLocID = iLocID;

	return true;
	}

CWeaponFireDesc *GetWeaponFireDescArg (ICCItem *pArg)

//	GetWeaponFireDescArg
//
//	If arg is a weapon UNID, then we return the first weapon desc
//	If arg is a missile, then we return the first weapon desc we find for the missile
//	If arg is a list, then the first is a weapon UNID and the second is a missile UNID
//	Returns NULL on error

	{
	int i;
	DWORD dwWeaponUNID;
	DWORD dwVariantUNID;

	//	If the argument is a list, then we get the weapon UNID and the variant
	//	from the list.

	if (pArg->IsList() && pArg->GetCount() >= 2)
		{
		dwWeaponUNID = (DWORD)pArg->GetElement(0)->GetIntegerValue();
		dwVariantUNID = (DWORD)pArg->GetElement(1)->GetIntegerValue();
		}

	//	Otherwise, get the first variant of the weapon

	else
		{
		dwWeaponUNID = (DWORD)pArg->GetIntegerValue();
		dwVariantUNID = 0;
		}

	//	Get the item associated with the UNID

	CItemType *pType = g_pUniverse->FindItemType(dwWeaponUNID);
	if (pType == NULL)
		return NULL;

	//	If this is a weapon, then return the weapon fire desc

	if (pType->GetCategory() == itemcatWeapon || pType->GetCategory() == itemcatLauncher)
		{
		CDeviceClass *pClass = pType->GetDeviceClass();
		if (pClass == NULL)
			return NULL;

		CWeaponClass *pWeapon = pClass->AsWeaponClass();
		if (pWeapon == NULL)
			return NULL;

		//	If variant UNID is 0, then we just want the weapon

		if (dwVariantUNID == 0)
			return pWeapon->GetVariant(0);

		//	Otherwise, we need to find the variant index for the given UNID

		int i;
		for (i = 0; i < pWeapon->GetVariantCount(); i++)
			{
			CWeaponFireDesc *pDesc = pWeapon->GetVariant(i);
			if (pDesc->GetAmmoType()->GetUNID() == dwVariantUNID)
				return pDesc;
			}

		//	If we get this far, then we couldn't find the missile

		return NULL;
		}

	//	Otherwise, if this is a missile, then find the appropriate weapon

	else if (pType->GetCategory() == itemcatMissile)
		{
		for (i = 0; i < g_pUniverse->GetItemTypeCount(); i++)
			{
			CItemType *pWeaponType = g_pUniverse->GetItemType(i);
			CDeviceClass *pClass;
			if (pClass = pWeaponType->GetDeviceClass())
				{
				int iWeaponVariant;
				if ((iWeaponVariant = pClass->GetAmmoVariant(pType)) != -1)
					{
					CWeaponClass *pWeapon = dynamic_cast<CWeaponClass *>(pClass);
					if (pWeapon)
						return pWeapon->GetVariant(iWeaponVariant);
					}
				}
			}

		return NULL;
		}

	//	Otherwise, nothing

	else
		return NULL;
	}

void DefineGlobalItem (CCodeChain &CC, const CString &sVar, const CItem &Item)

//	DefineGlobalItem
//
//	Defines a global variable and assigns it the given item

	{
	ICCItem *pItem = CreateListFromItem(CC, Item);
	CC.DefineGlobal(sVar, pItem);
	pItem->Discard(&CC);
	}

void DefineGlobalSpaceObject (CCodeChain &CC, const CString &sVar, CSpaceObject *pObj)

//	DefineGlobalSpaceObject
//
//	Defines a global variable and assigns it the given space object. If pObj
//	is NULL then the variable is Nil.

	{
	if (pObj)
		CC.DefineGlobalInteger(sVar, (int)pObj);
	else
		{
		ICCItem *pValue = CC.CreateNil();
		CC.DefineGlobal(sVar, pValue);
		pValue->Discard(&CC);
		}
	}

void DefineGlobalVector (CCodeChain &CC, const CString &sVar, const CVector &vVector)

//	DefineGlobalVector
//
//	Defines a global variable and assigns it the given vector

	{
	ICCItem *pValue = CreateListFromVector(CC, vVector);
	CC.DefineGlobal(sVar, pValue);
	pValue->Discard(&CC);
	}

void DefineGlobalWeaponType (CCodeChain &CC, const CString &sVar, CItemType *pWeaponType)
	{
	if (pWeaponType)
		CC.DefineGlobalInteger(sVar, pWeaponType->GetUNID());
	else
		CC.DefineGlobal(sVar, CC.CreateNil());
	}

ICCItem *StdErrorNoSystem (CCodeChain &CC)
	{
	return CC.CreateError(CONSTLIT("No current system available"));
	}