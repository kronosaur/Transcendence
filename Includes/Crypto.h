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

void cryptoRandom (int iCount, CIntegerIP *retx);

#endif