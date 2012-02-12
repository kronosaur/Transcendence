//	TMap.h
//
//	TMap class
//	Copyright (c) 2010 by George Moromisato. All Rights Reserved.

#ifndef INCL_TMAP
#define INCL_TMAP

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
		void InsertEntry (int iSlot, EntryBase *pEntry);

		void Reset (CMapIterator &Iterator) const;
		EntryBase *GetNext (CMapIterator &Iterator) const;
		bool HasMore (CMapIterator &Iterator) const;

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

		const KEY &GetKey (VALUE *pValue) const
			{
			//	LATER: Is there some keyword for field offset?
			Entry *pEntry = (Entry *)(((char *)pValue) - (sizeof(KEY) + sizeof(Entry *)));
			return pEntry->m_Key;
			}

		const KEY &GetNext (CMapIterator &Iterator, VALUE **retpValue) const
			{
			Entry *pEntryFound = (Entry *)CMapBase::GetNext(Iterator);
			ASSERT(pEntryFound);
			if (retpValue)
				*retpValue = &pEntryFound->m_Value;
			return pEntryFound->m_Key;
			}

		VALUE * const GetNext (CMapIterator &Iterator) const
			{
			Entry *pEntryFound = (Entry *)CMapBase::GetNext(Iterator);
			ASSERT(pEntryFound);
			return &pEntryFound->m_Value;
			}

		bool HasMore (CMapIterator &Iterator) const
			{
			return CMapBase::HasMore(Iterator);
			}

		VALUE * const Insert (const KEY &Key)
			{
			Entry *pNewEntry = new Entry(Key);
			InsertEntry(MapKeyHashData(Key), MapKeyHashDataSize(Key), (EntryBase *)pNewEntry);
			return &pNewEntry->m_Value;
			}

		void Insert (const KEY &Key, const VALUE &Value)
			{
			Entry *pNewEntry = new Entry(Key, Value);
			InsertEntry(MapKeyHashData(Key), MapKeyHashDataSize(Key), (EntryBase *)pNewEntry);
			}

		void Reset (CMapIterator &Iterator) const
			{
			CMapBase::Reset(Iterator);
			}

		VALUE * const Set (const KEY &Key)
			{
			int iSlot;
			Entry *pEntryFound = (Entry *)FindEntry((void *)&Key, MapKeyHashData(Key), MapKeyHashDataSize(Key), &iSlot);
			if (pEntryFound)
				return &pEntryFound->m_Value;

			Entry *pNewEntry = new Entry(Key);
			InsertEntry(iSlot, (EntryBase *)pNewEntry);
			return &pNewEntry->m_Value;
			}

		void Set (const KEY &Key, const VALUE &Value)
			{
			Entry *pEntryFound = (Entry *)FindEntry((void *)&Key, MapKeyHashData(Key), MapKeyHashDataSize(Key));
			if (pEntryFound)
				pEntryFound->m_Value = Value;
			else
				Insert(Key, Value);
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

template <class KEY, class VALUE> class TSortMap
	{
	public:
		TSortMap (ESortOptions iOrder = AscendingSort) : m_iOrder(iOrder) { }

		inline VALUE &operator [] (int iIndex) const { return GetValue(iIndex); }

		void DeleteAll (void)
			{
			m_Index.DeleteAll();
			m_Array.DeleteAll();
			}

		VALUE *GetAt (const KEY &key) const
			{
			int iPos;
			if (!FindPos(key, &iPos))
				return NULL;

			return &m_Array[m_Index[iPos]].theValue;
			}

		int GetCount (void) const
			{
			return m_Index.GetCount();
			}

		const KEY &GetKey (int iIndex) const
			{
			return m_Array[m_Index[iIndex]].theKey;
			}

		VALUE &GetValue (int iIndex) const
			{
			return m_Array[m_Index[iIndex]].theValue;
			}

		VALUE *Insert (const KEY &newKey, bool *retbInserted = NULL)
			{
			int iPos;
			if (FindPos(newKey, &iPos))
				{
				if (retbInserted)
					*retbInserted = false;
				return &m_Array[m_Index[iPos]].theValue;
				}

			m_Index.Insert(m_Array.GetCount(), iPos);
			SEntry *pEntry = m_Array.Insert();
			pEntry->theKey = newKey;

			if (retbInserted)
				*retbInserted = true;

			return &pEntry->theValue;
			}

		void Insert (const KEY &newKey, const VALUE &newValue)
			{
			VALUE *pNewValue = Insert(newKey);
			*pNewValue = newValue;
			}

	private:
		struct SEntry
			{
			KEY theKey;
			VALUE theValue;
			};

		bool FindPos (const KEY &key, int *retiPos = NULL) const
			{
			int iCount = m_Index.GetCount();
			int iMin = 0;
			int iMax = iCount;
			int iTry = iMax / 2;

			while (true)
				{
				if (iMax <= iMin)
					{
					if (retiPos)
						*retiPos = iMin;
					return false;
					}

				int iCompare = m_iOrder * KeyCompare(key, GetKey(iTry));
				if (iCompare == 0)
					{
					if (retiPos)
						*retiPos = iTry;
					return true;
					}
				else if (iCompare == -1)
					{
					iMin = iTry + 1;
					iTry = iMin + (iMax - iMin) / 2;
					}
				else if (iCompare == 1)
					{
					iMax = iTry;
					iTry = iMin + (iMax - iMin) / 2;
					}
				}

			return false;
			}

		ESortOptions m_iOrder;
		TArray<int> m_Index;
		TArray<SEntry> m_Array;
	};

//	Simple map classes

template <class VALUE> class TStringMap : public TMap<CString, VALUE> { };
template <class VALUE> class TIntMap : public TMap<int, VALUE> { };

#endif