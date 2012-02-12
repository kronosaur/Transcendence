//	CAtomTable.cpp
//
//	Implementation of an atom table

#include "Kernel.h"
#include "KernelObjID.h"

static DATADESCSTRUCT g_DataDesc[] =
	{	{ DATADESC_OPCODE_INT,			1,	0 },		//	m_iHashSize
		{ DATADESC_OPCODE_INT,			1,	0 },		//	m_iNextAtom
		{ DATADESC_OPCODE_ALLOC_OBJ,	1,	0 },		//	m_pBackbone
		{ DATADESC_OPCODE_STOP,	0,	0 } };
static CObjectClass<CAtomTable>g_ClassData(OBJID_CATOMTABLE, g_DataDesc);

CAtomTable::CAtomTable (void) : CObject(&g_ClassData),
		m_iHashSize(128),
		m_iNextAtom(1),
		m_pBackbone(NULL)

//	CAtomTable constructor

	{
	}

CAtomTable::CAtomTable (int iHashSize) : CObject(&g_ClassData),
		m_iHashSize(iHashSize),
		m_iNextAtom(1),
		m_pBackbone(NULL)

//	CAtomTable constructor

	{
	}

CAtomTable::~CAtomTable (void)

//	CAtomTable destructor

	{
	if (m_pBackbone)
		delete [] m_pBackbone;
	}

ALERROR CAtomTable::AppendAtom (const CString &sString, int *retiAtom)

//	AppendAtom
//
//	Add an atom to the atom table

	{
	ALERROR error;
	CSymbolTable *pTable;

	//	If we have not yet allocated the backbone, do it now

	if (m_pBackbone == NULL)
		{
		m_pBackbone = new CSymbolTable[m_iHashSize];
		if (m_pBackbone == NULL)
			return ERR_MEMORY;
		}

	//	Get the symbol table

	pTable = Hash(sString);

	//	Lookup the value in the symbol table

	if (error = pTable->AddEntry(sString, (CObject *)m_iNextAtom))
		return error;

	//	Next atom

	m_iNextAtom++;

	return NOERROR;
	}

int CAtomTable::Atomize (const CString &sString)

//	Atomize
//
//	Return the atom associated with the string. If no atom
//	is found, returns -1

	{
	CSymbolTable *pTable;
	int iAtom;

	if (m_pBackbone == NULL)
		return -1;

	//	Get the symbol table

	pTable = Hash(sString);

	//	Do a look up

	if (pTable->Lookup(sString, (CObject **)&iAtom) != NOERROR)
		return -1;

	return iAtom;
	}

CSymbolTable *CAtomTable::Hash (const CString &sString)

//	Hash
//
//	Returns the symbol table associated with this hash bucket

	{
	int iHash;
	char *pPos;

	ASSERT(m_pBackbone);

	//	Compute the hash value

	pPos = sString.GetASCIIZPointer();
	iHash = 0;
	while (*pPos != '\0')
		{
		iHash = 131 * iHash + (int)(DWORD)(*pPos);
		pPos++;
		}

	//	Return the table

	return &m_pBackbone[iHash % m_iHashSize];
	}
