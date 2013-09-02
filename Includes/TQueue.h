//	TQueue.h
//
//	TQueue Class

#ifndef INCL_TQUEUE
#define INCL_TQUEUE

template <class VALUE> class TQueue
	{
	public:
		TQueue (int iSize = 100)
			{
			m_pArray = NULL;
			Init(iSize);
			}

		TQueue (const TQueue<VALUE> &Src)
			{
			int i;

			m_iSize = Src.m_iSize;
			m_pArray = new VALUE [m_iSize];
			m_iHead = Src.m_iHead;
			m_iTail = Src.m_iTail;

			for (i = 0; i < GetCount(); i++)
				GetAt(i) = Src.GetAt(i);
			}

		~TQueue (void)
			{
			if (m_pArray)
				delete [] m_pArray;
			}

		inline VALUE &operator [] (int iIndex) const { return GetAt(iIndex); }

		void DeleteAll (void)
			{
			Dequeue(GetCount(), true);
			}

		void Dequeue (bool bDelete = false)
			{
			ASSERT(m_iHead != m_iTail);
			if (bDelete)
				m_pArray[m_iHead] = VALUE();

			m_iHead = (m_iHead + 1) % m_iSize;
			}

		void Dequeue (int iCount, bool bDelete = false)
			{
			int i;
			ASSERT(iCount <= GetCount());
			if (bDelete)
				{
				for (i = 0; i < iCount; i++)
					GetAt(i) = VALUE();
				}

			m_iHead = (m_iHead + iCount) % m_iSize;
			}

		void Enqueue (void)
			{
			if (IsFull())
				throw CException(ERR_MEMORY);

			m_iTail = (m_iTail + 1) % m_iSize;
			}

		void Enqueue (const VALUE &Value)
			{
			if (((m_iTail + 1) % m_iSize) == m_iHead)
				throw CException(ERR_MEMORY);

			m_pArray[m_iTail] = Value;
			m_iTail = (m_iTail + 1) % m_iSize;
			}

		VALUE &EnqueueAndOverwrite (void)
			{
			if (((m_iTail + 1) % m_iSize) == m_iHead)
				m_iHead = (m_iHead + 1) % m_iSize;

			m_iTail = (m_iTail + 1) % m_iSize;
			return m_pArray[(m_iTail + m_iSize - 1) % m_iSize];
			}

		void EnqueueAndOverwrite (const VALUE &Value)
			{
			if (((m_iTail + 1) % m_iSize) == m_iHead)
				m_iHead = (m_iHead + 1) % m_iSize;

			m_pArray[m_iTail] = Value;
			m_iTail = (m_iTail + 1) % m_iSize;
			}

		bool Find (const VALUE &ToFind) const
			{
			int i;
			for (i = 0; i < GetCount(); i++)
				if (KeyCompare(GetAt(i), ToFind) == 0)
					return true;

			return false;
			}

		inline VALUE &GetAt (int iIndex) const
			{
			ASSERT(iIndex < GetCount());
			return m_pArray[(m_iHead + iIndex) % m_iSize];
			}

		int GetCapacity (void) const
			{
			return m_iSize - 1;
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

		void Init (int iSize)
			{
			if (m_pArray)
				delete [] m_pArray;

			if (iSize > 0)
				{
				m_iSize = iSize + 1;
				m_pArray = new VALUE [m_iSize];
				}
			else
				{
				m_iSize = 0;
				m_pArray = NULL;
				}

			m_iHead = 0;
			m_iTail = 0;
			}

		inline bool IsEmpty (void) const
			{
			return (m_iHead == m_iTail);
			}

		inline bool IsFull (void) const
			{
			return (((m_iTail + 1) % m_iSize) == m_iHead);
			}

		inline VALUE &Tail (void) const
			{
			ASSERT(m_iHead != m_iTail);
			return m_pArray[(m_iTail + m_iSize - 1) % m_iSize];
			}

		bool TryEnqueue (const VALUE &Value)
			{
			if (((m_iTail + 1) % m_iSize) == m_iHead)
				return false;

			m_pArray[m_iTail] = Value;
			m_iTail = (m_iTail + 1) % m_iSize;
			return true;
			}

	private:
		VALUE *m_pArray;
		int m_iSize;
		int m_iHead;
		int m_iTail;
	};

#if 0
template <class VALUE> class TQueue
	{
	public:
		TQueue (int iSize)
			{
			ASSERT(iSize >= 0);

			m_iSize = iSize+1;		//	+1 because when tail == head-1 queue is full
			m_pArray = new VALUE [m_iSize];
			m_iHead = 0;
			m_iTail = 0;
			}

		~TQueue (void)
			{
			delete [] m_pArray;
			}

		inline VALUE &operator [] (int iIndex) const { return GetAt(iIndex); }

		void Dequeue (void)
			{
			ASSERT(GetCount() > 0);

			if (m_iHead != m_iTail)
				m_iHead = (m_iHead + 1) % m_iSize;
			}

		inline VALUE &GetAt (int iIndex) const
			{
			ASSERT(iIndex >= 0 && iIndex < GetCount());
			return m_pArray[(m_iHead + iIndex) % m_iSize];
			}

		inline int GetCount (void) const
			{
			return (m_iTail + m_iSize - m_iHead) % m_iSize;
			}

		inline int GetMaxCount (void) const { return m_iSize - 1; }

		inline VALUE &Head (void) const
			{
			ASSERT(GetCount() > 0);
			return m_pArray[m_iHead];
			}

		int Queue (void)
			{
			ASSERT(GetCount() < m_iSize-1);
			m_iTail = (m_iTail + 1) % m_iSize;
			return GetCount()-1;
			}

		int Queue (const VALUE &Value)
			{
			ASSERT(GetCount() < m_iSize-1);
			m_pArray[m_iTail] = Value;
			m_iTail = (m_iTail + 1) % m_iSize;
			return GetCount()-1;
			}

		inline VALUE &Tail (void) const
			{
			ASSERT(GetCount() > 0);
			return m_pArray[(m_iTail + m_iSize - 1) % m_iSize];
			}

	private:
		VALUE *m_pArray;
		int m_iSize;
		int m_iHead;
		int m_iTail;
	};
#endif

#endif