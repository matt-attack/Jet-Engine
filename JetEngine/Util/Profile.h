#ifndef PROFILE_HEADER
#define PROFILE_HEADER

#ifdef _WIN32
#include <Windows.h>
#endif
#include <map>
#include <vector>
#include <string>

struct Prof
{
	unsigned int count;
	float max;
	float average;

	float frametotal;
	float averageframetotal;

	int callLevel;

	Prof* parent;
	std::string parentname;
	std::vector<Prof*> children;

	std::string name;
};

extern std::map<std::string,Prof*> Profiles;
extern std::vector<Prof*> ProfileList;

#ifndef _WIN32
typedef long long INT64;//#define INT64 long long
#endif
class StackProfile
{
	StackProfile* parent;
	int level;
public:
	INT64 start;
	char* name;
	
	StackProfile() {};
	StackProfile(char* name);

	~StackProfile();
};

class StackTime
{
public:
	INT64 start;
	char* name;
	
	StackTime() {};
	StackTime(char* name);

	~StackTime();
};

#include <D3D11.h>

struct GPUProf
{
	float max;
	float average;
	float last;

	int callLevel;

	GPUProf* parent;
	std::string parentname;
	std::vector<GPUProf*> children;

	std::string name;


	bool started;
	int frame_started;

	ID3D11Query* disjoint_query, *start_query, *end_query;
};

extern std::map<std::string, GPUProf*> GPUProfiles;
extern std::vector<GPUProf*> GPUProfileList;
class StackProfileGPU
{
	GPUProf* prof;
public:
	StackProfileGPU() {};

	StackProfileGPU(const char* name, bool dummy = false);

	~StackProfileGPU();
};

void ProfileStartFrame();//recalculates per frame averages, need to mess with this to handle server stuff
void ProfilesDraw();
void ProfileExit();//saves profile information to file
#define PROFILE(name) StackProfile stackPrOf(name)

#define GPUPROFILE(name) StackProfileGPU stackPrOf2(name)
#define GPUPROFILE2(name) StackProfileGPU stackPrOf22(name)

#define GPUPROFILEGROUP(name) StackProfileGPU stackPrOfe2(name, true)
#endif