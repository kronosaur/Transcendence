//	TDBCompiler.h
//
//	CTDBCompiler class
//	Copyright (c) 2015 by Kronosaur Productions, LLC. All Rights Reserved.

#pragma once

enum EFlags
	{
	//	SResourceEntry flags
	FLAG_COMPRESS_ZLIB =		0x00000001,
	};

struct SResourceEntry
	{
	CString sFilename;
	int iEntryID;
	DWORD dwFlags;
	};

class CTDBCompiler
	{
	public:
		CTDBCompiler (void) :
				m_pCoreEntities(NULL),
				m_iErrorCount(0) 
			{ }

		~CTDBCompiler (void)
			{
			for (int i = 0; i < m_CoreEntities.GetCount(); i++)
				delete m_CoreEntities[i];
			}

		void AddEntityTable (CExternalEntityTable *pTable);
		void AddResource (const CString &sName, int iEntryID, bool bCompress);
		inline CExternalEntityTable *GetCoreEntities (void) const { return m_pCoreEntities; }
		inline const CString &GetRootPath (void) const { return m_sRootPath; }
		bool Init (const CString &sInputFilespec, const CString &sOutputFilespec, const TArray<CString> &EntityFilespecs, CXMLElement *pCmdLine, CString *retsError);
		void PopEntityTable (CExternalEntityTable *pOldTable);
		CExternalEntityTable *PushEntityTable (CExternalEntityTable *pNewTable);
		void ReportError (const CString &sError);
		void ReportErrorCount (void);
		bool WriteHeader (IWriteStream &Stream, int iGameFile, CString *retsError);


	private:

	private:
		enum ECompileTypes
			{
			typeNone,

			typeAmericaUniverse,			//	America.xml
			typeAmericaExtension,			//	A CSC America extension of some sort
			typeTranscendenceUniverse,		//	Transcendence.xml
			typeTransExtension,				//	A Transcendence extension of some sort
			};

		bool AddCoreEntityTables (const CString &sCoreTDB, CString *retsError);
		bool ReadEntities (const CString &sFilespec, CExternalEntityTable **retpEntityTable) const;

		CString m_sInputFilespec;
		CString m_sOutputFilespec;

		ECompileTypes m_iType;				//	Type of input file that we're compiling
		CString m_sRootPath;

		TSortMap<CString, SResourceEntry> m_ResourceMap;
		int m_iErrorCount;
		CExternalEntityTable *m_pCoreEntities;
		TArray<CExternalEntityTable *> m_CoreEntities;
	};
