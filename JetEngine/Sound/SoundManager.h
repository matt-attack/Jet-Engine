#ifndef SOUND_MANAGER_H
#define SOUND_MANAGER_H

class SoundObject;

typedef struct ALCdevice_struct ALCdevice;
typedef struct ALCcontext_struct ALCcontext;

#include <thread>
#include <mutex>

#include <map>
#include <vector>
#include <list>

#include "../Math/Vector.h"

#undef PlaySound


struct SoundBuffer
{
	bool free;
	unsigned int refID;
};

struct SoundSource
{
	bool free;
	unsigned int refID;
};

class SoundManager
{
public:

	static std::vector<std::string> GetAllDevices();
	static void Initialize(const std::string& deviceName = "", bool useThreadUpdate = true);
	static void Destroy();
	static SoundManager* GetInstance();

	bool ExistSound(const std::string& name) const;
	void ReleaseSound(const std::string& name);
	void AddSound(const std::string& fileName, const std::string& name);
	void AddSound(SoundObject* sound);

	SoundObject* GetSound(const std::string & name);

	void Update();

	float GetMasterVolume();
	void SetMasterVolume(float volume);

	//listener position settings
	void SetPosition(const Vec3& pos);
	void SetVelocity(const Vec3& vel);

	void SetOrientation(const Vec3& up, const Vec3& forward);

	void PlaySound(const std::string& name, Vec3 position, bool relative = false);//plays the sound at the position, for short sounds only


	bool IsEnabled();
	void Disable();
	void Enable();

	friend class SoundObject;

protected:
	SoundManager(const std::string& deviceName, bool useThreadUpdate);
	~SoundManager();

	static SoundManager* instance;

	ALCdevice* deviceAL;
	ALCcontext* contextAL;
	std::string deviceName;

	std::thread updateThread;
	std::mutex fakeMutex;

	bool useThreadUpdate;
	bool ended;

	bool enabled;
	float lastVolume;

	float masterVolume;

	std::map<std::string, SoundObject*> sounds;

	std::vector<SoundSource> sources;
	std::vector<SoundBuffer> buffers;

	std::list<SoundSource*> freeSources;
	std::list<SoundBuffer*> freeBuffers;

	struct PlayingSound
	{
		std::string name;
		SoundSource* source;
	};
	std::list<PlayingSound*> playingSources;

	void Init();

	SoundSource* GetFreeSource();
	SoundBuffer* GetFreeBuffer();

	void FreeSource(SoundSource* source);
	void FreeBuffer(SoundBuffer* buffer);

	static void* UpdateThread(void* c);
	void Wait(int timeInMS);
	void ThreadUpdate();
};

#endif