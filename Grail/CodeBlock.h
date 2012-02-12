//	CodeBlock.h
//
//	Data structures used for algorithms and expressions
//
//	expression :: identifier
//			| integer
//			| rational
//			| string
//			| list
//			| structure
//			| date-time
//			| ( expression )
//			| unary-operator expression
//			| expression binary-operator expression
//			| function-call
//
//	block :: statement
//			| { statement; statement; ... }
//
//	statement :: declaration
//			| assignment
//			| function-call
//
//	function-call :: function-expression ( expression, expression, ... )
//
//	function-expression :: identifier
//			| structure

#ifndef INCL_CODEBLOCK
#define INCL_CODEBLOCK

class CLStatement
	{
	private:

	};

class CLCodeBlock
	{
	private:
		CLStatement *m_pFirstStatement;
	};

#endif