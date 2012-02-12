//	Base64.cpp
//
//	Base64 encoding/decoding

#include "Kernel.h"

#include "Internets.h"

BYTE g_Table64[64] =
	{	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
		'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
		'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
		'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
		'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
		'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
		'w', 'x', 'y', 'z', '0', '1', '2', '3',
		'4', '5', '6', '7', '8', '9', '+', '/' };

CBase64Decoder::CBase64Decoder (IReadStream *pInput, DWORD dwFlags) :
		m_pStream(pInput),
		m_iBufferPos(3)

//	CBase64Decoder constructor

	{
	}

BYTE CBase64Decoder::CharToByte (char chChar)

//	CharToByte
//
//	Converts from a character to a base64 number

	{
	if (chChar >= 'A' && chChar <= 'Z')
		return (chChar - 'A');
	else if (chChar >= 'a' && chChar <= 'z')
		return 26 + (chChar - 'a');
	else if (chChar >= '0' && chChar <= '9')
		return 52 + (chChar - '0');
	else if (chChar == '+')
		return 62;
	else if (chChar == '/')
		return 63;
	else
		return 0xff;
	}

ALERROR CBase64Decoder::Read (char *pData, int iLength, int *retiBytesRead)

//	Read
//
//	Read the buffer

	{
	BYTE *pOutput = (BYTE *)pData;
	BYTE *pOutputEnd = pOutput + iLength;

	//	Keep reading from the stream

	while (pOutput < pOutputEnd)
		{
		//	Read 4 characters and decode them into 3 bytes

		if (m_iBufferPos == 3)
			{
			char pInputChar[4];
			int iRead;
			if (m_pStream->Read(pInputChar, 4, &iRead) != NOERROR || iRead != 4)
				return ERR_FAIL;

			BYTE pInput[4];
			pInput[0] = CharToByte(pInputChar[0]);
			pInput[1] = CharToByte(pInputChar[1]);
			pInput[2] = CharToByte(pInputChar[2]);
			pInput[3] = CharToByte(pInputChar[3]);

			//	Convert to 3 bytes of binary

			m_chBuffer[0] = (pInput[0] << 2) | (pInput[1] >> 4);
			m_chBuffer[1] = (pInput[1] << 4) | (pInput[2] >> 2);
			m_chBuffer[2] = (pInput[2] << 6) | pInput[3];

			m_iBufferPos = 0;
			}

		//	Read from buffer

		*pOutput++ = m_chBuffer[m_iBufferPos++];
		}

	//	Done

	if (retiBytesRead)
		*retiBytesRead = iLength;

	return NOERROR;
	}

CBase64Encoder::CBase64Encoder (IWriteStream *pOutput, DWORD dwFlags) : 
		m_pStream(pOutput),
		m_iBufferLen(0)

//	CBase64Encoder constructor

	{
	}

ALERROR CBase64Encoder::Close (void)

//	Close
//
//	Closes the stream. Note that the filter will not write out the last few
//	bytes until Close is called.

	{
	//	If we've got data in the buffer then we need to write it out

	if (m_iBufferLen == 1)
		{
		BYTE pOutput[4];

		pOutput[0] = g_Table64[m_chBuffer[0] >> 2];
		pOutput[1] = g_Table64[((m_chBuffer[0] & 0x03) << 4)];
		pOutput[2] = '=';
		pOutput[3] = '=';

		m_pStream->Write((char *)pOutput, 4);

		m_iBufferLen = 0;
		}
	else if (m_iBufferLen == 2)
		{
		BYTE pOutput[4];

		pOutput[0] = g_Table64[m_chBuffer[0] >> 2];
		pOutput[1] = g_Table64[((m_chBuffer[0] & 0x03) << 4) | (m_chBuffer[1] >> 4)];
		pOutput[2] = g_Table64[((m_chBuffer[1] & 0x0f) << 2)];
		pOutput[3] = '=';

		m_pStream->Write((char *)pOutput, 4);

		m_iBufferLen = 0;
		}

	return NOERROR;
	}

ALERROR CBase64Encoder::Write (char *pData, int iLength, int *retiBytesWritten)

//	Write
//
//	Writes out binary data and encodes it into base64.

	{
	BYTE *pInput = (BYTE *)pData;
	BYTE *pInputEnd = pInput + iLength;

	//	Edge-cases

	if (pInput == pInputEnd)
		{
		if (retiBytesWritten)
			*retiBytesWritten = 0;
		return NOERROR;
		}

	//	If we've got some data in the buffer, add to it until we have a complete
	//	triplet.

	if (m_iBufferLen > 0)
		{
		if (m_iBufferLen == 1)
			{
			m_chBuffer[1] = (pInput == NULL ? 0 : *pInput++);
			m_iBufferLen++;
			if (pInput == pInputEnd)
				{
				if (retiBytesWritten)
					*retiBytesWritten = 1;
				return NOERROR;
				}

			m_chBuffer[2] = (pInput == NULL ? 0 : *pInput++);
			m_iBufferLen++;
			}
		else if (m_iBufferLen == 2)
			{
			m_chBuffer[2] = (pInput == NULL ? 0 : *pInput++);
			m_iBufferLen++;
			}

		//	We have a full buffer, so write it out

		int iWritten = WriteTriplet(m_chBuffer);
		m_iBufferLen = 0;

		if (iWritten != 3)
			return ERR_FAIL;
		}

	//	Write out the rest of the data in triplets

	while (pInput + 3 <= pInputEnd)
		{
		if (WriteTriplet(pInput) != 3)
			return ERR_FAIL;

		pInput += 3;
		}

	//	Add the remainder to the buffer

	while (pInput < pInputEnd)
		m_chBuffer[m_iBufferLen++] = *pInput++;

	//	Done

	if (retiBytesWritten)
		*retiBytesWritten = iLength;

	return NOERROR;
	}

int CBase64Encoder::WriteTriplet (void *pData)

//	WriteTripler
//
//	Writes 3 bytes encoded in base64 to 4 bytes.

	{
	BYTE *pInput = (BYTE *)pData;
	BYTE pOutput[4];

	pOutput[0] = g_Table64[pInput[0] >> 2];
	pOutput[1] = g_Table64[((pInput[0] & 0x03) << 4) | (pInput[1] >> 4)];
	pOutput[2] = g_Table64[((pInput[1] & 0x0f) << 2) | (pInput[2] >> 6)];
	pOutput[3] = g_Table64[pInput[2] & 0x3f];

	int iWritten;
	if (m_pStream->Write((char *)pOutput, 4, &iWritten) != NOERROR || iWritten != 4)
		return 0;
	
	return 3;
	}
