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

		virtual bool LButtonDown (int x, int y);
		virtual void Paint (CG32bitImage &Dest, const RECT &rcRect);
		virtual void Update (void);

	protected:
		virtual void OnSetRect (void) override;

	private:
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

		void PaintLoci (CG32bitImage &Dest, const RECT &rcRect) const;

		const CVisualPalette &m_VI;

		//	Game state

		TArray<SCountermeasureLocus> m_CountermeasureLoci;
		TArray<SDaimonLocus> m_DaimonLoci;

		//	Metrics

		int m_xCenter;						//	Center of AI core
		int m_yCenter;

		RECT m_rcHand;						//	List of available daimons to deploy

		//	Visuals

		CG32bitPixel m_rgbCountermeasureBack;
		CG32bitPixel m_rgbDaimonBack;

		//	Paint helpers

		CArtifactAICorePainter m_AICorePainter;
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

