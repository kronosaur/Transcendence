//	CFileReadStream.cpp
//
//	Implements CFileReadStream object

#include "Kernel.h"
#include "KernelObjID.h"

static DATADESCSTRUCT g_DataDesc[] =
	{	{ DATADESC_OPCODE_VTABLE,		1,	0 },		//	IGAWriteStream virtuals
		{ DATADESC_OPCODE_EMBED_OBJ,	1,	0 },		//	m_sFilename
		{ DATADESC_OPCODE_INT,			2,	0 },		//	m_hFile, m_hFileMap
		{ DATADESC_OPCODE_REFERENCE,	2,	0 },		//	m_pFile, m_pPos
		{ DATADESC_OPCODE_INT,			1,	0 },		//	m_dwFileSize
		{ DATADESC_OPCODE_STOP,	0,	0 } };
static CObjectClass<CFileReadStream>g_Class(OBJID_CFILEREADSTREAM, g_DataDesc);

CFileReadStream::CFileReadStream (void) :
		CObject(&g_Class)

//	CFileReadStream constructor

	{
	}

CFileReadStream::CFileReadStream (const CString &sFilename) :
		CObject(&g_Class),
		m_sFilename(sFilename),
		m_hFile(NULL),
		m_dwFileSize(0)

//	CFileReadStream constructor

	{
	}

CFileReadStream::~CFileReadStream (void)

//	CFileReadStream destructor

	{
	//	Close the file if necessary

	Close();
	}

ALERROR CFileReadStream::Close (void)

//	CFileReadStream
//
//	Close the stream

	{
	if (m_hFile == NULL)
		return NOERROR;

	//	Close the file

	UnmapViewOfFile(m_pFile);
	CloseHandle(m_hFileMap);
	CloseHandle(m_hFile);
	m_hFile = NULL;

	return NOERROR;
	}

ALERROR CFileReadStream::Open (void)

//	Open
//
//	Opens the stream for reading

	{
	ASSERT(m_hFile == NULL);

	m_hFile = CreateFile(m_sFilename.GetASCIIZPointer(),
			GENERIC_READ,
			0,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL);
	if (m_hFile == INVALID_HANDLE_VALUE)
		{
		m_hFile = NULL;
		return ERR_FAIL;
		}

	//	Open a file mapping

	m_hFileMap = CreateFileMapping(m_hFile,
			NULL,
			PAGE_READONLY,
			0,
			0,
			NULL);
	if (m_hFileMap == INVALID_HANDLE_VALUE)
		{
		CloseHandle(m_hFile);
		m_hFile = NULL;
		return ERR_FAIL;
		}

	//	Map a view of the file

	m_pFile = (char *)MapViewOfFile(m_hFileMap,
			FILE_MAP_READ,
			0,
			0,
			0);
	if (m_pFile == NULL)
		{
		CloseHandle(m_hFileMap);
		CloseHandle(m_hFile);
		m_hFile = NULL;
		return ERR_FAIL;
		}

	//	Figure out the size of the file

	m_dwFileSize = ::GetFileSize(m_hFile, NULL);
	m_pPos = m_pFile;

	return NOERROR;
	}

ALERROR CFileReadStream::Read (char *pData, int iLength, int *retiBytesRead)

//	Read
//
//	Reads from the file. If this call returns NOERROR, it is
//	guaranteed that the requested number of bytes were read.

	{
	int iBytesRead = 0;
	int iBytesLeft = 0;
	int iWritten;

	ASSERT(m_hFile);
	iBytesLeft = (int)m_dwFileSize - (m_pPos - m_pFile);

	iWritten = min(iBytesLeft, iLength);

	utlMemCopy(m_pPos, pData, iWritten);

	m_pPos += iWritten;
	iBytesRead += iWritten;

	if (retiBytesRead)
		*retiBytesRead = iBytesRead;

	if (iBytesRead < iLength)
		return ERR_ENDOFFILE;
	else
		return NOERROR;
	}

