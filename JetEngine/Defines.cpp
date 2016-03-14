#include "Defines.h"

#include <cstdio>

int showdebug = 0;

#ifndef _WIN32
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdarg.h>

void Sleep(unsigned int t)
{
	usleep(t*1000);
}

#include <sys/time.h>
unsigned int GetTickCount()
{
	struct timeval tv;
	if(gettimeofday(&tv, NULL) != 0)
		return 0;

	return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}

void CreateDirectory(char* path, int wut)
{
	mkdir(path, 0770);
}

#ifndef ANDROID
#include <cstdio>
void log(char* o)
{
	printf(o);
}
void logf(const char* fmt, ...)
{
	va_list		argptr;
	char		msg[500];

	va_start (argptr,fmt);
	vsnprintf(msg, sizeof(msg), fmt, argptr);
	va_end (argptr);

	printf(msg);
}
#endif

#else
#include <Windows.h>


void RequestInput(char* i)
{

}

void log(char* o)
{
	printf(o);OutputDebugStringA(o);
}

#include <cstdio>
void logf(const char* fmt, ...)
{
	va_list		argptr;
	char		msg[500];

	va_start (argptr,fmt);
	vsnprintf(msg, sizeof(msg), fmt, argptr);
	va_end (argptr);

	printf(msg);OutputDebugStringA(msg);
}
#endif