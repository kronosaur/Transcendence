//	DockScreenDisplayImpl.h
//
//	Dock Screen display classes
//	Copyright (c) 2014 by Kronosaur Productions, LLC. All Rights Reserved.

class CDockScreenList : public IDockScreenDisplay
	{
	public:
		CDockScreenList (void) :
				m_pItemListControl(NULL),
				m_bNoListNavigation(false),
				m_dwNextFilterID(0)
			{ }

	protected:
		struct SFilter
			{
			CString sID;
			DWORD dwID;
			CString sLabel;
			CItemCriteria Filter;
			};

		//	IDockScreenDisplay

		virtual EResults OnAddListFilter (const CString &sID, const CString &sLabel, const CItemCriteria &Filter) override;
		virtual void OnDeleteCurrentItem (int iCount) override;
		virtual const CItem &OnGetCurrentItem (void) const override;
		virtual ICCItem *OnGetCurrentListEntry (void) const override;
		virtual bool OnGetDefaultBackground (SBackgroundDesc *retDesc) override { retDesc->iType = backgroundNone; return true; }
		virtual CItemListManipulator &OnGetItemListManipulator (void) override { return m_pItemListControl->GetItemListManipulator(); }
		virtual int OnGetListCursor (void) override { return m_pItemListControl->GetCursor(); }
		virtual IListData *OnGetListData (void) override { return m_pItemListControl->GetList(); }
		virtual CSpaceObject *OnGetSource (void) override { return m_pItemListControl->GetSource(); }
		virtual EResults OnHandleAction (DWORD dwTag, DWORD dwData) override;
		virtual EResults OnHandleKeyDown (int iVirtKey) override;
		virtual ALERROR OnInit (SInitCtx &Ctx, const SDisplayOptions &Options, CString *retsError) override;
		virtual bool OnIsCurrentItemValid (void) const override;
		virtual EResults OnResetList (CSpaceObject *pLocation) override;
		virtual EResults OnSetListCursor (int iCursor) override;
		virtual EResults OnSetListFilter (const CItemCriteria &Filter) override;
		virtual bool OnSelectNextItem (void) override;
		virtual bool OnSelectPrevItem (void) override;
		virtual void OnShowItem (void) override;
		virtual void OnShowPane (bool bNoListNavigation) override;

		//	CDockScreenList

		virtual ALERROR OnInitList (SInitCtx &Ctx, const SDisplayOptions &Options, CString *retsError) { return NOERROR; }

		//	Helpers

		bool FindFilter (DWORD dwID, int *retiIndex = NULL) const;
		bool FindFilter (const CString &sID, int *retiIndex = NULL) const;

		CGItemListArea *m_pItemListControl;
		DWORD m_dwID;
		bool m_bNoListNavigation;

		TArray<SFilter> m_Filters;
		DWORD m_dwNextFilterID;
	};

class CDockScreenCustomItemList : public CDockScreenList
	{
	protected:

		//	CDockScreenList

		virtual ALERROR OnInitList (SInitCtx &Ctx, const SDisplayOptions &Options, CString *retsError) override;

	private:
		CItemList m_CustomItems;
	};

class CDockScreenCustomList : public CDockScreenList
	{
	protected:

		//	CDockScreenList

		virtual ALERROR OnInitList (SInitCtx &Ctx, const SDisplayOptions &Options, CString *retsError) override;
	};

class CDockScreenItemList : public CDockScreenList
	{
	protected:

		//	CDockScreenList

		virtual ALERROR OnInitList (SInitCtx &Ctx, const SDisplayOptions &Options, CString *retsError) override;

	private:
		CItemCriteria m_ItemCriteria;
	};

class CDockScreenNullDisplay : public IDockScreenDisplay
	{
	};

class CDockScreenSelector : public IDockScreenDisplay
	{
	public:
		CDockScreenSelector (CGSelectorArea::EConfigurations iConfig) :
				m_iConfig(iConfig),
				m_bNoListNavigation(false),
				m_pControl(NULL)
			{ }

	protected:
		//	IDockScreenDisplay

		virtual void OnDeleteCurrentItem (int iCount) override;
		virtual const CItem &OnGetCurrentItem (void) const override;
		virtual ICCItem *OnGetCurrentListEntry (void) const override;
		virtual bool OnGetDefaultBackground (SBackgroundDesc *retDesc) override;
		virtual int OnGetListCursor (void) override { return m_pControl->GetCursor(); }
		virtual IListData *OnGetListData (void) override { return m_pControl->GetList(); }
		virtual CSpaceObject *OnGetSource (void) override { return m_pControl->GetSource(); }
		virtual EResults OnHandleAction (DWORD dwTag, DWORD dwData) override;
		virtual EResults OnHandleKeyDown (int iVirtKey) override;
		virtual ALERROR OnInit (SInitCtx &Ctx, const SDisplayOptions &Options, CString *retsError) override;
		virtual bool OnIsCurrentItemValid (void) const override;
		virtual EResults OnResetList (CSpaceObject *pLocation) override;
		virtual EResults OnSetListCursor (int iCursor) override;
		virtual EResults OnSetListFilter (const CItemCriteria &Filter) override;
		virtual bool OnSelectNextItem (void) override;
		virtual bool OnSelectPrevItem (void) override;
		virtual void OnShowItem (void) override;
		virtual void OnShowPane (bool bNoListNavigation) override;

	private:
		CDockScreen *m_pDockScreen;
		CSpaceObject *m_pLocation;
		CPlayerShipController *m_pPlayer;
		ICCItem *m_pData;
		DWORD m_dwID;
		CGSelectorArea::EConfigurations m_iConfig;
		bool m_bNoListNavigation;

		CGSelectorArea *m_pControl;
	};
