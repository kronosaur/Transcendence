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

void CDockScreenStack::DiscardOldFrame (SDockFrame &OldFrame)

//	DiscardOldFrame
//
//	Discards an old frame returns by SetCurrent.

	{
	CCodeChain &CC = g_pUniverse->GetCC();

	if (OldFrame.pInitialData)
		{
		OldFrame.pInitialData->Discard(&CC);
		OldFrame.pInitialData = NULL;
		}

	if (OldFrame.pStoredData)
		{
		OldFrame.pStoredData->Discard(&CC);
		OldFrame.pStoredData = NULL;
		}
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

void CDockScreenStack::Push (const SDockFrame &Frame)

//	Push
//
//	Push a frame

	{
	SDockFrame *pNewFrame = m_Stack.Insert();
	pNewFrame->pLocation = Frame.pLocation;
	pNewFrame->pRoot = Frame.pRoot;
	pNewFrame->sScreen = Frame.sScreen;
	pNewFrame->sPane = Frame.sPane;

	//	If we have data, inc ref count

	if (Frame.pInitialData)
		pNewFrame->pInitialData = Frame.pInitialData->Reference();

	if (Frame.pStoredData)
		pNewFrame->pStoredData = Frame.pStoredData->Reference();
	}

void CDockScreenStack::Pop (void)

//	Pop
//
//	Pop a frame

	{
	if (!IsEmpty())
		{
		int iTop = m_Stack.GetCount() - 1;

		DiscardOldFrame(m_Stack[iTop]);
		m_Stack.Delete(iTop);
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
		else
			DiscardOldFrame(m_Stack[iTop]);

		m_Stack[iTop] = NewFrame;
		if (m_Stack[iTop].pInitialData)
			m_Stack[iTop].pInitialData = NewFrame.pInitialData->Reference();

		if (m_Stack[iTop].pStoredData)
			m_Stack[iTop].pStoredData = NewFrame.pStoredData->Reference();
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
	if (Frame.pStoredData == NULL)
		Frame.pStoredData = CC.CreateSymbolTable();

	//	Add the entry

	ICCItem *pKey = CC.CreateString(sAttrib);
	ICCItem *pResult = Frame.pStoredData->AddEntry(&CC, pKey, pData);
	pResult->Discard(&CC);
	pKey->Discard(&CC);
	}
