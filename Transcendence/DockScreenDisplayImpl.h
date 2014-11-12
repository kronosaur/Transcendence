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
		virtual CSpaceObject *OnGetSource (void) { return m_pItemListControl->GetSource(); }
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

		CGItemListArea *m_pItemListControl;
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

		virtual void OnDeleteCurrentItem (int iCount);
		virtual const CItem &OnGetCurrentItem (void) const;
		virtual ICCItem *OnGetCurrentListEntry (void) const;
		virtual bool OnGetDefaultBackgroundObj (CSpaceObject **retpObj);
		virtual int OnGetListCursor (void) { return m_pControl->GetCursor(); }
		virtual IListData *OnGetListData (void) { return m_pControl->GetList(); }
		virtual CSpaceObject *OnGetSource (void) { return m_pControl->GetSource(); }
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
