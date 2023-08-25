#ifndef __INTERFACES_H__
#define __INTERFACES_H__

class IWriter
{
public:
	virtual ~IWriter() {};
	virtual unsigned int Write(void* p, unsigned int size) = 0;
	virtual unsigned __int64 Seek(unsigned __int64 pos) = 0;
	virtual unsigned __int64 GetPosition() = 0;
};

#endif
