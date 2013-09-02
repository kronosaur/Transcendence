//	TDB.h
//
//	Transcendence Database Files
//	Copyright (c) 2012 by Kronosaur Productions, LLC. All Rights Reserved.

#pragma once

class CTDBFile
	{
	public:
		enum Flags
			{
			//	Open flags
			FLAG_READ_ONLY =			0x00000001,
			};

		CTDBFile (void) : m_Db(NULL_STR) { }
		~CTDBFile (void);

		void Close (void);
		inline int GetResourceCount (void) const { return m_ResourceMap.GetCount(); }
		inline const CString &GetResourceName (int iIndex) const { return m_ResourceMap.GetKey(iIndex); }
		ALERROR Open (const CString &sFilespec, DWORD dwFlags);

	private:
		CDataFile m_Db;

		int m_iGameFile;					//	Main XML file
		CString m_sTitle;					//	Title
		TSortMap<CString, int> m_ResourceMap;	//	Map of resource filename to entryID.
	};

class CLibraryResolver : public IXMLParserController
	{
	public:
		~CLibraryResolver (void);

		inline void AddEntities (CExternalEntityTable *pEntities) { m_Tables.Insert(pEntities); }

		//	IXMLParserController virtuals
		virtual CString ResolveExternalEntity (const CString &sName, bool *retbFound = NULL);

	private:
		TArray<CExternalEntityTable *> m_Tables;
	};