#include "SoundObject.h"
#include "ISoundFileWrapper.h"

#include "Wrapper_WAV.h"
//#include "Wrapper_RAW.h"

#include "SoundManager.h"

#include "OpenAL.h"


SoundObject::SoundObject(const std::string & fileName, const std::string & name)
{
	this->SINGLE_BUFFER_SIZE = 64 * 1024;

	this->settings.fileName = fileName;
	this->settings.name = name;

	this->settings.pitch = 1.0f;
	this->settings.gain = 1.0f;
	this->settings.loop = false;
	this->settings.pos = Vec3(0, 0, 0);
	this->settings.velocity = Vec3(0, 0, 0);

	this->vfsFile = NULL;
	this->soundFileWrapper = NULL;
	this->soundData = NULL;
	this->dataSize = 0;

	this->playedCount = 0;

	this->state = STOPPED;

	this->LoadData();
}

SoundObject::SoundObject(const SoundSettings & settings)
{
	this->SINGLE_BUFFER_SIZE = 64 * 1024;

	this->settings = settings;

	this->vfsFile = NULL;
	this->soundFileWrapper = NULL;
	this->soundData = NULL;
	this->dataSize = 0;

	this->playedCount = 0;

	this->state = STOPPED;

	this->LoadData();
}

SoundObject::SoundObject(char * rawData, uint32 dataSize, SoundInfo soundInfo, const std::string & name)
{
	this->SINGLE_BUFFER_SIZE = 64 * 1024;

	this->soundInfo = soundInfo;

	this->settings.fileName = "<memory raw data>";
	this->settings.name = name;

	this->settings.pitch = 1.0f;
	this->settings.gain = 1.0f;
	this->settings.loop = false;
	this->settings.pos = Vec3(0, 0, 0);
	this->settings.velocity = Vec3(0, 0, 0);

	this->vfsFile = NULL;
	this->soundFileWrapper = NULL;
	this->soundData = NULL;
	this->dataSize = 0;

	this->playedCount = 0;

	this->state = STOPPED;

	this->LoadRawData(rawData, dataSize);
}


SoundObject::~SoundObject()
{
	this->Release();
}

void SoundObject::Release()
{
	if (this->state != STOPPED)
	{
		this->Stop();
	}

	if (this->vfsFile)
		fclose(this->vfsFile);//VFS::GetInstance()->CloseFile(this->vfsFile);
	this->vfsFile = NULL;
	if (this->soundData)
		delete[] this->soundData;
	//SAFE_DELETE(this->soundData);
	if (this->soundFileWrapper)
		delete this->soundFileWrapper;
	//SAFE_DELETE(this->soundFileWrapper);

	for (int i = 0; i < PRELOAD_BUFFERS_COUNT; i++)
	{
		SoundManager::GetInstance()->FreeBuffer(this->buffers[i]);
	}
}

const SoundSettings& SoundObject::GetSettings() const
{
	return this->settings;
}

const SoundInfo& SoundObject::GetInfo() const
{
	return this->soundInfo;
}

void SoundObject::PlayInLoop(bool val)
{
	this->settings.loop = val;
}

float SoundObject::GetMaxBufferedTime() const
{
	return this->GetBufferedTime(PRELOAD_BUFFERS_COUNT);
}

float SoundObject::GetBufferedTime(int buffersCount) const
{
	float preBufferTime = this->SINGLE_BUFFER_SIZE / static_cast<float>(this->soundInfo.freqency * this->soundInfo.channels * this->soundInfo.bitsPerChannel / 8);
	preBufferTime *= buffersCount;
	//preBufferTime *= (PRELOAD_BUFFERS_COUNT - 1);

	return preBufferTime;
}

float SoundObject::GetTime() const
{
	//Get duration of remaining buffer
	float preBufferTime = this->GetBufferedTime(this->remainBuffers);

	//get current time of file stream
	//this stream is "in future" because of buffered data
	//duration of buffer MUST be removed from time
	float time = this->soundFileWrapper->GetTime() - preBufferTime;


	if (this->remainBuffers < PRELOAD_BUFFERS_COUNT)
	{
		//file has already been read all
		//we are currently "playing" sound from cache only
		//and there is no loop active
		time = this->soundFileWrapper->GetTotalTime() - preBufferTime;
	}

	if (time < 0)
	{
		//file has already been read all
		//we are currently "playing" sound from last loop cycle
		//but file stream is already in next loop
		//because of the cache delay

		//Signe of "+" => "- abs(time)" rewritten to "+ time"
		time = this->soundFileWrapper->GetTotalTime() + time;
	}


	//add current buffer play time to time from file stream
	float result;
	AL_CHECK(alGetSourcef(this->source->refID, AL_SEC_OFFSET, &result));

	time += result;

	//time in seconds
	return time;
}

int SoundObject::GetPlayedCount() const
{
	return this->playedCount;
}

bool SoundObject::IsPlaying() const
{
	return (this->state == PLAYING);
}

void SoundObject::Rewind()
{
	this->soundFileWrapper->ResetStream();

	AL_CHECK( alSourceRewind(this->source->refID) );
}

void SoundObject::GetRawData(std::vector<char> * rawData)
{
	if (this->state == PLAYING)
	{
		printf("Can not get raw data for sound (%s), that is in PLAY state.", this->settings.name.c_str());
		return;
	}

	this->soundFileWrapper->DecompressAll(*rawData);
}

#include <fstream>
#include <Windows.h>
void SoundObject::LoadData()
{
	for (int i = 0; i < PRELOAD_BUFFERS_COUNT; i++)
		this->buffers[i] = NULL;

	this->spinLock = false;

	FILE * vfsFile = fopen(this->settings.fileName.c_str(), "rb");

	if (vfsFile == NULL)
	{
		MessageBox(0,this->settings.fileName.c_str(), "Missing Sound File!", MB_OK);
		printf("File %s not found\n", this->settings.fileName.c_str());
		return;
	}

	int p = this->settings.fileName.find_first_of('.');
	std::string extension = this->settings.fileName.substr(p+1);
	if (extension == "ogg")
	{
		//this->soundFileWrapper = new WrapperOgg(this->SINGLE_BUFFER_SIZE);
	}
	else if (extension == "wav")
	{
		this->soundFileWrapper = new WrapperWav(this->SINGLE_BUFFER_SIZE);
	}
	else 
	{
		printf("File extension %s not supported.\n", extension.c_str());
	}


	if (false)//true)//vfsFile->archiveInfo == NULL)
	{
		this->soundFileWrapper->LoadFromFile((FILE *)vfsFile, &this->soundInfo);
	}
	else 
	{
		//this->vfsFile;
		fclose(vfsFile);

		std::ifstream t;
		int length;
		t.open(this->settings.fileName.c_str(), std::istream::binary);      // open input file
		t.seekg(0, std::ios::end);    // go to the end
		length = t.tellg();           // report location (this is the length)
		t.seekg(0, std::ios::beg);    // go back to the beginning
		this->soundData = new char[length];    // allocate memory for a buffer of appropriate dimension
		t.read(soundData, length);       // read the whole file into the buffer
		//buffer[length] = 0;
		this->dataSize = length;
		t.close();  

		this->vfsFile = NULL;

		//this->soundData = VFS::GetInstance()->GetFileContent(this->settings.fileName, &this->dataSize);
		this->soundFileWrapper->LoadFromMemory(this->soundData, this->dataSize, &this->soundInfo);
	}

	//AL_CHECK( alGenSources((ALuint)1, &this->source)) ;

	//AL_CHECK( alGenBuffers((ALuint)1, &this->buffer) );

	if (this->soundInfo.bitsPerChannel == 16)
	{
		if (this->soundInfo.channels == 1)
			this->soundInfo.format = AL_FORMAT_MONO16;
		else
			this->soundInfo.format = AL_FORMAT_STEREO16;
	}
	else if (this->soundInfo.bitsPerChannel == 8)
	{
		if (this->soundInfo.channels == 1)
			this->soundInfo.format = AL_FORMAT_MONO8;
		else
			this->soundInfo.format = AL_FORMAT_STEREO8;
	}

	for (uint32 i = 0; i < PRELOAD_BUFFERS_COUNT; i++)
	{
		SoundBuffer * buf = SoundManager::GetInstance()->GetFreeBuffer();
		if (buf == NULL)
		{
			printf("Not enough free sound-buffers");
			continue;
		}
		this->buffers[i] = buf;
	}

	this->Preload();
}

void SoundObject::LoadRawData(char * rawData, uint32 dataSize)
{
	for (int i = 0; i < PRELOAD_BUFFERS_COUNT; i++)
		this->buffers[i] = NULL;

	this->spinLock = false;

	//this->soundFileWrapper = new WrapperRaw(this->soundInfo, this->SINGLE_BUFFER_SIZE);

	this->soundFileWrapper->LoadFromMemory(rawData, dataSize, &this->soundInfo);

	if (this->soundInfo.bitsPerChannel == 16)
	{
		if (this->soundInfo.channels == 1)
			this->soundInfo.format = AL_FORMAT_MONO16;
		else
			this->soundInfo.format = AL_FORMAT_STEREO16;
	}
	else if (this->soundInfo.bitsPerChannel == 8)
	{
		if (this->soundInfo.channels == 1)
			this->soundInfo.format = AL_FORMAT_MONO8;
		else
			this->soundInfo.format = AL_FORMAT_STEREO8;
	}

	for (uint32 i = 0; i < PRELOAD_BUFFERS_COUNT; i++)
	{
		SoundBuffer * buf = SoundManager::GetInstance()->GetFreeBuffer();
		if (buf == NULL)
		{
			printf("Not enough free sound-buffers");
			continue;
		}
		this->buffers[i] = buf;
	}

	this->Preload();
}

bool SoundObject::Preload()
{
	for (int i = 0; i < PRELOAD_BUFFERS_COUNT; i++)
	{		
		if (this->buffers[i] == NULL)
			continue;
		
		if (this->PreloadBuffer(this->buffers[i]->refID) == false)
		{
			//preload raw data - all buffers were not used, free them
			for (int j = i; j < PRELOAD_BUFFERS_COUNT; j++)
			{
				SoundManager::GetInstance()->FreeBuffer(this->buffers[j]);
				this->buffers[j] = NULL;
			}

			return false;
		}
	}

	return true;
}

bool SoundObject::PreloadBuffer(int bufferID)
{
	std::vector<char> decompressBuffer;
	this->soundFileWrapper->DecompressStream(decompressBuffer, this->settings.loop);

	if (decompressBuffer.size() == 0)
	{
		//nothing more to read
		return false;
	}

	AL_CHECK( alBufferData(bufferID, this->soundInfo.format, &decompressBuffer[0], static_cast<ALsizei>(decompressBuffer.size()), this->soundInfo.freqency) );

	//printf("Reload...");

	return true;
}

void SoundObject::Play()
{
	if (this->state == PLAYING)
	{
		//can not start play same sound twice
		return;
	}

	if (this->state == PAUSED)
	{
		this->state = PLAYING;

		AL_CHECK(alSourcef(this->source->refID, AL_PITCH, this->settings.pitch));
		AL_CHECK(alSourcef(this->source->refID, AL_GAIN, this->settings.gain));
		AL_CHECK(alSource3f(this->source->refID, AL_POSITION, this->settings.pos.x, this->settings.pos.y, this->settings.pos.z));
		AL_CHECK(alSource3f(this->source->refID, AL_VELOCITY, this->settings.velocity.x, this->settings.velocity.y, this->settings.velocity.z));
		AL_CHECK(alSourcei(this->source->refID, AL_LOOPING, false));

		AL_CHECK(alSourcePlay(this->source->refID) );

		return;
	}

	this->source = SoundManager::GetInstance()->GetFreeSource();

	if (this->source == NULL)
		return;

	AL_CHECK(alSourcef(this->source->refID, AL_PITCH, this->settings.pitch)) ;	
	AL_CHECK(alSourcef(this->source->refID, AL_GAIN, this->settings.gain) );	
	AL_CHECK(alSource3f(this->source->refID, AL_POSITION, this->settings.pos.x, this->settings.pos.y, this->settings.pos.z) );	
	AL_CHECK(alSource3f(this->source->refID, AL_VELOCITY, this->settings.velocity.x, this->settings.velocity.y, this->settings.velocity.z) );	
	AL_CHECK( alSourcei(this->source->refID, AL_LOOPING, this->settings.loop) );
	AL_CHECK( alSourcef(this->source->refID, AL_MAX_DISTANCE, 50.0f));
	AL_CHECK( alSourcei(source->refID, AL_REFERENCE_DISTANCE, 30.0));
	// The distance that the source will be the quietest (if the listener is
	// farther, it won't be any quieter than if they were at this distance)
	//alSourcei(source->refID, AL_MAX_DISTANCE, FLT_MAX);
	int usedBuffersCount = 0;
	for (int i = 0; i < PRELOAD_BUFFERS_COUNT; i++)
	{		
		if (this->buffers[i] == NULL)
			continue;
		
		AL_CHECK(alSourceQueueBuffers(this->source->refID, 1, &this->buffers[i]->refID) );
		usedBuffersCount++;
	}

	this->playedCount++;	
	AL_CHECK(alSourcePlay(this->source->refID) );
	this->state = PLAYING;
	this->activeBufferID = 0;
	this->remainBuffers = usedBuffersCount;
}

void SoundObject::Pause()
{
	if (this->state != PLAYING)
		return;

	this->state = PAUSED;

	AL_CHECK( alSourcePause(this->source->refID) );
}

void SoundObject::Stop()
{
	if (this->state == STOPPED)
		return;

	//while(this->spinLock){}

	//this->spinLock = true;
	this->state = STOPPED;

	//alSourceRewind(this->source->refID);

	AL_CHECK(alSourceStop(this->source->refID) );


	for (int i = 0; i < PRELOAD_BUFFERS_COUNT; i++)
	{		
		if (this->buffers[i] == NULL)
			continue;
		
		AL_CHECK(alSourceUnqueueBuffers(this->source->refID, 1, &this->buffers[i]->refID) );
	}


	SoundManager::GetInstance()->FreeSource(this->source);

	this->soundFileWrapper->ResetStream();

	for (unsigned int i = 0; i < PRELOAD_BUFFERS_COUNT; i++)
	{
		SoundManager::GetInstance()->FreeBuffer(this->buffers[i]);

		SoundBuffer * buf = SoundManager::GetInstance()->GetFreeBuffer();
		if (buf == NULL)
		{
			printf("Not enought free sound-buffers");
			continue;
		}
		this->buffers[i] = buf;
	}

	this->Preload();
	this->spinLock = false;

}

float lastTimeTmp = 0;
void SoundObject::Update()
{
	//while(this->spinLock){}

	if (this->state != PLAYING)
		return;
	
	this->spinLock = true;

	float result = this->GetTime();
	if (lastTimeTmp != result)
	{
		//printf("Time: %f\n", result);
		lastTimeTmp = result;
	}

	// get the processed buffer count
	int buffersProcessed = 0;
	AL_CHECK(alGetSourcei(this->source->refID, AL_BUFFERS_PROCESSED, &buffersProcessed) );

	// check to see if we have a buffer to deQ
	if (buffersProcessed > 0) 
	{
		if (buffersProcessed > 1)
		{
			//we have processed more than 1 buffer since last call of Update method
			//we should probably reload more buffers than just the one
			printf("Processed more than 1 buffer since last Update");
		}

		this->activeBufferID += buffersProcessed;
		this->activeBufferID %= PRELOAD_BUFFERS_COUNT;

		// great! deQ a buffer and re-fill it
		unsigned int bufferID;

		// remove the buffer form the source
		AL_CHECK(alSourceUnqueueBuffers(this->source->refID, 1, &bufferID) );

		// fill the buffer up and reQ! 
		// if we cant fill it up then we are finished
		// in which case we dont need to re-Q
		// return NO if we dont have more buffers to Q

		if (this->state == STOPPED)
		{
			//put it back
			AL_CHECK(alSourceQueueBuffers(this->source->refID, 1, &bufferID) );
			this->spinLock = false;					
			return;
		}

		if (this->PreloadBuffer(bufferID) == false)
		{
			//put it back
			//AL_CHECK( alSourceQueueBuffers(this->source->refID, 1, &bufferID) );
			//this->spinLock = false;
			//this->Stop();
			//return;

			this->remainBuffers--;
		}

		//put it back
		AL_CHECK(alSourceQueueBuffers(this->source->refID, 1, &bufferID) );
	}

	this->spinLock = false;
	if (this->remainBuffers <= 0)
		this->Stop();
}

void SoundObject::SetPosition(const Vec3& pos)
{
	this->settings.pos = pos;
	AL_CHECK(alSource3f(this->source->refID, AL_POSITION, this->settings.pos.x, this->settings.pos.y, this->settings.pos.z) );	
}

void SoundObject::SetRelative(bool ye)
{
	AL_CHECK(alSourcef(this->source->refID, AL_MIN_GAIN, ye ? 1.0f: 0.0f));
	AL_CHECK(alSourcei(this->source->refID, AL_SOURCE_RELATIVE, ye ? AL_TRUE : AL_FALSE));
}