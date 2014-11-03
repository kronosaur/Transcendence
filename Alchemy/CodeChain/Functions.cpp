//	Functions.cpp
//
//	Implements primitive functions
//
//	Validation codes for EvaluateArgs:
//
//		f	function expression
//		i	Integer
//		k	A linked-list
//		l	List
//		q	Quoted identifier
//		s	String
//		u	Any item (not evaluated)
//		v	Any item (including errors)
//		x	Atom table
//		y	Symbol table
//		*	Any number of items (must be last)

#include <windows.h>
#include <Math.h>
#include "Kernel.h"
#include "KernelObjID.h"
#include "CodeChain.h"

#include "Functions.h"

//	Forwards

double GetFractionArg (ICCItem *pArg, double *retrDenom = NULL);
ALERROR HelperSetq (CEvalContext *pCtx, ICCItem *pVar, ICCItem *pValue, ICCItem **retpError);

ICCItem *fnAppend (CEvalContext *pCtx, ICCItem *pArgs, DWORD dwData)

//	fnAppend
//
//	Appends two or more elements together
//	(append a b [...]) -> lists are concatenated

	{
	int i, j;
	CCodeChain *pCC = pCtx->pCC;

	//	Handle some edge-conditions

	if (pArgs->GetCount() == 0)
		return pCC->CreateNil();

	//	Create a new list to store the result in

	ICCItem *pResult = pCC->CreateLinkedList();
	if (pResult->IsError())
		return pResult;

	CCLinkedList *pList = (CCLinkedList *)pResult;

	//	Loop over all arguments and add to result list

	for (i = 0; i < pArgs->GetCount(); i++)
		{
		ICCItem *pItem = pArgs->GetElement(i);

		//	If the item is a symbol table, then treat it like an atom

		if (pItem->IsSymbolTable())
			pList->Append(pCC, pItem, NULL);

		//	Otherwise, add each of the elements

		else
			{
			for (j = 0; j < pItem->GetCount(); j++)
				pList->Append(pCC, pItem->GetElement(j), NULL);
			}
		}

	//	Done

	return pResult;
	}

ICCItem *fnApply (CEvalContext *pCtx, ICCItem *pArguments, DWORD dwData)

//	fnApply
//
//	Applies the given parameter list to the lambda expression
//
//	(apply exp arg1 arg2 ... argn list)

	{
	CCodeChain *pCC = pCtx->pCC;
	ICCItem *pArgs;
	ICCItem *pResult;
	ICCItem *pFunction;
	ICCItem *pLast;
	CCLinkedList *pList;
	int i;

	//	Evaluate the arguments and validate them

	pArgs = pCC->EvaluateArgs(pCtx, pArguments, CONSTLIT("v*"));
	if (pArgs->IsError())
		return pArgs;

	//	We better have at least two arguments

	if (pArgs->GetCount() < 2)
		{
		pArgs->Discard(pCC);
		return pCC->CreateError(CONSTLIT("apply needs a function and a list of arguments"));
		}

	//	The last argument better be a list

	pLast = pArgs->GetElement(pArgs->GetCount() - 1);
	if (!pLast->IsList())
		{
		pArgs->Discard(pCC);
		return pCC->CreateError(CONSTLIT("Last argument for apply must be a list"));
		}

	//	The first argument is the function

	pFunction = pArgs->Head(pCC);

	//	Create a new list to store the arguments in

	pResult = pCC->CreateLinkedList();
	if (pResult->IsError())
		{
		pArgs->Discard(pCC);
		return pResult;
		}

	pList = (CCLinkedList *)pResult;

	//	Add each of the arguments except the last

	for (i = 1; i < pArgs->GetCount() - 1; i++)
		{
		pList->Append(pCC, pArgs->GetElement(i), &pResult);
		if (pResult->IsError())
			{
			pList->Discard(pCC);
			pArgs->Discard(pCC);
			return pResult;
			}

		pResult->Discard(pCC);
		}

	//	Add each of the elements of the last list

	for (i = 0; i < pLast->GetCount(); i++)
		{
		pList->Append(pCC, pLast->GetElement(i), &pResult);
		if (pResult->IsError())
			{
			pList->Discard(pCC);
			pArgs->Discard(pCC);
			return pResult;
			}

		pResult->Discard(pCC);
		}

	//	Set the literal flag to indicate that the arguments should
	//	not be evaluated.

	pList->SetQuoted();

	//	Execute the function

	if (pFunction->IsFunction())
		pResult = pFunction->Execute(pCtx, pList);
	else
		pResult = pFunction->Reference();

	pList->Discard(pCC);

	//	Done

	pArgs->Discard(pCC);
	return pResult;
	}

ICCItem *fnAtmCreate (CEvalContext *pCtx, ICCItem *pArguments, DWORD dwData)

//	fnAtmCreate
//
//	Creates a new atom table
//
//	(atmAtomTable ((atom1 entry1) (atom2 entry2) ... (atomn entryn))) -> atmtable

	{
	CCodeChain *pCC = pCtx->pCC;
	ICCItem *pArgs;
	ICCItem *pAtomTable;
	ICCItem *pList;
	int i;

	//	Evaluate the argument

	pArgs = pCC->EvaluateArgs(pCtx, pArguments, CONSTLIT("l"));
	if (pArgs->IsError())
		return pArgs;

	//	Create the table

	pAtomTable = pCC->CreateAtomTable();
	if (pAtomTable->IsError())
		return pAtomTable;

	//	Add each entry

	pList = pArgs->Head(pCC);
	for (i = 0; i < pList->GetCount(); i++)
		{
		ICCItem *pPair = pList->GetElement(i);
		ICCItem *pResult;

		//	Make sure we have two elements

		if (pPair->GetCount() != 2)
			{
			pAtomTable->Discard(pCC);
			return pCC->CreateError(CONSTLIT("Invalid format for atom table entry"), pPair);
			}

		//	Get the atom and the entry

		pResult = pAtomTable->AddEntry(pCC, pPair->GetElement(0), pPair->GetElement(1));
		if (pResult->IsError())
			{
			pAtomTable->Discard(pCC);
			return pResult;
			}

		pResult->Discard(pCC);
		}

	//	Done

	pArgs->Discard(pCC);
	return pAtomTable;
	}

ICCItem *fnAtmTable (CEvalContext *pCtx, ICCItem *pArguments, DWORD dwData)

//	fnAtmTable
//
//	Various atom table manipulations
//
//	(atmAddEntry symTable symbol entry) -> entry
//	(atmDeleteEntry symTable symbol) -> True
//	(atmLookup symTable symbol) -> entry

	{
	CCodeChain *pCC = pCtx->pCC;
	ICCItem *pArgs;
	ICCItem *pSymTable;
	ICCItem *pSymbol;
	ICCItem *pResult;

	//	Evaluate the arguments and validate them

	if (dwData == FN_ATMTABLE_ADDENTRY)
		pArgs = pCC->EvaluateArgs(pCtx, pArguments, CONSTLIT("xiv"));
	else if (dwData == FN_ATMTABLE_LIST)
		pArgs = pCC->EvaluateArgs(pCtx, pArguments, CONSTLIT("x"));
	else
		pArgs = pCC->EvaluateArgs(pCtx, pArguments, CONSTLIT("xi"));

	if (pArgs->IsError())
		return pArgs;

	//	Get the args

	pSymTable = pArgs->Head(pCC);

	//	Do the right thing

	switch (dwData)
		{
		case FN_ATMTABLE_ADDENTRY:
			{
			ICCItem *pEntry;

			pSymbol = pArgs->GetElement(1);
			pEntry = pArgs->GetElement(2);
			pResult = pSymTable->AddEntry(pCC, pSymbol, pEntry);

			//	If we succeeded, return the entry

			if (!pResult->IsError())
				{
				pResult->Discard(pCC);
				pResult = pEntry->Reference();
				}

			break;
			}

		case FN_ATMTABLE_DELETEENTRY:
			{
			pResult = pCC->CreateNil();
			break;
			}

		case FN_ATMTABLE_LIST:
			{
			pResult = pSymTable->ListSymbols(pCC);
			break;
			}

		case FN_ATMTABLE_LOOKUP:
			{
			pSymbol = pArgs->GetElement(1);
			pResult = pSymTable->Lookup(pCC, pSymbol);
			break;
			}

		default:
			ASSERT(FALSE);
			return NULL;
		}

	//	Done

	pArgs->Discard(pCC);
	return pResult;
	}

ICCItem *fnBlock (CEvalContext *pCtx, ICCItem *pArguments, DWORD dwData)

//	fnBlock
//
//	Evaluates a list of expressions
//
//	(block (locals ...) exp1 exp2 ... expn)
//	(errblock (error locals ...) exp1 exp2 ... expn onerror)

	{
	CCodeChain *pCC = pCtx->pCC;
	ICCItem *pResult;
	ICCItem *pLocals;
	ICCItem *pExp;
	ICCItem *pVar;
	int i;

	//	The first argument must be a list of locals

	pLocals = pArguments->Head(pCC);
	if (pLocals == NULL || !pLocals->IsList())
		return pCC->CreateError(CONSTLIT("Locals list expected"), pLocals);

	//	If this is an error block then we must have at least one local

	if (dwData == FN_BLOCK_ERRBLOCK && pLocals->GetCount() == 0)
		return pCC->CreateError(CONSTLIT("errblock must have an 'error' local variable"));

	//	Now loop over the remaining arguments, evaluating each in turn

	pExp = pArguments->GetElement(1);

	//	If there are no expressions, then we just return Nil

	if (pExp == NULL)
		return pCC->CreateNil();

	//	Setup the locals. We start by creating a local symbol table
	//	(but only if we have local variables)

	ICCItem *pLocalSymbols;
	ICCItem *pOldSymbols;
	if (pLocals->GetCount() > 0)
		{
		pLocalSymbols = pCC->CreateSymbolTable();
		if (pLocalSymbols->IsError())
			return pLocalSymbols;

		pLocalSymbols->SetLocalFrame();

		//	Setup the context

		if (pCtx->pLocalSymbols)
			pLocalSymbols->SetParent(pCtx->pLocalSymbols);
		else
			pLocalSymbols->SetParent(pCtx->pLexicalSymbols);
		pOldSymbols = pCtx->pLocalSymbols;
		pCtx->pLocalSymbols = pLocalSymbols;

		//	Loop over each item and associate it

		for (i = 0; i < pLocals->GetCount(); i++)
			{
			ICCItem *pItem;
			ICCItem *pLocal;
			ICCItem *pValue;

			pLocal = pLocals->GetElement(i);

			//	If the local is a list, then the first element is the variable
			//	and the second element is the initial value.

			if (pLocal->IsList() && pLocal->GetCount() >= 2)
				{
				pVar = pLocal->GetElement(0);
				pValue = pCC->Eval(pCtx, pLocal->GetElement(1));

				//	If we get an error evaluating, return it

				if (pValue->IsError())
					{
					//	Clean up

					pCtx->pLocalSymbols = pOldSymbols;
					pLocalSymbols->Discard(pCC);

					//	Done

					return pValue;
					}
				}

			//	Otherwise, we expect an identifier (which we initialize to Nil)

			else
				{
				pVar = pLocal;
				pValue = pCC->CreateNil();
				}

			//	Add it

			if (pVar->IsIdentifier())
				{
				pItem = pLocalSymbols->AddEntry(pCC, pVar, pValue, true);
				pValue->Discard(pCC);
				if (pItem->IsError())
					{
					pCtx->pLocalSymbols = pOldSymbols;
					pLocalSymbols->Discard(pCC);
					return pItem;
					}

				pItem->Discard(pCC);
				}
			}

		}
	else
		{
		pOldSymbols = NULL;
		pLocalSymbols = NULL;
		}

	//	Start with a default result

	pResult = pCC->CreateNil();

	//	Loop (starting with the second arg)

	for (i = 1; i < pArguments->GetCount(); i++)
		{
		pExp = pArguments->GetElement(i);

		//	If this is an error block and this is the last expression,
		//	then it must be error condition and we don't want to 
		//	execute it.

		if (i+1 == pArguments->GetCount() && dwData == FN_BLOCK_ERRBLOCK)
			break;

		//	Evaluate the expression

		pResult->Discard(pCC);
		pResult = pCC->Eval(pCtx, pExp);

		//	If we got an error, handle it

		if (pResult->IsError())
			{
			//	If this is an error block, then find the last expression
			//	and evaluate it.

			if (dwData == FN_BLOCK_ERRBLOCK)
				{
				ICCItem *pItem;

				//	Set the first local variable to be the error result

				pVar = pLocals->Head(pCC);
				if (pVar->IsIdentifier())
					{
					pItem = pLocalSymbols->AddEntry(pCC, pVar, pResult);
					pItem->Discard(pCC);
					}

				pResult->Discard(pCC);

				//	Find the last expression

				pExp = pArguments->GetElement(pArguments->GetCount() - 1);

				//	Evaluate it

				pResult = pCC->Eval(pCtx, pExp);
				}

			//	Regardless, leave the block and return the result

			break;
			}
		}

	//	Clean up

	if (pLocalSymbols)
		{
		pCtx->pLocalSymbols = pOldSymbols;
		pLocalSymbols->Discard(pCC);
		}

	//	Done

	return pResult;
	}

ICCItem *fnCat (CEvalContext *pCtx, ICCItem *pArguments, DWORD dwData)

//	fnCat
//
//	Concatenates the given strings
//
//	(cat string1 string2 ... stringn)

	{
	CCodeChain *pCC = pCtx->pCC;
	ICCItem *pArgs;
	ICCItem *pResult;
	CString sResult;
	int i;

	//	Evaluate the arguments and validate them

	pArgs = pCC->EvaluateArgs(pCtx, pArguments, CONSTLIT("*"));
	if (pArgs->IsError())
		return pArgs;

	//	Append each of the items

	for (i = 0; i < pArgs->GetCount(); i++)
		if (!pArgs->GetElement(i)->IsNil())
			sResult.Append(pArgs->GetElement(i)->GetStringValue());

	pResult = pCC->CreateString(sResult);

	//	Done

	pArgs->Discard(pCC);
	return pResult;
	}

ICCItem *fnCount (CEvalContext *pCtx, ICCItem *pArguments, DWORD dwData)

//	fnCount
//
//	Returns the number of elements in the list
//
//	(count list)

	{
	CCodeChain *pCC = pCtx->pCC;
	ICCItem *pArgs;
	ICCItem *pResult;
	ICCItem *pList;

	//	Evaluate the arguments and validate them

	pArgs = pCC->EvaluateArgs(pCtx, pArguments, CONSTLIT("v"));
	if (pArgs->IsError())
		return pArgs;

	//	The first argument is the list

	pList = pArgs->Head(pCC);
	if (pList->IsList())
		pResult = pCC->CreateInteger(pList->GetCount());
	else if (pList->IsIdentifier())
		pResult = pCC->CreateInteger(pList->GetStringValue().GetLength());
	else
		pResult = pCC->CreateInteger(1);

	//	Done

	pArgs->Discard(pCC);
	return pResult;
	}

ICCItem *fnEnum (CEvalContext *pCtx, ICCItem *pArguments, DWORD dwData)

//	fnEnum
//
//	Enumerates the elements of a list
//
//	(enum list item-var exp)
//	(enumwhile list condition item-var exp)

	{
	CCodeChain *pCC = pCtx->pCC;
	ICCItem *pArgs;
	ICCItem *pResult;
	ICCItem *pVar;
	ICCItem *pBody;
	ICCItem *pList;
	ICCItem *pCondition;
	ICCItem *pLocalSymbols;
	ICCItem *pOldSymbols;
	ICCItem *pError;
	int i, iVarOffset;

	//	Evaluate the arguments and validate them

	if (dwData == FN_ENUM_WHILE)
		{
		pArgs = pCC->EvaluateArgs(pCtx, pArguments, CONSTLIT("vuqu"));
		if (pArgs->IsError())
			return pArgs;

		pList = pArgs->GetElement(0);
		pCondition = pArgs->GetElement(1);
		pVar = pArgs->GetElement(2);
		pBody = pArgs->GetElement(3);
		}
	else
		{
		pArgs = pCC->EvaluateArgs(pCtx, pArguments, CONSTLIT("vqu"));
		if (pArgs->IsError())
			return pArgs;

		pList = pArgs->GetElement(0);
		pCondition = NULL;
		pVar = pArgs->GetElement(1);
		pBody = pArgs->GetElement(2);
		}

	//	If we have an empty list, then we're done

	if (pList->GetCount() == 0)
		{
		pArgs->Discard(pCC);
		return pCC->CreateNil();
		}

	//	Setup the locals. We start by creating a local symbol table

	pLocalSymbols = pCC->CreateSymbolTable();
	if (pLocalSymbols->IsError())
		{
		pArgs->Discard(pCC);
		return pLocalSymbols;
		}

	//	Associate the enumeration variable

	pError = pLocalSymbols->AddEntry(pCC, pVar, pCC->CreateNil());
	if (pError->IsError())
		{
		pArgs->Discard(pCC);
		return pError;
		}

	pError->Discard(pCC);

	//	Setup the context

	if (pCtx->pLocalSymbols)
		pLocalSymbols->SetParent(pCtx->pLocalSymbols);
	else
		pLocalSymbols->SetParent(pCtx->pLexicalSymbols);
	pOldSymbols = pCtx->pLocalSymbols;
	pCtx->pLocalSymbols = pLocalSymbols;

	//	Start with a default result

	pResult = pCC->CreateNil();

	//	Get the offset of the variable so we don't have to
	//	search for it all the time

	iVarOffset = pLocalSymbols->FindOffset(pCC, pVar);

	//	Loop over all elements of the list

	for (i = 0; i < pList->GetCount(); i++)
		{
		//	Check the condition

		if (pCondition)
			{
			ICCItem *pEval;

			//	Evaluate the condition

			pEval = pCC->Eval(pCtx, pCondition);
			if (pEval->IsError())
				{
				pResult->Discard(pCC);
				pResult = pEval;
				break;
				}

			//	If the condition is Nil, then we're done

			if (pEval->IsNil())
				{
				pEval->Discard(pCC);
				break;
				}
			}

		ICCItem *pItem = pList->GetElement(pCC, i);

		//	Clean up the previous result

		pResult->Discard(pCC);

		//	Set the element

		pLocalSymbols->AddByOffset(pCC, iVarOffset, pItem);
		pItem->Discard(pCC);

		//	Eval

		pResult = pCC->Eval(pCtx, pBody);
		if (pResult->IsError())
			break;
		}

	//	Clean up

	pCtx->pLocalSymbols = pOldSymbols;
	pLocalSymbols->Discard(pCC);

	//	Done

	pArgs->Discard(pCC);
	return pResult;
	}

ICCItem *fnEquality (CEvalContext *pCtx, ICCItem *pArguments, DWORD dwData)

//	fnEquality
//
//	Equality and inequality
//
//	(eq exp1 exp2 ... expn)
//	(neq exp1 exp2 ... expn)
//	(> exp1 exp2 ... expn)
//	(>= exp1 exp2 ... expn)
//	(< exp1 exp2 ... expn)
//	(<= exp1 exp2 ... expn)

	{
	CCodeChain *pCC = pCtx->pCC;
	ICCItem *pResult;
	ICCItem *pExp;
	ICCItem *pPrev = NULL;
	ICCItem *pArgs;
	int i;

	//	Evaluate the arguments and validate them

	pArgs = pCC->EvaluateArgs(pCtx, pArguments, CONSTLIT("*"));
	if (pArgs->IsError())
		return pArgs;

	//	Loop over all arguments

	bool bOk = true;
	for (i = 0; i < pArgs->GetCount(); i++)
		{
		pExp = pArgs->GetElement(i);
		if (pExp->IsError())
			{
			pExp->Reference();
			pArgs->Discard(pCC);
			return pExp;
			}

		if (pPrev)
			{
			//	Special case for eq because we don't want compares
			//	to Nil to equal 0

			if ((dwData == FN_EQUALITY_EQ) || (dwData == FN_EQUALITY_NEQ))
				{
				if (pPrev->IsNil() || pExp->IsNil())
					bOk = (pPrev->IsNil() && pExp->IsNil());
				else
					{
					int iResult = HelperCompareItems(pPrev, pExp);
					bOk = (iResult == 0);
					}
				}

			//	For other comparisons, we coerce datatypes and we
			//	treat Nil as 0 (if necessary)

			else
				{
				int iResult = HelperCompareItems(pPrev, pExp);

				switch (dwData)
					{
					case FN_EQUALITY_LESSER:
						bOk = (iResult < 0);
						break;

					case FN_EQUALITY_LESSER_EQ:
						bOk = (iResult <= 0);
						break;

					case FN_EQUALITY_GREATER:
						bOk = (iResult > 0);
						break;

					case FN_EQUALITY_GREATER_EQ:
						bOk = (iResult >= 0);
						break;

					default:
						ASSERT(FALSE);
					}
				}

			//	If we don't have a match, return

			if (!bOk)
				break;
			}

		//	Remember the previous element so that we can compare

		pPrev = pExp;
		}

	//	Negate, if necessary

	if (dwData == FN_EQUALITY_NEQ)
		bOk = !bOk;

	//	If we get here, then all items are ok

	pArgs->Discard(pCC);
	pResult = pCC->CreateBool(bOk);

	//	Done

	return pResult;
	}

ICCItem *fnEval (CEvalContext *pCtx, ICCItem *pArguments, DWORD dwData)

//	fnEval
//
//	Evaluates an expression
//
//	(eval exp)

	{
	CCodeChain *pCC = pCtx->pCC;
	ICCItem *pArgs;
	ICCItem *pResult;

	//	Evaluate the arguments and validate them

	pArgs = pCC->EvaluateArgs(pCtx, pArguments, CONSTLIT("v"));
	if (pArgs->IsError())
		return pArgs;

	pResult = pCC->Eval(pCtx, pArgs->GetElement(0));

	//	Done

	pArgs->Discard(pCC);
	return pResult;
	}

ICCItem *fnFilter (CEvalContext *pCtx, ICCItem *pArgs, DWORD dwData)

//	fnFilter
//
//	Filters a list based on a boolean expression.
//
//	(filter list var exp) -> list

	{
	int i;
	CCodeChain *pCC = pCtx->pCC;

	if (pArgs->GetElement(0)->IsNil())
		return pCC->CreateNil();

	//	Args

	ICCItem *pSource = pArgs->GetElement(0);
	ICCItem *pVar = pArgs->GetElement(1);
	ICCItem *pBody = pArgs->GetElement(2);

	//	Create a destination list

	ICCItem *pResult = pCC->CreateLinkedList();
	if (pResult->IsError())
		return pResult;

	CCLinkedList *pList = (CCLinkedList *)pResult;

	//	Setup the locals. We start by creating a local symbol table

	ICCItem *pLocalSymbols = pCC->CreateSymbolTable();
	if (pLocalSymbols->IsError())
		{
		pResult->Discard(pCC);
		return pLocalSymbols;
		}

	//	Associate the enumaration variable

	ICCItem *pError = pLocalSymbols->AddEntry(pCC, pVar, pCC->CreateNil());
	if (pError->IsError())
		{
		pLocalSymbols->Discard(pCC);
		pResult->Discard(pCC);
		return pError;
		}

	pError->Discard(pCC);

	//	Setup the context

	if (pCtx->pLocalSymbols)
		pLocalSymbols->SetParent(pCtx->pLocalSymbols);
	else
		pLocalSymbols->SetParent(pCtx->pLexicalSymbols);
	ICCItem *pOldSymbols = pCtx->pLocalSymbols;
	pCtx->pLocalSymbols = pLocalSymbols;

	//	Get the offset of the variable so we don't have to
	//	search for it all the time

	int iVarOffset = pLocalSymbols->FindOffset(pCC, pVar);

	//	Loop over all elements of the list

	for (i = 0; i < pSource->GetCount(); i++)
		{
		ICCItem *pItem = pSource->GetElement(i);

		//	Set the element

		pLocalSymbols->AddByOffset(pCC, iVarOffset, pItem);

		//	Eval

		ICCItem *pSelect = pCC->Eval(pCtx, pBody);
		if (pSelect->IsError())
			{
			pResult->Discard(pCC);
			pResult = pSelect;
			break;
			}

		//	If the evaluation is not Nil, then we include the
		//	item in the result

		if (!pSelect->IsNil())
			pList->Append(pCC, pItem, NULL);

		pSelect->Discard(pCC);
		}

	//	Clean up

	pCtx->pLocalSymbols = pOldSymbols;
	pLocalSymbols->Discard(pCC);

	//	Done

	if (pResult->GetCount() > 0)
		return pResult;
	else
		{
		pResult->Discard(pCC);
		return pCC->CreateNil();
		}
	}

ICCItem *fnFind (CEvalContext *pCtx, ICCItem *pArgs, DWORD dwData)

//	fnFind
//
//	Finds a target in a source
//
//	(find source target ['sorted] [keyIndex])

	{
	int i;
	CCodeChain *pCC = pCtx->pCC;

	//	Get the source and target

	ICCItem *pSource = pArgs->GetElement(0);
	ICCItem *pTarget = pArgs->GetElement(1);

	//	Read optional flags

	int iSorted = 0;
	int iOptionalArg = 2;
	if (pArgs->GetCount() > iOptionalArg && !pArgs->GetElement(iOptionalArg)->IsInteger())
		{
		for (i = 0; i < pArgs->GetElement(iOptionalArg)->GetCount(); i++)
			{
			if (strEquals(pArgs->GetElement(iOptionalArg)->GetElement(i)->GetStringValue(), CONSTLIT("ascending")))
				iSorted = AscendingSort;
			else if (strEquals(pArgs->GetElement(iOptionalArg)->GetElement(i)->GetStringValue(), CONSTLIT("descending")))
				iSorted = DescendingSort;
			else if (strEquals(pArgs->GetElement(iOptionalArg)->GetElement(i)->GetStringValue(), CONSTLIT("sorted")))
				iSorted = AscendingSort;
			}

		iOptionalArg++;
		}

	//	If we specify a third argument, then we are searching a list
	//	of entries and want to match the nth element of an entry
	//	with the target

	int iListKey;
	if (pArgs->GetCount() > iOptionalArg && !pArgs->GetElement(iOptionalArg)->IsNil())
		iListKey = pArgs->GetElement(iOptionalArg++)->GetIntegerValue();
	else
		iListKey = -1;

	//	iPos will be the position that we want to return.

	int iPos = -1;

	//	If the source list is sorted, then use a binary search

	if (iSorted && pSource->IsList())
		{
		int iCount = pSource->GetCount();
		int iMin = 0;
		int iMax = iCount;
		int iTry = iMax / 2;

		if (iListKey != -1)
			{
			while (true)
				{
				if (iMax <= iMin)
					//	Not found
					break;

				ICCItem *pTry = pSource->GetElement(iTry)->GetElement(iListKey);
				int iCompare = iSorted * HelperCompareItems((pTry ? pTry : pCC->CreateNil()), pTarget);
				if (iCompare == 0)
					{
					//	Found
					iPos = iTry;
					break;
					}
				else if (iCompare < 0)
					{
					iMin = iTry + 1;
					iTry = iMin + (iMax - iMin) / 2;
					}
				else if (iCompare > 0)
					{
					iMax = iTry;
					iTry = iMin + (iMax - iMin) / 2;
					}
				}
			}
		else
			{
			while (true)
				{
				if (iMax <= iMin)
					//	Not found
					break;

				int iCompare = iSorted * HelperCompareItems(pSource->GetElement(iTry), pTarget);
				if (iCompare == 0)
					{
					//	Found
					iPos = iTry;
					break;
					}
				else if (iCompare < 0)
					{
					iMin = iTry + 1;
					iTry = iMin + (iMax - iMin) / 2;
					}
				else if (iCompare > 0)
					{
					iMax = iTry;
					iTry = iMin + (iMax - iMin) / 2;
					}
				}
			}
		}

	//	If this is a list, then look for the target in the list and
	//	return the item position in the list

	else if (pSource->IsList())
		{
		if (iListKey != -1)
			{
			for (i = 0; i < pSource->GetCount(); i++)
				{
				ICCItem *pEntry = pSource->GetElement(i);
				if (pEntry->GetCount() > iListKey
						&& HelperCompareItems(pEntry->GetElement(iListKey), pTarget, false) == 0)
					{
					iPos = i;
					break;
					}
				}
			}
		else
			{
			for (i = 0; i < pSource->GetCount(); i++)
				{
				ICCItem *pItem = pSource->GetElement(i);
				if (HelperCompareItems(pItem, pTarget, false) == 0)
					{
					iPos = i;
					break;
					}
				}
			}
		}

	//	Otherwise, look for the target string in the source string

	else
		iPos = strFind(pSource->GetStringValue(), pTarget->GetStringValue());

	//	Done

	if (iPos == -1)
		return pCC->CreateNil();
	else if (dwData == FN_LOOKUP && pSource->IsList())
		return pSource->GetElement(iPos)->Reference();
	else
		return pCC->CreateInteger(iPos);
	}

ICCItem *fnForLoop (CEvalContext *pCtx, ICCItem *pArguments, DWORD dwData)

//	fnForLoop
//
//	Evaluates an expression for a given number of iterations
//	Iterates from "from" to "to" inclusive.
//
//	(for var from to exp)

	{
	CCodeChain *pCC = pCtx->pCC;
	ICCItem *pArgs;
	ICCItem *pResult;
	ICCItem *pVar;
	ICCItem *pBody;
	ICCItem *pLocalSymbols;
	ICCItem *pOldSymbols;
	ICCItem *pError;
	int i, iFrom, iTo, iVarOffset;

	//	Evaluate the arguments and validate them

	pArgs = pCC->EvaluateArgs(pCtx, pArguments, CONSTLIT("qvvu"));
	if (pArgs->IsError())
		return pArgs;

	pVar = pArgs->GetElement(0);
	iFrom = pArgs->GetElement(1)->GetIntegerValue();
	iTo = pArgs->GetElement(2)->GetIntegerValue();
	pBody = pArgs->GetElement(3);

	//	Setup the locals. We start by creating a local symbol table

	pLocalSymbols = pCC->CreateSymbolTable();
	if (pLocalSymbols->IsError())
		{
		pArgs->Discard(pCC);
		return pLocalSymbols;
		}

	//	Associate the enumaration variable

	pError = pLocalSymbols->AddEntry(pCC, pVar, pCC->CreateNil());
	if (pError->IsError())
		{
		pArgs->Discard(pCC);
		return pError;
		}

	pError->Discard(pCC);

	//	Setup the context

	if (pCtx->pLocalSymbols)
		pLocalSymbols->SetParent(pCtx->pLocalSymbols);
	else
		pLocalSymbols->SetParent(pCtx->pLexicalSymbols);
	pOldSymbols = pCtx->pLocalSymbols;
	pCtx->pLocalSymbols = pLocalSymbols;

	//	Start with a default result

	pResult = pCC->CreateNil();

	//	Get the offset of the variable so we don't have to
	//	search for it all the time

	iVarOffset = pLocalSymbols->FindOffset(pCC, pVar);

	//	Loop over all elements of the list

	for (i = iFrom; i <= iTo; i++)
		{
		ICCItem *pItem = pCC->CreateInteger(i);

		//	Clean up the previous result

		pResult->Discard(pCC);

		//	Set the element

		pLocalSymbols->AddByOffset(pCC, iVarOffset, pItem);
		pItem->Discard(pCC);

		//	Eval

		pResult = pCC->Eval(pCtx, pBody);
		if (pResult->IsError())
			break;
		}

	//	Clean up

	pCtx->pLocalSymbols = pOldSymbols;
	pLocalSymbols->Discard(pCC);

	//	Done

	pArgs->Discard(pCC);
	return pResult;
	}

void OutputFunctionName (CMemoryWriteStream &Output, const CString &sHelp)
	{
	char *pPos = sHelp.GetASCIIZPointer();

	//	Only until we reach the body of the help. We expect the body of the
	//	help to be separated by two newlines.

	char *pStart = pPos;
	while (*pPos != '\0')
		{
		if (pPos[0] == '\n' && pPos[1] == '\n')
			break;

		pPos++;
		}

	//	Output

	Output.Write(pStart, (int)(pPos - pStart));
	}

ICCItem *fnHelp (CEvalContext *pCtx, ICCItem *pArgs, DWORD dwData)

//	fnHelp
//
//	(help)

	{
	CCodeChain *pCC = pCtx->pCC;
	int i;

	//	Prepare some output

	CMemoryWriteStream Output;
	if (Output.Create() != NOERROR)
		return pCC->CreateError(CONSTLIT("Out of memory."));

	//	If no parameters, then we show some help on help

	if (pArgs->GetCount() == 0)
		{
		CString sHelp = CONSTLIT("(help) -> this help\n(help '*) -> all functions\n(help 'partial-string) -> all functions starting with partial-string\n(help 'function-name) -> help on function-name\n");
		Output.Write(sHelp.GetASCIIZPointer(), sHelp.GetLength());
		}

	//	If parameter is * then show all functions

	else if (strEquals(pArgs->GetElement(0)->GetStringValue(), CONSTLIT("*")))
		{
		ICCItem *pGlobals = pCC->GetGlobals();
		for (i = 0; i < pGlobals->GetCount(); i++)
			{
			ICCItem *pItem = pGlobals->GetElement(i);
			if (pItem->IsFunction())
				{
				CString sHelp = pItem->GetHelp();

				//	If blank or deprecated, skip

				if (!sHelp.IsBlank() && !strStartsWith(sHelp, CONSTLIT("DEPRECATED")))
					{
					OutputFunctionName(Output, sHelp);
					Output.Write("\n", 1);
					}
				}
			}
		}

	//	Otherwise, look for the function

	else
		{
		CString sPartial = pArgs->GetElement(0)->GetStringValue();
		TArray<CString> Help;

		//	Compile a list of all functions that match

		CCSymbolTable *pGlobals = (CCSymbolTable *)pCC->GetGlobals();
		for (i = 0; i < pGlobals->GetCount(); i++)
			{
			ICCItem *pItem = pGlobals->GetElement(i);

			if (pItem->IsFunction() && strStartsWith(pGlobals->GetKey(i), sPartial))
				{
				CString sHelp = pItem->GetHelp();

				if (!sHelp.IsBlank() && !strStartsWith(sHelp, CONSTLIT("DEPRECATED")))
					Help.Insert(sHelp);
				}
			}

		//	Output

		if (Help.GetCount() == 1)
			Output.Write(Help[0].GetASCIIZPointer(), Help[0].GetLength());
		else
			{
			for (i = 0; i < Help.GetCount(); i++)
				{
				OutputFunctionName(Output, Help[i]);
				Output.Write("\n", 1);
				}
			}
		}

	//	Done

	return pCC->CreateString(CString(Output.GetPointer(), Output.GetLength()));
	}

ICCItem *fnIf (CEvalContext *pCtx, ICCItem *pArguments, DWORD dwData)

//	fnIf
//
//	If control function
//
//	(if exp then else)

	{
	CCodeChain *pCC = pCtx->pCC;
	ICCItem *pTest;
	ICCItem *pThen;
	ICCItem *pElse;
	ICCItem *pResult;

	//	Get the arguments

	pTest = pArguments->GetElement(0);
	if (pTest == NULL)
		return pCC->CreateError(CONSTLIT("Conditional expression expected"));

	pThen = pArguments->GetElement(1);
	if (pThen == NULL)
		return pCC->CreateError(CONSTLIT("Then expression expected"));

	pElse = pArguments->GetElement(2);

	//	Evaluate the test expression

	pResult = pCC->Eval(pCtx, pTest);
	if (pResult->IsError())
		return pResult;

	//	If the result is not true, evaluate the else expression

	if (pResult->IsNil())
		{
		if (pElse)
			{
			pResult->Discard(pCC);
			return pCC->Eval(pCtx, pElse);
			}
		else
			return pResult;
		}

	//	Otherwise, evaluate the then expression

	pResult->Discard(pCC);
	return pCC->Eval(pCtx, pThen);
	}

ICCItem *fnItem (CEvalContext *pCtx, ICCItem *pArgs, DWORD dwData)

//	fnItem
//
//	Returns nth entry in a list (0-based)
//
//	(@ list nth) -> item
//	(set@ list|struct index value) -> value

	{
	CCodeChain *pCC = pCtx->pCC;
	int i;

	switch (dwData)
		{
		case FN_ITEM:
			{
			ICCItem *pList = pArgs->GetElement(0);

			//	If index is nil then we always return nil

			if (pArgs->GetElement(1)->IsNil())
				return pCC->CreateNil();

			//	Handle symbol tables differently

			if (pList->IsSymbolTable())
				{
				BOOL bFound;
				ICCItem *pResult = pList->LookupEx(pCC, pArgs->GetElement(1), &bFound);
				if (!bFound)
					{
					pResult->Discard(pCC);
					return pCC->CreateNil();
					}

				return pResult;
				}

			//	Normal lists

			else
				{
				ICCItem *pResult = pList->GetElement(pArgs->GetElement(1)->GetIntegerValue());
				if (pResult == NULL)
					return pCC->CreateNil();
				else
					return pResult->Reference();
				}
			}

		case FN_ITEM_TYPE:
			{
			ICCItem *pItem = pArgs->GetElement(0);
			return pCC->CreateString(pItem->GetTypeOf());
			}

		case FN_SET_ITEM:
			{
			//	We evaluate the target. We either end up with an array or a
			//	structure.

			ICCItem *pTarget = pCC->Eval(pCtx, pArgs->GetElement(0));

			//	If index is nil then we always return the target (nothing to do)

			if (pArgs->GetElement(1)->IsNil())
				return pTarget;

			//	Figure out the data type of the key, because we sometimes need
			//	to use that to figure out what to do.

			bool bUseArray = (pArgs->GetElement(1)->IsInteger() ? true : false);

			//	If the target is a valid array, then set the item there.

			if (pTarget->GetClass()->GetObjID() == OBJID_CCLINKEDLIST)
				bUseArray = true;

			//	If the target is a valid structure, then set the item there.

			else if (pTarget->IsSymbolTable())
				bUseArray = false;

			//	If we have an error or nil, then we need to create a new array
			//	or structure

			else if (pTarget->IsError() || pTarget->IsNil())
				{
				pTarget->Discard(pCC);

				if (bUseArray)
					{
					pTarget = pCC->CreateLinkedList();
					if (pTarget->IsError())
						return pTarget;
					}
				else
					{
					pTarget = pCC->CreateSymbolTable();
					if (pTarget->IsError())
						return pTarget;
					}

				//	If the first arg is a variable, bind it now

				if (pArgs->GetElement(0)->IsIdentifier())
					{
					ICCItem *pError;
					if (HelperSetq(pCtx, pArgs->GetElement(0), pTarget, &pError) != NOERROR)
						{
						pTarget->Discard(pCC);
						return pError;
						}
					}
				}
			else
				{
				pTarget->Discard(pCC);
				return pCC->CreateError(CONSTLIT("List or struct expected"));
				}

			//	If bUseArray then pTarget is a list; Otherwise, pTarget is a
			//	structure. In either case, we already have a reference on it.

			if (bUseArray)
				{
				CCLinkedList *pLinkedList = (CCLinkedList *)pTarget;

				if (pArgs->GetCount() < 3)
					{
					pTarget->Discard(pCC);
					return pCC->CreateError(CONSTLIT("Not enough parameters for set@. Value expected."));
					}

				int iIndex = pArgs->GetElement(1)->GetIntegerValue();
				ICCItem *pItem = pArgs->GetElement(2);

				//	Make sure we're in range

				if (iIndex < 0 || iIndex >= pLinkedList->GetCount())
					{
					pTarget->Discard(pCC);
					return pCC->CreateError(CONSTLIT("Index out of range"), pArgs->GetElement(1));
					}

				pLinkedList->ReplaceElement(pCC, iIndex, pItem);
				return pTarget;
				}

			//	If we have a structure instead of a key, then we merge with the
			//	target.

			else if (pArgs->GetElement(1)->IsSymbolTable())
				{
				CCSymbolTable *pTable = (CCSymbolTable *)pArgs->GetElement(1);

				for (i = 0; i < pTable->GetCount(); i++)
					{
					ICCItem *pKey = pCC->CreateString(pTable->GetKey(i));
					ICCItem *pItem = pTable->GetElement(i);

					ICCItem *pError = pTarget->AddEntry(pCC, pKey, pItem);
					pKey->Discard(pCC);
					if (pError->IsError())
						{
						pTarget->Discard(pCC);
						return pError;
						}

					pError->Discard(pCC);
					}

				return pTarget;
				}
			
			//	Set the structure item

			else
				{
				if (pArgs->GetCount() < 3)
					{
					pTarget->Discard(pCC);
					return pCC->CreateError(CONSTLIT("Not enough parameters for set@. Value expected."));
					}

				ICCItem *pKey = pArgs->GetElement(1);
				ICCItem *pValue = pArgs->GetElement(2);

				//	If pValue is nil then we remove the item from the structure

				if (pValue->IsNil())
					{
					pTarget->DeleteEntry(pCC, pKey);
					}

				//	Otherwise, add it

				else
					{
					ICCItem *pError = pTarget->AddEntry(pCC, pKey, pValue);
					if (pError->IsError())
						{
						pTarget->Discard(pCC);
						return pError;
						}

					pError->Discard(pCC);
					}

				return pTarget;
				}
			}

		default:
			ASSERT(false);
			return pCC->CreateNil();
		}
	}

ICCItem *fnItemInfo (CEvalContext *pCtx, ICCItem *pArguments, DWORD dwData)

//	fnItemInfo
//
//	Returns info about a single item
//
//	(isatom item) -> True/Nil
//	(iserror item) -> True/Nil
//	(isfunction item) -> True/Nil

	{
	CCodeChain *pCC = pCtx->pCC;
	ICCItem *pArgs;
	ICCItem *pResult;

	//	Evaluate the arguments and validate them

	pArgs = pCC->EvaluateArgs(pCtx, pArguments, CONSTLIT("v"));
	if (pArgs->IsError())
		return pArgs;

	//	Do the right thing

	switch (dwData)
		{
		case FN_ITEMINFO_ISERROR:
			{
			if (pArgs->Head(pCC)->IsError())
				pResult = pCC->CreateTrue();
			else
				pResult = pCC->CreateNil();
			break;
			}

		case FN_ITEMINFO_ISATOM:
			{
			if (pArgs->Head(pCC)->IsAtom())
				pResult = pCC->CreateTrue();
			else
				pResult = pCC->CreateNil();
			break;
			}

		case FN_ITEMINFO_ISINT:
			{
			//	Right now, IsInteger returns True for a value of Nil. This
			//	was probably so that Nil could be accepted for functions that
			//	take integers are arguments.

			pResult = pCC->CreateBool((pArgs->Head(pCC)->IsInteger() && !pArgs->Head(pCC)->IsNil()) ? true : false);
			break;
			}

		case FN_ITEMINFO_ISFUNCTION:
			{
			if (pArgs->Head(pCC)->IsFunction())
				pResult = pCC->CreateTrue();
			else
				pResult = pCC->CreateNil();
			break;
			}

		case FN_ITEMINFO_ISPRIMITIVE:
			pResult = pCC->CreateBool(pArgs->Head(pCC)->IsPrimitive() ? true : false);
			break;

		case FN_ITEMINFO_ASINT:
			{
			ICCItem *pValue = pArgs->GetElement(0);
			if (pValue->IsNil())
				pResult = pCC->CreateInteger(0);
			else if (pValue->IsInteger())
				pResult = pValue->Reference();
			else if (pValue->IsIdentifier())
				{
				bool bFailed;
				int iResult = strToInt(pValue->GetStringValue(), 0, &bFailed);
				if (!bFailed)
					pResult = pCC->CreateInteger(iResult);
				else
					pResult = pCC->CreateNil();
				}
			else
				pResult = pCC->CreateInteger(pValue->GetIntegerValue());
			break;
			}

		case FN_ITEMINFO_HELP:
			{
			CString sHelp = pArgs->GetElement(0)->GetHelp();
			if (sHelp.IsBlank())
				pResult = pCC->CreateNil();
			else
				pResult = pCC->CreateString(sHelp);
			break;
			}

		default:
			ASSERT(FALSE);
			return NULL;
		}

	//	Done

	pArgs->Discard(pCC);
	return pResult;
	}

ICCItem *fnLambda (CEvalContext *pCtx, ICCItem *pArguments, DWORD dwData)

//	fnLambda
//
//	Lambda expression
//
//	(lambda (args...) code)

	{
	CCodeChain *pCC = pCtx->pCC;
	ICCItem *pItem;
	CCLambda *pLambda;

	//	Create a lambda expression based on this list

	pItem = pCC->CreateLambda(pArguments, TRUE);
	if (pItem->IsError())
		return pItem;

	pLambda = dynamic_cast<CCLambda *>(pItem);

	//	Set the local symbols at the time that we evaluate the
	//	expression (we will use these symbols when the lambda expression
	//	is applied).

	if (pCtx->pLocalSymbols)
		pLambda->SetLocalSymbols(pCC, pCtx->pLocalSymbols);

	//	Done

	return pLambda;
	}

ICCItem *fnLink (CEvalContext *pCtx, ICCItem *pArgs, DWORD dwData)

//	fnLink
//
//	Converts a string into an expression

	{
	CCodeChain *pCC = pCtx->pCC;
	return pCC->Link(pArgs->GetElement(0)->GetStringValue(), 0, NULL);
	}

ICCItem *fnLinkedList (CEvalContext *pCtx, ICCItem *pArguments, DWORD dwData)

//	fnLinkedList
//
//	Handles linked-list specific functions
//
//	(lnkAppend linked-list item) -> list
//	(lnkRemove linked-list index) -> list
//	(lnkRemoveNil linked-list) -> list
//	(lnkReplace linked-list index item) -> list
//
//	HACK: This function has different behavior depending on the first
//	argument. If the first argument is a variable holding a linked list,
//	then the variable contents will be changed. If the variable holds Nil,
//	then the variable contents are not changed. In all cases, the caller
//	should structure the call as: (setq ListVar (lnkAppend ListVar ...))
//	in order to handle all cases.

	{
	CCodeChain *pCC = pCtx->pCC;
	ICCItem *pArgs;
	CCLinkedList *pLinkedList;
	ICCItem *pResult;

	//	Evaluate the arguments

	if (dwData == FN_LINKEDLIST_REMOVE_NIL)
		pArgs = pCC->EvaluateArgs(pCtx, pArguments, CONSTLIT("q"));
	else if (dwData == FN_LINKEDLIST_REMOVE)
		//	This was previously (and erroneously) "qvv". For backwards
		//	compatibility we allow a third argument, but ignore it.
		pArgs = pCC->EvaluateArgs(pCtx, pArguments, CONSTLIT("qv*"));
	else
		pArgs = pCC->EvaluateArgs(pCtx, pArguments, CONSTLIT("qvv"));
	if (pArgs->IsError())
		return pArgs;

	//	Get the linked list

	ICCItem *pList = pCC->Eval(pCtx, pArgs->GetElement(0));
	if (pList->GetClass()->GetObjID() == OBJID_CCLINKEDLIST)
		pLinkedList = (CCLinkedList *)pList;
	else if (pList->IsError() || pList->IsNil())
		{
		pList->Discard(pCC);
		pList = pCC->CreateLinkedList();
		if (pList->IsError())
			{
			pArgs->Discard(pCC);
			return pList;
			}
		pLinkedList = (CCLinkedList *)pList;

		//	If the first arg is a variable, bind it now

		if (pArguments->GetElement(0)->IsIdentifier())
			{
			ICCItem *pError;
			if (HelperSetq(pCtx, pArguments->GetElement(0), pList, &pError) != NOERROR)
				{
				pList->Discard(pCC);
				pArgs->Discard(pCC);
				return pError;
				}
			}
		}
	else
		{
		pList->Discard(pCC);
		pArgs->Discard(pCC);
		return pCC->CreateError(CONSTLIT("Linked-list expected"));
		}

	//	Do the right thing

	switch (dwData)
		{
		case FN_LINKEDLIST_REMOVE:
			{
			//	If the index is nil then we don't do anything (but we still 
			//	return the untouched list).

			if (pArgs->GetElement(1)->IsNil())
				pResult = pLinkedList;

			//	Otherwise, we expect an integer

			else
				{
				int iIndex = pArgs->GetElement(1)->GetIntegerValue();

				//	Make sure we're in range

				if (iIndex < 0 || iIndex >= pLinkedList->GetCount())
					{
					pLinkedList->Discard(pCC);
					pResult = pCC->CreateError(CONSTLIT("Index out of range"), pArgs->GetElement(1));
					}
				else
					{
					pLinkedList->RemoveElement(pCC, iIndex);
					pResult = pLinkedList;
					}
				}

			break;
			}

		case FN_LINKEDLIST_REMOVE_NIL:
			{
			//	Iterate over all elements and remove any elements that are Nil

			int iIndex = 0;
			while (iIndex < pLinkedList->GetCount())
				{
				if (pLinkedList->GetElement(iIndex)->IsNil())
					pLinkedList->RemoveElement(pCC, iIndex);
				else
					iIndex++;
				}

			//	Done

			pResult = pLinkedList;
			break;
			}

		case FN_LINKEDLIST_REPLACE:
			{
			//	If the index is nil then we do nothing and just return the 
			//	unmodified list.

			if (pArgs->GetElement(1)->IsNil())
				pResult = pLinkedList;

			//	Otherwise, we expect an integer

			else
				{
				int iIndex = pArgs->GetElement(1)->GetIntegerValue();
				ICCItem *pItem = pArgs->GetElement(2);

				//	Make sure we're in range

				if (iIndex < 0 || iIndex >= pLinkedList->GetCount())
					{
					pLinkedList->Discard(pCC);
					pResult = pCC->CreateError(CONSTLIT("Index out of range"), pArgs->GetElement(1));
					}
				else
					{
					pLinkedList->ReplaceElement(pCC, iIndex, pItem);
					pResult = pLinkedList;
					}
				}

			break;
			}

		default:
			ASSERT(FALSE);
			return NULL;
		}

	//	Done

	pArgs->Discard(pCC);
	return pResult;
	}

ICCItem *fnLinkedListAppend (CEvalContext *pCtx, ICCItem *pArgs, DWORD dwData)

//	fnLinkedListAppend
//
//	(lnkAppend linked-list item) -> list
//
//	HACK: This function has different behavior depending on the first
//	argument. If the first argument is a variable holding a linked list,
//	then the variable contents will be changed. If the variable holds Nil,
//	then the variable contents are not changed. In all cases, the caller
//	should structure the call as: (setq ListVar (lnkAppend ListVar ...))
//	in order to handle all cases.

	{
	int i;
	CCodeChain *pCC = pCtx->pCC;
	CCLinkedList *pLinkedList = NULL;

	//	If the first argument is an identifier, then we expect it
	//	to be a variable (it can be unbound)

	if (pArgs->GetElement(0)->IsIdentifier())
		{
		ICCItem *pList = pCC->Eval(pCtx, pArgs->GetElement(0));

		//	If we are bound to a list, then we just append the variable

		if (pList->GetClass()->GetObjID() == OBJID_CCLINKEDLIST)
			pLinkedList = (CCLinkedList *)pList;

		//	If the variable is unbound, or if the value is Nil then
		//	we create a new list and bind it to the variable.

		else if (pList->IsError() || pList->IsNil())
			{
			pList->Discard(pCC);

			pLinkedList = (CCLinkedList *)pCC->CreateLinkedList();
			if (pLinkedList->IsError())
				return pLinkedList;

			ICCItem *pError;
			if (HelperSetq(pCtx, pArgs->GetElement(0), pLinkedList, &pError) != NOERROR)
				{
				pLinkedList->Discard(pCC);
				return pError;
				}
			}

		//	Otherwise, error

		else
			{
			pList->Discard(pCC);
			return pCC->CreateError(CONSTLIT("Linked-list expected"), pArgs->GetElement(0));
			}
		}

	//	Otherwise we have an expression

	else
		{
		ICCItem *pList = pCC->Eval(pCtx, pArgs->GetElement(0));

		//	Create a new list (because we don't want to clober some random list
		//	that was passed in).

		pLinkedList = (CCLinkedList *)pCC->CreateLinkedList();
		if (pLinkedList->IsError())
			return pLinkedList;

		//	Copy the elements

		for (i = 0; i < pList->GetCount(); i++)
			{
			ICCItem *pError;
			pLinkedList->Append(pCC, pList->GetElement(i), &pError);
			if (pError->IsError())
				{
				pList->Discard(pCC);
				pLinkedList->Discard(pCC);
				return pError;
				}
			else
				pError->Discard(pCC);
			}

		//	Done with the original list

		pList->Discard(pCC);
		}

	//	At this point pLinkedList is a linked list and it has a ref count
	//	[All other items have been discarded]

	//	Append the item

	ICCItem *pError;
	pLinkedList->Append(pCC, pArgs->GetElement(1), &pError);
	if (pError->IsError())
		{
		pLinkedList->Discard(pCC);
		return pError;
		}
	else
		pError->Discard(pCC);

	return pLinkedList;
	}

ICCItem *fnList (CEvalContext *pCtx, ICCItem *pArguments, DWORD dwData)

//	fnList
//
//	Creates a list out of multiple arguments
//
//	(list exp1 exp2 ... expn)

	{
	CCodeChain *pCC = pCtx->pCC;
	ICCItem *pArgs;

	//	Evaluate the arguments

	pArgs = pCC->EvaluateArgs(pCtx, pArguments, CONSTLIT("*"));
	if (pArgs->IsError())
		return pArgs;

	//	Return the args (since they are already in a list!)

	return pArgs;
	}

ICCItem *fnLogical (CEvalContext *pCtx, ICCItem *pArguments, DWORD dwData)

//	fnLogical
//
//	Logical operators
//
//	(and exp1 exp2 ... expn)
//	(or exp1 exp2 ... expn)

	{
	CCodeChain *pCC = pCtx->pCC;
	int i;

	//	Loop over all arguments

	for (i = 0; i < pArguments->GetCount(); i++)
		{
		ICCItem *pResult;
		ICCItem *pArg = pArguments->GetElement(i);

		//	Evaluate the item

		if (pArg->IsQuoted())
			pResult = pArg->Reference();
		else
			{
			pResult = pCC->Eval(pCtx, pArg);

			if (pResult->IsError())
				return pResult;
			}

		//	If we are evaluating NOT then reverse the value

		if (dwData == FN_LOGICAL_NOT)
			{
			if (pResult->IsNil())
				{
				pResult->Discard(pCC);
				return pCC->CreateTrue();
				}
			else
				{
				pResult->Discard(pCC);
				return pCC->CreateNil();
				}
			}

		//	If we are evaluating AND and we've got Nil, then
		//	we can guarantee that the expression is Nil

		else if (dwData == FN_LOGICAL_AND && pResult->IsNil())
			return pResult;

		//	Otherwise, if we're evaluating OR and we've got non-Nil,
		//	then we can guarantee that the expression is True.
		//	(We return the value of the expression)

		else if (dwData == FN_LOGICAL_OR && !pResult->IsNil())
			return pResult;

		//	If we evaluated the last expression and its not Nil,
		//	then we return it

		else if (dwData == FN_LOGICAL_AND && (i == pArguments->GetCount() - 1))
			return pResult;

		//	Otherwise, we continue

		pResult->Discard(pCC);
		}

	//	If we get here then all the operands are the same (either all
	//	True or all Nil depending)

	if (dwData == FN_LOGICAL_AND)
		return pCC->CreateTrue();
	else
		return pCC->CreateNil();
	}

ICCItem *fnLoop (CEvalContext *pCtx, ICCItem *pArguments, DWORD dwData)

//	fnLoop
//
//	Evaluates an expression until the condition is Nil
//
//	(loop condition exp)

	{
	CCodeChain *pCC = pCtx->pCC;
	ICCItem *pResult;
	ICCItem *pCondition;
	ICCItem *pBody;
	BOOL bDone;

	//	The first argument must be a conditional expression

	pCondition = pArguments->Head(pCC);
	if (pCondition == NULL)
		return pCC->CreateError(CONSTLIT("Loop condition expected"));

	//	The second argument is the body

	pBody = pArguments->GetElement(1);
	if (pBody == NULL)
		return pCC->CreateError(CONSTLIT("Loop body expected"));

	//	Create a default result

	pResult = pCC->CreateNil();

	//	Keep evaluating the condition and looping
	//	until we get an error or the condition is Nil

	bDone = FALSE;
	do
		{
		ICCItem *pEval;

		//	Evaluate the condition

		pEval = pCC->Eval(pCtx, pCondition);
		if (pEval->IsError())
			return pResult;

		//	If the condition is Nil, then we're done

		if (pEval->IsNil())
			{
			pEval->Discard(pCC);
			bDone = TRUE;
			}

		//	Otherwise, evaluate the body of the loop

		else
			{
			pEval->Discard(pCC);

			//	Clean up the old result

			pResult->Discard(pCC);

			//	Evaluate the body

			pResult = pCC->Eval(pCtx, pBody);
			if (pResult->IsError())
				return pResult;
			}
		}
	while (!bDone);

	//	Done

	return pResult;
	}

ICCItem *fnMap (CEvalContext *pCtx, ICCItem *pArgs, DWORD dwData)

//	fnMap
//
//	Maps every item in a list
//
//	(map list ['excludeNil] var exp) -> list

	{
	int i;
	CCodeChain *pCC = pCtx->pCC;

	if (pArgs->GetElement(0)->IsNil())
		return pCC->CreateNil();

	//	List

	ICCItem *pSource = pArgs->GetElement(0);

	//	Options

	bool bExcludeNil = false;
	bool bReduceMin = false;
	bool bReduceMax = false;
	bool bReduceAverage = false;
	bool bReduceSum = false;
	bool bOriginal = false;

	int iOptionalArg = 1;
	if (pArgs->GetCount() > 3)
		{
		ICCItem *pOptions = pArgs->GetElement(iOptionalArg++);
		for (i = 0; i < pOptions->GetCount(); i++)
			{
			CString sValue = pOptions->GetElement(i)->GetStringValue();

			if (strEquals(sValue, CONSTLIT("excludeNil")))
				bExcludeNil = true;
			else if (strEquals(sValue, CONSTLIT("original")))
				bOriginal = true;
			else if (strEquals(sValue, CONSTLIT("reduceMax")))
				bReduceMax = true;
			else if (strEquals(sValue, CONSTLIT("reduceMin")))
				bReduceMin = true;
			else if (strEquals(sValue, CONSTLIT("reduceAverage")))
				bReduceAverage = true;
			else if (strEquals(sValue, CONSTLIT("reduceSum")))
				bReduceSum = true;
			}
		}

	//	Var and body
	
	ICCItem *pVar = pArgs->GetElement(iOptionalArg++);
	ICCItem *pBody = pArgs->GetElement(iOptionalArg++);

	//	Create a destination list

	ICCItem *pResult = pCC->CreateLinkedList();
	if (pResult->IsError())
		return pResult;

	CCLinkedList *pList = (CCLinkedList *)pResult;

	//	Setup the locals. We start by creating a local symbol table

	ICCItem *pLocalSymbols = pCC->CreateSymbolTable();
	if (pLocalSymbols->IsError())
		{
		pResult->Discard(pCC);
		return pLocalSymbols;
		}

	//	Associate the enumeration variable

	ICCItem *pError = pLocalSymbols->AddEntry(pCC, pVar, pCC->CreateNil());
	if (pError->IsError())
		{
		pLocalSymbols->Discard(pCC);
		pResult->Discard(pCC);
		return pError;
		}

	pError->Discard(pCC);

	//	Setup the context

	if (pCtx->pLocalSymbols)
		pLocalSymbols->SetParent(pCtx->pLocalSymbols);
	else
		pLocalSymbols->SetParent(pCtx->pLexicalSymbols);
	ICCItem *pOldSymbols = pCtx->pLocalSymbols;
	pCtx->pLocalSymbols = pLocalSymbols;

	//	Get the offset of the variable so we don't have to
	//	search for it all the time

	int iVarOffset = pLocalSymbols->FindOffset(pCC, pVar);

	//	Loop over all elements of the list

	int iBestItem = -1;
	int iAccumulate = 0;
	int iCount = 0;
	for (i = 0; i < pSource->GetCount(); i++)
		{
		ICCItem *pItem = pSource->GetElement(pCC, i);

		//	Set the element

		pLocalSymbols->AddByOffset(pCC, iVarOffset, pItem);
		pItem->Discard(pCC);

		//	Eval

		ICCItem *pMapped = pCC->Eval(pCtx, pBody);
		if (pMapped->IsError())
			{
			pResult->Discard(pCC);
			pResult = pMapped;
			break;
			}

		//	If result is Nil, then exclude

		if (bExcludeNil && pMapped->IsNil())
			;

		//	If we're reducing to max/min, then look for the max/min value

		else if (bReduceMax)
			{
			int iValue = pMapped->GetIntegerValue();
			if (iBestItem == -1 || iValue > iAccumulate)
				{
				iBestItem = i;
				iAccumulate = iValue;
				}
			}
		else if (bReduceMin)
			{
			int iValue = pMapped->GetIntegerValue();
			if (iBestItem == -1 || iValue < iAccumulate)
				{
				iBestItem = i;
				iAccumulate = iValue;
				}
			}
		else if (bReduceAverage || bReduceSum)
			{
			iAccumulate += pMapped->GetIntegerValue();
			iCount++;
			}

		//	Add the mapped value to the result

		else
			{
			if (bOriginal)
				pList->Append(pCC, pItem);
			else
				pList->Append(pCC, pMapped);
			}

		//	Next

		pMapped->Discard(pCC);
		}

	//	Clean up

	pCtx->pLocalSymbols = pOldSymbols;
	pLocalSymbols->Discard(pCC);

	//	Done

	if (bReduceMax || bReduceMin)
		{
		pResult->Discard(pCC);
		if (iBestItem == -1)
			return pCC->CreateNil();

		if (bOriginal)
			return pSource->GetElement(iBestItem)->Reference();
		else
			return pCC->CreateInteger(iAccumulate);
		}
	else if (bReduceSum)
		{
		if (iCount == 0)
			return pCC->CreateNil();

		return pCC->CreateInteger(iAccumulate);
		}
	else if (bReduceAverage)
		{
		if (iCount == 0)
			return pCC->CreateNil();

		return pCC->CreateInteger(iAccumulate / iCount);
		}
	else
		{
		if (pResult->GetCount() > 0)
			return pResult;
		else
			{
			pResult->Discard(pCC);
			return pCC->CreateNil();
			}
		}
	}

ICCItem *fnMatch (CEvalContext *pCtx, ICCItem *pArgs, DWORD dwData)

//	fnMatch
//
//	Returns the first item in the list that passes the given expression
//
//	(match list var exp) -> list

	{
	int i;
	CCodeChain *pCC = pCtx->pCC;

	if (pArgs->GetElement(0)->IsNil())
		return pCC->CreateNil();

	//	Args

	ICCItem *pSource = pArgs->GetElement(0);
	ICCItem *pVar = pArgs->GetElement(1);
	ICCItem *pBody = pArgs->GetElement(2);

	//	Setup the locals. We start by creating a local symbol table

	ICCItem *pLocalSymbols = pCC->CreateSymbolTable();
	if (pLocalSymbols->IsError())
		return pLocalSymbols;

	//	Associate the enumaration variable

	ICCItem *pError = pLocalSymbols->AddEntry(pCC, pVar, pCC->CreateNil());
	if (pError->IsError())
		{
		pLocalSymbols->Discard(pCC);
		return pError;
		}

	pError->Discard(pCC);

	//	Setup the context

	if (pCtx->pLocalSymbols)
		pLocalSymbols->SetParent(pCtx->pLocalSymbols);
	else
		pLocalSymbols->SetParent(pCtx->pLexicalSymbols);
	ICCItem *pOldSymbols = pCtx->pLocalSymbols;
	pCtx->pLocalSymbols = pLocalSymbols;

	//	Get the offset of the variable so we don't have to
	//	search for it all the time

	int iVarOffset = pLocalSymbols->FindOffset(pCC, pVar);

	//	Loop over all elements of the list

	ICCItem *pResult = NULL;
	for (i = 0; i < pSource->GetCount(); i++)
		{
		ICCItem *pItem = pSource->GetElement(i);

		//	Set the element

		pLocalSymbols->AddByOffset(pCC, iVarOffset, pItem);

		//	Eval

		ICCItem *pSelect = pCC->Eval(pCtx, pBody);

		//	If error, return the error

		if (pSelect->IsError())
			{
			pResult = pSelect;
			break;
			}

		//	If the evaluation is not Nil, then this is match

		else if (!pSelect->IsNil())
			{
			pResult = pItem->Reference();
			pSelect->Discard(pCC);
			break;
			}

		//	Otherwise, keep looping

		else
			pSelect->Discard(pCC);
		}

	//	Clean up

	pCtx->pLocalSymbols = pOldSymbols;
	pLocalSymbols->Discard(pCC);

	//	Done

	if (pResult)
		return pResult;
	else
		return pCC->CreateNil();
	}

ICCItem *fnMathList (CEvalContext *pCtx, ICCItem *pArgs, DWORD dwData)

//	fnMathList
//
//	Simple integer functions
//
//	(add x1 x2 ... xn) -> z
//	(max x1 x2 ... xn) -> z
//	(min x1 x2 ... xn) -> z
//	(multiply x1 x2 .. .xn) -> z

	{
	int i;
	CCodeChain *pCC = pCtx->pCC;

	//	Get the list

	ICCItem *pList;
	if (pArgs->GetCount() == 1 && pArgs->GetElement(0)->IsList())
		{
		pList = pArgs->GetElement(0);

		if (pList->GetCount() < 1)
			return pCC->CreateInteger(0);
		}
	else
		pList = pArgs;

	//	Do the computation

	switch (dwData)
		{
		case FN_MATH_ADD:
			{
			int iResult = pList->GetElement(0)->GetIntegerValue();
			for (i = 1; i < pList->GetCount(); i++)
				iResult += pList->GetElement(i)->GetIntegerValue();
			return pCC->CreateInteger(iResult);
			}

		case FN_MATH_MULTIPLY:
			{
			int iResult = pList->GetElement(0)->GetIntegerValue();
			for (i = 1; i < pList->GetCount(); i++)
				iResult *= pList->GetElement(i)->GetIntegerValue();
			return pCC->CreateInteger(iResult);
			}

		case FN_MATH_MAX:
			{
			int iResult = pList->GetElement(0)->GetIntegerValue();
			for (i = 1; i < pList->GetCount(); i++)
				{
				int iVal = pList->GetElement(i)->GetIntegerValue();
				if (iVal > iResult)
					iResult = iVal;
				}
			return pCC->CreateInteger(iResult);
			}

		case FN_MATH_MIN:
			{
			int iResult = pList->GetElement(0)->GetIntegerValue();
			for (i = 1; i < pList->GetCount(); i++)
				{
				int iVal = pList->GetElement(i)->GetIntegerValue();
				if (iVal < iResult)
					iResult = iVal;
				}
			return pCC->CreateInteger(iResult);
			}

		default:
			ASSERT(false);
			return pCC->CreateNil();
		}
	}

ICCItem *fnMath (CEvalContext *pCtx, ICCItem *pArgs, DWORD dwData)

//	fnMath
//
//	Simple integer arithmetic
//
//	(modulo ['degrees] int1 int2)
//	(sqrt int1)

	{
	CCodeChain *pCC = pCtx->pCC;

	//	Compute

	switch (dwData)
		{
		case FN_MATH_ABSOLUTE:
			return pCC->CreateInteger(Absolute(pArgs->GetElement(0)->GetIntegerValue()));

		case FN_MATH_MODULUS:
			{
			int iArg = 0;

			bool bClock = false;
			if (pArgs->GetCount() > 0 && pArgs->GetElement(iArg)->IsIdentifier())
				{
				if (strEquals(pArgs->GetElement(iArg)->GetStringValue(), CONSTLIT("degrees")))
					bClock = true;
				else
					return pCC->CreateError(CONSTLIT("Unknown option"), pArgs->GetElement(iArg));

				iArg++;
				}

			if (pArgs->GetCount() < (iArg + 2))
				return pCC->CreateError(CONSTLIT("Insufficient arguments"), pArgs);

			int iOp1 = pArgs->GetElement(iArg++)->GetIntegerValue();
			int iOp2 = pArgs->GetElement(iArg++)->GetIntegerValue();

			if (iOp2 == 0)
				return pCC->CreateError(CONSTLIT("Division by zero"), pArgs);

			if (bClock)
				{
				int iResult = iOp1 % iOp2;

				if (iResult < 0)
					return pCC->CreateInteger(iOp2 + iResult);
				else
					return pCC->CreateInteger(iResult);
				}
			else
				return pCC->CreateInteger(iOp1 % iOp2);
			}

		case FN_MATH_SQRT:
			{
			int iValue = pArgs->GetElement(0)->GetIntegerValue();
			if (iValue >= 0)
				return pCC->CreateInteger(mathSqrt(iValue));
			else
				return pCC->CreateError(CONSTLIT("Imaginary number"), pArgs->GetElement(0));
			}

		default:
			ASSERT(false);
			return pCC->CreateNil();
		}
	}

double GetFractionArg (ICCItem *pArg, double *retrDenom)
	{
	if (pArg->IsList() && pArg->GetCount() >= 2)
		{
		double rNum = pArg->GetElement(0)->GetIntegerValue();
		double rDenom = pArg->GetElement(1)->GetIntegerValue();
		if (rDenom == 0.0)
			{
			rNum = 0.0;
			rDenom = 1.0;
			}
		else if (rDenom < 0.0)
			{
			rNum = -rNum;
			rDenom = -rDenom;
			}

		if (retrDenom)
			*retrDenom = rDenom;

		return rNum / rDenom;
		}
	else
		{
		if (retrDenom)
			*retrDenom = 1.0;

		return (double)pArg->GetIntegerValue();
		}
	}

ICCItem *fnMathFractions (CEvalContext *pCtx, ICCItem *pArgs, DWORD dwData)

//	fnMathFractions
//
//	Parameters can be fractions

	{
	CCodeChain *pCC = pCtx->pCC;

	//	Compute

	switch (dwData)
		{
		case FN_MATH_POWER:
			{
			double rResultDenom;
			double rX = GetFractionArg(pArgs->GetElement(0), &rResultDenom);
			double rY = GetFractionArg(pArgs->GetElement(1));
			double rResult = pow(rX, rY);

			if (rResultDenom == 1.0)
				return pCC->CreateInteger((int)(rResult + 0.5));
			else
				{
				ICCItem *pResult = pCC->CreateLinkedList();
				CCLinkedList *pList = (CCLinkedList *)pResult;

				pList->AppendIntegerValue(pCC, (int)((rResult * rResultDenom) + 0.5));
				pList->AppendIntegerValue(pCC, (int)(rResultDenom + 0.5));

				return pResult;
				}
			}

		default:
			ASSERT(false);
			return NULL;
		}
	}

ICCItem *fnMathOld (CEvalContext *pCtx, ICCItem *pArguments, DWORD dwData)

//	fnMathOld
//
//	Simple integer arithmetic
//
//	(divide int1 int2)
//	(subtract int1 int2)

	{
	CCodeChain *pCC = pCtx->pCC;
	ICCItem *pArgs;
	int iResult, iOp1, iOp2;

	//	Evaluate the arguments and validate them

	pArgs = pCC->EvaluateArgs(pCtx, pArguments, CONSTLIT("vv"));
	if (pArgs->IsError())
		return pArgs;

	//	Convert to integers

	iOp1 = pArgs->GetElement(0)->GetIntegerValue();
	iOp2 = pArgs->GetElement(1)->GetIntegerValue();

	//	Done with arguments

	pArgs->Discard(pCC);

	//	Compute

	switch (dwData)
		{
		case FN_MATH_SUBTRACT:
			iResult = iOp1 - iOp2;
			break;

		case FN_MATH_DIVIDE:
			if (iOp2 != 0)
				iResult = iOp1 / iOp2;
			else
				return pCC->CreateError(CONSTLIT("Division by zero"), pArguments);
			break;

		default:
			ASSERT(FALSE);
		}

	//	Done

	return pCC->CreateInteger(iResult);
	}

ICCItem *fnRandom (CEvalContext *pCtx, ICCItem *pArgs, DWORD dwData)

//	fnRandom
//
//	(random from to) -> number
//	(random list) -> random item in list

	{
	CCodeChain *pCC = pCtx->pCC;

	switch (dwData)
		{
		case FN_RANDOM:
			{
			//	Do stuff based on parameters

			if (pArgs->GetCount() == 2)
				{
				int iOp1 = pArgs->GetElement(0)->GetIntegerValue();
				int iOp2 = pArgs->GetElement(1)->GetIntegerValue();
				return pCC->CreateInteger(mathRandom(iOp1, iOp2));
				}
			else if (pArgs->GetCount() == 1)
				{
				ICCItem *pList = pArgs->GetElement(0);

				if (pList->IsNil() || pList->GetCount() == 0)
					return pCC->CreateNil();

				return pList->GetElement(mathRandom(0, pList->GetCount()-1))->Reference();
				}
			else
				return pCC->CreateNil();
			}

		case FN_RANDOM_GAUSSIAN:
			{
			//	Get the parameters

			double rLow = pArgs->GetElement(0)->GetIntegerValue();
			double rMid = pArgs->GetElement(1)->GetIntegerValue();
			double rHigh = pArgs->GetElement(2)->GetIntegerValue();
			if (rLow >= rMid || rLow >= rHigh || rMid >= rHigh)
				return pCC->CreateNil();

			//	Compute some ranges

			double rHighRange = rHigh - rMid;
			double rLowRange = rMid - rLow;

			//	Generate a gaussian, but clip out after 3 standard deviations

			double rMaxStdDev = 3.0;
			double rValue;
			do
				{
				rValue = mathRandomGaussian();
				}
			while (rValue > rMaxStdDev || rValue < -rMaxStdDev);

			rValue = rValue / rMaxStdDev;

			//	Scale to proper value

			if (rValue >= 0.0)
				return pCC->CreateInteger((int)floor(rMid + rValue * rHighRange + 0.5));
			else
				return pCC->CreateInteger((int)floor(rMid + rValue * rLowRange + 0.5));
			}

		default:
			ASSERT(false);
			return NULL;
		}
	}

ICCItem *fnRandomTable (CEvalContext *pCtx, ICCItem *pArguments, DWORD dwData)

//	fnRandomTable
//
//	(randomTable chance1 exp1 chance2 exp2 ... chancen expn) -> exp

	{
	int i;
	CCodeChain *pCC = pCtx->pCC;

	//	Generate a table of all chances

	int iEntryCount = pArguments->GetCount() / 2;
	if (iEntryCount == 0)
		return pCC->CreateNil();

	int *pTable = new int [iEntryCount];
	int iTotal = 0;
	for (i = 0; i < iEntryCount; i++)
		{
		//	Evaluate the expression

		ICCItem *pResult = pCC->Eval(pCtx, pArguments->GetElement(i * 2));
		if (pResult->IsError())
			{
			delete [] pTable;
			return pResult;
			}

		pTable[i] = pResult->GetIntegerValue();
		iTotal += pTable[i];

		pResult->Discard(pCC);
		}

	//	Now roll

	if (iTotal <= 0)
		{
		delete [] pTable;
		return pCC->CreateNil();
		}

	int iRoll = mathRandom(1, iTotal);

	//	Figure out which expression to evaluate

	for (i = 0; i < iEntryCount; i++)
		{
		if (iRoll > pTable[i])
			{
			iRoll -= pTable[i];
			continue;
			}

		//	Execute and return expression

		delete [] pTable;
		return pCC->Eval(pCtx, pArguments->GetElement(i * 2 + 1));
		}

	//	We should never get here

	delete [] pTable;
	return pCC->CreateNil();
	}

ICCItem *fnRegEx (CEvalContext *pCtx, ICCItem *pArgs, DWORD dwData)

//	fnRegEx
//
//	(regex source pattern) -> result

	{
	int i;
	CCodeChain *pCC = pCtx->pCC;

	//	Get the args

	CString sString = pArgs->GetElement(0)->GetStringValue();
	CString sRegEx = pArgs->GetElement(1)->GetStringValue();

	//	Get the flags

	bool bIncludeSubExp = false;
	bool bIncludeOffsets = false;
	if (pArgs->GetCount() > 2)
		{
		for (i = 0; i < pArgs->GetElement(2)->GetCount(); i++)
			{
			if (strEquals(pArgs->GetElement(2)->GetElement(i)->GetStringValue(), CONSTLIT("offset")))
				bIncludeOffsets = true;
			else if (strEquals(pArgs->GetElement(2)->GetElement(i)->GetStringValue(), CONSTLIT("subex")))
				bIncludeSubExp = true;
			}
		}

	//	RegEx

	TArray<SRegExMatch> Result;
	if (!strRegEx(sString.GetASCIIZPointer(),
			sRegEx,
			&Result))
		return pCC->CreateNil();

	//	If we have no results, nil

	if (Result.GetCount() == 0)
		return pCC->CreateNil();

	//	If we have a single result, return it as a string

	else if (Result.GetCount() == 1 || !bIncludeSubExp)
		return pCC->CreateString(Result[0].sMatch);

	//	If we have multiple results, return a list

	else
		{
		int i;

		//	Create a new list to store the result in

		ICCItem *pResult = pCC->CreateLinkedList();
		if (pResult->IsError())
			return pResult;

		CCLinkedList *pList = (CCLinkedList *)pResult;

		//	Loop over all arguments and add to result list

		for (i = 0; i < Result.GetCount(); i++)
			pList->AppendStringValue(pCC, Result[i].sMatch);

		return pResult;
		}
	}

ICCItem *fnSeededRandom (CEvalContext *pCtx, ICCItem *pArgs, DWORD dwData)

//	fnSeededRandom
//
//	(seededRandom seed from to) -> number
//	(seededRandom seed list) -> random item in list

	{
	CCodeChain *pCC = pCtx->pCC;
	ICCItem *pResult;

	int iSeed = pArgs->GetElement(0)->GetIntegerValue();

	//	Do stuff based on parameters

	if (pArgs->GetCount() == 3)
		{
		int iOp1 = pArgs->GetElement(1)->GetIntegerValue();
		int iOp2 = pArgs->GetElement(2)->GetIntegerValue();

		pResult = pCC->CreateInteger(mathSeededRandom(iSeed, iOp1, iOp2));
		}
	else if (pArgs->GetCount() == 2)
		{
		ICCItem *pList = pArgs->GetElement(1);

		if (pList->IsNil())
			pResult = pCC->CreateNil();
		else if (pList->GetCount() == 0)
			pResult = pCC->CreateNil();
		else
			pResult = pList->GetElement(mathSeededRandom(iSeed, 0, pList->GetCount()-1))->Reference();
		}
	else
		pResult = pCC->CreateNil();

	//	Done

	return pResult;
	}

ICCItem *fnSet (CEvalContext *pCtx, ICCItem *pArguments, DWORD dwData)

//	fnSet
//
//	Bind an identifier to some value
//
//	(set var exp)
//	(setq var exp)

	{
	CCodeChain *pCC = pCtx->pCC;
	ICCItem *pArgs;
	ICCItem *pVar;
	ICCItem *pValue;

	//	Evaluate the arguments and validate them

	if (dwData == FN_SET_SET)
		pArgs = pCC->EvaluateArgs(pCtx, pArguments, CONSTLIT("sv"));
	else
		pArgs = pCC->EvaluateArgs(pCtx, pArguments, CONSTLIT("qv"));
	if (pArgs->IsError())
		return pArgs;

	//	First is the variable; next is the value

	pVar = pArgs->GetElement(0);
	pValue = pArgs->GetElement(1);

	//	Figure out which symbol table to start with

	ICCItem *pError;
	if (HelperSetq(pCtx, pVar, pValue, &pError) != NOERROR)
		{
		pArgs->Discard(pCC);
		return pError;
		}

	//	Keep a reference to the value, so we can return it

	pValue->Reference();

	//	Done with these

	pArgs->Discard(pCC);

	//	Done

	return pValue;
	}

ICCItem *fnShuffle (CEvalContext *pCtx, ICCItem *pArgs, DWORD dwData)

//	fnShuffle
//
//	Shuffles a list randomly

	{
	int i;
	CCodeChain *pCC = pCtx->pCC;

	if (pArgs->GetElement(0)->IsNil())
		return pCC->CreateNil();

	//	Create a destination list

	ICCItem *pResult = pCC->CreateLinkedList();
	if (pResult->IsError())
		return pResult;

	CCLinkedList *pList = (CCLinkedList *)pResult;

	//	Copy the list

	ICCItem *pSource = pArgs->GetElement(0);
	for (i = 0; i < pSource->GetCount(); i++)
		pList->Append(pCC, pSource->GetElement(i), NULL);

	//	Shuffle the new list

	pList->Shuffle(pCC);

	//	Done

	return pResult;
	}

ICCItem *fnSort (CEvalContext *pCtx, ICCItem *pArgs, DWORD dwData)

//	fnSort
//
//	(sort list ['ascending|'descending] [keyIndex]) -> sorted list

	{
	int i;
	CCodeChain *pCC = pCtx->pCC;

	switch (dwData)
		{
		case FN_SORT:
			{
			//	Get the list to sort

			ICCItem *pSource = pArgs->GetElement(0);

			//	Get optional sort order

			int iOptionalArg = 1;
			int iOrder = AscendingSort;
			if (pArgs->GetCount() > iOptionalArg && pArgs->GetElement(iOptionalArg)->IsIdentifier())
				{
				if (strEquals(pArgs->GetElement(iOptionalArg)->GetStringValue(), CONSTLIT("descending")))
					iOrder = DescendingSort;

				iOptionalArg++;
				}

			//	Get optional index

			int iIndex = (pArgs->GetCount() > iOptionalArg ? pArgs->GetElement(iOptionalArg)->GetIntegerValue() : -1);

			//	Handle edge cases

			if (pSource->GetCount() < 2)
				return pSource->Reference();

			//	Create a new list

			ICCItem *pResult = pCC->CreateLinkedList();
			if (pResult->IsError())
				return pResult;

			CCLinkedList *pList = (CCLinkedList *)pResult;

			//	Copy the list

			for (i = 0; i < pSource->GetCount(); i++)
				pList->Append(pCC, pSource->GetElement(i), NULL);

			//	Sort the list

			pList->Sort(pCC, iOrder, iIndex);

			//	Done

			return pResult;
			}

		default:
			ASSERT(false);
			return pCC->CreateNil();
		}
	}

ICCItem *fnSpecial (CEvalContext *pCtx, ICCItem *pArgs, DWORD dwData)

//	fnSpecial
//
//	(error msg) -> error
//	(quote exp) -> exp

	{
	CCodeChain *pCC = pCtx->pCC;

	switch (dwData)
		{
		case FN_ERROR:
			return pCC->CreateError(pArgs->GetElement(0)->GetStringValue(), NULL);

		case FN_QUOTE:
			return pArgs->GetElement(0)->Reference();

		default:
			ASSERT(false);
			return pCC->CreateNil();
		}
	}

ICCItem *fnSplit (CEvalContext *pCtx, ICCItem *pArgs, DWORD dwData)

//	fnSplit
//
//	(split string [characters])

	{
	CCodeChain *pCC = pCtx->pCC;
	CString sString = pArgs->GetElement(0)->GetStringValue();

	if (sString.IsBlank())
		return pCC->CreateNil();

	//	Get the list of characters, if available.

	CString sDelimeters;
	if (pArgs->GetCount() >= 2)
		sDelimeters = pArgs->GetElement(1)->GetStringValue();
	
	//	By default, we do whitespace separation

	if (sDelimeters.IsBlank())
		sDelimeters = CONSTLIT(" \t\r\n");

	//	Create a new list

	ICCItem *pResult = pCC->CreateLinkedList();
	if (pResult->IsError())
		return pResult;

	CCLinkedList *pList = (CCLinkedList *)pResult;

	//	Parse the string

	char *pPos = sString.GetASCIIZPointer();
	char *pStart = pPos;
	bool bInWord = false;

	while (*pPos != '\0')
		{
		//	Is this a delimeter?

		bool bIsDelimeter = false;
		char *pDelim = sDelimeters.GetASCIIZPointer();
		while (*pDelim != '\0')
			{
			if (*pPos == *pDelim)
				{
				bIsDelimeter = true;
				break;
				}
			pDelim++;
			}

		//	Handle it based on our current state.

		if (bInWord)
			{
			if (bIsDelimeter)
				{
				pList->AppendStringValue(pCC, CString(pStart, (int)(pPos - pStart)));
				bInWord = false;
				}
			}
		else
			{
			if (!bIsDelimeter)
				{
				pStart = pPos;
				bInWord = true;
				}
			}

		//	Next
		
		pPos++;
		}

	//	Last word

	if (bInWord)
		pList->AppendStringValue(pCC, CString(pStart, (int)(pPos - pStart)));

	//	Done

	if (pList->GetCount() == 0)
		{
		pList->Discard(pCC);
		return pCC->CreateNil();
		}
	else
		return pList;
	}

ICCItem *fnStrCapitalize (CEvalContext *pCtx, ICCItem *pArguments, DWORD dwData)

//	fnStrCapitalize
//
//	Capitalizes the string
//
//	(strCapitalize string) -> string

	{
	CCodeChain *pCC = pCtx->pCC;
	ICCItem *pArgs;

	//	Evaluate the arguments and validate them

	pArgs = pCC->EvaluateArgs(pCtx, pArguments, CONSTLIT("s"));
	if (pArgs->IsError())
		return pArgs;

	CString sString = pArgs->GetElement(0)->GetStringValue();
	pArgs->Discard(pCC);

	//	Done

	sString.Capitalize(CString::capFirstLetter);
	return pCC->CreateString(sString);
	}

ICCItem *fnStrFind (CEvalContext *pCtx, ICCItem *pArgs, DWORD dwData)

//	fnStrFind
//
//	Finds a string in some text

	{
	CCodeChain *pCC = pCtx->pCC;
	int iPos = strFind(pArgs->GetElement(0)->GetStringValue(), pArgs->GetElement(1)->GetStringValue());

	if (iPos == -1)
		return pCC->CreateNil();
	else
		return pCC->CreateInteger(iPos);
	}

ICCItem *fnSubset (CEvalContext *pCtx, ICCItem *pArgs, DWORD dwData)

//	fnSubset
//
//	(subset list pos [count]) -> list
//
//	Returns a sub list

	{
	int i;
	CCodeChain *pCC = pCtx->pCC;

	//	Get start and end

	ICCItem *pSource = pArgs->GetElement(0);
	int iSourceCount = (pSource->IsList() ? pSource->GetCount() : pSource->GetStringValue().GetLength());
	int iStart = Max(0, Min(pArgs->GetElement(1)->GetIntegerValue(), iSourceCount));
	int iCount;
	if (pArgs->GetCount() > 2 && !pArgs->GetElement(2)->IsNil())
		iCount = Max(0, Min(pArgs->GetElement(2)->GetIntegerValue(), iSourceCount - iStart));
	else
		iCount = iSourceCount - iStart;

	if (pSource->IsNil() || iCount == 0)
		return pCC->CreateNil();

	//	If the source is a list then we subset the list

	if (pSource->IsList())
		{
		//	Create a new list to store the result in

		ICCItem *pResult = pCC->CreateLinkedList();
		if (pResult->IsError())
			return pResult;

		CCLinkedList *pList = (CCLinkedList *)pResult;

		//	Loop and add to result list

		for (i = iStart; i < iStart + iCount; i++)
			pList->Append(pCC, pSource->GetElement(i), NULL);

		//	Done

		return pResult;
		}

	//	Otherwise, we subset a string

	else
		{
		return pCC->CreateString(strSubString(pSource->GetStringValue(), iStart, iCount));
		}
	}

ICCItem *fnSubst (CEvalContext *pCtx, ICCItem *pArgs, DWORD dwData)

//	fnSubst
//
//	Substitutes string parameters
//
//	(subst string arg1 arg2 ... argn)
//	(subst string struct)

	{
	CCodeChain *pCC = pCtx->pCC;

	CString sPattern = pArgs->GetElement(0)->GetStringValue();
	char *pPos = sPattern.GetASCIIZPointer();

	ICCItem *pStruct = ((pArgs->GetCount() > 1) ? pArgs->GetElement(1) : NULL);

	//	Output buffer

	CMemoryWriteStream Stream(0);
	if (Stream.Create() != NOERROR)
		return pCC->CreateError(CONSTLIT("Out of memory"));

	//	Do the substitution

	while (*pPos != '\0')
		{
		if (*pPos == '%')
			{
			pPos++;

			//	If this is another % then it is an escape

			if (*pPos == '%')
				Stream.Write(pPos++, 1);

			//	See if we have an integer. If we do, then we look for the nth
			//	argument.

			else if (strIsDigit(pPos))
				{
				int iArg = strParseInt(pPos, 0, &pPos);
				if (iArg > 0 && iArg < pArgs->GetCount())
					{
					CString sParam = pArgs->GetElement(iArg)->GetStringValue();
					Stream.Write(sParam.GetASCIIZPointer(), sParam.GetLength());
					}

				if (*pPos == '%')
					pPos++;
				}

			//	Else if this is a string, then we expect this to be a key that
			//	we use to look up a value in a struct

			else
				{
				char *pStart = pPos;
				while (*pPos != '%' && *pPos != '\0')
					pPos++;

				CString sKey(pStart, (int)(pPos - pStart));

				if (!sKey.IsBlank() && pStruct)
					{
					ICCItem *pKey = pCC->CreateString(sKey);

					BOOL bFound;
					ICCItem *pResult = pStruct->LookupEx(pCC, pKey, &bFound);
					pKey->Discard(pCC);
					if (bFound)
						{
						CString sParam = pResult->GetStringValue();
						Stream.Write(sParam.GetASCIIZPointer(), sParam.GetLength());
						}
					else
						{
						//	If not found, write out the original parameter (with
						//	% delimiters) so that other functions can handle them.

						Stream.Write("%", 1);
						Stream.Write(sKey.GetASCIIZPointer(), sKey.GetLength());
						Stream.Write("%", 1);
						}

					pResult->Discard(pCC);
					}

				if (*pPos == '%')
					pPos++;
				}
			}
		else
			Stream.Write(pPos++, 1);
		}

	//	Done

	return pCC->CreateString(CString(Stream.GetPointer(), Stream.GetLength()));
	}

ICCItem *fnSwitch (CEvalContext *pCtx, ICCItem *pArguments, DWORD dwData)

//	fnSwitch
//
//	Switch control function
//
//	(switch exp1 case1 exp2 case2 ... default)

	{
	CCodeChain *pCC = pCtx->pCC;
	ICCItem *pTest;
	ICCItem *pThen;
	ICCItem *pResult;

	int iArgPos = 0;
	while (iArgPos+1 < pArguments->GetCount())
		{
		//	Get the arguments

		pTest = pArguments->GetElement(iArgPos);
		pThen = pArguments->GetElement(iArgPos+1);

		//	Evaluate the expression

		pResult = pCC->Eval(pCtx, pTest);
		if (pResult->IsError())
			return pResult;

		//	If the result is not Nil, then evaluate the Then expression

		if (!pResult->IsNil())
			{
			pResult->Discard(pCC);
			return pCC->Eval(pCtx, pThen);
			}

		//	Otherwise, continue with the loop

		pResult->Discard(pCC);
		iArgPos += 2;
		}

	//	Do we have a default case?

	if (iArgPos < pArguments->GetCount())
		{
		ICCItem *pElse = pArguments->GetElement(iArgPos);
		return pCC->Eval(pCtx, pElse);
		}

	//	Otherwise, we return Nil

	return pCC->CreateNil();
	}

ICCItem *fnSymCreate (CEvalContext *pCtx, ICCItem *pArguments, DWORD dwData)

//	fnSymCreate
//
//	Creates a new symbol table
//
//	(symCreate) -> symtable

	{
	CCodeChain *pCC = pCtx->pCC;

	return pCC->CreateSymbolTable();
	}

ICCItem *fnSymTable (CEvalContext *pCtx, ICCItem *pArguments, DWORD dwData)

//	fnSymTable
//
//	Various symbol table manipulations
//
//	(symAddEntry symTable symbol entry) -> entry
//	(symDeleteEntry symTable symbol) -> True
//	(symLookup symTable symbol) -> entry

	{
	CCodeChain *pCC = pCtx->pCC;
	ICCItem *pArgs;
	ICCItem *pSymTable;
	ICCItem *pSymbol;
	ICCItem *pResult;

	//	Evaluate the arguments and validate them

	if (dwData == FN_SYMTABLE_ADDENTRY)
		pArgs = pCC->EvaluateArgs(pCtx, pArguments, CONSTLIT("ysv"));
	else if (dwData == FN_SYMTABLE_LIST)
		pArgs = pCC->EvaluateArgs(pCtx, pArguments, CONSTLIT("y"));
	else
		pArgs = pCC->EvaluateArgs(pCtx, pArguments, CONSTLIT("ys"));

	if (pArgs->IsError())
		return pArgs;

	//	Get the args

	pSymTable = pArgs->Head(pCC);

	//	Do the right thing

	switch (dwData)
		{
		case FN_SYMTABLE_ADDENTRY:
			{
			ICCItem *pEntry;

			pSymbol = pArgs->GetElement(1);
			pEntry = pArgs->GetElement(2);
			pResult = pSymTable->AddEntry(pCC, pSymbol, pEntry);

			//	If we succeeded, return the entry

			if (!pResult->IsError())
				{
				pResult->Discard(pCC);
				pResult = pEntry->Reference();
				}

			break;
			}

		case FN_SYMTABLE_DELETEENTRY:
			{
			pResult = pCC->CreateNil();
			break;
			}

		case FN_SYMTABLE_LIST:
			{
			pResult = pSymTable->ListSymbols(pCC);
			break;
			}

		case FN_SYMTABLE_LOOKUP:
			{
			pSymbol = pArgs->GetElement(1);
			pResult = pSymTable->Lookup(pCC, pSymbol);

			//	Return Nil if we can't find it.

			if (pResult->IsError() && pResult->IsInteger() && pResult->GetIntegerValue() == CCRESULT_NOTFOUND)
				{
				pResult->Discard(pCC);
				pResult = pCC->CreateNil();
				}

			break;
			}

		default:
			ASSERT(FALSE);
			return NULL;
		}

	//	Done

	pArgs->Discard(pCC);
	return pResult;
	}

ICCItem *fnSysInfo (CEvalContext *pCtx, ICCItem *pArguments, DWORD dwData)

//	fnSysInfo
//
//	Returns system information
//
//	(globals)

	{
	CCodeChain *pCC = pCtx->pCC;

	switch (dwData)
		{
		case FN_SYSINFO_GLOBALS:
			return pCC->ListGlobals();

		case FN_SYSINFO_POOLUSAGE:
			return pCC->PoolUsage();

		case FN_SYSINFO_TICKS:
			return pCC->CreateInteger((int)GetTickCount());

		default:
			ASSERT(FALSE);
			return NULL;
		}
	}

ICCItem *fnVecCreate (CEvalContext *pCtx, ICCItem *pArguments, DWORD dwData)

//	fnVecCreate
//
//	Creates a new vector of a given size
//
//	(vecVector size) -> vector
//
//	All elements of the vector are initialized to 0

	{
	CCodeChain *pCC = pCtx->pCC;
	ICCItem *pArgs;
	ICCItem *pVector;

	//	Evaluate the argument

	pArgs = pCC->EvaluateArgs(pCtx, pArguments, CONSTLIT("v"));
	if (pArgs->IsError())
		return pArgs;

	//	Create the table

	pVector = pCC->CreateVector(pArgs->Head(pCC)->GetIntegerValue());

	//	Done

	pArgs->Discard(pCC);
	return pVector;
	}

ICCItem *fnVector (CEvalContext *pCtx, ICCItem *pArguments, DWORD dwData)

//	fnVector
//
//	Vector functions
//
//	(vecSetElement vector index element)

	{
	CCodeChain *pCC = pCtx->pCC;
	ICCItem *pArgs;
	CCVector *pVector;
	BOOL bOk;

	//	Evaluate the arguments

	pArgs = pCC->EvaluateArgs(pCtx, pArguments, CONSTLIT("vvv"));
	if (pArgs->IsError())
		return pArgs;

	//	Get the vector

	pVector = dynamic_cast<CCVector *>(pArgs->GetElement(0));
	if (pVector == NULL)
		{
		ICCItem *pError = pCC->CreateError(CONSTLIT("Vector expected"), pArgs->GetElement(0));
		pArgs->Discard(pCC);
		return pError;
		}

	//	Set the element

	bOk = pVector->SetElement(pArgs->GetElement(1)->GetIntegerValue(),
			pArgs->GetElement(2)->GetIntegerValue());

	if (!bOk)
		{
		pArgs->Discard(pCC);
		return pCC->CreateError(CONSTLIT("Unable to set vector element"), NULL);
		}

	//	Done

	pArgs->Discard(pCC);
	return pCC->CreateTrue();
	}

//	Helper Functions -----------------------------------------------------------

int HelperCompareItems (ICCItem *pFirst, ICCItem *pSecond, bool bCoerce)

//	HelperCompareItems
//
//	Compares two items and returns:
//
//		1	if pFirst > pSecond
//		0	if pFirst = pSecond
//		-1	if pFirst < pSecond
//		-2	if pFirst is not the same type as pSecond

	{
	//	Compare this with the first expression

	if (pFirst->GetValueType() == pSecond->GetValueType())
		{
		switch (pFirst->GetValueType())
			{
			case ICCItem::Boolean:
				{
				if (pFirst->IsNil() == pSecond->IsNil())
					return 0;
				else if (pFirst->IsNil())
					return -1;
				else
					return 1;
				}

			case ICCItem::Integer:
				{
				if (pFirst->GetIntegerValue() == pSecond->GetIntegerValue())
					return 0;
				else if (pFirst->GetIntegerValue() > pSecond->GetIntegerValue())
					return 1;
				else
					return -1;
				}

			case ICCItem::String:
				return strCompare(pFirst->GetStringValue(), pSecond->GetStringValue());

			case ICCItem::List:
				{
				if (pFirst->GetCount() == pSecond->GetCount())
					{
					int i;

					for (i = 0; i < pFirst->GetCount(); i++)
						{
						ICCItem *pItem1 = pFirst->GetElement(i);
						ICCItem *pItem2 = pSecond->GetElement(i);
						int iCompare;

						iCompare = HelperCompareItems(pItem1, pItem2);
						if (iCompare != 0)
							return iCompare;
						}

					return 0;
					}
				else if (pFirst->GetCount() > pSecond->GetCount())
					return 1;
				else
					return -1;
				break;
				}

			default:
				return -2;
			}
		}
	else if (bCoerce)
		{
		if (pFirst->IsNil())
			{
			switch (pSecond->GetValueType())
				{
				case ICCItem::Integer:
					{
					if (0 == pSecond->GetIntegerValue())
						return 0;
					else if (0 > pSecond->GetIntegerValue())
						return 1;
					else
						return -1;
					}

				case ICCItem::String:
					return -1;

				case ICCItem::List:
					return (pSecond->GetCount() == 0 ? 0 : -1);

				default:
					return -2;
				}
			}
		else if (pSecond->IsNil())
			{
			int iResult = HelperCompareItems(pSecond, pFirst, bCoerce);
			if (iResult == -1)
				return 1;
			else if (iResult == 1)
				return -1;
			else
				return iResult;
			}
		else
			return -2;
		}
	else
		return -2;
	}

int HelperCompareItemsLists (ICCItem *pFirst, ICCItem *pSecond, int iKeyIndex, bool bCoerce)
	{
	if (pFirst->GetCount() > iKeyIndex && pSecond->GetCount() > iKeyIndex)
		return HelperCompareItems(pFirst->GetElement(iKeyIndex), pSecond->GetElement(iKeyIndex), bCoerce);
	else
		{
		CCNil TempNil;

		if (pFirst->GetCount() > iKeyIndex)
			return HelperCompareItems(pFirst->GetElement(iKeyIndex), &TempNil, bCoerce);
		else if (pSecond->GetCount() > iKeyIndex)
			return HelperCompareItems(&TempNil, pSecond->GetElement(iKeyIndex), bCoerce);
		else
			return 0;
		}
	}

ALERROR HelperSetq (CEvalContext *pCtx, ICCItem *pVar, ICCItem *pValue, ICCItem **retpError)
	{
	CCodeChain *pCC = pCtx->pCC;
	ICCItem *pSymTable;

	ASSERT(pVar->IsIdentifier());

	//	Figure out which symbol table to start with

	if (pCtx->pLocalSymbols)
		pSymTable = pCtx->pLocalSymbols;
	else
		pSymTable = pCtx->pLexicalSymbols;

	//	If this variable has already been bound, then use a short-cut

	int iFrame, iOffset;
	if (pVar->GetBinding(&iFrame, &iOffset))
		{
		while (iFrame > 0)
			{
			pSymTable = pSymTable->GetParent();
			iFrame--;
			}

		pSymTable->AddByOffset(pCC, iOffset, pValue);
		}
	else
		{
		*retpError = pSymTable->AddEntry(pCC, pVar, pValue);

		//	Check for error

		if ((*retpError)->IsError())
			return ERR_FAIL;

		(*retpError)->Discard(pCC);
		}

	return NOERROR;
	}
