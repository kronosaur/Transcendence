//	WikiData.h
//
//	Include file for WikiData app

#ifndef INCL_WIKIDATA
#define INCL_WIKIDATA

class CPage
	{
	public:
		inline const CString &GetText (void) { return m_sText; }
		inline const CString &GetTitle (void) { return m_sTitle; }
		ALERROR InitFromXML (CXMLElement *pPage, CString *retsError);

	private:
		CString m_sTitle;
		CString m_sText;
	};

struct STitleEntry
	{
	CString sTitle;
	CPage *pPage;
	};

class CWiki
	{
	public:
		~CWiki (void);

		inline CPage *GetPage (int iIndex) { return m_TitleIndex[iIndex].pPage; }
		inline int GetPageCount (void) { return m_TitleIndex.GetCount(); }
		ALERROR InitFromXML (CXMLElement *pRoot, CString *retsError);

	private:
		TArray<CPage *> m_Pages;
		TArray<STitleEntry> m_TitleIndex;
	};

inline int KeyCompare (const STitleEntry &Key1, const STitleEntry &Key2) { return KeyCompare(Key1.sTitle, Key2.sTitle); }
void OutputPageList (CWiki &Wiki, CXMLElement *pCmdLine);


#endif

