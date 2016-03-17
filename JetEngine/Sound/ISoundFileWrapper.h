#ifndef I_SOUND_WRAPPER_H
#define I_SOUND_WRAPPER_H

struct SoundInfo;

#include <vector>
#include <cstring>

class ISoundFileWrapper
{
public:
	enum SEEK_POS 
	{ 
		START = 0, 
		CURRENT = 1 
	};

	ISoundFileWrapper(int minDecompressLengthAtOnce = -1) {};
	virtual ~ISoundFileWrapper() {};

	virtual void LoadFromMemory(char * data, int dataSize, SoundInfo * soundInfo) = 0;
	virtual void LoadFromFile(FILE * f, SoundInfo * soundInfo) = 0;
	virtual void DecompressStream(std::vector<char> & decompressBuffer, bool inLoop = false) = 0;
	virtual void DecompressAll(std::vector<char> & decompressBuffer) = 0;
	virtual void ResetStream() = 0;

	virtual void Seek(size_t pos, SEEK_POS start) = 0;
	virtual size_t GetCurrentStreamPos() const = 0;

	virtual float GetTime() const = 0;
	virtual float GetTotalTime() const = 0;
};

#endif