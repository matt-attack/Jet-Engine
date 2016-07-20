#include "Profile.h"
#include "../Graphics/CRenderer.h"
#include <map>
#include <vector>

std::map<std::string, Prof*> Profiles;
std::vector<Prof*> ProfileList;

#include "../Util/CriticalSection.h"

CriticalSection profile_lock;

#ifndef _WIN32
INT64 gettime2()//returns time in microseconds
{
	static __time_t start;
	timespec time;
	//gettimeofday(&time, 0);
	clock_gettime(CLOCK_MONOTONIC, &time);
	return (INT64)time.tv_sec*1000000 + time.tv_nsec/1000;
}
#endif
//improve profile stuff
//__declspec(thread) int currentLevel = -1;
__declspec(thread) StackProfile* current = 0;

int current_frame = 0;

INT64 rate;
void ProfileStartFrame()
{
	current_frame++;

#ifdef _WIN32
	QueryPerformanceFrequency((LARGE_INTEGER *)&rate);
#endif
	profile_lock.Enter();
	for (auto ii = Profiles.begin(); ii != Profiles.end(); ii++)
	{
		ii->second->averageframetotal = ii->second->frametotal;//ii->second.frametotal*0.15f + ii->second.averageframetotal*0.85f;
		ii->second->frametotal = 0.0f;
	}
	profile_lock.Leave();
}

char po[500];
//int yp = 180;
int RecursePrint(Prof* profile, int yp)
{
	int off = profile->callLevel * 40;
	sprintf(po, "%s Max: %f Avg: %f Frame: %f", profile->name.c_str(), profile->max, profile->average, profile->averageframetotal);
	renderer->DrawText(2 + off, yp, po, COLOR_ARGB(255, 255, 0, 0));
	yp += 12;

	for (auto ii : profile->children)
		yp = RecursePrint(ii, yp);
	return yp;
}

void ProfilesDraw()
{
	renderer->SetFont("arial", 12);
	char po[500];
	int yp = 180;
	if (showdebug > 2)
	for (int i = 0; i < ProfileList.size(); i++)
	{
		if (ProfileList[i]->parent == 0)
		{
			yp = RecursePrint(ProfileList[i], yp);
		}
	}

	//print gpu profiles
	for (int i = 0; i < GPUProfileList.size(); i++)
	{
		auto profile = GPUProfileList[i];
		int off = profile->callLevel * 40;
		if (profile->children.size() > 0 && profile->max == 0)
		{
			profile->average = 0;
			profile->last = 0;
			for (auto ii : profile->children)
			{
				profile->average += ii->average;
				profile->last += ii->average;
			}
			sprintf(po, "Group %s Avg: %f Current: %f", profile->name.c_str(), profile->average, profile->last);
		}
		else
			sprintf(po, "GPU %s Max: %f Avg: %f Current: %f", profile->name.c_str(), profile->max, profile->average, profile->last);
		renderer->DrawText(2 + off, yp, po, COLOR_ARGB(255, 255, 255, 0));
		yp += 12;
	}

	for (int i = 0; i < GPUProfileList.size(); i++)
		if (GPUProfileList[i]->started && GPUProfileList[i]->frame_started < current_frame - 11)
			GPUProfileList[i]->last = 0;

	renderer->SetFont("arial", 20);
}

#include <fstream>
void ProfileExit()
{
	profile_lock.Enter();
	std::fstream file("profiler.txt", std::ios_base::out);
	file << "Name    Average    Max    AveragePerFrame    Count\n";
	for (auto ii : Profiles)
		file << ii.first << " " << ii.second->average << " " << ii.second->max << " " << ii.second->averageframetotal << " " << ii.second->count << "\n";

	for (auto ii : ProfileList)
	{
		delete ii;
	}

	ProfileList.clear();
	Profiles.clear();
	profile_lock.Leave();
}

StackProfile::StackProfile(char* name)
{
	return;
	this->name = name;

#ifndef _WIN32
	start = gettime2();
	rate = 1000000;
#else
	QueryPerformanceCounter((LARGE_INTEGER *)&start);
#endif
	//this has threading issues
	//this->level = ++currentLevel;// + 1;
	//TlsFree();
	//use thread local storage to keep track here
	//GetThreadId();
	if (current)
		this->level = current->level + 1;
	else
		this->level = 0;

	this->parent = current;

	current = this;
	//currentLevel++;
}

#include <mutex>

StackProfile::~StackProfile()
{
	return;
	INT64 end;
#ifdef _WIN32
	QueryPerformanceCounter((LARGE_INTEGER *)&end);
#else
	end = gettime2();
#endif
	char o[100];
	INT64 diff = end - start;
	float dt = ((float)diff) / ((float)rate);
	//sprintf(o,"%s Time: %f\n", this->name, dt);
	//log(o);
	profile_lock.Enter();
	auto ii = Profiles.end();
	if ((ii = Profiles.find(name)) != Profiles.end())
	{
		ii->second->average = ii->second->average*0.85f + dt*0.15f;
		ii->second->count++;
		ii->second->frametotal += dt;
		if (dt > ii->second->max)
			ii->second->max = dt;
	}
	else
	{
		Prof* n = new Prof;
		n->count = 1;
		n->average = dt;
		n->max = dt;
		n->frametotal = dt;
		n->averageframetotal = dt;
		n->callLevel = this->level;
		if (n->callLevel > 10 || n->callLevel < 0)
			n->callLevel = 0;
		n->name = name;
		n->parent = 0;
		if (this->parent)
			n->parentname = this->parent->name;
		Profiles[name] = n;
		ProfileList.push_back(n);

		//look for profiles with my name
		for (auto ii : ProfileList)
		{
			if (ii->parentname == n->name)
			{
				ii->parent = n;
				n->children.push_back(ii);
			}
		}
	}
	profile_lock.Leave();

	current = this->parent;
}


StackTime::StackTime(char* name)
{
	this->name = name;

#ifndef _WIN32
	start = gettime2();
	rate = 1000000;
#else
	QueryPerformanceCounter((LARGE_INTEGER *)&start);
#endif
}

StackTime::~StackTime()
{
	INT64 end;
#ifdef _WIN32
	QueryPerformanceCounter((LARGE_INTEGER *)&end);
#else
	end = gettime2();
#endif
	char o[100];
	INT64 diff = end - start;
	float dt = ((double)diff) / ((double)rate);
	sprintf(o, "%s Time: %f seconds\n", this->name, dt);
	printf(o);
	OutputDebugString(o);
}

std::map<std::string, GPUProf*> GPUProfiles;
std::vector<GPUProf*> GPUProfileList;

__declspec(thread) GPUProf* currentGPU = 0;

StackProfileGPU::StackProfileGPU(const char* name, bool dummy)
{
	auto iter = GPUProfiles.find(name);
	if (iter == GPUProfiles.end())
	{
		//make new one
		GPUProf* p = new GPUProf;
		p->started = false;
		p->average = 0;
		p->max = 0;
		p->name = name;
		p->callLevel = 0;
		if (currentGPU)
		{
			p->parent = currentGPU;
			p->callLevel = currentGPU->callLevel + 1;
		}
		else
		{
			p->parent = 0;
			p->callLevel = 0;
		}
		
		if (p->parent)
			p->parent->children.push_back(p);

		GPUProfiles.insert({ name, p });
		GPUProfileList.push_back(p);

		iter = GPUProfiles.find(name);
	}

	if (iter->second->started == false && dummy == false)
	{
		auto p = iter->second;

		//create the queries
		D3D11_QUERY_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
		renderer->device->CreateQuery(&desc, &p->disjoint_query);
		desc.Query = D3D11_QUERY_TIMESTAMP;
		renderer->device->CreateQuery(&desc, &p->start_query);
		renderer->device->CreateQuery(&desc, &p->end_query);

		renderer->context->Begin(p->disjoint_query);

		renderer->context->End(p->start_query);

		p->started = true;
		p->frame_started = current_frame;
	}

	currentGPU = iter->second;
	this->prof = iter->second;
}

StackProfileGPU::~StackProfileGPU()
{
	//kill it
	if (this->prof->started && this->prof->frame_started == current_frame)
	{
		renderer->context->Flush();//wait for finish

		renderer->context->End(this->prof->end_query);
		renderer->context->End(this->prof->disjoint_query);
	}

	currentGPU = this->prof->parent;

	if (this->prof->started && this->prof->frame_started < current_frame - 10)
	{
		//wait for results
		UINT64 start_data = 0;
		auto res = renderer->context->GetData(this->prof->start_query, &start_data, sizeof(start_data), 0);


		UINT64 end_data = 0;
		while (res = renderer->context->GetData(this->prof->end_query, &end_data, sizeof(end_data), 0) == S_FALSE)
		{
			Sleep(0);
		}

		D3D11_QUERY_DATA_TIMESTAMP_DISJOINT dj_data;
		while (res = renderer->context->GetData(this->prof->disjoint_query, &dj_data, sizeof(dj_data), 0) == S_FALSE)
		{
			Sleep(0);
		}

		float dt = ((float)(end_data - start_data)) / (float)dj_data.Frequency * 1000.0f;
		this->prof->end_query->Release();
		this->prof->start_query->Release();
		this->prof->disjoint_query->Release();

		this->prof->started = false;

		if (this->prof->average != 0)
			this->prof->average = this->prof->average*0.85f + dt*0.15f;
		else
			this->prof->average = dt;

		if (dt > this->prof->max)
			this->prof->max = dt;

		this->prof->last = dt;

		//printf("%f ms %f ms avg\n", dt, this->prof->average);
	}
}