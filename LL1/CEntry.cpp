//	CEntry.cpp
//
//	CEntry object

#include "PreComp.h"
#include "Tags.h"

#define XML_VERSION							"1.0"
#define XML_ANSI_SIGNATURE					0x4d583f3c	//	little-endian "<?XM"

#define LIBRARYLINK_ENTRY_TAGID				0
#define TITLE_TAGID							1
#define BODY_TAGID							2
#define CATEGORIES_TAGID						3
#define FIELD_TAGID							4
#define CREATED_BY_TAGID						5
#define CREATED_ON_TAGID						6
#define MODIFIED_ON_TAGID						7
#define TEMPLATE_TAGID						8
#define TEXTBREAK_TAGID						9
#define XML_HEADER_TAGID						10

typedef struct
	{
	char *pPos;								//	Input stream
	CEntry *pEntry;							//	Entry to create
	int iLine;								//	Current line number

	CSymbolTable *pAttributes;				//	Current element attributes

	CString sErrorMsg;						//	Error message
	} LOADCTX;

static char *g_pszTag[] =
	{
	LIBRARY_LINK_ENTRY_TAG,
	TITLE_TAG,
	BODY_TAG,
	CATEGORIES_TAG,
	FIELD_TAG,
	CREATED_BY_TAG,
	CREATED_ON_TAG,
	MODIFIED_ON_TAG,
	TEMPLATE_TAG,
	BREAK_TAG,
	XML_TAG,
	NULL,
	};

static char *g_pszFieldType[] =
	{
	"DATE",
	"NUMBER",
	"TEXT",
	"PERSON-NAME",
	"CODE",
	NULL,
	};

//	Forwards

ALERROR GetAttribute (LOADCTX *pCtx, CString sAttribute, CString *retsValue);
int LookupSymbol (char **pTable, CString sTag);
ALERROR OutputAttribute (CMemoryWriteStream *pOutput, CString sAttribute, CString sValue);
ALERROR OutputField (CMemoryWriteStream *pOutput, CField *pField);
ALERROR OutputFieldElementHeader (CMemoryWriteStream *pOutput, int iTag, CString sFieldName);

#define OTFLAG_NEWLINE							0x00000001	//	New line after tag
#define OTFLAG_INDENT							0x00000002	//	Indent tag
#define OTFLAG_ATTRIBUTES						0x00000004	//	Allow attributes
#define OTFLAG_CLOSETAG							0x00000008	//	Close tag
ALERROR OutputTag (CMemoryWriteStream *pOutput, int iTag, DWORD dwFlags);

ALERROR OutputText (CMemoryWriteStream *pOutput, CString sText);
ALERROR OutputTextElement (CMemoryWriteStream *pOutput, int iTag, CString sFieldName, CString sValue);
ALERROR OutputTextListElement (CMemoryWriteStream *pOutput, int iTag, CString sFieldName, CStringArray &Values);
ALERROR OutputTimeDateElement (CMemoryWriteStream *pOutput, int iTag, CString sFieldName, TIMEDATE &tdTimeDate);
ALERROR ParseAttributes (LOADCTX *pCtx);
ALERROR ParseLibraryLinkEntry (LOADCTX *pCtx);
ALERROR ParseTag (LOADCTX *pCtx, int *retiTag, BOOL *retbClose);

#define PTFLAG_NOBREAKS							0x00000001	//	Ignore <BR>
ALERROR ParseText (LOADCTX *pCtx, CStringArray *pValues, DWORD dwFlags);

void ParseWhitespace (LOADCTX *pCtx);
void String2TimeDate (CString sTimeDate, TIMEDATE *rettd);
CString TimeDate2String (TIMEDATE &tdTimeDate);

//	Inlines

inline BOOL IsWhitespace (char chChar)
	{
	return chChar == ' ' || chChar == '\t' || chChar == '\n' || chChar == '\r';
	}

CEntry::CEntry (CCore *pCore) :
		m_pCore(pCore),
		m_dwUNID(UndefinedUNID),
		m_FieldList(TRUE),
		m_CodeList(FALSE, TRUE),
		m_fEphemeral(FALSE)

//	CEntry constructor

	{
	SetCreation();
	}

CEntry::~CEntry (void)

//	CEntry destructor

	{
	int i;

	//	We need to discard the code

	for (i = 0; i < m_CodeList.GetCount(); i++)
		{
		ICCItem *pItem = (ICCItem *)m_CodeList.GetValue(i);
		pItem->Discard(m_pCore->GetCC());
		}
	}

ALERROR CEntry::AddCategory (CString sCategory)

//	AddCategory
//
//	Adds the category if not already added

	{
	if (!InCategory(sCategory))
		return m_Categories.AppendString(sCategory, NULL);

	return NOERROR;
	}

ALERROR CEntry::CompleteLoad (void)

//	CompleteLoad
//
//	Done after an entry is loaded to set the on-the-fly
//	structures.

	{
	ALERROR error;

	if (error = LinkCode())
		return error;

	if (error = CreateRelatedLinksSection())
		return error;

	return NOERROR;
	}

ALERROR CEntry::ComputeRelatedLinks (CStringArray *pLinks)

//	ComputeRelatedLinks
//
//	Fills the array with related links for this entry. Each
//	Link is a <A>...</A> element

	{
	ALERROR error;
	int i;
	CSymbolTable SortedLinks(TRUE, FALSE);

	//	Start with a link for each category that this entry
	//	is in.

	for (i = 0; i < GetCategoryCount(); i++)
		{
		CString sCategory = GetCategory(i);

		if (!sCategory.IsBlank())
			{
			CStringArray SubCategories;

			//	Skip system categories

			if (m_pCore->IsSystemCategory(sCategory))
				continue;

			//	Parse the category into subcategories

			if (error = strDelimit(sCategory, '/', 1, &SubCategories))
				return error;

			CString sURL;
			for (int i = 0; i < SubCategories.GetCount(); i++)
				{
				sURL.Append(SubCategories.GetStringValue(i));

				CString *psLink;
				psLink = new CString(strPatternSubst(LITERAL("<A href=\"core://category/%s\">%s</A>"),
						sURL,
						SubCategories.GetStringValue(i)));
				if (psLink == NULL)
					return ERR_MEMORY;

				//	Add to the table sorted by category name. Note that we
				//	ignore errors because we might get a duplicate entry error
				//	(which is ok to ignore).

				error = SortedLinks.AddEntry(SubCategories.GetStringValue(i), psLink);

				//	Next

				sURL.Append(LITERAL("/"));
				}
			}
		}

	//	Add to the links table

	for (i = 0; i < SortedLinks.GetCount(); i++)
		{
		CString sLink = *(CString *)SortedLinks.GetValue(i);

		if (error = pLinks->AppendString(sLink, NULL))
			return error;
		}

	return NOERROR;
	}

ALERROR CEntry::CreateRelatedLinksSection (void)

//	CreateRelatedLinksSection
//
//	Creates the m_sRelatedLinks section, which is a text
//	field of related links

	{
	ALERROR error;
	CStringArray Links;

	//	Create a list of links.

	if (error = ComputeRelatedLinks(&Links))
		return error;

	if (Links.GetCount() > 0)
		{
		CMemoryWriteStream Output;
		int i, iLen;
		char szBuffer[1024];

		//	Generate the section

		if (error = Output.Create())
			return error;

		//	Output the header

		iLen = wsprintf(szBuffer, "<%s>Related Links<%s>",
				SECTION_TAG,
				BREAK_TAG);
		if (error = Output.Write(szBuffer, iLen, NULL))
			return error;

		//	Put it all inside a list so that we don't get the
		//	Body style

		iLen = wsprintf(szBuffer, "<%s type=\"3\">", LIST_TAG);
		if (error = Output.Write(szBuffer, iLen, NULL))
			return error;

		//	Output the links

		for (i = 0; i < Links.GetCount(); i++)
			{
			CString sLink = Links.GetStringValue(i);

			if (error = Output.Write(sLink.GetPointer(), sLink.GetLength(), NULL))
				return error;

			if (error = Output.Write("<BR>", 4, NULL))
				return error;
			}

		//	Close the list

		iLen = wsprintf(szBuffer, "</%s>", LIST_TAG);
		if (error = Output.Write(szBuffer, iLen, NULL))
			return error;

		//	Close the section

		iLen = wsprintf(szBuffer, "</%s>", SECTION_TAG);
		if (error = Output.Write(szBuffer, iLen, NULL))
			return error;

		//	Done

		if (error = Output.Close())
			return error;

		CString sData(Output.GetPointer(), Output.GetLength());
		m_sRelatedLinks = sData;
		}

	return NOERROR;
	}

BOOL CEntry::FindField (CString sName, CField **retpField)

//	FindField
//
//	Finds the given field by name. The resulting pointer
//	is owned by the entry.

	{
	for (int i = 0; i < GetFieldCount(); i++)
		{
		CField *pField = GetField(i);

		if (strCompare(pField->GetName(), sName) == 0)
			{
			*retpField = pField;
			return TRUE;
			}
		}

	return FALSE;
	}

BOOL CEntry::InCategory (CString sCategory)

//	InCategory
//
//	Returns TRUE if this entry has the given category

	{
	int i;

	for (i = 0; i < m_Categories.GetCount(); i++)
		if (strCompare(sCategory, m_Categories.GetStringValue(i)) == 0)
			return TRUE;

	return FALSE;
	}

ALERROR CEntry::LinkCode (void)

//	LinkCode
//
//	Looks through all the fields of type CODE and links them so that
//	they are ready to be run. This method will initialize the m_CodeList
//	member variable.

	{
	ALERROR error;
	int i;

	for (i = 0; i < m_FieldList.GetCount(); i++)
		{
		CField *pField = GetField(i);

		if (pField->GetType() == CField::Code)
			{
			ICCItem *pCode;
			CString sCode;
			CString sParsedCode;
			CString sError;
			char *pEnd;

			sCode = pField->GetValue(0);

			//	Translate &lt; etc.

			if (error = XMLParseContent(sCode.GetASCIIZPointer(),
					&sParsedCode,
					&pEnd,
					&sError))
				sParsedCode = sError;

			//	Link the codechain

			pCode = m_pCore->GetCC()->Link(sParsedCode, 0, NULL);

			//	Add it to the table

			if (error = m_CodeList.AddEntry(pField->GetName(), pCode))
				return error;
			}
		}

	return NOERROR;
	}

ALERROR CEntry::LoadFromXML (CCore *pCore, CString sData, CEntry **retpEntry, CString *retsError)

//	LoadFromXML
//
//	Load an entry from an XML format

	{
	ALERROR error;
	LOADCTX ctx;
	int iTag;

	//	Prepare a load context

	utlMemSet(&ctx, sizeof(ctx), 0);
	ctx.pPos = sData.GetASCIIZPointer();
	ctx.iLine = 1;
	ctx.pEntry = new CEntry(pCore);
	if (ctx.pEntry == NULL)
		return ERR_MEMORY;

	//	Validate the format

	if (*((DWORD *)ctx.pPos) != XML_ANSI_SIGNATURE)
		{
		*retsError = LITERAL("Invalid or unknown XML format");
		delete ctx.pEntry;
		return ERR_FAIL;
		}

	//	Skip the xml header

	if (error = ParseTag(&ctx, &iTag, NULL))
		{
		*retsError = strPatternSubst(LITERAL("(%d): %s"), ctx.iLine, ctx.sErrorMsg);
		delete ctx.pEntry;
		return ERR_FAIL;
		}

	if (iTag != XML_HEADER_TAGID)
		{
		*retsError = strPatternSubst(LITERAL("(%d): <?XML .. ?> element expected"), ctx.iLine);
		delete ctx.pEntry;
		return ERR_FAIL;
		}

	//	Skip until the end

	while (*ctx.pPos != '>' && *ctx.pPos != '\0')
		ctx.pPos++;

	if (*ctx.pPos != '>')
		{
		*retsError = strPatternSubst(LITERAL("(%d): <?XML .. ?> element expected"), ctx.iLine);
		delete ctx.pEntry;
		return ERR_FAIL;
		}

	ctx.pPos++;

	//	Load the librarylink:entry header

	if (error = ParseLibraryLinkEntry(&ctx))
		{
		*retsError = strPatternSubst(LITERAL("(%d): %s"), ctx.iLine, ctx.sErrorMsg);
		delete ctx.pEntry;
		return ERR_FAIL;
		}

	//	Make sure we link our code

	if (error = ctx.pEntry->CompleteLoad())
		{
		delete ctx.pEntry;
		return error;
		}

	//	Done

	if (ctx.pAttributes)
		delete ctx.pAttributes;
	*retpEntry = ctx.pEntry;

	return NOERROR;
	}

ALERROR CEntry::SaveToXML (CEntry *pEntry, CString *retsData)

//	SaveToXML
//
//	Save an entry to XML format

	{
	ALERROR error;
	CMemoryWriteStream Output;
	char szBuffer[1024];
	int i, iLen;

	if (error = Output.Create())
		return error;

	//	Output the xml header

	iLen = wsprintf(szBuffer, "<?XML version=\"%s\"?>\r\n", XML_VERSION);
	if (error = Output.Write(szBuffer, iLen, NULL))
		return error;

	//	<librarylink:entry>

	if (error = OutputTag(&Output, LIBRARYLINK_ENTRY_TAGID, OTFLAG_NEWLINE))
		return error;

	//	<title> ... </title>

	if (error = OutputTextElement(&Output, TITLE_TAGID, pEntry->m_sTitle, pEntry->m_sTitle))
		return error;

	//	<body> ... </body>

	if (error = OutputTextElement(&Output, BODY_TAGID, pEntry->m_sTitle, pEntry->m_sBody))
		return error;

	//	<categories> ... <BR> ... </categories>

	if (error = OutputTextListElement(&Output, CATEGORIES_TAGID, pEntry->m_sTitle, pEntry->m_Categories))
		return error;

	//	Output entry-defined fields

	for (i = 0; i < pEntry->m_FieldList.GetCount(); i++)
		{
		CField *pField = dynamic_cast<CField *>(pEntry->m_FieldList.GetObject(i));

		if (error = OutputField(&Output, pField))
			return error;
		}

	//	<created-by> ... </created-by>

	if (error = OutputTextElement(&Output, CREATED_BY_TAGID, pEntry->m_sTitle, pEntry->m_sCreatedBy))
		return error;

	//	<created-on> ... </created-on>

	if (error = OutputTimeDateElement(&Output, CREATED_ON_TAGID, pEntry->m_sTitle, pEntry->m_tdCreatedOn))
		return error;

	//	<modified-on> ... </modified-on>

	if (error = OutputTimeDateElement(&Output, MODIFIED_ON_TAGID, pEntry->m_sTitle, pEntry->m_tdModifiedOn))
		return error;

	//	<template> ... </template>

	if (error = OutputTextElement(&Output, TEMPLATE_TAGID, pEntry->m_sTitle, pEntry->m_sTemplate))
		return error;

	//	</librarylink:entry>

	if (error = OutputTag(&Output, LIBRARYLINK_ENTRY_TAGID, OTFLAG_NEWLINE | OTFLAG_CLOSETAG))
		return error;

	//	Done

	if (error = Output.Close())
		return error;

	CString sData(Output.GetPointer(), Output.GetLength());
	*retsData = sData;

	return NOERROR;
	}

void CEntry::SetCreation (void)

//	SetCreation
//
//	Sets the creation and modification dates to now

	{
	SYSTEMTIME time;

	GetSystemTime(&time);
	m_tdCreatedOn = time;
	m_tdModifiedOn = time;
	}

void CEntry::SetModification (void)

//	SetModification
//
//	Sets the modification date to now

	{
	SYSTEMTIME time;

	GetSystemTime(&time);
	m_tdModifiedOn = time;
	}

ALERROR GetAttribute (LOADCTX *pCtx, CString sAttribute, CString *retsValue)

//	GetAttribute
//
//	Gets an attribute

	{
	ALERROR error;
	CObject *pObj;

	if (error = pCtx->pAttributes->Lookup(sAttribute, &pObj))
		{
		pCtx->sErrorMsg = strPatternSubst(LITERAL("unable to find '%s' attribute"), sAttribute);
		return ERR_FAIL;
		}

	*retsValue = *(dynamic_cast<CString *>(pObj));
	return NOERROR;
	}

int LookupSymbol (char **pTable, CString sTag)

//	LookupSymbol
//
//	Returns the index of the symbol in the table (or -1 if not found)

	{
	int i;

	i = 0;
	while (*pTable)
		{
		CString sSymbol(*pTable);

		if (strCompare(sSymbol, sTag) == 0)
			return i;

		i++;
		pTable++;
		}

	return -1;
	}

ALERROR OutputAttribute (CMemoryWriteStream *pOutput, CString sAttribute, CString sValue)

//	OutputAttribute
//
//	Outputs an attribute

	{
	char szBuffer[1024];
	int iLen;

	iLen = wsprintf(szBuffer, " %s=\"%s\"", sAttribute.GetASCIIZPointer(), sValue.GetASCIIZPointer());
	return pOutput->Write(szBuffer, iLen, NULL);
	}

ALERROR OutputFieldElementHeader (CMemoryWriteStream *pOutput, int iTag, CString sFieldName, CString sType)

//	OutputFieldElementHeader
//
//	Outputs either a built-in element or a field

	{
	ALERROR error;

	if (iTag == FIELD_TAGID)
		{
		//	<field ...

		if (error = OutputTag(pOutput, iTag, OTFLAG_INDENT | OTFLAG_ATTRIBUTES))
			return error;

		//	name="..."

		if (error = OutputAttribute(pOutput, LITERAL("name"), sFieldName))
			return error;

		//	type="TEXT"

		if (error = OutputAttribute(pOutput, LITERAL("type"), sType))
			return error;

		//	>

		if (error = pOutput->Write(">", 1, NULL))
			return error;
		}
	else
		{
		if (error = OutputTag(pOutput, iTag, OTFLAG_INDENT))
			return error;
		}

	return NOERROR;
	}

ALERROR OutputField (CMemoryWriteStream *pOutput, CField *pField)

//	OutputField
//
//	Output a field element

	{
	ALERROR error;
	int i;

	if (error = OutputFieldElementHeader(pOutput, FIELD_TAGID, pField->GetName(), LITERAL(g_pszFieldType[(int)pField->GetType()])))
		return error;

	BOOL bNeedSeparator = FALSE;
	for (i = 0; i < pField->GetValueCount(); i++)
		{
		if (bNeedSeparator)
			{
			if (error = OutputTag(pOutput, TEXTBREAK_TAGID, 0))
				return error;
			}

		if (error = OutputText(pOutput, pField->GetValue(i)))
			return error;

		bNeedSeparator = TRUE;
		}

	if (error = OutputTag(pOutput, FIELD_TAGID, OTFLAG_CLOSETAG))
		return error;

	return NOERROR;
	}

ALERROR OutputTag (CMemoryWriteStream *pOutput, int iTag, DWORD dwFlags)

//	OutputTag
//
//	Outputs a tag

	{
	char szBuffer[1024];
	int iLen;

	if (dwFlags & OTFLAG_CLOSETAG)
		iLen = wsprintf(szBuffer, "</%s>\r\n", g_pszTag[iTag]);
	else
		{
		char *pszIndent;
		char *pszNewLine;

		if (dwFlags & OTFLAG_NEWLINE)
			pszNewLine = "\r\n";
		else
			pszNewLine = "";

		if (dwFlags & OTFLAG_INDENT)
			pszIndent = "\t";
		else
			pszIndent = "";

		if (dwFlags & OTFLAG_ATTRIBUTES)
			iLen = wsprintf(szBuffer, "%s<%s%s", pszIndent, g_pszTag[iTag], pszNewLine);
		else
			iLen = wsprintf(szBuffer, "%s<%s>%s", pszIndent, g_pszTag[iTag], pszNewLine);
		}

	//	Done

	return pOutput->Write(szBuffer, iLen, NULL);
	}

ALERROR OutputText (CMemoryWriteStream *pOutput, CString sText)

//	OutputText
//
//	Outputs content text to XML

	{
	ALERROR error;

	if (error = pOutput->Write(sText.GetPointer(), sText.GetLength(), NULL))
		return error;

	return NOERROR;
	}

ALERROR OutputTextElement (CMemoryWriteStream *pOutput, int iTag, CString sFieldName, CString sValue)

//	OutputTextElement
//
//	Outputs either a built-in element or a field

	{
	ALERROR error;

	if (error = OutputFieldElementHeader(pOutput, iTag, sFieldName, LITERAL("TEXT")))
		return error;

	if (error = OutputText(pOutput, sValue))
		return error;

	if (error = OutputTag(pOutput, iTag, OTFLAG_CLOSETAG))
		return error;

	return NOERROR;
	}

ALERROR OutputTextListElement (CMemoryWriteStream *pOutput, int iTag, CString sFieldName, CStringArray &Values)

//	OutputTextElement
//
//	Outputs either a built-in element or a field

	{
	ALERROR error;
	int i;

	if (error = OutputFieldElementHeader(pOutput, iTag, sFieldName, LITERAL("TEXT")))
		return error;

	BOOL bNeedSeparator = FALSE;
	for (i = 0; i < Values.GetCount(); i++)
		{
		if (bNeedSeparator)
			{
			if (error = OutputTag(pOutput, TEXTBREAK_TAGID, 0))
				return error;
			}

		if (error = OutputText(pOutput, Values.GetStringValue(i)))
			return error;

		bNeedSeparator = TRUE;
		}

	if (error = OutputTag(pOutput, iTag, OTFLAG_CLOSETAG))
		return error;

	return NOERROR;
	}

ALERROR OutputTimeDateElement (CMemoryWriteStream *pOutput, int iTag, CString sFieldName, TIMEDATE &tdTimeDate)

//	OutputTimeDateElement
//
//	Outputs either a built-in element or a field

	{
	CString sTimeDate;

	//	Convert to a string

	sTimeDate = TimeDate2String(tdTimeDate);

	//	Output

	return OutputTextElement(pOutput, iTag, sFieldName, sTimeDate);
	}

ALERROR ParseAttributes (LOADCTX *pCtx)

//	ParseAttribute
//
//	Parse the attribute

	{
	ALERROR error;

	//	First we initialize the symbol table

	if (pCtx->pAttributes)
		pCtx->pAttributes->RemoveAll();
	else
		{
		pCtx->pAttributes = new CSymbolTable(TRUE, FALSE);
		if (pCtx->pAttributes == NULL)
			return ERR_MEMORY;
		}

	//	Skip some whitespace

	ParseWhitespace(pCtx);

	//	Keep looping until we hit the end

	while (*pCtx->pPos != '>' && *pCtx->pPos != '\0')
		{
		char *pStart;

		//	Read a name

		pStart = pCtx->pPos;
		while (*pCtx->pPos != '=' && !IsWhitespace(*pCtx->pPos) && *pCtx->pPos != '\0')
			pCtx->pPos++;

		//	If this is the end, we're in trouble

		if (*pCtx->pPos == '\0')
			{
			pCtx->sErrorMsg = LITERAL("expected attribute name");
			return ERR_FAIL;
			}

		//	Get the name

		CString sName(pStart, pCtx->pPos - pStart);

		//	If we don't have an equals, remove whitespace

		if (*pCtx->pPos != '=')
			ParseWhitespace(pCtx);

		//	We better have an equals now

		if (*pCtx->pPos != '=')
			{
			pCtx->sErrorMsg = LITERAL("= expected");
			return ERR_FAIL;
			}

		//	Skip some more whitespace

		pCtx->pPos++;
		ParseWhitespace(pCtx);

		//	Expect an open quote

		if (*pCtx->pPos != '\"')
			{
			pCtx->sErrorMsg = LITERAL("expected attribute value");
			return ERR_FAIL;
			}

		//	Read a value

		pCtx->pPos++;
		pStart = pCtx->pPos;
		while (*pCtx->pPos != '\"' && *pCtx->pPos != '\0')
			pCtx->pPos++;

		//	If this is the end, we're in trouble

		if (*pCtx->pPos == '\0')
			{
			pCtx->sErrorMsg = LITERAL("expected ending quote");
			return ERR_FAIL;
			}

		//	Get the value

		CString *pValue = new CString(pStart, pCtx->pPos - pStart);
		if (pValue == NULL)
			return ERR_MEMORY;

		pCtx->pPos++;

		//	Add the name, value pair to the symbol table

		if (error = pCtx->pAttributes->AddEntry(sName, pValue))
			return error;

		//	Skip some more whitespace

		ParseWhitespace(pCtx);
		}

	//	This better be a closing bracket

	if (*pCtx->pPos != '>')
		{
		pCtx->sErrorMsg = LITERAL("Unexpected end of tag");
		return ERR_FAIL;
		}

	pCtx->pPos++;

	return NOERROR;
	}

ALERROR ParseLibraryLinkEntry (LOADCTX *pCtx)

//	ParseLibraryLinkEntry
//
//	Parse <librarylink:entry>

	{
	ALERROR error;
	int iTag, iCloseTag;
	BOOL bClose;
	CStringArray Values;

	//	Expect the <librarylink:entry> tag

	if (error = ParseTag(pCtx, &iTag, NULL))
		return error;

	if (iTag != LIBRARYLINK_ENTRY_TAGID)
		{
		pCtx->sErrorMsg = LITERAL("<librarylink:entry> element expected");
		return ERR_FAIL;
		}

	//	Load all fields

	if (error = ParseTag(pCtx, &iTag, &bClose))
		return error;

	//	Loop until we get the close tag

	while (iTag != LIBRARYLINK_ENTRY_TAGID)
		{
		//	Empty the value array

		Values.RemoveAll();

		//	Handle the tag

		switch (iTag)
			{
			case TITLE_TAGID:
				if (error = ParseText(pCtx, &Values, 0))
					return error;
				pCtx->pEntry->SetTitle(Values.GetStringValue(0));
				break;

			case BODY_TAGID:
				if (error = ParseText(pCtx, &Values, PTFLAG_NOBREAKS))
					return error;
				pCtx->pEntry->SetBody(Values.GetStringValue(0));
				break;

			case CATEGORIES_TAGID:
				if (error = ParseText(pCtx, &Values, 0))
					return error;
				pCtx->pEntry->SetCategories(Values);
				break;

			case FIELD_TAGID:
				{
				CField *pField;
				CString sFieldName;
				CString sFieldType;
				CField::DataTypes FieldType;
				int iIndex;

				if (error = ParseAttributes(pCtx))
					return error;

				//	Look for the "name" attribute

				if (error = GetAttribute(pCtx, LITERAL("name"), &sFieldName))
					return error;

				//	Look for the "type" attribute

				if (error = GetAttribute(pCtx, LITERAL("type"), &sFieldType))
					return error;

				iIndex = LookupSymbol(g_pszFieldType, sFieldType);
				if (iIndex == -1)
					{
					pCtx->sErrorMsg = LITERAL("invalid field type");
					return ERR_FAIL;
					}

				FieldType = (CField::DataTypes)iIndex;

				if (error = ParseText(pCtx, &Values, 0))
					return error;

				//	Create the field

				pField = new CField;
				if (pField == NULL)
					return ERR_MEMORY;

				pField->SetName(sFieldName);
				pField->SetType(FieldType);
				pField->SetValues(Values);

				//	Add the field to the entry

				if (error = pCtx->pEntry->AppendField(pField))
					return error;

				break;
				}

			case CREATED_BY_TAGID:
				if (error = ParseText(pCtx, &Values, 0))
					return error;
				pCtx->pEntry->SetCreatedBy(Values.GetStringValue(0));
				break;

			case CREATED_ON_TAGID:
				{
				TIMEDATE td;
				if (error = ParseText(pCtx, &Values, 0))
					return error;
				String2TimeDate(Values.GetStringValue(0), &td);
				pCtx->pEntry->SetCreatedOn(td);
				break;
				}

			case MODIFIED_ON_TAGID:
				{
				TIMEDATE td;
				if (error = ParseText(pCtx, &Values, 0))
					return error;
				String2TimeDate(Values.GetStringValue(0), &td);
				pCtx->pEntry->SetModifiedOn(td);
				break;
				}

			case TEMPLATE_TAGID:
				if (error = ParseText(pCtx, &Values, 0))
					return error;
				pCtx->pEntry->SetTemplate(Values.GetStringValue(0));
				break;

			default:
				pCtx->sErrorMsg = LITERAL("Unknown tag");
				return ERR_FAIL;
			}

		//	Parse the close tag

		if (error = ParseTag(pCtx, &iCloseTag, &bClose))
			return error;

		//	Validate

		if (iCloseTag != iTag || !bClose)
			{
			char szBuffer[256];
			wsprintf(szBuffer, "expected matching close tag for <%s>", g_pszTag[iTag]);
			pCtx->sErrorMsg = CString(szBuffer);
			return ERR_FAIL;
			}

		//	Next field

		if (error = ParseTag(pCtx, &iTag, &bClose))
			return error;
		}

	//	Make sure this is a close

	if (!bClose)
		{
		pCtx->sErrorMsg = LITERAL("</librarylink:entry> expected");
		return ERR_FAIL;
		}

	return NOERROR;
	}

ALERROR ParseTag (LOADCTX *pCtx, int *retiTag, BOOL *retbClose)

//	ParseTag
//
//	Parses an open or close tag

	{
	char *pPos;
	BOOL bClose = FALSE;

	ParseWhitespace(pCtx);

	if (*pCtx->pPos != '<')
		{
		pCtx->sErrorMsg = LITERAL("< expected");
		return ERR_FAIL;
		}

	pCtx->pPos++;

	//	Is this a close tag?

	if (*pCtx->pPos == '/')
		{
		bClose = TRUE;
		pCtx->pPos++;
		}

	//	Get the name

	pPos = pCtx->pPos;
	while (!IsWhitespace(*pPos) && *pPos != '>' && *pPos != '\0')
		pPos++;

	CString sTag(pCtx->pPos, pPos - pCtx->pPos);

	//	Lookup the tag

	*retiTag = LookupSymbol(g_pszTag, sTag);

	//	Skip to the end of the tag

	pCtx->pPos = pPos;

	//	If we've got a close tag, skip that too

	if (*pCtx->pPos == '>')
		pCtx->pPos++;

	if (retbClose)
		*retbClose = bClose;

	return NOERROR;
	}

ALERROR ParseText (LOADCTX *pCtx, CStringArray *pValues, DWORD dwFlags)

//	ParseText
//
//	Parse text

	{
	ALERROR error;
	BOOL bDone = FALSE;
	char *pStartPos;
	int iNestedTag;

	pStartPos = pCtx->pPos;
	iNestedTag = 1;
	while (!bDone)
		{
		char *pTag;

		//	Keep looping until we find the end of this value

		while (*pCtx->pPos != '<' && pCtx->pPos != '\0')
			{
			if (*pCtx->pPos == '\n')
				pCtx->iLine++;
			pCtx->pPos++;
			}

		//	We better not hit the end

		if (*pCtx->pPos == '\0')
			{
			pCtx->sErrorMsg = LITERAL("unexpected end of stream");
			return ERR_FAIL;
			}

		//	We need to see if we should stop parsing here. There are three
		//	possibilities:
		//	
		//	1.	The last </> tag means that we should stop parsing
		//	2.	If we have a <BR> and we're honoring breaks, we should go to the next line
		//	3.	Anything else we should pass through, making sure that we count open
		//		and close tags so that we know when we're done.

		//	Check to see if this is a closing tag

		pTag = pCtx->pPos + 1;
		if (*pTag == '/')
			{
			iNestedTag--;

			//	If we're done, bail out; otherwise continue parsing

			if (iNestedTag == 0)
				{
				CString sValue(pStartPos, pCtx->pPos - pStartPos);
				if (error = pValues->AppendString(sValue, NULL))
					return error;

				bDone = TRUE;
				}
			else
				pCtx->pPos++;
			}

		//	Check to see if this is a break

		else if ((*pTag == 'b' || *pTag == 'B')
					&& (*(pTag+1) == 'r' || *(pTag+1) == 'R')
					&& (*(pTag+2) == '>'))
			{
			//	If we are ignoring breaks, simply allow this tag to
			//	be passed-on

			if (dwFlags & PTFLAG_NOBREAKS)
				{
				pCtx->pPos++;
				}

			//	Otherwise, start a new item

			else
				{
				CString sValue(pStartPos, pCtx->pPos - pStartPos);
				if (error = pValues->AppendString(sValue, NULL))
					return error;

				pCtx->pPos += 4;
				pStartPos = pCtx->pPos;
				}
			}

		//	Otherwise this is a nested tag

		else
			{
			//	Check to see if this is an empty tag

			while (*pTag != '>' && *pTag != '\0')
				pTag++;

			//	If this tag is not an empty element (e.g., <tag/>) then
			//	we up our nesting count so that we expect a closing tag

			if (*pTag != '>' || *(pTag-1) != '/')
				iNestedTag++;

			pCtx->pPos++;
			}
		}

	return NOERROR;
	}

void ParseWhitespace (LOADCTX *pCtx)

//	ParseWhitespace
//
//	Skips whitespace

	{
	while (IsWhitespace(*pCtx->pPos))
		{
		if (*pCtx->pPos == '\n')
			pCtx->iLine++;

		pCtx->pPos++;
		}
	}

void String2TimeDate (CString sTimeDate, TIMEDATE *rettd)

//	String2TimeDate
//
//	Convert a string to a timedate

	{
	TIMEDATE td;
	char *pPos;

	utlMemSet(&td, sizeof(td), 0);
	pPos = sTimeDate.GetASCIIZPointer();

	td.wYear = strParseInt(pPos, 1998, &pPos, NULL);
	pPos++;		//	skip dash
	td.wMonth = strParseInt(pPos, 1, &pPos, NULL);
	pPos++;		//	skip dash
	td.wDay = strParseInt(pPos, 1, &pPos, NULL);

	//	See if we have a time

	if (*pPos)
		{
		td.wHour = strParseInt(pPos, 0, &pPos, NULL);
		pPos++;		//	skip colon
		td.wMinute = strParseInt(pPos, 0, &pPos, NULL);
		pPos++;		//	skip colon
		td.wSecond = strParseInt(pPos, 0, &pPos, NULL);
		pPos++;		//	skip colon
		td.wMilliseconds = strParseInt(pPos, 0, &pPos, NULL);
		}

	*rettd = td;
	}

CString TimeDate2String (TIMEDATE &tdTimeDate)

//	TimeDate2String
//
//	Convert a timedate to a string

	{
	char szBuffer[256];
	int iLen;

	iLen = wsprintf(szBuffer, "%04d-%02d-%02d %02d:%02d:%02d.%04d",
			tdTimeDate.wYear,
			tdTimeDate.wMonth,
			tdTimeDate.wDay,
			tdTimeDate.wHour,
			tdTimeDate.wMinute,
			tdTimeDate.wSecond,
			tdTimeDate.wMilliseconds);

	return CString(szBuffer, iLen);
	}
