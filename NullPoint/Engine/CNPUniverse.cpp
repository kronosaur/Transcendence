//	CNPUniverse.cpp
//
//	Implements CNPUniverse class

#include "Alchemy.h"
#include "NPEngine.h"

CNPUniverse::CNPUniverse (void) : CObject(NULL)
	{
	}

ALERROR CNPUniverse::CreateUniverse (const CString &sTemplate,
									 CNPUniverse **retpUniv,
									 CString *retsError)

//	CreateUniverse
//
//	Creates the universe

	{
	ALERROR error;
	CNPUniverse *pUniv = NULL;
	CXMLElement *pTemplate = NULL;
	CXMLElement *pSymbols;
	CSymbolTable Symbols(FALSE, TRUE);

	//	Load the template

	CFileReadBlock DataFile(sTemplate);
	if (error = CXMLElement::ParseXML(&DataFile, &pTemplate, retsError))
		goto Fail;

	//	Load the symbol table

	pSymbols = pTemplate->GetContentElementByTag(CONSTLIT("DefineSymbols"));
	if (pSymbols)
		{
		for (int i = 0; i < pSymbols->GetContentElementCount(); i++)
			{
			CXMLElement *pDefine = pSymbols->GetContentElement(i);
			CString sDefine = pDefine->GetAttribute(CONSTLIT("Name"));
			int iValue = pDefine->GetAttributeInteger(CONSTLIT("Value"));

			if (error = Symbols.AddEntry(sDefine, (CObject *)iValue))
				{
				if (error == ERR_FAIL)
					*retsError = LITERAL("Duplicate symbol definition");
				else
					*retsError = LITERAL("Out of memory");
				goto Fail;
				}
			}
		}

	//	Allocate the structure

	pUniv = new CNPUniverse;
	if (pUniv == NULL)
		{
		*retsError = LITERAL("Out of memory");
		error = ERR_MEMORY;
		goto Fail;
		}

	//	Load classes

	if (error = pUniv->LoadClassesFromXML(pTemplate, Symbols, retsError))
		goto Fail;

	//	Init

	if (error = pUniv->InitFromXML(pTemplate, Symbols, retsError))
		goto Fail;

	//	Done

	delete pTemplate;
	*retpUniv = pUniv;

	return NOERROR;

Fail:

	if (pUniv)
		delete pUniv;

	if (pTemplate)
		delete pTemplate;

	return error;
	}

ALERROR CNPUniverse::EnterHyperspace (CNPFleet *pFleet, CNPNullPoint *pDest)

//	EnterHyperspace
//
//	Fleet enter hyperspace

	{
	int iHyperdrive = pFleet->GetHyperdrive(this);
	if (iHyperdrive == 0)
		return NPERROR_FLEET_NO_HYPERDRIVE;

	pFleet->SetDest(pDest);
	pFleet->SetInTransit(iHyperdrive);

	return NOERROR;
	}

CString CNPUniverse::GetStardate (void)

//	GetStardate
//
//	Returns a string of the form (AAAA.DD oOO)

	{
	char szBuffer[256];
	int iAnno = m_iBaseAnno + (m_iTurn / DiasPerAnno * OrosPerDia);
	int iDia = (m_iTurn / OrosPerDia) % DiasPerAnno;
	int iOro = (m_iTurn % OrosPerDia) + 1;
	int iLen = wsprintf(szBuffer, "%d.%02d o%d", iAnno, iDia, iOro);
	return CString(szBuffer, iLen);
	}

void CNPUniverse::ScanNullPoint (CNPSovereign *pSovereign, CNPNullPoint *pNP, int iScannerLevel)

//	ScanNullPoint
//
//	Scans the given null point

	{
	int i;

	//	Null point is known

	pSovereign->SetKnowledge(pNP->GetUNID());

	//	Loop over all worlds at the null point and scan them

	for (i = 0; i < pNP->GetWorldCount(); i++)
		{
		CNPWorld *pWorld = pNP->GetWorld(i);
		pSovereign->SetKnowledge(pWorld->GetUNID());
		}
	}
