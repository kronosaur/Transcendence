//	DockScreenDisplayImpl.h
//
//	Dock Screen display classes
//	Copyright (c) 2014 by Kronosaur Productions, LLC. All Rights Reserved.

class CDockScreenList : public IDockScreenDisplay
	{
	public:
		CDockScreenList (void) :
				m_pItemListControl(NULL),
				m_bNoListNavigation(false)
			{ }

	protected:
		//	IDockScreenDisplay

		virtual void OnDeleteCurrentItem (int iCount);
		virtual const CItem &OnGetCurrentItem (void) const;
		virtual ICCItem *OnGetCurrentListEntry (void) const;
		virtual CItemListManipulator &OnGetItemListManipulator (void) { return m_pItemListControl->GetItemListManipulator(); }
		virtual int OnGetListCursor (void) { return m_pItemListControl->GetCursor(); }
		virtual IListData *OnGetListData (void) { return m_pItemListControl->GetList(); }
		virtual EResults OnHandleAction (DWORD dwTag, DWORD dwData);
		virtual EResults OnHandleKeyDown (int iVirtKey);
		virtual ALERROR OnInit (SInitCtx &Ctx, CString *retsError);
		virtual bool OnIsCurrentItemValid (void) const;
		virtual EResults OnResetList (CSpaceObject *pLocation);
		virtual EResults OnSetListCursor (int iCursor);
		virtual EResults OnSetListFilter (const CItemCriteria &Filter);
		virtual bool OnSelectNextItem (void);
		virtual bool OnSelectPrevItem (void);
		virtual void OnShowItem (void);
		virtual void OnShowPane (bool bNoListNavigation);

		//	CDockScreenList

		virtual ALERROR OnInitList (SInitCtx &Ctx, CString *retsError) { return NOERROR; }

		//	Helper functions

		bool EvalBool (const CString &sCode, bool *retbResult, CString *retsError);
		bool EvalString (const CString &sString, bool bPlain, ECodeChainEvents iEvent, CString *retsResult);

		CGItemListArea *m_pItemListControl;
		CDockScreen *m_pDockScreen;
		CSpaceObject *m_pLocation;
		CPlayerShipController *m_pPlayer;
		ICCItem *m_pData;
		DWORD m_dwID;
		bool m_bNoListNavigation;
	};

class CDockScreenCustomItemList : public CDockScreenList
	{
	protected:

		//	CDockScreenList

		virtual ALERROR OnInitList (SInitCtx &Ctx, CString *retsError);

	private:
		CItemList m_CustomItems;
	};

class CDockScreenCustomList : public CDockScreenList
	{
	protected:

		//	CDockScreenList

		virtual ALERROR OnInitList (SInitCtx &Ctx, CString *retsError);
	};

class CDockScreenItemList : public CDockScreenList
	{
	protected:

		//	CDockScreenList

		virtual ALERROR OnInitList (SInitCtx &Ctx, CString *retsError);

	private:
		CSpaceObject *EvalListSource (const CString &sString, CString *retsError);

		CItemCriteria m_ItemCriteria;
	};

class CDockScreenNullDisplay : public IDockScreenDisplay
	{
	};
