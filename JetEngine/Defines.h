#ifndef _DEFINES_HEADER
#define _DEFINES_HEADER

//given as local to the executable
#define JET_CONTENT_FOLDER "Content/"
//given as local to content folder
#define JET_SHADER_FOLDER "Shaders/"
//given as a filename/path relative to the shader folder
#define JET_DEFAULT_SHADERBUILDER "ubershader.txt"


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