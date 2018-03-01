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
/*struct RespawnPack
{
	char id;
	int size;
	int planet;
	float x;
	float y;
	float z;
};*/

/*struct damage_data
{
	int player_number;
	int damage;
	int damage_group;
	Vec3 direction;//local direction
	float torque;//todo: needs to affect vertical and horizontal
};*/
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
	//unsigned char flags;
	float x;
	float y;
	float z;


	//redo me later ok
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
	int planet;
	float x;
	float y;
	float z;
};
#pragma pack(pop)

/*struct CEffectPacket
{
	unsigned char id;
	int eid;
	int entityid;
	int entityid2;
	float ox;
	float oy;
	float oz;
	float dx;
	float dy;
	float dz;
	float magnitude;
};*/

#endif