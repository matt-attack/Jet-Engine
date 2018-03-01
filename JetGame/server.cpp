#include "server.h"

#include <JetNet/Sockets.h>

//#include "Entities/PlayerEntity.h"
//#include "Util/CThread.h"
#include "NetPackets.h"
#include <JetEngine/ResourceManager.h>
//include "Weapon.h"

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
	this->time = 0;
	this->player_count = 0;
	this->lastclear = 0;
	this->lasttick = 0;
	this->max_players = 15;//todo: is this even used? make this configurable
	this->tickrate = tickrate;

	this->localserver = false;

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

	//TODO, LOAD CONFIG FILE
	if (this->localserver)
	{
		this->port = 5007;
	}
	else
	{
		FILE* f = fopen("server.cfg", "r");
		if (f)
		{
			printf("found config file\n");
			fseek(f, 0, SEEK_END);   // non-portable
			int size = ftell(f);
			fseek(f, 0, SEEK_SET);
			char* d = new char[size];
			fread(d, size, 1, f);

			char* pch = strtok(d, "\n");
			while (pch != NULL)
			{
				printf("%s\n", pch);
				if (strncmp(pch, "name:", 5) == 0)
				{
					this->SetName(&pch[6]);
				}
				else if (strncmp(pch, "port:", 5) == 0)
				{
					float port = atof(&pch[6]);
					this->port = (int)port;
				}
				pch = strtok(NULL, "\n");
			}

			fclose(f);
			delete[] d;
		}
		else
		{
			printf("no config file found, creating default\n");
			f = fopen("server.cfg", "w+");
			char str[512];
			strcpy(str, "name: Mattcraft Server\nport: 5007\nother: stuff\n");
			fwrite(str, strlen(str), 1, f);
			fclose(f);

			char* pch = strtok(str, "\n");
			while (pch != NULL)
			{
				printf("%s\n", pch);
				if (strncmp(pch, "name:", 5) == 0)
				{
					this->SetName(&pch[6]);
				}
				else if (strncmp(pch, "port:", 5) == 0)
				{
					float port = atof(&pch[6]);
					this->port = (int)port;
				}
				pch = strtok(NULL, "\n");
			}
		}
	}

	//todo: magic max player count here
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

	//log("Server Started...");
	printf("Server Started\n");

	this->time = 0.0f;

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
				//CPlayerEntity* p = new CPlayerEntity;
				//this->EntityManager.AddEntity(p, i);
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
		packet.localplyid = c->entities[i]->GetID();// c->id;
		packet.time = this->time;
		packet.id = 1945;
		packet.planet = c->entities[i]->GetParent();//todo: remove this
		Vec3 p = c->entities[i]->GetPosition();

		packet.x = p.x;
		packet.y = p.y;
		packet.z = p.z;
		//need to send this immediately, add Out of Band function to instantly send
		c->peer->SendReliable((char*)&packet, sizeof(InitialPacket));

		//need an on connect hook to send other info

		/* Send Info About Joining Client To Every Other Player */
		PlayerInitialUpdate pi;
		pi.packetid = 5;
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
			p.packetid = 5;
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

		//client->Save();

		//prevents crashes (this was moved to player class)
		//if (entity->vehicle)
		//	static_cast<IVehicle*>(entity->vehicle)->activeply = 0;

		//remove the player
		PlayerDisconnect p;
		p.id = 99;
		p.plyid = entity->GetID();// client->id;
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

	//srv = this;//update this for effects

	this->timer.Update();
	float dT = timer.GetElapsedTime();
	this->time += dT;

	//this->EntityManager.time = this->time;

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
		continue;

		/*if (packetID == 21)//mech config
		{
			MechLoadout* loadout = (MechLoadout*)buffer;
			printf("got mechconfig packet\n");

			CPlayerEntity* ent = client->entities[0];//todo, make this work on other than the first player
			//apply it

			if (ent->vehicle == 0 || loadout->model != ent->vehicle->_model_s)
			{
				if (ent->vehicle)
					ent->vehicle->Remove();

				//make new mech entity and set it as my vehicle
				CMechVehicle* mech = new CMechVehicle;
				mech->manager = ent->manager;
				//mech->system = ent->system;
				mech->SetPosition(ent->position);
				mech->SetModel(loadout->model);
				ent->mech_model = mech->_model_data->name.substr(8, mech->_model_data->name.length() - 8);
				ent->SetVehicle(mech);
				ent->manager->AddEntity(mech);
			}

			for (int i = 0; i < 28; i++)
			{
				if (ent->slots[i] && loadout->itemid[i] == 0)
				{
					delete ent->slots[i];
					ent->slots[i] = 0;
				}
				else if (ent->slots[i])
				{
					delete ent->slots[i];
					ent->slots[i] = CreateWeapon<Weapon>(loadout->itemid[i]);
				}
				else if (loadout->itemid[i])
				{
					ent->slots[i] = CreateWeapon<Weapon>(loadout->itemid[i]);
				}
			}
		}
		else if (packetID == 42)
		{
			log("got client command: ");
			NetMsg m = NetMsg(2048, (char*)buffer + 1);
			int id = m.ReadInt();

			char t[150];
			m.ReadString(t, 150);
			log(t);
			log("\n");

			if (strncmp(t, "say ", 4) == 0)
			{
				char t2[500];
				char* msg = &t[4];
				strcpy(t2, client->entities[0]->name);
				strcat(t2, ": ");
				strcat(t2, msg);
				printf("%s\n", t2);

				this->ChatPrint(t2);

				//read "console" commands
				char* cmp = "!";
				if (msg[0] == cmp[0])
				{
					this->ConsoleCommand(client, &msg[1]);
				}
			}
			else if (strncmp(t, "attack ", 7) == 0)
			{
			}
			else if (strncmp(t, "set ", 4) == 0)
			{
				char n[500]; float value;
				sscanf(&t[4], "%s %f", &n, &value);
				client->entities[0]->SetCVar(n, value, false);

				logf("Set ClientVariable: '%s' to %f\n", n, value);
			}
			else if (strncmp(t, "order ", 6) == 0)
			{
				
			}
		}
		else
		{
			logf("Packet %d Recieved but not Handled from %i.%i.%i.%i:%i!\n", packetID, client->address.GetA(), client->address.GetB(), client->address.GetC(), client->address.GetD(), client->address.GetPort());
		}*/
		delete[] buffer;
	}

	//tests net and interpolation code for issues caused by entities being added/removed
	/*if (this->EntityManager->GetEntByID(15))
	{
	this->EntityManager->RemoveEntity(this->EntityManager->GetEntByID(15));
	this->EntityManager->RemoveEntity(this->EntityManager->GetEntByID(16));
	CDropShip* ship = new CDropShip(1);
	ship->SetPosition(0, 60, 0);
	ship->planet = this->system->planets[2];
	//ship->Dropoff(client->entity->position+client->entity->GetGravity()*-5.0f,client->entity->planet,5);
	//ship->Navigate(Vec3(500,0,0),0);//client->entity->planet);
	this->EntityManager->AddEntity(ship, 16);
	ship->Init();
	}
	else
	{
	this->EntityManager->RemoveEntity(this->EntityManager->GetEntByID(16));
	CTestEntity* ent = new CTestEntity;
	ent->Init();
	ent->SetPosition(0,50,0);
	ent->planet = this->system->planets[2];
	this->EntityManager->AddEntity(ent,15);
	ent = new CTestEntity;
	ent->Init();
	ent->SetPosition(0,50,5);
	ent->planet = this->system->planets[2];
	this->EntityManager->AddEntity(ent,16);
	}*/


	//do per client updates
	this->CalculatePings();

	//send heartbeat to master server list
	if (GetTickCount() > this->lastclear + 110000)
	{
		lastclear = GetTickCount();

		this->HeartBeat();
	}

	this->OnUpdate();

	//check for win conditions and what not
	/*if (this->system.GetTeam(1)->health == 0)
	{
	if (this->system.planets[2]->bases[0]->team == 2 || this->system.GetTeam(1)->health <= 0)
	{
	//we lost
	if (this->gameoverTimer == 0)
	{
	this->gameoverTimer = 10;
	this->ChatPrint("Game over!");

	for (auto ii: this->connection.peers)
	{
	Client* cl = (Client*)ii.second->data;
	if (cl->entity->team == 1)
	cl->entity->health = 0;
	}
	}
	gameoverTimer -= dT;
	}
	}
	if (this->system.GetTeam(2)->health == 0)
	{
	if (this->system.planets[1]->bases[0]->team == 1 || this->system.GetTeam(2)->health <= 0)
	{
	if (this->gameoverTimer == 0)
	{
	this->gameoverTimer = 10;
	this->ChatPrint("Game over!");

	for (auto ii: this->connection.peers)
	{
	Client* cl = (Client*)ii.second->data;
	if (cl->entity->team == 2)
	cl->entity->health = 0;
	}
	}
	gameoverTimer -= dT;
	}
	}*/

	if (false)//this->gameoverTimer < 0)
	{
		this->ChangeMap();
		return 0;
	}

	//ok, lets update ents here
	if (this->connection.peers.size() > 0)//dont bother to update if we do not have any players
		this->EntityManager->DoUpdate(dT);

	//build global snapshot entity list
	if (this->connection.peers.size() > 0)
		this->networker.BuildEntityData(this->EntityManager, this->time);

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

#ifdef MATT_SERVER
#ifdef _WIN32
	char title[60];
	sprintf(title, "Mattcraft: %s %d/%d Players", this->name, this->connection.peers.size(), this->max_players);
	SetConsoleTitleA(title);
#endif
#endif

	//force out the packets
	this->connection.SendPackets();

	return 0;
}

void Server::ShutDown()
{
	//this calls disconnect on all peers
	this->connection.Close();

	//this->EntityManager->RemoveAll();

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
		if (cl->entities[0])
			cl->entities[0]->ping = cl->networker.ping;
	}
}

void Server::HeartBeat()
{
	static const char *BODY = "";
	char Name[256];
	int len = this->name.length();// strlen(this->name);
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
	this->listenserver_state = 2;
	this->StartUp();
}