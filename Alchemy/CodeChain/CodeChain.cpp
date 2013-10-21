//	CCodeChain.cpp
//
//	Implementation of CCodeChain object

#include <windows.h>
#include "Kernel.h"
#include "KernelObjID.h"
#include "CodeChain.h"

#include "Functions.h"
#include "DefPrimitives.h"

#define INTEGER_POOL								0
#define STRING_POOL									1
#define LIST_POOL									2
#define PRIMITIVE_POOL								3
#define SYMBOLTABLE_POOL							4
#define LAMBDA_POOL									5
#define ATOMTABLE_POOL								6

#define POOL_COUNT									7

static CObjectClass<CCodeChain>g_Class(OBJID_CCODECHAIN, NULL);

CCodeChain::CCodeChain (void) : CObject(&g_Class),
		m_pGlobalSymbols(NULL)

//	CCodeChain constructor

	{
	}

CCodeChain::~CCodeChain (void)

//	CCodeChain destructor

	{
	CleanUp();
	}

ICCItem *CCodeChain::Apply (ICCItem *pFunc, ICCItem *pArgs, LPVOID pExternalCtx)

//	Apply
//
//	Runs the given function with the given arguments

	{
	ICCItem *pResult;
	CEvalContext EvalCtx;

	//	Set up the context

	EvalCtx.pCC = this;
	EvalCtx.pLexicalSymbols = m_pGlobalSymbols;
	EvalCtx.pLocalSymbols = NULL;
	EvalCtx.pExternalCtx = pExternalCtx;

	//	Evalute the actual code

	pArgs->SetQuoted();

	if (pFunc->IsFunction())
		pResult = pFunc->Execute(&EvalCtx, pArgs);
	else
		pResult = pFunc->Reference();

	return pResult;
	}

ALERROR CCodeChain::Boot (void)

//	Boot
//
//	Initializes the object. Clean up is done in the destructor

	{
	ALERROR error;
	int i;
	ICCItem *pItem;

	//	Initialize memory error

	m_sMemoryError.SetError();
	m_sMemoryError.SetValue(LITERAL("Out of memory"));

	//	Initialize Nil

	pItem = new CCNil;
	if (pItem == NULL)
		return ERR_FAIL;

	m_pNil = pItem->Reference();

	//	Initialize True

	pItem = new CCTrue;
	if (pItem == NULL)
		return ERR_FAIL;

	m_pTrue = pItem->Reference();

	//	Initialize global symbol table

	pItem = CreateSymbolTable();
	if (pItem->IsError())
		return ERR_FAIL;

	m_pGlobalSymbols = pItem;

	//	Register the built-in primitives

	for (i = 0; i < DEFPRIMITIVES_COUNT; i++)
		if (error = RegisterPrimitive(&g_DefPrimitives[i]))
			return error;

	return NOERROR;
	}

void CCodeChain::CleanUp (void)

//	CleanUp
//
//	The oppposite of Boot

	{
	if (m_pGlobalSymbols)
		{
		m_pGlobalSymbols->Discard(this);
		m_pGlobalSymbols = NULL;
		}

	//	Free strings, because if CCodeChain is global, its destructor
	//	might get called after strings have terminated.

	m_sMemoryError.SetValue(NULL_STR);
	}

ICCItem *CCodeChain::CreateAtomTable (void)

//	CreateAtomTable
//
//	Creates an item

	{
	ICCItem *pItem;

	pItem = m_AtomTablePool.CreateItem(this);
	if (pItem->IsError())
		return pItem;

	pItem->Reset();
	return pItem->Reference();
	}

ICCItem *CCodeChain::CreateBool (bool bValue)

//	CreateBool
//
//	Creates either True or Nil

	{
	if (bValue)
		return CreateTrue();
	else
		return CreateNil();
	}

ICCItem *CCodeChain::CreateError (const CString &sError, ICCItem *pData)

//	CreateError
//
//	Creates an item
//
//	sError: Error messages
//	pData: Item that caused error.

	{
	ICCItem *pError;
	CString sArg;
	CString sErrorLine;

	//	Convert the argument to a string

	if (pData)
		{
		sArg = pData->Print(this);
		sErrorLine = strPatternSubst(LITERAL("%s [%s]"), sError, sArg);
		}
	else
		sErrorLine = sError;

	//	Create the error

	pError = CreateString(sErrorLine);
	pError->SetError();
	return pError;
	}

ICCItem *CCodeChain::CreateErrorCode (int iErrorCode)

//	CreateError
//
//	Creates an item
//
//	iErrorCode: Error code (CCRESULT_???)

	{
	ICCItem *pError;

	pError = CreateInteger(iErrorCode);
	pError->SetError();
	return pError;
	}

ICCItem *CCodeChain::CreateInteger (int iValue)

//	CreateInteger
//
//	Creates an item

	{
	ICCItem *pItem;
	CCInteger *pInteger;

	pItem = m_IntegerPool.CreateItem(this);
	if (pItem->IsError())
		return pItem;

	pInteger = dynamic_cast<CCInteger *>(pItem);
	pInteger->Reset();
	pInteger->SetValue(iValue);

	return pInteger->Reference();
	}

ICCItem *CCodeChain::CreateLambda (ICCItem *pList, BOOL bArgsOnly)

//	CreateLambda
//
//	Creates a lambda expression from a list beginning with
//	a lambda symbol

	{
	ICCItem *pItem;
	CCLambda *pLambda;

	pItem = m_LambdaPool.CreateItem(this);
	if (pItem->IsError())
		return pItem;

	pLambda = dynamic_cast<CCLambda *>(pItem);
	pLambda->Reset();

	//	Initialize from list

	if (pList)
		{
		pItem = pLambda->CreateFromList(this, pList, bArgsOnly);
		if (pItem->IsError())
			{
			DestroyLambda(pLambda);
			return pItem;
			}

		pItem->Discard(this);
		}

	//	Done

	return pLambda->Reference();
	}

ICCItem *CCodeChain::CreateLinkedList (void)

//	CreateLinkedList
//
//	Creates an item

	{
	ICCItem *pItem;

	pItem = m_ListPool.CreateItem(this);
	if (pItem->IsError())
		return pItem;

	pItem->Reset();
	return pItem->Reference();
	}

ICCItem *CCodeChain::CreatePrimitive (PRIMITIVEPROCDEF *pDef)

//	CreatePrimitive
//
//	Creates a primitive function

	{
	ICCItem *pItem;
	CCPrimitive *pPrimitive;

	pItem = m_PrimitivePool.CreateItem(this);
	if (pItem->IsError())
		return pItem;

	pPrimitive = dynamic_cast<CCPrimitive *>(pItem);
	pPrimitive->SetProc(pDef);
	return pPrimitive->Reference();
	}

ICCItem *CCodeChain::CreateString (const CString &sString)

//	CreateString
//
//	Creates an item

	{
	ICCItem *pItem;
	CCString *pString;

	pItem = m_StringPool.CreateItem(this);
	if (pItem->IsError())
		return pItem;

	pString = dynamic_cast<CCString *>(pItem);
	pString->Reset();
	pString->SetValue(sString);
	return pString->Reference();
	}

ICCItem *CCodeChain::CreateSymbolTable (void)

//	CreateSymbolTable
//
//	Creates an item

	{
	ICCItem *pItem;

	pItem = m_SymbolTablePool.CreateItem(this);
	if (pItem->IsError())
		return pItem;

	pItem->Reset();
	return pItem->Reference();
	}

ICCItem *CCodeChain::CreateSystemError (ALERROR error)

//	CreateSystemError
//
//	Generates an error

	{
	ICCItem *pError;

	pError = CreateString(strPatternSubst(LITERAL("ERROR: %d"), error));
	pError->SetError();
	return pError;
	}

ICCItem *CCodeChain::CreateVector (int iSize)

//	CreateVector
//
//	Creates a vector of the given number of elements

	{
	CCVector *pVector;
	ICCItem *pError;

	pVector = new CCVector(this);
	if (pVector == NULL)
		return CreateMemoryError();

	pError = pVector->SetSize(this, iSize);
	if (pError->IsError())
		{
		delete pVector;
		return pError;
		}

	pError->Discard(this);

	//	Done

	return pVector->Reference();
	}

ALERROR CCodeChain::DefineGlobal (const CString &sVar, ICCItem *pValue)

//	DefineGlobal
//
//	Defines a global variable programmatically

	{
	ALERROR error;

	//	Create a string item

	ICCItem *pVar = CreateString(sVar);

	//	Add the symbol

	ICCItem *pError;
	pError = m_pGlobalSymbols->AddEntry(this, pVar, pValue);
	pVar->Discard(this);

	//	Check for error

	if (pError->IsError())
		error = ERR_FAIL;
	else
		error = NOERROR;

	pError->Discard(this);

	return error;
	}

ALERROR CCodeChain::DefineGlobalInteger (const CString &sVar, int iValue)
	{
	ALERROR error;
	ICCItem *pValue = CreateInteger(iValue);
	error = DefineGlobal(sVar, pValue);
	pValue->Discard(this);
	return error;
	}

ALERROR CCodeChain::DefineGlobalString (const CString &sVar, const CString &sValue)
	{
	ALERROR error;
	ICCItem *pValue = CreateString(sValue);
	error = DefineGlobal(sVar, pValue);
	pValue->Discard(this);
	return error;
	}

void CCodeChain::DiscardAllGlobals (void)

//	DiscardAllGlobals
//
//	Discard all globals.

	{
	if (m_pGlobalSymbols)
		m_pGlobalSymbols->DeleteAll(this, true);
	}

ICCItem *CCodeChain::Eval (CEvalContext *pEvalCtx, ICCItem *pItem)

//	Eval
//
//	Evaluates the given item and returns a result

	{
	//	Errors always evaluate to themselves

	if (pItem->IsError())
		return pItem->Reference();

	//	If this item is quoted, then return an unquoted item

	if (pItem->IsQuoted())
		{
		//	If this is a literal symbol table then we need to evaluate its 
		//	values.

		if (pItem->IsSymbolTable())
			return EvalLiteralStruct(pEvalCtx, pItem);

		//	HACK: We clone the item so that when we try to modify a literal list we
		//	mody a copy instead of the original.

		else
			{
			ICCItem *pResult = pItem->Clone(this);
			pResult->ClearQuoted();
			return pResult;
			}
		}

	//	Evaluate differently depending on whether or not
	//	this is an atom or a list. If it is an atom, either return
	//	the value or look up the atom in a symbol table. If the item
	//	is a list, try to evaluate as a function

	else if (pItem->IsIdentifier())
		return Lookup(pEvalCtx, pItem);

	//	If this is an expression (a list with multiple terms) then we
	//	try to evaluate it.

	else if (pItem->IsExpression())
		{
		ICCItem *pFunctionName;
		ICCItem *pFunction;
		ICCItem *pArgs;
		ICCItem *pResult;

		//	The first element of the list is the function

		pFunctionName = pItem->Head(this);

		//	We must have a first element since this is a list (but not Nil)

		ASSERT(pFunctionName);

		//	If this is an identifier, then look up the function
		//	in the global symbols

		if (pFunctionName->IsIdentifier())
			pFunction = LookupFunction(pEvalCtx, pFunctionName);

		//	Otherwise, evaluate it

		else
			pFunction = Eval(pEvalCtx, pFunctionName);

		//	If we get an error, return it

		if (pFunction->IsError())
			return pFunction;

		//	Make sure this is a function

		if (!pFunction->IsFunction())
			{
			pFunction->Discard(this);
			return CreateError(LITERAL("Function name expected"), pFunctionName);
			}

		//	Get the arguments

		pArgs = pItem->Tail(this);

		//	Do it

		pResult = pFunction->Execute(pEvalCtx, pArgs);

		//	Handle error by appending the function call that failed

		if (pResult->IsError())
			{
			CCString *pError = dynamic_cast<CCString *>(pResult);
			if (pError)
				{
				CString sError = pError->GetValue();
				if (!sError.IsBlank())
					{
					char *pPos = sError.GetASCIIZPointer() + sError.GetLength() - 1;
					if (*pPos != '#')
						{
						sError.Append(strPatternSubst(CONSTLIT(" ### %s ###"), pItem->Print(this)));
						pError->SetValue(sError);
						}
					}
				}
			}

		//	Done

		pFunction->Discard(this);
		pArgs->Discard(this);
		return pResult;
		}
	
	//	Anything else is a literal so we return it.

	else
		return pItem->Reference();
	}

ICCItem *CCodeChain::EvalLiteralStruct (CEvalContext *pCtx, ICCItem *pItem)

//	EvalLiteralStruct
//
//	Evalues the values in a structure and returns a structure
//	(or an error).

	{
	int i;

	CCSymbolTable *pTable = dynamic_cast<CCSymbolTable *>(pItem);
	if (pTable == NULL)
		return CreateError(CONSTLIT("Not a structure"), pItem);

	ICCItem *pNew = CreateSymbolTable();
	if (pNew->IsError())
		return pNew;

	CCSymbolTable *pNewTable = dynamic_cast<CCSymbolTable *>(pNew);

	//	Loop over all key/value pairs

	for (i = 0; i < pTable->GetCount(); i++)
		{
		CString sKey = pTable->GetKey(i);
		ICCItem *pValue = pTable->GetElement(i);

		ICCItem *pNewKey = CreateString(sKey);
		ICCItem *pNewValue = (pValue ? Eval(pCtx, pValue) : CreateNil());

		ICCItem *pResult = pNewTable->AddEntry(this, pNewKey, pNewValue);
		pNewKey->Discard(this);
		pNewValue->Discard(this);

		if (pResult->IsError())
			{
			pNewTable->Discard(this);
			return pResult;
			}

		pResult->Discard(this);
		}

	//	Done

	return pNewTable;
	}

ICCItem *CCodeChain::EvaluateArgs (CEvalContext *pCtx, ICCItem *pArgs, const CString &sArgValidation)

//	EvaluateArgs
//
//	Evaluate arguments and validate their types

	{
	ICCItem *pArg;
	ICCItem *pNew;
	ICCItem *pError;
	CCLinkedList *pEvalList;
	char *pValidation;
	int i;
	BOOL bNoEval;

	//	If the argument list if quoted, then it means that the arguments
	//	have already been evaluated. This happens if we've been called by
	//	(apply).

	bNoEval = pArgs->IsQuoted();

	//	Create a list to hold the results

	pNew = CreateLinkedList();
	if (pNew->IsError())
		return pNew;

	pEvalList = dynamic_cast<CCLinkedList *>(pNew);

	//	Start parsing at the beginning

	pValidation = sArgValidation.GetPointer();

	//	If there is a '*' in the validation, figure out
	//	how many arguments it represents

	int iVarArgs = Max(0, pArgs->GetCount() - (sArgValidation.GetLength() - 1));

	//	Loop over each argument

	for (i = 0; i < pArgs->GetCount(); i++)
		{
		ICCItem *pResult;

		pArg = pArgs->GetElement(i);

		//	If we're processing variable args, see if we're done

		if (*pValidation == '*')
			{
			if (iVarArgs == 0)
				pValidation++;
			else
				iVarArgs--;
			}

		//	Evaluate the item. If the arg is 'q' or 'u' then we 
		//	don't evaluate it.

		if (bNoEval || *pValidation == 'q' || *pValidation == 'u')
			pResult = pArg->Reference();

		//	If the arg is 'c' then we don't evaluate unless it is
		//	a lambda expression (or an identifier)

		else if (*pValidation == 'c' && !pArg->IsLambdaExpression() && !pArg->IsIdentifier())
			pResult = pArg->Reference();

		//	Evaluate

		else
			{
			pResult = Eval(pCtx, pArg);

			//	We don't want to return on error because some functions
			//	might want to pass errors around

			if (*pValidation != 'v' && *pValidation != '*')
				{
				if (pResult->IsError())
					{
					pEvalList->Discard(this);
					return pResult;
					}
				}
			}

		//	Check to see if the item is valid

		switch (*pValidation)
			{
			//	We expect a function...

			case 'f':
				{
				if (!pResult->IsFunction())
					{
					pError = CreateError(LITERAL("Function expected"), pResult);
					pResult->Discard(this);
					pEvalList->Discard(this);
					return pError;
					}
				break;
				}

			//	We expect an integer...

			case 'i':
				{
				if (!pResult->IsInteger())
					{
					pError = CreateError(LITERAL("Integer expected"), pResult);
					pResult->Discard(this);
					pEvalList->Discard(this);
					return pError;
					}
				break;
				}

			//	We expect a linked list

			case 'k':
				{
				if (pResult->GetClass()->GetObjID() != OBJID_CCLINKEDLIST)
					{
					pError = CreateError(LITERAL("Linked-list expected"), pResult);
					pResult->Discard(this);
					pEvalList->Discard(this);
					return pError;
					}
				break;
				}

			//	We expect a list

			case 'l':
				{
				if (!pResult->IsList())
					{
					pError = CreateError(LITERAL("List expected"), pResult);
					pResult->Discard(this);
					pEvalList->Discard(this);
					return pError;
					}
				break;
				}

			//	We expect an identifier

			case 's':
			case 'q':
				{
				if (!pResult->IsIdentifier())
					{
					pError = CreateError(LITERAL("Identifier expected"), pResult);
					pResult->Discard(this);
					pEvalList->Discard(this);
					return pError;
					}
				break;
				}

			//	We expect an atom table

			case 'x':
				{
				if (!pResult->IsAtomTable())
					{
					pError = CreateError(LITERAL("Atom table expected"), pResult);
					pResult->Discard(this);
					pEvalList->Discard(this);
					return pError;
					}
				break;
				}

			//	We expect a symbol table

			case 'y':
				{
				if (!pResult->IsSymbolTable())
					{
					pError = CreateError(LITERAL("Symbol table expected"), pResult);
					pResult->Discard(this);
					pEvalList->Discard(this);
					return pError;
					}
				break;
				}

			//	We expect anything

			case 'c':
			case 'u':
			case 'v':
				break;

			//	We expect any number of anythings...

			case '*':
				break;

			//	Too many arguments

			case '\0':
				{
				pError = CreateError(LITERAL("Too many arguments"), NULL);
				pResult->Discard(this);
				pEvalList->Discard(this);
				return pError;
				}

			default:
				ASSERT(FALSE);
			}

		//	Add the result to the list

		pEvalList->Append(this, pResult, NULL);
		pResult->Discard(this);

		//	Next validation sequence (note that *pValidation can never
		//	be '\0' because we return above if we find it)

		if (*pValidation != '*')
			pValidation++;
		}

	//	Make sure we have enough arguments

	if (*pValidation != '\0' && *pValidation != '*')
		{
		pError = CreateError(LITERAL("Insufficient arguments"), NULL);
		pEvalList->Discard(this);
		return pError;
		}

	//	Return the evaluation list

	return pEvalList;
	}

bool CCodeChain::HasIdentifier (ICCItem *pCode, const CString &sIdentifier)
	{
	if (!pCode->IsExpression())
		{
		if (pCode->IsIdentifier())
			return strEquals(pCode->GetStringValue(), sIdentifier);
		else
			return false;
		}
	else
		{
		for (int i = 0; i < pCode->GetCount(); i++)
			{
			ICCItem *pElement = pCode->GetElement(i);
			if (HasIdentifier(pElement, sIdentifier))
				return true;
			}

		return false;
		}
	}

ICCItem *CCodeChain::ListGlobals (void)

//	ListGlobals
//
//	Returns a list of all the symbols in the global symbol table

	{
	return m_pGlobalSymbols->ListSymbols(this);
	}

ICCItem *CCodeChain::Lookup (CEvalContext *pCtx, ICCItem *pItem)

//	Lookup
//
//	Returns the binding for this item

	{
	ICCItem *pBinding = NULL;
	BOOL bFound;
	ICCItem *pStart;
	int iFrame, iOffset;

	//	Start somewhere

	if (pCtx->pLocalSymbols)
		pStart = pCtx->pLocalSymbols;
	else
		pStart = pCtx->pLexicalSymbols;

	//	If this item already has been resolved, do a quick lookup

	if (pItem->GetBinding(&iFrame, &iOffset))
		{
		while (iFrame > 0)
			{
			pStart = pStart->GetParent();
			iFrame--;
			}

		pBinding = pStart->LookupByOffset(this, iOffset);
		bFound = TRUE;
		}

	//	Otherwise, do a lookup

	else
		{
		bFound = FALSE;
		iFrame = 0;

		while (!bFound && pStart)
			{
			pBinding = pStart->SimpleLookup(this, pItem, &bFound, &iOffset);
			if (!bFound)
				{
				pBinding->Discard(this);
				pBinding = NULL;
				pStart = pStart->GetParent();
				iFrame++;
				}
			}

		//	If we found it and this is a local frame,
		//	set the resolution info

		if (bFound && pStart->IsLocalFrame())
			pItem->SetBinding(iFrame, iOffset);
		}

	//	If there is no binding, return an error

	if (!bFound)
		{
		ASSERT(pBinding == NULL);
		//pBinding->Discard(this);
		pBinding = CreateError(LITERAL("No binding for symbol"), pItem);
		}

	return pBinding;
	}

ICCItem *CCodeChain::LookupFunction (CEvalContext *pCtx, ICCItem *pName)

//	LookupFunction
//
//	Returns the binding for a function

	{
	ICCItem *pBinding;

	//	See if the identifier is already bound

	pBinding = pName->GetFunctionBinding();
	if (pBinding)
		return pBinding;

	//	If not bound, check global scope

	pBinding = m_pGlobalSymbols->Lookup(this, pName);
	if (!pBinding->IsError() || pBinding->GetIntegerValue() != CCRESULT_NOTFOUND)
		{
		pName->SetFunctionBinding(this, pBinding);
		return pBinding;
		}

	//	If not found, check local scope

	if (pCtx)
		{
		pBinding->Discard(this);
		pBinding = Lookup(pCtx, pName);
		if (!pBinding->IsError())
			//	We don't set a function binding because local variables often change
			//	(as when we pass a function as a parameter)
			return pBinding;
		}

	//	If we get this far then we could not find it

	pBinding->Discard(this);
	pBinding = CreateError(LITERAL("Unknown function"), pName);
	return pBinding;
	}

ICCItem *CCodeChain::LookupGlobal (const CString &sGlobal, LPVOID pExternalCtx)

//	LookupGlobal
//
//	Returns the binding for this item

	{
	CEvalContext EvalCtx;
	ICCItem *pItem;
	ICCItem *pResult;

	//	Set up the context

	EvalCtx.pCC = this;
	EvalCtx.pLexicalSymbols = m_pGlobalSymbols;
	EvalCtx.pLocalSymbols = NULL;
	EvalCtx.pExternalCtx = pExternalCtx;

	//	Create a variable

	pItem = CreateString(sGlobal);
	if (pItem->IsError())
		return pItem;

	pResult = Lookup(&EvalCtx, pItem);
	pItem->Discard(this);
	return pResult;
	}

ICCItem *CCodeChain::PoolUsage (void)

//	PoolUsage
//
//	Returns a count of each pool

	{
	int iPoolCount[POOL_COUNT];
	int i;
	ICCItem *pResult;
	CCLinkedList *pList;

	//	Get the counts now so we don't affect the results

	iPoolCount[INTEGER_POOL] = m_IntegerPool.GetCount();
	iPoolCount[STRING_POOL] = m_StringPool.GetCount();
	iPoolCount[LIST_POOL] = m_ListPool.GetCount();
	iPoolCount[PRIMITIVE_POOL] = m_PrimitivePool.GetCount();
	iPoolCount[SYMBOLTABLE_POOL] = m_SymbolTablePool.GetCount();
	iPoolCount[LAMBDA_POOL] = m_LambdaPool.GetCount();
	iPoolCount[ATOMTABLE_POOL] = m_AtomTablePool.GetCount();

	//	Create

	pResult = CreateLinkedList();
	if (pResult->IsError())
		return pResult;

	pList = (CCLinkedList *)pResult;

	for (i = 0; i < POOL_COUNT; i++)
		{
		ICCItem *pItem;

		//	Make an item for the count

		pItem = CreateInteger(iPoolCount[i]);

		//	Add the item to the list

		pList->Append(this, pItem, NULL);
		pItem->Discard(this);
		}

	return pList;
	}

ICCItem *CCodeChain::TopLevel (ICCItem *pItem, LPVOID pExternalCtx)

//	TopLevel
//
//	Top level command processor

	{
	CEvalContext EvalCtx;

	//	Set up the context

	EvalCtx.pCC = this;
	EvalCtx.pLexicalSymbols = m_pGlobalSymbols;
	EvalCtx.pLocalSymbols = NULL;
	EvalCtx.pExternalCtx = pExternalCtx;

	//	Evalute the actual code

	return Eval(&EvalCtx, pItem);
	}

ALERROR CCodeChain::RegisterPrimitive (PRIMITIVEPROCDEF *pDef)

//	RegisterPrimitive
//
//	Registers a primitive function implemented in C

	{
	ICCItem *pError;
	ICCItem *pDefinition;

	//	Create a primitive definition

	pDefinition = CreatePrimitive(pDef);
	if (pDefinition->IsError())
		{
		pDefinition->Discard(this);
		return ERR_FAIL;
		}

	//	Add to global symbol table

	pError = m_pGlobalSymbols->AddEntry(this, pDefinition, pDefinition);
	if (pError->IsError())
		{
		pError->Discard(this);
		return ERR_FAIL;
		}

	//	Don't need these anymore; AddEntry keeps a reference

	pError->Discard(this);
	pDefinition->Discard(this);

	return NOERROR;
	}

ICCItem *CCodeChain::StreamItem (ICCItem *pItem, IWriteStream *pStream)

//	StreamItem
//
//	Save the item to an open stream

	{
	ALERROR error;
	DWORD dwClass;
	ICCItem *pError;

	//	Save out the object class

	dwClass = pItem->GetClass()->GetObjID();
	if (error = pStream->Write((char *)&dwClass, sizeof(dwClass), NULL))
		return CreateSystemError(error);

	//	Let the object save itself

	pError = pItem->Stream(this, pStream);
	if (pError->IsError())
		return pError;

	pError->Discard(this);

	//	Done

	return CreateTrue();
	}

ICCItem *CCodeChain::UnstreamItem (IReadStream *pStream)

//	UnstreamItem
//
//	Load the item from an open stream

	{
	ALERROR error;
	DWORD dwClass;
	ICCItem *pItem;
	ICCItem *pError;

	//	Read the object class

	if (error = pStream->Read((char *)&dwClass, sizeof(dwClass), NULL))
		return CreateSystemError(error);

	//	Instantiation an object of the right class

	if (dwClass == OBJID_CCINTEGER)
		pItem = m_IntegerPool.CreateItem(this);
	else if (dwClass == OBJID_CCSTRING)
		pItem = m_StringPool.CreateItem(this);
	else if (dwClass == OBJID_CCLINKEDLIST)
		pItem = m_ListPool.CreateItem(this);
	else if (dwClass == OBJID_CCPRIMITIVE)
		pItem = m_PrimitivePool.CreateItem(this);
	else if (dwClass == OBJID_CCNIL)
		pItem = m_pNil;
	else if (dwClass == OBJID_CCTRUE)
		pItem = m_pTrue;
	else if (dwClass == OBJID_CCSYMBOLTABLE)
		pItem = m_SymbolTablePool.CreateItem(this);
	else if (dwClass == OBJID_CCLAMBDA)
		pItem = m_LambdaPool.CreateItem(this);
	else if (dwClass == OBJID_CCATOMTABLE)
		pItem = m_AtomTablePool.CreateItem(this);
	else if (dwClass == OBJID_CCVECTOR)
		{
		pItem = new CCVector(this);
		if (pItem == NULL)
			pItem = CreateMemoryError();
		}
	else
		return CreateError(LITERAL("Unknown item type"), NULL);

	//	Check for error

	if (pItem->IsError())
		return pItem;

	//	We need to increment the reference here because the native
	//	create does not.

	pItem->Reset();
	pItem->Reference();

	//	Let the item load the rest

	pError = pItem->Unstream(this, pStream);
	if (pError->IsError())
		{
		pItem->Discard(this);
		return pError;
		}

	pError->Discard(this);

	//	Done

	return pItem;
	}

