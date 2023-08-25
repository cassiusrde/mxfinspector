// AESDecoder.cpp: implementation of the CAESDecoder class.
//
//////////////////////////////////////////////////////////////////////
#include "StdAfx.h"
#include "AESDecoder.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

struct AES3Header {
  unsigned int sequence_count : 3;
  unsigned int reserved       : 4;
  unsigned int fvucp_flag     : 1;
};

struct AES3Data {
  unsigned int stream_number : 3;
  unsigned int f             : 1;
  unsigned int sample        : 24;
  unsigned int v             : 1;
  unsigned int u             : 1;
  unsigned int c             : 1;
  unsigned int p             : 1;
};


CAESDecoder::CAESDecoder()
{

}

CAESDecoder::~CAESDecoder()
{

}


template <typename T>
int
bitcount(T i) {
  int count = 0;

  for (; i; ++count, i &= i - 1);
  return count;
}


// Decodifica para 16bits
int CAESDecoder::DecodeAES3Frame(unsigned char* bufAES3, unsigned int lenAES3, unsigned char* bufPCM, unsigned int* lenPCM, unsigned int* nChannels, unsigned int* sampleCount)
{
unsigned int sample;
unsigned int channel;
short* shBufPCM = (short*)bufPCM;

	AES3Header* aes3Header = reinterpret_cast<AES3Header*>(bufAES3);
	*sampleCount = *reinterpret_cast<short*>(bufAES3+1);
	*nChannels = bitcount(bufAES3[3]);

	AES3Data* pAES3Data = reinterpret_cast<AES3Data*>(bufAES3+4);

	for(sample = 0; sample < *sampleCount; sample++)
	{
		for(channel = 0; channel < (*nChannels); channel++)
		{
			*shBufPCM = (pAES3Data->sample) >> 8;
			shBufPCM++;
		
			pAES3Data++;
		}
	}
	
	*lenPCM = (*nChannels)*(*sampleCount)*2; // 16bits

	return 1;
}

