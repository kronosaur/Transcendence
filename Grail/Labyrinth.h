//	Labyrinth.h
//
//	Labyrinth is a simple database system optimized to implement
//	the Grail Programming Language.

#ifndef INCL_LABYRINTH
#define INCL_LABYRINTH

#ifndef INCL_CONSTANTS
#include "Constants.h"
#endif

#ifndef INCL_CODEBLOCK
#include "CodeBlock.h"
#endif

class CLEntry;

//	Some generic classes

class IReportProgress
	{
	public:
		virtual void Report (const CString &sLine) { }
	};

//	Lexer classes

enum GrailTokens
	{
	tkEOS,							//	End of stream

	tkIdentifier,					//	Alphanumeric sequence + '_'
	tkString,						//	Quoted string
	tkLineBlock,					//	Block of lines
	tkParagraph,					//	Paragraph of wrapped lines

	tkColon,						//	:
	tkDoubleColon,					//	::
	tkSemiColon,					//	;
	tkLeftParen,					//	(
	tkRightParen,					//	)
	tkLeftBrace,					//	{
	tkRightBrace,					//	}
	tkLeftBracket,					//	[
	tkRightBracket,					//	]
	tkSlash,						//	/
	tkStar,							//	*
	tkEquals,						//	=
	tkGreaterThan,					//	>
	tkLessThan,						//	<
	tkComma,						//	,
	tkPeriod,						//	.
	tkBang,							//	!

	tkOtherSymbol,					//	Any other symbol
	};

class ILexer
	{
	public:
		virtual ~ILexer (void) { }

		virtual GrailTokens CurToken (CString *retsToken) = 0;
		virtual int GetCurLineNumber (void) = 0;
		virtual GrailTokens NextToken (CString *retsToken) = 0;
	};

class CLexer : public ILexer
	{
	public:
		CLexer (char *pPos, char *pEndPos);

		virtual GrailTokens CurToken (CString *retsToken)
			{
			if (retsToken)
				*retsToken = m_sToken;
			return m_iToken;
			}
		virtual int GetCurLineNumber (void) { return m_iLineNumber; }
		virtual GrailTokens NextToken (CString *retsToken);

	private:
		char *m_pPos;
		char *m_pEndPos;

		CString m_sToken;
		GrailTokens m_iToken;
		int m_iLineNumber;
	};

//	CLValue
//
//	This is an abstract class that represents a type of data
//	in the Labyrinth database.

enum GrailValueTypes
	{
	vtInteger,
	vtRational,
	vtString,
	vtList,
	vtStructure,
	vtExpression,
	vtAlgorithm,
	vtIdentifier,
	vtDateTime,
	vtTokenStream,
	};

class CLValue
	{
	public:
		virtual ~CLValue (void) { }

		virtual GrailValueTypes GetType (void) = 0;
		virtual int GetCount (void) { return 1; }
		virtual CString GetString (int iIndex = 0) { return CONSTLIT(""); }
	};

class CLNumber : public CLValue
	{
	};

class CLInteger : public CLNumber
	{
	};

class CLRational : public CLNumber
	{
	};

class CLString : public CLValue
	{
	public:
		CLString (const CString &sValue) : m_sValue(sValue) { }

		virtual GrailValueTypes GetType (void) { return vtString; }
		virtual CString GetString (int iIndex = 0) { return m_sValue; }

	private:
		CString m_sValue;
	};

class CLList : public CLValue
	{
	public:
		CLList (void);
		virtual ~CLList (void);

		virtual GrailValueTypes GetType (void) { return vtList; }
		virtual int GetCount (void) { return m_List.GetCount(); }
		virtual CString GetString (int iIndex = 0) { return GetValue(iIndex)->GetString(); }

		ALERROR AddValue (CLValue *pValue);
		inline CLValue *GetValue (int iIndex) { return (CLValue *)m_List.GetObject(iIndex); }

	private:
		CObjectArray m_List;
	};

class CLStructure : public CLValue
	{
	public:
		CLStructure (void);
		virtual ~CLStructure (void);

		virtual GrailValueTypes GetType (void) { return vtStructure; }

		ALERROR AddField (const CString &sFieldName, CLValue *pValue);
		bool FindField (const CString &sFieldName, CLValue **retpValue);
		inline int GetCount (void) { return m_Fields.GetCount(); }
		inline CLValue *GetValue (int iIndex) { return (CLValue *)m_Fields.GetValue(iIndex); }

	private:
		CSymbolTable m_Fields;			//	Table of CLValues
	};

class CLExpression : public CLValue
	{
	};

class CLTokenStream : public CLValue,
					  public ILexer
	{
	public:
		CLTokenStream (void);
		virtual ~CLTokenStream (void);

		virtual GrailValueTypes GetType (void) { return vtTokenStream; }

		ALERROR AddToken (GrailTokens iToken, const CString &sToken);

		//	ILexer
		virtual GrailTokens CurToken (CString *retsToken);
		virtual int GetCurLineNumber (void);
		virtual GrailTokens NextToken (CString *retsToken);

	private:
		
	};

class CLAlgorithm : public CLValue
	{
	public:
		CLAlgorithm (void);
		virtual ~CLAlgorithm (void);

		virtual GrailValueTypes GetType (void) { return vtAlgorithm; }

	private:
		CLCodeBlock *m_pBlock;
	};

class CLIdentifier : public CLValue
	{
	public:
		CLIdentifier (const CString &sIdentifier) : m_sIdentifier(sIdentifier) { }

		virtual GrailValueTypes GetType (void) { return vtIdentifier; }

	private:
		CString m_sIdentifier;
	};

class CLDateTime : public CLValue
	{
	};

//	CLScope
//
//	This object represents a scoping level

class CLScope
	{
	public:
		CLScope (CLScope *pParent);
		~CLScope (void);

		ALERROR AddEntry (CLEntry *pEntry);
		bool FindEntry (const CString &sName, CLEntry **retpEntry);

	private:
		CLScope *m_pParent;				//	Parent scope
		CSymbolTable m_Entries;			//	Table of CLEntries
	};

//	CLEntry
//
//	An entry is a named entity in the database

class CLEntry : public CLStructure
	{
	public:
		CLEntry (const CString &sName, CLScope *pParentScope);
		virtual ~CLEntry (void) { }

		void AddCategory (const CString &sCategory);
		inline void AddSubEntry (CLEntry *pSubEntry) { m_Scope.AddEntry(pSubEntry); }
		const CString &GetName (void) { return m_sName; }
		inline CLScope *GetScope (void) { return &m_Scope; }

	private:
		CString m_sName;
		CStringArray m_Categories;
		
		CLScope m_Scope;				//	Scope
	};

//	CLabyrinth
//
//	This is the actual database object

class CLabyrinth
	{
	public:
		CLabyrinth (IReportProgress *pProgress);
		~CLabyrinth (void);

		ALERROR Boot (void);
		void Shutdown (void);

		inline const CString &GetLastErrorMsg (void) { return m_sLastErrorMsg; }
		inline CLScope *GetGlobalScope (void) { return &m_GlobalScope; }

	private:
		ALERROR LoadFile (const CString &sFilename);
		ALERROR ParseEntries (char *pPos, char *pEndPos, CString *retsError);
		inline void Report (const CString &sMsg) { m_pProgress->Report(sMsg); }
		inline ALERROR SetErrorMsg (const CString &sMsg) { m_sLastErrorMsg = sMsg; return ERR_FAIL; }

		CLScope m_GlobalScope;					//	Global symbol table of entries

		//	These member variables are only initialized
		//	after Boot
		CLEntry *m_pEnv;						//	Grail environment entry

		IReportProgress *m_pProgress;			//	Report progress
		CString m_sLastErrorMsg;				//	Text error message
	};

#endif