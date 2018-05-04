//	CDockScreenStack.cpp
//
//	CDockScreenStack class

#include "PreComp.h"
#include "Transcendence.h"

void CDockScreenStack::DeleteAll (void)

//	DeleteAll
//
//	Delete all frames

	{
	while (!IsEmpty())
		Pop();
	}

const SDockFrame &CDockScreenStack::GetCurrent (void) const

//	GetCurrent
//
//	Returns the top of the stack

	{
	int iTop = m_Stack.GetCount() - 1;
	return m_Stack[iTop];
	}

ICCItem *CDockScreenStack::GetData (const CString &sAttrib)

//	GetData
//
//	Returns data for the given attribute. The caller is responsible for 
//	discarding this data.

	{
	CCodeChain &CC = g_pUniverse->GetCC();
	if (IsEmpty())
		return CC.CreateNil();

	ICCItem *pResult = NULL;
	ICCItem *pKey = CC.CreateString(sAttrib);

	SDockFrame &Frame = m_Stack[m_Stack.GetCount() - 1];
	if (Frame.pStoredData)
		{
		pResult = Frame.pStoredData->Lookup(&CC, pKey);
		if (pResult->IsError())
			{
			pResult->Discard(&CC);
			pResult = NULL;
			}
		}

	if (pResult == NULL && Frame.pInitialData)
		{
		pResult = Frame.pInitialData->Lookup(&CC, pKey);
		if (pResult->IsError())
			{
			pResult->Discard(&CC);
			pResult = NULL;
			}
		}

	pKey->Discard(&CC);

	if (pResult == NULL)
		pResult = CC.CreateNil();

	return pResult;
	}

const CString &CDockScreenStack::GetDisplayData (const CString &sID)

//	GetDisplayData
//
//	Returns opaque data stored by displays

	{
	if (IsEmpty())
		return NULL_STR;

	SDockFrame &Frame = m_Stack[m_Stack.GetCount() - 1];
	CString *pValue = Frame.DisplayData.GetAt(sID);
	if (pValue == NULL)
		return NULL_STR;

	return *pValue;
	}

ICCItem *CDockScreenStack::GetReturnData (const CString &sAttrib)

//	GetReturnData
//
//	Returns data for the given attribute. The caller is responsible for 
//	discarding this data.

	{
	CCodeChain &CC = g_pUniverse->GetCC();
	if (IsEmpty())
		return CC.CreateNil();

	SDockFrame &Frame = m_Stack[m_Stack.GetCount() - 1];
	if (Frame.pReturnData)
		{
		ICCItem *pResult = Frame.pReturnData->GetElement(sAttrib);
		if (pResult)
			return pResult->Reference();
		}

	return CC.CreateNil();
	}

void CDockScreenStack::Push (const SDockFrame &Frame)

//	Push
//
//	Push a frame

	{
	//	Whenever we add a frame, we clear the return data block of the top frame
	//	so that we can get return data.

	if (!IsEmpty())
		m_Stack[m_Stack.GetCount() - 1].pReturnData.Delete();

	//	Add

	m_Stack.Insert(Frame);
	}

void CDockScreenStack::Pop (void)

//	Pop
//
//	Pop a frame

	{
	if (!IsEmpty())
		{
		int iTop = m_Stack.GetCount() - 1;
		m_Stack.Delete(iTop);
		}
	}

void CDockScreenStack::ResolveCurrent (const SDockFrame &ResolvedFrame)

//	SetCurrent
//
//	Sets the current frame (and optionally returns the old frame).
//
//	NOTE: Caller is responsible for discarding pInitialData if it get the previous
//	frame.

	{
	if (!IsEmpty())
		{
		int iTop = m_Stack.GetCount() - 1;

		m_Stack[iTop].pResolvedRoot = ResolvedFrame.pResolvedRoot;
		m_Stack[iTop].sResolvedScreen = ResolvedFrame.sResolvedScreen;
		}
	}

void CDockScreenStack::SetCurrent (const SDockFrame &NewFrame, SDockFrame *retPrevFrame)

//	SetCurrent
//
//	Sets the current frame (and optionally returns the old frame).
//
//	NOTE: Caller is responsible for discarding pInitialData if it get the previous
//	frame.

	{
	if (!IsEmpty())
		{
		int iTop = m_Stack.GetCount() - 1;

		if (retPrevFrame)
			*retPrevFrame = m_Stack[iTop];

		m_Stack[iTop] = NewFrame;
		}
	}

void CDockScreenStack::SetCurrentPane (const CString &sPane)

//	SetCurrentPane
//
//	Sets the current pane.

	{
	if (!IsEmpty())
		{
		int iTop = m_Stack.GetCount() - 1;
		m_Stack[iTop].sPane = sPane;
		}
	}

void CDockScreenStack::IncData (const CString &sAttrib, ICCItem *pValue, ICCItem **retpResult)

//	IncData
//
//	Increments data

	{
	CCodeChain &CC = g_pUniverse->GetCC();
	if (IsEmpty())
		{
		if (retpResult) *retpResult = CC.CreateNil();
		return;
		}

    //  If pValue is NULL, we default to 1. We add ref no matter what so that
    //  we can discard unconditionally.

    if (pValue == NULL)
        pValue = CC.CreateInteger(1);
    else
        pValue->Reference();

    //  If the entry is currently blank, then we just take the increment.

	ICCItem *pOriginal = GetData(sAttrib);
    ICCItem *pResult = NULL;
    if (pOriginal->IsNil())
        pResult = pValue->Reference();

    //  Otherwise, we need to get the data value

    else
        {
        if (pOriginal->IsDouble() || pValue->IsDouble())
            pResult = CC.CreateDouble(pOriginal->GetDoubleValue() + pValue->GetDoubleValue());
        else
            pResult = CC.CreateInteger(pOriginal->GetIntegerValue() + pValue->GetIntegerValue());
        }

    pOriginal->Discard(&CC);

    //  Store

	SetData(sAttrib, pResult);

    //  Done

    if (retpResult)
        *retpResult = pResult;
    else
        pResult->Discard(&CC);

    pValue->Discard(&CC);
	}

void CDockScreenStack::SetData (const CString &sAttrib, ICCItem *pData)

//	SetData
//
//	Sets data associated with the current frame

	{
	CCodeChain &CC = g_pUniverse->GetCC();
	if (IsEmpty())
		return;

	//	If necessary, create the stored data block

	SDockFrame &Frame = m_Stack[m_Stack.GetCount() - 1];
	if (!Frame.pStoredData)
		Frame.pStoredData = ICCItemPtr(CC.CreateSymbolTable());

	//	Add the entry

	ICCItem *pKey = CC.CreateString(sAttrib);
	ICCItem *pResult = Frame.pStoredData->AddEntry(&CC, pKey, pData);
	pResult->Discard(&CC);
	pKey->Discard(&CC);
	}

void CDockScreenStack::SetDisplayData (const CString &sID, const CString &sData)

//	SetDisplayData
//
//	Sets opaque data for a display

	{
	if (IsEmpty())
		return;

	SDockFrame &Frame = m_Stack[m_Stack.GetCount() - 1];
	Frame.DisplayData.SetAt(sID, sData);
	}

void CDockScreenStack::SetLocation (CSpaceObject *pLocation)

//	SetLocation
//
//	Sets the location for all frames. This is called, e.g., when we change ships.

	{
	int i;

	for (i = 0; i < m_Stack.GetCount(); i++)
		m_Stack[i].pLocation = pLocation;
	}

void CDockScreenStack::SetReturnData (const CString &sAttrib, ICCItem *pData)

//	SetReturnData
//
//	Sets data associated with previous frame.

	{
	CCodeChain &CC = g_pUniverse->GetCC();
	if (m_Stack.GetCount() < 2)
		return;

	//	If necessary, create the stored data block

	SDockFrame &Frame = m_Stack[m_Stack.GetCount() - 2];
	if (!Frame.pReturnData)
		Frame.pReturnData = ICCItemPtr(CC.CreateSymbolTable());

	//	Add the entry

	Frame.pReturnData->SetAt(CC, sAttrib, pData);
	}
