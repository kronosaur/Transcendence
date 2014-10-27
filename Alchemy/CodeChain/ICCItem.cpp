//	ICCItem.cpp
//
//	Implements ICCItem class

#include <windows.h>
#include "Kernel.h"
#include "KernelObjID.h"
#include "CodeChain.h"

#define ITEM_FLAG_QUOTED					0x00000001
#define ITEM_FLAG_ERROR						0x00000002
#define ITEM_FLAG_PARTOFLIST				0x00000004
#define ITEM_FLAG_NO_REF_COUNT				0x00000008
#define ITEM_FLAG_READ_ONLY					0x00000010

ICCItem::ICCItem (IObjectClass *pClass) : CObject(pClass)

//	ICCItem constructor

	{
	ResetItem();
	}

void ICCItem::CloneItem (ICCItem *pItem)

//	CloneItem
//
//	Makes this object take on the characteristics of the
//	given item (except for the ref count)

	{
	//	No need to set the refcount because it has already
	//	been set to 1

	m_bModified = FALSE;
	m_bReadOnly = FALSE;

	m_bQuoted = pItem->m_bQuoted;
	m_bError = pItem->m_bError;
	m_bNoRefCount = pItem->m_bNoRefCount;
	}

void ICCItem::Discard (CCodeChain *pCC)

//	Discard
//
//	Decrement reference count and delete if necessary

	{
	//	By checking for very-high reference counts we can
	//	also prevent discarding that are already in the free list
	//	(because we use the refcount field to store the free list chain)
	ASSERT(m_bNoRefCount || (m_dwRefCount > 0 && m_dwRefCount < 0x00010000));

	m_dwRefCount--;
	if (m_dwRefCount == 0)
		{
		//	Let subclasses handle this

		DestroyItem(pCC);
		}
	}

ICCItem *ICCItem::Execute (CEvalContext *pCtx, ICCItem *pArgs)

//	Execute
//
//	Execute this function

	{
	return pCtx->pCC->CreateNil();
	}

ICCItem *ICCItem::GetElement (CCodeChain *pCC, int iIndex)

//	GetElement
//
//	Returns an element that must be discarded

	{
	ICCItem *pItem = GetElement(iIndex);
	if (pItem == NULL)
		return pCC->CreateNil();

	return pItem->Reference();
	}

CString ICCItem::GetTypeOf (void)

//	GetTypeOf
//
//	Returns the type of basic types. Custom types should override this method.

	{
	if (IsError())
		return CONSTLIT("error");
	else if (IsNil())
		return CONSTLIT("nil");
	else if (IsInteger())
		return CONSTLIT("int32");
	else if (IsPrimitive())
		return CONSTLIT("primitive");
	else if (IsLambdaFunction())
		return CONSTLIT("function");
	else if (IsIdentifier())
		return CONSTLIT("string");
	else if (IsSymbolTable())
		return CONSTLIT("struct");
	else if (IsList())
		return CONSTLIT("list");
	else
		return CONSTLIT("true");
	}

bool ICCItem::IsLambdaExpression (void)

//	IsLambdaExpression
//
//	Returns TRUE if this is a lambda expression

	{
	return (IsList() && GetCount() >= 1 && GetElement(0)->IsLambdaSymbol());
	}

BOOL ICCItem::IsLambdaSymbol (void)

//	IsLambdaSymbol
//
//	Returns TRUE if this is the symbol lambda

	{
	if (IsIdentifier() && strCompareAbsolute(GetStringValue(), CONSTLIT("lambda")) == 0)
		return TRUE;
	else
		return FALSE;
	}

ICCItem *ICCItem::NotASymbolTable(CCodeChain *pCC)

//	NotASymbolTable
//
//	Returns an error when item is not a symbol table

	{
	return pCC->CreateError(LITERAL("Not a symbol table"));
	}

void ICCItem::ResetItem (void)

//	ResetItem
//
//	Returns item to original condition

	{
	m_dwRefCount = 0;
	m_bQuoted = FALSE;
	m_bError = FALSE;
	m_bModified = FALSE;
	m_bNoRefCount = FALSE;
	m_bReadOnly = FALSE;
	}

ICCItem *ICCItem::Stream (CCodeChain *pCC, IWriteStream *pStream)

//	Stream
//
//	Stream the item

	{
	ALERROR error;
	DWORD dwFlags = 0;

	//	Convert the flags into a single DWORD
	//	We do not save PartOfList because streamed elements are
	//	never part of a list until they are loaded back

	if (m_bQuoted)
		dwFlags |= ITEM_FLAG_QUOTED;
	if (m_bError)
		dwFlags |= ITEM_FLAG_ERROR;
	if (m_bNoRefCount)
		dwFlags |= ITEM_FLAG_NO_REF_COUNT;
	if (m_bReadOnly)
		dwFlags |= ITEM_FLAG_READ_ONLY;

	//	Save the flags

	if (error = pStream->Write((char *)&dwFlags, sizeof(dwFlags), NULL))
		return pCC->CreateSystemError(error);

	//	Save the sub-class specific stuff

	return StreamItem(pCC, pStream);
	}

ICCItem *ICCAtom::Enum (CEvalContext *pCtx, ICCItem *pCode)

//	Enum
//
//	Enumerates over all elements in the list

	{
	return pCtx->pCC->CreateNil();
	}

ICCItem *ICCAtom::Tail (CCodeChain *pCC)

//	Tail
//
//	Returns the tail of an atom

	{
	return pCC->CreateNil();
	}

ICCItem *ICCItem::Unstream (CCodeChain *pCC, IReadStream *pStream)

//	Unstream
//
//	Unstream the item

	{
	ALERROR error;
	DWORD dwFlags;

	//	Read the flags

	if (error = pStream->Read((char *)&dwFlags, sizeof(dwFlags), NULL))
		return pCC->CreateSystemError(error);

	//	Map flags

	m_bQuoted = ((dwFlags & ITEM_FLAG_QUOTED) ? TRUE : FALSE);
	m_bError = ((dwFlags & ITEM_FLAG_ERROR) ? TRUE : FALSE);
	m_bNoRefCount = ((dwFlags & ITEM_FLAG_NO_REF_COUNT) ? TRUE : FALSE);
	m_bReadOnly = ((dwFlags & ITEM_FLAG_READ_ONLY) ? TRUE : FALSE);

	//	Clear modified

	m_bModified = FALSE;

	//	Load the item specific stuff

	return UnstreamItem(pCC, pStream);
	}

//	IItemTransform -------------------------------------------------------------

ICCItem *IItemTransform::Transform (CCodeChain &CC, ICCItem *pItem)
	{
	return pItem->Reference();
	}
