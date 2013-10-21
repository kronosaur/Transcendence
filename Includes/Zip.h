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

bool arcDecompressFile (const CString &sArchive, const CString &sFile, const CString &sDestFilespec, CString *retsError = NULL);
bool arcDecompressFile (const CString &sArchive, const CString &sFile, IWriteStream &Output, CString *retsError = NULL);
bool arcList (const CString &sArchive, TArray<CString> *retFiles, CString *retsError = NULL);
bool zipCompress (IReadBlock &Data, ECompressionTypes iFormat, IWriteStream &Output, CString *retsError = NULL);
bool zipDecompress (IReadBlock &Data, ECompressionTypes iFormat, IWriteStream &Output, CString *retsError = NULL);

#endif