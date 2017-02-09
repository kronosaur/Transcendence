//  ExtensionImpl.h
//
//  Helpers for dealing with extensions
//  Copyright (c) 2016 by Kronosaur Productions, LLC. All Rights Reserved.

#pragma once

class CExtensionDirectory;

class CSimpleLibraryResolver : public IXMLParserController
	{
	public:
		CSimpleLibraryResolver (const CExtensionDirectory &Extensions) : 
				m_Extensions(Extensions),
				m_bReportError(false)
			{ }

		ALERROR AddLibrary (DWORD dwUNID, CString *retsError);
		inline void AddTable (IXMLParserController *pTable) { m_Tables.Insert(pTable); }
		inline void ReportLibraryErrors (void) { m_bReportError = true; }

		//	IXMLParserController virtuals
		virtual ALERROR OnOpenTag (CXMLElement *pElement, CString *retsError) override;
		virtual CString ResolveExternalEntity (const CString &sName, bool *retbFound = NULL) override;

	private:
		const CExtensionDirectory &m_Extensions;

		TArray<IXMLParserController *> m_Tables;
		bool m_bReportError;				//	If TRUE, we report errors if we fail to load a library
	};

class CExtensionDirectory
    {
    public:
        struct SExtensionInfo
            {
            DWORD dwUNID;
            EExtensionTypes iType;
            CString sName;
            CString sVersion;
            CString sDesc;
            CString sFilespec;

            TArray<DWORD> Dependencies;
            TArray<CString> Files;
			CString sCoverImage;
            };

        ~CExtensionDirectory (void);

        bool FindByFilespec (const CString &sFilespec, DWORD *retdwUNID = NULL) const;
        bool FindLibraryEntities (DWORD dwUNID, CExternalEntityTable **retpTable) const;
        bool GetExtensionInfo (DWORD dwUNID, SExtensionInfo &retInfo, CString *retsError = NULL);
        bool Init (const CString &sRootPath, const CString &sFilespec, CString *retsError = NULL);

    private:
		struct SImageDesc
			{
			CString sImage;
			};

        struct SExtensionDesc
            {
            SExtensionDesc (void) :
                    dwUNID(INVALID_UNID),
                    iType(extUnknown),
                    bCore(false),
					pGameFile(NULL)
                { }

			~SExtensionDesc (void)
				{
				if (pGameFile)
					delete pGameFile;
				}

            DWORD dwUNID;
            CString sName;
            CString sVersion;
            CString sFilespec;
            EExtensionTypes iType;
    		CExternalEntityTable Entities;
			DWORD dwCoverImage;
            bool bCore;

			CXMLElement *pGameFile;
			TArray<CString> Modules;
			TArray<DWORD> Libraries;
			TSortMap<DWORD, SImageDesc> Images;

            mutable bool bMarked;
            };

        void AddCoreLibraries (CSimpleLibraryResolver &Resolver, SExtensionDesc *pExtension) const;
        void AddLibrary (CSimpleLibraryResolver &Resolver, DWORD dwUNID) const;
        bool InitCore (const CString &sRootPath, CString *retsError = NULL);
		bool InitFolder (const CString &sPath, bool bWarnDuplicates, CString *retsError = NULL);
        bool LoadDependencies (DWORD dwUNID, TArray<DWORD> &Files, CString *retsError = NULL);
        bool LoadExtension (DWORD dwUNID, CString *retsError = NULL);
		bool LoadExtensionStub (const CString &sFilespec, SExtensionDesc **retpExtension, CString *retsError);
		bool LoadExtensionDefinitions (SExtensionDesc *pExtension, CXMLElement *pRoot, CString *retsError);
        bool MarkLibraries (SExtensionDesc *pExtension, CResourceDb &Resources, CXMLElement *pRoot, CString *retsError);
        bool MarkModule (SExtensionDesc *pExtension, CResourceDb &Resources, CXMLElement *pModule, CString *retsError);

        CString m_sRootPath;                //  Path for Transcendence.tdb
        TSortMap<DWORD, SExtensionDesc *> m_Extensions;
    };
