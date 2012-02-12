//	TStack.h
//
//	TStack Class
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#ifndef INCL_TSTACK
#define INCL_TSTACK

class CStackBase
	{
	protected:
		CStackBase (int iMaxSize);
		~CStackBase (void);

		void *GetDataAt (int iSP);
		void *PushData (int iDataSize);
		void *PopData (int iDataSize);

		ALERROR Commit (int iSize);

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

		void DeleteAll (void)
			{
			Pop(GetCount());
			}

		int GetCount (void) const
			{
			return (m_iSP / sizeof(VALUE));
			}

		int GetSP (void) const
			{
			return (m_iSP / sizeof(VALUE));
			}

		bool IsEmpty (void) const
			{
			return (m_iSP == 0);
			}

		void Pop (int iCount = 1)
			{
			VALUE *pElement = (VALUE *)PopData(iCount * sizeof(VALUE));
			for (int i = 0; i < iCount; i++, pElement++)
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
					new(pElement) VALUE;
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

#endif

