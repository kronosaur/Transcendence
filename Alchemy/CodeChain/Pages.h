//	Pages.h
//
//	Implementation of pages
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved

enum EPageEnumTypes
	{
	enumUnknown,
	enumTextLine,
	};

struct SPageEnumCtx
	{
	SPageEnumCtx (void) : iType(enumUnknown) { }

	EPageEnumTypes iType;
	DWORD dwPos1;
	DWORD dwPos2;
	};

class IPage
	{
	public:
		virtual ~IPage (void) { }

		virtual bool EnumHasMore (SPageEnumCtx &Ctx) { return false; }
		virtual ICCItem *EnumGetNext (CCodeChain &CC, SPageEnumCtx &Ctx) { return NULL; }
		virtual ALERROR EnumReset (CCodeChain &CC, ICCItem *pEnumType, SPageEnumCtx &Ctx, CString *retsError = NULL) { return ERR_FAIL; }
		virtual ALERROR Open (const CString &sAddress, CString *retsError = NULL) { return ERR_FAIL; }
	};

class CTextFilePage : public IPage
	{
	public:
		CTextFilePage (void) : m_pFile(NULL) { }
		virtual ~CTextFilePage (void);

		//	IPage virtuals
		virtual bool EnumHasMore (SPageEnumCtx &Ctx); 
		virtual ICCItem *EnumGetNext (CCodeChain &CC, SPageEnumCtx &Ctx);
		virtual ALERROR EnumReset (CCodeChain &CC, ICCItem *pEnumType, SPageEnumCtx &Ctx, CString *retsError = NULL);
		virtual ALERROR Open (const CString &sAddress, CString *retsError = NULL);

	private:
		CFileReadBlock *m_pFile;
	};

class CPageManager
	{
	public:
		void ClosePage (IPage *pPage);
		ALERROR OpenPage (const CString &sAddress, IPage **retpPage, CString *retsError = NULL);

	private:
		struct SEntry
			{
			IPage *pPage;
			CString sAddress;
			int iRefCount;
			};

		bool FindEntry (IPage *pPage, int *retiIndex);
		bool FindEntry (const CString &sAddress, int *retiIndex);
		bool IsFileAddress (const CString &sAddress);

		TArray<SEntry> m_Pages;
	};

extern CPageManager g_PM;
