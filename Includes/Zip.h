//	Zip.h
//
//	Basic compression classes

#ifndef INCL_ZIP
#define INCL_ZIP

enum ECompressionTypes
	{
	compressionNone =						0,
	compressionZlib =						1,	//	Zlib format
	compressionGzip =						2,	//	Gzip format
	};

bool zipCompress (IReadBlock &Data, ECompressionTypes iFormat, IWriteStream &Output, CString *retsError = NULL);
bool zipDecompress (IReadBlock &Data, ECompressionTypes iFormat, IWriteStream &Output, CString *retsError = NULL);

#endif