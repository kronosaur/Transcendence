//	Reanimator.h
//
//	Animation sequencers based on DirectXUtil

#ifndef INCL_REANIMATOR
#define INCL_REANIMATOR

#ifndef INCL_EUCLID
#include "Euclid.h"
#endif

#ifndef INCL_DIRECTXUTIL
#include "DirectXUtil.h
#endif

//	Command class --------------------------------------------------------------

class IAniCommand
	{
	public:
		virtual ~IAniCommand (void) { }

		inline void AniCommand (const CString &sID, const CString &sEvent, const CString &sCmd, DWORD dwData) { OnAniCommand(sID, sEvent, sCmd, dwData); }

	protected:
		virtual void OnAniCommand (const CString &sID, const CString &sEvent, const CString &sCmd, DWORD dwData) { }
	};

struct SAniEvent
	{
	CString sEvent;							//	Event to fire

	IAniCommand *pListener;					//	Event listener
	CString sCmd;
	DWORD dwData;
	};

//	Property classes -----------------------------------------------------------

enum EDurationConstants
	{
	durationInfinite = -1,
	durationUndefined = -2,
	};

struct SAniUpdateCtx
	{
	TArray<SAniEvent> EventsToFire;
	};

class CAniProperty
	{
	public:
		enum Types
			{
			typeNone,
			typeBool,
			typeInteger,
			typeMetric,
			typeColor,
			typeVector,
			typeOpacity,
			typeString,
			typeFont,
			};

		CAniProperty (void);
		CAniProperty (int iValue);
		CAniProperty (Metric rValue);
		CAniProperty (const CVector &vValue);
		~CAniProperty (void);

		CAniProperty &operator= (const CAniProperty &Obj);

		inline bool GetBool (void) const { return (m_iType == typeBool ? (m_Value.dwValue == 1) : false); }
		inline WORD GetColor (void) const { return (m_iType == typeColor ? (WORD)m_Value.dwValue : 0); }
		inline const CG16bitFont *GetFont (void) const { return (m_iType == typeFont ? (const CG16bitFont *)m_Value.pValue : NULL); }
		inline int GetInteger (void) const { return (m_iType == typeInteger ? (int)m_Value.dwValue : 0); }
		inline Metric GetMetric (void) const { return (m_iType == typeMetric ? m_Value.rValue : 0.0); }
		inline DWORD GetOpacity (void) const { return (m_iType == typeOpacity ? m_Value.dwValue : 0); }
		CString GetString (void) const;
		inline Types GetType (void) const { return m_iType; }
		const CVector &GetVector (void) const;
		void Set (Types iType, bool bValue);
		void Set (Types iType, const CG16bitFont *pFont);
		void Set (Types iType, int iValue);
		void Set (Types iType, DWORD dwValue);
		void Set (Types iType, Metric rValue);
		void Set (Types iType, const CString &sValue);
		void Set (Types iType, const CVector &vValue);

	private:
		struct UVector
			{
			Metric x;
			Metric y;
			};

		union UValue
			{
			DWORD dwValue;
			Metric rValue;
			UVector vValue;
			void *pValue;
			};

		void Clear (void);

		Types m_iType;
		UValue m_Value;
	};

class IPropertyAnimator
	{
	public:
		virtual ~IPropertyAnimator (void) { }

		void AddListener (const CString &sEvent, IAniCommand *pListener, const CString &sCmd = NULL_STR, DWORD dwData = 0);
		virtual int GetDuration (void) { return durationUndefined; }
		virtual void OnDoneAnimating (SAniUpdateCtx &Ctx);
		void RemoveListener (IAniCommand *pListener, const CString &sEvent = NULL_STR);
		virtual void SetProperty (int iFrame, CAniProperty &Property) { }

	protected:
		void RaiseEvent (const CString &sID, const CString &sEvent);

	private:
		TArray<SAniEvent> m_Listeners;
	};

class CAniPropertySet
	{
	public:
		~CAniPropertySet (void);

		inline const CAniProperty &operator [] (int iIndex) const { return m_Properties[iIndex].CurrentValue; }

		void AddAnimator (const CString &sProperty, IPropertyAnimator *pAnimator, int iStartFrame = 0, const CString &sID = NULL_STR, bool bStartNow = false);
		void DeleteAnimator (const CString &sID);
		bool FindAnimator (const CString &sID, IPropertyAnimator **retpAnimator);
		inline int FindProperty (const CString &sProperty) const { int iIndex; Find(sProperty, &iIndex); return iIndex; }
		bool GetBool (const CString &sName) const;
		WORD GetColor (const CString &sName) const;
		int GetDuration (void);
		const CG16bitFont *GetFont (const CString &sName) const;
		int GetInteger (const CString &sName) const;
		Metric GetMetric (const CString &sName) const;
		DWORD GetOpacity (const CString &sName) const;
		CString GetString (const CString &sName) const;
		CVector GetVector (const CString &sName) const;
		void GoToFrame (int iFrame);
		void GoToNextFrame (SAniUpdateCtx &Ctx, int iFrame);
		void GoToStart (void);
		void Set (const CString &sName, const CAniProperty &Value, int *retiIndex = NULL);
		void SetBool (const CString &sName, bool bValue, int *retiIndex = NULL);
		void SetColor (const CString &sName, WORD wValue, int *retiIndex = NULL);
		void SetFont (const CString &sName, const CG16bitFont *pFont, int *retiIndex = NULL);
		void SetInteger (const CString &sName, int iValue, int *retiIndex = NULL);
		void SetMetric (const CString &sName, Metric rValue, int *retiIndex = NULL);
		void SetOpacity (const CString &sName, DWORD dwValue, int *retiIndex = NULL);
		void SetString (const CString &sName, const CString &sValue, int *retiIndex = NULL);
		void SetVector (const CString &sName, const CVector &vValue, int *retiIndex = NULL);

	private:
		struct SAnimator
			{
			CString sID;
			IPropertyAnimator *pAnimator;
			int iProperty;
			int iStartFrame;

			int iFrame;
			};

		struct SProperty
			{
			CString sName;
			CAniProperty CurrentValue;
			CAniProperty InitialValue;
			};

		SProperty *Add (const CString &sName, int *retiIndex = NULL);
		SProperty *Find (const CString &sName, int *retiIndex = NULL) const;
		SProperty *FindOrAdd (const CString &sName, int *retiIndex = NULL);

		TArray<SProperty> m_Properties;
		TArray<SAnimator> m_Animators;
	};

//	Paint classes --------------------------------------------------------------

struct SAniPaintCtx
	{
	SAniPaintCtx (CG16bitImage &DestArg, const CXForm &ToDestArg, DWORD dwOpacityToDestArg, int iFrameArg) :
			Dest(DestArg),
			ToDest(ToDestArg),
			dwOpacityToDest(dwOpacityToDestArg),
			iFrame(iFrameArg)
		{ }

	CG16bitImage &Dest;
	const CXForm &ToDest;
	DWORD dwOpacityToDest;
	int iFrame;
	};

struct SSimpleRasterLine
	{
	int y;						//	y position of line
	int x;						//	x position of solid part of line
	int cxLength;				//	length of solid part of line
	BYTE byLeftEdge;			//	opacity of left edge pixel
	BYTE byRightEdge;			//	opacity of right edge pixel
	};

class IAniFillMethod
	{
	public:
		virtual ~IAniFillMethod (void) { }

		virtual void Fill (SAniPaintCtx &Ctx, int x, int y, int cxWidth, int cyHeight) { }
		virtual void Fill (SAniPaintCtx &Ctx, int x, int y, const TArray<SSimpleRasterLine> &Lines) { }
		virtual void Fill (SAniPaintCtx &Ctx, int x, int y, const CG16bitBinaryRegion &Region) { }
		virtual void InitDefaults (CAniPropertySet &Properties) { }
		virtual void InitPaint (SAniPaintCtx &Ctx, int xOrigin, int yOrigin, CAniPropertySet &Properties) { }
	};

class IAniLineMethod
	{
	public:
		virtual ~IAniLineMethod (void) { }

		virtual void Arc (SAniPaintCtx &Ctx, int xCenter, int yCenter, int iRadius, int iStartAngle, int iEndAngle) { }
		virtual void Corner (SAniPaintCtx &Ctx, int xCenter, int yCenter) { }
		virtual void InitDefaults (CAniPropertySet &Properties) { }
		virtual void InitPaint (SAniPaintCtx &Ctx, int xOrigin, int yOrigin, CAniPropertySet &Properties) { }
		virtual void Line (SAniPaintCtx &Ctx, int xFrom, int yFrom, int xTo, int yTo) { }
	};

//	Miscellaneous classes ------------------------------------------------------

enum EScrollTypes
	{
	scrollNone,

	scrollRelative,
	scrollToEnd,
	scrollToHome,
	};

//	The Reanimator -------------------------------------------------------------

class IAnimatron
	{
	public:
		virtual ~IAnimatron (void) { }

		inline void AnimateProperty (const CString &sName, IPropertyAnimator *pAnimator, int iStartFrame = 0, const CString &sID = NULL_STR, bool bStartNow = false) { m_Properties.AddAnimator(sName, pAnimator, iStartFrame, sID, bStartNow); }
		inline bool FindAnimation (const CString &sID, IPropertyAnimator **retpAnimator) { return m_Properties.FindAnimator(sID, retpAnimator); }
		inline void RemoveAnimation (const CString &sID) { m_Properties.DeleteAnimator(sID); }

		inline const CString &GetID (void) { return m_sID; }
		inline bool GetPropertyBool (const CString &sName) const { bool bValue; if (FindDynamicPropertyBool(sName, &bValue)) return bValue; return m_Properties.GetBool(sName); }
		inline WORD GetPropertyColor (const CString &sName) const { WORD wValue; if (FindDynamicPropertyColor(sName, &wValue)) return wValue; return m_Properties.GetColor(sName); }
		inline const CG16bitFont *GetPropertyFont (const CString &sName) const { return m_Properties.GetFont(sName); }
		inline int GetPropertyInteger (const CString &sName) const { int iValue; if (FindDynamicPropertyInteger(sName, &iValue)) return iValue; return m_Properties.GetInteger(sName); }
		inline Metric GetPropertyMetric (const CString &sName) const { Metric rValue; if (FindDynamicPropertyMetric(sName, &rValue)) return rValue; return m_Properties.GetMetric(sName); }
		inline DWORD GetPropertyOpacity (const CString &sName) const { return m_Properties.GetOpacity(sName); }
		inline CString GetPropertyString (const CString &sName) const { CString sValue; if (FindDynamicPropertyString(sName, &sValue)) return sValue; return m_Properties.GetString(sName); }
		inline CVector GetPropertyVector (const CString &sName) const { return m_Properties.GetVector(sName); }

		inline void SetID (const CString &sID) { m_sID = sID; }
		inline void SetProperty (const CString &sName, const CAniProperty &Value) { m_Properties.Set(sName, Value); OnPropertyChanged(sName); }
		inline void SetPropertyBool (const CString &sName, bool bValue) { m_Properties.SetBool(sName, bValue); OnPropertyChanged(sName); }
		inline void SetPropertyColor (const CString &sName, WORD wValue) { m_Properties.SetColor(sName, wValue); OnPropertyChanged(sName); }
		inline void SetPropertyFont (const CString &sName, const CG16bitFont *pFont) { m_Properties.SetFont(sName, pFont); OnPropertyChanged(sName); }
		inline void SetPropertyInteger (const CString &sName, int iValue) { m_Properties.SetInteger(sName, iValue); OnPropertyChanged(sName); }
		inline void SetPropertyMetric (const CString &sName, Metric rValue) { m_Properties.SetMetric(sName, rValue); OnPropertyChanged(sName); }
		inline void SetPropertyOpacity (const CString &sName, DWORD dwValue) { m_Properties.SetOpacity(sName, dwValue); OnPropertyChanged(sName); }
		inline void SetPropertyString (const CString &sName, const CString &sValue) { m_Properties.SetString(sName, sValue); OnPropertyChanged(sName); }
		inline void SetPropertyVector (const CString &sName, const CVector &vValue) { m_Properties.SetVector(sName, vValue); OnPropertyChanged(sName); }

		virtual bool DeleteElement (const CString &sID) { return false; }
		virtual bool FindElement (IAnimatron *pAni) { return (pAni == this); }
		virtual bool FindElement (const CString &sID, IAnimatron **retpAni = NULL) { if (strEquals(sID, m_sID)) { if (retpAni) *retpAni = this; return true; } else return false; }
		virtual void GetContentRect (RECT *retrcRect) { GetSpacingRect(retrcRect); }
		virtual int GetDuration (void) { return m_Properties.GetDuration(); }
		virtual void GetFocusElements (TArray<IAnimatron *> *retList) { }
		inline int GetSpacingHeight (void) { RECT rcRect; GetSpacingRect(&rcRect); return RectHeight(rcRect); }
		virtual void GetSpacingRect (RECT *retrcRect) { retrcRect->left = 0; retrcRect->top = 0; retrcRect->right = 0; retrcRect->bottom = 0; }
		virtual void GoToFrame (int iFrame) { m_Properties.GoToFrame(iFrame); }
		virtual void GoToNextFrame (SAniUpdateCtx &Ctx, int iFrame) { m_Properties.GoToNextFrame(Ctx, iFrame); }
		virtual void GoToStart (void) { m_Properties.GoToStart(); }
		virtual bool HandleChar (char chChar, DWORD dwKeyData) { return false; }
		virtual bool HandleKeyDown (int iVirtKey, DWORD dwKeyData) { return false; }
		virtual bool HandleKeyUp (int iVirtKey, DWORD dwKeyData) { return false; }
		virtual void HandleLButtonDblClick (int x, int y, DWORD dwFlags, bool *retbCapture, bool *retbFocus) { *retbCapture = false; *retbFocus = false; }
		virtual void HandleLButtonDown (int x, int y, DWORD dwFlags, bool *retbCapture, bool *retbFocus) { *retbCapture = false; *retbFocus = false; }
		virtual void HandleLButtonUp (int x, int y, DWORD dwFlags) { }
		virtual void HandleMouseEnter (void) { }
		virtual void HandleMouseLeave (void) { }
		virtual void HandleMouseMove (int x, int y, DWORD dwFlags) { }
		virtual bool HandleMouseWheel (int iDelta, int x, int y, DWORD dwFlags) { return false; }
		virtual IAnimatron *HitTest (const CXForm &ToDest, int x, int y) { return NULL; }
		virtual void KillFocus (void) { }
		virtual void Paint (SAniPaintCtx &Ctx) { }
		virtual void SetFocus (void) { }

		virtual IAniFillMethod *GetFillMethod (void) { return NULL; }
		virtual IAniLineMethod *GetLineMethod (void) { return NULL; }
		virtual void SetFillMethod (IAniFillMethod *pFill) { }
		virtual void SetLineMethod (IAniLineMethod *pLine) { }

		virtual void AddListener (const CString &sEvent, IAniCommand *pListener, const CString &sCmd = NULL_STR, DWORD dwData = 0) { }
		virtual void RemoveListener (IAniCommand *pListener, const CString &sEvent = NULL_STR) { }

		virtual IAnimatron *GetStyle (const CString &sComponent) const { return NULL; }
		virtual void SetStyle (const CString &sComponent, IAnimatron *pImpl) { }

		//	Helper functions
		void AnimateLinearFade (int iDuration, int iFadeIn, int iFadeOut, DWORD dwMaxOpacity = 255);
		void AnimateLinearRotation (int iStartAngle, Metric rRate, int iDuration);
		void AnimatePropertyLinear (const CString &sID, const CString &sName, const CAniProperty &Start, const CAniProperty &End, int iDuration, bool bStartNow = false);
		void Scroll (EScrollTypes iScroll, int iScrollDist = 0);

	protected:
		virtual bool FindDynamicPropertyBool (const CString &sName, bool *retbValue) const { return false; }
		virtual bool FindDynamicPropertyColor (const CString &sName, WORD *retwValue) const { return false; }
		virtual bool FindDynamicPropertyInteger (const CString &sName, int *retiValue) const { return false; }
		virtual bool FindDynamicPropertyMetric (const CString &sName, Metric *retrValue) const { return false; }
		virtual bool FindDynamicPropertyString (const CString &sName, CString *retsValue) const { return false; }
		virtual void OnPropertyChanged (const CString &sName) { }

		//	Helper function
		void ScrollTo (int iOriginalPos, int iPos);

		CString m_sID;
		CAniPropertySet m_Properties;
	};

class CReanimator
	{
	public:
		enum Flags
			{
			SPR_FLAG_DELETE_WHEN_DONE =			0x00000001,
			};

		CReanimator (void);
		~CReanimator (void);

		DWORD AddPerformance (IAnimatron *pAni, const CString &sID = NULL_STR);
		void DeleteElement (const CString &sID);
		static const CG16bitFont &GetDefaultFont (void);
		inline bool GetPropertyBool (const CString &sID, const CString &sProp) { IAnimatron *pAni = GetElement(sID); return (pAni ? pAni->GetPropertyBool(sProp) : false); }
		inline WORD GetPropertyColor (const CString &sID, const CString &sProp) { IAnimatron *pAni = GetElement(sID); return (pAni ? pAni->GetPropertyColor(sProp) : 0); }
		inline const CG16bitFont *GetPropertyFont (const CString &sID, const CString &sProp) { IAnimatron *pAni = GetElement(sID); return (pAni ? pAni->GetPropertyFont(sProp) : 0); }
		inline int GetPropertyInteger (const CString &sID, const CString &sProp) { IAnimatron *pAni = GetElement(sID); return (pAni ? pAni->GetPropertyInteger(sProp) : 0); }
		inline Metric GetPropertyMetric (const CString &sID, const CString &sProp) { IAnimatron *pAni = GetElement(sID); return (pAni ? pAni->GetPropertyMetric(sProp) : 0.0); }
		inline DWORD GetPropertyOpacity (const CString &sID, const CString &sProp) { IAnimatron *pAni = GetElement(sID); return (pAni ? pAni->GetPropertyOpacity(sProp) : 0); }
		inline CString GetPropertyString (const CString &sID, const CString &sProp) { IAnimatron *pAni = GetElement(sID); return (pAni ? pAni->GetPropertyString(sProp) : NULL_STR); }
		inline CVector GetPropertyVector (const CString &sID, const CString &sProp) { IAnimatron *pAni = GetElement(sID); return (pAni ? pAni->GetPropertyVector(sProp) : NullVector); }
		void DeletePerformance (DWORD dwID);
		IAnimatron *GetElement (const CString &sID);
		inline IAnimatron *GetPerformance (DWORD dwID) { SPerformance *pPerf = Find(dwID); if (pPerf) return pPerf->pAni; else return NULL; }
		inline IAnimatron *GetPerformance (const CString &sID, int *retiFrame = NULL) { SPerformance *pPerf = Find(sID); if (pPerf) { if (retiFrame) *retiFrame = pPerf->iFrame; return pPerf->pAni; } else return NULL; }
		void FFBurst (int iSpeed, int iCounter = -1) { m_iPlaySpeed = iSpeed; m_iFastPlayCounter = iCounter; }
		bool IsPaused (void) { return m_iPlaySpeed == 0; }
		bool IsPerformanceRunning (const CString &sID);
		bool IsRunning (void);
		void Pause (void) { m_iPlaySpeed = 0; m_iFastPlayCounter = -1; }
		void Resume (void) { m_iPlaySpeed = 1; m_iFastPlayCounter = -1; }
		void SetInputFocus (IAnimatron *pFocus);
		inline void SetPropertyBool (const CString &sID, const CString &sProp, bool bValue) { IAnimatron *pAni = GetElement(sID); if (pAni) pAni->SetPropertyBool(sProp, bValue); }
		inline void SetPropertyColor (const CString &sID, const CString &sProp, WORD wValue) { IAnimatron *pAni = GetElement(sID); if (pAni) pAni->SetPropertyColor(sProp, wValue); }
		inline void SetPropertyFont (const CString &sID, const CString &sProp, const CG16bitFont *pFont) { IAnimatron *pAni = GetElement(sID); if (pAni) pAni->SetPropertyFont(sProp, pFont); }
		inline void SetPropertyInteger (const CString &sID, const CString &sProp, int iValue) { IAnimatron *pAni = GetElement(sID); if (pAni) pAni->SetPropertyInteger(sProp, iValue); }
		inline void SetPropertyMetric (const CString &sID, const CString &sProp, Metric rValue) { IAnimatron *pAni = GetElement(sID); if (pAni) pAni->SetPropertyMetric(sProp, rValue); }
		inline void SetPropertyOpacity (const CString &sID, const CString &sProp, DWORD dwValue) { IAnimatron *pAni = GetElement(sID); if (pAni) pAni->SetPropertyOpacity(sProp, dwValue); }
		inline void SetPropertyString (const CString &sID, const CString &sProp, const CString &sValue) { IAnimatron *pAni = GetElement(sID); if (pAni) pAni->SetPropertyString(sProp, sValue); }
		inline void SetPropertyVector (const CString &sID, const CString &sProp, const CVector &vValue) { IAnimatron *pAni = GetElement(sID); if (pAni) pAni->SetPropertyVector(sProp, vValue); }
		inline void StartPerformance (DWORD dwID, DWORD dwFlags = 0, int *retiDuration = NULL) { StartPerformance(Find(dwID), dwFlags, retiDuration); }
		inline void StartPerformance (const CString &sID, DWORD dwFlags = 0, int *retiDuration = NULL) { StartPerformance(Find(sID), dwFlags, retiDuration); }
		void StopAll (void);
		void StopPerformance (const CString &sID);

		//	Host calls these
		bool HandleChar (char chChar, DWORD dwKeyData);
		bool HandleKeyDown (int iVirtKey, DWORD dwKeyData);
		bool HandleKeyUp (int iVirtKey, DWORD dwKeyData);
		bool HandleLButtonDblClick (int x, int y, DWORD dwFlags, bool *retbCapture);
		bool HandleLButtonDown (int x, int y, DWORD dwFlags, bool *retbCapture);
		bool HandleLButtonUp (int x, int y, DWORD dwFlags);
		bool HandleMouseMove (int x, int y, DWORD dwFlags);
		bool HandleMouseWheel (int iDelta, int x, int y, DWORD dwFlags);
		bool PaintFrame (CG16bitImage &Dest);

	private:
		struct SPerformance
			{
			DWORD dwID;
			CString sID;
			IAnimatron *pAni;

			int iFrame;						//	Current frame (or -1 if not running)
			int iDuration;					//	Total number of frames

			bool bDeleteWhenDone;
			};

		void DeleteAnimatron (IAnimatron *pAni);
		SPerformance *Find (DWORD dwID);
		SPerformance *Find (const CString &sID);
		bool GetFocusElements (TArray<IAnimatron *> *retList, int *retiCurrent);
		IAnimatron *HitTest (int x, int y);
		void SetInputFocusNext (void);
		void SetInputFocusPrev (void);
		void StartPerformance (SPerformance *pPerf, DWORD dwFlags = 0, int *retiDuration = NULL);

		DWORD m_dwNextID;
		TArray<SPerformance> m_Library;

		int m_iPlaySpeed;
		int m_iFastPlayCounter;
		CXForm m_ToDest;
		IAnimatron *m_pInputFocus;
		IAnimatron *m_pMouseCapture;
		IAnimatron *m_pHover;

		static CG16bitFont *m_pDefaultFont;
	};

//	Sequencer ------------------------------------------------------------------

class CAniSequencer : public IAnimatron
	{
	public:
		CAniSequencer (void);
		virtual ~CAniSequencer (void);

		static void Create (const CVector &vPos,
							CAniSequencer **retpAni);

		void AddTrack (IAnimatron *pAni, int iStartFrame);

		//	IAnimatron virtuals
		virtual bool DeleteElement (const CString &sID);
		virtual bool FindElement (IAnimatron *pAni);
		virtual bool FindElement (const CString &sID, IAnimatron **retpAni);
		virtual int GetDuration (void);
		virtual void GetFocusElements (TArray<IAnimatron *> *retList);
		virtual void GetSpacingRect (RECT *retrcRect);
		virtual void GoToFrame (int iFrame);
		virtual void GoToNextFrame (SAniUpdateCtx &Ctx, int iFrame);
		virtual void GoToStart (void);
		virtual IAnimatron *HitTest (const CXForm &ToDest, int x, int y);
		virtual void Paint (SAniPaintCtx &Ctx);

	private:
		struct STrack
			{
			IAnimatron *pAni;
			int iStartFrame;				//	When to start playing

			int iFrame;						//	Current local frame (or -1 if not running)
			int iDuration;					//	Total number of frames
			};

		TArray<STrack> m_Timeline;
	};

//	Scrollers ------------------------------------------------------------------

class CAniVScroller : public IAnimatron
	{
	public:
		CAniVScroller (void);
		virtual ~CAniVScroller (void);

		void AddLine (IAnimatron *pAni);
		void AddTextLine (const CString &sText, const CG16bitFont *pFont, WORD wColor, DWORD dwFlags, int cyExtra = 0);
		void AnimateLinearScroll (Metric rRate);
		inline Metric GetHeight (void) const { return m_cyEnd; }

		//	IAnimatron virtuals
		virtual bool DeleteElement (const CString &sID);
		virtual bool FindElement (IAnimatron *pAni);
		virtual bool FindElement (const CString &sID, IAnimatron **retpAni);
		virtual int GetDuration (void);
		virtual void GetFocusElements (TArray<IAnimatron *> *retList);
		virtual void GetSpacingRect (RECT *retrcRect);
		virtual void GoToFrame (int iFrame);
		virtual void GoToNextFrame (SAniUpdateCtx &Ctx, int iFrame);
		virtual void GoToStart (void);
		virtual IAnimatron *HitTest (const CXForm &ToDest, int x, int y);
		virtual void Paint (SAniPaintCtx &Ctx);

	protected:
		//	IAnimatron virtuals
		virtual bool FindDynamicPropertyMetric (const CString &sName, Metric *retrValue) const;

	private:
		struct SLine
			{
			IAnimatron *pAni;

			Metric cyHeight;				//	Height of line in pixels
			};

		Metric CalcTotalHeight (void);

		TArray<SLine> m_List;
		Metric m_cyEnd;						//	Current end (and height of total list)
	};

//	Animatron Primitives -------------------------------------------------------

class CAniRichText : public IAnimatron
	{
	public:
		CAniRichText (const IFontTable &FontTable);

		//	IAnimatron virtuals
		virtual void GetSpacingRect (RECT *retrcRect);
		virtual void Paint (SAniPaintCtx &Ctx);

	protected:
		//	IAnimatron virtuals
		virtual void OnPropertyChanged (const CString &sName);

	private:
		void Format (int cxWidth, int cyHeight);

		const IFontTable &m_FontTable;
		CTextBlock m_Text;
		bool m_bInvalid;
	};

class CAniText : public IAnimatron
	{
	public:
		CAniText (void);

		static void Create (const CString &sText,
							const CVector &vPos,
							const CG16bitFont *pFont,
							DWORD dwFontFlags,
							WORD wColor,
							IAnimatron **retpAni);

		inline void SetFontFlags (DWORD dwFlags) { m_dwFontFlags = dwFlags; }

		//	IAnimatron virtuals
		virtual void GetSpacingRect (RECT *retrcRect);
		virtual void Paint (SAniPaintCtx &Ctx);

	private:
		DWORD m_dwFontFlags;
	};

//	Animatron Shapes -----------------------------------------------------------

class CAniShape : public IAnimatron
	{
	public:
		CAniShape (void);
		~CAniShape (void);

		virtual IAniFillMethod *GetFillMethod (void);
		virtual IAniLineMethod *GetLineMethod (void);
		virtual void SetFillMethod (IAniFillMethod *pFill);
		virtual void SetLineMethod (IAniLineMethod *pLine);

	private:
		IAniFillMethod *m_pFill;
		IAniLineMethod *m_pLine;
	};

class CAniPolygon : public CAniShape
	{
	public:
		CAniPolygon (TArray<CVector> &Points);

		//	IAnimatron virtuals
		virtual void GetContentRect (RECT *retrcRect);
		virtual void GetSpacingRect (RECT *retrcRect);
		virtual void Paint (SAniPaintCtx &Ctx);

	private:
		void Rasterize (const CVector &vScale, int iRotation);

		TArray<CVector> m_Points;

		CVector m_vScale;
		int m_iRotation;
		CG16bitBinaryRegion m_Region;
	};

class CAniRect : public CAniShape
	{
	public:
		CAniRect (void);

		static void Create (const CVector &vPos,
							const CVector &vSize,
							WORD wColor,
							DWORD dwOpacity,
							IAnimatron **retpAni);

		//	IAnimatron virtuals
		virtual void GetContentRect (RECT *retrcRect);
		virtual void GetSpacingRect (RECT *retrcRect);
		virtual void Paint (SAniPaintCtx &Ctx);
	};

class CAniRoundedRect : public CAniShape
	{
	public:
		CAniRoundedRect (void);
		~CAniRoundedRect (void);

		static void Create (const CVector &vPos,
							const CVector &vSize,
							WORD wColor,
							DWORD dwOpacity,
							IAnimatron **retpAni);

		//	IAnimatron virtuals
		virtual void GetContentRect (RECT *retrcRect);
		virtual void GetSpacingRect (RECT *retrcRect);
		virtual void Paint (SAniPaintCtx &Ctx);

	private:
		enum Corners
			{
			upperLeftCorner		= 0,
			upperRightCorner	= 1,
			lowerLeftCorner		= 2,
			lowerRightCorner	= 3,

			cornerCount			= 4,
			};

		struct SCornerRaster
			{
			int iRadius;
			TArray<SSimpleRasterLine> Lines;
			};

		void InitCornerRaster (Corners iCorner, int iRadius);

		SCornerRaster m_Corner[cornerCount];
	};

//	UI Controls ----------------------------------------------------------------

class CAniControl : public IAnimatron
	{
	public:
		virtual ~CAniControl (void);

		virtual void AddListener (const CString &sEvent, IAniCommand *pListener, const CString &sCmd = NULL_STR, DWORD dwData = 0);
		virtual IAnimatron *GetStyle (const CString &sComponent) const;
		virtual void RemoveListener (IAniCommand *pListener, const CString &sEvent = NULL_STR);
		virtual void SetStyle (const CString &sComponent, IAnimatron *pImpl);

	protected:
		//	CAniControl virtuals
		virtual int MapStyleName (const CString &sComponent) const { return -1; }

		//	Helpers
		IAnimatron *GetStyle (int iIndex) const { return (iIndex >= 0 && iIndex < m_Styles.GetCount() ? m_Styles[iIndex].pAni : NULL); }
		void RaiseEvent (const CString &sEvent);
		void SetStyle (int iIndex, IAnimatron *pImpl);

	private:
		struct SListener
			{
			CString sEvent;

			IAniCommand *pListener;
			CString sCmd;
			DWORD dwData;
			};

		struct SStyleDef
			{
			SStyleDef (void) : pAni(NULL) { }

			IAnimatron *pAni;
			};

		TArray<SListener> m_Listeners;
		TArray<SStyleDef> m_Styles;
	};

class CAniButton : public CAniControl
	{
	public:
		enum EButtonTypes
			{
			typeNormal,
			typeCheckbox,
			typeLink,
			typeRadioButton,
			};

		CAniButton (EButtonTypes iType = typeNormal);
		~CAniButton (void);

		//	IAnimatron virtuals
		virtual void GetSpacingRect (RECT *retrcRect);
		virtual void HandleLButtonDblClick (int x, int y, DWORD dwFlags, bool *retbCapture, bool *retbFocus);
		virtual void HandleLButtonDown (int x, int y, DWORD dwFlags, bool *retbCapture, bool *retbFocus);
		virtual void HandleLButtonUp (int x, int y, DWORD dwFlags);
		virtual void HandleMouseEnter (void) { m_bHover = true; }
		virtual void HandleMouseLeave (void) { m_bHover = false; }
		virtual IAnimatron *HitTest (const CXForm &ToDest, int x, int y);
		virtual void Paint (SAniPaintCtx &Ctx);

	protected:
		//	CAniControl virtuals
		virtual int MapStyleName (const CString &sComponent) const;

	private:
		enum EStyleParts
			{
			styleDown =			0,
			styleHover =		1,
			styleNormal =		2,
			styleText =			3,
			styleDisabled =		4,
			styleCheck =		5,
			styleImage =		6,

			styleCount =		7,
			};

		EButtonTypes m_iType;
		bool m_bDown;
		bool m_bHover;
	};

class CAniListBox : public CAniControl
	{
	public:
		CAniListBox (void);
		~CAniListBox (void);

		void AddEntry (const CString &sID, IAnimatron *pEntry);

		//	IAnimatron virtuals
		virtual bool DeleteElement (const CString &sID);
		virtual bool FindElement (IAnimatron *pAni) { if (pAni == this) return true; return m_pScroller->FindElement(pAni); }
		virtual bool FindElement (const CString &sID, IAnimatron **retpAni) { if (strEquals(sID, m_sID)) { if (retpAni) *retpAni = this; return true; } return m_pScroller->FindElement(sID, retpAni); }
		virtual int GetDuration (void) { return m_pScroller->GetDuration(); }
		virtual void GetFocusElements (TArray<IAnimatron *> *retList);
		virtual void GetSpacingRect (RECT *retrcRect) { return m_pScroller->GetSpacingRect(retrcRect); }
		virtual void GoToFrame (int iFrame) { return m_pScroller->GoToFrame(iFrame); }
		virtual void GoToNextFrame (SAniUpdateCtx &Ctx, int iFrame) { return m_pScroller->GoToNextFrame(Ctx, iFrame); }
		virtual void GoToStart (void) { return m_pScroller->GoToStart(); }
		virtual bool HandleChar (char chChar, DWORD dwKeyData);
		virtual bool HandleKeyDown (int iVirtKey, DWORD dwKeyData);
		virtual void HandleLButtonDblClick (int x, int y, DWORD dwFlags, bool *retbCapture, bool *retbFocus);
		virtual void HandleLButtonDown (int x, int y, DWORD dwFlags, bool *retbCapture, bool *retbFocus);
		virtual void HandleLButtonUp (int x, int y, DWORD dwFlags);
		virtual bool HandleMouseWheel (int iDelta, int x, int y, DWORD dwFlags);
		virtual IAnimatron *HitTest (const CXForm &ToDest, int x, int y);
		virtual void KillFocus (void) { m_bFocus = false; }
		virtual void Paint (SAniPaintCtx &Ctx);
		virtual void SetFocus (void) { m_bFocus = true; }

	protected:
		//	IAnimatron virtuals
		virtual bool FindDynamicPropertyInteger (const CString &sName, int *retiValue) const;
		virtual bool FindDynamicPropertyMetric (const CString &sName, Metric *retrValue) const;
		virtual bool FindDynamicPropertyString (const CString &sName, CString *retsValue) const;
		virtual void OnPropertyChanged (const CString &sName);

		//	CAniControl virtuals
		virtual int MapStyleName (const CString &sComponent) const;

	private:
		enum EStyleParts
			{
			styleSelection =		0,
			styleSelectionFocus =	1,
			};

		struct SEntry
			{
			IAnimatron *pAni;
			RECT rcRect;
			};

		int GetRowAtPos (int x, int y);
		bool GetRowRect (int iRow, RECT *retrcRect);
		void Select (int iEntry);

		CAniVScroller *m_pScroller;
		TArray<SEntry> m_Entries;
		bool m_bFocus;
	};

class CAniTextInput : public CAniControl
	{
	public:
		enum EOptions
			{
			OPTION_PASSWORD = 0x000000001,
			};

		CAniTextInput (void);
		~CAniTextInput (void);

		static void Create (const RECT &rcRect,
							const CG16bitFont *pFont,
							DWORD dwOptions,
							IAnimatron **retpAni);

		static int GetDefaultHeight (const CG16bitFont &Font);

		//	IAnimatron virtuals
		virtual void GetFocusElements (TArray<IAnimatron *> *retList);
		virtual void GetSpacingRect (RECT *retrcRect);
		virtual bool HandleChar (char chChar, DWORD dwKeyData);
		virtual void HandleLButtonDblClick (int x, int y, DWORD dwFlags, bool *retbCapture, bool *retbFocus);
		virtual void HandleLButtonDown (int x, int y, DWORD dwFlags, bool *retbCapture, bool *retbFocus);
		virtual IAnimatron *HitTest (const CXForm &ToDest, int x, int y);
		virtual void KillFocus (void) { m_bFocus = false; }
		virtual void Paint (SAniPaintCtx &Ctx);
		virtual void SetFocus (void) { m_bFocus = true; }

	protected:
		//	CAniControl virtuals
		virtual int MapStyleName (const CString &sComponent) const;

	private:
		enum EStyleParts
			{
			styleFrame =			0,
			styleFrameFocus =		1,
			styleFrameDisabled =	2,

			styleCount =			3,
			};

		bool m_bFocus;
		bool m_bPassword;
	};

//	Property Animators ---------------------------------------------------------

class CLinearAnimator : public IPropertyAnimator
	{
	public:
		CLinearAnimator (void);

		void SetParams (const CAniProperty &Start, const CAniProperty &End, int iAnimateTime, int iDuration = durationInfinite);

		//	IPropertyAnimator virtuals
		virtual int GetDuration (void) { return m_iDuration; }
		virtual void SetProperty (int iFrame, CAniProperty &Property);

	private:
		CAniProperty m_Start;
		CAniProperty m_End;
		int m_iAnimateTime;
		int m_iDuration;
	};

class CLinearFade : public IPropertyAnimator
	{
	public:
		CLinearFade (void) :
				m_iDuration(durationInfinite),
				m_iFadeIn(0),
				m_iFadeOut(0)
			{ }

		void SetParams (int iDuration, int iFadeIn, int iFadeOut, DWORD dwMaxOpacity = 255);

		//	IPropertyAnimator virtuals
		virtual int GetDuration (void) { return m_iDuration; }
		virtual void SetProperty (int iFrame, CAniProperty &Property);

	private:
		int m_iDuration;
		int m_iFadeIn;
		int m_iFadeOut;
		DWORD m_dwMaxOpacity;
	};

class CLinearMetric : public IPropertyAnimator
	{
	public:
		CLinearMetric (void) :
				m_rStart(0.0),
				m_rEnd(100.0),
				m_rIncrement(1.0)
			{ }

		void SetParams (Metric rStart, Metric rEnd, Metric rIncrement);

		//	IPropertyAnimator virtuals
		virtual int GetDuration (void) { return m_iDuration; }
		virtual void SetProperty (int iFrame, CAniProperty &Property);

	private:
		Metric m_rStart;
		Metric m_rEnd;
		Metric m_rIncrement;
		int m_iDuration;
	};

class CLinearRotation : public IPropertyAnimator
	{
	public:
		CLinearRotation (void) :
				m_rStartAngle(0.0),
				m_rRate(1.0),
				m_iAnimateTime(durationInfinite),
				m_iDuration(durationInfinite)
			{ }

		void SetParams (Metric rStartAngle, Metric rRate, int iAnimateTime, int iDuration = durationInfinite);

		//	IPropertyAnimator virtuals
		virtual int GetDuration (void) { return m_iDuration; }
		virtual void SetProperty (int iFrame, CAniProperty &Property);

	private:
		Metric m_rStartAngle;			//	Starting angle (degrees)
		Metric m_rRate;					//	Rotation rate (degrees per frame)
		int m_iAnimateTime;				//	Continue rotation for this many frames
		int m_iDuration;				//	Total duration
	};

class CListScroller : public IPropertyAnimator
	{
	public:
		CListScroller (void) :
				m_rLastPos(0.0)
			{ }

		void AddScrollPos (Metric rPos, Metric rAnchor = 0.0);
		inline int GetCount (void) { return m_ScrollPos.GetCount(); }
		int GetCurScrollPos (void);
		inline Metric GetScrollPos (int iPos, Metric *retrAnchor = NULL) { if (retrAnchor) *retrAnchor = m_ScrollPos[iPos].rLinePos; return m_ScrollPos[iPos].rScrollPos; }
		void SetAtPos (int iPos);
		void SetScrollToPos (int iPos, Metric rSpeed = 16.0);

		//	IPropertyAnimator virtuals
		virtual int GetDuration (void) { return m_iDuration; }
		virtual void SetProperty (int iFrame, CAniProperty &Property);

	private:
		struct SEntry
			{
			Metric rScrollPos;
			Metric rLinePos;
			};

		TArray<SEntry> m_ScrollPos;
		Metric m_rStart;
		Metric m_rEnd;
		Metric m_rIncrement;

		Metric m_rLastPos;
		int m_iDuration;
	};

//	Fill Methods ---------------------------------------------------------------

class CAniNullFill : public IAniFillMethod
	{
	};

class CAniImageFill : public IAniFillMethod
	{
	public:
		CAniImageFill (const CG16bitImage *pImage, bool bFreeImage);
		virtual ~CAniImageFill (void);

		//	IAniFillMethod
		virtual void Fill (SAniPaintCtx &Ctx, int x, int y, int cxWidth, int cyHeight);
		virtual void Fill (SAniPaintCtx &Ctx, int x, int y, const TArray<SSimpleRasterLine> &Lines);
		virtual void Fill (SAniPaintCtx &Ctx, int x, int y, const CG16bitBinaryRegion &Region);
		virtual void InitDefaults (CAniPropertySet &Properties);
		virtual void InitPaint (SAniPaintCtx &Ctx, int xOrigin, int yOrigin, CAniPropertySet &Properties);

	private:
		void CalcTile (int x, int y, int *retxTile, int *retyTile);

		const CG16bitImage *m_pImage;
		bool m_bFreeImage;

		DWORD m_dwOpacity;
		int m_xOrigin;
		int m_yOrigin;
	};

class CAniSolidFill : public IAniFillMethod
	{
	public:
		//	IAniFillMethod
		virtual void Fill (SAniPaintCtx &Ctx, int x, int y, int cxWidth, int cyHeight);
		virtual void Fill (SAniPaintCtx &Ctx, int x, int y, const TArray<SSimpleRasterLine> &Lines);
		virtual void Fill (SAniPaintCtx &Ctx, int x, int y, const CG16bitBinaryRegion &Region);
		virtual void InitDefaults (CAniPropertySet &Properties);
		virtual void InitPaint (SAniPaintCtx &Ctx, int xOrigin, int yOrigin, CAniPropertySet &Properties);

	private:
		WORD m_wColor;
		DWORD m_dwOpacity;
	};

//	Line Methods ---------------------------------------------------------------

class CAniNullLine : public IAniLineMethod
	{
	};

class CAniSolidLine : public IAniLineMethod
	{
	public:
		//	IAniLineMethod
		virtual void Arc (SAniPaintCtx &Ctx, int xCenter, int yCenter, int iRadius, int iStartAngle, int iEndAngle);
		virtual void Corner (SAniPaintCtx &Ctx, int xCenter, int yCenter);
		virtual void InitDefaults (CAniPropertySet &Properties);
		virtual void InitPaint (SAniPaintCtx &Ctx, int xOrigin, int yOrigin, CAniPropertySet &Properties);
		virtual void Line (SAniPaintCtx &Ctx, int xFrom, int yFrom, int xTo, int yTo);

	private:
		WORD m_wColor;
		int m_iWidth;
	};

#endif