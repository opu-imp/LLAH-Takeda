#include "def_general.h"
#include "proctime.h"

#ifdef	WIN32
#include <windows.h>
#include <mmsystem.h>
#else
#include <sys/resource.h>
#include <sys/time.h>
#endif

int GetProcTimeSec( void )
{
#ifdef	WIN32
	return timeGetTime()*1000;
#else
	struct rusage RU;
	getrusage(RUSAGE_SELF, &RU);
	return RU.ru_utime.tv_sec + (int)(RU.ru_utime.tv_usec*1e-6);
#endif
}

int GetProcTimeMiliSec( void )
{
#ifdef	WIN32
	return timeGetTime();
#else
	struct rusage RU;
	getrusage(RUSAGE_SELF, &RU);
	return (int)(RU.ru_utime.tv_sec*1000) + (int)(RU.ru_utime.tv_usec*1e-3);
#endif
}

int GetProcTimeMicroSec( void )
{
#ifdef	WIN32
	return (int)(timeGetTime() / 1000);
#else
	struct rusage RU;
	getrusage(RUSAGE_SELF, &RU);
	return RU.ru_utime.tv_sec*1000000 + RU.ru_utime.tv_usec;
#endif
}
