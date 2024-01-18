#pragma once

#include "AudioData.h"
#include "Error.h"
#include "SDL.h"
#undef main
#include "Mutex.h"
#include "AudioSink.h"
#include <mutex>

class SDLAudioPlayer
{
public:
	SDLAudioPlayer();
	virtual ~SDLAudioPlayer();

	bool Setup(AudioDataLib::Error& error);
	bool Shutdown(AudioDataLib::Error& error);
	bool PlayAudio(AudioDataLib::AudioData* audioData, AudioDataLib::Error& error);
	bool IsPlayingSomething();
	bool ManagePlayback(AudioDataLib::Error& error);

private:

	class Mutex : public AudioDataLib::Mutex
	{
	public:
		Mutex();
		virtual ~Mutex();

		virtual void Lock() override;
		virtual void Unlock() override;

	private:
		std::mutex mutex;
	};

	static void SDLCALL AudioCallbackEntryPoint(void* userData, Uint8* buffer, int length);
	void AudioCallback(Uint8* buffer, int length);

	AudioDataLib::AudioSink audioSink;
	SDL_AudioSpec audioSpec;
	SDL_AudioDeviceID audioDeviceID;
	Mutex mutex;
};