//	DockScreenSubjugate.h
//
//	Dock Screen subjugateMinigame
//	Copyright (c) 2016 by Kronosaur Productions, LLC. All Rights Reserved.

#pragma once

class CDockScreenSubjugate;

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

class CGSubjugateArea : public AGArea
	{
	public:
		CGSubjugateArea (const CVisualPalette &VI, CDockScreenSubjugate &Controller);
		~CGSubjugateArea (void);

		void AddCountermeasure (CItemType *pItem);
		void AddDaimon (CItemType *pItem);
		inline void SetEgo (int iValue) { m_iEgo = iValue; }
		inline void SetIntelligence (int iValue) { m_iIntelligence = iValue; }
		inline void SetWillpower (int iValue) { m_iWillpower = iValue; }

		//	AGArea virtuals

		virtual bool LButtonDoubleClick (int x, int y) override;
		virtual bool LButtonDown (int x, int y) override;
		virtual void LButtonUp (int x, int y) override;
		virtual void MouseEnter (void) override;
		virtual void MouseLeave (void) override;
		virtual void MouseMove (int x, int y) override;
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
			selectDaimonLoci,				//	A daimon locus (a deployed daimon)
			selectDeployBtn,				//	The Deploy button
			};

		struct SCountermeasureEntry
			{
			CItemType *pCountermeasure;
			};

		struct SCountermeasureLocus
			{
			int iStartAngle;				//	Starting angle (degrees)
			int iArc;						//	Arc in degrees (counter-clockwise)
			int iInnerRadius;				//	Inner radius in pixels
			int iOuterRadius;				//	Outer radius in pixels
			};

		struct SDaimonEntry
			{
			CItemType *pDaimon;
			};

		struct SDaimonLocus
			{
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
			int iIndex;						//	Idex of countermeasure, daimon, etc.
			};

		void ArtifactSubdued (void);
		void DeployDaimon (void);
		void HideInfoPane (void);
		bool HitTest (int x, int y, SSelection &Sel) const;
		bool HitTestCountermeasureLoci (int x, int y, int *retiIndex = NULL) const;
		inline bool IsActive (void) const { return (m_iState == stateStart || m_iState == stateInBattle); }
		void PaintCountermeasureLocus (CG32bitImage &Dest, const SCountermeasureLocus &Locus) const;
		void PaintDaimonLocus (CG32bitImage &Dest, const SDaimonLocus &Locus) const;

		const CVisualPalette &m_VI;
		CDockScreenSubjugate &m_Controller;

		//	Game state

		EStates m_iState;
		int m_iEgo;
		int m_iIntelligence;
		int m_iWillpower;
		CArtifactAICorePainter m_AICorePainter;
		TSortMap<CString, SDaimonEntry> m_DaimonList;	//	List of available daimons to deploy
		TArray<SDaimonLocus> m_DaimonLoci;	//	Locations to which daimons are deployed

		TArray<SCountermeasureEntry> m_CountermeasureList;
		TArray<SCountermeasureLocus> m_CountermeasureLoci;

		//	UI state

		SSelection m_Hover;					//	What we're currently hovering over
		SSelection m_Clicked;				//	What we've clicked down on (but not yet up)
		CHoverDescriptionPainter m_InfoPane;//	Info pane on hover
		SSelection m_InfoPaneSel;			//	What the info pane is showing

		CDaimonButtonPainter m_DeployBtn;	//	Deploy button

		//	Metrics

		int m_xCenter;						//	Center of AI core
		int m_yCenter;

		RECT m_rcHand;						//	List of available daimons to deploy

		//	Visuals

		CG32bitPixel m_rgbCountermeasureBack;
		CG32bitPixel m_rgbDaimonBack;
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
		virtual ALERROR OnInit (SInitCtx &Ctx, const SDisplayOptions &Options, CString *retsError) override;

	private:
		void FireOnCompleted (bool bSuccess);

		CGSubjugateArea *m_pControl;
		DWORD m_dwID;						//	ID of control
		CEventHandler m_Events;
	};

