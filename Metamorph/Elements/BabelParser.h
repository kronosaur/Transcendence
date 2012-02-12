//	BabelParser.h
//
//	Parser for babel text
//	Copyright (c) 2000 by NeuroHack, Inc. All Rights Reserved.

#ifndef INCL_BABELPARSER
#define INCL_BABELPARSER

enum BabelTokens
	{
	tkEOS,							//	End of stream

	tkInteger,						//	A decimal or hexadecimal integer
	tkIdentifier,					//	Alphanumeric sequence + '_'
	tkString,						//	Quoted string
	tkLineBlock,					//	Block of lines
	tkParagraph,					//	Paragraph of wrapped lines
	tkCode,							//	Code

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

class CLexer
	{
	public:
		enum Options
			{
			modeNormal,
			modeCode,
			};

		CLexer (char *pPos, char *pEndPos);

		inline BabelTokens CurToken (CString *retsToken)
			{
			if (retsToken)
				*retsToken = m_sToken;
			return m_iToken;
			}
		inline int GetCurLineNumber (void) { return m_iLineNumber; }
		BabelTokens NextToken (CString *retsToken, Options iMode = modeNormal);

	private:
		char *m_pPos;
		char *m_pEndPos;

		CString m_sToken;
		BabelTokens m_iToken;
		int m_iLineNumber;
	};

class CParser
	{
	public:
		CParser (char *pPos, char *pEndPos);

		CBabelData Parse (void);

	private:
		inline BabelTokens CurToken (CString *retsToken = NULL) { return m_Lexer.CurToken(retsToken); }
		inline BabelTokens NextToken (CString *retsToken = NULL) { return m_Lexer.NextToken(retsToken); }

		CBabelData ParseCodeValue (void);
		CBabelData ParseListValue (void);
		CBabelData ParseStructureValue (void);
		CBabelData ParseValue (void);
		void ThrowError (const CString &sError);

		CLexer m_Lexer;
	};

#endif