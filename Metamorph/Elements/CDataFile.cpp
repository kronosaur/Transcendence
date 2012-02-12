//	CDataFile.cpp
//
//	CDataFile class
//	Copyright (c) 2000 by NeuroHack, Inc. All Rights Reserved.
//
//	This class implements a variable-length record database.
//	Each record is indexed by a 32-bit integer.
//
//	The layout of the data file looks like this:
//
//	+----------------------------+
//	|DWORD: Signature ('ELDF')   |
//	|DWORD: Version              |
//	|DWORD: Block size (bytes)   |
//	|DWORD: Block count          |
//	|DWORD: EntryTable block     |
//	|DWORD: EntryTable size      |
//	|DWORD: FreeTable entry      |
//	|DWORD: Default entry        |
//	+----------------------------+
//	|DWORD[8]: Spare (0)         |
//	+----------------------------+
//	|Block 0                     |
//	|   (Block size bytes long)  |
//	+----------------------------+
//	|Block 1                     |
//	|                            |
//	+----------------------------+
//	...
//	+----------------------------+
//	|Block n                     |
//	|                            |
//	+----------------------------+
//
//	The data file contains 0 or more entries identified by an ID
//	unique within the data file and that never changes regardless
//	of whether the contents and size of the entry changes.
//	Each entry consists of 1 or more contiguous blocks. The number
//	of blocks allocated to each entry is always equal to the minimum
//	number of blocks required to hold the entry data.
//
//	The EntryTable is an array indexed by ID that specifies the
//	block number and size (in bytes) of each entry. Since the
//	EntryTable itself needs to allocate and deallocate items,
//	it keeps the array index of the first free item in the table.
//	Free items in the table use the block number to point to the
//	next free item.
//
//	The FreeTable is an array of pointers to contiguous blocks
//	in the data file that are not allocated to any entry.

#include "Elements.h"

struct FileHeader
	{
	DWORD dwSignature;					//	'ELDF'
	DWORD dwVersion;					//	10
	DWORD dwBlockSize;					//	size of each block in bytes
	DWORD dwBlockCount;					//	number of blocks in file
	DWORD dwEntryTableBlock;			//	first block of entry table entry
	DWORD dwEntryTableSize;				//	size of entry table (bytes)
	DWORD dwFreeTableEntry;				//	ID of free table entry
	DWORD dwDefaultEntry;				//	ID of default entry

	DWORD dwSpare[8];
	};

struct EntryHeader
	{
	DWORD dwID;							//	ID of this entry (-1 if free)
	DWORD dwSize;						//	size of entry in bytes
	};

const DWORD DATAFILE_SIGNATURE	= 'ELDF';
const DWORD DATAFILE_VERSION	= 10;

const int INVALID_BLOCK			= -1;
const int INVALID_ENTRY			= -1;

//	CBlockRunTable Implementation ---------------------------------------------

CDataFile::CBlockRunTable::CBlockRunTable (void) : m_iFirstFree(-1)
	{
	}

int CDataFile::CBlockRunTable::AddRun (const BlockRun &Run)

//	AddRun
//
//	Add a contiguous run of blocks to the table and return the
//	ID for the table entry.

	{
	int iID;

	//	If we haven't got any free items in the table then add
	//	a new entry.

	if (m_iFirstFree == -1)
		{
		iID = m_Table.GetCount();
		m_Table.Insert(Run);
		}

	//	Otherwise, we pull an entry from the free list and
	//	return it.

	else
		{
		iID = m_iFirstFree;
		m_iFirstFree = m_Table[m_iFirstFree].iBlock;
		m_Table[iID] = Run;
		}

	//	Return the ID

	return iID;
	}

void CDataFile::CBlockRunTable::DeleteRun (int iID)

//	DeleteRun
//
//	Deletes a run of blocks from the table

	{
	ASSERT(iID >= 0 && iID < m_Table.GetCount());

	m_Table[iID].iBlock = m_iFirstFree;
	m_Table[iID].iSize = -1;
	m_iFirstFree = iID;
	}

bool CDataFile::CBlockRunTable::FindAdjacentRuns (int iBlock, int iSize, int *retiBefore, int *retiAfter)

//	FindAdjacentRuns
//
//	Looks for runs before or after the given block

	{
	int iBefore = -1;
	int iAfter = -1;
	int i;

	for (i = 0; i < m_Table.GetCount(); i++)
		if (iBlock + iSize == m_Table[i].iBlock)
			iAfter = i;
		else if (iBlock == m_Table[i].iBlock + m_Table[i].iSize)
			iBefore = i;

	//	Done

	*retiBefore = iBefore;
	*retiAfter = iAfter;

	return (iBefore != -1 || iAfter != -1);
	}

int CDataFile::CBlockRunTable::FindRunByBlock (int iBlock)

//	FindRunByBlock
//
//	Returns the ID of the run that starts at the given
//	block number. Returns -1 if not found.

	{
	int i;
	for (i = 0; i < m_Table.GetCount(); i++)
		if (m_Table[i].iBlock == iBlock)
			return i;

	return -1;
	}

int CDataFile::CBlockRunTable::FindRunBySize (int iSize)

//	FindRunBySize
//
//	Returns the ID of the first run found that is equal to
//	or larger than the given size. If no run of the appropriate
//	size is found, the function returns -1.

	{
	ASSERT(iSize >= 0);

	int i;
	int iBestRun = -1;

	for (i = 0; i < m_Table.GetCount(); i++)
		if (m_Table[i].iSize >= iSize)
			if (iBestRun == -1 || m_Table[i].iSize < m_Table[iBestRun].iSize)
				iBestRun = i;

	return iBestRun;
	}

int CDataFile::CBlockRunTable::GetSaveSize (void)

//	GetSaveSize
//
//	Returns the number of bytes required to save this table

	{
	//	DWORD: First free entry
	//	DWORD: Number of entries
	//	
	//	Entry 0
	//	DWORD: Block
	//	DWORD: Size
	//
	//	Entry 1
	//	DWORD: Block
	//	DWORD: Size
	//
	//	...

	return 2 * sizeof(DWORD) + (m_Table.GetCount() * 2 * sizeof(DWORD));
	}

void CDataFile::CBlockRunTable::LoadFromString (const CString &sData)

//	LoadFromString
//
//	Loads the run table from a buffer

	{
	CBuffer Buffer(sData);
	DWORD dwValue;

	Buffer.Read(&dwValue, sizeof(dwValue));
	m_iFirstFree = (int)dwValue;

	int iCount;
	Buffer.Read(&iCount, sizeof(int));

	for (int i = 0; i < iCount; i++)
		{
		BlockRun Run;

		Buffer.Read(&Run, sizeof(Run));
		m_Table.Insert(Run);
		}
	}

CString CDataFile::CBlockRunTable::SaveToString (void)

//	SaveToString
//
//	Saves the run table to a buffer

	{
	CString sData;
	sData.GetWritePointer(GetSaveSize());
	CBuffer Buffer(sData);

	DWORD dwValue = (DWORD)m_iFirstFree;
	Buffer.Write(&dwValue, sizeof(dwValue));

	dwValue = m_Table.GetCount();
	Buffer.Write(&dwValue, sizeof(dwValue));

	Buffer.Write(&m_Table[0], 2 * sizeof(DWORD) * m_Table.GetCount());

	return sData;
	}

//	CDataFile Implementation --------------------------------------------------

CDataFile::CDataFile (const CDataFile &sDataFile)

//	CDataFile copy constructor

	{
	m_pFile = sDataFile.m_pFile;
	if (m_pFile)
		m_pFile->iRefCount++;
	}

CDataFile::~CDataFile (void)

//	CDataFile destructor

	{
	Close();
	}

CDataFile &CDataFile::operator= (const CDataFile &sDataFile)

//	CDataFile operator =

	{
	Close();

	m_pFile = sDataFile.m_pFile;
	if (m_pFile)
		m_pFile->iRefCount++;

	return *this;
	}

int CDataFile::AddEntry (const CString &Data)

//	AddEntry
//
//	Adds an entry

	{
	ASSERT(m_pFile);

	//	Allocate a new entry in the table

	BlockRun Dummy;
	int iID = m_pFile->EntryTable.AddRun(Dummy);
	m_pFile->bEntryTableModified = true;

	//	Allocate a new run of blocks to hold the new entry

	int iBlock = AllocBlocks(iID, Data.GetLength());

	//	Update the entry table now that we know the block

	BlockRun &Run = m_pFile->EntryTable.GetRun(iID);
	Run.iBlock = iBlock;
	Run.iSize = Data.GetLength();

	//	Write the entry

	WriteEntryData(iBlock, Data);

	//	Done

	return iID;
	}

int CDataFile::AllocEntry (void)

//	AllocEntry
//
//	Allocates an entry without writing or reserving blocks

	{
	ASSERT(m_pFile);

	BlockRun Dummy;
	Dummy.iBlock = -1;
	Dummy.iSize = -1;
	int iID = m_pFile->EntryTable.AddRun(Dummy);
	m_pFile->bEntryTableModified = true;

	return iID;
	}

int CDataFile::AllocBlocks (int iID, int iSize)

//	AllocBlocks
//
//	Allocates enough continguous blocks to hold data of the
//	given size. Returns the initial block number. Note that
//	the block header and the free table will be initialized
//	properly after this call.

	{
	int iBlock;
	int iAlignedSize = AlignUp(iSize + sizeof(EntryHeader), m_pFile->iBlockSize);

	//	First look for a free run of blocks that might fit

	int iFreeRun = m_pFile->FreeTable.FindRunBySize(iAlignedSize);
	if (iFreeRun != -1)
		{
		BlockRun &FreeRun = m_pFile->FreeTable.GetRun(iFreeRun);
		iBlock = FreeRun.iBlock;

		//	If there is any free space left, update the free table

		if (FreeRun.iSize > iAlignedSize)
			{
			FreeRun.iBlock += iAlignedSize;
			FreeRun.iSize -= iAlignedSize;

			//	Update the header for the new block

			WriteEntryHeader(FreeRun.iBlock, -1, FreeRun.iSize);
			}

		//	Otherwise delete this entry from the free table

		else
			m_pFile->FreeTable.DeleteRun(iFreeRun);

		m_pFile->bFreeTableModified = true;
		}

	//	If we didn't find any free space of the appropriate size
	//	then allocate the space from the end of the file.

	else
		{
		iBlock = m_pFile->iBlockCount * m_pFile->iBlockSize;
		m_pFile->iBlockCount += (iAlignedSize / m_pFile->iBlockSize);

		m_pFile->bHeaderModified = true;
		}

	//	Write the header for the new block

	WriteEntryHeader(iBlock, iID, iSize);

	//	Done

	return iBlock;
	}

void CDataFile::Close (void)

//	Close
//
//	Closes this reference

	{
	if (m_pFile && (--m_pFile->iRefCount == 0))
		{
		Flush();

		//	Clean up

		::CloseHandle(m_pFile->hFile);

		//	Delete the structure

		delete m_pFile;
		m_pFile = NULL;
		}
	}

void CDataFile::Create (const CString &sFilename, int iBlockSize)

//	Create
//
//	Creates a new data file using the given filename

	{
	ASSERT(iBlockSize > 0);

	//	Create the file

	HANDLE hFile = ::CreateFile(sFilename,
			GENERIC_READ | GENERIC_WRITE,
			0,
			NULL,
			CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,
			NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		throw errOSError();

	//	Create an EntryTable with two entries. One for the
	//	entry table and one for the free table.

	CBlockRunTable EntryTable;
	BlockRun Dummy;
	int iEntryTableID = EntryTable.AddRun(Dummy);
	int iFreeTableID = EntryTable.AddRun(Dummy);
	int iEntryTableSize = EntryTable.GetSaveSize();

	//	Now fill-in the appropriate entries

	BlockRun &Run = EntryTable.GetRun(iEntryTableID);
	Run.iBlock = 0;
	Run.iSize = iEntryTableSize;

	//	Create a blank FreeTable

	CBlockRunTable FreeTable;
	BlockRun &FreeRun = EntryTable.GetRun(iFreeTableID);
	FreeRun.iBlock = AlignUp(iEntryTableSize + sizeof(EntryHeader), iBlockSize);
	FreeRun.iSize = FreeTable.GetSaveSize();

	//	Prepare the file header

	FileHeader header;
	::ZeroMemory(&header, sizeof(header));
	header.dwSignature = DATAFILE_SIGNATURE;
	header.dwVersion = DATAFILE_VERSION;
	header.dwBlockCount = (DWORD)(FreeRun.iBlock + AlignUp(FreeRun.iSize + sizeof(EntryHeader), iBlockSize)) / iBlockSize;
	header.dwBlockSize = (DWORD)iBlockSize;
	header.dwEntryTableBlock = (DWORD)0;
	header.dwEntryTableSize = (DWORD)iEntryTableSize;
	header.dwFreeTableEntry = (DWORD)iFreeTableID;
	header.dwDefaultEntry = (DWORD)INVALID_ENTRY;

	//	Write the header

	DWORD dwWritten;
	if (!::WriteFile(hFile, &header, sizeof(header), &dwWritten, NULL) || dwWritten != sizeof(header))
		{
		::CloseHandle(hFile);
		throw errOSError();
		}

	//	Write the entry header

	EntryHeader entry;
	entry.dwID = (DWORD)iEntryTableID;
	entry.dwSize = (DWORD)iEntryTableSize;
	if (!::WriteFile(hFile, 
			&entry, 
			sizeof(entry),
			&dwWritten,
			NULL) || dwWritten != (DWORD)sizeof(entry))
		{
		::CloseHandle(hFile);
		throw errOSError();
		}

	//	Write the entry table

	CString sBuffer = EntryTable.SaveToString();
	if (!::WriteFile(hFile, 
			sBuffer.GetASCIIZ(), 
			sBuffer.GetLength(),
			&dwWritten,
			NULL) || dwWritten != (DWORD)iEntryTableSize)
		{
		::CloseHandle(hFile);
		throw errOSError();
		}

	//	Write the free table

	if (::SetFilePointer(hFile,
			sizeof(FileHeader) + FreeRun.iBlock,
			NULL, 
			FILE_BEGIN) == 0xFFFFFFFF)
		{
		::CloseHandle(hFile);
		throw errOSError();
		}

	//	Write the entry header

	entry.dwID = (DWORD)FreeRun.iBlock;
	entry.dwSize = (DWORD)FreeRun.iSize;
	if (!::WriteFile(hFile, 
			&entry, 
			sizeof(entry),
			&dwWritten,
			NULL) || dwWritten != (DWORD)sizeof(entry))
		{
		::CloseHandle(hFile);
		throw errOSError();
		}

	//	Write the free table data

	sBuffer = FreeTable.SaveToString();
	if (!::WriteFile(hFile,
			sBuffer.GetASCIIZ(),
			sBuffer.GetLength(),
			&dwWritten,
			NULL) || dwWritten != (DWORD)FreeRun.iSize)
		{
		::CloseHandle(hFile);
		throw errOSError();
		}

	//	Done

	::CloseHandle(hFile);
	}

void CDataFile::DeleteEntry (int iID)

//	DeleteEntry
//
//	Deletes an entry

	{
	ASSERT(m_pFile);

	BlockRun &Run = m_pFile->EntryTable.GetRun(iID);

	//	Free these blocks

	FreeBlocks(Run.iBlock, Run.iSize);

	//	Remove the entry from the table

	m_pFile->EntryTable.DeleteRun(iID);
	m_pFile->bEntryTableModified = true;
	}

void CDataFile::Flush (void)

//	Flush
//
//	Flushes all changes to disk

	{
	ASSERT(m_pFile);

	//	If we need to save out the entry table then allocate enough
	//	space for it. Since the entry table is itself an entry, saving
	//	it out may modify the free table.

	if (m_pFile->bEntryTableModified)
		ResizeEntry(0, m_pFile->EntryTable.GetSaveSize());

	//	No matter what happens after this point, the entry table is
	//	guaranteed not to grow (although it may change).

	//	If the free table is modified, write it out

	if (m_pFile->bFreeTableModified)
		{
		//	Keep resizing until it fits. We need to do this because
		//	resizing the space for the free table may modify the
		//	free table!

		int iFreeTableSize;

		do
			{
			iFreeTableSize = m_pFile->FreeTable.GetSaveSize();
			ResizeEntry(m_pFile->iFreeTableEntry, iFreeTableSize);
			}
		while (AlignUp(iFreeTableSize, m_pFile->iBlockSize) < m_pFile->FreeTable.GetSaveSize());

		//	Now we can write it out

		WriteEntryData(m_pFile->EntryTable.GetRun(m_pFile->iFreeTableEntry).iBlock,
				m_pFile->FreeTable.SaveToString());

		m_pFile->bFreeTableModified = false;
		}

	//	Now write out the entry table

	if (m_pFile->bEntryTableModified)
		{
		WriteEntryData(m_pFile->EntryTable.GetRun(0).iBlock, 
				m_pFile->EntryTable.SaveToString());

		m_pFile->bEntryTableModified = false;
		m_pFile->bHeaderModified = true;
		}

	//	Write out the header, if necessary

	if (m_pFile->bHeaderModified)
		{
		FileHeader header;
		::ZeroMemory(&header, sizeof(header));
		header.dwSignature = DATAFILE_SIGNATURE;
		header.dwVersion = DATAFILE_VERSION;
		header.dwBlockCount = (DWORD)m_pFile->iBlockCount;
		header.dwBlockSize = (DWORD)m_pFile->iBlockSize;
		header.dwEntryTableBlock = (DWORD)m_pFile->EntryTable.GetRun(0).iBlock;
		header.dwEntryTableSize = (DWORD)m_pFile->EntryTable.GetRun(0).iSize;
		header.dwFreeTableEntry = (DWORD)m_pFile->iFreeTableEntry;
		header.dwDefaultEntry = (DWORD)m_pFile->iDefaultEntry;

		//	Write the header

		if (::SetFilePointer(m_pFile->hFile,
				0,
				NULL, 
				FILE_BEGIN) == 0xFFFFFFFF)
			throw errOSError();

		DWORD dwWritten;
		if (!::WriteFile(m_pFile->hFile, &header, sizeof(header), &dwWritten, NULL) || dwWritten != sizeof(header))
			throw errOSError();

		m_pFile->bHeaderModified = false;
		}
	}

void CDataFile::FreeBlocks (int iBlock, int iSize)

//	FreeBlocks
//
//	Frees the given blocks. Updates the entry header and the
//	free table.

	{
	ASSERT(iBlock == AlignUp(iBlock, m_pFile->iBlockSize));

	iSize = AlignUp(iSize, m_pFile->iBlockSize);

	//	See if there is a free block before or after this

	int iBefore, iAfter;
	if (m_pFile->FreeTable.FindAdjacentRuns(iBlock, iSize, &iBefore, &iAfter))
		{
		BlockRun &BeforeRun = m_pFile->FreeTable.GetRun(iBefore);
		BlockRun &AfterRun = m_pFile->FreeTable.GetRun(iAfter);

		//	If there are free blocks both before and after this block then
		//	coalesce them all.

		if (iBefore != -1 && iAfter != -1)
			{
			BeforeRun.iSize += iSize + AfterRun.iSize;
			m_pFile->FreeTable.DeleteRun(iAfter);

			//	Write the header

			WriteEntryHeader(BeforeRun.iBlock, -1, BeforeRun.iSize);
			}

		//	If we've got a free run before, deal with that one

		else if (iBefore != -1)
			{
			BeforeRun.iSize += iSize;

			//	Write the header

			WriteEntryHeader(BeforeRun.iBlock, -1, BeforeRun.iSize);
			}

		//	Otherwise, the run afterwards is free

		else if (iAfter != -1)
			{
			AfterRun.iBlock = iBlock;
			AfterRun.iSize += iSize;

			//	Write the header

			WriteEntryHeader(AfterRun.iBlock, -1, AfterRun.iSize);
			}
		else
			ASSERT(false);
		}

	//	Otherwise, just add these blocks to the free list

	else
		{
		BlockRun FreeRun;
		FreeRun.iBlock = iBlock;
		FreeRun.iSize = iSize;

		m_pFile->FreeTable.AddRun(FreeRun);

		//	Write the header

		WriteEntryHeader(iBlock, -1, iSize);
		}

	m_pFile->bFreeTableModified = true;
	}

int CDataFile::GetBlockFilePos (int iBlock)

//	GetBlockFilePos
//
//	Returns the file position of the given block

	{
	return sizeof(FileHeader) + iBlock;
	}

int CDataFile::GetDefaultEntry (void)

//	GetDefaultEntry
//
//	Returns the default entry

	{
	ASSERT(m_pFile);

	return m_pFile->iDefaultEntry;
	}

int CDataFile::GetEntryLength (int iID)

//	GetEntryLength
//
//	Returns the length (in bytes) of the given entry

	{
	ASSERT(m_pFile);

	return m_pFile->EntryTable.GetRun(iID).iSize;
	}

int CDataFile::GetNext (Iterator &Pos)

//	GetNext
//
//	Returns the next ID in the iteration

	{
	int iPos = Pos.iPos;

	Pos.iPos++;
	while (Pos.iPos < m_pFile->EntryTable.GetCount()
			&& !m_pFile->EntryTable.IsRunValid(Pos.iPos))
		Pos.iPos++;

	return iPos;
	}

bool CDataFile::HasMore (Iterator &Pos)

//	HasMore
//
//	Returns TRUE if there are more entries to enumerate

	{
	return (Pos.iPos < m_pFile->EntryTable.GetCount());
	}

CException CDataFile::InvalidFileError (const CString &sFilename)

//	InvalidFileError
//
//	Returns an error indicating that the file format
//	is invalid. This generally implies either corruption or
//	user error (user specifying the wrong file).

	{
	return CException(errFail,
			strPatternSubst("%s is not a valid data file.", sFilename));
	}

CDataFile CDataFile::Open (const CString &sFilename)

//	Open
//
//	Opens the data file and returns an object representation

	{
	CDataFile NewDataFile;
	DataFileInternals *pFile;

	try
		{
		//	Allocate internal structure

		pFile = new DataFileInternals;

		pFile->iRefCount = 1;
		pFile->sFilename = sFilename;

		//	Open the file and read the header

		pFile->hFile = ::CreateFile(sFilename,
				GENERIC_READ | GENERIC_WRITE,
				0,
				NULL,
				OPEN_EXISTING,
				FILE_ATTRIBUTE_NORMAL,
				NULL);
		if (pFile->hFile == INVALID_HANDLE_VALUE)
			throw errOSError(sFilename);

		//	Read the file header

		FileHeader header;
		DWORD dwRead;
		if (!::ReadFile(pFile->hFile, &header, sizeof(header), &dwRead, NULL) || dwRead != sizeof(header))
			throw InvalidFileError(sFilename);

		//	If the signature is not right, bail

		if (header.dwSignature != DATAFILE_SIGNATURE)
			throw InvalidFileError(sFilename);

		//	If the version is not right, bail

		if (header.dwVersion != DATAFILE_VERSION)
			throw InvalidFileError(sFilename);

		//	Store header info

		pFile->iBlockSize = (int)header.dwBlockSize;
		pFile->iBlockCount = (int)header.dwBlockCount;
		pFile->iDefaultEntry = (int)header.dwDefaultEntry;
		pFile->iFreeTableEntry = (int)header.dwFreeTableEntry;

		//	Initialize our local variable so that we can
		//	start calling methods

		NewDataFile.m_pFile = pFile;

		//	Load the entry table. This is a bootstrap technique
		//	because we store the entry table in an entry. Since we
		//	can't use the entry table to load it, we store the
		//	size of the entry table in the file header.

		if (header.dwEntryTableBlock != INVALID_BLOCK)
			{
			//	Read the table entry

			CString sEntry;
			NewDataFile.ReadEntryData(
					header.dwEntryTableBlock,
					header.dwEntryTableSize,
					&sEntry);

			//	Now load the table

			pFile->EntryTable.LoadFromString(sEntry);
			}
		else
			throw InvalidFileError(sFilename);

		//	Load the free table

		if (header.dwFreeTableEntry != INVALID_ENTRY)
			{
			CString sEntry = NewDataFile.ReadEntry(pFile->iFreeTableEntry);
			pFile->FreeTable.LoadFromString(sEntry);
			}
		else
			throw InvalidFileError(sFilename);

		//	Initialize modification flags

		pFile->bHeaderModified = false;
		pFile->bEntryTableModified = false;
		pFile->bFreeTableModified = false;
		}
	catch (...)
		{
		if (pFile)
			{
			if (pFile->hFile != INVALID_HANDLE_VALUE)
				::CloseHandle(pFile->hFile);

			delete pFile;
			}
		}

	return NewDataFile;
	}

CString CDataFile::ReadEntry (int iID)

//	ReadEntry
//
//	Reads and returns the given entry

	{
	ASSERT(m_pFile);

	BlockRun &Header = m_pFile->EntryTable.GetRun(iID);
	CString sData;
	ReadEntryData(Header.iBlock, Header.iSize, &sData);
	return sData;
	}

void CDataFile::ReadEntry (int iID, CString *retsData)

//	ReadEntry
//
//	Reads and returns the given entry

	{
	ASSERT(m_pFile);

	BlockRun &Header = m_pFile->EntryTable.GetRun(iID);
	ReadEntryData(Header.iBlock, Header.iSize, retsData);
	}

void CDataFile::ReadEntryData (int iBlock, int iSize, CString *retsData)

//	ReadEntryData
//
//	Reads an entry by block number

	{
	//	Figure out where the block is stored on disk. Note that we skip
	//	the entry header because we don't need it at read time.

	int iFilePos = GetBlockFilePos(iBlock) + sizeof(EntryHeader);

	//	Seek to the appropriate point on disk.

	if (::SetFilePointer(m_pFile->hFile,
			iFilePos,
			NULL, 
			FILE_BEGIN) == 0xFFFFFFFF)
		throw InvalidFileError();

	//	Now read the data into a string

	char *pDest = retsData->GetWritePointer(iSize);

	DWORD dwRead;
	if (!::ReadFile(m_pFile->hFile, 
			pDest, 
			iSize, 
			&dwRead, 
			NULL) || dwRead != (DWORD)iSize)
		throw InvalidFileError();
	}

void CDataFile::Reset(Iterator &Pos)

//	Reset
//
//	Resets the iterator (the next call to GetNext will return the
//	first entry ID)

	{
	Pos.iPos = 2;
	while (Pos.iPos < m_pFile->EntryTable.GetCount()
			&& !m_pFile->EntryTable.IsRunValid(Pos.iPos))
		Pos.iPos++;
	}

void CDataFile::ResizeEntry (int iID, int iSize)

//	ResizeEntry
//
//	Resizes the entry without actually writing any data. Note that
//	the original data is not preserved.

	{
	ASSERT(m_pFile);

	WriteEntryInt(iID, NULL, iSize);
	}

void CDataFile::SetDefaultEntry (int iID)

//	SetDefaultEntry
//
//	Sets the default entry

	{
	ASSERT(m_pFile);

	m_pFile->iDefaultEntry = iID;
	m_pFile->bHeaderModified = true;
	}

void CDataFile::WriteEntry (int iID, const CString &Data)

//	WriteEntry
//
//	Writes the entry from the given string

	{
	ASSERT(m_pFile);

	WriteEntryInt(iID, Data.GetASCIIZ(), Data.GetLength());
	}

void CDataFile::WriteEntryData (int iBlock, char *pData, int iLength)

//	WriteEntryData
//
//	Writes an entry at the given block number. This function assumes
//	that the blocks have been allocated appropriately.

	{
	//	Figure out where the block is stored on disk. Note that we skip
	//	the entry header because we assume that it is properly set

	int iFilePos = GetBlockFilePos(iBlock) + sizeof(EntryHeader);

	//	Seek to the appropriate point on disk.

	if (::SetFilePointer(m_pFile->hFile,
			iFilePos,
			NULL, 
			FILE_BEGIN) == 0xFFFFFFFF)
		throw errOSError(m_pFile->sFilename);

	//	Write the header

	DWORD dwWritten;
	if (!::WriteFile(m_pFile->hFile, 
			pData, 
			iLength, 
			&dwWritten, 
			NULL) || dwWritten != (DWORD)iLength)
		throw errOSError(m_pFile->sFilename);
	}

void CDataFile::WriteEntryHeader (int iBlock, int iID, int iSize)

//	WriteEntryHeader
//
//	Writes the entry header for an entry

	{
	EntryHeader header;
	header.dwID = (DWORD)iID;
	header.dwSize = (DWORD)iSize;

	//	Seek to the appropriate point on disk.

	if (::SetFilePointer(m_pFile->hFile,
			GetBlockFilePos(iBlock),
			NULL, 
			FILE_BEGIN) == 0xFFFFFFFF)
		throw errOSError(m_pFile->sFilename);

	//	Write the header

	DWORD dwWritten;
	if (!::WriteFile(m_pFile->hFile, &header, sizeof(header), &dwWritten, NULL) || dwWritten != sizeof(header))
		throw errOSError(m_pFile->sFilename);
	}

void CDataFile::WriteEntryInt (int iID, char *pData, int iLength)

//	WriteEntryInt
//
//	Write the entry with optional data

	{
	BlockRun &Run = m_pFile->EntryTable.GetRun(iID);
	int iAlignedCurrentSize = AlignUp(Run.iSize, m_pFile->iBlockSize);
	int iAlignedNewSize = AlignUp(iLength, m_pFile->iBlockSize);

	//	First figure out if the entry fits in the current slot

	if (iLength <= iAlignedCurrentSize)
		{
		//	Write the new entry header (if necessary)

		if (Run.iSize != iLength)
			{
			WriteEntryHeader(Run.iBlock, iID, iLength);
			Run.iSize = iLength;
			m_pFile->bEntryTableModified = true;
			}

		//	Write the data

		if (pData)
			WriteEntryData(Run.iBlock, pData, iLength);

		//	See if we've got left-over blocks at the end

		if (iAlignedNewSize < iAlignedCurrentSize)
			FreeBlocks(Run.iBlock + iAlignedNewSize, iAlignedCurrentSize - iAlignedNewSize);
		}

	//	Otherwise, free the current block and allocate a new one

	else
		{
		if (Run.iBlock != -1)
			FreeBlocks(Run.iBlock, Run.iSize);

		Run.iBlock = AllocBlocks(iID, iLength);
		Run.iSize = iLength;
		m_pFile->bEntryTableModified = true;

		//	Write the entry

		if (pData)
			WriteEntryData(Run.iBlock, pData, iLength);
		}
	}

//	CDataFile Test Code -------------------------------------------------------

bool CDataFile::TEST (ITextLog &Output)

//	TEST
//
//	Tests the code. Returns true for success

	{
	int i;

	Output.Log("CDataFile Test");

	//	Create a data file

	Output.Log("Test creating a data file");
	CDataFile::Create(CONSTLIT("CDataFileTest.dat"));
	Output.Log("File created: BlockSize = 64");

	CDataFile Test = CDataFile::Open(CONSTLIT("CDataFileTest.dat"));
	Output.Log("File opened");

	Test.Close();
	Output.Log("File closed");
	Output.Log("Test successful");

	//	Add some entries

	Output.Log("Test adding some entries");
	Test = CDataFile::Open(CONSTLIT("CDataFileTest.dat"));
	Output.Log("File opened");

	CString sData;
	for (i = 0; i < 100; i++)
		{
		sData.GetWritePointer(mathRandom(10,2048));
		Test.AddEntry(sData);
		}

	//	Change some entries

	for (i = 0; i < 100; i++)
		{
		int iID = mathRandom(10, 80);
		sData.GetWritePointer(mathRandom(10, 2048));
		Test.WriteEntry(iID, sData);
		}

	Test.Close();
	Output.Log("File closed");
	Output.Log("Test successful");

	return true;
	}
