//	ViewCode
//
//	Code components for UI

#include "Alchemy.h"
#include "NetUtil.h"
#include "Flatland.h"
#include "NullPointClient.h"
#include "ViewCode.h"

ALERROR CLoginPaneCode::Create (CFCode **retpCode)
	{
	*retpCode = new CLoginPaneCode;
	if (*retpCode == NULL)
		return ERR_MEMORY;

	return NOERROR;
	}

DWORD CLoginPaneCode::Message (int iMsg, DWORD dwData1, const CString &sData2)
	{
	switch (iMsg)
		{
		case GetUserName:
			{
			CFView *pEdit = GetView()->FindSubView(UserNameEdit);
			return pEdit->Message(CFTextEditView::GetText, 0);
			}

		case GetPassword:
			{
			CFView *pEdit = GetView()->FindSubView(PasswordEdit);
			return pEdit->Message(CFTextEditView::GetText, 0);
			}
		}

	return 0;
	}

void CLoginPaneCode::OnEvent (CFView *pView, int iID, int iEvent, DWORD dwData1, const CString &sData2)
	{
	switch (iID)
		{
		case OKButton:
			RaiseEvent(OK, 0);
			break;

		case CancelButton:
			RaiseEvent(Close, 0);
			break;
		}
	}

ALERROR RegisterCode (void)
	{
	ALERROR error;

	if (error = g_pFlatland->RegisterCode(CONSTLIT("LoginPaneCode"), CLoginPaneCode::Create))
		return error;

	return NOERROR;
	}