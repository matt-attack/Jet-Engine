#ifndef SERVER_HEADER
#define SERVER_HEADER

#include <JetNet/Sockets.h>
#include <JetNet/Connection.h>

#include <JetEngine/Util/CThread.h>
#include <JetEngine/Util/Profile.h>

#include "NetPackets.h"
#include <JetEngine/Util/CTimer.h>
#include <JetGame/Snapshot.h>
#include <JetGame/Networking.h>

class Client;

class Server: public IServer
{
public:
	CTimer timer;
private:

	//configuration info
	int port;
	std::string name;
	unsigned int max_players;

	bool use_tcp;
	SocketTCP tcp;

	unsigned int lasttick;
	unsigned int lastclear;

public:
	NetConnection connection;
	ServerNetworker networker;
	
	unsigned int tickrate;//how many updates are performed in a second
	int listenserver_state;
	bool localserver;

	int player_count;
	PlayerBase* players[MaxPlayers];//this is a silly arbitrary limit, we should have a define for this

	EntityManagerBase* EntityManager;

	double time;

	Server(EntityManagerBase* manager, int tickrate, bool use_tcp);
	~Server();

	int StartUp();
	int Update();

	void ShutDown();

	void ChangeMap();

	virtual void OnConnect(Peer* client, ConnectionRequest* p);
	virtual void OnDisconnect(Peer* client);

	virtual char* CanConnect(Address addr, ConnectionRequest* p);

	struct latestdata
	{
		int entid;
		Vec3 position;
		float frame;
		short anim;
	};

	void CalculatePings();
	
	//generic functions
	void ChatPrint(const char* str, PlayerBase* ent = 0);

	PlayerBase* GetPlayerByName(char* name);
	void SetName(char* n)
	{
		this->name = n;
	}

	void HeartBeat();//sends info to master server

	//virtual methods of overrides
	virtual void OnUpdate(float dT) = 0;
	virtual void OnPreUpdate() = 0;
	virtual void OnStartup() = 0;
	virtual void OnShutdown() = 0;

	virtual void TCPThread(Client* client) {};

	virtual void OnDisconnect(Client* client) = 0;
	virtual void OnMessage(Client* client, const char* buffer, int size) = 0;
};

class Client
{
	
public:
	ServerClientNetworker networker;

	int state;//current state of the client

	Address address;// Client's address
	
	int lastupdate; //Timestamp of last update

	// Number of local players on this client
	int num_entities;
	PlayerBase* entities[MaxLocalPlayers];//maximum number of local players will always be 4

	Peer* peer;

	SocketTCP* tcpcon = 0;
	CThread thread;

	Client()
	{
		this->num_entities = 0;
		this->state = 0;//0 = loading, 1 = loaded
		for (int i = 0; i < MaxLocalPlayers; i++)
			entities[i] = 0;
	}

	void BuildSnapsot(Server* server)
	{
		this->networker.BuildSnapshot(server->networker.currentSnapshot, this->num_entities, entities, server->EntityManager);
	}
};

#endif