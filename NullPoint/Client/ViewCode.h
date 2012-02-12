//	ViewCode.h
//
//	Defines code components for UI

#ifndef INCL_VIEWCODE
#define INCL_VIEWCODE

class CLoginPaneCode : CFCode
	{
	public:
		enum Events
			{
			Close =			1000,
			OK =			1001,
			};

		enum Messages
			{
			GetUserName =	1000,
			GetPassword =	1001,
			};

		static ALERROR Create (CFCode **retpCode);

		virtual DWORD Message (int iMsg, DWORD dwData1, const CString &sData2);

		//	IFController
		virtual void OnEvent (CFView *pView, int iID, int iEvent, DWORD dwData1, const CString &sData2);

	private:
		enum IDs
			{
			OKButton =		100,
			CancelButton =	101,
			UserNameEdit =	102,
			PasswordEdit =	103,
			};
	};

#endif