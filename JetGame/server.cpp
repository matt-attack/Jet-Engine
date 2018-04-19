#include "server.h"

#include <JetNet/Sockets.h>

#include "NetPackets.h"
#include <JetEngine/ResourceManager.h>

#include <stdio.h>
#include <queue>

THREAD_RETURN ServerThread(THREAD_INPUT lpParam)
{
	struct ThreadData
	{
		Server* server;
		Client* client;
	};
	ThreadData* td = (ThreadData*)lpParam;
	Client* client = td->client;
	Server* server = td->server;
	SocketTCP* sock = client->tcpcon;

	//ok, this needs a time out
	//sock->SetTimeout(2000);
	Address sender;

	SocketTCP* n = sock->Accept(sender);
	if (n == 0)
	{
		printf("TCP Client Accept Failed\n");
		delete td;

		return 0;
	}
	client->tcpcon = sock = n;
	//sock->SetTimeout(0);

	printf("Accepted TCP Connection from %i.%i.%i.%i:%i!\n", sender.GetA(), sender.GetB(), sender.GetC(), sender.GetD(), sender.GetPort());

	//call some kind of tcp thread function now
	server->TCPThread(client);

	delete td;

	return 0;
}

Server::Server(EntityManagerBase* manager, int tickrate, bool use_tcp) : use_tcp(use_tcp), EntityManager(manager)
{
	this->time_of_day = 0;
	this->player_count = 0;
	this->lastclear = 0;
	this->lasttick = 0;
	this->max_players = 15;//todo: is this even used? make this configurable
	this->tickrate = tickrate;

	this->name = "Local Multiplayer";
}

Server::~Server()
{
}

int Server::StartUp()
{
	int seed = 0;
	if (seed == 0)
		seed = GetTickCount();

	this->port = SERVER_PORT;

	for (int i = 0; i < MaxPlayers; i++)
		this->players[i] = 0;

	const int ports = this->port;

	this->connection.SetObserver(this);
	this->connection.Open(ports, this->max_players);
#ifdef _DEBUG
	this->connection.SetTimeout(120000);
#else
	this->connection.SetTimeout(15000);
#endif

	if (use_tcp && !tcp.Listen(ports))
	{
		printf("failed to start listening for tcp connections\n");
	}

	printf("Server Started\n");

	this->OnStartup();

	this->timer.Start();
	return 0;
}

int rand(int a, int b)
{
	int diff = b - a;
	int v = rand() % diff;

	return a + v;
}

#ifdef _WIN32
DWORD WINAPI server_ping(LPVOID data)
#else
void* server_ping(void* data)
#endif
{
	printf("started ping!!!\n");
	SocketTCP s;
	s.Connect("matchmaking.net46.net");
	s.SetTimeout(1600);

	char* HEADERS = (char*)data;
	char* BODY = "";

	char* buffer = (char*)malloc(strlen(BODY) + strlen(HEADERS) + 256);
	sprintf(buffer, "%s\r\n", (char*)HEADERS);
	if (s.Send(buffer, strlen(buffer)) == false)
	{
		perror("send");
	}
	else
	{
		char recvbuffer[8192] = { 0 };
		int recvcount = 0;
		recvcount = s.Receive(recvbuffer, sizeof(recvbuffer));
		while (recvcount > 0)
		{
			recvcount = s.Receive(recvbuffer, sizeof(recvbuffer));
			//log(recvbuffer);
		}
	}
	free(buffer);
	s.Close();
	printf("ping thread exited\n");
	delete[] HEADERS;
	return 0;
}

void Server::OnConnect(Peer* peer, ConnectionRequest* p)
{
	Client* c = new Client;
	c->lastupdate = GetTickCount();
	c->address = peer->remoteaddr;
	c->peer = peer;
	peer->data = c;

	//add the new player
	std::string name = p->plyname;
	//fix up name
	int l = name.length();
	for (int i = 0; i < l; i++)
	{
		if (name[i] == 13)
			name[i] = 0;
	}

	// Have each local player join
	c->num_entities = p->players;
	for (int pi = 0; pi < p->players; pi++)
	{
		//tell people that they joined
		char x[100];
		if (pi >= 1)
		{
			name = p->plyname;
			name += "(" + std::to_string(pi) + ")";
		}
		sprintf(x, "%s joined game", name.c_str());
		ChatPrint(x);

		bool found;
		for (int i = 0; i < this->max_players; i++)
		{
			if (players[i] == 0)
			{
				PlayerBase* p = this->EntityManager->CreatePlayer(i);

				c->entities[pi] = p;
				strncpy(c->entities[pi]->name, name.c_str(), 25);

				players[i] = p;//insert into list of players

				this->player_count++;//increment player count

				// Spawn and set player defaults
				p->OnJoin();

				found = true;
				break;
			}
		}

		//this shouldnt get hit, need to prevent it
		if (found == false)
		{
			printf("Player could not join, no open slots! FIX ME THIS BREAKS EVERYTHING\n");
			delete c;
			throw 7;
			break;
		}
	}

	for (int i = 0; i < p->players; i++)
	{
		/* send intial packet */
		InitialPacket packet;
		packet.localplyid = c->entities[i]->GetID();
		packet.time = this->time_of_day;
		packet.id = NetworkingPackets::InitialPacket;
		packet.parent = c->entities[i]->GetParent();//todo: remove this
		Vec3 p = c->entities[i]->GetPosition();

		packet.x = p.x;
		packet.y = p.y;
		packet.z = p.z;
		//need to send this immediately, add Out of Band function to instantly send
		c->peer->SendReliable((char*)&packet, sizeof(InitialPacket));

		//need an on connect hook to send other info

		/* Send Info About Joining Client To Every Other Player */
		PlayerInitialUpdate pi;
		pi.packetid = NetworkingPackets::AddPlayer;
		strncpy(pi.name, c->entities[i]->name, 25);
		pi.playerid = c->entities[i]->GetID();
		Vec3 pos = c->entities[i]->GetPosition();
		pi.x = pos.x;
		pi.y = pos.y;
		pi.z = pos.z;
		for (auto ii = this->connection.peers.begin(); ii != this->connection.peers.end(); ++ii)
		{
			if (ii->first != c->address)
			{
				ii->second->SendReliable((char*)&pi, sizeof(PlayerInitialUpdate));
			}
		}
	}

	/* Setup TCP Socket*/
	if (use_tcp)
	{
		SocketTCP* n = &this->tcp;
		c->tcpcon = n;

		/* create thread to handle connection */
		struct ThreadData
		{
			Server* server;
			Client* client;
		};
		ThreadData* td = new ThreadData;
		td->server = this;
		td->client = c;
		c->thread.Start(ServerThread, td);
	}

	c->state = 1;

	/* Send Info About All Other Clients to the One Who Is Joining */
	for (int i = 0; i < this->max_players; i++)
	{
		if (players[i] != 0)
		{
			PlayerInitialUpdate p;
			p.packetid = NetworkingPackets::AddPlayer;
			strcpy(p.name, players[i]->name);
			p.playerid = i;
			Vec3 pos = players[i]->GetPosition();
			p.x = pos.x;
			p.y = pos.y;
			p.z = pos.z;
			c->peer->SendReliable((char*)&p, sizeof(PlayerInitialUpdate));
			printf("sent player #%d\n", i);
		}
	}

	this->HeartBeat();//alert master server to having a new player
}

void Server::OnDisconnect(Peer* peer)
{
	Client* client = (Client*)peer->data;

	this->OnDisconnect(client);

	for (auto entity : client->entities)
	{
		if (entity == 0)
			continue;

		char x[100];
		sprintf(x, "'%s' Disconnected", entity->name);
		ChatPrint(x);

		//remove the player
		PlayerDisconnect p;
		p.id = NetworkingPackets::PlayerDisconnect;
		p.plyid = entity->GetID();
		for (auto ii = this->connection.peers.begin(); ii != this->connection.peers.end(); ++ii)
		{
			//lel, wut
			this->connection.Send(ii->second, (char*)&p, sizeof(PlayerDisconnect));
		}

		//remove player entity
		this->EntityManager->RemoveEntity(entity->GetID());
		players[entity->GetID()] = 0;
	}

	this->player_count--;

	delete client;
}

char* Server::CanConnect(Address addr, ConnectionRequest* p)
{
	//check if enough room
	for (int i = 0; i < this->max_players; i++)
	{
		if (players[i] == 0)
		{
			//theres an empty spot, we can join
			return 0;
		}
	}

	printf("Player could not join, no open slots!\n");

	return "No Open Slots";
}

int Server::Update()
{
	PROFILE("Server::Update");

	std::lock_guard<std::mutex> lg(resources.reload_lock);

	this->timer.Update();
	float dT = timer.GetElapsedTime();

	this->OnPreUpdate();

	char* buffer;
	int recvsize = 0;
	Peer* peer;
	while (buffer = this->connection.Receive(peer, recvsize))
	{
		Client* client = (Client*)peer->data;
		client->lastupdate = GetTickCount();//update timeout counter
		int packetID = buffer[0];
		if (packetID == 1)//player snapshot
		{
			PlayerUpdatePacket* p = (PlayerUpdatePacket*)buffer;
			int id = p->playerID;
			PlayerBase* ply = 0;
			int i;
			for (i = 0; i < client->num_entities; i++)
			{
				if (client->entities[i]->GetID() == id)
				{
					ply = client->entities[i];
					break;
				}
			}

			if (p == 0)
				throw 7;

			if (client->networker.ProcessMove(i, p) == false)
				continue;//it was old

			ply->ProcessSnapshot(p);
		}
		else
		{
			this->OnMessage(client, buffer, recvsize);
		}
		delete[] buffer;
	}

	//do per client updates
	this->CalculatePings();

	//send heartbeat to master server list
	if (GetTickCount() > this->lastclear + 110000)
	{
		lastclear = GetTickCount();

		this->HeartBeat();
	}

	// Run the per game update function
	this->OnUpdate(dT);

	if (false)//this->gameoverTimer < 0)
	{
		this->ChangeMap();
		return 0;
	}

	//build global snapshot entity list
	if (this->connection.peers.size() > 0)
		this->networker.BuildEntityData(this->EntityManager, this->time_of_day);

	//update snapshots
	for (auto ii = this->connection.peers.begin(); ii != this->connection.peers.end(); ++ii)
	{
		Client* client = (Client*)ii->second->data;
		for (int i = 0; i < client->num_entities; i++)
		{
			client->entities[i]->_lastposition = client->entities[i]->GetPosition();
			client->entities[i]->_lastaimdir = client->entities[i]->GetAimDir();
		}

		if (client->state == 0)//make sure client has joined
			continue;

		client->BuildSnapsot(this);

		//send snapshot
		NetMsg* rawsnap = client->networker.BuildDeltaSnapshot(this->EntityManager);//crashes with large sizes for some reason
		this->connection.Send(ii->second, rawsnap->data, rawsnap->cursize);

		delete[] rawsnap->data;
		delete rawsnap;
	}

	//force out the packets
	this->connection.SendPackets();

	return 0;
}

void Server::ShutDown()
{
	//this calls disconnect on all peers
	this->connection.Close();

	Sleep(100);//let messages go out

	printf("Server shut down...\n");
}

void Server::ChatPrint(const char* txt, PlayerBase* ent)
{
	printf("[Server]%s\n", txt);
	char buffer[200];
	NetMsg msg(200, (char*)&buffer);
	msg.WriteByte(30);
	msg.WriteString(txt, 199);
	for (auto ii = this->connection.peers.begin(); ii != this->connection.peers.end(); ++ii)
	{
		if (ent && ((Client*)ii->second->data)->entities[0] == ent)
		{
			ii->second->SendReliable((char*)&buffer, msg.cursize);
			break;
		}

		if (ent == 0)
			ii->second->SendReliable((char*)&buffer, msg.cursize);
	}
}

PlayerBase* Server::GetPlayerByName(char* name)
{
	for (int i = 0; i < MaxPlayers; i++)
	{
		if (this->players[i])
		{
			if (strcmp(this->players[i]->name, name) == 0)
				return this->players[i];
		}
	}
	return 0;
}

void Server::CalculatePings()
{
	for (auto ii = this->connection.peers.begin(); ii != this->connection.peers.end(); ii++)
	{
		Client* cl = (Client*)ii->second->data;
		cl->networker.CalculatePing();
		for (int i = 0; i < cl->num_entities; i++)
			if (cl->entities[i])
				cl->entities[i]->ping = cl->networker.ping;
	}
}

void Server::HeartBeat()
{
	static const char *BODY = "";
	char Name[256];
	int len = this->name.length();
	for (int i = 0; i < len + 1; i++)
	{
		if (this->name[i])
		{
			if (this->name[i] == 32)//space
				Name[i] = 43;
			else
				Name[i] = this->name[i];
		}
		else
		{
			Name[i] = 0;
			break;
		}
	}
	char* headers = new char[512];
	sprintf(headers, "GET http://matchmaking.net46.net/postServer.php?game=%s&version=1.0&port=%i&players=%i HTTP/1.0\r\nUser-Agent: game\r\n", Name, this->port, this->player_count);

	CThread thread;
#ifndef ANDROID
	thread.Start(server_ping, headers);
#else
	thread.Start(&server_ping, headers);
#endif
}

void Server::ChangeMap()
{
	//send messages to all players to disconnect and reconnect
	for (auto cl : this->connection.peers)
	{
		auto client = (Client*)cl.second->data;
		char oob = 66;//tells client to disconnect then reconnect
		for (int i = 0; i < 3; i++)//send three times to be sure
			cl.second->SendOOB(&oob, 1);
	}
	this->ChatPrint("Changing Map!");

	//now I need to restart
	//need signal to 
	Sleep(1000);
	this->ShutDown();
	Sleep(2000);//give time for players to disconnect
	//this->listenserver_state = 2;
	this->StartUp();
}