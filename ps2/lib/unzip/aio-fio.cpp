/*

  it's a very quick hack taken from pgen  
  to use with zlib provide in pgen 

*/

#include <tamtypes.h>
#include <kernel.h>
#include <string.h>
#include <sifrpc.h>
#include <libpad.h>
#include <loadfile.h>
#include <iopheap.h>
#include <fileio.h>
#include <malloc.h>
#include <libmc.h>

#define MIN(a, b)		(((a)<(b))?(a):(b))
#define SAFE_READ_SIZE	16384

//#include "aio.h"

/*
abstractIO *currentAIO = NULL;


void aioSetCurrent(abstractIO *thisAIO)
{
	currentAIO = thisAIO;
}

abstractIO *aioGetCurrent()
{
	return currentAIO;
}
*/
extern "C" int aioOpen(const char *name, int flags)
{
	/*if(!currentAIO)
		return -1;*/
    return fioOpen(name, flags);
	//return currentAIO->open(name, flags);
}

extern "C" int aioClose(int fd)
{
/*	if(!currentAIO)
		return -1;*/
		
    return fioClose(fd);
//	return currentAIO->close(fd);
}

extern "C" int aioRead(int fd, unsigned char *buffer, int size)
{
	/*if(!currentAIO)
		return -1;*/
		
	u8 *currentPtr = (u8 *)buffer;
	int read = 0;
	int rv;

	while(size)
	{
		int currentSize = MIN(SAFE_READ_SIZE, size);

		rv = fioRead(fd, currentPtr, currentSize);
		if(rv < 0)
			return rv;
		else if(rv != currentSize)
		{
			read += rv;
			return read;
		}

		currentPtr += currentSize;
		size -= currentSize;
		read += currentSize;
	}

	return read;
	//return currentAIO->read(fd, buffer, size);
}

extern "C" int aioWrite(int fd, const unsigned char *buffer, int size)
{
	//if(!currentAIO)
		return -1;

	//return currentAIO->write(fd, buffer, size);
}

extern "C" int aioLseek(int fd, int offset, int whence)
{
	/*if(!currentAIO)
		return -1;*/
		
	return fioLseek(fd, offset, whence);
	
	//return currentAIO->lseek(fd, offset, whence);
}
/*
extern "C" int aioRemove(const char *name)
{
//	if(!currentAIO)
		return -1;

//	return currentAIO->remove(name);
}

extern "C" int aioRename(const char *old, const char *newname)
{
//	if(!currentAIO)
		return -1;

//	return currentAIO->rename(old, newname);
}

extern "C" int aioMkdir(const char *name)
{
//	if(!currentAIO)
		return -1;

//	return currentAIO->mkdir(name);
}

extern "C" int aioRmdir(const char *name)
{
//	if(!currentAIO)
		return -1;

//	return currentAIO->rmdir(name);
}*/
/*
extern "C" int aioGetdir(const char *name, const char *extensions, t_aioDent dentBuf[], int maxEnt)
{
//	if(!currentAIO)
		return -1;

//	return currentAIO->getdir(name, extensions, dentBuf, maxEnt);
}

extern "C" int aioGetstat(const char *name, t_aioDent *dent)
{
//	if(!currentAIO)
		return -1;

//	return currentAIO->getstat(name, dent);
}*/
