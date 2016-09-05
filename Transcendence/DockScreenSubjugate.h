//	DockScreenSubjugate.h
//
//	Dock Screen subjugateMinigame
//	Copyright (c) 2016 by Kronosaur Productions, LLC. All Rights Reserved.

#pragma once

class CArtifactAICorePainter
	{
	public:
		CArtifactAICorePainter (void);

		void Paint (CG32bitImage &Dest, int x, int y);

	private:
	};

class CGSubjugateArea : public AGArea
	{
	public:
		CGSubjugateArea (const CVisualPalette &VI);
		~CGSubjugateArea (void);

		//	AGArea virtuals

		virtual bool LButtonDown (int x, int y) override;
		virtual void MouseEnter (void) override;
		virtual void MouseLeave (void) override;
		virtual void MouseMove (int x, int y) override;
		virtual void Paint (CG32bitImage &Dest, const RECT &rcRect) override;
		virtual void Update (void) override;

	protected:
		virtual void OnSetRect (void) override;

	private:
		enum ESelectionTypes
			{
			selectNone,
			
			selectCountermeasureLoci,
			selectDaimonLoci,
			};

		struct SCountermeasureLocus
			{
			int iStartAngle;				//	Starting angle (degrees)
			int iArc;						//	Arc in degrees (counter-clockwise)
			int iInnerRadius;				//	Inner radius in pixels
			int iOuterRadius;				//	Outer radius in pixels
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

		void HideInfoPane (void);
		bool HitTest (int x, int y, SSelection &Sel) const;
		bool HitTestCountermeasureLoci (int x, int y, int *retiIndex = NULL) const;
		void PaintCountermeasureLocus (CG32bitImage &Dest, const SCountermeasureLocus &Locus) const;
		void PaintDaimonLocus (CG32bitImage &Dest, const SDaimonLocus &Locus) const;

		const CVisualPalette &m_VI;

		//	Game state

		CArtifactAICorePainter m_AICorePainter;
		TArray<SCountermeasureLocus> m_CountermeasureLoci;
		TArray<SDaimonLocus> m_DaimonLoci;

		//	UI state

		SSelection m_Hover;					//	What we're currently hovering over
		CHoverDescriptionPainter m_InfoPane;//	Info pane on hover
		SSelection m_InfoPaneSel;			//	What the info pane is showing

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
	protected:
		//	IDockScreenDisplay

		virtual bool OnGetDefaultBackground (SBackgroundDesc *retDesc) override { retDesc->iType = backgroundNone; return true; }
		virtual ALERROR OnInit (SInitCtx &Ctx, const SDisplayOptions &Options, CString *retsError) override;

	private:
		CGSubjugateArea *m_pControl;
		DWORD m_dwID;						//	ID of control
	};

