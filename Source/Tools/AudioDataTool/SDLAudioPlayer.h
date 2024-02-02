#pragma once

#include "AudioData.h"
#include "Error.h"
#include "SDL.h"
#undef main
#include "ByteStream.h"

class SDLAudioPlayer
{
public:
	SDLAudioPlayer();
	virtual ~SDLAudioPlayer();

	bool Setup(const std::string& deviceSubStr, AudioDataLib::Error& error);
	bool Shutdown(AudioDataLib::Error& error);

	void SetAudioStream(AudioDataLib::AudioStream* audioStream) { this->audioStream = audioStream; }
	AudioDataLib::AudioStream* GetAudioStream() { return this->audioStream; }

private:

	static void SDLCALL AudioCallbackEntryPoint(void* userData, Uint8* buffer, int length);
	void AudioCallback(Uint8* buffer, int length);

	AudioDataLib::AudioStream* audioStream;
	SDL_AudioSpec audioSpec;
	SDL_AudioDeviceID audioDeviceID;
};