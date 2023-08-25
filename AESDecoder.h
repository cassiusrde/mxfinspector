// AESDecoder.h: interface for the CAESDecoder class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_AESDECODER_H__2E2284B3_7B08_4DDF_9DA0_379AB5FB8A67__INCLUDED_)
#define AFX_AESDECODER_H__2E2284B3_7B08_4DDF_9DA0_379AB5FB8A67__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000



class CAESDecoder  
{
public:
	CAESDecoder();
	virtual ~CAESDecoder();

	int DecodeAES3Frame(unsigned char* bufAES3, unsigned int lenAES3, unsigned char* bufPCM, unsigned int* lenPCM, unsigned int* nChannels, unsigned int* sampleCount);


};

#endif // !defined(AFX_AESDECODER_H__2E2284B3_7B08_4DDF_9DA0_379AB5FB8A67__INCLUDED_)
