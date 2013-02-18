//	CEnvironmentGrid.cpp
//
//	CEnvironmentGrid class
//	Copyright (c) 2013 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"

CEnvironmentGrid::CEnvironmentGrid (void) : m_Map(defaultSize, defaultScale)

//	CEnvironmentGrid constructor

	{
	}

void CEnvironmentGrid::ConvertSpaceEnvironmentToPointers (CTileMap &UNIDs, CTileMap *retpPointers)

//	ConvertSpaceEnvironmentToPointers
//
//	Converts m_pEnvironment to pointers (restore from save file)

	{
	retpPointers->Init(UNIDs.GetSize(), UNIDs.GetScale());

	STileMapEnumerator k;
	while (UNIDs.HasMore(k))
		{
		DWORD dwTile;
		int xTile;
		int yTile;

		UNIDs.GetNext(k, &xTile, &yTile, &dwTile);
		CSpaceEnvironmentType *pEnv = g_pUniverse->FindSpaceEnvironment(dwTile);
		if (pEnv)
			retpPointers->SetTile(xTile, yTile, MakeTileDWORD(pEnv));
		}
	}

void CEnvironmentGrid::GetNextTileType (STileMapEnumerator &i, int *retx, int *rety, CSpaceEnvironmentType **retpEnv) const

//	GetNextTileType
//
//	Returns the next tile type

	{
	DWORD dwTile;
	m_Map.GetNext(i, retx, rety, &dwTile);

	if (retpEnv)
		*retpEnv = GetSpaceEnvironmentFromTileDWORD(dwTile);
	}

CSpaceEnvironmentType *CEnvironmentGrid::GetSpaceEnvironmentFromTileDWORD (DWORD dwTile) const

//	GetSpaceEnvironmentFromTileDWORD
//
//	Returns the space environment.

	{
	DWORD dwAtom = (dwTile & 0xffff);
	if (dwAtom == 0)
		return NULL;

	return m_EnvList.atom_GetKey(dwAtom - 1);
	}

CSpaceEnvironmentType *CEnvironmentGrid::GetTileType (int xTile, int yTile)

//	GetTileType
//
//	Returns the space environment type at the given tile.

	{
	return GetSpaceEnvironmentFromTileDWORD(m_Map.GetTile(xTile, yTile));
	}

DWORD CEnvironmentGrid::MakeTileDWORD (CSpaceEnvironmentType *pEnv)

//	MakeTileDWORD
//
//	Encodes into a DWORD for the tile map
//
//	The encoding is as follows:
//
//	33222222222211111111110000000000
//	10987654321098765432109876543210
//	            |  ||    16-bits   |
//				|EE||      SEA     |
//
//	EE = Edge bitmask
//
//	SEA = Space Environment Atom. NOTE: We need to bias by 1 because an atom of
//			0 is valid, but a DWORD of 0 means no tile.

	{
	//	Get the atom for this environment

	DWORD dwAtom = m_EnvList.atom_Find(pEnv);
	if (dwAtom == NULL_ATOM)
		m_EnvList.atom_Insert(pEnv, &dwAtom);

	//	Edge bitmask

	DWORD dwEdge = 0;

	return (dwEdge << 16) | (dwAtom + 1);
	}

void CEnvironmentGrid::ReadFromStream (SLoadCtx &Ctx)

//	ReadFromStream
//
//	Read
//
//	DWORD			No. of atom entries
//	For each entry
//		DWORD		UNID of space environment
//
//	CTileMap		Tile Map

	{
	ALERROR error;
	int i;

	if (Ctx.dwVersion >= 82)
		{
		//	Read the environment list

		m_EnvList.DeleteAll();

		//	Since we saved the list in atom-order, we can reload and expect to
		//	add them and get back the same atom. We rely on the fact that the
		//	atom is the same as the order in which they are added.

		int iCount;
		Ctx.pStream->Read((char *)&iCount, sizeof(DWORD));
		for (i = 0; i < iCount; i++)
			{
			DWORD dwUNID;
			Ctx.pStream->Read((char *)&dwUNID, sizeof(DWORD));

			CSpaceEnvironmentType *pEnv = g_pUniverse->FindSpaceEnvironment(dwUNID);
			DWORD dwAtom;
			m_EnvList.atom_Insert(pEnv, &dwAtom);
			ASSERT(dwAtom == i);
			}

		//	Now load the tile map.

		if (error = m_Map.ReadFromStream(Ctx.pStream))
			throw CException(error);
		}

	//	In old versions we just save the UNID of the environment type for
	//	each tile.

	else
		{
		//	Load into a separate map

		CTileMap Env;
		if (error = Env.ReadFromStream(Ctx.pStream))
			throw CException(error);

		//	Convert to the new storage method

		ConvertSpaceEnvironmentToPointers(Env, &m_Map);
		}
	}

void CEnvironmentGrid::SetTileType (int xTile, int yTile, CSpaceEnvironmentType *pEnv)

//	SetTileType
//
//	Sets a tile.

	{
	m_Map.SetTile(xTile, yTile, MakeTileDWORD(pEnv));
	}

void CEnvironmentGrid::WriteToStream (IWriteStream *pStream)

//	WriteToStream
//
//	Write
//
//	DWORD			No. of atom entries
//	For each entry
//		DWORD		UNID of space environment
//
//	CTileMap		Tile Map

	{
	int i;

	//	Write the entry list. We write out in atom-order. Since we never delete
	//	atoms, we can guarantee that this is a contiguous list.

	DWORD dwSave = m_EnvList.GetCount();
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	for (i = 0; i < m_EnvList.GetCount(); i++)
		{
		CSpaceEnvironmentType *pEnv = m_EnvList.atom_GetKey((DWORD)i);

		dwSave = pEnv->GetUNID();
		pStream->Write((char *)&dwSave, sizeof(DWORD));
		}

	//	Now write the tile map

	m_Map.WriteToStream(pStream);
	}
