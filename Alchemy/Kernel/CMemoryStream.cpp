//	CMemoryStream.cpp
//
//	Implements CMemoryWriteStream and CMemoryReadStream object

#include "Kernel.h"
#include "KernelObjID.h"

#define ALLOC_SIZE							4096

static CObjectClass<CMemoryWriteStream>g_WriteClass(OBJID_CMEMORYWRITESTREAM, NULL);

static CObjectClass<CMemoryReadStream>g_ReadClass(OBJID_CMEMORYREADSTREAM, NULL);

CMemoryWriteStream::CMemoryWriteStream (int iMaxSize) :
		CObject(&g_WriteClass),
		m_iMaxSize(iMaxSize),
		m_iCommittedSize(0),
		m_iCurrentSize(0),
		m_pBlock(NULL)

//	CMemoryWriteStream constructor

	{
	ASSERT(m_iMaxSize >= 0);
	if (m_iMaxSize == 0)
		m_iMaxSize = DEFAULT_MAX_SIZE;
	}

CMemoryWriteStream::~CMemoryWriteStream (void)

//	CMemoryWriteStream destructor

	{
	//	Close the stream if necessary

	if (m_pBlock)
		{
        VirtualFree(m_pBlock, m_iCommittedSize, MEM_DECOMMIT);
        VirtualFree(m_pBlock, 0, MEM_RELEASE);
		}
	}

ALERROR CMemoryWriteStream::Close (void)

//	CMemoryWriteStream
//
//	Close the stream

	{
	return NOERROR;
	}

ALERROR CMemoryWriteStream::Create (void)

//	Create
//
//	Creates a new file

	{
	//	Reserve a block of memory equal to the maximum size requested

	if (m_pBlock == NULL)
		{
		m_pBlock = (char *)VirtualAlloc(NULL, m_iMaxSize, MEM_RESERVE, PAGE_NOACCESS);
		if (m_pBlock == NULL)
			{
			::kernelDebugLogMessage("Out of Memory: VirtualAlloc failed reserving %d bytes.", m_iMaxSize);
			return ERR_MEMORY;
			}

		m_iCommittedSize = 0;
		}

	//	Initialize

	m_iCurrentSize = 0;

	return NOERROR;
	}

ALERROR CMemoryWriteStream::Write (char *pData, int iLength, int *retiBytesWritten)

//	Write
//
//	Writes the given bytes to the file. If this call returns NOERROR, it is
//	guaranteed that the requested number of bytes were written.

	{
	//	Make sure we called Create

	ASSERT(m_pBlock);
	ASSERT(iLength >= 0);

	//	Commit the required space

	if (m_iCurrentSize + iLength > m_iCommittedSize)
		{
		int iAdditionalSize;

		//	Figure out how much to add

		iAdditionalSize = AlignUp(m_iCurrentSize + iLength, ALLOC_SIZE) - m_iCommittedSize;

		//	Figure out if we're over the limit. We cannot rely on VirtualAlloc
		//	to keep track of our maximum reservation

		if (m_iCommittedSize + iAdditionalSize > m_iMaxSize)
			{
			//	Allocate a new, bigger virtual block

			int iNewMaxSize = (m_iMaxSize < 0x3fff0000 ? m_iMaxSize * 2 : 0x7fff0000);
			char *pNewBlock = (char *)::VirtualAlloc(NULL, iNewMaxSize, MEM_RESERVE, PAGE_NOACCESS);
			if (pNewBlock == NULL)
				{
				::kernelDebugLogMessage("Out of Memory: VirtualAlloc failed reserving %d bytes.", iNewMaxSize);
				return ERR_MEMORY;
				}

			//	Commit and copy the new block

			if (m_iCommittedSize > 0)
				{
				if (::VirtualAlloc(pNewBlock, m_iCommittedSize, MEM_COMMIT, PAGE_READWRITE) == NULL)
					{
					::kernelDebugLogMessage("Out of Memory: VirtualAlloc failed committing %d bytes.", m_iCommittedSize);
					return ERR_MEMORY;
					}

				//	Copy over to the new block

				utlMemCopy(m_pBlock, pNewBlock, m_iCommittedSize);

				//	Free the old block

				::VirtualFree(m_pBlock, m_iCommittedSize, MEM_DECOMMIT);
				}

			//	Free original

			::VirtualFree(m_pBlock, 0, MEM_RELEASE);

			//	Flip over

			m_pBlock = pNewBlock;
			m_iMaxSize = iNewMaxSize;
			}

		//	Commit

		if (VirtualAlloc(m_pBlock + m_iCommittedSize,
				iAdditionalSize,
				MEM_COMMIT,
				PAGE_READWRITE) == NULL)
			{
			::kernelDebugLogMessage("Out of Memory: VirtualAlloc failed committing %d bytes.", m_iCommittedSize + iAdditionalSize);
			return ERR_MEMORY;
			}

		m_iCommittedSize += iAdditionalSize;
		}

	//	Copy the stuff over

	if (pData)
		utlMemCopy(pData, m_pBlock + m_iCurrentSize, iLength);

	m_iCurrentSize += iLength;
	if (retiBytesWritten)
		*retiBytesWritten = iLength;

	return NOERROR;
	}

CMemoryReadStream::CMemoryReadStream (void) :
		CObject(&g_WriteClass)

//	CMemoryReadStream constructor

	{
	}

CMemoryReadStream::CMemoryReadStream (char *pData, int iDataSize) :
		CObject(&g_WriteClass),
		m_pData(pData),
		m_iDataSize(iDataSize)

//	CMemoryReadStream constructor

	{
#ifdef DEBUG
	m_iPos = -1;
#endif
	}

CMemoryReadStream::~CMemoryReadStream (void)

//	CMemoryReadStream destructor

	{
	}

ALERROR CMemoryReadStream::Read (char *pData, int iLength, int *retiBytesRead)

//	Read

	{
	ASSERT(m_iPos >= 0);	//	This happens if we don't Open the stream first
	ASSERT(iLength >= 0);

	ALERROR error = NOERROR;

	//	If we don't have enough data left, read out what we can

	if (m_iPos + iLength > m_iDataSize)
		{
		iLength = m_iDataSize - m_iPos;
		error = ERR_ENDOFFILE;
		}

	//	Copy the stuff over

	if (pData)
		utlMemCopy(m_pData + m_iPos, pData, iLength);
	m_iPos += iLength;
	if (retiBytesRead)
		*retiBytesRead = iLength;

	return error;
	}

//	IWriteStream ----------------------------------------------------------------

ALERROR IWriteStream::WriteChar (char chChar, int iLength)

//	WriteChar
//
//	Write out a sequence of characters

	{
	int i;
	char chBuffer[sizeof(DWORD)];

	for (i = 0; i < sizeof(DWORD); i++)
		chBuffer[i] = chChar;

	while (iLength > 0)
		{
		int iChunk = Min((int)sizeof(DWORD), iLength);
		Write(chBuffer, iChunk);
		iLength -= iChunk;
		}

	return NOERROR;
	}
