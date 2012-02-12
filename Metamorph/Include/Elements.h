//	Elements.h
//
//	Elemental classes and algorithms
//	Copyright (c) 2000 by NeuroHack, Inc. All Rights Reserved.

#ifndef INCL_ELEMENTS
#define INCL_ELEMENTS

//#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#ifndef _WIN32_WINNT
#define _WIN32_WINNT	0x0400		//	WinNT 4.0 or higher (needed for TrackMouseEvents)
#endif
#include <windows.h>

//	HACK: Declare _alloca so that we don't have to include malloc.h
extern "C" void *          __cdecl _alloca(size_t);

//	DEBUGGING SUPPORT ---------------------------------------------------------

#ifdef _DEBUG
#ifndef DEBUG
#define DEBUG
#endif
#endif

#ifndef ASSERT
#ifdef DEBUG
#define ASSERT(exp)						\
			{							\
			if (!(exp))					\
				DebugBreak();			\
			}
#else
#define ASSERT(exp)
#endif
#endif

//	BASIC TYPES ---------------------------------------------------------------

//	API flags

#define API_FLAG_MASKBLT				0x00000001	//	MaskBlt is available
#define API_FLAG_WINNT					0x00000002	//	Running on Windows NT

//	MISCELLANEOUS MACROS ------------------------------------------------------

inline int Absolute(int iValue) { return (iValue < 0 ? -iValue : iValue); }
inline int AlignUp(int iValue, int iGranularity) { return ((iValue + iGranularity - 1) / iGranularity) * iGranularity; }
inline int RectHeight(const RECT &Rect) { return Rect.bottom - Rect.top; }
inline int RectWidth(const RECT &Rect) { return Rect.right - Rect.left; }

//	FORWARDS

class IMemoryBlock;

//	STRING CLASS --------------------------------------------------------------

class CString
	{
	public:
		CString (void) : m_pString(NULL) { }
		CString (const CString &sStr) : m_pString(sStr.CopyBuffer()) { }
		CString (char *szStr);
		CString (char *pStr, int iLen);
		CString (char *pStr, bool bLiteral) : m_pString(pStr) { }
		CString (char *pStr, int iLen, bool bLiteral);
		~CString (void);

		CString &operator= (const CString &sStr);
		CString &operator= (IMemoryBlock &Block);
		operator char *() const { return m_pString; }

		inline void Append (const CString &sStr) { Insert(sStr, -1); }
		void Delete (int iIndex, int iCount = 1);
		inline char *GetASCIIZ (void) const { if (m_pString) return m_pString; else return ""; }
		int GetLength (void) const;
		char *GetWritePointer (int iLength);
		void Insert (const CString &sStr, int iIndex = -1);
		inline bool IsBlank (void) const { return GetLength() == 0; }
		void Truncate (int iLength);

	private:
		inline int BufferLength (void) const { return (sizeof(int) + GetLength() + 1); }
		void CleanUp (void);
		char *CopyBuffer (void) const;
		inline int GetLengthParameter (void) const { return *((int *)(m_pString - sizeof(int))); }
		void Init (char *pStr, int iLen, bool bLiteral);
		inline bool IsLiteral (void) const { return (m_pString && GetLengthParameter() < 0); }

		char *m_pString;
	};

template <int l> struct SConstString
	{
	int iLength;
	char pString[l];
	};

#define DECLARE_CONST_STRING(label,string)	\
	static SConstString<sizeof(string)> g_p##label = { 0-(sizeof(string)-1), string }; \
	static const CString label (g_p##label##.pString, true);

extern const CString NULL_STRING;

#define CONSTLIT(x)		(CString(x, sizeof(x)-1, false))

CString strCat (const CString &sString1, const CString &sString2);
int strCompare (const CString &sString1, const CString &sString2);
bool strEquals (const CString &sStr1, const CString &sStr2);
bool strEqualsCase (const CString &sStr1, const CString &sStr2);
int strFind (const CString &sString, const CString &sTarget);
CString strFromInt (int iInteger, bool bSigned = true);
inline bool strIsAlphanumeric (char *pPos) { return (*pPos >= '0' && *pPos <= '9') || (*pPos >= 'A' && *pPos <= 'Z') || (*pPos >= 'a' && * pPos <= 'z'); }
inline bool strIsWhitespace (char *pPos) { return *pPos == ' ' || *pPos == '\t' || *pPos == '\n' || *pPos == '\r'; }
CString strPattern (char *pPattern, LPVOID *pArgs);
CString strPatternSubst (char *pLine, ...);
int strParseInt (char *pStart, int iNullResult, char **retpEnd, bool *retbNullValue);
int strParseIntOfBase (char *pStart, int iBase, int iNullResult, char **retpEnd, bool *retbNullValue);
void strParseWhitespace (char *pPos, char **retpPos);
int strToInt (const CString &sString, int iFailResult, bool *retbFailed);
CString strToLower (const CString &sString);
CString strToUpper (const CString &sString);
CString strTrimWhitespace (const CString &sString);
CString strSubString (const CString &sString, int iOffset, int iLength = -1);

//	ERROR CLASS ---------------------------------------------------------------

enum ErrorCodes
	{
	errFail,						//	Operation failed in a way that may need to
									//	be displayed to the user. (e.g., parsing
									//	error that must be shown to the user).
									//	m_sErrorInfo is error message.

	errCancel,						//	The user cancelled an operation in
									//	the middle. Upper layers must recover
									//	silently.

	errOutOfMemory,
	errOutOfDiskspace,
	errUnableToOpenFile,			//	m_sErrorInfo is file and reason for error
	errUnableToSaveFile,			//	m_sErrorInfo is file and reason for error
	errPathInvalid,					//	m_sErrorInfo is path and reason for error
	errDisk,						//	Operation failed because of bad disk or
									//	database corruption or other disk-related
									//	reason that the user cannot fix easily.
									//	m_sErrorInfo is user message.

	errProgrammerError,				//	m_sErrorInfo may have extra data
	};

class CException
	{
	public:
		CException (ErrorCodes Code) : m_Code(Code) { }
		CException (ErrorCodes Code, const CString &sErrorInfo) : m_Code(Code), m_sErrorInfo(sErrorInfo) { }

		inline ErrorCodes GetCode (void) const { return m_Code; }
		inline const CString &GetErrorInfo (void) const { return m_sErrorInfo; }
		CString GetMessage (void);

	private:
		ErrorCodes m_Code;
		CString m_sErrorInfo;
	};

//	SMART POINTERS ------------------------------------------------------------

template <class POINTER> class TSmartPointer
	{
	public:
		TSmartPointer (void) : m_pPtr(NULL) { }
		TSmartPointer (POINTER *pPtr) : m_pPtr(pPtr) { }

        ~TSmartPointer (void)
			{
			if (m_pPtr)
				delete m_pPtr; 
			}

        inline POINTER * operator-> (void) { return m_pPtr; }
		TSmartPointer<POINTER> &operator= (POINTER *pPtr)
			{
			if (m_pPtr)
				delete m_pPtr;
			m_pPtr = pPtr;
			}

	private:
		POINTER *m_pPtr;
	};

//	ARRAY CLASS ---------------------------------------------------------------

//	Explicit placement operator
inline void *operator new (size_t, void *p) { return p; }

class CArrayBase
	{
	protected:
		CArrayBase (HANDLE hHeap);
		~CArrayBase (void);

		void DeleteBytes (int iOffset, int iLength);
		void InsertBytes (int iOffset, void *pData, int iLength, int iAllocQuantum);
		void Resize (int iNewSize, bool bPreserve, int iAllocQuantum);

		HANDLE m_hHeap;
		int m_iSize;
		int m_iAllocSize;
		char *m_pArray;
	};

#pragma warning(disable:4291)			//	No need for a delete because we're placing object
template <class VALUE> class TArray : public CArrayBase
	{
	public:
		TArray (HANDLE hHeap = NULL) : CArrayBase(NULL) { }
		TArray (const TArray<VALUE> &Obj) : CArrayBase(Obj.m_hHeap)
			{
			m_iSize = 0;
			m_iAllocSize = 0;
			m_pArray = NULL;
			Resize(Obj.m_iAllocSize, false, sizeof(VALUE));
			for (int i = 0; i < Obj.GetCount(); i++)
				{
				VALUE *pElement = new(m_pArray + (i * sizeof(VALUE))) VALUE(Obj[i]);
				}
			m_iSize = Obj.m_iSize;
			}

		~TArray (void) { DeleteAll(); }

		TArray<VALUE> &operator= (const TArray<VALUE> &Obj)
			{
			DeleteAll();
			m_hHeap = Obj.m_hHeap;
			m_iSize = 0;
			Resize(Obj.m_iAllocSize, false, sizeof(VALUE));
			for (int i = 0; i < Obj.GetCount(); i++)
				{
				VALUE *pElement = new(m_pArray + (i * sizeof(VALUE))) VALUE(Obj[i]);
				}
			m_iSize = Obj.m_iSize;
			return *this;
			}

		inline VALUE &operator [] (int iIndex) const { return GetAt(iIndex); }

		void Delete (int iIndex)
			{
			VALUE *pElement = (VALUE *)(m_pArray + iIndex * sizeof(VALUE));
			ASSERT((char *)pElement - m_pArray < m_iSize);
			pElement->VALUE::~VALUE();
			DeleteBytes(iIndex * sizeof(VALUE), sizeof(VALUE));
			}

		void DeleteAll (void)
			{
			VALUE *pElement = (VALUE *)m_pArray;
			for (int i = 0; i < GetCount(); i++, pElement++)
				pElement->VALUE::~VALUE();

			DeleteBytes(0, m_iSize);
			}

		inline VALUE &GetAt (int iIndex) const
			{
			VALUE *pElement = (VALUE *)(m_pArray + iIndex * sizeof(VALUE));
			return *pElement;
			}

		inline int GetCount (void) const
			{
			return m_iSize / sizeof(VALUE);
			}

		void Insert (const VALUE &Value, int iIndex = -1)
			{
			int iOffset;
			if (iIndex == -1) iIndex = GetCount();
			iOffset = iIndex * sizeof(VALUE);
			InsertBytes(iOffset, NULL, sizeof(VALUE), 10 * sizeof(VALUE));

			VALUE *pElement = new(m_pArray + iOffset) VALUE(Value);
			}

		void InsertEmpty (int iCount = 1, int iIndex = -1)
			{
			int iOffset;
			if (iIndex == -1) iIndex = GetCount();
			iOffset = iIndex * sizeof(VALUE);
			InsertBytes(iOffset, NULL, iCount * sizeof(VALUE), iCount * 10 * sizeof(VALUE));

			for (int i = 0; i < iCount; i++)
				{
				VALUE *pElement = new(m_pArray + iOffset + (i * sizeof(VALUE))) VALUE();
				}
			}
	};
#pragma warning(default:4291)

//	Simple array classes

class CStringArray : public TArray<CString> { };
class CIntArray : public TArray<int> { };

//	MAP CLASS -----------------------------------------------------------------

class CMapIterator
	{
	public:
		CMapIterator (void) : m_iTableEntry(0), m_pPos(NULL) { }

	private:
		int m_iTableEntry;
		void *m_pPos;

	friend class CMapBase;
	};

class CMapBase
	{
	protected:
		struct EntryBase
			{
			EntryBase *pNext;
			};

		CMapBase (int iTableSize);
		~CMapBase (void);

		EntryBase *DeleteEntry (void *pVoidKey, void *pKey, int iKeyLen);
		EntryBase *FindEntry (void *pVoidKey, void *pKey, int iKeyLen, int *retiSlot = NULL, EntryBase **retpPrevEntry = NULL) const;
		int Hash (void *pKey, int iKeyLen) const;
		void InsertEntry (void *pKey, int iKeyLen, EntryBase *pEntry);

		void Reset (CMapIterator &Iterator);
		EntryBase *GetNext (CMapIterator &Iterator);
		bool HasMore (CMapIterator &Iterator);

		virtual bool KeyEquals (void *pVoidKey, EntryBase *pEntry) const = 0;

		EntryBase **m_pTable;						//	An array of entries
		int m_iTableSize;							//	Size of table
		int m_iCount;
	};

//	Comparison functions

bool MapKeyEquals (const CString &sKey1, const CString &sKey2);
template<class KEY> bool MapKeyEquals (const KEY &Key1, const KEY &Key2) { return Key1 == Key2; }

void *MapKeyHashData (const CString &Key);
template<class KEY> void *MapKeyHashData (const KEY &Key) { return (void *)&Key; }
int MapKeyHashDataSize (const CString &Key);
template<class KEY> int MapKeyHashDataSize (const KEY &Key) { return sizeof(KEY); }

//	Template

template <class KEY, class VALUE> class TMap : public CMapBase
	{
	public:
		TMap (int iTableSize = 999) : CMapBase(iTableSize) { }
		TMap (const TMap<KEY, VALUE> &Src) : CMapBase(Src.m_iTableSize)
			{
			Copy(Src);
			}

		~TMap (void)
			{
			DeleteAll();
			}

		TMap<KEY, VALUE> &operator= (const TMap<KEY, VALUE> &Obj)
			{
			DeleteAll();

			delete [] m_pTable;
			m_iTableSize = Obj.m_iTableSize;
			m_pTable = new EntryBase * [Obj.m_iTableSize];

			Copy(Obj);

			return *this;
			}

		void Delete (const KEY &Key)
			{
			Entry *pDeletedEntry = (Entry *)DeleteEntry((void *)&Key, MapKeyHashData(Key), MapKeyHashDataSize(Key));
			if (pDeletedEntry)
				delete pDeletedEntry;
			}

		void DeleteAll (void)
			{
			for (int i = 0; i < m_iTableSize; i++)
				{
				Entry *pEntry = (Entry *)m_pTable[i];
				while (pEntry)
					{
					Entry *pNextEntry = pEntry->m_pNext;
					delete pEntry;
					pEntry = pNextEntry;
					}
				m_pTable[i] = NULL;
				}

			m_iCount = 0;
			}

		VALUE * const Find (const KEY &Key) const
			{
			Entry *pEntryFound = (Entry *)FindEntry((void *)&Key, MapKeyHashData(Key), MapKeyHashDataSize(Key));
			if (pEntryFound)
				return &pEntryFound->m_Value;
			else
				return NULL;
			}

		inline int GetCount (void) { return m_iCount; }

		const KEY &GetNext (CMapIterator &Iterator, VALUE **retpValue)
			{
			Entry *pEntryFound = (Entry *)CMapBase::GetNext(Iterator);
			ASSERT(pEntryFound);
			if (retpValue)
				*retpValue = &pEntryFound->m_Value;
			return pEntryFound->m_Key;
			}

		VALUE * const GetNext (CMapIterator &Iterator)
			{
			Entry *pEntryFound = (Entry *)CMapBase::GetNext(Iterator);
			ASSERT(pEntryFound);
			return &pEntryFound->m_Value;
			}

		VALUE * const Insert (const KEY &Key)
			{
			Entry *pNewEntry = new Entry(Key);
			InsertEntry(MapKeyHashData(Key), MapKeyHashDataSize(Key), (EntryBase *)pNewEntry);
			return &pNewEntry->m_Value;
			}

		bool HasMore (CMapIterator &Iterator)
			{
			return CMapBase::HasMore(Iterator);
			}

		void Insert (const KEY &Key, const VALUE &Value)
			{
			Entry *pNewEntry = new Entry(Key, Value);
			InsertEntry(MapKeyHashData(Key), MapKeyHashDataSize(Key), (EntryBase *)pNewEntry);
			}

		void Reset (CMapIterator &Iterator)
			{
			CMapBase::Reset(Iterator);
			}

	protected:
		virtual bool KeyEquals (void *pVoidKey, EntryBase *pEntry) const
			{
			return MapKeyEquals(*(KEY *)pVoidKey, ((Entry *)pEntry)->m_Key);
			}

	private:
		class Entry
			{
			public:
				Entry (const KEY &Key) :
						m_pNext(NULL),
						m_Key(Key)
					{ }

				Entry (const KEY &Key, const VALUE &Value) :
						m_pNext(NULL),
						m_Key(Key),
						m_Value(Value)
					{ }

				Entry *m_pNext;
				KEY m_Key;
				VALUE m_Value;
			};

		void Copy (const TMap<KEY, VALUE> &Src)
			{
			m_iCount = Src.m_iCount;

			for (int i = 0; i < m_iTableSize; i++)
				{
				Entry *pStartDest = NULL;
				Entry *pNextDest = NULL;
				Entry *pNext = (Entry *)Src.m_pTable[i];
				while (pNext)
					{
					Entry *pNew = new Entry(pNext->m_Key, pNext->m_Value);
					if (pNextDest)
						pNextDest->m_pNext = pNew;
					else
						pStartDest = pNew;

					pNextDest = pNew;
					pNext = pNext->m_pNext;
					}

				m_pTable[i] = (EntryBase *)pStartDest;
				}
			}
	};

//	Simple map classes

template <class VALUE> class TStringMap : public TMap<CString, VALUE> { };
template <class VALUE> class TIntMap : public TMap<int, VALUE> { };

//	LINKED-LIST CLASS ---------------------------------------------------------

template <class VALUE> class TLinkedList
	{
	public:
		TLinkedList (void) : m_pFirst(NULL), m_pLast(NULL)
			{
			}

		~TLinkedList (void)
			{
			DeleteAll();
			}

		void Delete (VALUE *pPos)
			{
			ASSERT(pPos);
			SNode *pEntry = (SNode *)pPos;
			
			//	Unlink it

			if (pEntry->pPrev)
				pEntry->pPrev->pNext = pEntry->pNext;
			else
				m_pFirst = pEntry->pNext;

			if (pEntry->pNext)
				pEntry->pNext->pPrev = pEntry->pPrev;
			else
				m_pLast = pEntry->pPrev;

			//	done

			delete pEntry;
			}

		void DeleteAll (void)
			{
			SNode *pEntry = m_pFirst;
			while (pEntry)
				{
				SNode *pDelete = pEntry;
				pEntry = pEntry->pNext;
				delete pDelete;
				}
			}

		inline VALUE *GetFirst (void) { return (VALUE *)m_pFirst; }
		inline VALUE *GetNext (VALUE *pPos) { return (VALUE *)((SNode *)pPos)->pNext; }

		void Insert (const VALUE &Entry, VALUE *pAt = m_pLast)
			{
			SNode *pInsertAt = (SNode *)pAt;
			if (pInsertAt == NULL)
				pInsertAt = m_pLast;

			//	Allocate the new node

			SNode *pNode = new SNode(Entry);

			//	Link it

			if (pInsertAt == NULL)
				{
				pNode->pNext = m_pFirst;
				pNode->pPrev = NULL;

				m_pFirst = pNode;
				if (m_pLast == NULL)
					m_pLast = pNode;
				}
			else
				{
				pNode->pNext = pInsertAt->pNext;
				pNode->pPrev = pInsertAt;
				pInsertAt->pNext = pNode;
				
				if (pInsertAt->pNext)
					pInsertAt->pNext->pPrev = pNode;
				else
					m_pLast = pNode;
				}
			}

	private:
		struct SNode
			{
			SNode (const VALUE &newValue) : Value(newValue) { }
				
			VALUE Value;

			SNode *pNext;
			SNode *pPrev;
			};

		SNode *m_pFirst;
		SNode *m_pLast;
	};

//	Other collections

class CAtomTable
	{
	public:
		CAtomTable (int iBaseAtom = 0);
		~CAtomTable (void);

		inline int AddAtom (const CString &sString) { return AddAtomWithData(sString, NULL, 0); }
		int AddAtomWithData (const CString &sString, void *pData, int iDataLen);
		bool FindAtom (const CString &sString, int *retiAtom);
		const CString &GetAtom (int iAtom);
		void *GetAtomData (int iAtom);
		inline int GetCount (void) { return m_iCount; }

	private:
		enum Constants
			{
			ctHashTableSize = 999,
			ctSegmentSize = 1000,
			};

		struct SEntry
			{
			int iNext;
			CString sStr;
			int iLen;					//	NOTE: iLen is negative, for compatibility with literal strings
			//	NULL-terminated string follows

			//	DWORD-padding
			//	Extra data follows

			//inline CString GetString (void) { return CString(((char *)this) + sizeof(SEntry), true); }
			inline const CString &GetString (void) { return sStr; }
			inline void *GetData (void) { return &((char *)this)[sizeof(SEntry) + AlignUp((-iLen) + 1, sizeof(DWORD))]; }
			};

		int AllocAtom (const CString &sString, int iHash, void *pExtraData = NULL, int iExtraDataLen = 0);
		SEntry *GetEntry (int iAtom);

		int m_iCount;
		int m_iBaseAtom;
		TArray<SEntry **> m_Backbone;
		int m_HashTable[ctHashTableSize];
	};

template <class VALUE> class TQueue
	{
	public:
		TQueue (int iSize = 100)
			{
			ASSERT(iSize > 0);
			m_iSize = iSize + 1;
			m_pArray = new VALUE [m_iSize];
			m_iHead = 0;
			m_iTail = 0;
			}

		~TQueue (void)
			{
			delete [] m_pArray;
			}

		inline VALUE &operator [] (int iIndex) const { return GetAt(iIndex); }

		void Dequeue (bool bDelete = false)
			{
			ASSERT(m_iHead != m_iTail);
			if (bDelete)
				m_pArray[m_iHead] = VALUE();

			m_iHead = (m_iHead + 1) % m_iSize;
			}

		void Enqueue (void)
			{
			if (((m_iTail + 1) % m_iSize) == m_iHead)
				throw CException(errOutOfMemory);

			m_iTail = (m_iTail + 1) % m_iSize;
			}

		void Enqueue (const VALUE &Value)
			{
			if (((m_iTail + 1) % m_iSize) == m_iHead)
				throw CException(errOutOfMemory);

			m_pArray[m_iTail] = Value;
			m_iTail = (m_iTail + 1) % m_iSize;
			}

		inline VALUE &GetAt (int iIndex) const
			{
			ASSERT(iIndex < GetCount());
			return m_pArray[(m_iHead + iIndex) % m_iSize];
			}

		int GetCount (void) const
			{
			if (m_iTail >= m_iHead)
				return (m_iTail - m_iHead);
			else
				return (m_iTail + m_iSize - m_iHead);
			}

		inline VALUE &Head (void) const
			{
			ASSERT(m_iHead != m_iTail);
			return m_pArray[m_iHead];
			}

		inline VALUE &Tail (void) const
			{
			ASSERT(m_iHead != m_iTail);
			return m_pArray[(m_iTail + m_iSize - 1) % m_iSize];
			}

	private:
		VALUE *m_pArray;
		int m_iSize;
		int m_iHead;
		int m_iTail;
	};

class CStackBase
	{
	protected:
		CStackBase (int iMaxSize);
		~CStackBase (void);

		void *GetDataAt (int iSP);
		void *PushData (int iDataSize);
		void *PopData (int iDataSize);

		void Commit (int iSize);

		char *m_pBlock;
		int m_iMaxSize;
		int m_iCommittedSize;

		int m_iSP;
	};

#pragma warning(disable:4291)			//	No need for a delete because we're placing object
template <class VALUE> class TStack : public CStackBase
	{
	public:
		TStack (int iMaxSize = 1000000) : CStackBase(iMaxSize)
			{
			}

		~TStack (void)
			{
			VALUE *pElement = (VALUE *)m_pBlock;
			for (int i = 0; i < GetCount(); i++, pElement++)
				pElement->VALUE::~VALUE();
			}

		inline VALUE &operator [] (int iIndex) const 
			{
			return *(VALUE *)(m_pBlock + (iIndex * sizeof(VALUE)));
			}

		int GetCount (void) const
			{
			return (m_iSP / sizeof(VALUE));
			}

		int GetSP (void) const
			{
			return (m_iSP / sizeof(VALUE));
			}

		void Pop (void)
			{
			VALUE *pElement = (VALUE *)PopData(sizeof(VALUE));
			pElement->VALUE::~VALUE();
			}

		void Push (const VALUE &Element)
			{
			void *pPos = PushData(sizeof(VALUE));
			VALUE *pElement = new(pPos) VALUE(Element);
			}

		void SetSP (int iSP)
			{
			int i;
			int iDiff = iSP - (m_iSP / sizeof(VALUE));
			if (iDiff > 0)
				{
				VALUE *pElement = (VALUE *)PushData(iDiff * sizeof(VALUE));
				for (i = 0; i < iDiff; i++, pElement++)
					new(pElement) VALUE();
				}
			else if (iDiff < 0)
				{
				iDiff = -iDiff;
				VALUE *pElement = (VALUE *)PopData(iDiff * sizeof(VALUE));
				for (i = 0; i < iDiff; i++, pElement++)
					pElement->VALUE::~VALUE();
				}
			}

		VALUE &Top (void)
			{
			ASSERT(m_iSP > 0);
			VALUE *pTop = (VALUE *)(m_pBlock + m_iSP - sizeof(VALUE));
			return *pTop;
			}
	};
#pragma warning(default:4291)

//	CONCURRENCY CLASSES -------------------------------------------------------

class CCriticalSection
	{
	public:
		CCriticalSection (void) { ::InitializeCriticalSection(&m_cs); }
		~CCriticalSection (void) { ::DeleteCriticalSection(&m_cs); }

		inline void Lock (void) { ::EnterCriticalSection(&m_cs); }
		inline void Unlock (void) { ::LeaveCriticalSection(&m_cs); }

	private:
		CRITICAL_SECTION m_cs;
	};

class CSmartLock
	{
	public:
		CSmartLock(CCriticalSection &cs) : m_cs(cs) { m_cs.Lock(); }
		~CSmartLock (void) { m_cs.Unlock(); }

	private:
		CCriticalSection &m_cs;
	};

//	MEMORY BLOCKS AND FILE STREAMS --------------------------------------------

class IMemoryBlock
	{
	public:
		virtual ~IMemoryBlock (void) { }
		virtual int GetLength (void) = 0;
		virtual char *GetPointer (void) = 0;
	};

class IByteStream
	{
	public:
		virtual ~IByteStream (void) { }
		virtual int GetPos (void) = 0;
		virtual int GetStreamLength (void) = 0;
		virtual int Read (void *pData, int iLength) = 0;
		virtual void Seek (int iPos, bool bFromEnd = false) = 0;
		virtual int Write (void *pData, int iLength) = 0;
	};

class ITextLog
	{
	public:
		virtual void Log (char *pLine, ...) = 0;
	};

class CFileBlock : public IMemoryBlock
	{
	public:
		CFileBlock (const CString &sFilename);
		virtual ~CFileBlock (void);

		//	IMemoryBlock virtuals
		virtual int GetLength (void);
		virtual char *GetPointer (void);

	private:
		void OpenFile (void);

		//	Hide assignment and copy constructor
		CFileBlock &operator= (const CFileBlock &File);
		CFileBlock (const CFileBlock &File);

		CString m_sFilename;
		HANDLE m_hFile;
		HANDLE m_hFileMap;
		char *m_pFile;
		DWORD m_dwFileSize;
	};

class CFileStream : public IByteStream
	{
	public:
		CFileStream (const CString &sFilename, bool bUnique = false);
		~CFileStream (void);

		//	IByteStream virtuals
		virtual int GetPos (void);
		virtual int GetStreamLength (void);
		virtual int Read (void *pData, int iLength);
		virtual void Seek (int iPos, bool bFromEnd = false);
		virtual int Write (void *pData, int iLength);

	private:
		void OpenFile (void);

		CString m_sFilename;
		bool m_bUnique;

		HANDLE m_hFile;
	};

class CBuffer : public IMemoryBlock,
		public IByteStream
	{
	public:
		CBuffer (int iSize);
		CBuffer (const CString &sString, int iPos = 0, int iLength = -1);
		virtual ~CBuffer (void);

		//	IMemoryBlock virtuals
		virtual int GetLength (void) { return m_iLength; }
		virtual char *GetPointer (void) { return m_pBuffer; }

		//	IByteStream virtuals
		virtual int GetPos (void) { return m_pPointer - m_pBuffer; }
		virtual int GetStreamLength (void) { return m_iLength; }
		virtual int Read (void *pData, int iLength);
		virtual void Seek (int iPos, bool bFromEnd = false);
		virtual int Write (void *pData, int iLength);

	private:
		//	Hide copy constructor and equals operator
		CBuffer (const CBuffer &Unused);
		CBuffer &operator= (const CBuffer &Unused);

		int m_iLength;

		char *m_pBuffer;
		char *m_pPointer;

		bool m_bAllocated;
	};

class CWORM : public IMemoryBlock,
		public IByteStream
	{
	public:
		CWORM (int iMaxSize = 1000000);
		virtual ~CWORM (void);

		//	IMemoryBlock virtuals
		virtual int GetLength (void) { return m_iCurrentSize; }
		virtual char *GetPointer (void) { return m_pBlock; }

		//	IByteStream virtuals
		virtual int GetPos (void) { return m_pPointer - m_pBlock; }
		virtual int GetStreamLength (void) { return m_iCurrentSize; }
		virtual int Read (void *pData, int iLength);
		virtual void Seek (int iPos, bool bFromEnd = false);
		virtual int Write (void *pData, int iLength);

	private:
		void Commit (int iSize);

		int m_iMaxSize;
		int m_iCommittedSize;
		int m_iCurrentSize;

		char *m_pBlock;
		char *m_pPointer;
	};

//	DATA FILE CLASSES ---------------------------------------------------------

class CDataFile
	{
	public:
		class Iterator
			{
			private:
				int iPos;

			friend class CDataFile;
			};

		CDataFile (void) : m_pFile(NULL) { }
		CDataFile (const CDataFile &sDataFile);
		~CDataFile (void);

		CDataFile &operator= (const CDataFile &sDataFile);

		void Close (void);
		static void Create (const CString &sFilename, int iBlockSize = 64);
		static CDataFile Open (const CString &sFilename);

		int AddEntry (const CString &Data);
		int AllocEntry (void);
		void DeleteEntry (int iID);
		void Flush (void);
		int GetDefaultEntry (void);
		int GetEntryLength (int iID);
		int GetNext (Iterator &Pos);
		bool HasMore (Iterator &Pos);
		CString ReadEntry (int iID);
		void ReadEntry (int iID, CString *retsData);
		void ResizeEntry (int iID, int iSize);
		void Reset(Iterator &Pos);
		void SetDefaultEntry (int iID);
		void WriteEntry (int iID, const CString &Data);

		static bool TEST (ITextLog &Output);

	private:
		struct BlockRun
			{
			int iBlock;							//	First block for this entry (or next
												//		free entry index)
			int iSize;							//	Size of entry in bytes
			};

		class CBlockRunTable
			{
			public:
				CBlockRunTable (void);

				int AddRun (const BlockRun &Run);
				void DeleteRun (int iID);
				bool FindAdjacentRuns (int iBlock, int iSize, int *retiBefore, int *retiAfter);
				int FindRunByBlock (int iBlock);
				int FindRunBySize (int iSize);
				inline int GetCount (void) { return m_Table.GetCount(); }
				inline BlockRun &GetRun (int iID) { return m_Table[iID]; }
				inline bool IsRunValid (int iID) { return m_Table[iID].iSize != -1; }

				int GetSaveSize (void);
				void LoadFromString (const CString &sData);
				CString SaveToString (void);

			private:
				TArray<BlockRun> m_Table;
				int m_iFirstFree;
			};

		struct DataFileInternals
			{
			int iRefCount;

			CString sFilename;					//	Filename
			HANDLE hFile;						//	OS file handle

			int iBlockSize;						//	Size of each block
			int iBlockCount;					//	Number of blocks in file
			int iDefaultEntry;					//	Default entry
			int iFreeTableEntry;				//	Free table entry

			CBlockRunTable EntryTable;			//	Table of entries
			CBlockRunTable FreeTable;			//	Table of free blocks

			bool bHeaderModified;				//	Header changes need to be saved to disk
			bool bEntryTableModified;			//	Entry table needs to be saved
			bool bFreeTableModified;			//	Free table needs to be saved
			};

		int AllocBlocks (int iID, int iSize);
		void FreeBlocks (int iBlock, int iSize);
		inline CException InvalidFileError (void) { return InvalidFileError(m_pFile->sFilename); }
		static CException InvalidFileError (const CString &sFilename);
		int GetBlockFilePos (int iBlock);
		void ReadEntryData (int iBlock, int iSize, CString *retsData);
		void WriteEntryInt (int iID, char *pData, int iLength);
		void WriteEntryData (int iBlock, char *pData, int iLength);
		inline void WriteEntryData (int iBlock, const CString &Data) 
			{ WriteEntryData(iBlock, Data.GetASCIIZ(), Data.GetLength()); }
		void WriteEntryHeader (int iBlock, int iID, int iSize);

		DataFileInternals *m_pFile;
	};

//	DIB FUNCTIONS

enum dibBitPatternEnum
	{
	dib32bits,
	dib24bits,
	dib16bits555,
	dib16bits565,
	dib8bits,
	dib8bitsGrayscale,
	dib4bits,
	dibMono,
	};

struct dibInfoStruct
	{
	int cxWidth;
	int cyHeight;
	dibBitPatternEnum iBitPattern;
	BYTE *pBase;
	int iStride;

	int iColorTableSize;
	COLORREF ColorTable[256];
	};

void dibGetInfo (HBITMAP hDIB, dibInfoStruct *dibInfo);
void dibGetInfo (BITMAPINFO *pbmi, dibInfoStruct *dibInfo);
inline BYTE *dibGetFirstLine (const dibInfoStruct &di) { return di.pBase; }
inline BYTE *dibGetEndLine (const dibInfoStruct &di) { return di.pBase + (di.iStride * di.cyHeight); }
inline BYTE *dibGetNextLine (const dibInfoStruct &di, BYTE *pLine) { return pLine + di.iStride; }

//	INITIALIZATION FUNCTIONS

void elementsCleanUp (void);
bool elementsInit (void);

//	GDI FUNCTIONS

void gdiConvertToDDB (HBITMAP hDIB, HPALETTE hPalette, HBITMAP *rethBitmap);
void gdiCreate16bitDIB (int cxWidth, int cyHeight, HBITMAP *rethBitmap, WORD **retpPixel);
void gdiCreate32bitDIB (int cxWidth, int cyHeight, HBITMAP *rethBitmap, DWORD **retpPixel);
void gdiFillRect (HDC hDC, const RECT &rcRect, COLORREF Color);
int gdiGetFontHeight (HDC hDC, int iPointSize);

//	MATH FUNCTIONS

const double g_Pi = 3.14159265358979323846;			//	Pi

void mathFFT (int iSamples,
			  double *pRealIn,
			  double *pImagIn,
			  double *pRealOut,
			  double *pImagOut,
			  bool bInverse = false);
int mathNearestPowerOf2 (int x);
int mathSqrt (int x);
int mathRandom (int iFrom, int iTo);

//	PATH FUNCTIONS

CString pathAbsolutePath (const CString &sPath);
CString pathAddComponent (const CString &sPath, const CString &sComponent);
bool pathCreate (const CString &sPath);
bool pathExists (const CString &sPath);
bool pathIsAbsolute (const CString &sPath);
CString pathGetExecutablePath (HINSTANCE hInstance = NULL);
CString pathGetExtension (const CString &sPath);
CString pathGetFilename (const CString &sPath);
CString pathGetPath (const CString &sPath);
CString pathRelativePath (const CString &sRoot, const CString &sPath);
CString pathStripExtension (const CString &sPath);

//	SYSTEM FUNCTIONS

HANDLE sysCreateThread (LPTHREAD_START_ROUTINE pfStart, LPVOID pData);
DWORD sysGetAPIFlags (void);
inline bool sysCtrlKeyDown (void) { return ((::GetKeyState(VK_CONTROL) & 0x8000) ? true : false); }
inline bool sysShiftKeyDown (void) { return ((::GetKeyState(VK_SHIFT) & 0x8000) ? true : false); }

//	UI FUNCTIONS

bool uiChooseColor (HWND hParent,
					COLORREF *ioColor);

bool uiGetOpenFilename (HWND hParent,
						const CString &sExtensions,
						const CString &sFolder,
						bool bAllowMultipleFiles,
						CString *retsFilenames);

bool uiGetSaveFilename (HWND hParent,
						const CString &sExtensions,
						const CString &sFolder,
						CString *retsFilename);

//	UTILITY FUNCTIONS

CException errOSError (void);
CException errOSError (const CString &sError);

DWORD utlHashFunctionCase (BYTE *pKey, int iKeyLen);

void utlMemSet (void *pDest, int Count, char Value);
void utlMemCopy (void *pSource, void *pDest, int iCount);
bool utlMemCompare (void *pSource, void *pDest, int iCount);

#endif