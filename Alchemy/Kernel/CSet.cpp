//	CSet.cpp
//
//	Implementation of set structure

#ifdef LATER

#include <windows.h>
#include "Kernel.h"
#include "KernelObjID.h"

static DATADESCSTRUCT g_DataDesc[] =
	{	{ DATADESC_OPCODE_EMBED_OBJ,	1,	0 },		//	m_Runs
		{ DATADESC_OPCODE_INT,			1,	0 },		//	m_iCount
		{ DATADESC_OPCODE_STOP,	0,	0 } };
static CObjectClass<CSet>g_ClassData(OBJID_CSET, g_DataDesc);

class CSet : public CObject
	{
	public:
		CSet (void);
		virtual ~CSet (void);

		virtual ALERROR AddElement (DWORD dwElement);
		virtual int GetCount (void);
		virtual DWORD GetElement (int iIndex);
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

CSet::CSet (void)

//	CSet constructor

	{
	}

CSet::~CSet (void)

//	CSet destructor

	{
	}

ALERROR CSet::AddElement (DWORD dwElement)

//	AddElement
//
//	Adds an element to the set

	{
	return NOERROR;
	}

int CSet::GetCount (void)

//	GetCount
//
//	Returns the number of elements in the set

	{
	return m_iCount;
	}

ALERROR CSet::GetAllElement (CIntArray *retpArray)

//	GetAllElements
//
//	Returns all the elements in the set sorted

	{
	}

BOOL CSet::IsMember (DWORD dwElement)

//	IsMember
//
//	Return TRUE if the given element is a member of the
//	set

	{
	return FALSE;
	}

ALERROR CSet::RemoveElement (DWORD dwElement)
void CSet::RemoveAll (void)

#endif