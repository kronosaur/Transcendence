//	TArray.h
//
//	TArray Class

#ifndef INCL_TARRAY
#define INCL_TARRAY

enum ESortOptions
	{
	DescendingSort = 1,
	AscendingSort = -1,
	};

#ifdef DEBUG_MEMORY_LEAKS
#undef new
#endif

//	Explicit placement operator
inline void *operator new (size_t, void *p) { return p; }

const int DEFAULT_ARRAY_GRANULARITY = 10;

class CArrayBase
	{
	protected:
		CArrayBase (HANDLE hHeap, int iGranularity);
		~CArrayBase (void);

		void CopyOptions (const CArrayBase &Src);
		void DeleteBytes (int iOffset, int iLength);
		inline char *GetBytes (void) const { return (m_pBlock ? (char *)(&m_pBlock[1]) : NULL); }
		inline int GetGranularity (void) const { return (m_pBlock ? m_pBlock->m_iGranularity : DEFAULT_ARRAY_GRANULARITY); }
		inline HANDLE GetHeap (void) const { return (m_pBlock ? m_pBlock->m_hHeap : ::GetProcessHeap()); }
		inline int GetSize (void) const { return (m_pBlock ? m_pBlock->m_iSize : 0); }
		void InsertBytes (int iOffset, void *pData, int iLength, int iAllocQuantum);
		ALERROR Resize (int iNewSize, bool bPreserve, int iAllocQuantum);
		void TakeHandoffBase (CArrayBase &Src);

	private:
		struct SHeader
			{
			HANDLE m_hHeap;				//	Heap on which block is allocated
			int m_iSize;				//	Size of data portion (as seen by callers)
			int m_iAllocSize;			//	Current size of block
			int m_iGranularity;			//	Used by descendants to resize block
			};

		SHeader *m_pBlock;
	};

#pragma warning(disable:4291)			//	No need for a delete because we're placing object
template <class VALUE> class TRawArray : public CArrayBase
	{
	public:
		TRawArray (void) : CArrayBase(NULL, DEFAULT_ARRAY_GRANULARITY), m_iExtraBytes(0) { }
		TRawArray (int iExtraBytes) : CArrayBase(NULL, DEFAULT_ARRAY_GRANULARITY), m_iExtraBytes(iExtraBytes) { }

		inline VALUE &operator [] (int iIndex) const { return GetAt(iIndex); }

		void Delete (int iIndex)
			{
			VALUE *pElement = (VALUE *)(GetBytes() + iIndex * GetElementSize());
			ASSERT((char *)pElement - GetBytes() < GetSize());
			pElement->VALUE::~VALUE();
			DeleteBytes(iIndex * GetElementSize(), GetElementSize());
			}

		void DeleteAll (void)
			{
			VALUE *pElement = (VALUE *)GetBytes();
			for (int i = 0; i < GetCount(); i++, pElement++)
				pElement->VALUE::~VALUE();

			DeleteBytes(0, GetSize());
			}

		inline VALUE &GetAt (int iIndex) const
			{
			VALUE *pElement = (VALUE *)(GetBytes() + iIndex * GetElementSize());
			return *pElement;
			}

		inline int GetCount (void) const { return (GetSize() / GetElementSize()); }

		inline int GetElementSize (void) const { return sizeof(VALUE) + m_iExtraBytes; }

		inline int GetExtraBytes (void) const { return m_iExtraBytes; }

		void Insert (const VALUE &Value, int iIndex = -1)
			{
			int iOffset;
			if (iIndex == -1) iIndex = GetCount();
			iOffset = iIndex * GetElementSize();
			InsertBytes(iOffset, NULL, GetElementSize(), DEFAULT_ARRAY_GRANULARITY * GetElementSize());

			VALUE *pElement = new(GetBytes() + iOffset) VALUE(Value);
			}

		VALUE *InsertEmpty (int iCount = 1, int iIndex = -1)
			{
			int iOffset;
			if (iIndex == -1) iIndex = GetCount();
			iOffset = iIndex * GetElementSize();
			InsertBytes(iOffset, NULL, iCount * GetElementSize(), iCount * DEFAULT_ARRAY_GRANULARITY * GetElementSize());

			for (int i = 0; i < iCount; i++)
				{
				VALUE *pElement = new(GetBytes() + iOffset + (i * GetElementSize())) VALUE;
				}

			return &GetAt(iIndex);
			}

		void SetExtraBytes (int iExtraBytes)
			{
			ASSERT(GetCount() == 0);
			m_iExtraBytes = iExtraBytes;
			}

	private:
		int m_iExtraBytes;
	};

template <class VALUE> class TArray : public CArrayBase
	{
	public:
		TArray (HANDLE hHeap = NULL) : CArrayBase(hHeap, DEFAULT_ARRAY_GRANULARITY) { }
		TArray (int iGranularity) : CArrayBase(NULL, iGranularity) { }
		TArray (const TArray<VALUE> &Obj) : CArrayBase(Obj.GetHeap(), Obj.GetGranularity())
			{
			InsertBytes(0, NULL, Obj.GetCount() * sizeof(VALUE), GetGranularity() * sizeof(VALUE));

			for (int i = 0; i < Obj.GetCount(); i++)
				{
				VALUE *pElement = new(GetBytes() + (i * sizeof(VALUE))) VALUE(Obj[i]);
				}
			}

		~TArray (void) { DeleteAll(); }

		TArray<VALUE> &operator= (const TArray<VALUE> &Obj)
			{
			DeleteAll();

			CopyOptions(Obj);
			InsertBytes(0, NULL, Obj.GetCount() * sizeof(VALUE), GetGranularity() * sizeof(VALUE));

			for (int i = 0; i < Obj.GetCount(); i++)
				{
				VALUE *pElement = new(GetBytes() + (i * sizeof(VALUE))) VALUE(Obj[i]);
				}

			return *this;
			}

		inline VALUE &operator [] (int iIndex) const { return GetAt(iIndex); }

		void Delete (int iIndex)
			{
			VALUE *pElement = (VALUE *)(GetBytes() + iIndex * sizeof(VALUE));
			ASSERT((char *)pElement - GetBytes() < GetSize());
			pElement->VALUE::~VALUE();
			DeleteBytes(iIndex * sizeof(VALUE), sizeof(VALUE));
			}

		void DeleteAll (void)
			{
			VALUE *pElement = (VALUE *)GetBytes();
			for (int i = 0; i < GetCount(); i++, pElement++)
				pElement->VALUE::~VALUE();

			DeleteBytes(0, GetSize());
			}

		bool Find (const VALUE &ToFind, int *retiIndex = NULL) const
			{
			int iCount = GetCount();

			for (int i = 0; i < iCount; i++)
				if (GetAt(i) == ToFind)
					{
					if (retiIndex)
						*retiIndex = i;
					return true;
					}

			return false;
			}

		inline VALUE &GetAt (int iIndex) const
			{
			VALUE *pElement = (VALUE *)(GetBytes() + iIndex * sizeof(VALUE));
			return *pElement;
			}

		inline int GetCount (void) const
			{
			return GetSize() / sizeof(VALUE);
			}

		void Insert (const VALUE &Value, int iIndex = -1)
			{
			int iOffset;
			if (iIndex == -1) iIndex = GetCount();
			iOffset = iIndex * sizeof(VALUE);
			InsertBytes(iOffset, NULL, sizeof(VALUE), GetGranularity() * sizeof(VALUE));

			VALUE *pElement = new(GetBytes() + iOffset) VALUE(Value);
			}

		VALUE *Insert (void)
			{
			int iOffset = GetCount() * sizeof(VALUE);
			InsertBytes(iOffset, NULL, sizeof(VALUE), GetGranularity() * sizeof(VALUE));

			return new(GetBytes() + iOffset) VALUE;
			}

		VALUE *InsertAt (int iIndex)
			{
			int iOffset;
			if (iIndex == -1) iIndex = GetCount();
			iOffset = iIndex * sizeof(VALUE);
			InsertBytes(iOffset, NULL, sizeof(VALUE), GetGranularity() * sizeof(VALUE));

			return new(GetBytes() + iOffset) VALUE;
			}

		void InsertEmpty (int iCount = 1, int iIndex = -1)
			{
			int iOffset;
			if (iIndex == -1) iIndex = GetCount();
			iOffset = iIndex * sizeof(VALUE);
			InsertBytes(iOffset, NULL, iCount * sizeof(VALUE), GetGranularity() * sizeof(VALUE));

			for (int i = 0; i < iCount; i++)
				{
				VALUE *pElement = new(GetBytes() + iOffset + (i * sizeof(VALUE))) VALUE;
				}
			}

		void Shuffle (void)
			{
			if (GetCount() < 2)
				return;

			//	Fisher-Yates algorithm

			int i = GetCount() - 1;
			while (i > 0)
				{
				int x = mathRandom(0, i);

				VALUE Temp = GetAt(x);
				GetAt(x) = GetAt(i);
				GetAt(i) = Temp;

				i--;
				}
			}

		void Sort (ESortOptions Order = AscendingSort)
			{
			if (GetCount() < 2)
				return;

			TArray<int> Result;

			//	Binary sort the contents into an indexed array

			SortRange(Order, 0, GetCount() - 1, Result);

			//	Create a new sorted array

			TArray<VALUE> SortedArray;
			SortedArray.InsertEmpty(GetCount());
			for (int i = 0; i < GetCount(); i++)
				SortedArray[i] = GetAt(Result[i]);

			TakeHandoff(SortedArray);
			}

		void TakeHandoff (TArray<VALUE> &Obj)
			{
			DeleteAll();
			TakeHandoffBase(Obj);
			}

	private:
		void SortRange (ESortOptions Order, int iLeft, int iRight, TArray<int> &Result)
			{
			if (iLeft == iRight)
				Result.Insert(iLeft);
			else if (iLeft + 1 == iRight)
				{
				int iCompare = Order * KeyCompare(GetAt(iLeft), GetAt(iRight));
				if (iCompare == 1)
					{
					Result.Insert(iLeft);
					Result.Insert(iRight);
					}
				else
					{
					Result.Insert(iRight);
					Result.Insert(iLeft);
					}
				}
			else
				{
				int iMid = iLeft + ((iRight - iLeft) / 2);

				TArray<int> Buffer1;
				TArray<int> Buffer2;

				SortRange(Order, iLeft, iMid, Buffer1);
				SortRange(Order, iMid+1, iRight, Buffer2);

				//	Merge

				int iPos1 = 0;
				int iPos2 = 0;
				bool bDone = false;
				while (!bDone)
					{
					if (iPos1 < Buffer1.GetCount() && iPos2 < Buffer2.GetCount())
						{
						int iCompare = Order * KeyCompare(GetAt(Buffer1[iPos1]), GetAt(Buffer2[iPos2]));
						if (iCompare == 1)
							Result.Insert(Buffer1[iPos1++]);
						else if (iCompare == -1)
							Result.Insert(Buffer2[iPos2++]);
						else
							{
							Result.Insert(Buffer1[iPos1++]);
							Result.Insert(Buffer2[iPos2++]);
							}
						}
					else if (iPos1 < Buffer1.GetCount())
						Result.Insert(Buffer1[iPos1++]);
					else if (iPos2 < Buffer2.GetCount())
						Result.Insert(Buffer2[iPos2++]);
					else
						bDone = true;
					}
				}
			}
	};
#pragma warning(default:4291)

//	Simple array classes

//class CStringArray : public TArray<CString> { };
//class CIntArray : public TArray<int> { };

#ifdef DEBUG_MEMORY_LEAKS
#define new DEBUG_NEW
#endif

#endif