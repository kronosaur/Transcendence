//	CResourceReadBlock.cpp
//
//	Implements CResourceReadBlock object

#include "Kernel.h"
#include "KernelObjID.h"

static CObjectClass<CResourceReadBlock>g_Class(OBJID_CRESOURCEREADBLOCK, NULL);

CResourceReadBlock::CResourceReadBlock (void) :
		CObject(&g_Class)

//	CResourceReadBlock constructor

	{
	}

CResourceReadBlock::CResourceReadBlock (HMODULE hModule, char *pszRes, char *pszType) :
		CObject(&g_Class),
		m_hModule(hModule),
		m_pszRes(pszRes),
		m_pszType(pszType),
		m_pData(NULL)

//	CResourceReadBlock constructor

	{
	}

CResourceReadBlock::~CResourceReadBlock (void)

//	CResourceReadBlock destructor

	{
	//	Close the file if necessary

	Close();
	}

ALERROR CResourceReadBlock::Close (void)

//	CResourceReadBlock
//
//	Close the stream

	{
	return NOERROR;
	}

ALERROR CResourceReadBlock::Open (void)

//	Open
//
//	Opens the stream for reading

	{
	HRSRC hRsrc;
	HGLOBAL hGlobal;

	hRsrc = FindResource(m_hModule, m_pszRes,  m_pszType);
	if (hRsrc == NULL)
		return ERR_FAIL;

	//	Get the length

	m_dwLength = SizeofResource(m_hModule, hRsrc);

	//	Load it

	hGlobal = LoadResource(m_hModule, hRsrc);
	if (hGlobal == NULL)
		return ERR_FAIL;

	m_pData = (char *)LockResource(hGlobal);
	if (m_pData == NULL)
		return ERR_FAIL;

	return NOERROR;
	}
