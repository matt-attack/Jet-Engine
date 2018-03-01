#ifndef SNAPSHOT_HEADER
#define SNAPSHOT_HEADER

#include <vector>
#include <memory>

#pragma pack(push)
#pragma pack(1)

struct ED
{
	short id;
	int classid;
	char data[128];//todo this is a dangerous limit to watch out for
};

struct ServerSnap
{
	int framenumber;
	unsigned int time;
	float tod;
	unsigned short num_entities;
	ED* entdata;
};


#pragma pack(pop)

struct PlayerSnapshot
{
	struct inv_item
	{
		int id;
		//int count;
		int clip;
		int ammo;
		int entity;
	};
	inv_item inventoryItems[28];
};

struct PlayerSnapshotData
{
	PlayerSnapshotData()
	{

	}
	PlayerSnapshotData(const PlayerSnapshotData& that) = delete;
	int size;
	std::unique_ptr<char[]> data;
};

struct Snapshot
{
	char id;
	float tod;
	double timef;//time since startup sent from server is build time of this snapshot
	int framenumber;
	int diffnumber;
	unsigned int messageSent;
	unsigned int messageAcked;

	//lets have playersnapshots just be an arbitary data blob
	int num_player_snapshots;
	PlayerSnapshotData* player_snapshots;

	template <typename T>
	T* GetPlayerSnapshot(int i)
	{
		return (T*)this->player_snapshots[i].data.get();
	}

	template <typename T>
	void AddPlayerSnapshot(int i, T& snap)
	{
		this->player_snapshots[i].size = sizeof(T);
		this->player_snapshots[i].data = std::unique_ptr<char[]>(new char[sizeof(T)]);
		memcpy(this->player_snapshots[i].data.get(), &snap, sizeof(T));
	}

	unsigned short entcount;//number of entdata allocated
	ED* entdata;//only used on the client

	//server only stuff, for PVS
	int num_snapshotEntities;//number of entities actually visible
	int* snapshotEntities;

	unsigned int time;//time received by the client
};


#endif