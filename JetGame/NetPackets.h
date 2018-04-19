#ifndef _NETPACKET_HEADER
#define _NETPACKET_HEADER

#include "JetEngine/Math/Vector.h"
#include "JetEngine/Math/Quaternion.h"

#pragma pack(push)
#pragma pack(1)

struct PlayerDisconnect
{
	char id;
	char plyid;
};

#pragma pack(pop)

struct PlayerUpdatePacket
{
	char id;
	char playerID;
	unsigned char flags;
	char weapon;
	short parent;

	//command id
	int commandid;

	//char state;
	float x;
	float y;
	float z;

	Quaternion rotation;//for vehicles

	//this contains all "binds" to perform actions
	signed char axes[4];
	int binds;

	int lastsnapshot;

	Quaternion view;
	Vec3 velocity;
	float delta_time;
};

struct PlayerInitialUpdate
{
	int packetid;
	int playerid;
	char name[25];
	float x;
	float y;
	float z;
};

/*struct PlayerDisconnect
{
	char id;
	char plyid;
};*/

#pragma pack(push)
#pragma pack(1)
struct InitialPacket//send to tell client the join was successful
{
	int id;//required
	//all of these are custom and optional
	char localplyid;
	double time;
	int parent;
	float x;
	float y;
	float z;
};
#pragma pack(pop)

#endif