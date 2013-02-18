//	CTileMap.cpp
//
//	CTileMap class
//
//	USAGE
//
//	This datastructure allows the caller to store a sparse array of DWORDs (Tiles).
//	The coordinates range is:
//
//		0..(iSize ** (iScale + 1))-1
//
//	The structure consists of nested arrays of (iScale + 1) levels. That is, if
//	iScale = 0, there is a single 2D array of of (iSize, iSize) elements. If
//	iScale = 1, then each element in the first array points to a second 2D array
//	of (iSize, iSize).
//
//	Arrays that have no tiles are not allocated.

#include "PreComp.h"

CTileMap::CTileMap (void) : m_pMap(NULL),
		m_iSize(0),
		m_iScale(0)

//	CTileMap constructor

	{
	}

CTileMap::CTileMap (int iSize, int iScale) : m_pMap(NULL),
		m_iSize(iSize),
		m_iScale(iScale)

//	CTileMap constructor

	{
	ASSERT(iScale >= 0);
	ASSERT(iSize > 1);

	m_pMap = new CTileMapSection(iSize * iSize);

	ComputeDenominator();
	}

CTileMap::~CTileMap (void)

//	CTileMap destructor

	{
	if (m_pMap)
		CleanUpMapSection(m_pMap, m_iScale);
	}

void CTileMap::CleanUpMapSection (CTileMapSection *pMap, int iScale)

//	CleanUpMapSection
//
//	Deletes all nested map sections

	{
	if (iScale > 0)
		{
		int iCount = m_iSize * m_iSize;
		for (int i = 0; i < iCount; i++)
			{
			CTileMapSection *pSection = pMap->GetTileMapSection(i);
			if (pSection)
				CleanUpMapSection(pSection, iScale-1);
			}
		}

	delete pMap;
	}

void CTileMap::ComputeDenominator (void)

//	ComputeDenominator
//
//	Initializes m_iFirstDenominator based on m_iSize and m_iScale

	{
	m_iFirstDenominator = 1;
	for (int i = 0; i < m_iScale; i++)
		m_iFirstDenominator *= m_iSize;
	}

ALERROR CTileMap::CreateFromStream (IReadStream *pStream, CTileMap **retpMap)

//	CreateFromStream
//
//	Read the tile map
//
//	DWORD		m_iSize
//	DWORD		m_iScale
//	CTileMapSection recursion

	{
	ALERROR error;

	CTileMap *pMap = new CTileMap;
	if (error = pMap->ReadFromStream(pStream))
		return error;

	//	Done

	*retpMap = pMap;

	return NOERROR;
	}

void CTileMap::GetNext (STileMapEnumerator &i, int *retx, int *rety, DWORD *retdwTile) const

//	GetNext
//
//	Returns the next valid tile

	{
	ASSERT(!i.bDone && i.pCurPos);

	*retdwTile = i.pCurPos->pMap->GetTile(i.pCurPos->iIndex);

	STileMapSectionPos *pPos = i.pCurPos;
	int iMult = 1;
	*retx = 0;
	*rety = 0;
	while (pPos)
		{
		*retx += iMult * (pPos->iIndex / m_iSize);
		*rety += iMult * (pPos->iIndex % m_iSize);

		pPos = pPos->pParent;
		iMult *= m_iSize;
		}

	SelectNext(i);
	}

DWORD CTileMap::GetTile (int x, int y) const

//	GetTile
//
//	Returns the tile at the given coordinates.

	{
	if (x < 0 || y < 0 || m_pMap == NULL)
		return 0;

	CTileMapSection *pMap = m_pMap;
	int iDenom = m_iFirstDenominator;
	while (iDenom > 1)
		{
		int x1 = x / iDenom;
		int y1 = y / iDenom;

		if (x1 >= m_iSize || y1 >= m_iSize)
			return 0;

		pMap = pMap->GetTileMapSection(x1 * m_iSize + y1);
		if (pMap == NULL)
			return 0;

		x = x % iDenom;
		y = y % iDenom;

		iDenom /= m_iSize;
		}

	return pMap->GetTile(x * m_iSize + y);
	}

int CTileMap::GetTotalSize (void) const

//	GetTotalSize
//
//	Returns the total size of the tile map (per side)

	{
	return m_iFirstDenominator * m_iSize;
	}

bool CTileMap::HasMore (STileMapEnumerator &i) const

//	HasMore
//
//	Returns TRUE if there are more tiles left to enumerate

	{
	if (i.bDone)
		return false;
	else if (i.pCurPos == NULL)
		return SelectNext(i);
	else
		return true;
	}

void CTileMap::Init (int iSize, int iScale)

//	Init
//
//	Initialize

	{
	ASSERT(iScale >= 0);
	ASSERT(iSize >= 0);

	if (m_pMap)
		{
		CleanUpMapSection(m_pMap, m_iScale);
		m_pMap = NULL;
		}

	m_iScale = iScale;
	m_iSize = iSize;
	if (iSize > 0)
		m_pMap = new CTileMapSection(iSize * iSize);

	ComputeDenominator();
	}

ALERROR CTileMap::ReadFromStream (IReadStream *pStream)

//	ReadFromStream
//
//	Read the tile map
//
//	DWORD		m_iSize
//	DWORD		m_iScale
//	CTileMapSection recursion

	{
	ALERROR error;
	int iSize, iScale;
	pStream->Read((char *)&iSize, sizeof(DWORD));
	pStream->Read((char *)&iScale, sizeof(DWORD));

	Init(iSize, iScale);
	int iCount = iSize * iSize;

	//	Load list of map sections

	DWORD dwOffset;
	pStream->Read((char *)&dwOffset, sizeof(DWORD));
	while (dwOffset != 0xffffffff)
		{
		int iDenom = m_iFirstDenominator * m_iFirstDenominator;

		//	Load the map section

		if (iDenom == 1)
			m_pMap->ReadFromStream(iCount, pStream);
		else
			{
			CTileMapSection *pSection = new CTileMapSection(iCount);
			if (error = pSection->ReadFromStream(iCount, pStream))
				return error;

			//	Add the map to the appropriate level

			int iOffset = (int)dwOffset;
			CTileMapSection *pDest = m_pMap;
			while (iDenom > iCount)
				{
				int iIndex = iOffset / iDenom;

				CTileMapSection *pOldMap = pDest;
				pDest = pDest->GetTileMapSection(iIndex);
				if (pDest == NULL)
					{
					pDest = new CTileMapSection(iCount);
					pOldMap->SetTileMapSection(iIndex, pDest);
					}

				iOffset = iOffset % iDenom;
				iDenom = iDenom / iCount;
				}

			pDest->SetTileMapSection(iOffset, pSection);
			}

		if (error = pStream->Read((char *)&dwOffset, sizeof(DWORD)))
			return error;
		}

	return NOERROR;
	}

bool CTileMap::SelectNext (STileMapEnumerator &i) const

//	SelectNext
//
//	Selects the next tile. Returns FALSE if there is no
//	next tile. If i.pCurPos == NULL then we start at the beginning

	{
	int iLevel;

	if (m_pMap == NULL)
		return false;
	else if (i.bDone)
		return false;
	else if (i.pCurPos == NULL)
		{
		i.pCurPos = new STileMapSectionPos;
		i.pCurPos->iIndex = -1;
		i.pCurPos->pMap = m_pMap;
		i.pCurPos->pParent = NULL;
		iLevel = 0;
		}
	else
		iLevel = m_iScale;

	//	Keep looking for the next tile until we reach
	//	the end.

	int iMaxIndex = m_iSize * m_iSize;
	while (true)
		{
		i.pCurPos->iIndex++;
		if (i.pCurPos->iIndex >= iMaxIndex)
			{
			if (iLevel == 0)
				{
				i.bDone = true;
				return false;
				}

			//	Pop-up a level

			STileMapSectionPos *pParent = i.pCurPos->pParent;
			delete i.pCurPos;
			i.pCurPos = pParent;
			iLevel--;
			continue;
			}

		if (iLevel < m_iScale)
			{
			if (i.pCurPos->pMap->GetTileMapSection(i.pCurPos->iIndex))
				{
				//	Go down a level

				STileMapSectionPos *pChild = new STileMapSectionPos;
				pChild->pMap = i.pCurPos->pMap->GetTileMapSection(i.pCurPos->iIndex);
				pChild->iIndex = -1;
				pChild->pParent = i.pCurPos;
				i.pCurPos = pChild;
				iLevel++;
				}
			}
		else if (i.pCurPos->pMap->GetTile(i.pCurPos->iIndex) != 0)
			return true;
		}
	}

void CTileMap::SetTile (int x, int y, DWORD dwTile)

//	SetTile
//
//	Sets the tile at the given coordinates.

	{
	if (x < 0 || y < 0 || m_pMap == NULL)
		return;

	CTileMapSection *pMap = m_pMap;
	int iDenom = m_iFirstDenominator;
	while (iDenom > 1)
		{
		int x1 = x / iDenom;
		int y1 = y / iDenom;

		if (x1 >= m_iSize || y1 >= m_iSize)
			return;

		CTileMapSection *pOldMap = pMap;
		pMap = pMap->GetTileMapSection(x1 * m_iSize + y1);
		if (pMap == NULL)
			{
			pMap = new CTileMapSection(m_iSize * m_iSize);
			pOldMap->SetTileMapSection(x1 * m_iSize + y1, pMap);
			}

		x = x % iDenom;
		y = y % iDenom;

		iDenom /= m_iSize;
		}

	pMap->SetTile(x * m_iSize + y, dwTile);
	}

void CTileMap::WriteMapSection (CTileMapSection *pMap, 
								int iOffset,
								int iDenom, 
								IWriteStream *pStream) const

//	WriteMapSection
//
//	Writes out the map recursively

	{
	if (iDenom == 1)
		{
		pStream->Write((char *)&iOffset, sizeof(DWORD));
		pMap->WriteToStream(m_iSize * m_iSize, pStream);
		}
	else
		{
		int iCount = m_iSize * m_iSize;
		int iNewDenom = iDenom / iCount;
		for (int i = 0; i < iCount; i++)
			{
			CTileMapSection *pSection = pMap->GetTileMapSection(i);
			if (pSection)
				WriteMapSection(pSection, iOffset + i * iNewDenom, iNewDenom, pStream);
			}
		}
	}

void CTileMap::WriteToStream (IWriteStream *pStream) const

//	WriteToStream
//
//	Write the tile map out
//
//	DWORD		m_iSize
//	DWORD		m_iScale
//	CTileMapSection recursion
//	DWORD		0xffffffff;

	{
	pStream->Write((char *)&m_iSize, sizeof(DWORD));
	pStream->Write((char *)&m_iScale, sizeof(DWORD));

	//	Write out each tile at the lowest scale

	if (m_pMap)
		{
		int iCount = m_iSize * m_iSize;
		int iDenom = m_iFirstDenominator * m_iFirstDenominator;
		WriteMapSection(m_pMap, 0, iDenom, pStream);
		}

	//	End marker

	DWORD dwSave = 0xffffffff;
	pStream->Write((char *)&dwSave, sizeof(DWORD));
	}
