//	CGAreas.h
//
//	CGAreas
//	Copyright (c) 2014 by Kronosaur Productions, LLC. All Rights Reserved.

#define ITEM_LIST_AREA_PAGE_UP_ACTION			(0xffff0001)
#define ITEM_LIST_AREA_PAGE_DOWN_ACTION			(0xffff0002)

class CDetailList
	{
	public:
		CDetailList (const CVisualPalette &VI) : m_VI(VI)
			{ }

		void Format (const RECT &rcRect, int *retcyHeight = NULL);
		void Load (ICCItem *pDetails);
		void Paint (CG32bitImage &Dest) const;
        inline void SetColor (CG32bitPixel rgbColor) { m_rgbTextColor = rgbColor; }
		
	private:
		struct SDetailEntry
			{
			CString sTitle;
			CG32bitImage *pIcon = NULL;
			RECT rcIcon = { 0, 0, 0, 0 };
			CRTFText Desc;

			bool bAlignRight = false;
			RECT rcRect = { 0, 0, 0, 0 };
			int cyText = 0;
			int cyRect = 0;
			};

		static const int DETAIL_ICON_HEIGHT = 48;
		static const int DETAIL_ICON_WIDTH = 48;
		static const int SPACING_X = 8;
		static const int SPACING_Y = 8;

		const CVisualPalette &m_VI;
        CG32bitPixel m_rgbTextColor = CG32bitPixel(255, 255, 255);
		TArray<SDetailEntry> m_List;
	};

class CGCarouselArea : public AGArea
	{
	public:
		enum EStyles
			{
			styleNone,

			styleShipList,					//	Optimized for list of ship classes
			styleShipCompare,				//	Compare two ship classes
			};

		CGCarouselArea (const CVisualPalette &VI);

		void CleanUp (void);
		inline int GetCursor (void) { return (m_pListData ? m_pListData->GetCursor() : -1); }
		ICCItem *GetEntryAtCursor (void);
		inline IListData *GetList (void) const { return m_pListData; }
		inline CSpaceObject *GetSource (void) { return (m_pListData ? m_pListData->GetSource() : NULL); }
		inline bool IsCursorValid (void) const { return (m_pListData ? m_pListData->IsCursorValid() : false); }
		bool MoveCursorBack (void);
		bool MoveCursorForward (void);
		inline void ResetCursor (void) { if (m_pListData) m_pListData->ResetCursor(); Invalidate(); }
        inline void SetBackColor (CG32bitPixel rgbColor) { m_rgbBackColor = rgbColor; }
        inline void SetColor (CG32bitPixel rgbColor) { m_rgbTextColor = rgbColor; }
		inline void SetCursor (int iIndex) { if (m_pListData) m_pListData->SetCursor(iIndex); Invalidate(); }
		void SetList (CCodeChain &CC, ICCItem *pList);
		bool SetStyle (const CString &sStyle);
		inline void SyncCursor (void) { if (m_pListData) m_pListData->SyncCursor(); Invalidate(); }

		//	AGArea virtuals

		virtual bool LButtonDown (int x, int y) override;
		virtual void MouseLeave (void) override;
		virtual void MouseMove (int x, int y) override;
		virtual void MouseWheel (int iDelta, int x, int y, DWORD dwFlags) override;
		virtual void Paint (CG32bitImage &Dest, const RECT &rcRect) override;
		virtual void Update (void) override;

	private:
		static const int BORDER_RADIUS = 4;
		static const int DEFAULT_ICON_HEIGHT = 64;
		static const int DEFAULT_ICON_WIDTH = 64;
		static const int SPACING_X = 8;
		static const int SPACING_Y = 8;
		static const int SELECTION_WIDTH = 2;
		static const int SELECTOR_HEIGHT = DEFAULT_ICON_HEIGHT + 40;
		static const int SELECTOR_WIDTH = 140;
		static const int SELECTOR_AREA_HEIGHT = SELECTOR_HEIGHT + SPACING_Y;
		static const int MOUSE_SCROLL_SENSITIVITY =	30;
		static const int SELECTOR_PADDING_TOP =	2;
		static const int SELECTOR_PADDING_LEFT = 4;
		static const int SELECTOR_PADDING_RIGHT = 4;
		static const int LARGE_ICON_HEIGHT = 320;
		static const int LARGE_ICON_WIDTH = 320;
		static const int DETAIL_ICON_HEIGHT = 48;
		static const int DETAIL_ICON_WIDTH = 48;

		int FindSelector (int x) const;
		void PaintContent (CG32bitImage &Dest, const RECT &rcRect) const;
		void PaintList (CG32bitImage &Dest, const RECT &rcRect);
		void PaintListShipCompare (CG32bitImage &Dest, const RECT &rcRect);
		void PaintSelector (CG32bitImage &Dest, const RECT &rcRect, bool bSelected) const;

		TUniquePtr<IListData> m_pListData;
		EStyles m_iStyle = styleNone;
		int m_iOldCursor = -1;							//	Cursor pos

		const CVisualPalette &m_VI;
        CG32bitPixel m_rgbTextColor = CG32bitPixel(255, 255, 255);
        CG32bitPixel m_rgbBackColor = CG32bitPixel(0, 0, 0);
		CG32bitPixel m_rgbDisabledText = CG32bitPixel(128,128,128);
		int m_xOffset = 0;								//	Painting offset for smooth scroll
		int m_xFirst = 0;								//	coord of first row relative to list rect
		int m_cxSelector = SELECTOR_WIDTH;				//	Width of selector cell
		int m_cySelector = SELECTOR_HEIGHT;				//	Height of selector cell
		int m_cySelectorArea = SELECTOR_AREA_HEIGHT;	//	Height of selector area
		int m_cxIcon = DEFAULT_ICON_WIDTH;				//	Icon width
		int m_cyIcon = DEFAULT_ICON_HEIGHT;				//	Icon height
		int m_cxLargeIcon = LARGE_ICON_WIDTH;			//	Large content icon
		int m_cyLargeIcon = LARGE_ICON_HEIGHT;			//	Large content icon
		Metric m_rIconScale = 1.0;						//	Icon scale
	};

class CGDetailsArea : public AGArea
	{
	public:
		CGDetailsArea (const CVisualPalette &VI);

		void CleanUp (void);
		inline ICCItem *GetData (void) const { return m_pData; }
        inline void SetBackColor (CG32bitPixel rgbColor) { m_rgbBackColor = rgbColor; }
        inline void SetColor (CG32bitPixel rgbColor) { m_rgbTextColor = rgbColor; }
		inline void SetData (CCodeChain &CC, ICCItem *pList) { m_pData = pList; }

		//	AGArea virtuals

		virtual void Paint (CG32bitImage &Dest, const RECT &rcRect) override;

	private:
		static const int BORDER_RADIUS = 4;
		static const int SPACING_X = 8;
		static const int SPACING_Y = 8;
		static const int LARGE_ICON_HEIGHT = 320;
		static const int LARGE_ICON_WIDTH = 320;
		static const int DETAIL_ICON_HEIGHT = 48;
		static const int DETAIL_ICON_WIDTH = 48;

		ICCItemPtr m_pData;

		const CVisualPalette &m_VI;
        CG32bitPixel m_rgbTextColor = CG32bitPixel(255, 255, 255);
        CG32bitPixel m_rgbBackColor = CG32bitPixel(0, 0, 0);
		CG32bitPixel m_rgbDisabledText = CG32bitPixel(128,128,128);
		int m_cxLargeIcon = LARGE_ICON_WIDTH;			//	Large content icon
		int m_cyLargeIcon = LARGE_ICON_HEIGHT;			//	Large content icon
	};

class CGDrawArea : public AGArea
	{
	public:
		CGDrawArea (void);

		inline CG32bitImage &GetCanvas (void) { CreateImage(); return m_Image; }

		//	AGArea virtuals
		virtual void Paint (CG32bitImage &Dest, const RECT &rcRect) override;

	private:
		void CreateImage (void);

		CG32bitImage m_Image;

		CG32bitPixel m_rgbBackColor;
		bool m_bTransBackground;
	};

class CGItemDisplayArea : public AGArea
	{
	public:
		CGItemDisplayArea (void);

        inline void SetBackColor (CG32bitPixel rgbColor) { m_rgbBackColor = rgbColor; }
        inline void SetColor (CG32bitPixel rgbColor) { m_rgbTextColor = rgbColor; }
		inline void SetItem (CSpaceObject *pSource, const CItem &Item) { m_pSource = pSource; m_Item = Item; Invalidate(); }
		inline void SetText (const CString &sTitle, const CString &sDesc) { m_sTitle = sTitle; m_sDesc = sDesc; Invalidate(); }

		//	AGArea virtuals
		virtual int Justify (const RECT &rcRect) override;
		virtual void Paint (CG32bitImage &Dest, const RECT &rcRect) override;

	private:
		CSpaceObject *m_pSource;
		CItem m_Item;

		CString m_sTitle;					//	If no item
		CString m_sDesc;					//	If no item

        CG32bitPixel m_rgbTextColor;
        CG32bitPixel m_rgbBackColor;
	};

class CGItemListDisplayArea : public AGArea
	{
	public:
		CGItemListDisplayArea (void);

		bool InitFromDesc (CCodeChain &CC, ICCItem *pDesc);
        inline void SetBackColor (CG32bitPixel rgbColor) { m_rgbBackColor = rgbColor; }
        inline void SetColor (CG32bitPixel rgbColor) { m_rgbTextColor = rgbColor; }
		void SetItemList (CSpaceObject *pSource, const CItemList &ItemList);
		inline void SetText (const CString &sTitle, const CString &sDesc) { m_sTitle = sTitle; m_sDesc = sDesc; m_ItemList.DeleteAll(); Invalidate(); }

		//	AGArea virtuals
		virtual int Justify (const RECT &rcRect) override;
		virtual void Paint (CG32bitImage &Dest, const RECT &rcRect) override;

	private:
		struct SEntry
			{
			SEntry (void) :
					bGrayed(false)
				{ }

			CItem Item;
			bool bGrayed;
			};

		bool InitFromItemList (CCodeChain &CC, ICCItem *pItemList);
		void SortItemList (void);

		CSpaceObject *m_pSource;
		TArray<SEntry> m_ItemList;

		CString m_sTitle;					//	If no items
		CString m_sDesc;					//	If no items

		CG32bitPixel m_rgbTextColor;
		CG32bitPixel m_rgbBackColor;

		//	Initialized on Justify

		int m_cxBox;
		int m_cyBox;
		int m_iCols;
		int m_iRows;
		int m_xOffset;
		int m_xLastRowOffset;
	};

class CGItemListArea : public AGArea
	{
	public:
		CGItemListArea (const CVisualPalette &VI);
		~CGItemListArea (void);

		void AddTab (DWORD dwID, const CString &sLabel);
		void CleanUp (void);
		void EnableTab (DWORD dwID, bool bEnabled = true);
		inline void DeleteAtCursor (int iCount) { if (m_pListData) m_pListData->DeleteAtCursor(iCount); InitRowDesc(); Invalidate(); }
		inline int GetCursor (void) { return (m_pListData ? m_pListData->GetCursor() : -1); }
		ICCItem *GetEntryAtCursor (void);
		inline const CItem &GetItemAtCursor (void) { return (m_pListData ? m_pListData->GetItemAtCursor() : g_DummyItem); }
		inline CItemListManipulator &GetItemListManipulator (void) { return (m_pListData ? m_pListData->GetItemListManipulator() : g_DummyItemListManipulator); }
		inline IListData *GetList (void) const { return m_pListData; }
		bool GetNextTab (DWORD *retdwID) const;
		bool GetPrevTab (DWORD *retdwID) const;
		inline CSpaceObject *GetSource (void) { return (m_pListData ? m_pListData->GetSource() : NULL); }
		inline bool IsCursorValid (void) const { return (m_pListData ? m_pListData->IsCursorValid() : false); }
		bool MoveCursorBack (void);
		bool MoveCursorForward (void);
		void MoveTabToFront (DWORD dwID);
		inline void ResetCursor (void) { if (m_pListData) m_pListData->ResetCursor(); InitRowDesc(); Invalidate(); }
		void SelectTab (DWORD dwID);
        inline void SetBackColor (CG32bitPixel rgbColor) { m_rgbBackColor = rgbColor; }
        inline void SetColor (CG32bitPixel rgbColor) { m_rgbTextColor = rgbColor; }
		inline void SetCursor (int iIndex) { if (m_pListData) m_pListData->SetCursor(iIndex); Invalidate(); }
		inline void SetDisplayAsKnown (bool bValue = true) { m_bActualItems = bValue; }
		inline void SetFilter (const CItemCriteria &Filter) { if (m_pListData) m_pListData->SetFilter(Filter); InitRowDesc(); Invalidate(); }
		inline void SetFontTable (const SFontTable *pFonts) { m_pFonts = pFonts; }
		inline void SetIconHeight (int cyHeight) { m_cyIcon = cyHeight; }
		inline void SetIconScale (Metric rScale) { m_rIconScale = rScale; }
		inline void SetIconWidth (int cxWidth) { m_cxIcon = cxWidth; }
		void SetList (CCodeChain &CC, ICCItem *pList);
		void SetList (CSpaceObject *pSource);
		void SetList (CItemList &ItemList);
		inline void SetNoArmorSpeedDisplay (bool bValue = true) { m_bNoArmorSpeedDisplay = bValue; }
		inline void SetRowHeight (int cyHeight) { m_cyRow = Max(1, cyHeight); }
		inline void SetUIRes (const CUIResources *pUIRes) { m_pUIRes = pUIRes; }
		inline void SyncCursor (void) { if (m_pListData) m_pListData->SyncCursor(); Invalidate(); }

		//	AGArea virtuals
		virtual bool LButtonDown (int x, int y) override;
		virtual void MouseLeave (void) override;
		virtual void MouseMove (int x, int y) override;
		virtual void MouseWheel (int iDelta, int x, int y, DWORD dwFlags) override;
		virtual void Paint (CG32bitImage &Dest, const RECT &rcRect) override;
		virtual void Update (void) override;

	private:
		enum ListTypes
			{
			listNone,
			listItem,
			listCustom,
			};

		struct SRowDesc
			{
			int yPos;							//	Position of the row (sum of height of previous rows)
			int cyHeight;						//	Height of this row
			};

		struct STabDesc
			{
			STabDesc (void) :
					bDisabled(false)
				{ }

			DWORD dwID;
			CString sLabel;
			int cxWidth;
			bool bDisabled;
			};

		static const int DEFAULT_ROW_HEIGHT =				96;
		static const int ICON_WIDTH =						96;
		static const int ICON_HEIGHT =						96;


		int CalcRowHeight (int iRow);
		void InitRowDesc (void);
		int FindRow (int y);
		bool FindTab (DWORD dwID, int *retiIndex = NULL) const;
		bool HitTestTabs (int x, int y, int *retiTab);
		void PaintCustom (CG32bitImage &Dest, const RECT &rcRect, bool bSelected);
		void PaintItem (CG32bitImage &Dest, const CItem &Item, const RECT &rcRect, bool bSelected);
		void PaintTab (CG32bitImage &Dest, const STabDesc &Tab, const RECT &rcRect, bool bSelected, bool bHover);

		IListData *m_pListData = NULL;
		ListTypes m_iType = listNone;

		const CVisualPalette &m_VI;
		const CUIResources *m_pUIRes = NULL;
		const SFontTable *m_pFonts = NULL;
        CG32bitPixel m_rgbTextColor = CG32bitPixel(255, 255, 255);
        CG32bitPixel m_rgbBackColor = CG32bitPixel(0, 0, 0);
		int m_iOldCursor = -1;					//	Cursor pos
		int m_yOffset = 0;						//	Painting offset for smooth scroll
		int m_yFirst = 0;						//	coord of first row relative to list rect
		int m_cyRow = DEFAULT_ROW_HEIGHT;		//	Row height
		int m_cxIcon = ICON_WIDTH;				//	Icon width
		int m_cyIcon = ICON_HEIGHT;				//	Icon height
		Metric m_rIconScale = 1.0;				//	Icon scale

		int m_cyTotalHeight = 0;				//	Total heigh of all rows
		TArray<SRowDesc> m_Rows;

		TArray<STabDesc> m_Tabs;
		int m_iCurTab = -1;						//	Current selected tab (-1 = none)
		int m_iHoverTab = -1;					//	Hover tab (-1 = none)
		int m_cyTabHeight = 0;					//	Height of tab row

		bool m_bNoArmorSpeedDisplay = false;	//	Do not show armor bonus/penalty to speed
		bool m_bActualItems = false;			//	Show actual items, even if unknown
	};

class CGNeurohackArea : public AGArea
	{
	public:
		CGNeurohackArea (void);
		~CGNeurohackArea (void);

		void CleanUp (void);
		void SetData (int iWillpower, int iDamage);
		inline void SetFontTable (const SFontTable *pFonts) { m_pFonts = pFonts; }

		//	AGArea virtuals
		virtual void Paint (CG32bitImage &Dest, const RECT &rcRect) override;
		virtual void Update (void) override;

	private:
		struct SNode
			{
			int x;
			int y;
			int iWidth;
			int iSphereSize;
			int iRootDist;
			int iDamageLevel;

			SNode *pParent;
			SNode *pNext;
			SNode *pPrev;
			SNode *pFirstChild;
			};

		void CreateBranch (SNode *pParent, int iDirection, int iGeneration, int iWidth, const RECT &rcRect);
		void CreateNetwork (const RECT &rcRect);
		SNode *CreateNode (SNode *pParent, int x, int y);
		void PaintBranch (CG32bitImage &Dest, SNode *pNode, SNode *pNext = NULL);
		void PaintSphere (CG32bitImage &Dest, int x, int y, int iRadius, CG32bitPixel rgbGlowColor);

		const SFontTable *m_pFonts;

		SNode *m_pNetwork;
		int m_iNodeCount;
		int m_iNodeAlloc;

		SNode **m_pRootNodes;
		int m_iRootCount;
		int m_iRootAlloc;

		SNode **m_pTerminalNodes;
		int m_iTerminalCount;
		int m_iTerminalAlloc;

		SNode **m_pActiveNodes;
		int m_iActiveCount;
		int m_iActiveAlloc;

		int m_iWillpower;
		int m_iDamage;
	};

class CGSelectorArea : public AGArea
	{
	public:
		enum EConfigurations
			{
			configNone,

			configArmor,
            configDevices,
			configMiscDevices,
			configWeapons,
			};

		enum EDirections
			{
			moveDown,						//	Move cursor to region below
			moveLeft,						//	Move cursor to region to the left
			moveRight,						//	Move cursor to region to the right
			moveUp,							//	Move cursor to region above

			moveNext,						//	Move to next region
			movePrev,						//	Move to previous region
			};

        struct SOptions
            {
            SOptions (void) :
                    iConfig(configDevices),
                    bNoEmptySlots(false),
                    bAlwaysShowShields(false)
                {
                CItem::InitCriteriaAll(&ItemCriteria);
                }

            EConfigurations iConfig;
            CItemCriteria ItemCriteria;
            bool bNoEmptySlots;
            bool bAlwaysShowShields;
            };

		CGSelectorArea (const CVisualPalette &VI);
		~CGSelectorArea (void);

		inline int GetCursor (void) { return m_iCursor; }
		ICCItem *GetEntryAtCursor (void);
		const CItem &GetItemAtCursor (void);
		inline IListData *GetList (void) const { return NULL; }
		inline CSpaceObject *GetSource (void) { return m_pSource; }
		inline bool IsCursorValid (void) const { return (m_iCursor != -1); }
		bool MoveCursor (EDirections iDir);
		void Refresh (void);
		inline void ResetCursor (void) { m_iCursor = -1; Invalidate(); }
        inline void SetBackColor (CG32bitPixel rgbColor) { m_rgbBackColor = rgbColor; }
        inline void SetColor (CG32bitPixel rgbColor) { m_rgbTextColor = rgbColor; }
		inline void SetCursor (int iIndex) { m_iCursor = iIndex; Invalidate(); }
		void SetRegions (CSpaceObject *pSource, const SOptions &Options);
        void SetSlotNameAtCursor (const CString &sName);
		void SyncCursor (void) { if (m_iCursor != -1 && m_iCursor >= m_Regions.GetCount()) m_iCursor = m_Regions.GetCount() - 1; }

		//	AGArea virtuals

		virtual bool LButtonDown (int x, int y) override;
		virtual void Paint (CG32bitImage &Dest, const RECT &rcRect) override;
		virtual void Update (void) override;

	private:
		enum ETypes
			{
			typeNone,

			typeEmptySlot,
			typeInstalledItem,
			};

		struct SEntry
			{
			SEntry (void) :
					iType(typeNone),
					pItemCtx(NULL),
					iSlotType(devNone),
					iSlotPosIndex(-1)
				{ }

			~SEntry (void)
				{
				if (pItemCtx)
					delete pItemCtx;
				}

			ETypes iType;					//	Type of entry
			CItemCtx *pItemCtx;				//	Item represented (may be NULL)
			DeviceNames iSlotType;			//	Type of slot (if empty)
			CString sSlotID;				//	ID of slot (may be NULL_STR)
            CString sSlotName;              //  If empty

			int iSlotPosIndex;				//	Position index
			RECT rcRect;					//	Location of region (always relative to the center
											//	of the area).
			};

		void CalcRegionRect (const SEntry &Entry, int xCenter, int yCenter, RECT *retrcRect);
		void CleanUp (void);
		bool FindLayoutForPos (const CVector &vPos, const TArray<bool> &SlotStatus, int *retiIndex = NULL);
		bool FindNearestRegion (int xCur, int yCur, EDirections iDir, bool bDiagOnly, int *retiIndex) const;
		bool FindRegionInDirection (EDirections iDir, int *retiIndex = NULL) const;
		void PaintEmptySlot (CG32bitImage &Dest, const RECT &rcRect, const SEntry &Entry);
		void PaintInstalledItem (CG32bitImage &Dest, const RECT &rcRect, const SEntry &Entry);
		void PaintModifier (CG32bitImage &Dest, int x, int y, const CString &sText, CG32bitPixel rgbColor, CG32bitPixel rgbBackColor, int *rety);
		void SetRegionsFromArmor (CSpaceObject *pSource);
		void SetRegionsFromDevices (CSpaceObject *pSource);
		void SetRegionsFromMiscDevices (CSpaceObject *pSource);
		void SetRegionsFromWeapons (CSpaceObject *pSource);

		const CVisualPalette &m_VI;
        CG32bitPixel m_rgbTextColor = CG32bitPixel(255, 255, 255);
        CG32bitPixel m_rgbBackColor;

		CSpaceObject *m_pSource = NULL;
        CItemCriteria m_Criteria;
		EConfigurations m_iConfig = configNone;
        bool m_bNoEmptySlots = false;		//  Do not show empty slots
        bool m_bAlwaysShowShields = false;	//  Always show shields in armor selected, even
                                            //      if not part of criteria

		TArray<SEntry> m_Regions;
		int m_iCursor = -1;
	};
