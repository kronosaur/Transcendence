//	CPPLexer.h
//
//	Lexer for CPP syntax
//	Copyright (c) 2001 by NeuroHack, Inc. All Rights Reserved.

#ifndef INCL_CPPLEXER
#define INCL_CPPLEXER

enum CPPTokens
	{
	tkNone =					0,
	tkEOS =						-1,		//	End of stream
	tkError =					-2,

	//	Comments & White Space
	tkWhiteSpace =				1,
	tkLineComment =				2,
	tkBlockComment =			3,

	//	Identifier
	tkIdentifier =				4,

	//	Symbols
	tkNot =						5,		//	!
	//tkMod,							//	%
	//tkBitXOR,							//	^
	//tkBitAnd,							//	&
	tkTimes =					9,		//	*
	tkOpenParen =				10,		//	(
	tkCloseParen =				11,		//	)
	tkMinus =					12,		//	-
	tkPlus =					13,		//	+
	tkAssign =					14,		//	=
	tkOpenBrace =				15,		//	{
	tkCloseBrace =				16,		//	}
	//tkBitOr,							//	|
	//tkBitNot,							//	~
	tkOpenBracket =				19,		//	[
	tkCloseBracket =			20,		//	]
	//tkBackslash,						//	
	tkSemiColon =				22,		//	;
	tkColon =					23,		//	:
	tkGreaterThan =				24,		//	>
	tkLessThan =				25,		//	<
	tkTernary =					26,		//	?
	tkComma =					27,		//	,
	//tkDot,							//	.
	tkDividedBy =				29,		//	/
	//tkDereference,					//	->
	//tkPlusPlus,						//	++
	//tkMinusMinus,						//	--
	//tkDotStar,						//	.*
	//tkDeferenceStar,					//	->*
	//tkShiftLeft,						//	<<
	//tkShiftRight,						//	>>
	tkLessThanOrEqual =			37,		//	<=
	tkGreaterThanOrEqual =		38,		//	>=
	tkEquals =					39,		//	==
	tkNotEquals =				40,		//	!=
	tkAnd =						41,		//	&&
	tkOr =						42,		//	||
	//tkTimesEquals						//	*=
	//tkDividedByEquals					//	/=
	//tkModEquals						//	%=
	//tkPlusEquals						//	+=
	//tkMinusEquals						//	-=
	//tkShiftLeftEquals					//	<<=
	//tkShiftRightEquals				//	>>=
	//tkAndEquals						//	&=
	//tkXOREquals,						//	^=
	//tkOrEquals,						//	|=
	//tkScope,							//	::

	//	Literals
	tkDecimalLiteral =			54,		//	123
	tkStringLiteral =			55,		//	"abc"
	tkHexLiteral =				56,		//	0x1ab
	//tkOctalLiteral,					//	0123
	//tkCharLiteral,					//	'a'
	//tkFloatLiteral,					//	123.45
	};

class CCPPLexer
	{
	public:
		CCPPLexer (char *pPos, int iLength);

		inline int GetLineNumber (void) { return m_iLineNumber; }
		inline CPPTokens GetToken (void) { return m_iToken; }
		int GetTokenInteger (void);
		inline CString &GetTokenString (void) { return m_sToken; }
		CPPTokens NextToken (void);

	private:
		enum Constants
			{
			maxStates = 100,
			};

		CPPTokens Recognize (char *pPos, char **retpEndPos);

		char *m_pPos;
		char *m_pEndPos;

		CPPTokens m_iToken;
		CString m_sToken;
		int m_iLineNumber;

		int m_iStateOffset[maxStates];
	};

#endif

