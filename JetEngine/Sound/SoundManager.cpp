//This sound library is a modified version of that by Martin Prantl released under this license
//http://www.gamedev.net/page/resources/_/gdnethelp/gamedevnet-open-license-r2956
//http://www.gamedev.net/page/resources/_/technical/game-programming/basic-openal-sound-manager-for-your-project-r3791

#include "SoundManager.h"
#include "SoundObject.h"

#include "AL\al.h"
#include "AL\alc.h"

#include <Windows.h>

#include "OpenAL.h"
#include "../Math/Vector.h"


SoundManager* SoundManager::instance = NULL;

SoundManager::SoundManager(const std::string & deviceName, bool useThreadUpdate)
{
	this->deviceAL = NULL;
	this->contextAL = NULL;
	this->deviceName = deviceName;
	this->ended = false;
	this->useThreadUpdate = useThreadUpdate;

	this->masterVolume = 1.0f;
	this->lastVolume = 1.0f;

	this->enabled = true;

	this->Init();
}

SoundManager::~SoundManager()
{
	this->ended = true;

	if (this->useThreadUpdate)
		this->updateThread.join();

	for (auto it = this->sounds.begin(); it != this->sounds.end(); it++)
	{		
		it->second->Stop();
		if (it->second)
			delete it->second;
	}

	this->sounds.clear();

	for (unsigned int i = 0; i < this->buffers.size(); i++)
		AL_CHECK( alDeleteBuffers(1, &this->buffers[i].refID) );

	for (unsigned int i = 0; i < this->sources.size(); i++)
		AL_CHECK( alDeleteSources(1, &this->sources[i].refID) );

	alcDestroyContext(this->contextAL);
	alcCloseDevice(this->deviceAL);
}


void SoundManager::Initialize(const std::string & deviceName, bool useThreadUpdate)
{
	if (instance == NULL)
		instance = new SoundManager(deviceName, useThreadUpdate);
}

void SoundManager::Destroy()
{
	if (instance)
		delete instance;
}

SoundManager * SoundManager::GetInstance()
{
	return instance;
}

std::vector<std::string> SoundManager::GetAllDevices()
{
	const char * devices = alcGetString(NULL, ALC_DEVICE_SPECIFIER);	
	const char * next = devices + 1;

	size_t len = 0;
	std::vector<std::string> devicesNames; 

	while (devices && *devices != '\0' && next && *next != '\0') 
	{
		std::string d = devices;
		devicesNames.push_back(d);                
		len = strlen(devices);
		devices += (len + 1);
		next += (len + 2);
	}
	return devicesNames;
}

bool SoundManager::IsEnabled()
{
	return this->enabled;
}

void SoundManager::Disable()
{
	if (this->enabled == false)
		return;

	this->enabled = false;
	this->lastVolume = this->masterVolume; //store last volume

	this->SetMasterVolume(0.0f);
}

void SoundManager::Enable()
{
	if (this->enabled)
		return;
	
	this->enabled = true;

	this->SetMasterVolume(this->lastVolume); //restore volume
}

void SoundManager::Init()
{
	//reset error stack
	alGetError();

	this->deviceAL = alcOpenDevice(NULL);

	if (this->deviceAL == NULL)
	{
		printf("Failed to init OpenAL device.");
		return;
	}

	this->contextAL = alcCreateContext(this->deviceAL, NULL);
	AL_CHECK( alcMakeContextCurrent(this->contextAL) );


	for (int i = 0; i < 512; i++)
	{
		SoundBuffer buffer;
		AL_CHECK( alGenBuffers((ALuint)1, &buffer.refID) );
		this->buffers.push_back(buffer);
	}

	for (int i = 0; i < 16; i++)
	{
		SoundSource source;
		AL_CHECK( alGenSources((ALuint)1, &source.refID)) ;
		this->sources.push_back(source);
	}


	for (unsigned int i = 0; i < this->buffers.size(); i++)
		this->freeBuffers.push_back(&this->buffers[i]);

	for (unsigned int i = 0; i < this->sources.size(); i++)
		this->freeSources.push_back(&this->sources[i]);

	if (this->useThreadUpdate)
	{
		//this->fakeMutex = PTHREAD_MUTEX_INITIALIZER;
		//this->fakeCond = PTHREAD_COND_INITIALIZER;

		//if(pthread_create(&this->updateThread, NULL, &SoundManager::UpdateThread, this)) 
		{	
			printf("Error creating thread");
			return;
		}
	}
}


void* SoundManager::UpdateThread(void* c)
{
	SoundManager * context = ((SoundManager *)c);
	while (true)
	{
		context->ThreadUpdate();
		context->Wait(400);

		if (context->ended) 
			break;
	}

	return NULL;
}

void SoundManager::Wait(int timeInMS)
{
	Sleep(timeInMS);
}

float SoundManager::GetMasterVolume()
{
	return this->masterVolume;
}

void SoundManager::SetMasterVolume(float volume)
{
	if (volume < 0)
		volume = 0;
	else if (volume > 1)
		volume = 1;
	this->masterVolume = volume;
	AL_CHECK( alListenerf(AL_GAIN, this->masterVolume) );
}

SoundObject* SoundManager::GetSound(const std::string & name)
{
	if (!this->ExistSound(name))
		return NULL;

	return this->sounds[name];
}

bool SoundManager::ExistSound(const std::string & name) const
{
	if (this->sounds.find(name) == this->sounds.end())
		return false;

	return true;
}

void SoundManager::ReleaseSound(const std::string & name)
{
	if (!this->ExistSound(name))
		return;

	this->sounds[name]->Stop();
	if (this->sounds[name])
		delete this->sounds[name];

	this->sounds.erase(this->sounds.find(name));
}

void SoundManager::AddSound(const std::string & fileName, const std::string & name)
{
	if (this->ExistSound(name))
	{
		printf("Sound with name %s already exist.", name.c_str());
		return;
	}

	SoundObject * sound = new SoundObject(fileName, name);
	this->sounds.insert(std::make_pair(name, sound));
}

void SoundManager::AddSound(SoundObject * sound)
{
	std::string name = sound->GetSettings().name;
	if (this->ExistSound(name))
	{
		printf("Sound with name %s already exist.", name.c_str());
		return;
	}

	this->sounds.insert(std::make_pair(name, sound));
}


SoundSource * SoundManager::GetFreeSource()
{
	if (this->freeSources.size() == 0)
		return NULL;

	SoundSource * source = this->freeSources.front();
	this->freeSources.pop_front();
	source->free = false;

	return source;
}


SoundBuffer * SoundManager::GetFreeBuffer()
{
	if (this->freeBuffers.size() == 0)
		return NULL;

	SoundBuffer * buf = this->freeBuffers.front();
	this->freeBuffers.pop_front();
	buf->free = false;

	return buf;
}


void SoundManager::FreeSource(SoundSource * source)
{
	if (source == NULL)
		return;
	
	source->free = true;
	this->freeSources.push_back(source);
}

void SoundManager::FreeBuffer(SoundBuffer * buffer)
{
	if (buffer == NULL)
		return;

	AL_CHECK( alDeleteBuffers(1, &buffer->refID) );
	AL_CHECK( alGenBuffers(1, &buffer->refID) );

	buffer->free = true;
	this->freeBuffers.push_back(buffer);	
}

#include "../Util/Profile.h"
void SoundManager::Update()
{
	PROFILE("SoundUpdate");

	if (this->useThreadUpdate == false)
		this->ThreadUpdate();
}

void SoundManager::ThreadUpdate()
{
	if (this->enabled == false)
		return;

	for (auto it = this->sounds.begin(); it != this->sounds.end(); it++)
	{
		if (it->second->IsPlaying())
			it->second->Update();
	}

	//ok, add another type of sound object that just stores the data, and doesnt actually take a sound source reference
	std::list<PlayingSound*> toremove;
	for (auto ii: this->playingSources)
	{
		int buffersProcessed = 0;
		AL_CHECK( alGetSourcei(ii->source->refID, AL_BUFFERS_PROCESSED, &buffersProcessed) );

		auto c = this->GetSound(ii->name);
		//fixme I only work with 1 buffer objects
		if (buffersProcessed >= 1)
		{
			AL_CHECK( alSourceStop(ii->source->refID) );

			for (int i = 0; i < PRELOAD_BUFFERS_COUNT; i++)
			{		
				if (c->buffers[i] == NULL)
					continue;

				AL_CHECK( alSourceUnqueueBuffers(ii->source->refID, 1, &c->buffers[i]->refID) );
			}
			toremove.push_back(ii);
			this->FreeSource(ii->source);
		}
	}

	for (auto ii: toremove)
	{
		delete ii;
		this->playingSources.remove(ii);
	}
}

void SoundManager::SetPosition(const Vec3& pos)
{
	AL_CHECK( alListener3f(AL_POSITION, pos.x, pos.y, pos.z) );	
}

void SoundManager::SetVelocity(const Vec3& vel)
{
	AL_CHECK( alListener3f(AL_VELOCITY, vel.x, vel.y, vel.z) );	
}

void SoundManager::SetOrientation(const Vec3& up, const Vec3& forward)
{
	float vecs[6];
	vecs[0] = up.x;
	vecs[1] = up.y;
	vecs[2] = up.z;
	vecs[3] = forward.x;
	vecs[4] = forward.y;
	vecs[5] = forward.z;
	AL_CHECK( alListenerfv(AL_ORIENTATION, vecs));
}
#undef PlaySound
void SoundManager::PlaySound(const std::string& name, Vec3 pos, bool relative)
{
	SoundSource* source = this->GetFreeSource();
	if (source)
	{
		SoundObject* snd = this->GetSound(name);
		if (snd == 0)
		{
			printf("[SoundManager] No sound: %s found!\n", name.c_str());
			this->FreeSource(source);

			return;
		}
		//add a way to randomize pitch on specific sounds
		//AL_CHECK( alSourcef(source->refID, AL_PITCH, this->settings.pitch)) ;	
		//AL_CHECK( alSourcef(source->refID, AL_GAIN, this->settings.gain) );	
		AL_CHECK( alSource3f(source->refID, AL_POSITION, pos.x, pos.y, pos.z) );	
		//AL_CHECK( alSource3f(source->refID, AL_VELOCITY, this->settings.velocity.x, this->settings.velocity.y, this->settings.velocity.z) );	
		AL_CHECK( alSourcei(source->refID, AL_LOOPING, AL_FALSE) );
		AL_CHECK( alSourcef(source->refID, AL_MAX_DISTANCE, 50.0f));
		AL_CHECK( alSourcei(source->refID, AL_REFERENCE_DISTANCE, 10.0));
		//retrieve buffers for the sound from the sound files
		int usedBuffersCount = 0;
		for (int i = 0; i < PRELOAD_BUFFERS_COUNT; i++)
		{		
			if (snd->buffers[i] == NULL)
				continue;
			
			AL_CHECK( alSourceQueueBuffers(source->refID, 1, &snd->buffers[i]->refID) );
			usedBuffersCount++;
		}

		if (relative)
			AL_CHECK( alSourcef(source->refID, AL_MIN_GAIN, 1.0f));

		AL_CHECK( alSourcePlay(source->refID) );

		PlayingSound* src = new PlayingSound;
		src->source = source;
		src->name = name;
		//then push the source to a playing list that every update is checked for if finished, then released back to the pool
		this->playingSources.push_back(src);
	}
	else
	{
		printf("[SoundManager] No Sources Availible: Could not play sound '%s'!\n", name.c_str());
	}

	//ok, have sound object just retrieve buffers from a soundfilewrapper object
	//which will be stored in the manager in a separate file
	//that way buffers can be shared by sounds and you can play same sound several times at once
	//SoundObject* snd = this->GetSound(name);
	//snd->Play();
	//snd->SetPosition(position);
	//if (relative)
	//snd->SetRelative(true);
}