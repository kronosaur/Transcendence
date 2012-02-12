//	CFileDirectory.cpp
//
//	Implements CFileDirectory object

#include "Kernel.h"
#include "KernelObjID.h"

CFileDirectory::CFileDirectory (const CString &sFilespec) :
		m_sFilespec(sFilespec),
		m_hSearch(INVALID_HANDLE_VALUE)

//	CFileDirectory constructor

	{
	m_hSearch = ::FindFirstFile(sFilespec.GetASCIIZPointer(), &m_FindData);
	}

CFileDirectory::~CFileDirectory (void)

//	CFileDirectory destructor

	{
	if (m_hSearch != INVALID_HANDLE_VALUE)
		::FindClose(m_hSearch);
	}

bool CFileDirectory::HasMore (void)

//	HasMore
//
//	Returns TRUE if there are more files in the directory

	{
	return (m_hSearch != INVALID_HANDLE_VALUE);
	}

CString CFileDirectory::GetNext (bool *retbIsFolder)

//	GetNext
//
//	Returns the next filename

	{
	CString sFilename;

	ASSERT(m_hSearch != INVALID_HANDLE_VALUE);

	sFilename = CString(m_FindData.cFileName);
	if (retbIsFolder)
		*retbIsFolder = ((m_FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? true : false);

	//	Get the next file

	if (!::FindNextFile(m_hSearch, &m_FindData))
		{
		::FindClose(m_hSearch);
		m_hSearch = INVALID_HANDLE_VALUE;
		}

	//	Done

	return sFilename;
	}

void CFileDirectory::GetNextDesc (SFileDesc *retDesc)

//	GetNextDesc
//
//	Returns the next file descriptor

	{
	ASSERT(m_hSearch != INVALID_HANDLE_VALUE);

	retDesc->sFilename = CString(m_FindData.cFileName);
	retDesc->bFolder = ((m_FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? true : false);
	retDesc->bSystemFile = ((m_FindData.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) ? true : false);
	retDesc->bHiddenFile = ((m_FindData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) ? true : false);
	retDesc->bReadOnly = ((m_FindData.dwFileAttributes & FILE_ATTRIBUTE_READONLY) ? true : false);

	//	Get the next file

	if (!::FindNextFile(m_hSearch, &m_FindData))
		{
		::FindClose(m_hSearch);
		m_hSearch = INVALID_HANDLE_VALUE;
		}
	}

