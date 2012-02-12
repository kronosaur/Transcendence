//	Lingua.h
//
//	Basic text processing classes

#ifndef INCL_LINGUA
#define INCL_LINGUA

//	CMarkovWordGenerator -------------------------------------------------------

class CMarkovWordGenerator
	{
	public:
		CMarkovWordGenerator (void) : m_dwStartCount(0) { }
		~CMarkovWordGenerator (void);

		void AddSample (const CString &sWord);
		CString Generate (void);
		int GenerateUnique (int iCount, TArray<CString> *retArray);
		ALERROR WriteAsXML (IWriteStream *pOutput);

	private:
		enum Flags
			{
			FRAG_WORD_START = 0x01,
			};

		struct SFragHead
			{
			char sFrag[4];
			DWORD dwFlags:8;
			DWORD dwCount:24;
			};

		struct SChainChar
			{
			DWORD chChar:8;
			DWORD dwCount:24;
			};

		void AddChain (char *pFrag, char chChar, DWORD dwFlags);
		SFragHead *AllocFrag (char *pFrag, char chChar, DWORD dwFlags);
		SFragHead *AppendChainChar (SFragHead *pFrag, char chChar, DWORD dwFlags);
		bool Find (char *pFrag, int *retiPos);
		bool FindChainChar (SFragHead *pFrag, char chChar, SChainChar **retpChain);
		inline SChainChar *GetChain (SFragHead *pFrag) { return (SChainChar *)(&pFrag[1]); }
		bool InitStart (void);

		TArray<SFragHead *> m_Table;

		DWORD m_dwStartCount;
		TArray<SFragHead *> m_Start;
	};

#endif