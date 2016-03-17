#include "Wrapper_WAV.h"
#include "SoundObject.h"

#include "OpenAL.h"

#include <string.h>
#include <climits>
#include <algorithm>
#include <xutility>

WrapperWav::WrapperWav(int minProcesssLengthAtOnce)
{
	this->minProcesssLengthAtOnce = minProcesssLengthAtOnce;

	if (this->minProcesssLengthAtOnce == -1)
	{
		this->minProcesssLengthAtOnce = INT_MAX;
	}
	else
	{
		if (this->minProcesssLengthAtOnce % WAV_BUFFER_SIZE != 0)
		{
			printf("OGG buffer size and OpenAL buffer size are not modulable !!");
			printf("this->minProcesssLengthAtOnce % WAV_BUFFER_SIZE != 0");
		}
	}

	memset(&this->curChunk, 0, sizeof(WAV_CHUNK));
	memset(&this->t, 0, sizeof(wav_file));
}


WrapperWav::~WrapperWav()
{

}


void WrapperWav::LoadFromMemory(char* data, int dataSize, SoundInfo* soundInfo)
{
	this->t.curPtr = this->t.filePtr = data;// +sizeof(WAV_DESC) + sizeof(WAV_FORMAT));
	this->t.fileSize = dataSize;
	this->t.processedSize = 0;

	this->ReadData(&this->desc, sizeof(WAV_DESC));
	this->ReadData(&this->format, sizeof(WAV_FORMAT));
	//memcpy(&this->desc, data, sizeof(WAV_DESC));
	//memcpy(&this->format, data + sizeof(WAV_DESC), sizeof(WAV_FORMAT));

	memset(soundInfo, 0, sizeof(SoundInfo));

	soundInfo->channels = this->format.channels;
	soundInfo->bitrate_lower = this->format.byteRate;
	soundInfo->bitrate_nominal = this->format.byteRate;
	soundInfo->bitrate_upper = this->format.byteRate;
	soundInfo->bitrate_window = this->format.byteRate;
	soundInfo->freqency = this->format.sampleRate;
	soundInfo->bitsPerChannel = this->format.bitsPerSample;

	soundInfo->seekable = true;

	this->ResetStream();
}


void WrapperWav::LoadFromFile(FILE * f, SoundInfo * soundInfo)
{
	//this->LoadFromMemory(data, size, soundInfo);
	//TO DO
}

void WrapperWav::ResetStream()
{
	if (this->t.f != NULL)
		fseek(this->t.f, sizeof(WAV_DESC) + sizeof(WAV_FORMAT), SEEK_SET);
	else 
		this->t.curPtr = this->t.filePtr + sizeof(WAV_DESC) + sizeof(WAV_FORMAT);//72

	this->curChunk.size = 0;
	this->t.processedSize = sizeof(WAV_DESC) + sizeof(WAV_FORMAT);
}

void WrapperWav::ReadData(void * dst, size_t size)
{
	if (this->t.f != NULL)
	{
		//read from file directly
		fread(dst, sizeof(uint8), size, this->t.f);
	}
	else 
	{
		//read from memory stream
		memcpy(dst, this->t.curPtr, size);
		this->t.curPtr += size;
	}

	this->t.processedSize += size;
}

void WrapperWav::Seek(size_t size, SEEK_POS start)
{
	if (start == SEEK_POS::START)
	{
		this->ResetStream();
	}

	if (this->t.f != NULL)
	{
		//read from file directly
		//fread(dst, sizeof(uint8), size, this->t.f);
		fseek(this->t.f, size, SEEK_CUR);
	}
	else
	{
		//read from memory stream
		this->t.curPtr += size;
	}

	this->t.processedSize += size;
}

size_t WrapperWav::GetCurrentStreamPos() const
{
	long pos = 0;

	if (this->t.f != NULL)
		pos = ftell(this->t.f);
	else
		pos = (this->t.curPtr - this->t.filePtr);

	pos -= sizeof(WAV_DESC) + sizeof(WAV_FORMAT);

	if (pos < 0)
		pos = 0;

	return pos;
}

float WrapperWav::GetTime() const
{
	float pos = static_cast<float>(this->GetCurrentStreamPos());

	return pos / (this->format.sampleRate * this->format.channels * this->format.bitsPerSample / 8);
}

float WrapperWav::GetTotalTime() const
{
	float pos = this->t.fileSize - sizeof(WAV_DESC) - sizeof(WAV_FORMAT) - sizeof(WAV_CHUNK);

	float t = pos / (this->format.sampleRate * this->format.channels * this->format.bitsPerSample / 8);

	return t;
}

void WrapperWav::DecompressAll(std::vector<char> & decompressBuffer)
{
	//backup current state
	WAV_CHUNK curChunkBckp = this->curChunk;
	char bufArrayBckp[WAV_BUFFER_SIZE];
	memcpy(bufArrayBckp, this->bufArray, sizeof(char) * WAV_BUFFER_SIZE);

	int curBufSizeBckp = this->curBufSize;

	size_t pos = this->GetCurrentStreamPos();
	this->ResetStream();

	std::vector<char> rawBuffer;
	decompressBuffer.clear();

	//run fill data
	while (true)
	{
		this->DecompressStream(rawBuffer, false);

		if (rawBuffer.size() == 0)
			break;

		decompressBuffer.insert(decompressBuffer.end(), rawBuffer.begin(), rawBuffer.end());
	}

	//restore backup
	this->Seek(pos, ISoundFileWrapper::SEEK_POS::START);

	this->curChunk = curChunkBckp;
	memcpy(this->bufArray, bufArrayBckp, sizeof(char) * WAV_BUFFER_SIZE);

	this->curBufSize = curBufSizeBckp;
}

void WrapperWav::DecompressStream(std::vector<char> & decompressBuffer, bool inLoop)
{
	decompressBuffer.clear();

	if (this->t.curPtr == 0)
		return;//we already read it all

	bool eof = false;
	int curBufSize = 0;
	do
	{
		do
		{
			curBufSize = 0;

			while (curBufSize < WAV_BUFFER_SIZE)
			{
				uint32 remainToRead = WAV_BUFFER_SIZE - curBufSize;

				if (this->curChunk.size <= 0)
				{
					//need load chunk info
					if (this->t.curPtr[0] == 't' && this->t.curPtr[1] == 'a')
					{
						short t;
						this->t.curPtr -= 2;
						this->t.processedSize -= 2;
					}

					this->ReadData(&this->curChunk, sizeof(WAV_CHUNK));
				}

				// Check for .WAV data chunk
				if (
					(this->curChunk.id[0] == 'd') && (this->curChunk.id[1] == 'a') &&
					(this->curChunk.id[2] == 't') && (this->curChunk.id[3] == 'a')
					)
				{
					uint64 readSize = std::min(this->curChunk.size, remainToRead); //how many data can we read in current chunk

					this->ReadData(this->bufArray + curBufSize, readSize);

					curBufSize += readSize; //buffer filled from [0...curBufSize]
					this->curChunk.size -= readSize;//in current chunk, remain to read
				}
				else
				{
					//not a "data" chunk - advance stream
					this->Seek(this->curChunk.size, SEEK_POS::CURRENT);

					this->curChunk.size = 0;
				}

				if (this->t.processedSize >= this->t.fileSize)
				{
					eof = true;
					break;
				}
			}

			// Append to end of buffer
			decompressBuffer.insert(decompressBuffer.end(), this->bufArray, this->bufArray + curBufSize);

			if (static_cast<int>(decompressBuffer.size()) >= this->minProcesssLengthAtOnce)
				return;
		} while (!eof);

		if (inLoop)
			this->ResetStream();
		else
			this->t.curPtr = 0;

		if (this->minProcesssLengthAtOnce == INT_MAX)
			return;
	} while (inLoop);
}
