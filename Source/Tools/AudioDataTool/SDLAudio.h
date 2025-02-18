#pragma once

#include "AudioDataLib/FileDatas/AudioData.h"
#include "AudioDataLib/ErrorSystem.h"
#include "SDL.h"
#undef main
#include "AudioDataLib/ByteStream.h"

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

	virtual bool Setup(const std::string& deviceSubStr);
	virtual bool Shutdown();

	void SetAudioStream(std::shared_ptr<AudioDataLib::AudioStream> audioStream) { this->audioStream = audioStream; }
	std::shared_ptr<AudioDataLib::AudioStream> GetAudioStream() { return this->audioStream; }

	void SetRecordedAudioStream(std::shared_ptr<AudioDataLib::AudioStream> recordedAudioStream) { this->recordedAudioStream = recordedAudioStream; }
	std::shared_ptr<AudioDataLib::AudioStream> GetRecordedAudioStream() { return this->recordedAudioStream; }

protected:

	static void SDLCALL AudioCallbackEntryPoint(void* userData, Uint8* buffer, int length);
	virtual void AudioCallback(Uint8* buffer, int length);

	std::shared_ptr<AudioDataLib::AudioStream> audioStream;
	std::shared_ptr<AudioDataLib::AudioStream> recordedAudioStream;

	SDL_AudioSpec audioSpec;
	SDL_AudioDeviceID audioDeviceID;

	AudioDirection audioDirection;
};