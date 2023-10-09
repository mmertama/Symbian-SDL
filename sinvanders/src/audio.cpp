/*
    SDL SPACE INVADERS - The Game
    Copyright (C) 2009  Markus Mertama

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#include <SDL_audio.h>
#include <vector>
#include "audio.h"

#include <SDL_endian.h>

using namespace std;



AudioException::AudioException(const string& reason) : iReason("Audio:" + reason)
    {}

const char* AudioException::what() const throw()
    {
   return iReason.c_str();
    }


class AudioData
    {
public:
	AudioData(int handle, SDL_AudioSpec* spec, int length, Uint8* buffer);
	AudioData& operator=(const AudioData& aData);
	void begin(bool loop);
	void stop();
	void close();
	enum {EActive = 0x1, ELooping = 0x2};
public:
    const int iHandle;
    SDL_AudioSpec* iSpec;
    int iLength;
    Uint8* iBuffer;
	int iRemainder;
	Uint8* iPos;
	int iFlags;
    };

AudioData::AudioData(int handle, SDL_AudioSpec* spec, int length, Uint8* buffer) : 
iHandle(handle), iSpec(spec), iLength(length), iBuffer(buffer), iFlags(0)
	{
	}

AudioData& AudioData::operator=(const AudioData& aData)
    {
    if(iBuffer != aData.iBuffer)
        {
        close();
        iSpec = aData.iSpec;
        iLength = aData.iLength;
        iBuffer = aData.iBuffer;
        iRemainder = aData.iRemainder;
        iPos = aData.iPos;
        }
    return *this;
    }
	
void AudioData::close()
	{
	SDL_FreeWAV(iBuffer);
	iBuffer = NULL;
	iLength = 0;
	iRemainder = 0;
	iPos = NULL;
	delete iSpec;
	iSpec = NULL;
	}	
	
void AudioData::begin(bool loop)
	{
	iRemainder = iLength;
	iPos = iBuffer;
	if(loop)
		iFlags |= ELooping;
	else
		iFlags &= ~ELooping;

	iFlags |= EActive;
	}	
	
void AudioData::stop()
	{
	iFlags &= ~EActive;
	}	
	
class AudioPlr
    {
public:
	AudioPlr();
	void addAudio(const AudioData& data);
	AudioData* get(int handle);
	~AudioPlr();
private:
	static void callback(void* userData, Uint8* stream, int len);
	void playAudio(Uint8* stream, int len);
private:
    vector<AudioData> iChannels;
	SDL_AudioSpec iSpec;
    };

AudioPlr::AudioPlr() 
	{
	SDL_AudioSpec desired;
    desired.freq = 22050;
    desired.format = AUDIO_S16;
    desired.channels = 1;
    desired.samples = 4096;
    desired.callback = callback;
    desired.userdata = this;
    if(SDL_OpenAudio(&desired, &iSpec) < 0)
        {
        throw AudioException(SDL_GetError());
        }
	}	

AudioPlr::~AudioPlr()
	{
	for(vector<AudioData>::iterator ii = iChannels.begin(); ii != iChannels.end(); ii++)
		{
		(*ii).close();
		}
	iChannels.clear();
	}

void AudioPlr::addAudio(const AudioData& data)
    {
    AudioData* d = get(data.iHandle);
    if(d != NULL)
        {
        d->close();
        *d = data;
        return;
        }
    iChannels.push_back(data);
    }   
	
AudioData* AudioPlr::get(int handle)
	{
	for(vector<AudioData>::iterator ii = iChannels.begin(); ii != iChannels.end(); ii++)
		{
		if((*ii).iHandle == handle)
			{
			return &(*ii);
			}
		}
	return NULL;
	}
	

void lsbAdd(Sint16* target, const Sint16* src, int len)
	{
	for(int i = 0; i < len; i++)
		{
		/*http://stackoverflow.com/questions/233148/c-pointers-and-arrays-question :-)*/
		/*Is this endianess stuff right? ... I cannot test this and I have a headache*/
		target[i] += (SDL_SwapLE16(src[i]));
		SDL_SwapLE16(target[i]);
		}
	}
	
/*http://sdl.beuc.net/sdl.wiki/Audio_Examples*/
void AudioPlr::callback(void* userData, Uint8* stream, int len)
    {
	static_cast<AudioPlr*>(userData)->playAudio(stream, len);
    }
	
void AudioPlr::playAudio(Uint8* stream8, int len8)
    {
	for(vector<AudioData>::iterator ii = iChannels.begin(); ii != iChannels.end(); ii++)
		{
		AudioData& d = (*ii);
		if(d.iFlags & AudioData::EActive)
			{
			Sint16* stream = reinterpret_cast<Sint16*>(stream8);
			Sint16* src = reinterpret_cast<Sint16*>(d.iPos);
			const int remainder = d.iRemainder / 2;
			const int len = len8 / 2;
			if(len <= remainder)
				{
				lsbAdd(stream, src, len);
				d.iRemainder -= len8;
				d.iPos += len8;
				}
			else
				{
				lsbAdd(stream, src, remainder);
				if(d.iFlags & AudioData::ELooping)
					{
					stream += remainder;
					d.iPos = d.iBuffer;
					Sint16* src = reinterpret_cast<Sint16*>(d.iPos);
					const int tail = len - remainder;
					lsbAdd(stream, src, tail);
					d.iRemainder = (d.iLength - tail * 2);
					d.iPos +=  (tail * 2);
					}
				else
					{
					d.iFlags &= ~AudioData::EActive;
					}
				}
			}
		}
    }

Audio::Audio()
    {
    iAudioPlr = new AudioPlr;
    pause();
    }

Audio::~Audio()
    {
	pause();
	delete iAudioPlr;
    SDL_CloseAudio();
	}

void Audio::resume()
	{
	SDL_PauseAudio(0);
	}
	
void Audio::pause()
	{
	SDL_PauseAudio(1);
	}

void Audio::set(const std::string& audioName, int handle)
    {
    const std::string name = audioName + ".wav";
    SDL_AudioSpec* spec = new SDL_AudioSpec;
    Uint32 length;
    Uint8 *buffer;
    if(NULL == SDL_LoadWAV(name.c_str(), spec, &buffer, &length))
        {
        throw AudioException(SDL_GetError());
        }
	if(spec->freq != 22050 || spec->format != AUDIO_S16 || spec->channels != 1 || spec->samples != 4096)
		{
		 throw AudioException(audioName +": Unsupported format");
		}
		
	AudioData* data = iAudioPlr->get(handle);
	delete data;
	iAudioPlr->addAudio(AudioData(handle, spec, length, buffer));
    }

void Audio::play(int handle, bool loop)
	{
	AudioData* data = iAudioPlr->get(handle);
	if(data == NULL)
		throw AudioException("handle not found");
	resume();
	data->begin(loop);
	}
	
	
void Audio::stop(int handle)
	{
	AudioData* data = iAudioPlr->get(handle);
	if(data == NULL)
		throw AudioException("handle not found");
	data->stop();
	}
