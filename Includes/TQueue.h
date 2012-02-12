//	TQueue.h
//
//	TQueue Class

#ifndef INCL_TQUEUE
#define INCL_TQUEUE

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