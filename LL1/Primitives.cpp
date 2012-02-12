//	Primitives.cpp
//
//	Primitive extensions to CodeChain interpreter

#include "PreComp.h"

#define FN_ENTRY_SETBODY				1
#define FN_ENTRY_SETTITLE				2
#define FN_ENTRY_ADDCATEGORY			3
#define FN_ENTRY_ADDFIELD				4

ICCItem *fnEntrySet (CEvalContext *pEvalCtx, ICCItem *pArguments, DWORD dwData);
CEntry *GetEntryArg (ICCItem *pArg);

static PRIMITIVEPROCDEF g_Extensions[] =
	{
		{	"entAddCategory",		fnEntrySet,		FN_ENTRY_ADDCATEGORY,	"",		"",		PPFLAG_SIDEEFFECTS,	},
		{	"entAddField",			fnEntrySet,		FN_ENTRY_ADDFIELD,	"",		"",		PPFLAG_SIDEEFFECTS,	},
		{	"entSetBody",			fnEntrySet,		FN_ENTRY_SETBODY,	"",		"",		PPFLAG_SIDEEFFECTS,	},
		{	"entSetTitle",			fnEntrySet,		FN_ENTRY_SETTITLE,	"",		"",		PPFLAG_SIDEEFFECTS,	},
	};

#define EXTENSIONS_COUNT		(sizeof(g_Extensions) / sizeof(g_Extensions[0]))

ALERROR CCore::RegisterPrimitives (void)

//	RegisterPrimitives
//
//	Registers all the primitive extensions

	{
	ALERROR error;
	int i;

	for (i = 0; i < EXTENSIONS_COUNT; i++)
		if (error = m_pCC->RegisterPrimitive(&g_Extensions[i]))
			return error;

	return NOERROR;
	}

CEntry *GetEntryArg (ICCItem *pArg)

//	GetEntryArg
//
//	Converts an integer argument into an entry pointer. We assume
//	that pArg has already been validated as an integer.
//	Returns NULL if it is not a valid entry.

	{
	return (CEntry *)pArg->GetIntegerValue();
	}

ICCItem *fnEntrySet (CEvalContext *pEvalCtx, ICCItem *pArguments, DWORD dwData)

//	fnEntrySet
//
//	Sets fields in an entry
//
//	(entAddCategory entry category)
//	(entSetField entry field-name field-type value)
//	(entSetBody entry body)
//	(entSetTitle entry title)

	{
	ALERROR error;
	CCodeChain *pCC = pEvalCtx->pCC;
	COREEXECCTX *pCtx = (COREEXECCTX *)pEvalCtx->pExternalCtx;
	ICCItem *pArgs;
	CEntry *pEntry;
	CString sParsing;

	//	Parse the parameters

	if (dwData == FN_ENTRY_ADDFIELD)
		sParsing = LITERAL("isis");
	else
		sParsing = LITERAL("is");

	//	Evaluate the arguments and validate them

	pArgs = pCC->EvaluateArgs(pEvalCtx, pArguments, sParsing);
	if (pArgs->IsError())
		return pArgs;

	//	Convert the first argument into an entry

	pEntry = GetEntryArg(pArgs->GetElement(0));
	if (pEntry == NULL)
		{
		pArgs->Discard(pCC);
		return pCC->CreateError(LITERAL("Entry pointer expected:"), pArgs->GetElement(0));
		}

	//	Do the appropriate command

	switch (dwData)
		{
		case FN_ENTRY_ADDFIELD:
			{
			CString sFieldName;
			CString sValue;
			CField::DataTypes FieldType;
			CField *pNewField;
			CString sError;
			int iErrorPos;

			//	Get the parameters

			sFieldName = pArgs->GetElement(1)->GetStringValue();
			FieldType = (CField::DataTypes)pArgs->GetElement(2)->GetIntegerValue();
			sValue = pArgs->GetElement(3)->GetStringValue();
			pArgs->Discard(pCC);

			//	Validate the XML

			if (error = XMLValidate(sValue, &sError, &iErrorPos))
				return pCC->CreateError(sError, NULL);

			//	Create the field

			pNewField = new CField;
			if (pNewField == NULL)
				return pCC->CreateMemoryError();

			pNewField->SetName(sFieldName);
			pNewField->SetType(FieldType);
			pNewField->SetSingleValue(sValue);

			//	Add to the entry

			if (error = pEntry->AppendField(pNewField))
				{
				delete pNewField;
				return pCC->CreateSystemError(error);
				}

			break;
			}

		case FN_ENTRY_ADDCATEGORY:
		case FN_ENTRY_SETBODY:
		case FN_ENTRY_SETTITLE:
			{
			CString sText;
			CString sError;
			int iErrorPos;

			//	Get the new body

			sText = pArgs->GetElement(1)->GetStringValue();
			pArgs->Discard(pCC);

			//	Validate the XML

			if (error = XMLValidate(sText, &sError, &iErrorPos))
				return pCC->CreateError(sError, NULL);

			//	Set the body for the entry

			if (dwData == FN_ENTRY_SETBODY)
				pEntry->SetBody(sText);
			else if (dwData == FN_ENTRY_SETTITLE)
				pEntry->SetTitle(sText);
			else if (dwData == FN_ENTRY_ADDCATEGORY)
				pEntry->AddCategory(sText);

			break;
			}

		default:
			ASSERT(FALSE);
		}

	//	Done

	return pCC->CreateTrue();
	}
