#include <stdafx.h>
#include "MXFUtils.h"


unsigned __int64 GetNumber(HANDLE hFile, int len)
{
unsigned __int64 ret = 0;
unsigned char ch;
DWORD nRead;
	for(int i=0; i < len; i++)
	{
		ret<<=8;
		ReadFile(hFile, &ch, 1, &nRead, NULL);
		ret|=ch;
	}
	return ret;
}


unsigned __int64 GetNumber64(HANDLE hFile)
{
unsigned __int64 ret = 0;
unsigned char ch;
DWORD nRead;
	for(int i=0; i < 8; i++)
	{
		ret<<=8;
		ReadFile(hFile, &ch, 1, &nRead, NULL);
		ret|=ch;
	}
	return ret;
}

unsigned int GetNumber32(HANDLE hFile)
{
unsigned int ret = 0;
unsigned char ch;
DWORD nRead;
	for(int i=0; i < 4; i++)
	{
		ret<<=8;
		ReadFile(hFile, &ch, 1, &nRead, NULL);
		ret|=ch;
	}
	return ret;
}


unsigned short GetNumber16(HANDLE hFile)
{
unsigned short ret = 0;
unsigned char ch;
DWORD nRead;
	for(int i=0; i < 2; i++)
	{
		ret<<=8;
		ReadFile(hFile, &ch, 1, &nRead, NULL);
		ret|=ch;
	}
	return ret;
}


unsigned char GetNumber8(HANDLE hFile)
{
unsigned char ch;
DWORD nRead;

	ReadFile(hFile, &ch, 1, &nRead, NULL);
	return ch;
}

unsigned int ReadBERLength(HANDLE hFile)
{
DWORD size, szSize;
unsigned char ch;
DWORD nRead;
	ReadFile(hFile, &ch, 1, &nRead, NULL);

	if( ch & 0x80 )
	{
		szSize = ch&0x0f;
		size = (DWORD)GetNumber(hFile, szSize);
	}
	else
	{
		size = ch;
	}	
	return size;
}


unsigned __int64 GetFilePosition(HANDLE hFile)
{
LARGE_INTEGER pos = {0};

	pos.LowPart = SetFilePointer(hFile, 0, &pos.HighPart, FILE_CURRENT);
	return pos.QuadPart;
}

unsigned __int64 GetFileSize(HANDLE hFile)
{
LARGE_INTEGER size = {0};

	GetFileSizeEx(hFile, &size);
	return size.QuadPart;
}


void InvertUTF16String(String str)
{
size_t len = wcslen(str);
	for(unsigned int i = 0; i < len; i++)
		str[i] = ((str[i] << 8)&0xff00) + (str[i] >> 8);
}


unsigned int WriteNumber8(unsigned char n, IWriter* pWriter)
{
	return pWriter->Write(&n, 1);
}


unsigned int WriteNumber16(unsigned short n, IWriter* pWriter)
{
DWORD nWritten = 0;
unsigned char* p = (unsigned char*)&n;
	nWritten += WriteNumber8(p[1], pWriter);
	nWritten += WriteNumber8(p[0], pWriter);
	return nWritten;
}


unsigned int WriteNumber32(unsigned int n, IWriter* pWriter)
{
DWORD nWritten = 0;
unsigned char* p = (unsigned char*)&n;
	nWritten += WriteNumber8(p[3], pWriter);
	nWritten += WriteNumber8(p[2], pWriter);
	nWritten += WriteNumber8(p[1], pWriter);
	nWritten += WriteNumber8(p[0], pWriter);
	return nWritten;
}


unsigned int WriteNumber64(unsigned __int64 n, IWriter* pWriter)
{
DWORD nWritten = 0;
unsigned char* p = (unsigned char*)&n;
	nWritten += WriteNumber8(p[7], pWriter);
	nWritten += WriteNumber8(p[6], pWriter);
	nWritten += WriteNumber8(p[5], pWriter);
	nWritten += WriteNumber8(p[4], pWriter);
	nWritten += WriteNumber8(p[3], pWriter);
	nWritten += WriteNumber8(p[2], pWriter);
	nWritten += WriteNumber8(p[1], pWriter);
	nWritten += WriteNumber8(p[0], pWriter);
	return nWritten;
}


unsigned int WriteBuffer(void* p, unsigned int size, IWriter* pWriter)
{
	return pWriter->Write(p, size);
}


unsigned int WriteBERLength(unsigned int length, IWriter* pWriter)
{
unsigned int written = 0;

	// sempre 3 bytes
	written += WriteNumber8(0x83, pWriter);
	unsigned char* pLen = (unsigned char*)&length;
	written += WriteNumber8(pLen[2], pWriter);
	written += WriteNumber8(pLen[1], pWriter);
	written += WriteNumber8(pLen[0], pWriter);

	return written;
}


UInt64 UInt64ByteSwap(UInt64 number)
{
unsigned char* p = (unsigned char*)&number;
unsigned char a;
	a = p[0]; p[0] = p[7]; p[7] = a;
	a = p[1]; p[1] = p[6]; p[6] = a;
	a = p[2]; p[2] = p[5]; p[5] = a;
	a = p[3]; p[3] = p[4]; p[4] = a;

	return number;
}



void Frame2TimeCode(UInt64 n_frames, unsigned char& hours, unsigned char& minutes, unsigned char& seconds, unsigned char& frames, bool dropframe /*= true*/)
{
	UInt64 t_hora, t_min, t_seg, t_frame;

	if (!dropframe)
	{
		// Cálculo sem Drop Frame
		// Simplesmente faz o cálculo com 30 frames por segundo
		t_frame = n_frames % 30;
		n_frames /= 30;
		t_seg = n_frames % 60;
		n_frames /= 60;
		t_min = n_frames % 60;
		n_frames /= 60;
		t_hora = n_frames % 24;
	}
	else
	{
		// Correção de DropFrame:
		// Inserir (adcionar) 2 frames na contagem de timecode a cada minuto
		// completado pulando de *:xx:59:29 -> *:xx:00:02. com exceção dos
		// minutos divisiveis por dez, ou seja: a cada 10 minutos não serão
		// adcionados os 2 frames.
		// No total são adcionados 18 frames a cada 10 minutos

		// O algoritmo transformar o número de frames reais em um time code, pulando os TC inválidos, é:
		// - Primeiro passo: transformar o número de frames de 33,37 ms em frames de 30 ms
		//    - É fácil calcular o tempo real dado o número de fames com uma distribuição regular
		// - O ciclo do algoritmo é 10 minutos
		// - A cada 10 minutos, acrescenta 18 time codes inválidos ao número de frames
		// - O número de frames reais em 10 minutos é: 1800 + 1798 * 9 = 17982
		// - Se o número de frames fora do ciclo de 10 minutos é menor ou igual a 1800,
		//      ainda estamos no primeiro minuto do ciclo --> este minuto tem 1800 frames
		// - Se já tivermos passado do primeiro minuto, salta os primeiros 1800 frames e
		//      divide o resultado por 1798, para chegarmos no número de minutos de 1798 frames
		// - Adiciona 2 frames para cada um dos minutos de 1798 frames

		UInt64 n_dec = n_frames / 17982;          // Número de grupos de 10 minutos
		UInt64 n_fram = n_frames % 17982;         // Número de frames que sobraram

		UInt64 num_tc_dropped = 18 * n_dec;       // Adiciona 18 frames a cada 10 min completos
		
		if (n_fram >= 1800)
		{
			n_fram -= 1800;
			num_tc_dropped += 2 * (n_fram / 1798) + 2;
		}

		t_frame = n_frames + num_tc_dropped;

		// Cáluculo agora com frames de exatos 30 ms
		t_hora  = t_frame / 108000; // Número de frames por Hora = 30 * 60 * 60 = 108000
		t_frame -= t_hora * 108000;
		t_min = t_frame / 1800;     // Número de frames por Minuto = 30 * 60 = 1800
		t_frame -= t_min * 1800;
		t_seg = t_frame / 30;       // Número de frames por Segundo = 30
		t_frame -= t_seg * 30;
		t_hora %= 24;
	}

	hours = (unsigned char)t_hora;
	minutes = (unsigned char)t_min;
	seconds = (unsigned char)t_seg;
	frames = (unsigned char)t_frame;
}
