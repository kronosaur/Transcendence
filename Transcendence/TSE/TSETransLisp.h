//	TSETransLisp.h
//
//	Transcendence Lisp Utilities

#ifndef INCL_TSE_TRANS_LISP
#define INCL_TSE_TRANS_LISP

//	CodeChain context

enum ECodeChainEvents
	{
	eventNone =							0,
	eventOnAIUpdate =					1,
	eventOnUpdate =						2,
	eventGetName =						3,
	eventGetTradePrice =				4,
	eventDoEvent =						5,
	eventObjFireEvent =					6,
	eventOnGlobalTypesInit =			7,
	eventGetGlobalPlayerPriceAdj =		8,
	eventGetDescription =				9,
	eventInitDockScreenList =			10,		//	Code inside <ListOptions> to (e.g.) set list filter
	};

class CCodeChainCtx
	{
	public:
		CCodeChainCtx (void);
		~CCodeChainCtx (void);

		inline ICCItem *CreateNil (void) { return m_CC.CreateNil(); }
		void DefineDamageEffects (const CString &sVar, SDamageCtx &Ctx);
		inline void DefineInteger (const CString &sVar, int iValue) { m_CC.DefineGlobalInteger(sVar, iValue); }
		void DefineItem (const CItem &Item);
		void DefineItem (const CString &sVar, const CItem &Item);
		void DefineItem (const CString &sVar, CItemCtx &ItemCtx);
		void DefineItemType (const CString &sVar, CItemType *pType);
		inline void DefineNil (const CString &sVar) { m_CC.DefineGlobal(sVar, m_CC.CreateNil()); }
		void DefineOrbit (const CString &sVar, const COrbit &OrbitDesc);
		void DefineSource (CSpaceObject *pSource);
		void DefineSpaceObject (const CString &sVar, CSpaceObject *pObj);
		inline void DefineString (const CString &sVar, const CString &sValue) { m_CC.DefineGlobalString(sVar, sValue); }
		inline void DefineVar (const CString &sVar, ICCItem *pValue) { m_CC.DefineGlobal(sVar, pValue); }
		void DefineVector (const CString &sVar, const CVector &vVector);
		DWORD GetAPIVersion (void) const;
		inline CG16bitImage *GetCanvas (void) const { return m_pCanvas; }
		inline CExtension *GetExtension (void) const { return m_pExtension; }
		inline CItemType *GetItemType (void) const { return m_pItemType; }
		inline CDesignType *GetScreensRoot (void) const { return m_pScreensRoot; }
		inline SSystemCreateCtx *GetSystemCreateCtx (void) const { return m_pSysCreateCtx; }
		bool InEvent (ECodeChainEvents iEvent) const;
		inline ICCItem *Link (const CString &sString, int iOffset, int *retiLinked) { return m_CC.Link(sString, iOffset, retiLinked); }
		void RestoreVars (void);
		ICCItem *Run (ICCItem *pCode);
		ICCItem *Run (const SEventHandlerDesc &Event);
		bool RunEvalString (const CString &sString, bool bPlain, CString *retsResult);
		ICCItem *RunLambda (ICCItem *pCode);
		void SaveAndDefineDataVar (ICCItem *pData);
		void SaveAndDefineItemVar (const CItem &Item);
		void SaveAndDefineItemVar (CItemCtx &ItemCtx);
		void SaveAndDefineSourceVar (CSpaceObject *pSource);
		void SaveItemVar (void);
		void SaveSourceVar (void);
		inline void SetDockScreenList (IListData *pListData) { m_pListData = pListData; }
		inline void SetCanvas (CG16bitImage *pCanvas) { m_pCanvas = pCanvas; }
		inline void SetEvent (ECodeChainEvents iEvent) { m_iEvent = iEvent; }
		inline void SetExtension (CExtension *pExtension) { m_pExtension = pExtension; }
		void SetGlobalDefineWrapper (CExtension *pExtension);
		inline void SetItemType (CItemType *pType) { m_pItemType = pType; }
		inline void SetScreen (void *pScreen) { m_pScreen = pScreen; }
		inline void SetScreensRoot (CDesignType *pRoot) { m_pScreensRoot = pRoot; }
		inline void SetSystemCreateCtx (SSystemCreateCtx *pCtx) { m_pSysCreateCtx = pCtx; }

		CSpaceObject *AsSpaceObject (ICCItem *pItem);
		CVector AsVector (ICCItem *pItem);
		inline void Discard (ICCItem *pItem) { pItem->Discard(&m_CC); }

	private:
		struct SInvokeFrame
			{
			ECodeChainEvents iEvent;		//	Event raised

			//	If we're viewing a dock screen list
			IListData *pListData;
			};

		void AddFrame (void);
		void RemoveFrame (void);

		CCodeChain &m_CC;					//	CodeChain
		ECodeChainEvents m_iEvent;			//	Event raised
		void *m_pScreen;					//	Cast to CDockScreen by upper-levels (may be NULL)
		CG16bitImage *m_pCanvas;			//	Used for dock screen canvas (may be NULL)
		CItemType *m_pItemType;				//	Used for item events (may be NULL)
		CDesignType *m_pScreensRoot;		//	Used to resolve local screens (may be NULL)
		SSystemCreateCtx *m_pSysCreateCtx;	//	Used during system create (may be NULL)
		CExtension *m_pExtension;			//	Extension that defined this code

		IListData *m_pListData;

		//	Saved variables
		ICCItem *m_pOldData;
		ICCItem *m_pOldSource;
		ICCItem *m_pOldItem;

		bool m_bRestoreGlobalDefineHook;
		IItemTransform *m_pOldGlobalDefineHook;

		static TArray<SInvokeFrame> g_Invocations;
	};

class CFunctionContextWrapper : public ICCAtom
	{
	public:
		CFunctionContextWrapper (ICCItem *pFunction);

		inline void SetExtension (CExtension *pExtension) { m_pExtension = pExtension; }

		//	ICCItem virtuals
		virtual ICCItem *Clone (CCodeChain *pCC);
		virtual ICCItem *Execute (CEvalContext *pCtx, ICCItem *pArgs);
		virtual CString GetHelp (void) { return NULL_STR; }
		virtual CString GetStringValue (void) { return m_pFunction->GetStringValue(); }
		virtual ValueTypes GetValueType (void) { return Function; }
		virtual BOOL IsIdentifier (void) { return FALSE; }
		virtual BOOL IsFunction (void) { return TRUE; }
		virtual BOOL IsPrimitive (void) { return FALSE; }
		virtual CString Print (CCodeChain *pCC, DWORD dwFlags = 0) { return m_pFunction->Print(pCC, dwFlags); }
		virtual void Reset (void) { }

	protected:
		//	ICCItem virtuals
		virtual void DestroyItem (CCodeChain *pCC);
		virtual ICCItem *StreamItem (CCodeChain *pCC, IWriteStream *pStream);
		virtual ICCItem *UnstreamItem (CCodeChain *pCC, IReadStream *pStream);

	private:
		ICCItem *m_pFunction;
		CExtension *m_pExtension;
	};

class CAddFunctionContextWrapper : public IItemTransform
	{
	public:
		CAddFunctionContextWrapper (void) : m_pExtension(NULL) { }

		inline void SetExtension (CExtension *pExtension) { m_pExtension = pExtension; }

		//	IItemTransform
		virtual ICCItem *Transform (CCodeChain &CC, ICCItem *pItem);

	private:
		CExtension *m_pExtension;
	};

class CCXMLWrapper : public ICCAtom
	{
	public:
		CCXMLWrapper (CXMLElement *pXML, ICCItem *pRef = NULL);

		inline CXMLElement *GetXMLElement (void) { return m_pXML; }

		//	ICCItem virtuals

		virtual ICCItem *Clone (CCodeChain *pCC);
		virtual CString GetStringValue (void) { return m_pXML->ConvertToString(); }
		virtual CString GetTypeOf (void) { return CONSTLIT("xmlElement"); }
		virtual ValueTypes GetValueType (void) { return Complex; }
		virtual BOOL IsIdentifier (void) { return FALSE; }
		virtual BOOL IsFunction (void) { return FALSE; }
		virtual BOOL IsPrimitive (void) { return FALSE; }
		virtual CString Print (CCodeChain *pCC, DWORD dwFlags = 0) { return CCString::Print(GetStringValue(), dwFlags); }
		virtual void Reset (void) { }

	protected:
		virtual void DestroyItem (CCodeChain *pCC);
		virtual ICCItem *StreamItem (CCodeChain *pCC, IWriteStream *pStream);
		virtual ICCItem *UnstreamItem (CCodeChain *pCC, IReadStream *pStream);

	private:
		CXMLElement *m_pXML;
		ICCItem *m_pRef;
	};

#endif