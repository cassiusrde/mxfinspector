#ifndef __MXF_UTILS_H__
#define __MXF_UTILS_H__

#include "MXFTypes.h"

unsigned __int64 GetNumber(HANDLE hFile, int len);
unsigned __int64 GetNumber64(HANDLE hFile);
unsigned int     GetNumber32(HANDLE hFile);
unsigned short   GetNumber16(HANDLE hFile);
unsigned char    GetNumber8(HANDLE hFile);
unsigned int	 ReadBERLength(HANDLE hFile);

unsigned __int64 GetFileSize(HANDLE hFile);
unsigned __int64 GetFilePosition(HANDLE hFile);
void InvertUTF16String(String str);


unsigned int WriteNumber8(unsigned char n, IWriter* pWriter);
unsigned int WriteNumber16(unsigned short n, IWriter* pWriter);
unsigned int WriteNumber32(unsigned int n, IWriter* pWriter);
unsigned int WriteNumber64(unsigned __int64 n, IWriter* pWriter);
unsigned int WriteBuffer(void* p, unsigned int size, IWriter* pWriter);
unsigned int WriteBERLength(unsigned int length, IWriter* pWriter);

UInt64 UInt64ByteSwap(UInt64);

void Frame2TimeCode(UInt64 n_frames, unsigned char& hours, unsigned char& minutes, unsigned char& seconds, unsigned char& frames, bool dropframe = true);

#endif
