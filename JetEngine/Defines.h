#ifndef _DEFINES_HEADER
#define _DEFINES_HEADER

//move to a variable, configurable, server defined tickrate
//#define TICKRATE 20

#define PACKET_BACKUP 32
#define	PACKET_MASK	(PACKET_BACKUP-1)

//enable this to improve listen server performance
#ifndef MATT_SERVER
#define SOLARSYSTEM_SHARE
#endif

//BLOCKTYPE DEFINES
#define BLOCK_STONE 2
#define BLOCK_DIRT 3
#define BLOCK_GRASS 4

#define BLOCK_SAND 19
#define BLOCK_GRAVEL 20

#define BLOCK_WOOD 22

#define BLOCK_GOLD 33
#define BLOCK_IRON 34
#define BLOCK_COAL 35


#define BLOCK_DIAMOND 51

#define BLOCK_LEAVES 54//change to 54 for LQ

//Special blocktypes
#define BLOCK_WATER 206
#define BLOCK_LAVA 255

//CHUNK SIZE DATA/DEFINES
#define CHUNK_X 16//16
#define CHUNK_X_B 4//4//bitshift

#define CHUNK_Y 16//64
#define CHUNK_Y_B 4//6//bitshift

#define CHUNK_Z 16//16
#define CHUNK_Z_B 4//4//bitshift

//rate that planets rotate, 1 = 24 seconds per rotation
#define SPIN_RATE 0//0.05//0.05//0.05

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
	BIND_VIEW = 4096
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