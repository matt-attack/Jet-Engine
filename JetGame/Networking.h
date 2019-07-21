#ifndef NETWORKINGH_HEADER
#define NETWORKINGH_HEADER

#include "Snapshot.h"
#include "Datatables.h"
#include "NetPackets.h"

#include <JetEngine/CInput.h>

#include <JetNet/Connection.h>
#include <JetNet/NetMsg.h>

#include <functional>
#include <queue>

#define SERVER_PORT 5007

#define	PACKET_BACKUP	64	// number of old messages that must be kept on client and
// server for delta compression and ping estimation (should be at least several times the maximum ping)
#define	PACKET_MASK		(PACKET_BACKUP-1)

#define MAX_PACKET_ENTITIES 0x7FFF//25555

enum NetworkingConstants
{
	MaxLocalPlayers = 4, //limit of number of local players per client for splitscreen usually
	MaxPlayers = 20 //hardcoded limit for number of players a server can have
};

namespace NetworkingPackets
{
	enum NetworkingPackets
	{
		AddPlayer = 5,
		PlayerDisconnect = 99,
		Snapshot = 73,
		InitialPacket = 1945,
	};
}


void DecodeDeltaEntity(NetMsg* msg, Snapshot* newframe, int num, ED* oldstate, bool full, int newindex);

void WriteDeltaEntity(NetMsg* msg, ED* oldent, ED* newent, bool full);

class CPlayer;

struct command
{
	int size;
	char* data;
};

class NetworkedEntity
{
public:
	bool predicted;
	short EntID;//index in entity array
	int typeID;//hash for class name
	Vec3 position;
	Vec3 last_snap_position;//used for calculating the velocity
	Vec3 velocity;
};

class CEntity;
class CPlayerEntity;
class PlayerBase;
class EntityManagerBase
{
public:
	// Basic functions you will need to implement
	virtual int MaxID() = 0;
	virtual int Count() = 0;
	virtual CEntity* CreateEntity(int id) = 0;
	virtual void AddEntity(CEntity* ent, unsigned int id) = 0;
	virtual void RemoveEntity(short id) = 0;

	// Snapshot related overrides
	virtual void EncodeEntities(ServerSnap* snap);// Already has a implementation, but can be overriden
	virtual void ApplySnapshot(Snapshot* snap, Snapshot* oldsnap);// Already has a implementation, but can be overriden
	virtual void Interpolate(Snapshot* Old, Snapshot* New, float fraction) = 0;

	virtual CEntity* GetEntByID(short id) = 0;
	virtual NetworkedEntity* GetByID(short id) = 0;

	virtual bool ShouldTransmit(CEntity* ent, PlayerBase* player) = 0;

	//Player related overrides
	virtual PlayerBase* CreatePlayer(short id) = 0;
	virtual void BuildDeltaPlayerSnapshot(const PlayerSnapshotData* oldframe, const PlayerSnapshotData* frame, NetMsg* msg) = 0;
	virtual void BuildPlayerSnapshot(Snapshot* snap, int num_entities, PlayerBase** entities) = 0;
	virtual void DecodePlayerSnapshots(Snapshot* snap, Snapshot* oldsnap, NetMsg* msg) = 0;
	virtual void ApplyPlayerSnapshots(Snapshot* snap, Snapshot* oldsnap, PlayerBase** local_players) = 0;
};

//this should be imported into your clientside player class
class PlayerBase
{
	std::map<std::string, float> variables;//cvars

public:
	bool islocalplayer;
	int lastsentmove;
	PlayerUpdatePacket* moves;

	char name[25];
	Vec3 server_position;

	char selected_weapon;

	Quaternion view;

	PlayerBase()
	{
		islocalplayer = false;
		lastsentmove = 0;
		moves = 0;
	}

	~PlayerBase()
	{
		delete[] moves;
	}

	void AllocateMoves()
	{
		if (moves == 0)
			moves = new PlayerUpdatePacket[100];
	}

	void SetCVar(char* name, float val, bool network = true)
	{
		//todo get this networking to work
		/*if (this->manager->isclient)
		{
		char o[200];
		sprintf(o, "set %s %f", name, val);
		gMultiplayer->network.SendCommand(o);
		}
		else if (network)
		log("OOOPS CVAR SETTING NOT IMPLEMENTED ON THE SERVER\n");*/
		this->variables[name] = val;
	}

	float GetCVar(char* val)
	{
		return this->variables[val];
	}


	//virtual functions
	virtual Vec3 GetPosition() = 0;
	virtual Vec3 GetVelocity() = 0;
	virtual void SetPosition(const Vec3& np) = 0;
	virtual int GetID() = 0;
	virtual CEntity* Base() = 0;
	virtual unsigned char GetFlags() = 0;
	virtual Quaternion GetRotation() = 0;
	virtual Vec3 GetAimDir() = 0;
	virtual char GetParent()
	{
		return -1;
	}

	virtual void SetParent(char p)
	{

	}
	virtual void OnJoin() = 0;
	virtual void ProcessSnapshot(PlayerUpdatePacket* snapshot) = 0;

	//server only hack
	Vec3 _lastposition;
	Vec3 _lastaimdir;

	unsigned short ping;
};

class ClientNetworker
{
	bool use_tcp;
public:
	NetConnection connection;
	SocketTCP data_connection;

	int lastSnapshotNum;
	int lastAckedMove;
	unsigned short ping;
	int droppedPackets;

	float time_of_day;
private:
	Snapshot framescache[PACKET_BACKUP];

	EntityManagerBase* EntityManager;

	std::queue<command> commands;

	int num_local_players;
	PlayerBase** local_players;

	std::function<void(char, unsigned int, char*)> message_callback;
public:

	ClientNetworker(EntityManagerBase* manager, PlayerBase** local_players, std::function<void(char, unsigned int, char*)> message_callback, bool use_tcp) :
		EntityManager(manager), local_players(local_players), message_callback(message_callback), use_tcp(use_tcp)
	{
		lastAckedMove = -1;
		ping = 0;
	}

	//this updates network stuff and runs interpolation
	bool Update(float dT);

	void Cleanup()
	{
		for (int i = 0; i < PACKET_BACKUP; i++)
		{
			if (framescache[i].entdata)
				delete[] framescache[i].entdata;
			framescache[i].entdata = 0;

			if (framescache[i].player_snapshots)
				delete[] framescache[i].player_snapshots;
		}
	}

	int Connect(Address serveraddr, unsigned short cport, const char* plyname, const char* password, int players = 1, char** status = 0);

private:
	void ProcessPacket(char packetID, int size, char* buffer);

public:
	void CorrectPlayerPos(int last, PlayerBase** local_players, int num_local_players);

	void CreateMove(float dT, PlayerBase* player, int player_id, CInput* input, bool in_menu);

	void SendCommand(char* cmd)
	{
		char* cpy = new char[strlen(cmd) + 8];
		strcpy(cpy + 5, cmd);
		cpy[0] = 42;
		command c;
		c.data = cpy;
		c.size = strlen(cmd) + 6;
		commands.push(c);
	}

	void UpdateCommands()
	{
		while (commands.empty() == false)//no commands left to send out
		{
			//ok, we are clear to send new stuff
			NetMsg m = NetMsg(200, commands.front().data);
			m.WriteByte(42);
			m.WriteInt(0);//this->lastsentcommand);
			//send it and update our sent command id
			this->connection.SendReliable(commands.front().data, commands.front().size);
			delete[] commands.front().data;
			commands.pop();
		}
	}

	Snapshot* DecodeDeltaSnapshot(NetMsg* msg);

	Snapshot* GetPreviousSnapshot(int framenumber)//gets the snapshot that came before the one supplied
	{
		if (framenumber == 0)
			throw 7;

		Snapshot* old = &this->framescache[(framenumber - 1) & PACKET_MASK];
		if (old == 0)
		{
			//get most recent one
			for (int i = 0; i < PACKET_BACKUP; i++)
			{
				Snapshot* osnap = &this->framescache[i];
				if (osnap->framenumber != framenumber)
				{
					if (old == 0)
						old = osnap;
					else if (old->framenumber < osnap->framenumber)
						old = osnap;
				}
			}
		}
		return old;
	}

	// This gets the two snapshots boardering some timestamp most closely
	// Used for interpolating between them
	bool GetInterpolationSnapshots(unsigned int irt, Snapshot** min, Snapshot** max)
	{
		for (int i = 0; i < PACKET_BACKUP; i++)
		{
			Snapshot* c = &framescache[i];
			if (c->received_time > irt)
			{
				if (*max == 0 || c->received_time < (*max)->received_time)
				{
					*max = c;
				}
			}
			else if (c->received_time < irt)
			{
				if (*min == 0 || c->received_time > (*min)->received_time)
				{
					*min = c;
				}
			}
		}
		if (*min && *max && (*min)->received_time && (*max)->received_time)
			return true;
		else
			return false;//cant interpolate
	}
};

//A Server has one of these
class ServerNetworker
{
	int frameNumber;
	ServerSnap snapshotEntities[PACKET_BACKUP];

public:
	ServerSnap* currentSnapshot;

	ServerNetworker()
	{
		this->frameNumber = 0;

		for (int i = 0; i < PACKET_BACKUP; i++)
		{
			this->snapshotEntities[i].entdata = 0;
		}
	}

	~ServerNetworker()
	{
		for (int i = 0; i < PACKET_BACKUP; i++)
		{
			if (this->snapshotEntities[i].entdata)
				delete[] this->snapshotEntities[i].entdata;
		}
	}

	//maybe rename me
	ServerSnap* BuildEntityData(EntityManagerBase* entityManager, float TOD);

	// This gets the two snapshots boardering some timestamp most closely
	// Used for interpolating between them
	bool GetInterpolationSnapshots(unsigned int irt, ServerSnap** min, ServerSnap** max)
	{
		for (int i = 0; i < PACKET_BACKUP; i++)
		{
			ServerSnap* c = &snapshotEntities[i];
			if (c->time > irt)
			{
				if (*max == 0 || c->time < (*max)->time)
				{
					*max = c;
				}
			}
			else if (c->time < irt)
			{
				if (*min == 0 || c->time > (*min)->time)
				{
					*min = c;
				}
			}
		}
		if (*min && *max && (*min)->time && (*max)->time)
			return true;
		else
			return false;//cant interpolate
	}
};


//There is one of these per client, it handles each potential local player on that client
//ok each client can have multiple players, but only receives 1 snapshot per update
//for each player the client has it sends one playersnapshot
class ServerClientNetworker
{
	int frameNumber;
	int lastSnapshotRecieved;
	int acknowledgedMove[MaxLocalPlayers];
	Snapshot frames[PACKET_BACKUP];

public:
	unsigned short ping;

	//need to be able to handle networked players dropping in and out
	//	so need to communicate number of players between 
	ServerClientNetworker()
	{
		for (int i = 0; i < MaxLocalPlayers; i++)
			this->acknowledgedMove[i] = -1;
		this->frameNumber = 0;
		this->lastSnapshotRecieved = -10000;

		for (int i = 0; i < PACKET_BACKUP; i++)
		{
			this->frames[i].messageAcked = 0;
			this->frames[i].messageSent = 0;
			this->frames[i].entcount = 0;
			this->frames[i].entdata = 0;
			this->frames[i].snapshotEntities = 0;
			this->frames[i].player_snapshots = 0;
		}
	}

	~ServerClientNetworker()
	{
		for (int i = 0; i < PACKET_BACKUP; i++)
		{
			if (this->frames[i].snapshotEntities)
				delete[] this->frames[i].snapshotEntities;
		}
	}

	bool ProcessMove(int player_index, PlayerUpdatePacket* p)
	{
		//fix the connect packet not having a way to add extra data add a optional netmsg arg
		//	and move name to it
		if (p->commandid <= this->acknowledgedMove[player_index])
		{
			printf("got out of order client move command, dropping\n");
			return false;
		}

		//use this to drop old move commands
		//this needs to me moved to per client entity, not per client
		this->acknowledgedMove[player_index] = p->commandid;

		//only update our count if the number we got from the client is higher than what we have
		//(drop old client update packets)
		if (p->lastsnapshot > this->lastSnapshotRecieved)
		{
			//printf("got snapshot ack from client\n");
			this->frames[p->lastsnapshot & PACKET_MASK].messageAcked = GetTickCount();//use timeGetTime()?
			this->lastSnapshotRecieved = p->lastsnapshot;
		}
		//else
		//printf("got old client snapshot\n");
		return true;
	}

	//this is silly
	void CalculatePing()
	{
		int total = 0;
		int count = 0;
		for (int i = 0; i < PACKET_BACKUP; i++)
		{
			if (this->frames[i].messageAcked == 0)
				continue;
			int delta = this->frames[i].messageAcked - this->frames[i].messageSent;
			count++;
			total += delta;
		}
		if (count > 0)
		{
			this->ping = total / count;
		}
	}

	//tell this how many players there are, that way we dont have to share the num_entities variable
	void BuildSnapshot(ServerSnap* ss, int num_entities, PlayerBase** entities, EntityManagerBase* entityManager);

	NetMsg* BuildDeltaSnapshot(EntityManagerBase* manager);
};


#endif