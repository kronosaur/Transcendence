//	CryptoRandom.cpp
//
//	Implements random function
//	Portions copyright (c) 2012 by George Moromisato. All Rights Reserved.

#include "Kernel.h"
#include "KernelObjID.h"

#include "Crypto.h"

void cryptoRandom (int iCount, CIntegerIP *retx)

//	cryptoRandom
//
//	Generate iCount BYTEs of randomness
//

	{
	//	Edge cases

	if (iCount == 0)
		*retx = CIntegerIP();

	//	Generate bytes

	CMemoryWriteStream Chaos(iCount);
	Chaos.Create();

	int iLeft = iCount;
	while (iLeft)
		{
		//	rand_s is Window's cryptographic random number generator.

		unsigned int dwRnd;
		rand_s(&dwRnd);

		int iWrite = Min(iLeft, (int)sizeof(DWORD));
		Chaos.Write((char *)&dwRnd, iWrite);

		//	Next
		
		iLeft -= iWrite;
		}

	//	Initialize. Note that CIPInteger expects the byte stream to be in big-
	//	endian order, but since random is random it doesn't matter that we're
	//	backwards.

	CIntegerIP Result(iCount, (BYTE *)Chaos.GetPointer());
	retx->TakeHandoff(Result);
	}
