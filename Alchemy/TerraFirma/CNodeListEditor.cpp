//	CNodeListEditor.cpp
//
//	Implements CNodeListEditor object.
//
//	Data Format:
//
//		(options select-expression node-list)
//
//		options: This is an ordered list of items, each item representing
//		a particular option for the node list. There are no options currently
//		defined.
//
//		select-expression: This is an expression that returns a list of
//		NodeIDs to include in the node-table. If the expression returns
//		True, then the node-table is a hard-coded list of NodeIDs.
//
//		node-table: if select-expression is True, then this is the list of
//		NodeIDs to include. Otherwise, this value is ignored.

#include "Alchemy.h"
#include "TerraFirma.h"

CNodeListEditor::CNodeListEditor (void) :
		m_hListbox(NULL),
		m_pNodeData(NULL)

//	CNodeListEditor constructor

	{
	}

CNodeListEditor::~CNodeListEditor (void)

//	CNodeListEditor destructor

	{
	CleanUp();
	}

void CNodeListEditor::CleanUp (void)

//	Destroy
//
//	Destroys the current node editor

	{
	if (m_hListbox)
		{
		DestroyWindow(m_hListbox);
		m_hListbox = NULL;
		}

	if (m_pNodeData)
		{
		m_pNodeData->Discard(GetCC());
		m_pNodeData = NULL;
		}
	}

ICCItem *CNodeListEditor::ComputeNodeList (ICCItem *pNodeData)

//	ComputeNodeList
//
//	Returns a list of NodeIDs from the NodeData

	{
	ICCItem *pSelectionExp;
	ICCItem *pResult;

	//	This better be a list

	if (!pNodeData->IsList() || pNodeData->GetCount() < 3)
		return GetCC()->CreateError(LITERAL("Invalid node list data"), NULL);

	//	The second item is the selection expression

	pSelectionExp = pNodeData->GetElement(1);
	if (pSelectionExp == NULL)
		return GetCC()->CreateError(LITERAL("Invalid node list data"), NULL);

	//	If the expression evaluates to a list, then it must be a list
	//	of NodeIDs.

	pResult = GetCC()->TopLevel(pSelectionExp, GetTF());
	if (pResult->IsList())
		return pResult;

	//	Otherwise, the third item in the node data is the list

	pResult->Discard(GetCC());
	pResult = pNodeData->GetElement(2);
	if (pResult == NULL)
		return GetCC()->CreateError(LITERAL("Invalid node list data"), NULL);

	//	Done

	return pResult->Reference();
	}

ICCItem *CNodeListEditor::GetNodeData (void)

//	GetNodeData
//
//	Returns the contents of the editor

	{
	if (m_pNodeData)
		return m_pNodeData->Reference();
	else
		return GetCC()->CreateNil();
	}

LONG CNodeListEditor::HandleControlNotification (int iNotifyCode)

//	HandleControlNotification
//
//	Handles a notification from the list control

	{
	switch (iNotifyCode)
		{
		case LBN_DBLCLK:
			{
			int iIndex = SendMessage(m_hListbox, LB_GETCURSEL, 0, 0);
			int iNodeID = SendMessage(m_hListbox, LB_GETITEMDATA, iIndex, 0);
			ICCItem *pItem;
			ICCItem *pResult;

			//	Create an item representing the NodeID

			pItem = GetCC()->CreateInteger(iNodeID);
			if (pItem->IsError())
				{
				pItem->Discard(GetCC());
				return 0;
				}

			//	Send the notification

			pResult = GetTF()->EditorNotification(METHOD_INODELISTSHOWNODE, pItem, NULL, NULL);

			//	Done

			pItem->Discard(GetCC());
			pResult->Discard(GetCC());
			return 0;
			}

		default:
			return 0;
		}
	}

ALERROR CNodeListEditor::Initialize (HWND hParent, RECT rcRect)

//	Initialize
//
//	Creates a new node editor.

	{
	DWORD dwStyle;

	//	Create the listbox

	ASSERT(m_hListbox == NULL);

	//	First, create the window

	dwStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE | WS_VSCROLL;
	dwStyle |= LBS_DISABLENOSCROLL | LBS_SORT | LBS_NOTIFY;

	m_hListbox = CreateWindow(
			"listbox",
			"",
			dwStyle,
			rcRect.left, rcRect.top, RectWidth(&rcRect), RectHeight(&rcRect),
			hParent,
			(HMENU)1,
			GetTF()->GetInstance(),
			NULL);
	if (m_hListbox == NULL)
		return ERR_FAIL;

	//	Set the focus

	::SetFocus(m_hListbox);

	return NOERROR;
	}

void CNodeListEditor::RefreshListbox (ICCItem *pNodeList)

//	RefreshListbox
//
//	Fills the listbox with the node data

	{
	int i;
	ICCItem *pNodeTable;

	ASSERT(m_hListbox);

	//	Get the node table

	pNodeTable = GetTF()->GetNodeTable();

	//	Empty the listbox

	SendMessage(m_hListbox, LB_RESETCONTENT, 0, 0);

	//	Now loop over the data

	for (i = 0; i < pNodeList->GetCount(); i++)
		{
		ICCItem *pNodeID = pNodeList->GetElement(i);

		if (pNodeID)
			{
			ICCItem *pEntry = pNodeTable->GetElement(pNodeID->GetIntegerValue());

			if (pEntry)
				{
				ICCItem *pTitle = pEntry->GetElement(1);
				CString sTitle = pTitle->GetStringValue();
				DWORD dwIndex;

				dwIndex = SendMessage(m_hListbox, LB_ADDSTRING, 0, (LPARAM)sTitle.GetASCIIZPointer());
				SendMessage(m_hListbox, LB_SETITEMDATA, dwIndex, (LPARAM)pNodeID->GetIntegerValue());
				}
			}
		}

	pNodeTable->Discard(GetCC());
	}

ICCItem *CNodeListEditor::SetNodeData (ICCItem *pNodeData)

//	SetNodeData
//
//	Replaces the contents of the editor

	{
	ICCItem *pList;

	//	Calculate the data

	pList = ComputeNodeList(pNodeData);
	if (pList->IsError())
		return pList;

	//	Load the listbox with the new data

	RefreshListbox(pList);
	pList->Discard(GetCC());

	//	Lose the old reference

	if (m_pNodeData)
		m_pNodeData->Discard(GetCC());

	//	Keep a reference to the data

	m_pNodeData = pNodeData->Reference();

	//	Done

	return GetCC()->CreateTrue();
	}
