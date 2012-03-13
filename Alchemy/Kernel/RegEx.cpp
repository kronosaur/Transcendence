//	RegEx.cpp
//
//	Regular expression package.
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.
//
//	Pattern format:

#include "Kernel.h"

#define SPECIAL_ALPHA							CONSTLIT("A")
#define SPECIAL_ANY								CONSTLIT("?")
#define SPECIAL_WHITESPACE						CONSTLIT("_")
#define SPECIAL_DECIMAL							CONSTLIT("0")
#define SPECIAL_HEX								CONSTLIT("X")
#define SPECIAL_ALPHANUMERIC					CONSTLIT("A0")

class CNFA
	{
	public:
		CNFA (void) : m_pStart(NULL), m_pEnd(NULL), m_iSubExpCount(-1) { }
		CNFA (const CNFA &Obj);
		~CNFA (void);
		CNFA &operator= (const CNFA &Obj);

		bool CalcEClosure (const CLargeSet &In, char *pPos, CLargeSet *retOut);
		bool CalcMove (const CLargeSet &In, char *pPos, CLargeSet *retOut);
		static ALERROR CreateRegExNFA (char *pPos, char **retpEnd, CNFA *retN, CString *retsError);
		void DebugDump (void);
		inline int GetEndState (void) const { return (m_pEnd ? m_pEnd->iID : -1); }
		inline int GetStartState (void) const { return (m_pStart ? m_pStart->iID : -1); }
		inline int GetStateCount (void) const { return m_States.GetCount(); }
		int GetSubExpressionCount (void);
		bool GetSubExpressions (TArray<SRegExMatch> *retMatches) const;
		inline bool IsEmpty (void) { return (m_States.GetCount() == 0); }

	private:
		struct SState;

		enum ETransitionTypes
			{
			transAlpha,
			transAlphaNumeric,
			transAny,
			transDecimal,
			transEpsilon,
			transHex,
			transLiteral,
			transWhitespace,
			};

		struct STransition
			{
			ETransitionTypes iType;
			CString sLiteral;
			SState *pNewState;
			};

		struct SSubExp
			{
			SSubExp (void) : pStart(NULL), pEnd(NULL) { }

			char *pStart;
			char *pEnd;
			};

		class CSubExpState
			{
			public:
				CSubExpState (void) : m_pArray(NULL) { }
				CSubExpState (const CSubExpState &Obj) { if (Obj.m_pArray) m_pArray = new TArray<SSubExp>(*Obj.m_pArray); else m_pArray = NULL; }
				~CSubExpState (void) { if (m_pArray) delete m_pArray; }
				CSubExpState &operator= (const CSubExpState &Obj) { if (m_pArray) delete m_pArray; if (Obj.m_pArray) m_pArray = new TArray<SSubExp>(*Obj.m_pArray); else m_pArray = NULL; return *this; }

				inline int GetCount (void) { return (m_pArray ? m_pArray->GetCount() : 0); }
				inline char *GetEnd (int iIndex) { return m_pArray->GetAt(iIndex).pEnd; }
				inline char *GetStart (int iIndex) { return m_pArray->GetAt(iIndex).pStart; }
				void Init (int iCount) { if (m_pArray == NULL) { m_pArray = new TArray<SSubExp>; m_pArray->InsertEmpty(iCount); } }
				inline void SetStart (int iIndex, char *pPos) { m_pArray->GetAt(iIndex).pStart = pPos; }
				inline void SetEnd (int iIndex, char *pPos) { m_pArray->GetAt(iIndex).pEnd = pPos; }

			private:
				TArray<SSubExp> *m_pArray;
			};

		struct SState
			{
			SState (void) : iStartSubExpID(-1), iEndSubExpID(-1), pMatch(NULL) { }

			int iID;
			TArray<STransition> Transitions;

			int iStartSubExpID;
			int iEndSubExpID;
			char *pMatch;

			CSubExpState MatchState;
			};

		void CleanUp (void);
		void Copy (const CNFA &Obj);
		static void Create0OrMoreNFA (CNFA &N, CNFA *retNFA);
		static void CreateAnyCharNFA (ETransitionTypes iTransition, CNFA *retNFA);
		static void CreateLiteralNFA (const CString &sLiteral, CNFA *retNFA);
		static void CreateOrNFA (CNFA &N1, CNFA &N2, CNFA *retNFA);
		static void CreateSequenceNFA (CNFA &N1, CNFA &N2, CNFA *retNFA);
		SState *GetState (int iState) const { return m_States[iState]; }
		SState *InsertState (SState *pNewState = NULL);
		void MarkAsSubExpression (int iSubExpID);
		bool Matches (STransition *pTrans, char chChar);
		void MoveStates (CNFA &N, SState **retpStart, SState **retpEnd);
		void SetSubExpState (SState *pState, char *pPos);

		TArray<SState *> m_States;
		SState *m_pStart;
		SState *m_pEnd;
		int m_iSubExpCount;
	};


bool strRegEx (char *pStart, const CString &sPattern, TArray<SRegExMatch> *retMatches, char **retpEnd)

//	strRegEx
//
//	Regular expression processor

	{
	//	An empty pattern only matches an empty string

	if (sPattern.IsBlank())
		{
		if (*pStart == '\0')
			{
			SRegExMatch *pMatch = retMatches->Insert();
			pMatch->pPos = pStart;
			return true;
			}
		else
			return false;
		}

	//	Parse the pattern into a NFA

	CNFA N;
	CString sError;
	if (CNFA::CreateRegExNFA(sPattern.GetASCIIZPointer(), NULL, &N, &sError) != NOERROR)
		return false;

#ifdef DEBUG_NFA
	N.DebugDump();
#endif

	//	If the pattern is empty, then nothing to do

	if (N.GetStateCount() == 0)
		return false;

	//	Initial state

	CLargeSet Start;
	Start.Set(N.GetStartState());

	CLargeSet S;
	N.CalcEClosure(Start, pStart, &S);

	//	Walk the input string

	char *pPos = pStart;
	while (*pPos != '\0')
		{
		CLargeSet M;
		N.CalcMove(S, pPos, &M);
		N.CalcEClosure(M, pPos + 1, &S);

		//	If we have no more states, then we did not match

		if (S.IsEmpty())
			return false;

		//	Next

		pPos++;
		}

	//	If we've got a finished state, then we found something

	if (S.IsSet(N.GetEndState()))
		{
		SRegExMatch *pMatch = retMatches->Insert();
		pMatch->pPos = pStart;
		pMatch->sMatch = CString(pStart, pPos - pStart);

		//	Add subexpressions

		N.GetSubExpressions(retMatches);
		return true;
		}

	return false;
	}

//	CNFA -----------------------------------------------------------------------

CNFA::CNFA (const CNFA &Obj)

//	CNFA constructor

	{
	Copy(Obj);
	}

CNFA::~CNFA (void)

//	CNFA destructor

	{
	CleanUp();
	}

CNFA &CNFA::operator= (const CNFA &Obj)

//	CNFA operator=

	{
	CleanUp();
	Copy(Obj);
	return *this;
	}

bool CNFA::CalcEClosure (const CLargeSet &In, char *pPos, CLargeSet *retOut)

//	CalcEClosure
//
//	Computes the set of states that can be reached from the input set
//	solely on e-transitions (Everything).
//
//	Returns TRUE if retOut has any members

	{
	int i;
	bool bFound = false;
	int iSubExpCount = GetSubExpressionCount();

	//	Push all input states in the stack

	TStack<int> Stack;
	for (i = 0; i < GetStateCount(); i++)
		if (In.IsSet(i))
			{
			Stack.Push(i);
			bFound = true;

			//	Initialize sub expressions, if necessary

			if (iSubExpCount)
				SetSubExpState(GetState(i), pPos);
			}

	//	Result is initializes to input

	*retOut = In;

	//	Keep processing the stack

	while (!Stack.IsEmpty())
		{
		SState *pFrom = GetState(Stack.Top());
		Stack.Pop();

		//	Loop over all states that can be reached from iFrom
		//	with any input.

		for (i = 0; i < pFrom->Transitions.GetCount(); i++)
			if (pFrom->Transitions[i].iType == transEpsilon 
					&& !retOut->IsSet(pFrom->Transitions[i].pNewState->iID))
				{
				retOut->Set(pFrom->Transitions[i].pNewState->iID);
				Stack.Push(pFrom->Transitions[i].pNewState->iID);

				//	We've reached a new state, so take the sub expression state from
				//	the source state

				if (iSubExpCount)
					{
					pFrom->Transitions[i].pNewState->MatchState = pFrom->MatchState;
					SetSubExpState(pFrom->Transitions[i].pNewState, pPos);
					}
				}
		}

	//	We keep track of string position for any resulting state

	for (i = 0; i < GetStateCount(); i++)
		if (retOut->IsSet(i))
			{
			SState *pState = GetState(i);
			if (pState->iEndSubExpID != -1 || pState->iStartSubExpID != -1)
				pState->pMatch = pPos;
			}

	return bFound;
	}

bool CNFA::CalcMove (const CLargeSet &In, char *pPos, CLargeSet *retOut)

//	CalcMove
//
//	Returns the set of states that can be reached from In by the given
//	input character.

	{
	int i, j;
	bool bFound = false;
	int iSubExpCount = GetSubExpressionCount();

	retOut->ClearAll();

	for (i = 0; i < GetStateCount(); i++)
		if (In.IsSet(i))
			{
			SState *pState = GetState(i);

			for (j = 0; j < pState->Transitions.GetCount(); j++)
				{
				if (Matches(&pState->Transitions[j], *pPos))
					{
					//	Add the new state as a valid transition

					retOut->Set(pState->Transitions[j].pNewState->iID);
					bFound = true;

					//	We've reached a new state, so take the sub expression state from
					//	the source state

					if (iSubExpCount)
						pState->Transitions[j].pNewState->MatchState = pState->MatchState;
					}
				}
			}

	return bFound;
	}

void CNFA::CleanUp (void)

//	CleanUp
//
//	Delete all

	{
	int i;

	for (i = 0; i < m_States.GetCount(); i++)
		delete m_States[i];

	m_States.DeleteAll();
	}

void CNFA::Copy (const CNFA &Obj)

//	Copy
//
//	Copies structures from Obj.

	{
	int i;

	ASSERT(m_States.GetCount() == 0);

	m_States.InsertEmpty(Obj.m_States.GetCount());
	for (i = 0; i < Obj.m_States.GetCount(); i++)
		m_States[i] = new SState(*Obj.m_States[i]);

	m_pStart = m_States[Obj.m_pStart->iID];
	m_pEnd = m_States[Obj.m_pEnd->iID];
	}

void CNFA::Create0OrMoreNFA (CNFA &N, CNFA *retNFA)

//	Create0OrMoreNFA
//
//	Creates an NFA that accept 0 or more copies of the given NFA.

	{
	//	Move the states from N to this NFA
	//	(And remember the start and end states of each)

	SState *pNStart;
	SState *pNEnd;
	retNFA->MoveStates(N, &pNStart, &pNEnd);

	//	Add a loop from end state back to start state

	STransition *pTrans = pNEnd->Transitions.Insert();
	pTrans->iType = transEpsilon;
	pTrans->pNewState = pNStart;

	//	Create new start state and end states

	retNFA->m_pStart = retNFA->InsertState();
	retNFA->m_pEnd = retNFA->InsertState();

	//	Transition from new start to old

	pTrans = retNFA->m_pStart->Transitions.Insert();
	pTrans->iType = transEpsilon;
	pTrans->pNewState = pNStart;

	//	Transition from new start to end

	pTrans = retNFA->m_pStart->Transitions.Insert();
	pTrans->iType = transEpsilon;
	pTrans->pNewState = retNFA->m_pEnd;

	//	Transition from old end to new end

	pTrans = pNEnd->Transitions.Insert();
	pTrans->iType = transEpsilon;
	pTrans->pNewState = retNFA->m_pEnd;
	}

void CNFA::CreateAnyCharNFA (ETransitionTypes iTransition, CNFA *retNFA)

//	CreateAnyCharNFA
//
//	Creates an NFA that accepts any character

	{
	ASSERT(retNFA->m_States.GetCount() == 0);

	//	Create start and end states

	retNFA->m_pStart = retNFA->InsertState();
	retNFA->m_pEnd = retNFA->InsertState();

	//	Create a transition from start to end

	STransition *pTrans = retNFA->m_pStart->Transitions.Insert();
	pTrans->iType = iTransition;
	pTrans->pNewState = retNFA->m_pEnd;
	}

void CNFA::CreateLiteralNFA (const CString &sLiteral, CNFA *retNFA)

//	CreateLiteralNFA
//
//	Creates an NFA that accepts the given literal.

	{
	ASSERT(retNFA->m_States.GetCount() == 0);

	//	Create start and end states

	retNFA->m_pStart = retNFA->InsertState();
	retNFA->m_pEnd = retNFA->InsertState();

	//	Create a transition from start to end

	STransition *pTrans = retNFA->m_pStart->Transitions.Insert();
	pTrans->iType = transLiteral;
	pTrans->sLiteral = sLiteral;
	pTrans->pNewState = retNFA->m_pEnd;
	}

void CNFA::CreateOrNFA (CNFA &N1, CNFA &N2, CNFA *retNFA)

//	CreateOrNFA
//
//	Creates N1|N2.
//
//	For efficiency, this is a destructive operation (N1 and N2 are not preserved)
//	OK if retNFA is N1 or N2.

	{
	//	Move the states from N1 and N2 to this NFA
	//	(And remember the start and end states of each)

	SState *pN1Start;
	SState *pN1End;
	retNFA->MoveStates(N1, &pN1Start, &pN1End);

	SState *pN2Start;
	SState *pN2End;
	retNFA->MoveStates(N2, &pN2Start, &pN2End);

	//	Create new start state and end states

	retNFA->m_pStart = retNFA->InsertState();
	retNFA->m_pEnd = retNFA->InsertState();

	//	Add transition from start to N1

	STransition *pTrans = retNFA->m_pStart->Transitions.Insert();
	pTrans->iType = transEpsilon;
	pTrans->pNewState = pN1Start;

	//	Add transition from N1 to end

	pTrans = pN1End->Transitions.Insert();
	pTrans->iType = transEpsilon;
	pTrans->pNewState = retNFA->m_pEnd;

	//	Add transition from start to N2

	pTrans = retNFA->m_pStart->Transitions.Insert();
	pTrans->iType = transEpsilon;
	pTrans->pNewState = pN2Start;

	//	Add transition from N2 to end

	pTrans = pN2End->Transitions.Insert();
	pTrans->iType = transEpsilon;
	pTrans->pNewState = retNFA->m_pEnd;
	}

ALERROR CNFA::CreateRegExNFA (char *pPos, char **retpEnd, CNFA *retN, CString *retsError)

//	CreateRegExNFA
//
//	Parses a regex
//
//	Reserved characters: $ { } + * . :
//
//	abc						literal (any character, including whitespace, except reserved)
//	abc def					literal
//	"abc def"				literal
//	${						reserved character {
//	{$?}					any character
//	{$A}					any alpha character
//	{$0}					any digit 0-9
//	{$x}					any hex digit
//	{$A0}					any alpha numeric character
//	{$_}					whitespace character
//	{$A}*					0 or more
//	{$A}+					1 or more
//	{abc}					any subexpression can be placed inside brackets (and is returned separately)
//
//	{$char:a..z}			any character from a to z
//	{$char:${$}$+$$$.$:}	any reserved character
//	{$except:,}				any character except comma
//	{$or:abc:def}			abc or def

	{
	ALERROR error;

	enum States
		{
		stateStart,
		stateEscape,
		stateGroup,
		stateCloseGroup,
		};

	//	Keep parsing a sequence of subexpressions

	int iSubExpID = 0;
	int iState = stateStart;
	CNFA SubExp;

	while (true)
		{
		switch (iState)
			{
			case stateStart:
				if (*pPos == '$')
					iState = stateEscape;
				else if (*pPos == '{')
					iState = stateGroup;
				else if (*pPos == '\0' || *pPos == '}' || *pPos == '.' || *pPos == ':')
					{
					//	Done

					if (retpEnd)
						*retpEnd = pPos;

					return NOERROR;
					}
				else if (*pPos == '*' || *pPos == '+')
					{
					if (retsError)
						*retsError = CONSTLIT("Cannot use repeat operator without something to repeat.");

					return ERR_FAIL;
					}
				else
					{
					//	Literal

					CNFA::CreateLiteralNFA(CString(pPos, 1), &SubExp);
					iState = stateCloseGroup;
					}
				break;

			case stateEscape:
				//	Literal
				CNFA::CreateLiteralNFA(CString(pPos, 1), &SubExp);
				CNFA::CreateSequenceNFA(*retN, SubExp, retN);
				iState = stateStart;
				break;

			case stateGroup:
				//	If the first character of a group is $ then it means either that
				//	we have a special group or that we have an escape character.
				//	For now we assume that we have a special group unless we have one
				//	of the reserved characters.

				if (*pPos == '$'
						&& (pPos[1] != '$' && pPos[1] != '{' && pPos[1] != '}' && pPos[1] != '+' && pPos[1] != '*' && pPos[1] != '.' && pPos[1] != ':'))
					{
					//	Parse until ':' or '}'

					pPos++;
					char *pSpecial = pPos;

					while (*pPos != '\0' && *pPos != ':' && *pPos != '}')
						pPos++;

					//	If we hit the end of the string, then its an error

					if (*pPos == '\0')
						{
						if (retsError)
							*retsError = CONSTLIT("Unexpected end of pattern");
						return ERR_FAIL;
						}

					//	Parse the special group

					CString sSpecial(pSpecial, pPos - pSpecial);
					if (strEquals(sSpecial, SPECIAL_ANY))
						CNFA::CreateAnyCharNFA(transAny, &SubExp);
					else if (strEquals(sSpecial, SPECIAL_WHITESPACE))
						CNFA::CreateAnyCharNFA(transWhitespace, &SubExp);
					else if (strEquals(sSpecial, SPECIAL_ALPHA))
						CNFA::CreateAnyCharNFA(transAlpha, &SubExp);
					else if (strEquals(sSpecial, SPECIAL_DECIMAL))
						CNFA::CreateAnyCharNFA(transDecimal, &SubExp);
					else if (strEquals(sSpecial, SPECIAL_HEX))
						CNFA::CreateAnyCharNFA(transHex, &SubExp);
					else if (strEquals(sSpecial, SPECIAL_ALPHANUMERIC))
						CNFA::CreateAnyCharNFA(transAlphaNumeric, &SubExp);
					else
						{
						if (retsError)
							*retsError = strPatternSubst(CONSTLIT("Unknown special form: %s"), sSpecial);
						return ERR_FAIL;
						}

					//	Done

					iState = stateCloseGroup;
					}

				//	Otherwise, we have a subexpression, which we need to mark

				else
					{
					//	Recurse

					if (error = CreateRegExNFA(pPos, &pPos, &SubExp, retsError))
						return error;

					//	Mark as subexpression

					SubExp.MarkAsSubExpression(iSubExpID++);

					//	We expect a close

					if (*pPos != '}')
						{
						if (retsError)
							*retsError = strPatternSubst(CONSTLIT("'}' expected: %s"), CString(pPos, 1));

						return ERR_FAIL;
						}

					//	Next

					iState = stateCloseGroup;
					}
				break;

			case stateCloseGroup:
				//	Close the group

				if (*pPos == '+')
					{
					CNFA SubExp2(SubExp);

					//	Create 0 or more

					CNFA::Create0OrMoreNFA(SubExp, &SubExp);

					//	Combine the NFA with the result

					CNFA::CreateSequenceNFA(*retN, SubExp2, retN);
					CNFA::CreateSequenceNFA(*retN, SubExp, retN);

					//	Done

					iState = stateStart;
					}
				else if (*pPos == '*')
					{
					//	Create 0 or more

					CNFA::Create0OrMoreNFA(SubExp, &SubExp);

					//	Combine the NFA with the result

					CNFA::CreateSequenceNFA(*retN, SubExp, retN);

					//	Done

					iState = stateStart;
					}
				else
					{
					//	Combine the group's NFA with the result

					CNFA::CreateSequenceNFA(*retN, SubExp, retN);

					//	Next sub-expression

					if (*pPos == '$')
						iState = stateEscape;
					else if (*pPos == '{')
						iState = stateGroup;
					else if (*pPos == '\0' || *pPos == '}' || *pPos == '.' || *pPos == ':')
						{
						//	Done

						if (retpEnd)
							*retpEnd = pPos;

						return NOERROR;
						}
					else
						{
						//	Literal

						CNFA::CreateLiteralNFA(CString(pPos, 1), &SubExp);
						iState = stateCloseGroup;
						}
					}
				break;
			}

		pPos++;
		}
	}

void CNFA::CreateSequenceNFA (CNFA &N1, CNFA &N2, CNFA *retNFA)

//	CreateSequenceNFA
//
//	Creates N1 followed by N2
//
//	For efficiency, this is a destructive operation (N1 and N2 are not preserved)
//	OK if retNFA is N1 or N2.

	{
	if (N1.IsEmpty())
		{
		//	Copy from N2 to result

		retNFA->MoveStates(N2, &retNFA->m_pStart, &retNFA->m_pEnd);
		}
	else if (N2.IsEmpty())
		{
		//	Copy from N1 to result

		retNFA->MoveStates(N1, &retNFA->m_pStart, &retNFA->m_pEnd);
		}
	else
		{
		//	Move the states from N1 and N2 to this NFA
		//	(And remember the start and end states of each)

		SState *pN1Start;
		SState *pN1End;
		retNFA->MoveStates(N1, &pN1Start, &pN1End);

		SState *pN2Start;
		SState *pN2End;
		retNFA->MoveStates(N2, &pN2Start, &pN2End);

		//	Collapse N1 endstate with N2 start
		//	(Since we can guarantee that N1 endstate has no
		//	transitions out, we just copy all transitions from
		//	N2 start).

		ASSERT(pN1End->Transitions.GetCount() == 0);
		pN1End->Transitions = pN2Start->Transitions;
		if (pN2Start->iStartSubExpID != -1)
			pN1End->iStartSubExpID = pN2Start->iStartSubExpID;

		//	NOTE: We leave N2 start alone; it has no incoming transitions
		//	and it will get cleaned up at the end.

		pN2Start->Transitions.DeleteAll();
		pN2Start->iStartSubExpID = -1;
		pN2Start->iEndSubExpID = -1;

		//	Remember start and end

		retNFA->m_pStart = pN1Start;
		retNFA->m_pEnd = pN2End;
		}
	}

void CNFA::DebugDump (void)

//	DebugDump
//
//	Dump out the NFA

	{
	int i, j;

	for (i = 0; i < GetStateCount(); i++)
		{
		SState *pState = GetState(i);

		char szBuffer[1024];
		wsprintf(szBuffer, "STATE: %d\n", pState->iID);
		::OutputDebugString(szBuffer);

		for (j = 0; j < pState->Transitions.GetCount(); j++)
			{
			char *pType;
			switch (pState->Transitions[j].iType)
				{
				case transAlpha:
					pType = "alpha";
					break;

				case transAlphaNumeric:
					pType = "alphanumeric";
					break;

				case transAny:
					pType = "?";
					break;

				case transDecimal:
					pType = "decimal";
					break;

				case transEpsilon:
					pType = "epsilon";
					break;

				case transHex:
					pType = "hex";
					break;

				case transLiteral:
					pType = pState->Transitions[j].sLiteral.GetASCIIZPointer();
					break;

				case transWhitespace:
					pType = "whitespace";
					break;

				default:
					pType = "(unknown)";
				}

			wsprintf(szBuffer, "   -> %d (%s)\n", pState->Transitions[j].pNewState->iID, pType);
			::OutputDebugString(szBuffer);
			}
		}
	}

int CNFA::GetSubExpressionCount (void)

//	GetSubExpressionCount
//
//	Returns the number of sub-expressions

	{
	int i;

	if (m_iSubExpCount != -1)
		return m_iSubExpCount;

	m_iSubExpCount = 0;
	for (i = 0; i < GetStateCount(); i++)
		if (GetState(i)->iStartSubExpID != -1)
			m_iSubExpCount++;

	return m_iSubExpCount;
	}

bool CNFA::GetSubExpressions (TArray<SRegExMatch> *retMatches) const

//	GetSubExpressions
//
//	Fills in retMatches with 1 or more sub expression. If none are
//	found, returns FALSE.

	{
	int i;
	bool bAdded = false;

	//	The end state better have everything

	for (i = 0; i < m_pEnd->MatchState.GetCount(); i++)
		{
		SRegExMatch *pMatch = retMatches->Insert();
		char *pStart = m_pEnd->MatchState.GetStart(i);
		char *pEnd = m_pEnd->MatchState.GetEnd(i);

		if (pStart && pEnd && pEnd >= pStart)
			{
			pMatch->pPos = pStart;
			pMatch->sMatch = CString(pStart, pEnd - pStart);
			}
		else
			pMatch->pPos = NULL;

		bAdded = true;
		}

	//	Done

	return bAdded;
	}

CNFA::SState *CNFA::InsertState (SState *pNewState)

//	InsertState
//
//	Inserts a new state

	{
	if (pNewState == NULL)
		pNewState = new SState;

	pNewState->iID = m_States.GetCount();
	m_States.Insert(pNewState);

	return pNewState;
	}

void CNFA::MarkAsSubExpression (int iSubExpID)

//	MarkAsSubExpression
//
//	Marks this as a subexpression with the given ID

	{
	if (m_pStart)
		m_pStart->iStartSubExpID = iSubExpID;

	if (m_pEnd)
		m_pEnd->iEndSubExpID = iSubExpID;

	//	Reset count so we recompute it when needed.

	m_iSubExpCount = -1;
	}

bool CNFA::Matches (STransition *pTrans, char chChar)

//	Matches
//
//	Returns TRUE if the given transition matches the given character

	{
	switch (pTrans->iType)
		{
		case transAlpha:
			return (::IsCharAlpha(chChar) ? true : false);

		case transAlphaNumeric:
			return (::IsCharAlpha(chChar) || (chChar >= '0' && chChar <= '9'));

		case transAny:
			return true;

		case transDecimal:
			return (chChar >= '0' && chChar <= '9');

		case transHex:
			return ((chChar >= '0' && chChar <= '9') || (chChar >= 'a' && chChar <= 'f') || (chChar >= 'A' && chChar <= 'F'));

		case transLiteral:
			return (*(pTrans->sLiteral.GetASCIIZPointer()) == chChar);

		case transWhitespace:
			return (chChar == ' ' || chChar == '\t' || chChar == '\n' || chChar == '\r');

		default:
			return false;
		}
	}

void CNFA::MoveStates (CNFA &N, SState **retpStart, SState **retpEnd)

//	MoveStates
//
//	Moves all the states from N to this NFA, destructively.

	{
	//	Remember start and end

	if (retpStart)
		*retpStart = N.m_pStart;

	if (retpEnd)
		*retpEnd = N.m_pEnd;

	//	Only if we're not moving to ourselves

	if (&N != this)
		{
		int i;

		for (i = 0; i < N.m_States.GetCount(); i++)
			InsertState(N.m_States[i]);

		//	Remove from N without deleting (because we take ownership)

		N.m_States.DeleteAll();
		N.m_pStart = NULL;
		N.m_pEnd = NULL;
		}
	}

void CNFA::SetSubExpState (SState *pState, char *pPos)

//	SetSubExpState
//
//	Set the sub expression state for the given NFA state

	{
	int iSubExpCount = GetSubExpressionCount();
	if (iSubExpCount > 0)
		{
		pState->MatchState.Init(iSubExpCount);

		if (pState->iStartSubExpID != -1)
			pState->MatchState.SetStart(pState->iStartSubExpID, pPos);

		if (pState->iEndSubExpID != -1)
			pState->MatchState.SetEnd(pState->iEndSubExpID, pPos);
		}
	}
