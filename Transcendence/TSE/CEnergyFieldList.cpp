//	CEnergyFieldList.cpp
//
//	CEnergyFieldList class

#include "PreComp.h"

CEnergyFieldList::CEnergyFieldList (void) :
		m_pFirst(NULL)

//	CEnergyFieldList constructor

	{
	}

CEnergyFieldList::~CEnergyFieldList (void)

//	CEnergyFieldList destructor

	{
	CEnergyField *pField = m_pFirst;
	while (pField)
		{
		CEnergyField *pDelete = pField;
		pField = pField->GetNext();
		delete pDelete;
		}
	}

bool CEnergyFieldList::AbsorbDamage (CSpaceObject *pSource, SDamageCtx &Ctx)

//	AbsorbDamage
//
//	Absorbs damage (returns TRUE if damage was absorbed)

	{
	DEBUG_TRY

	bool bDamageAbsorbed = false;

	CEnergyField *pField = m_pFirst;
	while (pField)
		{
		if (!pField->IsDestroyed()
				&& pField->IsShieldOverlay())
			{
			if (pField->AbsorbDamage(pSource, Ctx))
				bDamageAbsorbed = true;
			}

		pField = pField->GetNext();
		}

	return bDamageAbsorbed;

	DEBUG_CATCH
	}

bool CEnergyFieldList::AbsorbsWeaponFire (CInstalledDevice *pDevice)

//	AbsorbWeaponFire
//
//	Returns TRUE if the field prevents the ship from firing the given weapon

	{
	CEnergyField *pField = m_pFirst;
	while (pField)
		{
		if (!pField->IsDestroyed())
			{
			if (pField->GetType()->AbsorbsWeaponFire(pDevice))
				return true;
			}

		pField = pField->GetNext();
		}

	return false;
	}

void CEnergyFieldList::AccumulateBounds (CSpaceObject *pSource, RECT *ioBounds)

//	AccumulateBounds
//
//	Adds to bounds

	{
	CEnergyField *pField = m_pFirst;
	while (pField)
		{
		if (!pField->IsDestroyed())
			pField->AccumulateBounds(pSource, ioBounds);

		pField = pField->GetNext();
		}
	}

bool CEnergyFieldList::Damage (CSpaceObject *pSource, SDamageCtx &Ctx)

//	Damage
//
//	Takes damage (returns TRUE if damage was absorbed)

	{
	DEBUG_TRY

	bool bDamageAbsorbed = false;

	CEnergyField *pField = m_pFirst;
	while (pField)
		{
		if (!pField->IsDestroyed()
				&& !pField->IsShieldOverlay())
			{
			if (pField->AbsorbDamage(pSource, Ctx))
				bDamageAbsorbed = true;
			}

		pField = pField->GetNext();
		}

	return bDamageAbsorbed;

	DEBUG_CATCH
	}

int CEnergyFieldList::GetCountOfType (COverlayType *pType)

//	GetCountOfType
//
//	Returns the number of existing overlays of the given type.

	{
	int iCount = 0;
	CEnergyField *pField = m_pFirst;
	while (pField)
		{
		if (pField->GetType() == pType)
			iCount++;

		pField = pField->GetNext();
		}

	return iCount;
	}

void CEnergyFieldList::GetImpact (CSpaceObject *pSource, SImpactDesc *retImpact) const

//	GetImpact
//
//	Returns the impact of this set of overlays on the source.

	{
	CEnergyField *pField = m_pFirst;
	while (pField)
		{
		//	Do we disarm the source?

		if (pField->Disarms(pSource))
			retImpact->bDisarm = true;

		//	Do we paralyze the source?

		if (pField->Paralyzes(pSource))
			retImpact->bParalyze = true;

		//	Can't bring up ship status

		if (pField->IsShipScreenDisabled())
			retImpact->bShipScreenDisabled = true;

		//	Do we spin the source ?

		if (pField->Spins(pSource))
			retImpact->bSpin = true;

		//	Get appy drag

		Metric rDrag;
		if ((rDrag = pField->GetDrag(pSource)) < 1.0)
			retImpact->rDrag *= rDrag;

		//	Next

		pField = pField->GetNext();
		}
	}

void CEnergyFieldList::AddField (CSpaceObject *pSource, 
								 COverlayType *pType,
								 int iPosAngle,
								 int iPosRadius,
								 int iRotation,
								 int iLifeLeft, 
								 DWORD *retdwID)

//	AddField
//
//	Adds a field of the given type to the head of the list

	{
	CEnergyField *pField;
	CEnergyField::CreateFromType(pType, 
			iPosAngle, 
			iPosRadius, 
			iRotation, 
			iLifeLeft, 
			&pField);

	//	Add the field to the head of the list

	pField->SetNext(m_pFirst);
	m_pFirst = pField;

	//	Call OnCreate

	pField->FireOnCreate(pSource);

	//	If we deleted, then we're done

	if (pField->IsDestroyed())
		{
		CEnergyField *pNext = pField->GetNext();
		m_pFirst = pNext;
		delete pField;

		if (retdwID)
			*retdwID = 0;
		return;
		}

	//	Done

	if (retdwID)
		*retdwID = pField->GetID();
	}

void CEnergyFieldList::FireOnObjDestroyed (CSpaceObject *pSource, const SDestroyCtx &Ctx) const

//	FireOnObjDestroyed
//
//	Calls OnObjDestroyed on all overlays

	{
	DEBUG_TRY

	CEnergyField *pField = m_pFirst;
	while (pField)
		{
		if (!pField->IsDestroyed())
			pField->FireOnObjDestroyed(pSource, Ctx);

		pField = pField->GetNext();
		}

	DEBUG_CATCH
	}

const CString &CEnergyFieldList::GetData (DWORD dwID, const CString &sAttrib)

//	GetData
//
//	Returns opaque data for the given energy field

	{
	CEnergyField *pField = m_pFirst;
	while (pField)
		{
		if (!pField->IsDestroyed())
			{
			if (pField->GetID() == dwID)
				return pField->GetData(sAttrib);
			}

		pField = pField->GetNext();
		}

	return NULL_STR;
	}

void CEnergyFieldList::GetList (TArray<CEnergyField *> &List)

//	GetList
//
//	Returns all the fields in an array

	{
	List.DeleteAll();

	CEnergyField *pField = m_pFirst;
	while (pField)
		{
		if (!pField->IsDestroyed())
			List.Insert(pField);

		pField = pField->GetNext();
		}
	}

CEnergyField *CEnergyFieldList::GetOverlay (DWORD dwID) const

//	GetOverlay
//
//	Returns the overlay with the given ID (or NULL)

	{
	CEnergyField *pField = m_pFirst;
	while (pField)
		{
		if (pField->GetID() == dwID && !pField->IsDestroyed())
			return pField;

		pField = pField->GetNext();
		}

	return NULL;
	}

CVector CEnergyFieldList::GetPos (CSpaceObject *pSource, DWORD dwID)

//	GetPos
//
//	Returns the current (absolute) position of the overlay

	{
	CEnergyField *pField = m_pFirst;
	while (pField)
		{

		if (pField->GetID() == dwID && !pField->IsDestroyed())
			return pField->GetPos(pSource);

		pField = pField->GetNext();
		}

	return CVector();
	}

ICCItem *CEnergyFieldList::GetProperty (CCodeChainCtx *pCCCtx, CSpaceObject *pSource, DWORD dwID, const CString &sName)

//	GetProperty
//
//	Returns the property

	{
	CEnergyField *pField = m_pFirst;
	while (pField)
		{

		if (pField->GetID() == dwID && !pField->IsDestroyed())
			return pField->GetProperty(pCCCtx, pSource, sName);

		pField = pField->GetNext();
		}

	return g_pUniverse->GetCC().CreateNil();
	}

int CEnergyFieldList::GetRotation (DWORD dwID)

//	GetRotation
//
//	Returns the rotation of the given overlay

	{
	CEnergyField *pField = m_pFirst;
	while (pField)
		{
		if (pField->GetID() == dwID && !pField->IsDestroyed())
			return pField->GetRotation();

		pField = pField->GetNext();
		}

	return -1;
	}

COverlayType *CEnergyFieldList::GetType (DWORD dwID)

//	GetType
//
//	Returns the type of overlay

	{
	CEnergyField *pField = m_pFirst;
	while (pField)
		{
		if (pField->GetID() == dwID && !pField->IsDestroyed())
			return pField->GetType();

		pField = pField->GetNext();
		}

	return NULL;
	}

int CEnergyFieldList::GetWeaponBonus (CInstalledDevice *pDevice, CSpaceObject *pSource)

//	GetWeaponBonus
//
//	Returns the weapon bonus confered by the fields

	{
	int iBonus = 0;

	CEnergyField *pField = m_pFirst;
	while (pField)
		{
		if (!pField->IsDestroyed())
			iBonus += pField->GetType()->GetWeaponBonus(pDevice, pSource);

		pField = pField->GetNext();
		}

	return iBonus;
	}

void CEnergyFieldList::Paint (CG16bitImage &Dest, int iScale, int x, int y, SViewportPaintCtx &Ctx)

//	Paint
//
//	Paints all fields

	{
	CEnergyField *pField = m_pFirst;
	while (pField)
		{
		if (!pField->IsDestroyed())
			pField->Paint(Dest, iScale, x, y, Ctx);

		pField = pField->GetNext();
		}
	}

void CEnergyFieldList::PaintAnnotations (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx)

//	PaintAnnotations
//
//	Paints all field annotations

	{
	CEnergyField *pField = m_pFirst;
	while (pField)
		{
		if (!pField->IsDestroyed())
			pField->PaintAnnotations(Dest, x, y, Ctx);

		pField = pField->GetNext();
		}
	}

void CEnergyFieldList::PaintBackground (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx)

//	PaintBackground
//
//	Paints all field backgrounds

	{
	CEnergyField *pField = m_pFirst;
	while (pField)
		{
		if (!pField->IsDestroyed())
			pField->PaintBackground(Dest, x, y, Ctx);

		pField = pField->GetNext();
		}
	}

void CEnergyFieldList::ReadFromStream (SLoadCtx &Ctx, CSpaceObject *pSource)

//	ReadFromStream
//
//	DWORD		no of fields
//	CEnergyField array

	{
	DWORD dwCount;

	Ctx.pStream->Read((char *)&dwCount, sizeof(DWORD));
	CEnergyField *pPrevField = NULL;
	while (dwCount)
		{
		//	Allocate the overlay and connect it to
		//	the linked list.

		CEnergyField *pField = new CEnergyField;
		if (pPrevField)
			pPrevField->SetNext(pField);
		else
			m_pFirst = pField;

		//	Read

		pField->ReadFromStream(Ctx);

		//	If this field is associated with a device, make
		//	the association now (we assume that in all objects
		//	the devices are loaded first).

		int iDevice;
		if ((iDevice = pField->GetDevice()) != -1)
			{
			if (iDevice < pSource->GetDeviceCount())
				pSource->GetDevice(iDevice)->SetOverlay(pField);
			else
				pField->SetDevice(-1);
			}

		//	Next

		pPrevField = pField;
		dwCount--;
		}
	}

void CEnergyFieldList::RemoveField (CSpaceObject *pSource, DWORD dwID)

//	RemoveField
//
//	Removes the field by ID

	{
	//	Mark the field as destroyed. The field will be deleted in Update

	CEnergyField *pField = m_pFirst;
	while (pField)
		{
		if (pField->GetID() == dwID)
			{
			pField->Destroy(pSource);
			return;
			}

		pField = pField->GetNext();
		}
	}

void CEnergyFieldList::SetData (DWORD dwID, const CString &sAttrib, const CString &sData)

//	SetData
//
//	Sets opaque data

	{
	CEnergyField *pField = m_pFirst;
	while (pField)
		{
		if (pField->GetID() == dwID && !pField->IsDestroyed())
			return pField->SetData(sAttrib, sData);

		pField = pField->GetNext();
		}
	}

bool CEnergyFieldList::SetEffectProperty (DWORD dwID, const CString &sProperty, ICCItem *pValue)

//	SetEffectProperty
//
//	Sets a property on the effect

	{
	CEnergyField *pField = m_pFirst;
	while (pField)
		{
		if (pField->GetID() == dwID && !pField->IsDestroyed())
			return pField->SetEffectProperty(sProperty, pValue);

		pField = pField->GetNext();
		}

	return false;
	}

void CEnergyFieldList::SetPos (CSpaceObject *pSource, DWORD dwID, const CVector &vPos)

//	SetPosition
//
//	Sets the position of the overlay

	{
	CEnergyField *pField = m_pFirst;
	while (pField)
		{
		if (pField->GetID() == dwID && !pField->IsDestroyed())
			return pField->SetPos(pSource, vPos);

		pField = pField->GetNext();
		}
	}

bool CEnergyFieldList::SetProperty (CSpaceObject *pSource, DWORD dwID, const CString &sName, ICCItem *pValue)

//	SetProperty
//
//	Sets a property.

	{
	CEnergyField *pField = m_pFirst;
	while (pField)
		{
		if (pField->GetID() == dwID && !pField->IsDestroyed())
			return pField->SetProperty(pSource, sName, pValue);

		pField = pField->GetNext();
		}

	return false;
	}

void CEnergyFieldList::SetRotation (DWORD dwID, int iRotation)

//	SetRotation
//
//	Sets the rotation of an overlay

	{
	CEnergyField *pField = m_pFirst;
	while (pField)
		{
		if (pField->GetID() == dwID && !pField->IsDestroyed())
			return pField->SetRotation(iRotation);

		pField = pField->GetNext();
		}
	}

void CEnergyFieldList::Update (CSpaceObject *pSource, bool *retbModified)

//	Update
//
//	Update fields. Returns bModified = TRUE if any field changed such that the object
//	need to be recalculated.

	{
	//	First update all fields

	CEnergyField *pField = m_pFirst;
	while (pField)
		{
		if (!pField->IsDestroyed())
			{
			pField->Update(pSource);

			//	If the source got destroyed, then we're done

			if (CSpaceObject::IsDestroyedInUpdate())
				return;
			}

		pField = pField->GetNext();
		}

	//	Do a second pass in which we destroy fields marked for deletion

	bool bModified = false;

	pField = m_pFirst;
	CEnergyField *pPrevField = NULL;
	while (pField)
		{
		CEnergyField *pNext = pField->GetNext();

		//	If this overlay was destroyed, handle that now

		if (pField->IsDestroyed())
			{
			delete pField;

			if (pPrevField)
				pPrevField->SetNext(pNext);
			else
				m_pFirst = pNext;

			bModified = true;
			}
		else
			pPrevField = pField;

		//	Next energy field

		pField = pNext;
		}

	*retbModified = bModified;
	}

void CEnergyFieldList::WriteToStream (IWriteStream *pStream)

//	WriteToStream
//	ReadFromStream
//
//	DWORD		no of fields
//	CEnergyField array

	{
	//	NOTE: We have to saved destroyed overlays because we need to run some 
	//	code when removing an overlay (e.g., see CShip::CalcOverlayImpact).

	DWORD dwSave = 0;
	CEnergyField *pField = m_pFirst;
	while (pField)
		{
		dwSave++;
		pField = pField->GetNext();
		}

	pStream->Write((char *)&dwSave, sizeof(DWORD));
	pField = m_pFirst;
	while (pField)
		{
		pField->WriteToStream(pStream);
		pField = pField->GetNext();
		}
	}
