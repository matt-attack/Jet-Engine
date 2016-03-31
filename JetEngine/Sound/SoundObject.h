#ifndef SOUND_OBJECT_H
#define SOUND_OBJECT_H

//This sound library is a modified version of that by Martin Prantl released under this license
//http://www.gamedev.net/page/resources/_/gdnethelp/gamedevnet-open-license-r2956
//http://www.gamedev.net/page/resources/_/technical/game-programming/basic-openal-sound-manager-for-your-project-r3791

struct SoundSource;
struct SoundBuffer;
class ISoundFileWrapper;

#define PRELOAD_BUFFERS_COUNT 3

#include <string>
#include <vector>

#include "../Math/Vector.h"

typedef struct SoundSettings
{
	std::string name;
	std::string fileName;

	float pitch;
	float gain;
	bool loop;
	Vec3 pos;
	Vec3 velocity;
} SoundSettings;

typedef struct SoundInfo 
{
	int freqency;
	int channels;
	int format;
	int bitsPerChannel;

	/* The below bitrate declarations are *hints*.
	Combinations of the three values carry the following implications:

	all three set to the same value:
	implies a fixed rate bitstream
	only nominal set:
	implies a VBR stream that averages the nominal bitrate.  No hard
	upper/lower limit
	upper and or lower set:
	implies a VBR bitstream that obeys the bitrate limits. nominal
	may also be set to give a nominal rate.
	none set:
	the coder does not care to speculate.
	*/

	long bitrate_upper;
	long bitrate_nominal;
	long bitrate_lower;
	long bitrate_window;

	bool seekable;

} SoundInfo;

class SoundObject
{
public:

	typedef enum SOUND_STATE { PLAYING = 1, PAUSED = 2, STOPPED = 3} SOUND_STATE; 

	SoundObject(const std::string & fileName, const std::string & name);
	SoundObject(const SoundSettings & settings);
	SoundObject(char * rawData, unsigned int dataSize, SoundInfo soundInfo, const std::string & name);
	~SoundObject();

	const SoundSettings & GetSettings() const;
	const SoundInfo & GetInfo() const;

	void PlayInLoop(bool val);

	float GetTime() const;
	float GetMaxBufferedTime() const;
	int GetPlayedCount() const;

	bool IsPlaying() const;

	void Release();

	void Play();
	void Pause();
	void Stop();

	void Rewind();

	void SetPosition(const Vec3& pos);
	void SetRelative(bool yes);


	void GetRawData(std::vector<char> * rawData);

	friend class SoundManager;

protected:

	SoundSource * source;
	SoundBuffer * buffers[PRELOAD_BUFFERS_COUNT];
	int activeBufferID;
	int SINGLE_BUFFER_SIZE;

	SoundSettings settings;
	SoundInfo soundInfo;

	FILE * vfsFile;
	char * soundData;
	int dataSize;
	ISoundFileWrapper * soundFileWrapper;

	SOUND_STATE state;

	int remainBuffers;


	int playedCount;

	void LoadData();
	void LoadRawData(char * rawData, unsigned int dataSize);

	bool Preload();
	bool PreloadBuffer(int bufferID);
	void Update();

	bool spinLock;

	float GetBufferedTime(int buffersCount) const;
};

#endif