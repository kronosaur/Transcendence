//	Crypto.h
//
//	Basic cryptography classes

#ifndef INCL_CRYPTO
#define INCL_CRYPTO

#ifndef INCL_EUCLID
#include "Euclid.h"
#endif

class CDigest : public CIntegerIP
	{
	public:
		CDigest (void) { }
		CDigest (BYTE *pBytes);
		CDigest (IReadBlock &Data);

	private:
		enum Constants
			{
			digestLength =			20,
			digestLengthDWORD =		5,
			};
	};

void cryptoCreateDigest (IReadBlock &Data, CIntegerIP *retDigest);
void cryptoCreateMAC (IReadBlock &Data, const CIntegerIP &Key, CIntegerIP *retMAC);
void cryptoRandom (int iCount, CIntegerIP *retx);
ALERROR fileCreateDigest (const CString &sFilespec, CIntegerIP *retDigest);

#endif