//	CAniPropertySet.cpp
//
//	CAniPropertySet class

#include <windows.h>
#include <ddraw.h>
#include "Alchemy.h"
#include "DirectXUtil.h"
#include <math.h>
#include <stdio.h>

#include "Reanimator.h"

CAniPropertySet::~CAniPropertySet (void)

//	CAniPropertySet destructor

	{
	int i;

	for (i = 0; i < m_Animators.GetCount(); i++)
		delete m_Animators[i].pAnimator;
	}

CAniPropertySet::SProperty *CAniPropertySet::Add (const CString &sName, int *retiIndex)

//	Add
//
//	Adds a new property

	{
	if (retiIndex)
		*retiIndex = m_Properties.GetCount();

	SProperty *pProp = m_Properties.Insert();
	pProp->sName = sName;

	return pProp;
	}

void CAniPropertySet::AddAnimator (const CString &sProperty, IPropertyAnimator *pAnimator, int iStartFrame, const CString &sID, bool bStartNow)

//	AddAnimator
//
//	Adds an animator to the set
//
//	LATER: bStartNow is a HACK to add new animators in the middle of a 
//	performance. Right now it causes an animator's iFrame to start at 0 when the
//	animator is added. This works only if we use GoToNextFrame; it will not work
//	with GoToFrame or GoToStart. To fix this we would have to set a flag in the
//	entry and wait for the next GoToNextFrame to store the current frame number.

	{
	int i;
	int iIndex;
	Find(sProperty, &iIndex);
	ASSERT(iIndex != -1);
	if (iIndex == -1)
		return;

	ASSERT(pAnimator);

	//	Animator

	SAnimator NewAnimator;
	NewAnimator.sID = sID;
	NewAnimator.pAnimator = pAnimator;
	NewAnimator.iProperty = iIndex;
	NewAnimator.iStartFrame = iStartFrame;
	NewAnimator.iFrame = (bStartNow ? 0 : -1);

	//	Insert the animator in sorted order by start frame

	int iInsertPos = -1;
	for (i = 0; i < m_Animators.GetCount(); i++)
		if (m_Animators[i].iStartFrame > iStartFrame)
			{
			iInsertPos = i;
			break;
			}

	m_Animators.Insert(NewAnimator, iInsertPos);
	}

void CAniPropertySet::DeleteAnimator (const CString &sID)

//	DeleteAnimator
//
//	Deletes all animators with the given ID

	{
	int i;

	for (i = 0; i < m_Animators.GetCount(); i++)
		if (strEquals(m_Animators[i].sID, sID))
			{
			delete m_Animators[i].pAnimator;

			m_Animators.Delete(i);
			i--;
			}
	}

CAniPropertySet::SProperty *CAniPropertySet::Find (const CString &sName, int *retiIndex) const

//	Find
//
//	Finds the property

	{
	int i;

	for (i = 0; i < m_Properties.GetCount(); i++)
		if (strEquals(m_Properties[i].sName, sName))
			{
			if (retiIndex)
				*retiIndex = i;

			return &m_Properties[i];
			}

	if (retiIndex)
		*retiIndex = -1;

	return NULL;
	}

bool CAniPropertySet::FindAnimator (const CString &sID, IPropertyAnimator **retpAnimator)

//	FindAnimator
//
//	Finds the animator

	{
	int i;

	for (i = 0; i < m_Animators.GetCount(); i++)
		if (strEquals(m_Animators[i].sID, sID))
			{
			*retpAnimator = m_Animators[i].pAnimator;
			return true;
			}

	return false;
	}

CAniPropertySet::SProperty *CAniPropertySet::FindOrAdd (const CString &sName, int *retiIndex)

//	FindOrAdd
//
//	Finds or adds the property

	{
	SProperty *pProp = Find(sName, retiIndex);
	if (pProp)
		return pProp;

	return Add(sName, retiIndex);
	}

bool CAniPropertySet::GetBool (const CString &sName) const

//	GetBool

	{
	SProperty *pProp;
	if (pProp = Find(sName))
		return pProp->CurrentValue.GetBool();
	else
		return false;
	}

WORD CAniPropertySet::GetColor (const CString &sName) const

//	GetColor

	{
	SProperty *pProp;
	if (pProp = Find(sName))
		return pProp->CurrentValue.GetColor();
	else
		return 0;
	}

const CG16bitFont *CAniPropertySet::GetFont (const CString &sName) const

//	GetFont

	{
	SProperty *pProp;
	if (pProp = Find(sName))
		return pProp->CurrentValue.GetFont();
	else
		return NULL;
	}

int CAniPropertySet::GetDuration (void)

//	GetDuration
//
//	Returns the total animation duration

	{
	int i;
	int iTotalDuration = durationUndefined;

	for (i = 0; i < m_Animators.GetCount(); i++)
		{
		SAnimator *pAni = &m_Animators[i];
		int iAniDuration = pAni->pAnimator->GetDuration();
		if (iAniDuration == durationInfinite)
			return durationInfinite;
		else if (iAniDuration == durationUndefined)
			continue;

		int iDuration = iAniDuration + pAni->iStartFrame;
		if (iDuration > iTotalDuration)
			iTotalDuration = iDuration;
		}

	return iTotalDuration;
	}

int CAniPropertySet::GetInteger (const CString &sName) const

//	GetInteger

	{
	SProperty *pProp;
	if (pProp = Find(sName))
		return pProp->CurrentValue.GetInteger();
	else
		return 0;
	}

Metric CAniPropertySet::GetMetric (const CString &sName) const

//	GetMetric

	{
	SProperty *pProp;
	if (pProp = Find(sName))
		return pProp->CurrentValue.GetMetric();
	else
		return 0;
	}

DWORD CAniPropertySet::GetOpacity (const CString &sName) const

//	GetOpacity

	{
	SProperty *pProp;
	if (pProp = Find(sName))
		return pProp->CurrentValue.GetOpacity();
	else
		return 0;
	}

CString CAniPropertySet::GetString (const CString &sName) const

//	GetString

	{
	SProperty *pProp;
	if (pProp = Find(sName))
		return pProp->CurrentValue.GetString();
	else
		return 0;
	}

CVector CAniPropertySet::GetVector (const CString &sName) const

//	GetVector

	{
	SProperty *pProp;
	if (pProp = Find(sName))
		return pProp->CurrentValue.GetVector();
	else
		return CVector();
	}

void CAniPropertySet::GoToFrame (int iFrame)

//	GoToFrame
//
//	Sets the position at the given frame

	{
	int i;

	//	For all animators

	for (i = 0; i < m_Animators.GetCount(); i++)
		{
		SAnimator *pAni = &m_Animators[i];
		int iDuration = pAni->pAnimator->GetDuration();

		//	If this animator has not yet started, then reset its properties
		//	to initial values (we rely on the fact that animators are sorted
		//	by start frame).

		if (pAni->iStartFrame > iFrame)
			{
			SProperty *pProp = &m_Properties[pAni->iProperty];
			pProp->CurrentValue = pProp->InitialValue;
			pAni->iFrame = -1;
			}

		//	If this animator has already ended, then set the properties
		//	to the final value

		else if (iDuration >= 0 && iFrame >= pAni->iStartFrame + iDuration)
			{
			SProperty *pProp = &m_Properties[pAni->iProperty];
			pAni->pAnimator->SetProperty(iDuration - 1, pProp->CurrentValue);
			pAni->iFrame = -1;
			}

		//	Otherwise, set at the proper frame

		else
			{
			pAni->iFrame = iFrame - pAni->iStartFrame;
			SProperty *pProp = &m_Properties[pAni->iProperty];
			pAni->pAnimator->SetProperty(pAni->iFrame, pProp->CurrentValue);
			}
		}
	}

void CAniPropertySet::GoToNextFrame (SAniUpdateCtx &Ctx, int iFrame)

//	GoToNextFrame
//
//	Advance

	{
	int i;

	//	Advance all animators

	for (i = 0; i < m_Animators.GetCount(); i++)
		{
		SAnimator *pAni = &m_Animators[i];
		int iDuration = pAni->pAnimator->GetDuration();

		//	Start and stop animators

		if (pAni->iFrame != -1)
			{
			pAni->iFrame++;
			if (iDuration >= 0 && pAni->iFrame >= iDuration)
				{
				pAni->iFrame = -1;
				pAni->pAnimator->OnDoneAnimating(Ctx);
				}
			}
		else if (pAni->iStartFrame == iFrame)
			{
			pAni->iFrame = 0;
			}

		//	Animator sets the property

		if (pAni->iFrame != -1)
			{
			SProperty *pProp = &m_Properties[pAni->iProperty];
			pAni->pAnimator->SetProperty(pAni->iFrame, pProp->CurrentValue);
			}
		}
	}

void CAniPropertySet::GoToStart (void)

//	GoToStart
//
//	Set the initial values for all properties

	{
	int i;

	//	All properties start at their initial value

	for (i = 0; i < m_Properties.GetCount(); i++)
		{
		SProperty *pProp = &m_Properties[i];
		pProp->CurrentValue = pProp->InitialValue;
		}

	//	Start any animators that run at start

	for (i = 0; i < m_Animators.GetCount(); i++)
		{
		SAnimator *pAni = &m_Animators[i];

		if (pAni->iStartFrame == 0)
			{
			pAni->iFrame = 0;

			SProperty *pProp = &m_Properties[pAni->iProperty];
			pAni->pAnimator->SetProperty(pAni->iFrame, pProp->CurrentValue);
			}
		}
	}

void CAniPropertySet::Set (const CString &sName, const CAniProperty &Value, int *retiIndex)

//	Set
//
//	Sets a value

	{
	SProperty *pProp = FindOrAdd(sName, retiIndex);
	pProp->InitialValue = Value;
	pProp->CurrentValue = Value;
	}

void CAniPropertySet::SetBool (const CString &sName, bool bValue, int *retiIndex)

//	SetBool
//
//	Sets boolean property

	{
	SProperty *pProp = FindOrAdd(sName, retiIndex);
	pProp->InitialValue.Set(CAniProperty::typeBool, bValue);
	pProp->CurrentValue.Set(CAniProperty::typeBool, bValue);
	}

void CAniPropertySet::SetColor (const CString &sName, WORD wValue, int *retiIndex)

//	SetColor
//
//	Add an opacity property

	{
	SProperty *pProp = FindOrAdd(sName, retiIndex);
	pProp->InitialValue.Set(CAniProperty::typeColor, (DWORD)wValue);
	pProp->CurrentValue.Set(CAniProperty::typeColor, (DWORD)wValue);
	}

void CAniPropertySet::SetFont (const CString &sName, const CG16bitFont *pFont, int *retiIndex)

//	SetFont
//
//	Sets a font property

	{
	SProperty *pProp = FindOrAdd(sName, retiIndex);
	pProp->InitialValue.Set(CAniProperty::typeFont, pFont);
	pProp->CurrentValue.Set(CAniProperty::typeFont, pFont);
	}

void CAniPropertySet::SetInteger (const CString &sName, int iValue, int *retiIndex)

//	SetInteger
//
//	Add an integer property

	{
	SProperty *pProp = FindOrAdd(sName, retiIndex);
	pProp->InitialValue.Set(CAniProperty::typeInteger, iValue);
	pProp->CurrentValue.Set(CAniProperty::typeInteger, iValue);
	}

void CAniPropertySet::SetMetric (const CString &sName, Metric rValue, int *retiIndex)

//	SetMetric
//
//	Add a Metric property

	{
	SProperty *pProp = FindOrAdd(sName, retiIndex);
	pProp->InitialValue.Set(CAniProperty::typeMetric, rValue);
	pProp->CurrentValue.Set(CAniProperty::typeMetric, rValue);
	}

void CAniPropertySet::SetOpacity (const CString &sName, DWORD dwValue, int *retiIndex)

//	SetOpacity
//
//	Add an opacity property

	{
	SProperty *pProp = FindOrAdd(sName, retiIndex);
	pProp->InitialValue.Set(CAniProperty::typeOpacity, dwValue);
	pProp->CurrentValue.Set(CAniProperty::typeOpacity, dwValue);
	}

void CAniPropertySet::SetString (const CString &sName, const CString &sValue, int *retiIndex)

//	SetString
//
//	Add a string property

	{
	SProperty *pProp = FindOrAdd(sName, retiIndex);
	pProp->InitialValue.Set(CAniProperty::typeString, sValue);
	pProp->CurrentValue.Set(CAniProperty::typeString, sValue);
	}

void CAniPropertySet::SetVector (const CString &sName, const CVector &vValue, int *retiIndex)

//	SetVector
//
//	Sets a vector property

	{
	SProperty *pProp = FindOrAdd(sName, retiIndex);
	pProp->InitialValue.Set(CAniProperty::typeVector, vValue);
	pProp->CurrentValue.Set(CAniProperty::typeVector, vValue);
	}
