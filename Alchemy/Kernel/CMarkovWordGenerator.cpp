//	CMarkovWordGenerator.cpp
//
//	CMarkovWordGenerator class

#include "Kernel.h"
#include "KernelObjID.h"

#include "Lingua.h"

CMarkovWordGenerator::~CMarkovWordGenerator (void)

//	CMarkovWordGenerator destructor

	{
	int i;

	for (i = 0; i < m_Table.GetCount(); i++)
		delete m_Table[i];
	}

void CMarkovWordGenerator::AddChain (char *pFrag, char chChar, DWORD dwFlags)

//	AddChain
//
//	Adds a fragment to character mapping.

	{
	//	Reset counter to indicate that we need to re-initialize
	//	the start fragment table

	m_dwStartCount = 0;

	//	Look for the fragment. If we don't find it,
	//	we can add a new one.

	int iPos;
	if (!Find(pFrag, &iPos))
		{
		m_Table.Insert(AllocFrag(pFrag, chChar, dwFlags), iPos);
		return;
		}

	//	Increment the fragment count

	m_Table[iPos]->dwCount++;

	//	Look for the character chain. If we don't find it,
	//	then we add it.

	SChainChar *pChain;
	if (!FindChainChar(m_Table[iPos], chChar, &pChain))
		{
		m_Table[iPos] = AppendChainChar(m_Table[iPos], chChar, dwFlags);
		return;
		}

	//	Increment the char count

	pChain->dwCount++;
	}

void CMarkovWordGenerator::AddSample (const CString &sWord)

//	AddSample
//
//	Adds an example word for the generator to assimilate

	{
	char sFrag[3];
	char *pPos = sWord.GetASCIIZPointer();

	//	Handle words shorter than 3 characters.

	if (sWord.IsBlank())
		;
	else if (sWord.GetLength() == 1)
		{
		sFrag[0] = pPos[0];
		sFrag[1] = '\0';
		sFrag[2] = '\0';
		AddChain(sFrag, '\0', FRAG_WORD_START);
		}
	else if (sWord.GetLength() == 2)
		AddChain(pPos, '\0', FRAG_WORD_START);
	else
		{
		DWORD dwFlags = FRAG_WORD_START;
		char *pPosEnd = pPos + 3;
		while (true)
			{
			AddChain(pPos, *pPosEnd, dwFlags);

			if (*pPosEnd == '\0')
				break;
			else
				{
				pPos++;
				pPosEnd++;
				}

			//	No longer start of word

			dwFlags &= ~FRAG_WORD_START;
			}
		}
	}

CMarkovWordGenerator::SFragHead *CMarkovWordGenerator::AllocFrag (char *pFrag, char chChar, DWORD dwFlags)

//	AllocFrag
//
//	Allocates a new fragment

	{
	SFragHead *pNewFrag = (SFragHead *) new char [sizeof(SFragHead) + 2 * sizeof(SChainChar)];
	pNewFrag->sFrag[0] = pFrag[0];
	pNewFrag->sFrag[1] = pFrag[1];
	pNewFrag->sFrag[2] = pFrag[2];
	pNewFrag->sFrag[3] = '\0';
	pNewFrag->dwFlags = dwFlags;
	pNewFrag->dwCount = 1;

	SChainChar *pChain = GetChain(pNewFrag);
	pChain->chChar = chChar;
	pChain->dwCount = 1;

	pChain++;
	pChain->chChar = '\0';
	pChain->dwCount = 0;

	return pNewFrag;
	}

CMarkovWordGenerator::SFragHead *CMarkovWordGenerator::AppendChainChar (SFragHead *pFrag, char chChar, DWORD dwFlags)

//	AppendChainChar
//
//	Appends the character to the chain; frees the original fragment and
//	returns a newly allocated one.

	{
	DWORD *pChainStart = (DWORD *)GetChain(pFrag);
	DWORD *pChain = pChainStart;
	while (*pChain != 0)
		pChain++;

	int iChainCount = (pChain - pChainStart) + 1;

	//	Allocate the new fragment

	SFragHead *pNewFrag = (SFragHead *) new char [sizeof(SFragHead) + (iChainCount + 1) * sizeof(SChainChar)];
	*pNewFrag = *pFrag;
	if (dwFlags & FRAG_WORD_START)
		pNewFrag->dwFlags |= FRAG_WORD_START;

	//	Copy all but the last

	DWORD *pChainSrc = pChainStart;
	DWORD *pChainSrcEnd = pChainSrc + (iChainCount - 1);
	DWORD *pChainDest = (DWORD *)GetChain(pNewFrag);
	while (pChainSrc < pChainSrcEnd)
		*pChainDest++ = *pChainSrc++;

	//	Add the new char

	((SChainChar *)pChainDest)->chChar = chChar;
	((SChainChar *)pChainDest)->dwCount = 1;
	pChainDest++;

	//	Terminator

	*pChainDest = 0;

	//	Free original and return

	delete pFrag;
	return pNewFrag;
	}

bool CMarkovWordGenerator::Find (char *pFrag, int *retiPos)

//	Find
//
//	Returns the index of the given fragment in m_Table.
//	If the fragment is already in the table, we return TRUE.
//	Otherwise, we return FALSE and the return position is
//	the insertion pos.

	{
	int iTop = 0;
	int iBottom = m_Table.GetCount();

	int iPos = iTop + (iBottom - iTop) / 2;
	while (iPos != iBottom)
		{
		char chSrc = pFrag[0];
		char chTable = m_Table[iPos]->sFrag[0];
		if (chSrc < chTable)
			iBottom = iPos;
		else if (chSrc > chTable)
			iTop = iPos + 1;
		else
			{
			char chSrc = pFrag[1];
			char chTable = m_Table[iPos]->sFrag[1];
			if (chSrc < chTable)
				iBottom = iPos;
			else if (chSrc > chTable)
				iTop = iPos + 1;
			else
				{
				char chSrc = pFrag[2];
				char chTable = m_Table[iPos]->sFrag[2];
				if (chSrc < chTable)
					iBottom = iPos;
				else if (chSrc > chTable)
					iTop = iPos + 1;
				else
					{
					//	Found!
					*retiPos = iPos;
					return true;
					}
				}
			}

		iPos = iTop + (iBottom - iTop) / 2;
		}

	//	Not found

	*retiPos = (iPos < m_Table.GetCount() ? iPos : -1);
	return false;
	}

bool CMarkovWordGenerator::FindChainChar (SFragHead *pFrag, char chChar, SChainChar **retpChain)

//	FindChainChar
//
//	Looks for the char in the chain and returns the SChainChar entry.

	{
	SChainChar *pChain = GetChain(pFrag);
	while ((*(DWORD *)pChain) != 0)
		{
		if (pChain->chChar == chChar)
			{
			*retpChain = pChain;
			return true;
			}

		pChain++;
		}

	return false;
	}

CString CMarkovWordGenerator::Generate (void)

//	Generate
//
//	Generates a random word

	{
	int i;

	if (!InitStart())
		return NULL_STR;

	//	Allocate a result buffer

	char szBuffer[1024];
	int iLength = 0;

	//	Start by picking a random fragment

	DWORD dwRoll = (DWORD)mathRandom(1, m_dwStartCount);
	for (i = 0; i < m_Start.GetCount() && dwRoll > m_Start[i]->dwCount; i++)
		dwRoll -= m_Start[i]->dwCount;

	SFragHead *pFrag = m_Start[i];
	szBuffer[0] = pFrag->sFrag[0];
	szBuffer[1] = pFrag->sFrag[1];
	szBuffer[2] = pFrag->sFrag[2];
	char *pPosFrag = szBuffer;
	char *pPos = szBuffer + 3;
	char *pPosEnd = szBuffer + sizeof(szBuffer);

	while (true)
		{
		//	Bounds check

		if (pPos + 1 == pPosEnd)
			{
			*pPos = '\0';
			return CString(szBuffer);
			}

		//	Pick a random character for this fragment

		dwRoll = (DWORD)mathRandom(1, pFrag->dwCount);
		SChainChar *pChain = GetChain(pFrag);
		while ((*(DWORD *)pChain) != 0)
			{
			if (dwRoll <= pChain->dwCount)
				break;

			dwRoll -= pChain->dwCount;
			pChain++;
			}

		//	Are we done?

		if (pChain->chChar == '\0')
			{
			*pPos = '\0';
			return CString(szBuffer);
			}

		//	Add the character

		*pPos++ = pChain->chChar;
		pPosFrag++;

		//	Look for the new fragment

		int iPos;
		if (Find(pPosFrag, &iPos))
			pFrag = m_Table[iPos];
		else
			{
			*pPos = '\0';
			return CString(szBuffer);
			}
		}

	return NULL_STR;
	}

int CMarkovWordGenerator::GenerateUnique (int iCount, TArray<CString> *retArray)

//	GenerateUnique
//
//	Generates an array of unique words

	{
	int i;
	TMap<CString, DWORD> Generated;

	for (i = 0; i < iCount; i++)
		{
		int iTriesLeft = 500;

		while (iTriesLeft > 0)
			{
			//	Generate a random word

			CString sWord = Generate();

			//	Lookup the word in our map. If we found it,
			//	try again.

			if (Generated.Find(sWord))
				{
				iTriesLeft--;
				continue;
				}

			//	If it is unique, add it

			Generated.Insert(sWord, 1);
			break;
			}

		//	If we couldn't find a unique word, then quit

		if (iTriesLeft == 0)
			break;
		}

	//	Add the entries that we generated to the output array

	CMapIterator j;
	Generated.Reset(j);
	int iGeneratedCount = 0;
	while (Generated.HasMore(j))
		{
		DWORD *pDummy;
		CString sWord = Generated.GetNext(j, &pDummy);
		retArray->Insert(sWord);
		iGeneratedCount++;
		}

	return iGeneratedCount;
	}

bool CMarkovWordGenerator::InitStart (void)

//	InitStart
//
//	Initializes the list of all starting fragments. Returns TRUE
//	if the table is not empty.

	{
	int i;

	if (m_dwStartCount == 0)
		{
		m_Start.DeleteAll();

		for (i = 0; i < m_Table.GetCount(); i++)
			if (m_Table[i]->dwFlags & FRAG_WORD_START)
				{
				m_Start.Insert(m_Table[i]);
				m_dwStartCount += m_Table[i]->dwCount;
				}
		}

	return (m_dwStartCount != 0);
	}

ALERROR CMarkovWordGenerator::WriteAsXML (IWriteStream *pOutput)

//	WriteAsXML
//
//	Writes out the Markov chain data to an XML element

	{
	ALERROR error;
	int i;

	//	Open tag

	CString sData;
	sData = CONSTLIT("\t<WordGenerator>\r\n");
	if (error = pOutput->Write(sData.GetPointer(), sData.GetLength(), NULL))
		return error;

	//	Fragments

	for (i = 0; i < m_Table.GetCount(); i++)
		{
		sData = CONSTLIT("\t\t<Syl>");
		if (error = pOutput->Write(sData.GetPointer(), sData.GetLength(), NULL))
			return error;

		sData = strPatternSubst(CONSTLIT("%s;%d;%d;"), strToXMLText(CString(m_Table[i]->sFrag)), m_Table[i]->dwCount, m_Table[i]->dwFlags);
		if (error = pOutput->Write(sData.GetPointer(), sData.GetLength(), NULL))
			return error;

		SChainChar *pChain = GetChain(m_Table[i]);
		while ((*(DWORD *)pChain) != 0)
			{
			char chChar[2];
			chChar[0] = pChain->chChar;
			chChar[1] = '\0';
			CString sChar = strToXMLText(CString(chChar, 1, true));
			sData = strPatternSubst(CONSTLIT("%s;%d;"), sChar, pChain->dwCount);

			if (error = pOutput->Write(sData.GetPointer(), sData.GetLength(), NULL))
				return error;

			pChain++;
			}

		sData = CONSTLIT("</Syl>\r\n");
		if (error = pOutput->Write(sData.GetPointer(), sData.GetLength(), NULL))
			return error;
		}

	//	Done

	//	Close tag

	sData = CONSTLIT("\t</WordGenerator>\r\n");
	if (error = pOutput->Write(sData.GetPointer(), sData.GetLength(), NULL))
		return error;

	return NOERROR;
	}
