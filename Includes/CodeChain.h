//	CodeChain.h
//
//	CodeChain is a micro-LISP variant designed for high performance
//	and efficiency.
//
//	Requires Kernel.h
//
//	Basic Coding Rules
//
//	1.	Routines that return an ICCItem * will always increase the refcount
//		if you do not keep the item returned, you must call Discard on the
//		item.
//
//	2.	When passing in ICCItem *, the called routine is responsible for
//		incrementing the refcount if it decides to keep a reference

#ifndef INCL_CODECHAIN
#define INCL_CODECHAIN

class CCodeChain;
class CEvalContext;
class ICCItem;

//	Simple definitions

typedef ICCItem *(*PRIMITIVEPROC)(CEvalContext *pCtx, ICCItem *pArguments, DWORD dwData);

#define PPFLAG_SIDEEFFECTS						0x00000001	//	Function has side-effects
#define PPFLAG_NOERRORS							0x00000002	//	Function never returns errors
#define PPFLAG_SYNONYM							0x00000004	//	Function is a synonym (pszDescription is name of function)
#define PPFLAG_CUSTOM_ARG_EVAL					0x00000008	//	Raw args are passed to the function (before evaluation)

typedef struct
	{
	char *pszName;
	PRIMITIVEPROC pfFunction;
	DWORD dwData;
	char *pszDescription;
	char *pszArguments;
	DWORD dwFlags;
	} PRIMITIVEPROCDEF, *PPRIMITIVEPROCDEF;

//	Error Definitions. These are common result codes returned by basic
//	primitives.

#define CCRESULT_NOTFOUND							1		//	Used for symbol tables, files, etc.
#define CCRESULT_CANCEL								2		//	User canceled operation
#define CCRESULT_DISKERROR							3		//	Out of disk space, etc.

//	A Cons is the primitive list glue.

class CCons
	{
	public:
		ICCItem *m_pItem;
		CCons *m_pNext;
	};

//	Print flags

const DWORD PRFLAG_NO_QUOTES =						0x00000001;
const DWORD	PRFLAG_ENCODE_FOR_DISPLAY =				0x00000002;
const DWORD PRFLAG_FORCE_QUOTES =					0x00000004;

//	Some helper classes

class IItemTransform
	{
	public:
		virtual ~IItemTransform (void) { }

		virtual ICCItem *Transform (CCodeChain &CC, ICCItem *pItem);
	};

//	An item is a generic element of a list. This is the basic unit of
//	CodeChain.

class ICCItem : public CObject
	{
	public:
		enum ValueTypes
			{
			Boolean,
			Integer,
			String,
			List,
			Function,
			Complex,
			};

		ICCItem (IObjectClass *pClass);

		//	Increment and decrement ref counts

		virtual ICCItem *Clone (CCodeChain *pCC) = 0;
		virtual void Discard (CCodeChain *pCC);
		inline ICCItem *Reference (void) { m_dwRefCount++; return this; }
		virtual void Reset (void) = 0;
		inline void SetNoRefCount (void) { m_bNoRefCount = true; }

		//	List interface

		virtual ICCItem *Enum (CEvalContext *pCtx, ICCItem *pCode) = 0;
		virtual int GetCount (void) = 0;
		virtual ICCItem *GetElement (int iIndex) = 0;
		virtual ICCItem *GetElement (const CString &sKey) { return NULL; }
		virtual ICCItem *GetElement (CCodeChain *pCC, int iIndex);
		virtual ICCItem *Head (CCodeChain *pCC) = 0;
		inline BOOL IsList (void) { return IsNil() || !IsAtom(); }
		virtual ICCItem *Tail (CCodeChain *pCC) = 0;

		//	Evaluation

		inline BOOL IsQuoted (void) { return m_bQuoted; }
		inline void SetQuoted (void) { m_bQuoted = TRUE; }
		inline void ClearQuoted (void) { m_bQuoted = FALSE; }

		//	Errors

		inline BOOL IsError (void) { return m_bError; }
		inline void SetError (void) { m_bError = TRUE; }

		//	Load/save

		ICCItem *Stream (CCodeChain *pCC, IWriteStream *pStream);
		ICCItem *Unstream (CCodeChain *pCC, IReadStream *pStream);
		inline void ClearModified (void) { m_bModified = FALSE; }
		inline BOOL IsModified (void) { return m_bModified; }
		inline void SetModified (void) { m_bModified = TRUE; }

		//	Virtuals that must be overridden

		virtual ICCItem *Execute (CEvalContext *pCtx, ICCItem *pArgs);
		virtual BOOL GetBinding (int *retiFrame, int *retiOffset) { return FALSE; }
		virtual ICCItem *GetFunctionBinding (void) { return NULL; }
		virtual CString GetHelp (void) { return NULL_STR; }
		virtual int GetIntegerValue (void) { return 0; }
		virtual CString GetStringValue (void) { return LITERAL(""); }
		virtual ValueTypes GetValueType (void) = 0;
		virtual CString GetTypeOf (void);
		virtual BOOL IsAtom (void) = 0;
		virtual BOOL IsAtomTable (void) { return FALSE; }
		virtual bool IsExpression (void) { return false; }
		virtual BOOL IsFunction (void) = 0;
		virtual BOOL IsIdentifier (void) = 0;
		virtual BOOL IsInteger (void) = 0;
		virtual bool IsLambdaFunction (void) { return false; }
		virtual BOOL IsNil (void) = 0;
		virtual BOOL IsPrimitive (void) { return FALSE; }
		virtual BOOL IsSymbolTable (void) { return FALSE; }
		virtual BOOL IsTrue (void) { return FALSE; }
		virtual CString Print (CCodeChain *pCC, DWORD dwFlags = 0) = 0;
		virtual void SetBinding (int iFrame, int iOffset) { }
		virtual void SetFunctionBinding (CCodeChain *pCC, ICCItem *pBinding) { }

		//	Miscellaneous utility functions

		bool IsLambdaExpression (void);
		BOOL IsLambdaSymbol (void);
		inline bool IsReadOnly (void) { return m_bReadOnly; }
		inline void SetReadOnly (bool bReadOnly = true) { m_bReadOnly = bReadOnly; }
		void ResetItem (void);

		//	Symbol/Atom table functions

		virtual ICCItem *AddEntry (CCodeChain *pCC, ICCItem *pKey, ICCItem *pEntry, bool bForceLocalAdd = false) { return NotASymbolTable(pCC); }
		virtual void AddByOffset (CCodeChain *pCC, int iOffset, ICCItem *pEntry) { ASSERT(FALSE); }
		virtual void DeleteAll (CCodeChain *pCC, bool bLambdaOnly) { }
		virtual void DeleteEntry (CCodeChain *pCC, ICCItem *pKey) { }
		virtual int FindOffset (CCodeChain *pCC, ICCItem *pKey) { return -1; }
		virtual int FindValue (ICCItem *pValue) { return -1; }
		virtual IItemTransform *GetDefineHook (void) { return NULL; }
		virtual ICCItem *GetParent (void) { return NULL; }
		virtual BOOL IsLocalFrame (void) { return FALSE; }
		virtual ICCItem *ListSymbols (CCodeChain *pCC) { return NotASymbolTable(pCC); }
		virtual ICCItem *Lookup (CCodeChain *pCC, ICCItem *pKey) { return NotASymbolTable(pCC); }
		virtual ICCItem *LookupByOffset (CCodeChain *pCC, int iOffset) { return NotASymbolTable(pCC); }
		virtual ICCItem *LookupEx (CCodeChain *pCC, ICCItem *pKey, BOOL *retbFound) { return NotASymbolTable(pCC); }
		virtual void SetDefineHook (IItemTransform *pHook) { }
		virtual void SetParent (ICCItem *pParent) { ASSERT(FALSE); }
		virtual void SetLocalFrame (void) { ASSERT(FALSE); }
		virtual ICCItem *SimpleLookup (CCodeChain *pCC, ICCItem *pKey, BOOL *retbFound, int *retiOffset) { return NotASymbolTable(pCC); }

		//	Pool access

		ICCItem *GetNextFree() { return (ICCItem *)m_dwRefCount; }
		void SetNextFree (ICCItem *pNext) { m_dwRefCount = (DWORD)pNext; }

	protected:
		void CloneItem (ICCItem *pItem);

		virtual void DestroyItem (CCodeChain *pCC) { }
		virtual ICCItem *StreamItem (CCodeChain *pCC, IWriteStream *pStream) = 0;
		virtual ICCItem *UnstreamItem (CCodeChain *pCC, IReadStream *pStream) = 0;

		ICCItem *NotASymbolTable(CCodeChain *pCC);

		DWORD m_dwRefCount;						//	Number of references to this item

		DWORD m_bQuoted:1;						//	TRUE if quoted
		DWORD m_bError:1;						//	TRUE if it represents a runtime error
		DWORD m_bModified:1;					//	TRUE if this item was modified
		DWORD m_bNoRefCount:1;					//	TRUE if we don't care about ref count
		DWORD m_bReadOnly:1;					//	TRUE if we should do a copy-on-write
	};

//	An atom is a single value

class ICCAtom : public ICCItem
	{
	public:
		ICCAtom (IObjectClass *pClass) : ICCItem(pClass) { }

		//	ICCItem virtuals

		virtual ICCItem *Enum (CEvalContext *pCtx, ICCItem *pCode);
		virtual ICCItem *GetElement (int iIndex) { return (iIndex == 0 ? Reference() : NULL); }
		virtual int GetCount (void) { return 1; }
		virtual ICCItem *Head (CCodeChain *pCC) { return Reference(); }
		virtual BOOL IsAtom (void) { return TRUE; }
		virtual BOOL IsInteger (void) { return FALSE; }
		virtual BOOL IsNil (void) { return FALSE; }
		virtual ICCItem *Tail (CCodeChain *pCC);
	};

//	An integer is an atom that represents a natural number

class ICCInteger : public ICCAtom
	{
	public:
		ICCInteger (IObjectClass *pClass) : ICCAtom(pClass) { }

		//	ICCItem virtuals

		virtual ValueTypes GetValueType (void) { return Integer; }
		virtual BOOL IsIdentifier (void) { return FALSE; }
		virtual BOOL IsInteger (void) { return TRUE; }
		virtual BOOL IsFunction (void) { return FALSE; }
	};

//	This is a standard implementation of an integer

class CCInteger : public ICCInteger
	{
	public:
		CCInteger (void);

		inline int GetValue (void) { return m_iValue; }
		inline void SetValue (int iValue) { m_iValue = iValue; }

		//	ICCItem virtuals

		virtual ICCItem *Clone (CCodeChain *pCC);
		virtual int GetIntegerValue (void) { return m_iValue; }
		virtual CString GetStringValue (void) { return strFromInt(m_iValue); }
		virtual CString Print (CCodeChain *pCC, DWORD dwFlags = 0);
		virtual void Reset (void);

	protected:
		virtual void DestroyItem (CCodeChain *pCC);
		virtual ICCItem *StreamItem (CCodeChain *pCC, IWriteStream *pStream);
		virtual ICCItem *UnstreamItem (CCodeChain *pCC, IReadStream *pStream);

	private:
		int m_iValue;							//	Value of 32-bit integer
	};

//	Nil class

class CCNil : public ICCAtom
	{
	public:
		CCNil (void);

		//	ICCItem virtuals

		virtual ICCItem *Clone (CCodeChain *pCC);
		virtual int GetCount (void) { return 0; }
		virtual int GetIntegerValue (void) { return 0; }
		virtual CString GetStringValue (void) { return LITERAL("Nil"); }
		virtual ValueTypes GetValueType (void) { return Boolean; }
		virtual BOOL IsIdentifier (void) { return FALSE; }
		virtual BOOL IsInteger (void) { return TRUE; }
		virtual BOOL IsFunction (void) { return FALSE; }
		virtual BOOL IsNil (void) { return TRUE; }
		virtual CString Print (CCodeChain *pCC, DWORD dwFlags = 0) { return LITERAL("Nil"); }
		virtual void Reset (void) { }

	protected:
		virtual void DestroyItem (CCodeChain *pCC);
		virtual ICCItem *StreamItem (CCodeChain *pCC, IWriteStream *pStream);
		virtual ICCItem *UnstreamItem (CCodeChain *pCC, IReadStream *pStream);
	};

//	True class

class CCTrue : public ICCAtom
	{
	public:
		CCTrue (void);

		//	ICCItem virtuals

		virtual ICCItem *Clone (CCodeChain *pCC);
		virtual int GetIntegerValue (void) { return 1; }
		virtual CString GetStringValue (void) { return LITERAL("True"); }
		virtual ValueTypes GetValueType (void) { return Boolean; }
		virtual BOOL IsIdentifier (void) { return FALSE; }
		virtual BOOL IsFunction (void) { return FALSE; }
		virtual BOOL IsTrue (void) { return TRUE; }
		virtual CString Print (CCodeChain *pCC, DWORD dwFlags = 0) { return LITERAL("True"); }
		virtual void Reset (void) { }

	protected:
		virtual void DestroyItem (CCodeChain *pCC);
		virtual ICCItem *StreamItem (CCodeChain *pCC, IWriteStream *pStream);
		virtual ICCItem *UnstreamItem (CCodeChain *pCC, IReadStream *pStream);
	};

//	A string is an atom that represents a sequence of characters

class ICCString : public ICCAtom
	{
	public:
		ICCString (IObjectClass *pClass) : ICCAtom(pClass) { }

		//	ICCItem virtuals

		virtual ValueTypes GetValueType (void) { return String; }
		virtual BOOL IsIdentifier (void) { return TRUE; }
		virtual BOOL IsFunction (void) { return FALSE; }
	};

//	This is the standard implementation of a string

class CCString : public ICCString
	{
	public:
		CCString (void);

		inline CString GetValue (void) { return m_sValue; }
		static CString Print (const CString &sString, DWORD dwFlags = 0);
		inline void SetValue (CString sValue) { m_sValue = sValue; }

		//	ICCItem virtuals

		virtual ICCItem *Clone (CCodeChain *pCC);
		virtual BOOL GetBinding (int *retiFrame, int *retiOffset);
		virtual int GetIntegerValue (void) { return strToInt(m_sValue, 0); }
		virtual ICCItem *GetFunctionBinding (void) { if (m_pBinding) return m_pBinding->Reference(); else return NULL; }
		virtual CString GetStringValue (void) { return m_sValue; }
		virtual CString Print (CCodeChain *pCC, DWORD dwFlags = 0);
		virtual void SetBinding (int iFrame, int iOffset);
		virtual void SetFunctionBinding (CCodeChain *pCC, ICCItem *pBinding);
		virtual void Reset (void);

	protected:
		virtual void DestroyItem (CCodeChain *pCC);
		virtual ICCItem *StreamItem (CCodeChain *pCC, IWriteStream *pStream);
		virtual ICCItem *UnstreamItem (CCodeChain *pCC, IReadStream *pStream);

	private:
		CString m_sValue;						//	Value of string
		int m_dwBinding;						//	Index into binding
		ICCItem *m_pBinding;					//	Function binding
	};

//	This is a primitive function definition

class CCPrimitive : public ICCAtom
	{
	public:
		CCPrimitive (void);

		void SetProc (PRIMITIVEPROCDEF *pDef);

		//	ICCItem virtuals

		virtual ICCItem *Clone (CCodeChain *pCC);
		virtual ICCItem *Execute (CEvalContext *pCtx, ICCItem *pArgs);
		virtual CString GetHelp (void) { return m_sDesc; }
		virtual CString GetStringValue (void) { return m_sName; }
		virtual ValueTypes GetValueType (void) { return Function; }
		virtual BOOL IsIdentifier (void) { return FALSE; }
		virtual BOOL IsFunction (void) { return TRUE; }
		virtual BOOL IsPrimitive (void) { return TRUE; }
		virtual CString Print (CCodeChain *pCC, DWORD dwFlags = 0);
		virtual void Reset (void);

	protected:
		virtual void DestroyItem (CCodeChain *pCC);
		virtual ICCItem *StreamItem (CCodeChain *pCC, IWriteStream *pStream);
		virtual ICCItem *UnstreamItem (CCodeChain *pCC, IReadStream *pStream);

	private:
		CString m_sName;
		PRIMITIVEPROC m_pfFunction;
		CString m_sArgPattern;
		CString m_sDesc;
		DWORD m_dwData;
		DWORD m_dwFlags;
	};

//	This is a lambda structure

class CCLambda : public ICCAtom
	{
	public:
		CCLambda (void);

		ICCItem *CreateFromList (CCodeChain *pCC, ICCItem *pList, BOOL bArgsOnly);
		void SetLocalSymbols (CCodeChain *pCC, ICCItem *pSymbols);

		//	ICCItem virtuals

		virtual ICCItem *Clone (CCodeChain *pCC);
		virtual ICCItem *Execute (CEvalContext *pCtx, ICCItem *pArgs);
		virtual CString GetStringValue (void) { return LITERAL("[lambda expression]"); }
		virtual ValueTypes GetValueType (void) { return Function; }
		virtual BOOL IsIdentifier (void) { return FALSE; }
		virtual BOOL IsFunction (void) { return TRUE; }
		virtual bool IsLambdaFunction (void) { return true; }
		virtual CString Print (CCodeChain *pCC, DWORD dwFlags = 0);
		virtual void Reset (void);

	protected:
		virtual void DestroyItem (CCodeChain *pCC);
		virtual ICCItem *StreamItem (CCodeChain *pCC, IWriteStream *pStream);
		virtual ICCItem *UnstreamItem (CCodeChain *pCC, IReadStream *pStream);

	private:
		ICCItem *m_pArgList;
		ICCItem *m_pCode;
		ICCItem *m_pLocalSymbols;
	};

//	A list is a list of items

class ICCList : public ICCItem
	{
	public:
		ICCList (IObjectClass *pClass) : ICCItem(pClass) { }

		//	ICCItem virtuals

		virtual ValueTypes GetValueType (void) { return List; }
		virtual BOOL IsAtom (void) { return FALSE; }
		virtual BOOL IsFunction (void) { return FALSE; }
		virtual BOOL IsIdentifier (void) { return FALSE; }
		virtual BOOL IsInteger (void) { return FALSE; }
		virtual BOOL IsNil (void) { return (GetCount() == 0); }
	};

//	This is a linked-list implementation of a list

class CCLinkedList : public ICCList
	{
	public:
		CCLinkedList (void);
		virtual ~CCLinkedList (void);

		void Append (CCodeChain *pCC, ICCItem *pItem, ICCItem **retpError = NULL);
		void AppendIntegerValue (CCodeChain *pCC, int iValue, ICCItem **retpError = NULL);
		void AppendStringValue (CCodeChain *pCC, const CString &sString, ICCItem **retpError = NULL);
		void CreateIndex (void);
		void RemoveElement (CCodeChain *pCC, int iIndex);
		void ReplaceElement (CCodeChain *pCC, int iIndex, ICCItem *pNewItem);
		void Shuffle (CCodeChain *pCC);
		void Sort (CCodeChain *pCC, int iOrder, int iIndex = -1);

		//	ICCItem virtuals

		virtual ICCItem *Clone (CCodeChain *pCC);
		virtual ICCItem *Enum (CEvalContext *pCtx, ICCItem *pCode);
		virtual int GetCount (void) { return m_iCount; }
		virtual ICCItem *GetElement (int iIndex);
		virtual ICCItem *Head (CCodeChain *pCC) { return GetElement(0); }
		virtual bool IsExpression (void) { return (GetCount() > 0); }
		virtual CString Print (CCodeChain *pCC, DWORD dwFlags = 0);
		virtual ICCItem *Tail (CCodeChain *pCC);
		virtual void Reset (void);

	protected:
		virtual void DestroyItem (CCodeChain *pCC);
		virtual ICCItem *StreamItem (CCodeChain *pCC, IWriteStream *pStream);
		virtual ICCItem *UnstreamItem (CCodeChain *pCC, IReadStream *pStream);

	private:
		void QuickSort (int iLeft, int iRight, int iOrder);
		void QuickSortLists (int iKeyIndex, int iLeft, int iRight, int iOrder);
		void UpdateLinksFromIndex (void);

		CCons *m_pFirst;						//	First element in the list
		CCons *m_pLast;							//	Last element in the list
		int m_iCount;							//	Number of elements

		CCons **m_pIndex;						//	GetElement array
	};

//	This is an array of 32-bit integers

class CCVector : public ICCList
	{
	public:
		CCVector (void);
		CCVector (CCodeChain *pCC);
		virtual ~CCVector (void);

		int *GetArray (void);
		BOOL SetElement (int iIndex, int iElement);
		ICCItem *SetSize (CCodeChain *pCC, int iNewSize);

		//	ICCItem virtuals

		virtual ICCItem *Clone (CCodeChain *pCC);
		virtual ICCItem *Enum (CEvalContext *pCtx, ICCItem *pCode);
		virtual int GetCount (void) { return m_iCount; }
		virtual ICCItem *GetElement (int iIndex);
		virtual ICCItem *Head (CCodeChain *pCC) { return GetElement(0); }
		virtual CString Print (CCodeChain *pCC, DWORD dwFlags = 0);
		virtual ICCItem *Tail (CCodeChain *pCC);
		virtual void Reset (void);

	protected:
		virtual void DestroyItem (CCodeChain *pCC);
		virtual ICCItem *StreamItem (CCodeChain *pCC, IWriteStream *pStream);
		virtual ICCItem *UnstreamItem (CCodeChain *pCC, IReadStream *pStream);

	private:
		CCodeChain *m_pCC;						//	CodeChain
		int m_iCount;							//	Number of elements
		int *m_pData;							//	Array of elements
	};

//	This is an atom table object

class CCAtomTable : public ICCAtom
	{
	public:
		CCAtomTable (void);

		//	ICCItem virtuals

		virtual ICCItem *Clone (CCodeChain *pCC);
		virtual ValueTypes GetValueType (void) { return Complex; }
		virtual BOOL IsIdentifier (void) { return FALSE; }
		virtual BOOL IsFunction (void) { return FALSE; }
		virtual BOOL IsAtomTable (void) { return TRUE; }
		virtual CString Print (CCodeChain *pCC, DWORD dwFlags = 0);
		virtual void Reset (void);

		virtual ICCItem *AddEntry (CCodeChain *pCC, ICCItem *pAtom, ICCItem *pEntry, bool bForceLocalAdd = false);
		virtual ICCItem *ListSymbols (CCodeChain *pCC);
		virtual ICCItem *Lookup (CCodeChain *pCC, ICCItem *pAtom);
		virtual ICCItem *LookupEx (CCodeChain *pCC, ICCItem *pAtom, BOOL *retbFound);

	protected:
		virtual void DestroyItem (CCodeChain *pCC);
		virtual ICCItem *StreamItem (CCodeChain *pCC, IWriteStream *pStream);
		virtual ICCItem *UnstreamItem (CCodeChain *pCC, IReadStream *pStream);

	private:
		CDictionary m_Table;
	};

//	This is a symbol table object

class CCSymbolTable : public ICCList
	{
	public:
		CCSymbolTable (void);

		inline CString GetKey (int iIndex) { return m_Symbols.GetKey(iIndex); }
		void SetIntegerValue (CCodeChain &CC, const CString &sKey, int iValue);
		void SetStringValue (CCodeChain &CC, const CString &sKey, const CString &sValue);
		void SetValue (CCodeChain &CC, const CString &sKey, ICCItem *pValue);

		//	ICCItem virtuals

		virtual ICCItem *Clone (CCodeChain *pCC);
		virtual ValueTypes GetValueType (void) { return Complex; }
		virtual BOOL IsIdentifier (void) { return FALSE; }
		virtual BOOL IsFunction (void) { return FALSE; }
		virtual BOOL IsLocalFrame (void) { return m_bLocalFrame; }
		virtual BOOL IsSymbolTable (void) { return TRUE; }
		virtual CString Print (CCodeChain *pCC, DWORD dwFlags = 0);
		virtual void Reset (void);

		//	List interface

		virtual ICCItem *Enum (CEvalContext *pCtx, ICCItem *pCode) { ASSERT(false); return NULL; }
		virtual int GetCount (void) { return m_Symbols.GetCount(); }
		virtual ICCItem *GetElement (int iIndex);
		virtual ICCItem *GetElement (const CString &sKey);
		virtual ICCItem *GetElement (CCodeChain *pCC, int iIndex);
		virtual ICCItem *Head (CCodeChain *pCC) { return GetElement(0); }
		virtual ICCItem *Tail (CCodeChain *pCC) { return GetElement(1); }

		//	Symbols

		virtual void AddByOffset (CCodeChain *pCC, int iOffset, ICCItem *pEntry);
		virtual ICCItem *AddEntry (CCodeChain *pCC, ICCItem *pKey, ICCItem *pEntry, bool bForceLocalAdd = false);
		virtual void DeleteAll (CCodeChain *pCC, bool bLambdaOnly);
		virtual void DeleteEntry (CCodeChain *pCC, ICCItem *pKey);
		virtual int FindOffset (CCodeChain *pCC, ICCItem *pKey);
		virtual int FindValue (ICCItem *pValue);
		virtual IItemTransform *GetDefineHook (void) { return m_pDefineHook; }
		virtual ICCItem *GetParent (void) { return m_pParent; }
		virtual ICCItem *ListSymbols (CCodeChain *pCC);
		virtual ICCItem *Lookup (CCodeChain *pCC, ICCItem *pKey);
		virtual ICCItem *LookupByOffset (CCodeChain *pCC, int iOffset);
		virtual ICCItem *LookupEx (CCodeChain *pCC, ICCItem *pKey, BOOL *retbFound);
		virtual void SetDefineHook (IItemTransform *pHook) { m_pDefineHook = pHook; }
		virtual void SetLocalFrame (void) { m_bLocalFrame = TRUE; }
		virtual void SetParent (ICCItem *pParent) { m_pParent = pParent->Reference(); }
		virtual ICCItem *SimpleLookup (CCodeChain *pCC, ICCItem *pKey, BOOL *retbFound, int *retiOffset);

	protected:
		virtual void DestroyItem (CCodeChain *pCC);
		virtual ICCItem *StreamItem (CCodeChain *pCC, IWriteStream *pStream);
		virtual ICCItem *UnstreamItem (CCodeChain *pCC, IReadStream *pStream);

	private:
		CSymbolTable m_Symbols;
		ICCItem *m_pParent;
		BOOL m_bLocalFrame;

		IItemTransform *m_pDefineHook;
	};

//	Item pools

template <class ItemClass>
class CCItemPool
	{
	public:
		CCItemPool (void);
		~CCItemPool (void);

		ICCItem *CreateItem (CCodeChain *pCC);
		void DestroyItem (CCodeChain *pCC, ICCItem *pItem);
		inline int GetCount (void) { return m_iCount; }

	private:
		ICCItem *m_pFreeList;
		ItemClass **m_pBackbone;
		int m_iBackboneSize;
		int m_iCount;
	};

class CConsPool
	{
	public:
		CConsPool (void);
		~CConsPool (void);

		CCons *CreateCons (void);
		void DestroyCons (CCons *pCons);
		inline int GetCount (void) { return m_iCount; }

	private:
		CCons *m_pFreeList;
		CCons **m_pBackbone;
		int m_iBackboneSize;
		int m_iCount;
	};

//	Misc structures

class CEvalContext
	{
	public:
		CCodeChain *pCC;
		ICCItem *pLexicalSymbols;
		ICCItem *pLocalSymbols;

		LPVOID pExternalCtx;
	};

//	This is the main CodeChain context

class CCodeChain : public CObject
	{
	public:
		CCodeChain (void);
		virtual ~CCodeChain (void);

		ALERROR Boot (void);
		void CleanUp (void);

		//	Create/Destroy routines

		ICCItem *CreateAtomTable (void);
		ICCItem *CreateBool (bool bValue);
		inline CCons *CreateCons (void) { return m_ConsPool.CreateCons(); }
		ICCItem *CreateError (const CString &sError, ICCItem *pData = NULL);
		ICCItem *CreateErrorCode (int iErrorCode);
		ICCItem *CreateInteger (int iValue);
		ICCItem *CreateLambda (ICCItem *pList, BOOL bArgsOnly);
		ICCItem *CreateLinkedList (void);
		inline ICCItem *CreateMemoryError (void) { return m_sMemoryError.Reference(); }
		inline ICCItem *CreateNil (void) { return m_pNil->Reference(); }
		ICCItem *CreatePrimitive (PRIMITIVEPROCDEF *pDef);
		ICCItem *CreateString (const CString &sString);
		ICCItem *CreateSymbolTable (void);
		ICCItem *CreateSystemError (ALERROR error);
		inline ICCItem *CreateTrue (void) { return m_pTrue->Reference(); }
		ICCItem *CreateVector (int iSize);
		inline void DestroyAtomTable (ICCItem *pItem) { m_AtomTablePool.DestroyItem(this, pItem); }
		inline void DestroyCons (CCons *pCons) { m_ConsPool.DestroyCons(pCons); }
		inline void DestroyInteger (ICCItem *pItem) { m_IntegerPool.DestroyItem(this, pItem); }
		inline void DestroyLambda (ICCItem *pItem) { m_LambdaPool.DestroyItem(this, pItem); }
		inline void DestroyLinkedList (ICCItem *pItem) { m_ListPool.DestroyItem(this, pItem); }
		inline void DestroyPrimitive (ICCItem *pItem) { m_PrimitivePool.DestroyItem(this, pItem); }
		inline void DestroyString (ICCItem *pItem) { m_StringPool.DestroyItem(this, pItem); }
		inline void DestroySymbolTable (ICCItem *pItem) { m_SymbolTablePool.DestroyItem(this, pItem); }
		inline void DestroyVector (ICCItem *pItem) { delete pItem; }

		//	Load/save routines

		ICCItem *StreamItem (ICCItem *pItem, IWriteStream *pStream);
		ICCItem *UnstreamItem (IReadStream *pStream);

		//	Evaluation and parsing routines

		ICCItem *Apply (ICCItem *pFunc, ICCItem *pArgs, LPVOID pExternalCtx);
		inline ICCItem *GetNil (void) { return m_pNil; }
		inline ICCItem *GetTrue (void) { return m_pTrue; }
		ICCItem *Eval (CEvalContext *pEvalCtx, ICCItem *pItem);
		ICCItem *Link (const CString &sString, int iOffset, int *retiLinked = NULL, int *ioiCurLine = NULL);
		ICCItem *LoadApp (HMODULE hModule, char *pszRes);
		ICCItem *LoadInitFile (const CString &sFilename);
		ICCItem *LookupGlobal (const CString &sGlobal, LPVOID pExternalCtx);
		ICCItem *TopLevel (ICCItem *pItem, LPVOID pExternalCtx);
		CString Unlink (ICCItem *pItem);

		//	Extensions

		ALERROR DefineGlobal (const CString &sVar, ICCItem *pValue);
		ALERROR DefineGlobalInteger (const CString &sVar, int iValue);
		ALERROR DefineGlobalString (const CString &sVar, const CString &sValue);
		void DiscardAllGlobals (void);
		ICCItem *EvaluateArgs (CEvalContext *pCtx, ICCItem *pArgs, const CString &sArgValidation);
		IItemTransform *GetGlobalDefineHook (void) const { return m_pGlobalSymbols->GetDefineHook(); }
		inline ICCItem *GetGlobals (void) { return m_pGlobalSymbols; }
		ICCItem *ListGlobals (void);
		ICCItem *LookupFunction (CEvalContext *pCtx, ICCItem *pName);
		ICCItem *PoolUsage (void);
		ALERROR RegisterPrimitive (PRIMITIVEPROCDEF *pDef);
		inline void SetGlobalDefineHook (IItemTransform *pHook) { m_pGlobalSymbols->SetDefineHook(pHook); }

		//	Miscellaneous

		bool HasIdentifier (ICCItem *pCode, const CString &sIdentifier);

	private:
		ICCItem *CreateParseError (int iLine, const CString &sError);
		ICCItem *EvalLiteralStruct (CEvalContext *pCtx, ICCItem *pItem);
		ICCItem *Lookup (CEvalContext *pCtx, ICCItem *pItem);
		ALERROR LoadDefinitions (IReadBlock *pBlock);
		char *SkipWhiteSpace (char *pPos, int *ioiLine);

		CCItemPool<CCInteger> m_IntegerPool;
		CCItemPool<CCString> m_StringPool;
		CCItemPool<CCLinkedList> m_ListPool;
		CCItemPool<CCPrimitive> m_PrimitivePool;
		CCItemPool<CCAtomTable> m_AtomTablePool;
		CCItemPool<CCSymbolTable> m_SymbolTablePool;
		CCItemPool<CCLambda> m_LambdaPool;
		CConsPool m_ConsPool;
		ICCItem *m_pNil;
		ICCItem *m_pTrue;
		CCString m_sMemoryError;

		ICCItem *m_pGlobalSymbols;
	};

//	Libraries

ALERROR pageLibraryInit (CCodeChain &CC);

//	Utilities

int HelperCompareItems (ICCItem *pFirst, ICCItem *pSecond, bool bCoerce = true);
int HelperCompareItemsLists (ICCItem *pFirst, ICCItem *pSecond, int iKeyIndex, bool bCoerce = true);

#endif