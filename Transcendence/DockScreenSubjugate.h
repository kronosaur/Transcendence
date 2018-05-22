//	DockScreenSubjugate.h
//
//	Dock Screen subjugateMinigame
//	Copyright (c) 2016 by Kronosaur Productions, LLC. All Rights Reserved.

#pragma once

class CDockScreen;
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

		enum EGlyphs
			{
			glyphNone =							0,

			glyphRight =						1,
			};

		static CG32bitPixel GetColor (EColors iColor);
		static void PaintGlyph (CG32bitImage &Dest, EGlyphs iGlyph, int x, int y, int cxWidth, int cyHeight, CG32bitPixel rgbColor);
	};

#define AA_STYLECOLOR(x)						(CArtifactAwakenVisuals::GetColor(CArtifactAwakenVisuals::##x))

class CArtifactAICorePainter
	{
	public:
		CArtifactAICorePainter (void);

		void Paint (CG32bitImage &Dest, int x, int y);
		void Update (void);

	private:
		static void CreateSphere (Metric rRadius, TArray<CVector3D> &Result);
		static void CreateSphereRandom (int iPoints, Metric rRadius, TArray<CVector3D> &Result);

		TArray<CVector3D> m_SpherePoints;
		TArray<CGeometry3D::SIndexedLine> m_SphereEdges;
		CStepIncrementor m_RadiusAdj1;
		CStepIncrementor m_RadiusAdj2;
		CStepIncrementor m_RadiusAdj3;
		CVector3D m_vRotationRate;			//	Rotation per tick

		int m_iTick;						//	Current tick
		CVector3D m_vRotation;				//	Current rotation

		CProjection3D m_Camera;
		CXForm3D m_Xform;
	};

class CArtifactStatPainter
	{
	public:
		CArtifactStatPainter (const CVisualPalette &VI);

		inline void GetPos (int *retx, int *rety) const { RectCenter(m_rcRect, retx, rety); }
		void Paint (CG32bitImage &Dest) const;
		inline void SetLabel (const CString &sValue) { m_sLabel = sValue; }
		inline void SetRect (const RECT &rcRect) { m_rcRect = rcRect; }
		inline void SetStat (int iValue) { m_iValue = iValue; }

	private:
		const CVisualPalette &m_VI;
		CString m_sLabel;
		int m_iValue;

		RECT m_rcRect;
	};

class CArtifactResultPainter
	{
	public:
		CArtifactResultPainter (CArtifactProgram *pSource, 
				int xSource, 
				int ySource, 
				CArtifactAwakening::EEventTypes iEvent, 
				CArtifactProgram *pTarget, 
				int xTarget, 
				int yTarget,
				int iDelay);

		inline CArtifactAwakening::EEventTypes GetEventType (void) const { return m_iEvent; }
		inline CArtifactProgram *GetProgram (void) const { return m_pSource; }
		CArtifactProgram *GetProgramActivated (void) const;
		CArtifactProgram *GetProgramHalted (void) const;
		inline CArtifactProgram *GetTarget (void) const { return m_pTarget; }
		bool IsEqualTo (const CArtifactResultPainter &Src) const;
		inline bool IsMarked (void) const { return m_bMarked; }
		inline void Mark (bool bValue = true) { m_bMarked = bValue; }
		void Paint (CG32bitImage &Dest) const;
		void Update (void);

	private:
		enum EStyles
			{
			styleNone,

			styleActivateProgram,
			styleArcLightning,
			styleCircuit,
			};

		static CVector CircuitLineBendPoint (const CVector &vFrom, const CVector &vTo);
		static void CircuitLineCreate (const CVector &vFrom, const CVector &vTo, const CVector &vMiddle, TArray<CVector> &Result);
		static void CircuitLineSplit (TArray<CVector> &Result, int iStart, int iEnd, int iDepth, int *retiNewEnd = NULL);
		static CVector CircuitLineSplitPoint (const CVector &vFrom, const CVector &vTo);

		CArtifactProgram *m_pSource;
		int m_xSource;
		int m_ySource;

		CArtifactAwakening::EEventTypes m_iEvent;
		CArtifactProgram *m_pTarget;
		int m_xTarget;
		int m_yTarget;

		EStyles m_iStyle;
		CG32bitPixel m_rgbPrimaryColor;
		CG32bitPixel m_rgbSecondaryColor;
		int m_iDelay;

		TArray<CVector> m_Line;
		CGRegion m_Region;

		bool m_bMarked;
	};

class CArtifactMessagePainter
	{
	public:
		enum EStyles
			{
			styleNone,

			styleInfo,
			styleDanger,
			};
		
		CArtifactMessagePainter (const CVisualPalette &VI);

		void AddMessage (const CString &sText, EStyles iStyle, int x, int y, int iDelay = 0);
		void Paint (CG32bitImage &Dest) const;
		void Update (void);

	private:
		enum EStates
			{
			stateNone,

			stateBlinkIn,
			stateDelay,
			stateDelete,
			stateFadeOut,
			stateNormal,
			stateTTYIn,
			};

		struct SEntry
			{
			SEntry (void) :
					iStyle(styleNone),
					iState(stateNone),
					iTick(0),
					pFont(NULL)
				{ }

			EStyles iStyle;
			CString sText;
			int x;
			int y;

			EStates iState;
			int iTick;
			const CG16bitFont *pFont;
			CG32bitPixel rgbText;
			};


		void PaintBlinkIn (CG32bitImage &Dest, SEntry &Entry) const;
		void PaintFadeOut (CG32bitImage &Dest, SEntry &Entry) const;
		void PaintNormal (CG32bitImage &Dest, SEntry &Entry) const;
		void PaintTTYIn (CG32bitImage &Dest, SEntry &Entry) const;
		void UpdateNextState (SEntry &Entry) const;

		const CVisualPalette &m_VI;
		TArray<SEntry> m_Entries;
		int m_iTick;
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

		void CleanUp (void);
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
			SCountermeasureLocus (void) :
					iIndex(-1),
					bHidden(false),
					bPaintHalted(false)
				{ }

			int iIndex;						//	Locus index
			int iStartAngle;				//	Starting angle (degrees)
			int iArc;						//	Arc in degrees (counter-clockwise)
			int iInnerRadius;				//	Inner radius in pixels
			int iOuterRadius;				//	Outer radius in pixels

			bool bHidden;					//	Don't paint
			bool bPaintHalted;				//	Paint grayed out
			};

		struct SDaimonLocus
			{
			SDaimonLocus (void) :
					iIndex(-1),
					bPaintHalted(false)
				{ }

			int iIndex;						//	Locus index
			int xPos;						//	Pos of upper-left corner relative to center
			int yPos;
			int cxWidth;
			int cyHeight;

			bool bPaintHalted;
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

		void AddEffect (CArtifactResultPainter *pEffect);
		bool AddEffect (const CArtifactAwakening::SEventDesc &Event, int iDelay);
		void ArtifactSubdued (void);
		void DeployDaimon (void);
		void GetProgramPos (CArtifactProgram::EProgramTypes iType, int iLocus, int *retx, int *rety) const;
		void HideInfoPane (void);
		bool HitTest (int x, int y, SSelection &Sel) const;
		bool HitTestCountermeasureLoci (int x, int y, int *retiIndex = NULL) const;
		inline bool IsActive (void) const { return (m_Artifact.GetStatus() == CArtifactAwakening::resultBattleContinues) || (m_Artifact.GetStatus() == CArtifactAwakening::resultNone); }
		void PaintCoreStats (CG32bitImage &Dest) const;
		void PaintCountermeasureLocus (CG32bitImage &Dest, const SCountermeasureLocus &Locus) const;
		void PaintDaimonLocus (CG32bitImage &Dest, const SDaimonLocus &Locus) const;
		void PaintProgram (CG32bitImage &Dest, const CArtifactProgram &Program, int x, int y, bool bGrayed) const;
		void PlayerFailed (void);
		void RefreshEffects (const TArray<CArtifactAwakening::SEventDesc> &Events);
		void RefreshStatsPainters (void);
		void SelectDaimon (int iNewSelection);

		const CVisualPalette &m_VI;
		CDockScreenSubjugate &m_Controller;

		//	Game state

		CArtifactAwakening &m_Artifact;		//	Core mini-game
		CDaimonList m_DaimonList;			//	List of available daimons to deploy

		//	Painters

		CArtifactAICorePainter m_AICorePainter;
		CArtifactStatPainter m_StatsPainter[CArtifactStat::statCount];
		CDaimonListPainter m_DaimonListPainter;
		TArray<SDaimonLocus> m_DaimonLoci;	//	Locations to which daimons are deployed
		TArray<SCountermeasureLocus> m_CountermeasureLoci;
		TArray<CArtifactResultPainter *> m_Effects;
		CArtifactMessagePainter m_Messages;

		CDaimonButtonPainter m_DeployBtn;	//	Deploy button

		CHoverDescriptionPainter m_InfoPane;//	Info pane on hover
		SSelection m_InfoPaneSel;			//	What the info pane is showing

		//	UI state

		SSelection m_Hover;					//	What we're currently hovering over
		SSelection m_Clicked;				//	What we've clicked down on (but not yet up)

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
		CDockScreenSubjugate (CDockScreen &DockScreen);

		void OnCompleted (bool bSuccess);
		void OnStarted (void);

	protected:

		//	IDockScreenDisplay

		virtual bool OnGetDefaultBackground (SBackgroundDesc *retDesc) override { retDesc->iType = backgroundNone; return true; }
		virtual EResults OnHandleKeyDown (int iVirtKey) override;
		virtual ALERROR OnInit (SInitCtx &Ctx, const SDisplayOptions &Options, CString *retsError) override;
		virtual ICCItemPtr OnGetProperty (const CString &sProperty) const override;

	private:
		void FireOnCompleted (const CString &sReason);

		CArtifactAwakening m_Artifact;		//	Mini-game data
		CGSubjugateArea *m_pControl;
		DWORD m_dwID;						//	ID of control
		CEventHandler m_Events;
	};

