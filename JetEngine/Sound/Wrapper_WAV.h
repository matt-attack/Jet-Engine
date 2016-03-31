#ifndef WRAPPER_WAV_H
#define WRAPPER_WAV_H

//This sound library is a modified version of that by Martin Prantl released under this license
//http://www.gamedev.net/page/resources/_/gdnethelp/gamedevnet-open-license-r2956
//http://www.gamedev.net/page/resources/_/technical/game-programming/basic-openal-sound-manager-for-your-project-r3791


struct SoundInfo;

#define WAV_BUFFER_SIZE 2048//2097152

#include <vector>
#include <cstring>
#include "OpenAL.h"

#include "ISoundFileWrapper.h"

typedef struct wav_file
{
	FILE * f;
	char* curPtr;
	char* filePtr;
	size_t fileSize;
	size_t processedSize;
} wav_file;

#pragma pack(push)
#pragma pack(1)
typedef struct WAV_DESC
{
	unsigned char riff[4];
	unsigned long size;
	unsigned char wave[4];
} WAV_DESC;


typedef struct WAV_FORMAT
{
	unsigned char id[4];
	unsigned long size;
	unsigned short format;
	unsigned short channels;
	unsigned long sampleRate;
	unsigned long byteRate;
	unsigned short blockAlign;
	uint16 bitsPerSample;
	uint16 extraformat;
	//short test;
} WAV_FORMAT;

typedef struct WAV_CHUNK
{
	unsigned char id[4];
	unsigned int size;
} WAV_CHUNK;
#pragma pack(pop)
class WrapperWav : public ISoundFileWrapper
{
public:
	WrapperWav(int minDecompressLengthAtOnce = -1);
	~WrapperWav();

	virtual void LoadFromMemory(char * data, int dataSize, SoundInfo * soundInfo);
	virtual void LoadFromFile(FILE * f, SoundInfo * soundInfo);
	virtual void DecompressStream(std::vector<char> & decompressBuffer, bool inLoop = false);
	virtual void DecompressAll(std::vector<char> & decompressBuffer);
	virtual void ResetStream();

	virtual void Seek(size_t pos, SEEK_POS start);
	virtual size_t GetCurrentStreamPos() const;

	virtual float GetTime() const;
	virtual float GetTotalTime() const;

private:
	wav_file t;
	char bufArray[WAV_BUFFER_SIZE];


	int minProcesssLengthAtOnce;

	WAV_DESC desc;
	WAV_FORMAT format;

	WAV_CHUNK curChunk;
	int curBufSize;

	void ReadData(void * dst, size_t size);
};
#endif