//	CSharedObjectQueue.cpp
//
//	Implements a thread-safe queue of objects

#include "Kernel.h"
#include "KernelObjID.h"

static DATADESCSTRUCT g_DataDesc[] =
	{	{ DATADESC_OPCODE_EMBED_OBJ,	1,	0 },		//	m_Array
		{ DATADESC_OPCODE_INT,			2,	0 },		//	m_iHead, m_iTail
		{ DATADESC_OPCODE_ZERO,			sizeof(CRITICAL_SECTION) / sizeof(DWORD),	0 },		//	m_csLock
		{ DATADESC_OPCODE_ZERO,			1,	0 },		//	m_hEvent
		{ DATADESC_OPCODE_STOP,	0,	0 } };
static CObjectClass<CObjectArray>g_ClassData(OBJID_CSHAREDOBJECTQUEUE,g_DataDesc);

CSharedObjectQueue::CSharedObjectQueue (void) : CObject(&g_ClassData),
		m_Array(FALSE),
		m_iHead(-1),
		m_iTail(0)

//	CSharedObjectQueue constructor

	{
	InitializeCriticalSection(&m_csLock);
	m_hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	}

CSharedObjectQueue::~CSharedObjectQueue (void)

//	CSharedObjectQueue destructor

	{
	CloseHandle(m_hEvent);
	DeleteCriticalSection(&m_csLock);
	}

CObject *CSharedObjectQueue::DequeueObject (void)

//	DequeueObject
//
//	Pulls the next object out of the queue. The caller
//	is reponsible for freeing the object when done. Returns
//	NULL if there are no objects in the queue.

	{
	CObject *pObj = NULL;

	EnterCriticalSection(&m_csLock);

	if (m_iHead != -1)
		{
		pObj = m_Array.GetObject(m_iHead);
		m_Array.ReplaceObject(m_iHead, NULL);

		m_iHead = (m_iHead + 1) % m_Array.GetCount();

		//	Is the queue empty?

		if (m_iHead == m_iTail)
			{
			m_iHead = -1;
			ResetEvent(m_hEvent);
			}
		}

	LeaveCriticalSection(&m_csLock);

	return pObj;
	}

ALERROR CSharedObjectQueue::EnqueueObject (CObject *pObj)

//	EnqueueObject
//
//	Enqueue the given object. The queue takes ownership.

	{
	ALERROR error;

	EnterCriticalSection(&m_csLock);

	//	Allocate a new entry in the array, if necessary.

	if (m_iTail == m_iHead || m_iTail >= m_Array.GetCount())
		{
		if (error = m_Array.InsertObject(pObj, m_iTail, NULL))
			goto Fail;

		//	If we added a new entry before the head index then
		//	we need to increment because the whole array has
		//	to move up.

		if (m_iTail <= m_iHead)
			m_iHead++;
		}

	//	Otherwise, just place the object in the proper place
	//	in the array.

	else
		{
		ASSERT(m_Array.GetObject(m_iTail) == NULL);
		m_Array.ReplaceObject(m_iTail, pObj);
		}

	//	If the queue was empty before, set the head appropriately.

	if (m_iHead == -1)
		{
		m_iHead = m_iTail;
		SetEvent(m_hEvent);
		}

	//	Next

	m_iTail = (m_iTail + 1) % m_Array.GetCount();

	LeaveCriticalSection(&m_csLock);

	return NOERROR;

Fail:

	LeaveCriticalSection(&m_csLock);
	return error;
	}

