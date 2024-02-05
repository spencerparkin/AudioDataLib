#pragma once

#include "AudioData.h"
#include "Error.h"
#include "SDL.h"
#undef main
#include "ByteStream.h"

class SDLAudio
{
public:
	enum AudioDirection
	{
		SOUND_IN,
		SOUND_OUT
	};

	SDLAudio(AudioDirection audioDirection);
	virtual ~SDLAudio();

	virtual bool Setup(const std::string& deviceSubStr, AudioDataLib::Error& error);
	virtual bool Shutdown(AudioDataLib::Error& error);

	void SetAudioStream(AudioDataLib::AudioStream* audioStream) { this->audioStream = audioStream; }
	AudioDataLib::AudioStream* GetAudioStream() { return this->audioStream; }

protected:

	static void SDLCALL AudioCallbackEntryPoint(void* userData, Uint8* buffer, int length);
	virtual void AudioCallback(Uint8* buffer, int length);

	AudioDataLib::AudioStream* audioStream;
	SDL_AudioSpec audioSpec;
	SDL_AudioDeviceID audioDeviceID;

	AudioDirection audioDirection;
};