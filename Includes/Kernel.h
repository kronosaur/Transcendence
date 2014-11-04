//	Kernel.h
//
//	Kernel definitions.

#ifndef INCL_KERNEL
#define INCL_KERNEL

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						

#define _CRT_RAND_S
#include <stdlib.h>
#include <windows.h>

//	Debugging defines

#ifdef DEBUG
//#define DEBUG_MEMORY_LEAKS
//#define DEBUG_STRING_LEAKS
#endif

#ifdef DEBUG_MEMORY_LEAKS
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif

//	HACK: Declare _alloca so that we don't have to include malloc.h
extern "C" void *          __cdecl _alloca(size_t);

//	Define ASSERT macro, if necessary

#ifndef ASSERT
#ifdef _DEBUG
#define ASSERT(exp)						\
			{							\
			if (!(exp))					\
				DebugBreak();			\
			}
#else
#define ASSERT(exp)
#endif
#endif

#define INLINE_DECREF				TRUE

//	Error definitions

typedef DWORD ALERROR;

#ifndef NOERROR
#define NOERROR									0
#endif

#define ERR_FAIL								1	//	Generic failure
#define ERR_MEMORY								2	//	Out of memory
#define ERR_ENDOFFILE							3	//	Read past end of file
#define ERR_CANCEL								4	//	User canceled operation
#define ERR_NOTFOUND							5	//	Entry not found
#define ERR_FILEOPEN							6	//	Unable to open file
#define ERR_CLASSNOTFOUND						7	//	Constructor for class not found
#define ERR_OUTOFDATE							8	//	Not latest version
#define ERR_MORE								9	//	More needed
#define ERR_WIN32_EXCEPTION						10	//	Win32 exception
#define ERR_OUTOFROOM							11	//	Unable to insert

#define ERR_MODULE						0x00010000	//	First module error message (see GlobalErr.h)
#define ERR_APPL						0x01000000	//	First application error message
#define ERR_FLAG_DISPLAYED				0x80000000	//	Error message already displayed

inline BOOL ErrorWasDisplayed (ALERROR error) { return (error & ERR_FLAG_DISPLAYED) ? TRUE : FALSE; }
inline ALERROR ErrorSetDisplayed (ALERROR error) { return error | ERR_FLAG_DISPLAYED; }
inline ALERROR ErrorCode (ALERROR error) { return error & ~ERR_FLAG_DISPLAYED; }

class CException
	{
	public:
		CException (ALERROR error) : m_error(error) { }

	private:
		int m_error;
	};

//	Miscellaneous macros

inline int Absolute (int iValue) { return (iValue < 0 ? -iValue : iValue); }
inline double Absolute (double rValue) { return (rValue < 0.0 ? -rValue : rValue); }
inline int AlignUp (int iValue, int iGranularity) { return ((iValue + (iGranularity - 1)) / iGranularity) * iGranularity; }
inline int ClockMod (int iValue, int iDivisor) { int iResult = (iValue % iDivisor); return (iResult < 0 ? iResult + iDivisor : iResult); }
inline BOOL IsShiftDown (void) { return (GetAsyncKeyState(VK_SHIFT) & 0x8000) ? TRUE : FALSE; }
inline BOOL IsControlDown (void) { return (GetAsyncKeyState(VK_CONTROL) & 0x8000) ? TRUE : FALSE; }
inline int Sign (int iValue) { return (iValue == 0 ? 0 : (iValue > 0 ? 1 : -1)); }
template <class VALUE> VALUE Clamp (VALUE x, VALUE a, VALUE b)
	{
	return (x < a ? a : (x > b ? b : x));
	}
template <class VALUE> VALUE Max (VALUE a, VALUE b)
	{
	return (a > b ? a : b);
	}
template <class VALUE> VALUE Min (VALUE a, VALUE b)
	{
	return (a < b ? a : b);
	}
template <class VALUE> void Swap (VALUE &a, VALUE &b)
	{
	VALUE temp = a;
	a = b;
	b = temp;
	}

inline int RectHeight(RECT *pRect) { return pRect->bottom - pRect->top; }
inline int RectHeight(const RECT &Rect) { return Rect.bottom - Rect.top; }
inline int RectWidth(RECT *pRect) { return pRect->right - pRect->left; }
inline int RectWidth(const RECT &Rect) { return Rect.right - Rect.left; }
inline BOOL RectsIntersect(RECT &R1, RECT &R2)
	{
	return (R1.right >= R2.left)
			&& (R1.left < R2.right)
			&& (R1.bottom >= R2.top)
			&& (R1.top < R2.bottom);
	}
inline BOOL RectEncloses (RECT *pR1, RECT *pR2)
	{
	return (pR1->left <= pR2->left)
			&& (pR1->right >= pR2->right)
			&& (pR1->top <= pR2->top)
			&& (pR1->bottom >= pR2->bottom);
	}
inline bool RectEquals (const RECT &rc1, const RECT &rc2)
	{
	return (rc1.left == rc2.left
			&& rc1.top == rc2.top
			&& rc1.right == rc2.right
			&& rc1.bottom == rc2.bottom);
	}
inline void RectInit (RECT *pRect)
	{
	pRect->left = 0;
	pRect->top = 0;
	pRect->right = 0;
	pRect->bottom = 0;
	}

const int MAX_SHORT =					32767;

//	API flags

#define API_FLAG_MASKBLT				0x00000001	//	MaskBlt is available
#define API_FLAG_WINNT					0x00000002	//	Running on Windows NT
#define API_FLAG_DWM					0x00000004	//	Desktop Window Manager running (Vista or Win7)

//	Forward class definitions

class CArchiver;
class CObject;
class CUnarchiver;
class CString;
class CStructArray;
class CIDTable;
class IReadStream;
class IWriteStream;

//	Templates

#include "TArray.h"
#include "TMap.h"
#include "TQueue.h"
#include "TStack.h"

//	TimeDate classes

#define SECONDS_PER_DAY					(60 * 60 * 24)

class CTimeDate
	{
	public:
		enum Constants
			{
			Now,
			Today,
			};

		CTimeDate (void);
		CTimeDate (Constants Init);
		CTimeDate (const SYSTEMTIME &Time);
		CTimeDate (int iDaysSince1AD, int iMillisecondsSinceMidnight);

		inline operator SYSTEMTIME () const { return m_Time; }

		inline int Year (void) const { return m_Time.wYear; }
		inline int Month (void) const { return m_Time.wMonth; }
		inline int Day (void) const { return m_Time.wDay; }
		inline int Hour (void) const { return m_Time.wHour; }
		inline int Minute (void) const { return m_Time.wMinute; }
		inline int Second (void) const { return m_Time.wSecond; }
		inline int Millisecond (void) const { return m_Time.wMilliseconds; }

		int Compare (const CTimeDate &Src) const;
		int DayOfWeek (void) const;
		int DaysSince1AD (void) const;
		CString Format (const CString &sFormat) const;
		int MillisecondsSinceMidnight (void) const;
		bool Parse (const CString &sFormat, const CString &sValue, CString *retsError = NULL);
		CTimeDate ToLocalTime (void) const;

	private:
		SYSTEMTIME m_Time;
	};

class CTimeSpan
	{
	public:
		CTimeSpan (void);
		CTimeSpan (int iMilliseconds);
		CTimeSpan (int iDays, int iMilliseconds);

		static bool Parse (const CString &sValue, CTimeSpan *retValue);

		inline int Days (void) const { return (int)m_Days; }
		inline int Seconds (void) const { return (SECONDS_PER_DAY * m_Days) + (m_Milliseconds / 1000); }
		inline int Milliseconds (void) const { return (SECONDS_PER_DAY * 1000 * m_Days) + m_Milliseconds; }
		inline int MillisecondsSinceMidnight (void) const { return (int)m_Milliseconds; }

		CString Encode (void) const;
		CString Format (const CString &sFormat) const;
		inline bool IsBlank (void) const { return (m_Days == 0 && m_Milliseconds == 0); }
		void ReadFromStream (IReadStream *pStream);
		void WriteToStream (IWriteStream *pStream) const;

	private:
		static bool ParsePartial (char *pPos, DWORD *retdwDays, DWORD *retdwMilliseconds, char **retpPos);

		DWORD m_Days;
		DWORD m_Milliseconds;
	};

const CTimeSpan operator+ (const CTimeSpan &op1, const CTimeSpan &op2);
const CTimeSpan operator- (const CTimeSpan &op1, const CTimeSpan &op2);

CTimeDate timeAddTime (const CTimeDate &StartTime, const CTimeSpan &Addition);
CTimeSpan timeSpan (const CTimeDate &StartTime, const CTimeDate &EndTime);
CTimeDate timeSubtractTime (const CTimeDate &StartTime, const CTimeSpan &Subtraction);
bool timeIsLeapYear (int iYear);

//	High-performance timer classes --------------------------------------------

class CPeriodicWaiter
	{
	public:
		CPeriodicWaiter (DWORD dwPeriod);

		void Wait (void);

	private:
		DWORD m_dwPeriod;
		LONGLONG m_LastCounter;

		LONGLONG m_PCFreq;
		LONGLONG m_PCCountsPerPeriod;
	};

//	Object class ID definitions

typedef DWORD OBJCLASSID;

#define OBJCLASS_MODULE_MASK					0xFFF00000
#define OBJCLASS_MODULE_SHIFT					20

#define OBJCLASS_MODULE_KERNEL					0
#define OBJCLASS_MODULE_APPLICATION				1
#define OBJCLASS_MODULE_GLOBAL					2
#define OBJCLASS_MODULE_COUNT					3

inline OBJCLASSID MakeOBJCLASSIDExt (int iModule, int iID) { return (((DWORD)iModule) << OBJCLASS_MODULE_SHIFT) + (DWORD)iID; }
inline OBJCLASSID MakeOBJCLASSID (int iID) { return MakeOBJCLASSIDExt(OBJCLASS_MODULE_APPLICATION, iID); }
inline OBJCLASSID MakeGlobalOBJCLASSID (int iID) { return MakeOBJCLASSIDExt(OBJCLASS_MODULE_GLOBAL, iID); }
inline int OBJCLASSIDGetID (OBJCLASSID ObjID) { return (int)(ObjID & ~OBJCLASS_MODULE_MASK); }
inline int OBJCLASSIDGetModule (OBJCLASSID ObjID) { return (int)((ObjID & OBJCLASS_MODULE_MASK) >> OBJCLASS_MODULE_SHIFT); }

//	Object data description

#define DATADESC_OPCODE_STOP				0	//	No more entries
#define DATADESC_OPCODE_INT					1	//	32-bit integer (iCount valid)
#define DATADESC_OPCODE_REFERENCE			2	//	Reference to memory location or object (iCount valid)
#define DATADESC_OPCODE_ALLOC_OBJ			3	//	Pointer to owned object (derived from CObject)
#define DATADESC_OPCODE_EMBED_OBJ			4	//	Embedded object (derived from CObject)
#define DATADESC_OPCODE_ZERO				5	//	32-bit of zero-init data (iCount valid)
#define DATADESC_OPCODE_VTABLE				6	//	This is a vtable (which is initialized by new) (iCount is valid)
#define DATADESC_OPCODE_ALLOC_SIZE32		7	//	Number of 32-bit words allocated in the following memory block
#define DATADESC_OPCODE_ALLOC_MEMORY		8	//	Block of memory; previous must be ALLOC_SIZE

#define DATADESC_FLAG_CUSTOM		0x00000001	//	Object handles saving this part

typedef struct
	{
	int iOpCode:8;								//	Op-code
	int iCount:8;								//	Count
	DWORD dwFlags:16;							//	Miscellaneous flags
	} DATADESCSTRUCT, *PDATADESCSTRUCT;

//	Abstract object class

class IObjectClass
	{
	public:
		IObjectClass (OBJCLASSID ObjID, PDATADESCSTRUCT pDataDesc) : m_ObjID(ObjID), m_pDataDesc(pDataDesc) { }

		inline PDATADESCSTRUCT GetDataDesc (void) { return m_pDataDesc; }
		inline OBJCLASSID GetObjID (void) { return m_ObjID; }
		virtual CObject *Instantiate (void) = 0;
		virtual int GetObjSize (void) = 0;

	private:
		OBJCLASSID m_ObjID;
		PDATADESCSTRUCT m_pDataDesc;
	};

//	Base object class

class CObject
	{
	public:
		CObject (IObjectClass *pClass);
		virtual ~CObject (void);

		CObject *Copy (void);
		inline IObjectClass *GetClass (void) { return m_pClass; }
		static bool IsValidPointer (CObject *pObj);
		ALERROR Load (CUnarchiver *pUnarchiver);
		ALERROR LoadDone (void);
		ALERROR Save (CArchiver *pArchiver);

		static ALERROR Flatten (CObject *pObject, CString *retsData);
		static ALERROR Unflatten (CString sData, CObject **retpObject);

	protected:
		virtual void CopyHandler (CObject *pOriginal) { }
		virtual ALERROR LoadCustom (CUnarchiver *pUnarchiver, BYTE *pDest) { return NOERROR; }
		virtual ALERROR LoadDoneHandler (void) { return NOERROR; }
		virtual ALERROR LoadHandler (CUnarchiver *pUnarchiver);
		virtual LPVOID MemAlloc (int iSize) { return (BYTE *)HeapAlloc(GetProcessHeap(), 0, iSize); }
		virtual void MemFree (LPVOID pMem) { HeapFree(GetProcessHeap(), 0, pMem); }
		virtual ALERROR SaveCustom (CArchiver *pArchiver, BYTE *pSource) { return NOERROR; }
		virtual ALERROR SaveHandler (CArchiver *pArchiver);

	private:
		CObject *Clone (void) { return m_pClass->Instantiate(); }
		BOOL CopyData (PDATADESCSTRUCT pPos, BYTE **iopSource, BYTE **iopDest);
		PDATADESCSTRUCT DataDescNext (PDATADESCSTRUCT pPos);
		PDATADESCSTRUCT DataDescStart (void);
		BYTE *DataStart (void);
		void VerifyDataDesc (void);

		IObjectClass *m_pClass;
	};

//	Factory for creating objects

class CObjectClassFactory
	{
	public:
		static CObject *Create (OBJCLASSID ObjID);
		static IObjectClass *GetClass (OBJCLASSID ObjID);
		static void NewClass (IObjectClass *pClass);
	};

//	Template for object classes

template <class T>
class CObjectClass : public IObjectClass
	{
	public:
		CObjectClass (OBJCLASSID ObjID, PDATADESCSTRUCT pDataDesc = NULL)
				: IObjectClass(ObjID, pDataDesc)
				{ CObjectClassFactory::NewClass(this); }
 
		virtual CObject *Instantiate (void) { return new T; }
		virtual int GetObjSize (void) { return sizeof(T); }
	};

//	Synchronization -----------------------------------------------------------

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

//	CINTDynamicArray. Implementation of a dynamic array.
//	(NOTE: To save space, this class does not have a virtual
//	destructor. Do not sub-class this class without taking that into account).

class CINTDynamicArray
	{
	public:
		CINTDynamicArray (void);
		CINTDynamicArray (HANDLE hHeap);
		~CINTDynamicArray (void);

		inline ALERROR Append (BYTE *pData, int iLength, int iAllocQuantum)
			{ return Insert(-1, pData, iLength, iAllocQuantum); }
		ALERROR Delete (int iOffset, int iLength);
		inline ALERROR DeleteAll (void) { return Delete(0, m_iLength); }
		inline int GetLength (void) const { return m_iLength; }
		inline void SetLength (int iLength) { m_iLength = iLength; }
		inline BYTE *GetPointer (int iOffset) const { return (m_pArray ? m_pArray + iOffset : NULL); }
		ALERROR Insert (int iOffset, BYTE *pData, int iLength, int iAllocQuantum);
		ALERROR Resize (int iNewSize, BOOL bPreserve, int iAllocQuantum);

	private:
		int m_iLength;							//	Length of the array in bytes
		int m_iAllocSize;						//	Allocated size of the array
		HANDLE m_hHeap;							//	Heap to use
		BYTE *m_pArray;							//	Array data
	};

//	Link list classes

template <class TYPE> class TListNode
	{
	public:
		TListNode (void) : m_pNext(NULL) { }
		virtual ~TListNode (void) { }

		int GetCount (void)
			{
			int iCount = 0;
			TListNode<TYPE> *pNext = m_pNext;
			while (pNext)
				{
				iCount++;
				pNext = pNext->m_pNext;
				}
			return iCount;
			}

		inline TYPE *GetNext (void) { return (TYPE *)m_pNext; }

		void Insert (TListNode<TYPE> *pNewNode)
			{
			pNewNode->m_pNext = m_pNext;
			m_pNext = pNewNode;
			}

		void Remove (TListNode<TYPE> *pNodeToRemove)
			{
			TListNode<TYPE> *pNext = m_pNext;
			while (pNext)
				{
				if (pNext == pNodeToRemove)
					{
					RemoveNext();
					break;
					}
				pNext = pNext->m_pNext;
				}
			}

	private:
		void RemoveNext (void)
			{
			TListNode<TYPE> *pDelete = m_pNext;
			m_pNext = m_pNext->m_pNext;
			delete pDelete;
			}

		TListNode<TYPE> *m_pNext;
	};

//	CIntArray. Implementation of a dynamic array of integers

class CIntArray : public CObject
	{
	public:
		CIntArray (void);
		virtual ~CIntArray (void);

		CIntArray &operator= (const CIntArray &Obj);

		ALERROR AppendElement (int iElement, int *retiIndex = NULL);
		inline ALERROR CollapseArray (int iPos, int iCount) { return RemoveRange(iPos, iPos + iCount - 1); }
		ALERROR ExpandArray (int iPos, int iCount);
		int FindElement (int iElement) const;
		int GetCount (void) const;
		int GetElement (int iIndex) const;
		ALERROR InsertElement (int iElement, int iPos, int *retiIndex);
		ALERROR InsertRange (CIntArray *pList, int iStart, int iEnd, int iPos);
		ALERROR MoveRange (int iStart, int iEnd, int iPos);
		ALERROR Set (int iCount, int *pData);
		ALERROR RemoveAll (void);
		ALERROR RemoveElement (int iPos) { return RemoveRange(iPos, iPos); }
		ALERROR RemoveRange (int iStart, int iEnd);
		void ReplaceElement (int iPos, int iElement);
		void Shuffle (void);

	private:
		int m_iAllocSize;					//	Number of integers allocated
		int *m_pData;						//	Pointer to integer array
		int m_iLength;						//	Number of integers used
	};

#ifdef LATER
class CSet : public CObject
	{
	public:
		CSet (void);
		virtual ~CSet (void);

		virtual ALERROR AddElement (DWORD dwElement);
		virtual int GetCount (void);
		virtual ALERROR GetAllElement (CIntArray *retpArray);
		virtual BOOL IsMember (DWORD dwElement);
		virtual ALERROR RemoveElement (DWORD dwElement);
		virtual void RemoveAll (void);

	private:
		const int RUN_LENGTH = 128;

		class CRun
			{
			int iCount;
			DWORD Bits[RUN_LENGTH / sizeof(DWORD)];
			};

		CStructArray m_Runs;				//	Run array
		int m_iCount;						//	Number of elements
	};
#endif

//	CObjectArray. Implements a dynamic array of objects

class CObjectArray : public CObject
	{
	public:
		CObjectArray (void);
		CObjectArray (BOOL bOwned);
		virtual ~CObjectArray (void);

		ALERROR AppendObject (CObject *pObj, int *retiIndex = NULL)
				{ return m_Array.AppendElement((int)pObj, retiIndex); }
		CObject *DetachObject (int iIndex);
		int FindObject (CObject *pObj);
		int GetCount (void) const { return m_Array.GetCount(); }
		inline CObject *GetObject (int iIndex) const
				{ return (CObject *)m_Array.GetElement(iIndex); }
		ALERROR InsertObject (CObject *pObj, int iPos, int *retiIndex)
				{ return m_Array.InsertElement((int)pObj, iPos, retiIndex); }
		ALERROR Set (int iCount, CObject **pData)
				{ return m_Array.Set(iCount, (int *)pData); }
		void RemoveAll (void);
		void RemoveObject (int iPos);
		void ReplaceObject (int iPos, CObject *pObj, bool bDelete = true);

	protected:
		virtual void CopyHandler (CObject *pOriginal);
		virtual ALERROR LoadDoneHandler (void);
		virtual ALERROR LoadHandler (CUnarchiver *pUnarchiver);
		virtual ALERROR SaveHandler (CArchiver *pArchiver);

	private:
		BOOL m_bOwned;
		CIntArray m_Array;
	};

//	CString. Implementation of a standard string class

#define LITERAL(str)		((CString)(str))
#define CONSTLIT(str)		(CString(str, sizeof(str)-1, TRUE))

struct SConstString
	{
	int iLen;
	char *pszString;
	};

#define CONSTDEF(str)		((int)(sizeof(str) - 1)), str
#define CONSTDEFS(str)		{ ((int)(sizeof(str) - 1)), str }
#define CONSTUSE(constEntry)	CString((constEntry).pszString, (constEntry).iLen, true)
extern const CString NULL_STR;

class CString : public CObject
	{
	public:
		enum CapitalizeOptions
			{
			capFirstLetter,
			};

		CString (void);
		CString (const char *pString);
		CString (char *pString, int iLength);
		CString (char *pString, int iLength, BOOL bExternal);
		virtual ~CString (void);

		CString (const CString &pString);
		CString &operator= (const CString &pString);
		inline operator LPSTR () const { return GetASCIIZPointer(); }
		bool operator== (const CString &sValue) const;

		char *GetASCIIZPointer (void) const;
		ALERROR Append (const CString &sString);
		void Capitalize (CapitalizeOptions iOption);
		int GetLength (void) const;
		char *GetPointer (void) const;
		char *GetWritePointer (int iLength);
		inline bool IsBlank (void) const { return (GetLength() == 0); }
		void ReadFromStream (IReadStream *pStream);
		ALERROR Transcribe (const char *pString, int iLen);
		void Truncate (int iLength);
		void WriteToStream (IWriteStream *pStream) const;

		//	These are used internally only

		static void INTStringCleanUp (void);
		static ALERROR INTStringInit (void);

		//	These are used for custom string arrays

		static void *INTCopyStorage (void *pvStore);
		static void *INTGetStorage (const CString &sString);
		static void INTFreeStorage (void *pStore);
		static CString INTMakeString (void *pvStore);
		static void INTSetStorage (CString &sString, void *pStore);
		void INTTakeStorage (void *pStore);

		//	Debugging APIs

#ifdef DEBUG_STRING_LEAKS
		static int DebugGetStringCount (void);
		static void DebugMark (void);
		static void DebugOutputLeakedStrings (void);
#endif

	protected:
		virtual void CopyHandler (CObject *pOriginal);
		virtual ALERROR LoadHandler (CUnarchiver *pUnarchiver);
		virtual ALERROR SaveHandler (CArchiver *pArchiver);

	private:
		struct STORESTRUCT
			{
			int iRefCount;
			int iAllocSize;				//	If negative, this is a read-only external allocation
			int iLength;
			char *pString;
#ifdef DEBUG_STRING_LEAKS
			int iMark;
#endif
			};
		typedef struct STORESTRUCT *PSTORESTRUCT;

		CString (void *pStore, bool bDummy);

		static void AddToFreeList (PSTORESTRUCT pStore, int iSize);
		PSTORESTRUCT AllocStore (int iSize, BOOL bAllocString);
#ifdef INLINE_DECREF
		inline void DecRefCount (void)
			{
			if (m_pStore && (--m_pStore->iRefCount) == 0)
				FreeStore(m_pStore);
			}
#else
		void DecRefCount (void);
#endif

		static void FreeStore (PSTORESTRUCT pStore);
		inline void IncRefCount (void) { if (m_pStore) m_pStore->iRefCount++; }
		inline BOOL IsExternalStorage (void) { return (m_pStore->iAllocSize < 0 ? TRUE : FALSE); }
		BOOL Size (int iLength, BOOL bPreserveContents);

		static void InitLowerCaseAbsoluteTable (void);

		PSTORESTRUCT m_pStore;

		static PSTORESTRUCT g_pStore;
		static int g_iStoreSize;
		static PSTORESTRUCT g_pFreeStore;
	};

//	CStructArray. Implements a dynamic array of simple structures

class CStructArray : public CObject
	{
	public:
		CStructArray (void);
		CStructArray (int iElementSize, int iInitSize);

		inline ALERROR AppendStruct (void *pData, int *retiIndex = NULL) { return InsertStruct(pData, -1, retiIndex); }
		ALERROR ExpandArray (int iPos, int iCount);
		int GetCount (void) const;
		inline int GetElementSize (void) { return m_iElementSize; }
		void *GetStruct (int iIndex) const;
		ALERROR InsertStruct (void *pData, int iPos, int *retiIndex);
		void Remove (int iIndex);
		void RemoveAll (void);
		void SetStruct (int iIndex, void *pData);

	protected:
		virtual void CopyHandler (CObject *pOriginal);
		virtual ALERROR LoadHandler (CUnarchiver *pUnarchiver);
		virtual ALERROR SaveHandler (CArchiver *pArchiver);

	private:
		CINTDynamicArray m_Array;

		int m_iElementSize;
		int m_iInitSize;
	};

//	CSharedObjectQueue. Implements a thread-safe queue of objects

class CSharedObjectQueue : public CObject
	{
	public:
		CSharedObjectQueue (void);
		virtual ~CSharedObjectQueue (void);

		CObject *DequeueObject (void);
		ALERROR EnqueueObject (CObject *pObj);
		inline HANDLE GetQueueEvent (void) { return m_hEvent; }

	private:
		CObjectArray m_Array;
		int m_iHead;
		int m_iTail;

		CRITICAL_SECTION m_csLock;
		HANDLE m_hEvent;			//	Set if there is at least one object in queue
	};

//	CDictionary. Implementation of a dynamic array of entries

class CDictionary : public CObject
	{
	public:
		CDictionary (void);
		CDictionary (IObjectClass *pClass);
		virtual ~CDictionary (void);

		ALERROR AddEntry (int iKey, int iValue);
		ALERROR Find (int iKey, int *retiValue) const;
		ALERROR FindEx (int iKey, int *retiEntry) const;
		ALERROR FindOrAdd (int iKey, int iValue, BOOL *retbFound, int *retiValue);
		inline int GetCount (void) const { return m_Array.GetCount() / 2; }
		void GetEntry (int iEntry, int *retiKey, int *retiValue) const;
		ALERROR ReplaceEntry (int iKey, int iValue, BOOL bAdd, BOOL *retbAdded, int *retiOldValue);
		ALERROR RemoveAll (void) { return m_Array.RemoveAll(); }
		ALERROR RemoveEntryByOrdinal (int iEntry, int *retiOldValue = NULL);
		ALERROR RemoveEntry (int iKey, int *retiOldValue);

	protected:
		virtual int Compare (int iKey1, int iKey2) const;
		inline ALERROR ExpandArray (int iPos, int iCount) { return m_Array.ExpandArray(2 * iPos, 2 * iCount); }
		void SetEntry (int iEntry, int iKey, int iValue);

		BOOL FindSlot (int iKey, int *retiPos) const;

		CIntArray m_Array;
	};

//	CIDTable. Implementation of a table that matches IDs with objects

class CIDTable : public CDictionary
	{
	public:
		CIDTable (void);
		CIDTable (BOOL bOwned, BOOL bNoReference);
		virtual ~CIDTable (void);

		inline ALERROR AddEntry (int iKey, CObject *pValue) { return CDictionary::AddEntry(iKey, (int)pValue); }
		int GetKey (int iEntry) const;
		CObject *GetValue (int iEntry) const;
		ALERROR Lookup (int iKey, CObject **retpValue) const;
		ALERROR LookupEx (int iKey, int *retiEntry) const;
		ALERROR RemoveAll (void);
		ALERROR RemoveEntry (int iKey, CObject **retpOldValue);
		ALERROR ReplaceEntry (int iKey, CObject *pValue, BOOL bAdd, CObject **retpOldValue);
		void SetValue (int iEntry, CObject *pValue, CObject **retpOldValue);

	protected:
		virtual int Compare (int iKey1, int iKey2) const;
		virtual void CopyHandler (CObject *pOriginal);
		virtual ALERROR LoadHandler (CUnarchiver *pUnarchiver);
		virtual ALERROR SaveHandler (CArchiver *pArchiver);

	private:
		BOOL m_bOwned;
		BOOL m_bNoReference;
	};

//	CSymbolTable. Implementation of a symbol table

class CSymbolTable : public CDictionary
	{
	public:
		CSymbolTable (void);
		CSymbolTable (BOOL bOwned, BOOL bNoReference);
		virtual ~CSymbolTable (void);
		CSymbolTable &operator= (const CSymbolTable &Obj);

		ALERROR AddEntry (const CString &sKey, CObject *pValue);
		CString GetKey (int iEntry) const;
		CObject *GetValue (int iEntry) const;
		ALERROR Lookup (const CString &sKey, CObject **retpValue = NULL) const;
		ALERROR LookupEx (const CString &sKey, int *retiEntry) const;
		ALERROR RemoveAll (void);
		ALERROR RemoveEntry (int iEntry, CObject **retpOldValue = NULL);
		ALERROR RemoveEntry (const CString &sKey, CObject **retpOldValue);
		ALERROR ReplaceEntry (const CString &sKey, CObject *pValue, BOOL bAdd, CObject **retpOldValue);
		void SetValue (int iEntry, CObject *pValue, CObject **retpOldValue);

	protected:
		virtual int Compare (int iKey1, int iKey2) const;
		virtual void CopyHandler (CObject *pOriginal);
		virtual ALERROR LoadHandler (CUnarchiver *pUnarchiver);
		virtual ALERROR SaveHandler (CArchiver *pArchiver);

	private:
		BOOL m_bOwned;
		BOOL m_bNoReference;
	};

//	CAtomTable. Implementation of a string hash table

class CAtomTable : public CObject
	{
	public:
		CAtomTable (void);
		CAtomTable (int iHashSize);
		virtual ~CAtomTable (void);

		ALERROR AppendAtom (const CString &sString, int *retiAtom);
		int Atomize (const CString &sString);

	private:
		CSymbolTable *Hash (const CString &sString);

		int m_iHashSize;
		int m_iNextAtom;
		CSymbolTable *m_pBackbone;
	};

//	CIntSet. Implementation of a set of integers. This implementation
//	can have any number of elements and any range of elements. However,
//	the code is very inefficient if the elements are wildly discontiguous.
//
//	DEPRECATED: Use CLargeSet instead

#ifdef DEPRECATED
class CIntSet : public CObject
	{
	public:
		CIntSet (void);

		ALERROR AddElement (int iElement);
		ALERROR EnumElements (CIntArray *pArray) const;
		bool HasElement (int iElement) const;
		inline void RemoveAll (void) { m_Set.RemoveAll(); }
		void RemoveElement (int iElement);

	private:
		CIDTable m_Set;
	};
#endif

//	CLargeSet

class CLargeSet
	{
	public:
		CLargeSet (int iSize = -1);

		void Clear (DWORD dwValue);
		void ClearAll (void);
		bool IsEmpty (void) const;
		bool IsSet (DWORD dwValue) const;
		void Set (DWORD dwValue);

	private:
		TArray<DWORD> m_Set;
	};

//	Memory Blocks

class IReadBlock
	{
	public:
		virtual ~IReadBlock (void) { }

		virtual ALERROR Close (void) = 0;
		virtual ALERROR Open (void) = 0;
		virtual int GetLength (void) = 0;
		virtual char *GetPointer (int iOffset, int iLength = -1) = 0;
	};

class CFileReadBlock : public CObject, public IReadBlock
	{
	public:
		CFileReadBlock (void);
		CFileReadBlock (const CString &sFilename);
		virtual ~CFileReadBlock (void);

		inline const CString &GetFilename (void) const { return m_sFilename; }

		//	IReadBlock virtuals

		virtual ALERROR Close (void);
		virtual ALERROR Open (void);
		virtual int GetLength (void) { return (int)m_dwFileSize; }
		virtual char *GetPointer (int iOffset, int iLength = -1) { return m_pFile + iOffset; }

	private:
		CString m_sFilename;
		HANDLE m_hFile;
		HANDLE m_hFileMap;
		char *m_pFile;
		DWORD m_dwFileSize;
	};

class CResourceReadBlock : public CObject, public IReadBlock
	{
	public:
		CResourceReadBlock (void);
		CResourceReadBlock (HMODULE hInst, char *pszRes, char *pszType = RT_RCDATA);
		virtual ~CResourceReadBlock (void);

		//	IReadBlock virtuals

		virtual ALERROR Close (void);
		virtual ALERROR Open (void);
		virtual int GetLength (void) { return m_dwLength; }
		virtual char *GetPointer (int iOffset, int iLength = -1) { return m_pData + iOffset; }

	private:
		HMODULE m_hModule;
		char *m_pszRes;
		char *m_pszType;

		char *m_pData;
		DWORD m_dwLength;
	};

class CBufferReadBlock : public CObject, public IReadBlock
	{
	public:
		CBufferReadBlock (void) : CObject(NULL) { }
		CBufferReadBlock (const CString &sData) : CObject(NULL), m_sData(sData) { }

		//	IReadBlock virtuals

		virtual ALERROR Close (void) { return NOERROR; }
		virtual ALERROR Open (void) { return NOERROR; }
		virtual int GetLength (void) { return m_sData.GetLength(); }
		virtual char *GetPointer (int iOffset, int iLength = -1) { return m_sData.GetPointer() + iOffset; }

	private:
		CString m_sData;
	};

//	File System

class IWriteStream
	{
	public:
		virtual ALERROR Close (void) = 0;
		virtual ALERROR Create (void) = 0;
		virtual ALERROR Write (char *pData, int iLength, int *retiBytesWritten = NULL) = 0;
		virtual ALERROR WriteChar (char chChar, int iLength);
	};

class IReadStream
	{
	public:
		virtual ALERROR Close (void) = 0;
		virtual ALERROR Open (void) = 0;
		virtual ALERROR Read (char *pData, int iLength, int *retiBytesRead = NULL) = 0;
	};

//	CMemoryWriteStream. This object is used to write variable length
//	data to a memory block.

class CMemoryWriteStream : public CObject, public IWriteStream
	{
	public:
		CMemoryWriteStream (int iMaxSize = DEFAULT_MAX_SIZE);
		virtual ~CMemoryWriteStream (void);

		inline char *GetPointer (void) { return m_pBlock; }
		inline int GetLength (void) { return m_iCurrentSize; }

		//	IWriteStream virtuals

		virtual ALERROR Close (void);
		virtual ALERROR Create (void);
		virtual ALERROR Write (char *pData, int iLength, int *retiBytesWritten = NULL);

	private:
		enum Constants
			{
			DEFAULT_MAX_SIZE = 			(1024 * 1024),
			};

		int m_iMaxSize;
		int m_iCommittedSize;
		int m_iCurrentSize;
		char *m_pBlock;
	};

class CMemoryReadBlockWrapper : public IReadBlock
	{
	public:
		CMemoryReadBlockWrapper (CMemoryWriteStream &Stream) : 
				m_pPointer(Stream.GetPointer()),
				m_iLength(Stream.GetLength())
			{ }

		virtual ALERROR Close (void) { return NOERROR; }
		virtual ALERROR Open (void) { return NOERROR; }
		virtual int GetLength (void) { return m_iLength; }
		virtual char *GetPointer (int iOffset, int iLength = -1) { return m_pPointer + iOffset; }

	private:
		char *m_pPointer;
		int m_iLength;
	};

//	CMemoryReadStream. This object is used to read variable length data

class CMemoryReadStream : public CObject, public IReadStream
	{
	public:
		CMemoryReadStream (void);
		CMemoryReadStream (char *pData, int iDataSize);
		virtual ~CMemoryReadStream (void);

		//	IReadStream virtuals

		virtual ALERROR Close (void) { return NOERROR; }
		virtual ALERROR Open (void) { m_iPos = 0; return NOERROR; }
		virtual ALERROR Read (char *pData, int iLength, int *retiBytesRead = NULL);

	private:
		char *m_pData;
		int m_iDataSize;
		int m_iPos;
	};

//	CFileWriteStream. This object is used to write a file out

class CFileWriteStream : public CObject, public IWriteStream
	{
	public:
		CFileWriteStream (void);
		CFileWriteStream (const CString &sFilename, BOOL bUnique = FALSE);
		virtual ~CFileWriteStream (void);

		ALERROR Open (void);

		//	IWriteStream virtuals

		virtual ALERROR Close (void);
		virtual ALERROR Create (void);
		virtual ALERROR Write (char *pData, int iLength, int *retiBytesWritten = NULL);

	private:
		CString m_sFilename;
		BOOL m_bUnique;
		HANDLE m_hFile;
	};

//	CFileReadStream. This object is used to read a file in

class CFileReadStream : public CObject, public IReadStream
	{
	public:
		CFileReadStream (void);
		CFileReadStream (const CString &sFilename);
		virtual ~CFileReadStream (void);

		inline DWORD GetFileSize (void) { return m_dwFileSize; }

		//	IReadStream virtuals

		virtual ALERROR Close (void);
		virtual ALERROR Open (void);
		virtual ALERROR Read (char *pData, int iLength, int *retiBytesRead = NULL);

	private:
		CString m_sFilename;
		HANDLE m_hFile;
		HANDLE m_hFileMap;
		char *m_pFile;
		char *m_pPos;
		DWORD m_dwFileSize;
	};

//	CArchive. This is an object that knows how to archive objects to a 
//	stream.

class CArchiver : public CObject
	{
	public:
		CArchiver (void);
		CArchiver (IWriteStream *pStream);
		virtual ~CArchiver (void);

		ALERROR AddExternalReference (CString sTag, void *pReference);
		ALERROR AddObject (CObject *pObject);
		ALERROR BeginArchive (void);
		ALERROR EndArchive (void);
		inline void SetVersion (DWORD dwVersion) { m_dwVersion = dwVersion; }

		//	These methods should only be called by objects
		//	that are being saved

		ALERROR Reference2ID (void *pReference, int *retiID);
		ALERROR SaveObject (CObject *pObject);
		ALERROR WriteData (char *pData, int iLength);

	private:
		IWriteStream *m_pStream;					//	Stream to save to
		CObjectArray m_List;						//	List of objects to save
		CDictionary m_ReferenceList;				//	Pointer references
		CSymbolTable m_ExternalReferences;			//	List of external references
		int m_iNextID;								//	Next ID to use for references
		DWORD m_dwVersion;							//	User-defined version
	};

//	CUnarchiver. This is an object that knows how to load objects from
//	a stream.

class CUnarchiver : public CObject
	{
	public:
		CUnarchiver (void);
		CUnarchiver (IReadStream *pStream);
		virtual ~CUnarchiver (void);

		ALERROR BeginUnarchive (void);
		ALERROR EndUnarchive (void);
		inline CObjectArray *GetList (void) { return &m_List; }
		CObject *GetObject (int iIndex);
		DWORD GetVersion (void) { return m_dwVersion; }
		ALERROR ResolveExternalReference (CString sTag, void *pReference);
		void SetMinVersion (DWORD dwVersion) { m_dwMinVersion = dwVersion; }

		//	These methods should only be called by objects
		//	that are being loaded

		ALERROR LoadObject (CObject **retpObject);
		ALERROR ReadData (char *pData, int iLength);
		ALERROR ResolveReference (int iID, void **pReference);

	private:
		IReadStream *m_pStream;
		CObjectArray m_List;
		CSymbolTable *m_pExternalReferences;
		CIntArray m_ReferenceList;
		CIntArray m_FixupTable;
		DWORD m_dwVersion;
		DWORD m_dwMinVersion;
	};

//	CDataFile. This is a file-based collection of variable-sized records.

#define DFOPEN_FLAG_READ_ONLY					0x00000001

class CDataFile : public CObject
	{
	public:
		struct SVersionInfo
			{
			DWORD dwVersion;
			int iEntry;
			};

		CDataFile (const CString &sFilename);
		virtual ~CDataFile (void);

		ALERROR AddEntry (const CString &sData, int *retiEntry);
		ALERROR Close (void);
		ALERROR DeleteEntry (int iEntry);
		ALERROR Flush (void);
		inline CString GetFilename (void) const { return m_sFilename; }
		int GetDefaultEntry (void);
		int GetEntryLength (int iEntry);
		inline BOOL IsOpen (void) { return (m_hFile != INVALID_HANDLE_VALUE || m_pFile); }
		inline ALERROR Open (DWORD dwFlags = 0) { return Open(NULL_STR, dwFlags); }
		ALERROR Open (const CString &sFilename, DWORD dwFlags = 0);
		ALERROR OpenFromResource (HMODULE hInst, char *pszRes, DWORD dwFlags = 0);
		ALERROR ReadEntry (int iEntry, CString *retsData);
		ALERROR ReadEntryPartial (int iEntry, int iPos, int iLength, CString *retsData);
		ALERROR ReadHistory (int iEntry, TArray<SVersionInfo> *retHistory);
		void SetDefaultEntry (int iEntry);
		ALERROR WriteEntry (int iEntry, const CString &sData);
		ALERROR WriteVersion (int iEntry, const CString &sData, DWORD *retdwVersion = NULL);

		static ALERROR Create (const CString &sFilename,
							   int iBlockSize,
							   int iInitialEntries);

	private:
		typedef struct
			{
			DWORD dwBlock;								//	Block Number (-1 = unused)
			DWORD dwBlockCount;							//	Number of blocks reserved for entry
			DWORD dwSize;								//	Size of entry
			DWORD dwVersion;							//	Version number
			DWORD dwPrevEntry;							//	Previous version
			DWORD dwLatestEntry;						//	Latest entry (-1 if this is latest)
			DWORD dwFlags;								//	Misc flags
			} ENTRYSTRUCT, *PENTRYSTRUCT;

		struct SEntryV1
			{
			DWORD dwBlock;								//	Block Number (-1 = unused)
			DWORD dwBlockCount;							//	Number of blocks reserved for entry
			DWORD dwSize;								//	Size of entry
			DWORD dwFlags;								//	Misc flags
			};

		ALERROR AllocBlockChain (DWORD dwBlockCount, DWORD *retdwStartingBlock);
		ALERROR FreeBlockChain (DWORD dwStartingBlock, DWORD dwBlockCount);
		ALERROR GrowEntryTable (int *retiEntry);
		ALERROR OpenInt (void);
		ALERROR ReadBuffer (DWORD dwFilePos, DWORD dwLen, void *pBuffer);
		ALERROR ResizeEntry (int iEntry, DWORD dwSize, DWORD *retdwBlockCount);
		ALERROR WriteBlockChain (DWORD dwStartingBlock, char *pData, DWORD dwSize);

		CString m_sFilename;							//	Filename of data file
		HANDLE m_hFile;									//	Open file handle
		IReadBlock *m_pFile;							//	Memory file

		int m_iBlockSize;								//	Size of each block
		int m_iBlockCount;								//	Number of blocks in file
		int m_iDefaultEntry;							//	Default entry

		int m_iEntryTableCount;							//	Number of entries
		PENTRYSTRUCT m_pEntryTable;						//	Entry table

		DWORD m_fHeaderModified:1;						//	TRUE if header has changed
		DWORD m_fEntryTableModified:1;					//	TRUE if entry table has changed
		DWORD m_fFlushing:1;							//	TRUE if we're inside ::Flush
		DWORD m_fReadOnly:1;							//	TRUE if we're open read-only
	};

//	Directory classes

struct SFileDesc
	{
	CString sFilename;

	bool bFolder;
	bool bReadOnly;
	bool bSystemFile;
	bool bHiddenFile;
	};

class CFileDirectory
	{
	public:
		CFileDirectory (const CString &sFilespec);
		~CFileDirectory (void);

		bool HasMore (void);
		CString GetNext (bool *retbIsFolder = NULL);
		void GetNextDesc (SFileDesc *retDesc);

	private:
		CString m_sFilespec;
		HANDLE m_hSearch;
		WIN32_FIND_DATA m_FindData;
	};

//	Logging classes

#define ILOG_FLAG_WARNING					0x00000001	//	Warning log entry
#define ILOG_FLAG_ERROR						0x00000002	//	Error log entry
#define ILOG_FLAG_FATAL						0x00000004	//	Fatal error log entry
#define ILOG_FLAG_TIMEDATE					0x00000008	//	Include time date

class ILog
	{
	public:
		virtual ALERROR Close (void) = 0;
		virtual ALERROR Create (BOOL bAppend) = 0;
		virtual void LogOutput (DWORD dwFlags, char *pszLine, ...) = 0;
		virtual void LogOutput (DWORD dwFlags, const CString &sLine) = 0;
	};

class CTextFileLog : public CObject, public ILog
	{
	public:
		CTextFileLog (void);
		CTextFileLog (const CString &sFilename);

		CString GetSessionLog (void);
		void SetFilename (const CString &sFilename);
		void SetSessionStart (void);
		virtual ~CTextFileLog (void);

		//	ILog virtuals

		virtual ALERROR Close (void);
		virtual ALERROR Create (BOOL bAppend);
		virtual void LogOutput (DWORD dwFlags, char *pszLine, ...);
		virtual void LogOutput (DWORD dwFlags, const CString &sLine);

	private:
		HANDLE m_hFile;
		CString m_sFilename;

		DWORD m_dwSessionStart;				//	Offset to file at start of session
	};

//	Registry classes

class CRegKey
	{
	public:
		CRegKey (void);
		~CRegKey (void);

		static ALERROR OpenUserAppKey (const CString &sCompany, 
									   const CString &sAppName,
									   CRegKey *retKey);

		inline operator HKEY() const { return m_hKey; }

		bool FindStringValue (const CString &sValue, CString *retsData);
		void SetStringValue (const CString &sValue, const CString &sData);

	private:
		void CleanUp (void);

		HKEY m_hKey;
	};

extern char g_LowerCaseAbsoluteTable[256];

//	Initialization functions (Kernel.cpp)

void kernelCleanUp (void);
void kernelClearDebugLog (void);
void kernelDebugLogMessage (char *pszLine, ...);
CString kernelGetSessionDebugLog (void);

#define KERNEL_FLAG_INTERNETS					0x00000001
BOOL kernelInit (DWORD dwFlags = 0);
ALERROR kernelSetDebugLog (const CString &sFilespec, bool bAppend = true);
ALERROR kernelSetDebugLog (CTextFileLog *pLog, bool bAppend = true, bool bFreeLog = false);
HANDLE kernelCreateThread (LPTHREAD_START_ROUTINE pfStart, LPVOID pData);
bool kernelDispatchUntilEventSet (HANDLE hEvent, DWORD dwTimeout = INFINITE);

//	String functions (CString.cpp)

CString strCat (const CString &sString1, const CString &sString2);
int strCompare (const CString &sString1, const CString &sString2);
int strCompareAbsolute (const CString &sString1, const CString &sString2);
CString strConvert (const CString &sText, DWORD dwFromCP, DWORD dwToCP);
inline CString strANSIToUTF8 (const CString &sText) { return strConvert(sText, CP_ACP, CP_UTF8); }
inline CString strUTF8ToANSI (const CString &sText) { return strConvert(sText, CP_UTF8, CP_ACP); }

#define DELIMIT_TRIM_WHITESPACE					0x00000001
#define DELIMIT_ALLOW_BLANK_STRINGS				0x00000002
ALERROR strDelimitEx (const CString &sString, char cDelim, DWORD dwFlags, int iMinParts, TArray<CString> *retList);

inline ALERROR strDelimit (const CString &sString, char cDelim, int iMinParts, TArray<CString> *pStringList)
	{ return strDelimitEx(sString, cDelim, 0, iMinParts, pStringList); }

int strDelimitCount (const CString &sString, char cDelim, DWORD dwFlags);
CString strDelimitGet (const CString &sString, char cDelim, DWORD dwFlags, int iIndex);

CString strCapitalize (const CString &sString, int iOffset = 0);
CString strCapitalizeWords (const CString &sString);
CString strCEscapeCodes (const CString &sString);
CString strEncodeUTF8Char (DWORD dwCodePoint);
CString strEncodeW1252ToUTF8Char (char chChar);
bool strEndsWith (const CString &sString, const CString &sStringToFind);
bool strEquals (const CString &sString1, const CString &sString2);
int strFind (const CString &sString, const CString &sStringToFind);

CString strFormatBytes (DWORD dwBytes);

#define FORMAT_LEADING_ZERO						0x00000001
#define FORMAT_THOUSAND_SEPARATOR				0x00000002
#define FORMAT_UNSIGNED							0x00000004
CString strFormatInteger (int iValue, int iMinFieldWidth = -1, DWORD dwFlags = 0);

CString strFormatMicroseconds (DWORD dwMicroseconds);
CString strFormatMilliseconds (DWORD dwMilliseconds);
CString strFromDouble (double rValue);
CString strFromInt (int iInteger, bool bSigned = true);
int strGetHexDigit (char *pPos);
char strGetHexDigit (int iDigit);
inline bool strIsAlpha (char *pPos) { return (::IsCharAlpha(*pPos) == TRUE); }
inline bool strIsAlphaNumeric (char *pPos) { return (::IsCharAlphaNumeric(*pPos) == TRUE); }
inline bool strIsASCIIAlpha (char *pPos) { return (*pPos >= 'a' && *pPos <= 'z') || (*pPos >= 'A' && *pPos <= 'Z'); }
inline bool strIsASCIIControl (char *pPos) { return ((BYTE)*pPos <= (BYTE)0x1f) || *pPos == 0x7f; }
bool strIsASCIISymbol (char *pPos);
inline bool strIsDigit (char *pPos) { return (*pPos >= '0' && *pPos <= '9'); }
bool strIsInt (const CString &sValue, DWORD dwFlags = 0, int *retiValue = NULL);
inline bool strIsWhitespace (char *pPos) { return *pPos == ' ' || *pPos == '\t' || *pPos == '\n' || *pPos == '\r'; }
CString strLoadFromRes (HINSTANCE hInst, int iResID);
inline char strLowerCaseAbsolute (char chChar) { return g_LowerCaseAbsoluteTable[(BYTE)chChar]; }
bool strNeedsEscapeCodes (const CString &sString);

#define PARSE_THOUSAND_SEPARATOR				0x00000001
int strParseInt (char *pStart, int iNullResult, DWORD dwFlags, char **retpEnd = NULL, bool *retbNullValue = NULL);
inline int strParseInt (char *pStart, int iNullResult, char **retpEnd = NULL, bool *retbNullValue = NULL) { return strParseInt(pStart, iNullResult, 0, retpEnd, retbNullValue); }
int strParseIntOfBase (char *pStart, int iBase, int iNullResult, char **retpEnd = NULL, bool *retbNullValue = NULL);

void strParseWhitespace (char *pPos, char **retpPos);
CString strPattern (const CString &sPattern, LPVOID *pArgs);
CString strPatternSubst (CString sLine, ...);
CString strRepeat (const CString &sString, int iCount);
CString strRomanNumeral (int i);
bool strStartsWith (const CString &sString, const CString &sStringToFind);
CString strSubString (const CString &sString, int iOffset, int iLength = -1);
CString strTitleCapitalize (const CString &sString, char **pExceptions = NULL, int iExceptionsCount = 0);
double strToDouble (const CString &sString, double rFailResult, bool *retbFailed = NULL);
CString strToFilename (const CString &sString);
int strToInt (const CString &sString, int iFailResult, bool *retbFailed = NULL);
CString strToLower (const CString &sString);
CString strToUpper (const CString &sString);
CString strToXMLText (const CString &sString, bool bInBody = false);
CString strTrimWhitespace (const CString &sString);
CString strWord (const CString &sString, int iWordPos);

//	Path functions (Path.cpp)

enum ESpecialFolders
	{
	folderAppData,
	folderDocuments,
	folderPictures,
	folderMusic,
	};

struct SFileVersionInfo
	{
	SFileVersionInfo (void) :
			dwFileVersion(0),
			dwProductVersion(0)
		{ }

	CString sProductName;
	CString sProductVersion;
	CString sCompanyName;
	CString sCopyright;

	ULONG64 dwFileVersion;
	ULONG64 dwProductVersion;
	};

bool fileCopy (const CString &sSourceFilespec, const CString &sDestFilespec);
bool fileDelete (const CString &sFilespec, bool bRecycle = false);

const DWORD FFL_FLAG_DIRECTORIES_ONLY =		0x00000001;
const DWORD FFL_FLAG_RELATIVE_FILESPEC =	0x00000002;
const DWORD FFL_FLAG_RECURSIVE =			0x00000004;
bool fileGetFileList (const CString &sRoot, const CString &sPath, const CString &sSearch, DWORD dwFlags, TArray<CString> *retFiles);

CTimeDate fileGetModifiedTime (const CString &sFilespec);
DWORD fileGetProductVersion (void);
ALERROR fileGetVersionInfo (const CString &sFilename, SFileVersionInfo *retInfo);
bool fileMove (const CString &sSourceFilespec, const CString &sDestFilespec);
bool fileOpen (const CString &sFile, const CString &sParameters = NULL_STR, const CString &sCurrentFolder = NULL_STR, CString *retsError = NULL);

CString pathAddComponent (const CString &sPath, const CString &sComponent);
CString pathAddExtensionIfNecessary (const CString &sPath, const CString &sExtension);
bool pathCreate (const CString &sPath);
bool pathExists (const CString &sPath);
CString pathGetExecutablePath (HINSTANCE hInstance);
CString pathGetExtension (const CString &sPath);
CString pathGetFilename (const CString &sPath);
CString pathGetPath (const CString &sPath);
CString pathGetResourcePath (char *pszResID);
CString pathGetSpecialFolder (ESpecialFolders iFolder);
bool pathIsAbsolute (const CString &sPath);
bool pathIsFolder (const CString &sFilespec);
inline bool pathIsPathSeparator (char *pPos) { return (*pPos == '\\' || *pPos == '/'); }
bool pathIsResourcePath (const CString &sPath, char **retpszResID);
bool pathIsWritable (const CString &sFilespec);
CString pathMakeAbsolute (const CString &sPath, const CString &sRoot = NULL_STR);
CString pathMakeRelative (const CString &sFilespec, const CString &sRoot, bool bNoCheck = false);
CString pathStripExtension (const CString &sPath);
bool pathValidateFilename (const CString &sFilename, CString *retsValidFilename = NULL);

//	RegEx functions (RegEx.cpp)

struct SRegExMatch
	{
	char *pPos;
	CString sMatch;
	};

bool strRegEx (char *pStart, const CString &sPattern, TArray<SRegExMatch> *retMatches = NULL, char **retpEnd = NULL);

//	Math functions (Math.cpp)

const double g_Pi = 3.14159265358979323846;			//	Pi

DWORD mathGetSeed (void);
int mathNearestPowerOf2 (int x);
int mathPower (int x, int n);
DWORD mathRandom (void);
int mathRandom (int iFrom, int iTo);
double mathRandomGaussian (void);
int mathRound (double x);
int mathSeededRandom (int iSeed, int iFrom, int iTo);
void mathSetSeed (DWORD dwSeed);
int mathSqrt (int x);
inline double mathDegreesToRadians (double rDegrees) { return g_Pi * rDegrees / 180.0; }

//	Compression functions

void CompressRunLengthByte (IWriteStream *pOutput, IReadBlock *pInput);
void UncompressRunLengthByte (IWriteStream *pOutput, IReadBlock *pInput);

//	System functions

DWORD sysGetAPIFlags (void);
int sysGetProcessorCount (void);
CString sysGetUserName (void);
bool sysIsBigEndian (void);
bool sysOpenURL (const CString &sURL);

//	Utility functions (Utilities.cpp)

DWORD utlHashFunctionCase (BYTE *pKey, int iKeyLen);
void utlMemSet (LPVOID pDest, DWORD Count, char Value);
void utlMemCopy (char *pSource, char *pDest, DWORD dwCount);
BOOL utlMemCompare (char *pSource, char *pDest, DWORD dwCount);
inline LPVOID MemAlloc (int iSize) { return (BYTE *)HeapAlloc(GetProcessHeap(), 0, iSize); }
inline void MemFree (LPVOID pMem) { HeapFree(GetProcessHeap(), 0, pMem); }

//	UI functions

ALERROR uiCopyTextToClipboard (HWND hWnd, const CString &sText);
void uiGetCenteredWindowRect (int cxWidth, 
							  int cyHeight, 
							  RECT *retrcRect,
							  bool bClip = true);
inline bool uiIsControlDown (void) { return (::GetAsyncKeyState(VK_CONTROL) & 0x8000) ? true : false; }
inline bool uiIsKeyDown (int iVirtKey) { return ((::GetAsyncKeyState(iVirtKey) & 0x8000) ? true : false); }
inline bool uiIsKeyRepeat (DWORD dwKeyData) { return ((dwKeyData & 0x40000000) ? true : false); }
inline bool uiIsShiftDown (void) { return (::GetAsyncKeyState(VK_SHIFT) & 0x8000) ? true : false; }

//	Note: This cannot be an inline because it will fail if the inline is
//	ever compiled as a function call
#define MemStackAlloc(iSize) (_alloca(iSize))

//	Comparison functions

inline int KeyCompare (const CString &sKey1, const CString &sKey2)
	{
	return strCompare(sKey1, sKey2);
	}

inline int KeyCompare (const CTimeDate &Key1, const CTimeDate &Key2)
	{
	return Key1.Compare(Key2);
	}

template<class KEY> int KeyCompare (const KEY &Key1, const KEY &Key2) 
	{
	if (Key1 > Key2)
		return 1;
	else if (Key1 < Key2)
		return -1;
	else
		return 0;
	}

//	The following macro "NoEmptyFile()" can be put into a file 
//	in order suppress the MS Visual C++ Linker warning 4221 
// 
//	warning LNK4221: no public symbols found; archive member will be inaccessible 
// 
//	Thanks to: http://stackoverflow.com/users/14904/adisak
//	See: http://stackoverflow.com/questions/1822887/what-is-the-best-way-to-eliminate-ms-visual-c-linker-warning-warning-lnk422
 
#define NoEmptyFile()   namespace { char NoEmptyFileDummy##__LINE__; } 

#endif