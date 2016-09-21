//	DockScreenSubjugate.h
//
//	Dock Screen subjugateMinigame
//	Copyright (c) 2016 by Kronosaur Productions, LLC. All Rights Reserved.

#pragma once

class CDockScreenSubjugate;

class CArtifactAwakenVisuals
	{
	public:
		enum EColors
			{
			colorDeployDaimonBack =				0,
			colorDeployDaimonFore =				1,
			colorDeployDaimonTitle =			2,
			colorDaimonLocusBack =				3,
			colorCountermeasureLocusBack =		4,
			colorAICoreBack =					5,
			colorAICoreFore =					6,

			colorCount =						7,
			};

		static CG32bitPixel GetColor (EColors iColor);
	};

class CArtifactAICorePainter
	{
	public:
		CArtifactAICorePainter (void);

		void Paint (CG32bitImage &Dest, int x, int y);

	private:
	};

class CDaimonButtonPainter
	{
	public:
		enum EStyles
			{
			styleNone,

			styleMain,
			};

		enum EStates
			{
			stateNone,

			stateNormal,
			stateHover,
			stateDown,
			stateDisabled,
			};

		CDaimonButtonPainter (const CVisualPalette &VI);

		bool HitTest (int x, int y) const;
		void Paint (CG32bitImage &Dest, EStates iState) const;
		inline void SetLabel (const CString &sLabel) { m_sLabel = sLabel; }
		inline void SetPos (int x, int y) { m_xPos = x; m_yPos = y; }
		inline void SetStyle (EStyles iStyle) { m_iStyle = iStyle; }
		void Update (void);

	private:
		void PaintMainStyle (CG32bitImage &Dest, EStates iState) const;

		const CVisualPalette &m_VI;

		EStyles m_iStyle;
		int m_xPos;							//	Center position of button
		int m_yPos;
		int m_iRadius;						//	Button radius
		CString m_sLabel;					//	Label
	};

class CDaimonList
	{
	public:
		CDaimonList (void);
		~CDaimonList (void);

		void Add (CItemType *pItem);
		void DeleteAll (void);
		int DeleteSelectedDaimon (void);
		inline int GetCount (void) const { return m_Sorted.GetCount(); }
		inline CItemType *GetDaimon (int iIndex) const { return (IsValid(iIndex) ? m_Sorted[iIndex]->pDaimon : NULL); }
		inline int GetDaimonHeight (int iIndex) const { return (IsValid(iIndex) ? m_Sorted[iIndex]->cyHeight : 0); }
		inline int GetSelection (void) const { return m_iSelection; }
		inline void SetDaimonHeight (int iIndex, int cyHeight) { if (IsValid(iIndex)) m_Sorted[iIndex]->cyHeight = cyHeight; }
		inline int SetSelection (int iSelection) { if (GetCount() == 0) return -1; m_iSelection = Max(0, Min(iSelection, GetCount() - 1)); return m_iSelection; }

	private:
		struct SDaimonEntry
			{
			SDaimonEntry (void) :
					dwID(0),
					pDaimon(NULL),
					cyHeight(-1)
				{ }

			DWORD dwID;
			CItemType *pDaimon;

			int cyHeight;					//	Height (-1 if not yet justified)
			};

		inline bool IsValid (int iIndex) const { return (iIndex >= 0 && iIndex < m_Sorted.GetCount()); }

		DWORD m_dwNextID;
		TSortMap<DWORD, SDaimonEntry *> m_List;
		TSortMap<CString, SDaimonEntry *> m_Sorted;

		int m_iSelection;					//	Currently selected index (-1 = no selection)
	};

class CDaimonListPainter
	{
	public:
		CDaimonListPainter (const CVisualPalette &VI);

		bool HitTest (int xPos, int yPos, int *retiIndex) const;
		void OnSelectionChanged (int iOldSelection, int iNewSelection);
		void OnSelectionDeleted (int iOldSelection);
		void Paint (CG32bitImage &Dest);
		inline void SetList (CDaimonList &List) { m_pList = &List; }
		inline void SetRect (const RECT &rcRect) { m_rcRect = rcRect; m_cxWidth = RectWidth(rcRect); }
		bool Update (void);

	private:
		int CalcListPos (void) const;
		int CalcTop (int iIndex) const;
		void PaintDaimon (CG32bitImage &Dest, CItemType *pDaimon, int x, int y, int cxWidth, int cyHeight);
		int Justify (int cxWidth) const;

		const CVisualPalette &m_VI;
		CDaimonList *m_pList;
		RECT m_rcRect;
		int m_cxWidth;						//	Width to paint
		int m_yAnimation;					//	Animate scrolling
	};

class CGSubjugateArea : public AGArea
	{
	public:
		enum ECommands
			{
			cmdDeployDaimon,				//	Deploys the selected daimon
			cmdSelectPrevDaimon,
			cmdSelectNextDaimon,
			};

		CGSubjugateArea (const CVisualPalette &VI, CDockScreenSubjugate &Controller, CArtifactAwakening &Artifact);
		~CGSubjugateArea (void);

		void Command (ECommands iCommand, void *pData = NULL);
		bool IsCommandValid (ECommands iCommand, void *pData = NULL) const;

		//	AGArea virtuals

		virtual bool LButtonDoubleClick (int x, int y) override;
		virtual bool LButtonDown (int x, int y) override;
		virtual void LButtonUp (int x, int y) override;
		virtual void MouseEnter (void) override;
		virtual void MouseLeave (void) override;
		virtual void MouseMove (int x, int y) override;
		virtual void MouseWheel (int iDelta, int x, int y, DWORD dwFlags) override;
		virtual void Paint (CG32bitImage &Dest, const RECT &rcRect) override;
		virtual void Update (void) override;

	protected:
		virtual void OnSetRect (void) override;

	private:
		enum EStates
			{
			stateStart,						//	Have not yet deployed a single daimon
			stateInBattle,					//	Have deployed a daimon, but no resolution
			stateSuccess,					//	Artifact subjugated
			stateFailure,					//	Failed to subjugate
			};

		enum ESelectionTypes
			{
			selectNone,

			selectCountermeasureLoci,		//	A countermeasure locus
			selectDaimonList,				//	A daimon in the list.
			selectDaimonLoci,				//	A daimon locus (a deployed daimon)
			selectDeployBtn,				//	The Deploy button
			};

		struct SCountermeasureLocus
			{
			int iIndex;						//	Locus index
			int iStartAngle;				//	Starting angle (degrees)
			int iArc;						//	Arc in degrees (counter-clockwise)
			int iInnerRadius;				//	Inner radius in pixels
			int iOuterRadius;				//	Outer radius in pixels
			};

		struct SDaimonLocus
			{
			int iIndex;						//	Locus index
			int xPos;						//	Pos of upper-left corner relative to center
			int yPos;
			int cxWidth;
			int cyHeight;
			};

		struct SSelection
			{
			SSelection (void) :
					iType(selectNone),
					iIndex(-1)
				{ }

			bool operator== (const SSelection &Src) const 
				{ return (iType == Src.iType && iIndex == Src.iIndex); }
			bool operator!= (const SSelection &Src) const 
				{ return (iType != Src.iType || iIndex != Src.iIndex); }

			ESelectionTypes iType;
			int iIndex;						//	Index of countermeasure, daimon, etc.
			};

		void ArtifactSubdued (void);
		void DeployDaimon (void);
		void HideInfoPane (void);
		bool HitTest (int x, int y, SSelection &Sel) const;
		bool HitTestCountermeasureLoci (int x, int y, int *retiIndex = NULL) const;
		inline bool IsActive (void) const { return (m_iState == stateStart || m_iState == stateInBattle); }
		void PaintCountermeasureLocus (CG32bitImage &Dest, const SCountermeasureLocus &Locus) const;
		void PaintDaimonLocus (CG32bitImage &Dest, const SDaimonLocus &Locus) const;
		void PaintProgram (CG32bitImage &Dest, const CArtifactProgram &Program, int x, int y) const;
		void SelectDaimon (int iNewSelection);

		const CVisualPalette &m_VI;
		CDockScreenSubjugate &m_Controller;

		//	Game state

		EStates m_iState;
		CArtifactAwakening &m_Artifact;		//	Core mini-game
		CDaimonList m_DaimonList;			//	List of available daimons to deploy

		CArtifactAICorePainter m_AICorePainter;
		TArray<SDaimonLocus> m_DaimonLoci;	//	Locations to which daimons are deployed
		TArray<SCountermeasureLocus> m_CountermeasureLoci;

		//	UI state

		SSelection m_Hover;					//	What we're currently hovering over
		SSelection m_Clicked;				//	What we've clicked down on (but not yet up)
		CHoverDescriptionPainter m_InfoPane;//	Info pane on hover
		SSelection m_InfoPaneSel;			//	What the info pane is showing

		CDaimonListPainter m_DaimonListPainter;
		CDaimonButtonPainter m_DeployBtn;	//	Deploy button

		//	Metrics

		int m_xCenter;						//	Center of AI core
		int m_yCenter;

		RECT m_rcHand;						//	List of available daimons to deploy

		//	Visuals

		CG32bitPixel m_rgbCountermeasureBack;
		CG32bitPixel m_rgbCountermeasureLabel;
		CG32bitPixel m_rgbDaimonBack;
		CG32bitPixel m_rgbDaimonLabel;
	};

class CDockScreenSubjugate : public IDockScreenDisplay
	{
	public:
		CDockScreenSubjugate (void);

		void OnCompleted (bool bSuccess);
		void OnStarted (void);

	protected:

		//	IDockScreenDisplay

		virtual bool OnGetDefaultBackground (SBackgroundDesc *retDesc) override { retDesc->iType = backgroundNone; return true; }
		virtual EResults OnHandleKeyDown (int iVirtKey) override;
		virtual ALERROR OnInit (SInitCtx &Ctx, const SDisplayOptions &Options, CString *retsError) override;

	private:
		void FireOnCompleted (bool bSuccess);

		CArtifactAwakening m_Artifact;		//	Mini-game data
		CGSubjugateArea *m_pControl;
		DWORD m_dwID;						//	ID of control
		CEventHandler m_Events;
	};

