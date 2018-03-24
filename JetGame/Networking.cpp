#include "Networking.h"
#include "JetEngine/Util/Profile.h"
#include "JetEngine/Graphics/CRenderer.h"

void DecodeDeltaEntity(NetMsg* msg, Snapshot* newframe, int num, ED* oldstate, bool full, int newindex)
{
	if (full)
	{
		/*LARGE_INTEGER start, stop, freq;
		QueryPerformanceCounter(&start);*/

		//logf("[Client] unchanged entity %d\n", oldstate->id);
		//just copy over old value
		newframe->entdata[newindex].id = oldstate->id;
		newframe->entdata[newindex].classid = oldstate->classid;
		memcpy(newframe->entdata[newindex].data, oldstate->data, sizeof(oldstate->data));

		/*QueryPerformanceCounter(&stop);
		QueryPerformanceFrequency(&freq);
		float elapsed = (float)(stop.QuadPart - start.QuadPart)/ (float)freq.QuadPart;
		char sz[150];
		sprintf(sz, "it took %0.9f sec to copy one ent data\n", elapsed);
		OutputDebugString(sz);*/
		return;
	}

	if (oldstate == 0)
	{
		//logf("[Client] got new entity message for %d\n", num);

		//full update
		int i = 0;
		ED* newent = &newframe->entdata[newindex];
		newent->id = num;
		newent->classid = msg->ReadInt();
		int off = 0;

		netField_t* fields = (*GetDataTable())[newent->classid];
		while (fields[i].bits != FIELD_END)
		{
			char* d = (char*)newent->data + off;
			msg->ReadData(d, fields[i].bits);
			off += fields[i].bits;
			i++;
		}

		return;
	}

	unsigned char command = msg->ReadByte();
	if (command == 0)
	{
		//entity was deleted
	}
	else
	{
		//logf("[Client] got delta message for %d\n", oldstate->id);

		/*LARGE_INTEGER start, stop, freq;
		QueryPerformanceCounter(&start);*/

		ED* newent = &newframe->entdata[newindex];
		newent->id = oldstate->id;
		newent->classid = oldstate->classid;
		netField_t* fields = (*GetDataTable())[newent->classid];

		memcpy(newent->data, oldstate->data, sizeof(oldstate->data));

		int i = 0;
		int off = 0;
		while (fields[i].bits != FIELD_END)
		{
			if (command == 254)
				break;

			if (i == (command - 1))
			{
				//logf("[Client]   %s changed\n", fields[i].name);
				char* d = (char*)newent->data + off;
				msg->ReadData(d, fields[i].bits);
				command = msg->ReadByte();
			}

			off += fields[i].bits;
			i++;
		}

		/*QueryPerformanceCounter(&stop);
		QueryPerformanceFrequency(&freq);
		float elapsed = (float)(stop.QuadPart - start.QuadPart)/ (float)freq.QuadPart;
		char sz[150];
		sprintf(sz, "it took %0.9f sec to decode one ent data\n", elapsed);
		OutputDebugString(sz);*/
	}
}

void WriteDeltaEntity(NetMsg* msg, ED* oldent, ED* newent, bool full)
{
	if (newent == 0)//entity was removed
	{
		//printf("[Server] wrote delete with id %d\n", oldent->id);

		msg->WriteShort(oldent->id);
		msg->WriteByte(0);
		return;
	}

	netField_t* fields = (*GetDataTable())[newent->classid];
	if (oldent == 0)//send all data, new entity
	{
		//printf("[Server] wrote full with id %d\n", newent->id);

		msg->WriteShort(newent->id);
		msg->WriteInt(newent->classid);

		//send all data
		int i = 0;
		int off = 0;
		while (fields[i].bits != FIELD_END)
		{
			char* d = (char*)newent->data + off;
			msg->WriteData(d, fields[i].bits);
			off += fields[i].bits;
			i++;
		}
		return;
	}

	//send changed data, delta
	int i = 0;
	int off = 0;
	bool sentid = false;
	while (fields[i].bits != FIELD_END)
	{
		char* d = (char*)newent->data + off;
		char *od = (char*)oldent->data + off;
		if (memcmp(d, od, fields[i].bits) != 0)//are they different?
		{
			if (sentid == false)
			{
				//printf("[Server] wrote delta with id %d\n", newent->id);
				msg->WriteShort(newent->id);
				//msg->WriteInt(newent->classid);
				sentid = true;
			}

			//printf( "[Server]   %s changed\n", fields[i].name);

			msg->WriteByte(i + 1);
			msg->WriteData(d, fields[i].bits);
		}
		off += fields[i].bits;
		i++;
	}
	if (sentid)
		msg->WriteByte(254);//tell it to stop looking for change data
}

bool ClientNetworker::Update(float dT)
{
	char* buffer;
	int recvsize = 0;

	int last = this->lastSnapshotNum;
	Peer* sender;
	while (buffer = this->connection.Receive(sender, recvsize))
	{
		this->ProcessPacket(buffer[0], recvsize, buffer);
		this->message_callback(buffer[0], recvsize, buffer);

		//this->ProcessUDPPacket((char)buffer[0], recvsize, buffer);
		if ((char)buffer[0] == 66)
		{
			//we got kill packet
			printf("Got Newgame Packet\n");
			return true;
		}
		delete[] buffer;
	}
	if (last == this->lastSnapshotNum)
		renderer->AddPoint(0.01f);//add a filler if no snapshot recieved

	this->UpdateCommands();


	//we want the time of render to be
	unsigned int irt = GetTickCount() - (unsigned int)this->local_players[0]->GetCVar("interpolate");//1000;//add interpolation delay of ~300ms
	double rt = (double)irt / 1000.0;

	//find the packets with the time > than rt and < rt that are closest to rt and get their times
	Snapshot *min = 0;
	Snapshot *max = 0;
	if (this->GetInterpolationSnapshots(irt, &min, &max))
	{
		//packet receive times
		//todo: stop using packet receive time
		double oldtime = ((double)min->time) / 1000.0;
		double newtime = ((double)max->time) / 1000.0;
		double dt = newtime - oldtime;//seems correct

		double diff = rt - oldtime;
		double fraction = diff / dt;

		double real = max->time - min->time;
		real /= 1000;
		//printf("Interp Calculated: %f Sent: %f Real: %f\n", dt, (max->timef - min->timef), real);
		//last = GetTickCount();

		//interpolate between the two snapshots
		this->EntityManager->Interpolate(min, max, fraction);
	}

	//ok, lets make player position correction here
	this->CorrectPlayerPos(last, this->local_players, this->num_local_players);

	return false;
}

int ClientNetworker::Connect(Address serveraddr, unsigned short cport, const char* plyname, const char* password, int players, char** status)
{
	lastSnapshotNum = 0;
	droppedPackets = 0;
	lastAckedMove = -1;
	ping = 0;
	num_local_players = players;

	for (int i = 0; i < PACKET_BACKUP; i++)
	{
		this->framescache[i].entdata = 0;
		this->framescache[i].player_snapshots = 0;
	}

	this->connection.Open(cport, 0);
	this->connection.SetTimeout(99999);

	int result = this->connection.Connect(serveraddr, plyname, password, this->num_local_players, status);
	if (result != -1)
	{
		Peer* peer = this->connection.peers[serveraddr];

		if (status)
			status[0] = "UDP Connection Established";

		if (this->use_tcp)
		{
			Sleep(1000);
			status[0] = "Establishing TCP Connection...";

			//try and get a TCP connection going for chunks, and reliable data
			bool success = this->data_connection.Connect(peer->remoteaddr);

			if (success)
			{
				if (status)
					status[0] = "TCP Connection Established";
				Sleep(1000);
			}
			else
			{
				if (status)
					status[0] = "An Error Occurred While Connecting...";
				Sleep(1000);

				//remove the peer
				this->connection.Disconnect();// peer);

				return -1;
			}
		}

		Sleep(1000);

		//ok, idle around for first packet
		Peer* sender;
		int size = 0;
		char* data;
		int joins_recieved = 0;
		while (true)
		{
			while ((data = connection.Receive(sender, size)) == 0) { Sleep(0); }

			InitialPacket *pack = (InitialPacket*)(data);
			if (pack && pack->id == 1945)
			{
				//this->TOD = pack->tod;
				//todo make initial packet handle more than one player
				PlayerBase* player = this->local_players[joins_recieved];

				//store local player id
				player->AllocateMoves();//allocate since we are a local player
				player->islocalplayer = true;
				player->SetPosition(Vec3(pack->x, pack->y, pack->z));

				//spawn the player
				auto ent = this->local_players[joins_recieved];
				ent->islocalplayer = true;
				//ent->velocity = Vec3(0, 0, 0);
				//ent->cam.quat = Quaternion::IDENTITY;
				this->EntityManager->AddEntity(ent->Base(), pack->localplyid);

				//this doesnt work right because we havent loaded planets...
				player->SetParent(pack->planet);

				joins_recieved++;
				delete[] data;

				if (joins_recieved == this->num_local_players)
					break;
			}
			else
			{
				this->ProcessPacket(data[0], size, data);
				this->message_callback(data[0], size, data);

				delete[] data;
			}
		}
	}

	if (result == -1)
		this->connection.Close();

	return result;
}

Snapshot* ClientNetworker::DecodeDeltaSnapshot(NetMsg* msg)
{
	PROFILE("DecodeDeltaSnapshot");
	msg->ReadByte();//get past id
	double time = msg->ReadDouble();
	float tod = msg->ReadFloat();

	int acked_move = msg->ReadInt();
	this->lastAckedMove = acked_move;

	int framenum = msg->ReadInt();

	Snapshot* newsnap = &framescache[framenum & PACKET_MASK];
	Snapshot* oldsnap = 0;

	int from = msg->ReadInt();//read from id
	if (from != 0)
		oldsnap = &framescache[from & PACKET_MASK];//need to cache old frames

	//todo: why is this here?
	if (oldsnap)
		memcpy(newsnap, oldsnap, 16);//sizeof(Snapshot));

	newsnap->timef = time;
	newsnap->tod = tod;
	newsnap->framenumber = framenum;

	/*    read player data      */
	unsigned short ping = msg->ReadShort();
	this->ping = ping;

	//read inventory
	//if (oldsnap)
	//newsnap->player_snapshots = oldsnap->player_snapshots;// memcpy(newsnap->inventoryItems, oldsnap->inventoryItems, sizeof(Snapshot::inv_item) * 28);
	//else
	//memset(newsnap->inventoryItems, 0, sizeof(Snapshot::inv_item)*28);

	//number of local players in this snapshot
	int numlocalplayers = msg->ReadByte();

	newsnap->num_player_snapshots = numlocalplayers;
	delete[] newsnap->player_snapshots;
	newsnap->player_snapshots = new PlayerSnapshotData[numlocalplayers];
	for (int i = 0; i < numlocalplayers; i++)
	{
		PlayerSnapshot psnap;

		if (oldsnap == 0)
			memset(psnap.inventoryItems, 0, sizeof(PlayerSnapshot::inv_item) * 28);
		else
			memcpy(psnap.inventoryItems, oldsnap->player_snapshots[i].data.get(), sizeof(PlayerSnapshot::inv_item) * 28);

		//do this per local player
		//move to a decode playerdelta function to mirror the server
		byte id = msg->ReadByte();
		while (id != 254)
		{
			int itemid = msg->ReadInt();
			psnap.inventoryItems[id].id = itemid;
			psnap.inventoryItems[id].ammo = msg->ReadShort();
			psnap.inventoryItems[id].clip = msg->ReadShort();
			psnap.inventoryItems[id].entity = msg->ReadShort();

			id = msg->ReadByte();
		}

		newsnap->AddPlayerSnapshot(i, psnap);
	}

	unsigned short entcount = msg->ReadShort();//entcount
	newsnap->entcount = entcount;

	if (newsnap->entdata)
		delete[] newsnap->entdata;
	newsnap->entdata = new ED[entcount];

	int newnum; int newindex;
	ED* oldstate;
	int oldindex, oldnum;

	//newframe->parseEntitiesNum = cl.parseEntitiesNum;
	//newsnap->entcount = 0;
	newindex = 0;

	// delta from the entities present in oldframe
	oldindex = 0;
	oldstate = NULL;
	if (!oldsnap)
	{
		//OutputDebugString("no old snap");
		oldnum = MAX_PACKET_ENTITIES;
	}
	else
	{
		if (oldindex >= oldsnap->entcount)
		{
			//OutputDebugString("old>old->entcount");
			oldnum = MAX_PACKET_ENTITIES;
		}
		else
		{
			oldstate = &oldsnap->entdata[oldindex];//oldstate = &oldsnap->ents[oldindex];//&cl.parseEntities[(oldframe->parseEntitiesNum + oldindex) & (MAX_PARSE_ENTITIES-1)];
			oldnum = oldstate->id;
		}
	}
	//LARGE_INTEGER start, stop, freq;
	//QueryPerformanceCounter(&start);

	while (1)
	{
		// read the entity index number
		newnum = msg->ReadShort();//MSG_ReadBits( msg, GENTITYNUM_BITS );

		if (newnum == MAX_PACKET_ENTITIES)//(MAX_GENTITIES-1) )
		{
			break;
		}
		else if (newnum > MAX_PACKET_ENTITIES)
		{
			printf("CL_ParsePacketEntities: ERROR: Snapshot entity index number too large!! Corrupted/malformatted packet!\n");
			break;
		}

		if (msg->readpos > msg->maxsize)
		{
			log("CL_ParsePacketEntities: ERROR: end of message\n");
			break;
		}

		while (oldnum < newnum)
		{
			// one or more entities from the old packet are unchanged
			//if ( cl_shownet->integer == 3 )
			//{
			//Com_Printf ("%3i: unchanged: %i\n", msg->readcount, oldnum);
			//}
			DecodeDeltaEntity(msg, newsnap, oldnum, oldstate, true, newindex);
			//unchanged, copy old value over
			//newsnap->ents[newindex] = oldsnap->ents[oldindex];

			newindex++;
			oldindex++;

			if (oldindex >= oldsnap->entcount)
			{
				oldnum = MAX_PACKET_ENTITIES;
			}
			else
			{
				//only unchanged ents were copied after last decode
				oldstate = &oldsnap->entdata[oldindex];//oldstate = &oldsnap->ents[oldindex];//&oldsnap->e&cl.parseEntities[(oldframe->parseEntitiesNum + oldindex) & (MAX_PARSE_ENTITIES-1)];
				oldnum = oldstate->id;
			}
		}
		if (oldnum == newnum)
		{
			// delta from previous state
			//if ( cl_shownet->integer == 3 )
			//{
			//Com_Printf ("%3i: delta: %i\n", msg->readcount, newnum);
			//}

			//entdata* d = &newsnap->ents[newindex];
			//newsnap->ents[newindex] = oldsnap->ents[oldindex];

			//while(true)
			//{
			byte typeId = msg->ReadByte();
			if (typeId == 0)//it was deleted, do not advance newindex so that the entity is effectively removed/replaced by the next one
			{
				//remove entity from snapshot
				//char p[50];
				//sprintf(p, "got remove entity message id %d\n", newnum);
				//log(p);
			}
			else if (typeId != 254)//do a diff
			{
				msg->readpos -= 1;
				DecodeDeltaEntity(msg, newsnap, newnum, oldstate, false, newindex);
				newindex++;
			}
			else
			{
				newindex++;
			}

			oldindex++;

			if (oldindex >= oldsnap->entcount)
			{
				oldnum = MAX_PACKET_ENTITIES;
			}
			else
			{
				oldstate = &oldsnap->entdata[oldindex];//oldstate = &oldsnap->ents[oldindex];//&cl.parseEntities[(oldframe->parseEntitiesNum + oldindex) & (MAX_PARSE_ENTITIES-1)];
				oldnum = oldstate->id;
			}
			continue;
		}

		if (oldnum > newnum)//new entity
		{
			// delta from baseline
			//if ( cl_shownet->integer == 3 )
			//{
			//Com_Printf ("%3i: baseline: %i\n", msg->readcount, newnum);
			//}
			//index = msg->ReadByte();
			//char p[50];
			//sprintf(p, "got new entity message for id %d/%d\n", newnum, oldnum);
			//OutputDebugString(p);
			DecodeDeltaEntity(msg, newsnap, newnum, 0/*&cl.entityBaselines[newnum]*/, false, newindex);
			newindex++;

			continue;
		}
	}

	/*QueryPerformanceCounter(&stop);
	QueryPerformanceFrequency(&freq);
	float elapsed = (float)(stop.QuadPart - start.QuadPart)/ (float)freq.QuadPart;
	char sz[150];
	sprintf(sz, "it took %0.9f sec to decode first part of ent data\n", elapsed);
	OutputDebugString(sz);
	QueryPerformanceCounter(&start);*/

	// any remaining entities in the old frame are copied over
	while (oldnum != MAX_PACKET_ENTITIES)
	{
		// one or more entities from the old packet are unchanged
		//if ( cl_shownet->integer == 3 )
		//{
		//Com_Printf("%3i: unchanged: %i\n", msg->readcount, oldnum);
		//}
		//char p[51];
		//sprintf(p, "unchanged entity %d\n", oldnum);
		//OutputDebugString(p);
		DecodeDeltaEntity(msg, newsnap, oldnum, oldstate, true, newindex);

		newindex++;
		oldindex++;

		if (oldindex >= oldsnap->entcount)
		{
			oldnum = MAX_PACKET_ENTITIES;
		}
		else
		{
			oldstate = &oldsnap->entdata[oldindex];//oldstate = &oldsnap->ents[oldindex];//&cl.parseEntities[(oldframe->parseEntitiesNum + oldindex) & (MAX_PARSE_ENTITIES-1)];
			oldnum = oldstate->id;
		}
	}
	/*QueryPerformanceCounter(&stop);
	QueryPerformanceFrequency(&freq);
	elapsed = (float)(stop.QuadPart - start.QuadPart)/ (float)freq.QuadPart;
	//char sz[150];
	sprintf(sz, "it took %0.9f sec to decode last part of ent data\n", elapsed);
	OutputDebugString(sz);*/

	//update counts
	if (this->lastSnapshotNum + 1 != newsnap->framenumber)
		this->droppedPackets++;

	if (newsnap->framenumber > this->lastSnapshotNum)
		this->lastSnapshotNum = newsnap->framenumber;
	else
		log("Network: Got old or Duplicate Snapshot!!\n");

	return newsnap;
}

void ServerClientNetworker::BuildSnapshot(ServerSnap* ss, int num_entities, PlayerBase** entities, EntityManagerBase* entityManager)
{
	PROFILE("BuildSnapshot");
	this->frameNumber++;

	Snapshot* snap = &frames[this->frameNumber & PACKET_MASK];//get the frame to edit, so we save old ones
	snap->id = 0xBB;
	snap->tod = ss->tod;
	snap->timef = ss->time;
	snap->framenumber = this->frameNumber;//for now

	//write items
	delete[] snap->player_snapshots;
	entityManager->BuildPlayerSnapshot(snap, num_entities, entities);

	snap->entdata = ss->entdata;
	snap->entcount = ss->num_entities;

	if (snap->snapshotEntities)//delete old one if we have it
		delete[] snap->snapshotEntities;

	//need to guess about size and just use max possible, because slow to figure it out and do visibility test twice
	snap->snapshotEntities = new int[snap->entcount];
	int currentSize = 0;
	for (int i = 0; i < snap->entcount; i++)//loop through all entities in the frame
	{
		ED* e = &snap->entdata[i];
		if (e->id < 0)
		{
			log("got bad entity id in loop\n");
			break;
		}

		CEntity* ent = entityManager->GetEntByID(e->id);
		if (ent)
		{
			//this is the PVS check
			if (entityManager->ShouldTransmit(ent, entities[0]))//->ShouldTransmit(entities[0]))
			{
				//add it to the player's snapshot
				//logf("Ent %d Is In PVS\n", e->id);
				//check if was in last
				snap->snapshotEntities[currentSize] = i;
				currentSize++;
			}
			else
			{
				//check if was in last, if was
				//logf("Ent %d Is Out Of PVS\n", e->id);
			}
		}
	}
	snap->num_snapshotEntities = currentSize;
}

NetMsg* ServerClientNetworker::BuildDeltaSnapshot(EntityManagerBase* manager)
{
	PROFILE("BuildDelta");
	Snapshot* oldframe;
	Snapshot* frame;
	int lastframe;
	//todo danger zone max size here, this can cause problems on first join(how to handle this?)
	char* dat = new char[8048];//todo: what size should I use?
	NetMsg* msg = new NetMsg(8048, dat);

	if (this->lastSnapshotRecieved > this->frameNumber - PACKET_BACKUP && this->lastSnapshotRecieved != 0)//how old is it?
	{
		oldframe = &this->frames[this->lastSnapshotRecieved & PACKET_MASK];
		lastframe = this->lastSnapshotRecieved;
	}
	else
	{
		oldframe = NULL;//do a full snapshot resend, this means bad connection, or first packet
		lastframe = 0;
	}

	frame = &this->frames[this->frameNumber & PACKET_MASK];

	//ok, lets build the snapshot :D

	//first write important data
	msg->WriteByte(73);//id
	msg->WriteDouble(frame->timef);
	msg->WriteFloat(frame->tod);
	msg->WriteInt(this->acknowledgedMove[0]);

	msg->WriteInt(frame->framenumber);
	//write frame we are delta'ing from
	msg->WriteInt(lastframe);

	msg->WriteShort(this->ping);

	/*             write player state           */
	//write number of local players
	msg->WriteByte(frame->num_player_snapshots);
	for (int i = 0; i < frame->num_player_snapshots; i++)
		manager->BuildDeltaPlayerSnapshot(oldframe ? &oldframe->player_snapshots[i]/*GetPlayerSnapshot<PlayerSnapshot>(i)*/ : 0, &frame->player_snapshots[i]/*GetPlayerSnapshot<PlayerSnapshot>(i)*/, msg);


	//then write entities
	msg->WriteShort(frame->num_snapshotEntities);//write count

	int newnum = 0;
	int oldnum = 0;

	int from_num_entities = oldframe ? oldframe->num_snapshotEntities : 0;
	ED* newent = NULL;
	ED* oldent = NULL;
	int newindex = 0;
	int oldindex = 0;
	while (newindex < frame->num_snapshotEntities || oldindex < from_num_entities)
	{
		if (newindex >= frame->num_snapshotEntities)
		{
			newnum = MAX_PACKET_ENTITIES;
		}
		else
		{
			newent = &frame->entdata[frame->snapshotEntities[newindex]];
			newnum = newent->id;
		}

		if (oldindex >= from_num_entities)
		{
			oldnum = MAX_PACKET_ENTITIES;
		}
		else
		{
			oldent = &oldframe->entdata[oldframe->snapshotEntities[oldindex]];//oldindex];//oldent = &oldframe->ents[oldindex];//&svs.snapshotEntities[(0+oldindex)];// % svs.numSnapshotEntities];
			oldnum = oldent->id;
		}
		//check if type is the same as well
		if (newnum == oldnum)
		{
			if (newent->classid != oldent->classid)
			{
				printf("[Server] Entity at %d was replaced with a different one!\n", newnum);
				//write delete message
				WriteDeltaEntity(msg, oldent, NULL, true);
				//write full update
				WriteDeltaEntity(msg, NULL/*&sv.svEntities[newnum].baseline*/, newent, true);

				oldindex++;
				newindex++;
				continue;
			}
			// delta update from old position
			// because the force parm is false, this will not result
			// in any bytes being emited if the entity has not changed at all
			WriteDeltaEntity(msg, oldent, newent, false);

			oldindex++;
			newindex++;
			continue;
		}

		if (newnum < oldnum)
		{
			WriteDeltaEntity(msg, NULL/*&sv.svEntities[newnum].baseline*/, newent, true);
			newindex++;
			continue;
		}

		if (newnum > oldnum)
		{
			//need to handle entering and leaving pvs
			WriteDeltaEntity(msg, oldent, NULL, true);
			//OutputDebugString("old entity not present anymore in the snapshot, it was replaced\n");

			oldindex++;
			continue;
		}
	}
	msg->WriteShort(MAX_PACKET_ENTITIES);//tell it we are finished with telling it entity data

	//write when we send it
	this->frames[this->frameNumber & PACKET_MASK].messageSent = GetTickCount();
	this->frames[this->frameNumber & PACKET_MASK].messageAcked = 0;

	return msg;
}

ServerSnap* ServerNetworker::BuildEntityData(EntityManagerBase* entityManager, float TOD)
{
	PROFILE("Build Ent Data");
	ServerSnap* snap = &snapshotEntities[this->frameNumber & PACKET_MASK];//get the frame to edit, so we save old ones
	snap->tod = TOD;
	snap->framenumber = this->frameNumber;//for now
	snap->time = GetTickCount();

	if (snap->entdata)
		delete[] snap->entdata;//delete old data

	entityManager->EncodeEntities(snap);

	this->frameNumber++;

	//set this as the latest
	this->currentSnapshot = snap;
	return snap;
}

void ClientNetworker::CorrectPlayerPos(int last, PlayerBase** local_players, int num_local_players)
{
	for (int i = 0; i < num_local_players; i++)
	{
		PlayerBase* player = local_players[i];
		if (player == 0)
			continue;

		if (player->lastsentmove > 0 && last != this->lastSnapshotNum)
		{
			//ok, need to have a list of last moves, and then execute all of the ones up past the last confirmed
			//	execute just by adding a delta from the last
			PlayerUpdatePacket* latest = &player->moves[(player->lastsentmove - 1) % 100];
			PlayerUpdatePacket* acked = &player->moves[this->lastAckedMove % 100];

			PlayerUpdatePacket sstate;//state we got from the server
			Vec3 ppos = player->GetPosition();
			sstate.x = ppos.x;
			sstate.y = ppos.y;
			sstate.z = ppos.z;
			sstate.flags = 0;

			//this should be abstracted out, but i can probably implement this in a generic way if I take into account parents

			PlayerUpdatePacket* last = &sstate;
			int p = max(this->lastAckedMove, 0);
			for (int i = p; i < player->lastsentmove; i++)
			{
				//apply the moves
				PlayerUpdatePacket* move = &player->moves[i % 100];
				//move the movement here from the player->Update()
				//calculate delta P
				Vec3 dp = move->velocity*move->delta_time;//todo: limit the velocity on the server

				Vec3 np = player->server_position + dp;
				if (np.dist(Vec3(latest->x, latest->y, latest->z)) < 15.0f)//limit max player velocity
				{
					//Vec3 np = player->server_position + dp.getnormal()*15.0f;
					//if (this->player->movetype == 1)
					//this->player->TryToMoveTo(move->velocity, 0/*this->player->planet->world*/, move->delta_time);
				}
				else
				{
					//correct based on server position if its more than 15m from us
					//todo make this do something more useful
					player->SetPosition(np);
					//player->position = np;
					//if (player->vehicle)
					//	player->vehicle->position = np;
				}

				last = move;
			}
		}
	}
}

void ClientNetworker::CreateMove(float dT, PlayerBase* player, int player_id, CInput* input, bool in_menu)
{
	typedef struct usercmd_s {
		int serverTime;
		int angles[3];
		int buttons;
		byte weapon; // weapon
		signed char forwardmove, rightmove, upmove;
	} usercmd_t;

	//todo, compact this a bit

	// Send the player position
	PlayerUpdatePacket n;
	n.id = 1;//todo remove packet id from message structs and move it to send
	n.playerID = player_id;
	n.flags = player->GetFlags();
	//if (player->planet)
	//n.parent = player->planetid;
	//else
	n.parent = -1;

	n.weapon = player->selected_weapon;

	//lets pass deltas
	Vec3 pos = player->GetPosition();
	n.x = pos.x;// - moves[this->lastsentmove-1%100].x;
	n.y = pos.y;// - moves[this->lastsentmove-1%100].y;
	n.z = pos.z;// - moves[this->lastsentmove-1%100].z;

	n.velocity = player->GetVelocity();// velocity;
	n.view = player->view;

	/*if (player->vehicle)
		n.rotation = player->vehicle->rotationq;
	else
		n.rotation = Quaternion::IDENTITY;*/
	n.rotation = player->GetRotation();

	n.binds = 0;

	//process all the binds
	for (auto bind : input->GetBindings())
		n.binds |= input->GetBindBool(player_id, bind.first) ? bind.first : 0;

	//build the axes
	for (int i = 0; i < 4; i++)
		n.axes[i] = (signed char)(input->GetAxis(player_id, i)*127.0f);

	n.delta_time = dT;//use me to perform move on the server

	//networking info
	n.commandid = player->lastsentmove;
	n.lastsnapshot = lastSnapshotNum;

	//if (in_menu)//this->showchat || this->freeze)
	//	n.binds = 0;

	if (n.lastsnapshot)
	{
		player->moves[player->lastsentmove % 100] = n;
		this->connection.Send((char*)(&n), sizeof(PlayerUpdatePacket));
		player->lastsentmove++;
	}
}

void ClientNetworker::ProcessPacket(char packetID, int size, char* buffer)
{
	if (packetID == 5)//add new player entity
	{
		PlayerInitialUpdate *p = (PlayerInitialUpdate*)buffer;

		PlayerBase* player = 0;
		bool islocal = false;
		for (int i = 0; i < MaxLocalPlayers; i++)
		{
			PlayerBase* ii = this->local_players[i];
			if (ii && p->playerid == ii->GetID())//EntID)
			{
				islocal = true;
				player = ii;
			}
		}
		if (islocal == false)
		{
			//need to give this the right classes to use
			//need to instantiate it using something else that doesnt require knowing the class
			PlayerBase* ent = EntityManager->CreatePlayer(p->playerid);// new CPlayerEntity;//THIS IS HOW TO CREATE NEW ENTS <-----------------------------------------------
			ent->SetPosition(Vec3(p->x, p->y, p->z));
			//ent->SetModel("erebus.iqm");
			ent->islocalplayer = false;
			//this->EntityManager->AddEntity((CEntity*)ent, p->playerid);

			memcpy(ent->name, p->name, 25);//copy the name over
		}
		else
		{
			PlayerBase* ent = player;

			memcpy(ent->name, p->name, 25);//copy the name over
		}
	}
	else if (packetID == 73)//important entity data packet
	{
		//this is the snapshot...
		NetMsg msg(size, buffer);

		int oldn = this->lastSnapshotNum;
		Snapshot* snap = this->DecodeDeltaSnapshot(&msg);
		snap->time = GetTickCount();

		if (oldn >= snap->framenumber)
		{
			log("got old or duplicate snapshot, removing\n");
			return;
		}

		//ok, need to figure out how to network other non entity things better like game state vars
		//	maybe have the gamestate be a special type that acts like entities and lets you register variables to transport
		renderer->AddPoint(size);

		//get previous snapshot to calculate velocity
		Snapshot* old = this->GetPreviousSnapshot(snap->framenumber);

		this->EntityManager->ApplySnapshot(snap, old);
		this->EntityManager->ApplyPlayerSnapshots(snap, old, this->local_players);
	}
}

void EntityManagerBase::EncodeEntities(ServerSnap* snap)
{
	snap->num_entities = this->Count();
	snap->entdata = new ED[snap->num_entities];
	//should be able to pull this out, just pass in an array of pointers to entities with class id and id at front
	//encode teh ents
	unsigned short entcount = 0;
	//need a way to iterate over all of em
	for (int ei = 0; ei < this->MaxID(); ei++)
	{
		NetworkedEntity* nent = this->GetByID(ei);
		CEntity* ent = this->GetEntByID(ei);//lets condense this 
		if (ent)
		{
			int offset = (char*)ent - (char*)nent;
			snap->entdata[entcount].id = nent->EntID;
			snap->entdata[entcount].classid = nent->typeID;//changeme

			char* data = snap->entdata[entcount].data;
			netField_t* fields = (*GetDataTable())[snap->entdata[entcount].classid];
			if (fields == 0)
			{
				//logf("Got Entity with no DataTable: %s\n", "");// ent->GetClass());
				continue;
			}

			int o = fields[0].offset;
			int i = 0;
			int curpos = 0;
			while (o != FIELD_END)
			{
				netField_t* cf = &fields[i];

				if (cf->flags & FIELD_ENTITY)
				{
					//void* d = (byte*)ent + cf->offset;
					//NetworkedEntity* e = *(NetworkedEntity**)d;
					//byte* epd = (byte*)ent + cf->offset;
					//CEntity* ne = *(CEntity**)(epd);
					byte* ep = (byte*)ent + cf->offset;
					char* epd = *(char**)(ep);//this is a pointer to a CEntity, need to add an offset
					CEntity* ne = (CEntity*)(epd);
					NetworkedEntity* e = (NetworkedEntity*)(epd - offset);
					if (ne != 0)
					{
						//entity exists
						short id = e->EntID;
						memcpy(data + curpos, &id, cf->bits);
					}
					else
					{
						short temp = -1;
						memcpy(data + curpos, &temp, cf->bits);
					}
					curpos += cf->bits;//field size
				}
				else if (cf->flags & FIELD_NORMAL)
					//really code for networking normal vectors efficiently
				{//saves 1/2 the bandwidth, with minimal precision loss, this has precision to ~0.000030518509
					byte* d = (byte*)ent + cf->offset;
					float* v = (float*)d;//vector is array of 3 floats
					//normalize to short
					for (int i = 0; i < 3; i++)
					{
						short val = (short)(v[i] * 32766.0f);
						memcpy(data + curpos + i * 2, &val, 2);
					}

					curpos += cf->bits;//6 bytes total
				}
				else if (cf->flags & FIELD_QUATERNION)//really code for networking normal vectors efficiently
				{//saves 1/2 the bandwidth
					void* d = (byte*)ent + cf->offset;
					Quaternion* q = *(Quaternion**)d;
					//normalize to short

					curpos += cf->bits;
				}
				else if (cf->flags & FIELD_STRING)
				{
					char* d = (char*)ent + cf->offset;
					//read each character until null
					for (int i = 0; i < 50; i++)
					{
						*(data + curpos) = d[i];
						curpos++;

						if (d[i] == 0)
							break;
					}
				}
				else
				{
					void* d = (byte*)ent + cf->offset;
					memcpy(data + curpos, d, cf->bits);//replace 4 with field size
					curpos += cf->bits;//field size
				}

				i++;
				o = fields[i].offset;
			}
			entcount++;
		}
	}
}

void EntityManagerBase::ApplySnapshot(Snapshot* snap, Snapshot* oldsnap)
{
	//used for velocity calculation
	//the 2x is to correct for a wierd error where velocity is 2x more than it should be
	double fdelta = (snap->timef - oldsnap->timef)*2.0 / 1000.0;//(float)(delta)/1000.0f;

	//double real = snap->time - oldsnap->time;
	//real /= 1000;
	//printf("Calculated: %f Sent: %f Real: %f\n", fdelta, (snap->timef - oldsnap->timef), real);
	//last = GetTickCount();

	//ok, lets ignore predicted stuff, but only if ent has prediction enabled, so maybe lets set a flag on it
	unsigned short ents = snap->entcount;
	for (int i = 0; i < ents; i++)
	{
		ED* e = &snap->entdata[i];
		unsigned short entid = e->id;

		char* data = snap->entdata[i].data;
		netField_t* fields = (*GetDataTable())[snap->entdata[i].classid];
		if (fields == 0)
		{
			printf("Got Entity with no DataTable: %d\n", snap->entdata[i].classid);
		}

		//does the ent already exist?
		CEntity* curEnt = this->GetEntByID(entid);
		NetworkedEntity* netEnt = this->GetByID(entid);
		int offset = -4;
		bool isnewent = false;
		if (curEnt && netEnt->typeID == snap->entdata[i].classid)
		{

		}
		else
		{
			isnewent = true;
			curEnt = this->CreateEntity(e->classid);// CreateEntity<CEntity>(e->classid);
			netEnt = (NetworkedEntity*)((char*)curEnt - offset);
			this->AddEntity(curEnt, entid);

			//printf("Added Ent From Snapshot: %s %d\n", curEnt->GetClass(), e->id);
		}
		//int oldm = curEnt->_model_s;
		Vec3 oldpos = netEnt->position;
		//ok issue is the snapshot is being applied to the rotation of the mech
		//if (oldsnap && curEnt->IsPredicted() && snap->time%1000 > 10)//dont interpolate for predicted entities
		//	continue;

		//todo fix possible problem when entity is created and referred to in the same frame
		// may need to do two passes over this data to find that out

		int o = fields[0].offset;
		int i2 = 0;
		int curpos = 0;
		while (o != FIELD_END)
		{
			netField_t* cf = &fields[i2];
			//could have specific type for position and velocity?
			if (cf->flags & FIELD_ENTITY)
			{
				void* d = (byte*)curEnt + cf->offset;
				short id = *(short*)(data + curpos);
				if (id == -1)//null entity
				{
					CEntity* t = 0;
					memcpy(d, &t, 4);
				}
				else
				{
					CEntity* t = this->GetEntByID(id);
					memcpy(d, &t, 4);//replace 4 with field size
				}
				curpos += cf->bits;//field size
			}
			else if (cf->flags & FIELD_NORMAL)
			{
				byte* d = (byte*)curEnt + cf->offset;
				short* val = (short*)(data + curpos);
				for (int i = 0; i < 3; i++)
				{
					float f = ((float)(val[i])) / 32766.0f;
					memcpy(d + i * 4, &f, 4);
				}
				curpos += cf->bits;
			}
			else if (cf->flags & FIELD_STRING)
			{
				char* d = (char*)curEnt + cf->offset;
				//read each character until null
				for (int i = 0; i < 50; i++)
				{
					char val = *(data + curpos);
					d[i] = val;

					curpos++;

					if (val == 0)
						break;
				}
			}
			else if (cf->flags & FIELD_PREDICTED && netEnt->predicted)
			{
				//do nothing!
				if (cf->data)
				{
					//store it in the alternative location if we have one
					void* d = (byte*)curEnt + (int)cf->data;//offset;
					memcpy(d, data + curpos, cf->bits);//replace 4 with field size
				}

				curpos += cf->bits;
			}
			else if (cf->flags & FIELD_ONCHANGE)
			{
				//todo warn or error if the size is not 4
				void* d = (byte*)curEnt + cf->offset;
				int ov = *(int*)(d);
				int nv = *(int*)(data + curpos);

				memcpy(d, data + curpos, cf->bits);//replace 4 with field size
				curpos += cf->bits;//field size

				if ((ov != nv) || isnewent)
				{
					void(*cb)(void*) = (callback_t*)cf->data;
					cb(curEnt);
				}
			}
			else
			{
				void* d = (byte*)curEnt + cf->offset;
				memcpy(d, data + curpos, cf->bits);//replace 4 with field size
				curpos += cf->bits;//field size
			}

			i2++;
			o = fields[i2].offset;
		}

		//ok, need to fix quaternion networking so it actually SLERPs between them,
		//lerp is derping over long distance since isnt orthogonal

		//calculate velocity
		if (netEnt->predicted == false && isnewent == false && oldsnap && fdelta > 0)
		{
			//help, this is off by a factor of two, and idk why
			netEnt->velocity = (netEnt->position - oldpos) / fdelta;
		}
	}

	//now delete entities not in the snapshot
	//first build list of ents in snapshot
	//todo document this random entity limit
	unsigned short ids[512];
	memset(ids, 0, sizeof(unsigned short) * 512);

	for (int i = 0; i < snap->entcount; i++)
	{
		ED* e = &snap->entdata[i];
		unsigned short entid = e->id;
		ids[entid] = 1;
	}

	//delete old entities
	for (int i = 0; i < 512; i++)
	{
		if (ids[i] == 0)
		{
			CEntity* ent = this->GetEntByID(i);
			if (ent != 0)
			{
				//delete it
				//logf("removed entity id: %d from snapshot\n", ent->EntID);
				this->RemoveEntity(this->GetByID(i)->EntID);
			}
		}
	}
}