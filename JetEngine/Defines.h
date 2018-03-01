#ifndef _DEFINES_HEADER
#define _DEFINES_HEADER

//given as local to the executable
#define JET_CONTENT_FOLDER "Content/"
//given as local to content folder
#define JET_SHADER_FOLDER "Shaders/"
//given as a filename/path relative to the shader folder
#define JET_DEFAULT_SHADERBUILDER "ubershader.txt"

enum Binds
{
	BIND_FIRE = 0x1,
	//BIND_FORWARD = 0x2,
	//BIND_BACK = 0x4,
	//BIND_LEFT = 0x8,
	//BIND_RIGHT = 16,
	BIND_JUMP = 32,
	BIND_RELOAD = 64,
	BIND_USE = 128,
	BIND_Z = 256,
	BIND_C = 512,
	BIND_SHIFT = 1024,
	BIND_Q = 2048,
	BIND_VIEW = 4096,
	BIND_LIGHTS = 4096*2,
	BIND_NEXT_TARGET = 4096*4,
};

#ifdef _WIN32
#undef USEOPENGL
#else
#define USEOPENGL
#endif

extern int showdebug;

#ifndef _WIN32
unsigned int GetTickCount();

void Sleep(unsigned int t);

void CreateDirectory(char* path, int wut);

#ifdef ANDROID
#define logf log
#else
void log(char* l);
void logf(const char* l, ...);
#endif
#else
void RequestInput(char* i);

void log(char* l);
void logf(const char* l, ...);

#endif

#endif