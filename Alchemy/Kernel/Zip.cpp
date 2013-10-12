//	Zip.cpp
//
//	Implements compression functions
//	Portions copyright (c) 2013 by Kronosaur Productions, LLC. All Rights Reserved.

#include "Kernel.h"
#include "KernelObjID.h"
#include "Zip.h"

#define ZLIB_WINAPI
#include "..\..\zlib-1.2.7\zlib.h"

#ifdef ZLIB_H
//	unzip.h needs _ZLIB_H defined (not ZLIB_H, for some reason)
#define _ZLIB_H
#endif

#include "..\..\zlib-1.2.7\contrib\minizip\unzip.h"

const int BUFFER_SIZE = 1024 * 1024;

bool Deflate (IReadBlock &Data, ECompressionTypes iFormat, IWriteStream &Output, CString *retsError);
bool Inflate (IReadBlock &Data, ECompressionTypes iFormat, IWriteStream &Output, CString *retsError);
bool NullCopy (IReadBlock &Data, IWriteStream &Output, CString *retsError);

bool zipCompress (IReadBlock &Data, ECompressionTypes iFormat, IWriteStream &Output, CString *retsError)

//	zipCompress
//
//	Compresses a block

	{
	switch (iFormat)
		{
		case compressionNone:
			if (!NullCopy(Data, Output, retsError))
				return false;
			break;

		case compressionGzip:
		case compressionZlib:
			if (!Deflate(Data, iFormat, Output, retsError))
				return false;
			break;

		default:
			ASSERT(false);
			return false;
		}

	return true;
	}

bool zipDecompress (IReadBlock &Data, ECompressionTypes iFormat, IWriteStream &Output, CString *retsError)

//	zipDecompress
//
//	Decompresses a block

	{
	switch (iFormat)
		{
		case compressionNone:
			if (!NullCopy(Data, Output, retsError))
				return false;
			break;

		case compressionGzip:
		case compressionZlib:
			if (!Inflate(Data, iFormat, Output, retsError))
				return false;
			break;

		default:
			ASSERT(false);
			return false;
		}

	return true;
	}

bool Deflate (IReadBlock &Data, ECompressionTypes iFormat, IWriteStream &Output, CString *retsError)
	{
	//	Initialize library

	z_stream zcpr;
	utlMemSet(&zcpr, sizeof(zcpr), '\0');

	if (iFormat == compressionZlib)
		deflateInit(&zcpr, Z_DEFAULT_COMPRESSION);
	else if (iFormat == compressionGzip)
		deflateInit2(&zcpr,
				Z_DEFAULT_COMPRESSION,
				Z_DEFLATED,
				0x1f,		//	Bit 0x10 means gzip header
				8,
				Z_DEFAULT_STRATEGY);
	else
		{
		ASSERT(false);
		if (retsError)
			*retsError = CONSTLIT("Unknown deflate format.");
		return false;
		}

	//	Allocate a buffer to compress to

	BYTE *pBuffer = new BYTE [BUFFER_SIZE];

	//	Compress

	DWORD dwSourceLeft = Data.GetLength();
	BYTE *pSource = (BYTE *)Data.GetPointer(0);

	//	The entire source is available

	zcpr.next_in = pSource;
	zcpr.avail_in = dwSourceLeft;

	int iWritten = 0;
	int ret;
	while (true)
		{
		zcpr.next_out = pBuffer;
		zcpr.avail_out = BUFFER_SIZE;

		ret = deflate(&zcpr, Z_FINISH);

		//	If error, then fail

		if (ret != Z_OK 
				&& ret != Z_STREAM_END
				&& ret != Z_BUF_ERROR)
			{
			deflateEnd(&zcpr);
			delete [] pBuffer;
			if (retsError)
				*retsError = CONSTLIT("Error in deflate.");
			return false;
			}

		//	Write out the compressed data

		int iChunk = zcpr.total_out - iWritten;
		iWritten += iChunk;

		if (Output.Write((char *)pBuffer, iChunk) != NOERROR)
			{
			deflateEnd(&zcpr);
			delete [] pBuffer;
			if (retsError)
				*retsError = CONSTLIT("Unable to write to output.");
			return false;
			}

		//	If we need more output buffer, then continue

		if (ret == Z_BUF_ERROR
				|| (ret == Z_OK && zcpr.avail_out == 0))
			continue;

		//	Otherwise we're done

		break;
		}

	//	Done

	deflateEnd(&zcpr);
	delete [] pBuffer;
	return true;
	}

bool Inflate (IReadBlock &Data, ECompressionTypes iFormat, IWriteStream &Output, CString *retsError)
	{
	//	Initialize library

	z_stream zcpr;
	utlMemSet(&zcpr, sizeof(zcpr), '\0');

	if (iFormat == compressionZlib)
		inflateInit(&zcpr);
	else if (iFormat == compressionGzip)
		inflateInit2(&zcpr,
				0x1f);		//	Bit 0x10 means gzip header
	else
		{
		ASSERT(false);
		if (retsError)
			*retsError = CONSTLIT("Unknown deflate format.");
		return false;
		}

	//	Allocate a buffer to compress to

	BYTE *pBuffer = new BYTE [BUFFER_SIZE];

	//	Compress

	DWORD dwSourceLeft = Data.GetLength();
	BYTE *pSource = (BYTE *)Data.GetPointer(0);

	//	The entire source is available

	zcpr.next_in = pSource;
	zcpr.avail_in = dwSourceLeft;

	int ret;
	int iWritten = 0;

	while (true)
		{
		zcpr.next_out = pBuffer;
		zcpr.avail_out = BUFFER_SIZE;

		ret = inflate(&zcpr, Z_FINISH);

		//	If error, then fail

		if (ret != Z_OK 
				&& ret != Z_STREAM_END
				&& ret != Z_BUF_ERROR)
			{
			inflateEnd(&zcpr);
			delete [] pBuffer;
			if (retsError)
				*retsError = CONSTLIT("Error in deflate.");
			return false;
			}

		//	Write out the compressed data

		int iChunk = zcpr.total_out - iWritten;
		iWritten += iChunk;

		if (Output.Write((char *)pBuffer, iChunk) != NOERROR)
			{
			inflateEnd(&zcpr);
			delete [] pBuffer;
			if (retsError)
				*retsError = CONSTLIT("Unable to write to output.");
			return false;
			}

		//	If we need more output buffer, then continue

		if (ret == Z_BUF_ERROR
				|| (ret == Z_OK && zcpr.avail_out == 0))
			continue;

		//	Otherwise we're done

		break;
		}

	//	Done

	inflateEnd(&zcpr);
	delete [] pBuffer;
	return true;
	}

bool NullCopy (IReadBlock &Data, IWriteStream &Output, CString *retsError)
	{
	if (Output.Write(Data.GetPointer(0), Data.GetLength()) != NOERROR)
		{
		if (retsError)
			*retsError = CONSTLIT("Unable to copy data to output.");
		return false;
		}

	return true;
	}
