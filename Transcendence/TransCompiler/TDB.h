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
