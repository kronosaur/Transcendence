//	DockScreenDisplayImpl.h
//
//	Dock Screen display classes
//	Copyright (c) 2014 by Kronosaur Productions, LLC. All Rights Reserved.

//	CDockScreenCarousel --------------------------------------------------------

class CDockScreenCarousel : public IDockScreenDisplay
	{
	public:
		CDockScreenCarousel (CDockScreen &DockScreen) : IDockScreenDisplay(DockScreen)
			{ }

	protected:

		//	IDockScreenDisplay

		virtual ICCItem *OnGetCurrentListEntry (void) const override;
		virtual bool OnGetDefaultBackground (SBackgroundDesc *retDesc) override { retDesc->iType = backgroundNone; return true; }
		virtual int OnGetListCursor (void) override { return m_pControl->GetCursor(); }
		virtual IListData *OnGetListData (void) override { return m_pControl->GetList(); }
		virtual CSpaceObject *OnGetSource (void) const override { return m_pControl->GetSource(); }
		virtual EResults OnHandleAction (DWORD dwTag, DWORD dwData) override;
		virtual EResults OnHandleKeyDown (int iVirtKey) override;
		virtual ALERROR OnInit (SInitCtx &Ctx, const SDisplayOptions &Options, CString *retsError) override;
		virtual bool OnIsCurrentItemValid (void) const override;
		virtual EResults OnResetList (CSpaceObject *pLocation) override;
		virtual EResults OnSetListCursor (int iCursor) override;
		virtual EResults OnSetLocation (CSpaceObject *pLocation) override;
		virtual bool OnSelectNextItem (void) override;
		virtual bool OnSelectPrevItem (void) override;
		virtual void OnShowItem (void) override;
		virtual void OnShowPane (bool bNoListNavigation) override;

	private:
		CGCarouselArea *m_pControl = NULL;
		DWORD m_dwID = 0;
		bool m_bNoListNavigation = false;
	};

//	CDockScreenDetailsPane -----------------------------------------------------

class CDockScreenDetailsPane : public IDockScreenDisplay
	{
	public:
		CDockScreenDetailsPane (CDockScreen &DockScreen) : IDockScreenDisplay(DockScreen)
			{ }

	protected:

		//	IDockScreenDisplay

		virtual ICCItem *OnGetCurrentListEntry (void) const override;
		virtual bool OnGetDefaultBackground (SBackgroundDesc *retDesc) override { retDesc->iType = backgroundNone; return true; }
		virtual ALERROR OnInit (SInitCtx &Ctx, const SDisplayOptions &Options, CString *retsError) override;
		virtual void OnShowItem (void) override;
		virtual void OnShowPane (bool bNoListNavigation) override;

	private:
		CGDetailsArea *m_pControl = NULL;
		DWORD m_dwID = 0;
	};

//	CDockScreenList and descendants --------------------------------------------

class CDockScreenList : public IDockScreenDisplay
	{
	public:
		CDockScreenList (CDockScreen &DockScreen) : IDockScreenDisplay(DockScreen),
				m_pItemListControl(NULL),
				m_bNoListNavigation(false),
				m_dwNextFilterID(0)
			{ }

	protected:
		struct SFilter
			{
			SFilter (void) :
					bAllFilter(false),
					bDisabled(false)
				{ }

			CString sID;
			DWORD dwID;
			CString sLabel;
			CItemCriteria Filter;
			bool bAllFilter;
			bool bDisabled;
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
		virtual CSpaceObject *OnGetSource (void) const override { return m_pItemListControl->GetSource(); }
		virtual EResults OnHandleAction (DWORD dwTag, DWORD dwData) override;
		virtual EResults OnHandleKeyDown (int iVirtKey) override;
		virtual ALERROR OnInit (SInitCtx &Ctx, const SDisplayOptions &Options, CString *retsError) override;
		virtual bool OnIsCurrentItemValid (void) const override;
		virtual EResults OnResetList (CSpaceObject *pLocation) override;
		virtual bool OnSelectItem (const CItem &Item) override;
		virtual bool OnSelectNextItem (void) override;
		virtual bool OnSelectPrevItem (void) override;
		virtual EResults OnSetListCursor (int iCursor) override;
		virtual EResults OnSetListFilter (const CItemCriteria &Filter) override;
		virtual EResults OnSetLocation (CSpaceObject *pLocation) override;
		virtual void OnShowItem (void) override;
		virtual void OnShowPane (bool bNoListNavigation) override;

		//	CDockScreenList

		virtual ALERROR OnInitList (SInitCtx &Ctx, const SDisplayOptions &Options, CString *retsError) { return NOERROR; }

		//	Helpers

		bool FilterHasItems (const CItemCriteria &Filter) const;
		bool FindFilter (DWORD dwID, int *retiIndex = NULL) const;
		bool FindFilter (const CString &sID, int *retiIndex = NULL) const;
		bool SelectTab (DWORD dwID, int iFilter = -1);
		bool SetDefaultTab (const CString &sID);

		CGItemListArea *m_pItemListControl;
		DWORD m_dwID;
		bool m_bNoListNavigation;

		TArray<SFilter> m_Filters;
		DWORD m_dwNextFilterID;
	};

class CDockScreenCustomItemList : public CDockScreenList
	{
	public:
		CDockScreenCustomItemList (CDockScreen &DockScreen) : CDockScreenList(DockScreen)
			{ }

	protected:

		//	CDockScreenList

		virtual DWORD OnGetUIFlags (void) const override { return FLAG_UI_ITEM_LIST; }
		virtual ALERROR OnInitList (SInitCtx &Ctx, const SDisplayOptions &Options, CString *retsError) override;

	private:
		CItemList m_CustomItems;
	};

class CDockScreenCustomList : public CDockScreenList
	{
	public:
		CDockScreenCustomList (CDockScreen &DockScreen) : CDockScreenList(DockScreen)
			{ }

	protected:

		//	CDockScreenList

		virtual ALERROR OnInitList (SInitCtx &Ctx, const SDisplayOptions &Options, CString *retsError) override;
	};

class CDockScreenItemList : public CDockScreenList
	{
	public:
		CDockScreenItemList (CDockScreen &DockScreen) : CDockScreenList(DockScreen)
			{ }

	protected:

		//	CDockScreenList

		virtual DWORD OnGetUIFlags (void) const override { return FLAG_UI_ITEM_LIST; }
		virtual ALERROR OnInitList (SInitCtx &Ctx, const SDisplayOptions &Options, CString *retsError) override;

	private:
		CItemCriteria m_ItemCriteria;
	};

//	CDockScreenNullDisplay -----------------------------------------------------

class CDockScreenNullDisplay : public IDockScreenDisplay
	{
	public:
		CDockScreenNullDisplay (CDockScreen &DockScreen) : IDockScreenDisplay(DockScreen)
			{ }
	};

//	CDockScreenSelector --------------------------------------------------------

class CDockScreenSelector : public IDockScreenDisplay
	{
	public:
		CDockScreenSelector (CDockScreen &DockScreen, CGSelectorArea::EConfigurations iConfig) : IDockScreenDisplay(DockScreen),
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
		virtual CSpaceObject *OnGetSource (void) const override { return m_pControl->GetSource(); }
		virtual DWORD OnGetUIFlags (void) const { return FLAG_UI_ITEM_SELECTOR; }
		virtual EResults OnHandleAction (DWORD dwTag, DWORD dwData) override;
		virtual EResults OnHandleKeyDown (int iVirtKey) override;
		virtual ALERROR OnInit (SInitCtx &Ctx, const SDisplayOptions &Options, CString *retsError) override;
		virtual bool OnIsCurrentItemValid (void) const override;
		virtual EResults OnResetList (CSpaceObject *pLocation) override;
		virtual bool OnSelectNextItem (void) override;
		virtual bool OnSelectPrevItem (void) override;
		virtual EResults OnSetListCursor (int iCursor) override;
		virtual EResults OnSetListFilter (const CItemCriteria &Filter) override;
		virtual EResults OnSetLocation (CSpaceObject *pLocation) override;
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
