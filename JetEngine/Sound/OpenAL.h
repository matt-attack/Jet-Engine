#ifndef OPEN_AL_H
#define OPEN_AL_H

//This sound library is a modified version of that by Martin Prantl released under this license
//http://www.gamedev.net/page/resources/_/gdnethelp/gamedevnet-open-license-r2956
//http://www.gamedev.net/page/resources/_/technical/game-programming/basic-openal-sound-manager-for-your-project-r3791

#include <AL\al.h>
#include <AL\alc.h>

typedef unsigned long long uint64;
typedef unsigned int uint32;
typedef unsigned short uint16;
typedef unsigned char uint8;


void CheckOpenALError(const char* stmt, const char* fname, int line);
std::string GetOpenALErrorString(int errID);


inline std::string GetOpenALErrorString(int errID)
{	
	if (errID == AL_NO_ERROR)
	{
		return "";
	}

	if(errID == AL_INVALID_NAME)
    {
        return "Invalid name";
    }
    else if(errID == AL_INVALID_ENUM)
    {
        return " Invalid enum ";
    }
    else if(errID == AL_INVALID_VALUE)
    {
        return " Invalid value ";
    }
    else if(errID == AL_INVALID_OPERATION)
    {
        return " Invalid operation ";
    }
    else if(errID == AL_OUT_OF_MEMORY)
    {
        return " Out of memory! ";
    }

    return " Don't know ";	
}

inline void CheckOpenALError(const char* stmt, const char* fname, int line)
{
	
	ALenum err = alGetError();
    if (err != AL_NO_ERROR)
    {		
		printf("OpenAL error %08x, (%s) at %s:%i - for %s", err, GetOpenALErrorString(err).c_str(), fname, line, stmt);
        //abort();
    }
};

#ifndef AL_CHECK
#ifdef _DEBUG
       #define AL_CHECK(stmt) do { \
            stmt; \
            CheckOpenALError(#stmt, __FILE__, __LINE__); \
        } while (0);
#else
    #define AL_CHECK(stmt) stmt
#endif
#endif

#endif