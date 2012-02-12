//	RunLengthCompression.cpp
//
//	Run-Length Compression functions

#include "Kernel.h"

const int MIN_LONG_RUN =						3;
const int MAX_RUN_LENGTH_BYTE =					254;
const BYTE RUN_CODE_MIXED =						255;
const BYTE RUN_CODE_END =						0;

void CompressRunLengthByte (IWriteStream *pOutput, IReadBlock *pInput)

//	CompressRunLengthByte
//
//	Compress the input block into a run-length encoded stream (by BYTE)
//
//	Header:
//
//	DWORD			size of uncompressed block
//
//	Runs:
//
//	BYTE			length (1-254)
//	BYTE			value
//
//	BYTE			255
//	BYTE			length (1-254)
//	{bytes}
//
//	BYTE			0 (end-code)

	{
	enum RunTypes
		{
		typeUnknown,
		typeMixed,
		typeLong,
		};

	BYTE byCode;
	BYTE byValue;

	//	Write the length of the uncompressed block

	DWORD dwLen = pInput->GetLength();
	pOutput->Write((char *)&dwLen, sizeof(DWORD));

	//	Process the block

	char *pPos = pInput->GetPointer(0, dwLen);
	char *pEndPos = pPos + dwLen;

	RunTypes iRunType = typeMixed;
	char *pStart = pPos;
	char *pMixedEnd = pPos;

	while (pPos < pEndPos)
		{
		//	Advance the position

		pPos++;

		//	We can advance pMixedEnd if this byte is different.
		//	Or if we've reached the end of the buffer and we don't
		//	have enough to create an independent run, then advance
		//	pMixedEnd

		if ((*pPos != *pMixedEnd)
				|| (pPos == pEndPos && (pPos - pMixedEnd) < MIN_LONG_RUN))
			{
			if (pPos <=	pStart + MAX_RUN_LENGTH_BYTE)
				pMixedEnd = pPos;
			else
				pMixedEnd = pStart + MAX_RUN_LENGTH_BYTE;
			}

		//	See if we've reached the end of the run

		if (iRunType == typeMixed)
			{
			if (pPos == pEndPos 
					|| (pPos - pMixedEnd) == MIN_LONG_RUN 
					|| (pMixedEnd - pStart) == MAX_RUN_LENGTH_BYTE)
				{
				int iRunLen = (pMixedEnd - pStart);

				byValue = RUN_CODE_MIXED;
				pOutput->Write((char *)&byValue, 1);

				byValue = (BYTE)iRunLen;
				pOutput->Write((char *)&byValue, 1);

				pOutput->Write(pStart, iRunLen);

				//	Set up the next run

				iRunType = typeMixed;
				pPos = pMixedEnd;
				pStart = pPos;
				}
			}
		else if (iRunType == typeLong)
			{
			if (pPos == pEndPos
					|| (*pPos != *pStart)
					|| (pPos - pStart) == MAX_RUN_LENGTH_BYTE)
				{
				int iRunLen = (pPos - pStart);

				byValue = (BYTE)iRunLen;
				pOutput->Write((char *)&byValue, 1);

				byValue = (BYTE)(*pStart);
				pOutput->Write((char *)&byValue, 1);

				iRunType = typeMixed;
				pStart = pPos;
				pMixedEnd = pPos;
				}
			}
		}

	//	Done

	byCode = RUN_CODE_END;
	pOutput->Write((char *)&byCode, 1);
	}

void UncompressRunLengthByte (IWriteStream *pOutput, IReadBlock *pInput)

//	UncompressRunLengthByte
//
//	Expands the input block

	{
	char *pPos = pInput->GetPointer(0, pInput->GetLength());

	//	Read the total size of the uncompressed buffer

	DWORD dwBufferSize = *(DWORD *)pPos;
	pPos += sizeof(DWORD);

	//	Loop over all the runs

	BYTE byCode = (BYTE)*pPos;
	while (byCode != RUN_CODE_END)
		{
		//	Mixed run

		if (byCode == RUN_CODE_MIXED)
			{
			pPos++;

			int iLen = (BYTE)*pPos++;
			pOutput->Write(pPos, iLen);

			pPos += iLen;
			}
		
		//	Otherwise, it is a long run and the byte code is the 
		//	length of the run.

		else
			{
			int iLen = (BYTE)*pPos++;
			pOutput->WriteChar(*pPos++, iLen);
			}
		}
	}
