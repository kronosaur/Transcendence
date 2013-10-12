//	CFileWriteStream.cpp
//
//	Implements CFileWriteStream object

#include "Kernel.h"
#include "KernelObjID.h"

static DATADESCSTRUCT g_DataDesc[] =
	{	{ DATADESC_OPCODE_VTABLE,		1,	0 },		//	IWriteStream virtuals
		{ DATADESC_OPCODE_EMBED_OBJ,	1,	0 },		//	m_sFilename
		{ DATADESC_OPCODE_INT,			1,	0 },		//	m_bUnique
		{ DATADESC_OPCODE_INT,			1,	0 },		//	m_hFile
		{ DATADESC_OPCODE_STOP,	0,	0 } };
static CObjectClass<CFileWriteStream>g_Class(OBJID_CFILEWRITESTREAM, g_DataDesc);

CFileWriteStream::CFileWriteStream (void) :
		CObject(&g_Class)

//	CFileWriteStream constructor

	{
	}

CFileWriteStream::CFileWriteStream (const CString &sFilename, BOOL bUnique) :
		CObject(&g_Class),
		m_sFilename(sFilename),
		m_bUnique(bUnique),
		m_hFile(NULL)

//	CFileWriteStream constructor

	{
	}

CFileWriteStream::~CFileWriteStream (void)

//	CFileWriteStream destructor

	{
	//	Close the file if necessary

	Close();
	}

ALERROR CFileWriteStream::Close (void)

//	CFileWriteStream
//
//	Close the stream

	{
	if (m_hFile == NULL)
		return NOERROR;

	CloseHandle(m_hFile);
	m_hFile = NULL;

	return NOERROR;
	}

ALERROR CFileWriteStream::Create (void)

//	Create
//
//	Creates a new file

	{
	ASSERT(m_hFile == NULL);

	m_hFile = CreateFile(m_sFilename.GetASCIIZPointer(),
			GENERIC_WRITE,
			FILE_SHARE_READ,
			NULL,
			(m_bUnique ? CREATE_NEW : CREATE_ALWAYS),
			FILE_ATTRIBUTE_NORMAL,
			NULL);
	if (m_hFile == INVALID_HANDLE_VALUE)
		{
		m_hFile = NULL;
		return ERR_FAIL;
		}

	return NOERROR;
	}

ALERROR CFileWriteStream::Open (void)

//	Open
//
//	Opens up an existing file for writing.

	{
	ASSERT(m_hFile == NULL);

	m_hFile = CreateFile(m_sFilename.GetASCIIZPointer(),
			GENERIC_WRITE,
			FILE_SHARE_READ,
			NULL,
			OPEN_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,
			NULL);
	if (m_hFile == INVALID_HANDLE_VALUE)
		{
		m_hFile = NULL;
		return ERR_FAIL;
		}

	//	Seek to the end of the file

	DWORD dwPos = ::SetFilePointer(m_hFile, 0, NULL, FILE_END);
	if (dwPos == INVALID_SET_FILE_POINTER)
		{
		::CloseHandle(m_hFile);
		m_hFile = NULL;
		return ERR_FAIL;
		}

	return NOERROR;
	}

ALERROR CFileWriteStream::Write (char *pData, int iLength, int *retiBytesWritten)

//	Write
//
//	Writes the given bytes to the file. If this call returns NOERROR, it is
//	guaranteed that the requested number of bytes were written.

	{
	int iBytesWritten = 0;
	DWORD dwWritten;

	ASSERT(m_hFile);

	//	Pass through the filters

	if (!WriteFile(m_hFile, pData, iLength, &dwWritten, NULL))
		goto Fail;

	iBytesWritten += iLength;

	ASSERT(iBytesWritten == iLength);
	if (retiBytesWritten)
		*retiBytesWritten = iBytesWritten;

	return NOERROR;

Fail:

	if (retiBytesWritten)
		*retiBytesWritten = iBytesWritten;

	return ERR_FAIL;
	}

