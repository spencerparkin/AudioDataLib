#include "SDLAudioPlayer.h"

using namespace AudioDataLib;

//------------------------------------- SDLAudioPlayer -------------------------------------

SDLAudioPlayer::SDLAudioPlayer() : audioSink(true)
{
	this->audioDeviceID = 0;
}

/*virtual*/ SDLAudioPlayer::~SDLAudioPlayer()
{
}

bool SDLAudioPlayer::Setup(Error& error)
{
	int result = SDL_Init(SDL_INIT_AUDIO);
	if (result != 0)
	{
		error.Add(FormatString("Failed to initialized SDL audio subsystem: %s", SDL_GetError()));
		return false;
	}

	int numAudioDevices = SDL_GetNumAudioDevices(0);
	if (numAudioDevices == 0)
	{
		error.Add("SDL did not detect any audio output devices.");
		return false;
	}

	printf("Found %d output audio device(s)...\n", numAudioDevices);
	for (int i = 0; i < numAudioDevices; i++)
	{
		std::string audioDeviceName = SDL_GetAudioDeviceName(i, 0);
		printf("%d: %s\n", i + 1, audioDeviceName.c_str());
	}

	std::string chosenAudioDeviceName = SDL_GetAudioDeviceName(0, 0);		// TODO: Maybe let the user pick?

	this->audioSpec.freq = 44000;
	this->audioSpec.format = AUDIO_S16LSB;
	this->audioSpec.channels = 1;
	this->audioSpec.callback = &SDLAudioPlayer::AudioCallbackEntryPoint;

	this->audioDeviceID = SDL_OpenAudioDevice(chosenAudioDeviceName.c_str(), 0, &this->audioSpec, &this->audioSpec, SDL_AUDIO_ALLOW_ANY_CHANGE);
	if (this->audioDeviceID == 0)
	{
		error.Add(FormatString("Failed to open output audio device: %s", SDL_GetError()));
		return false;
	}

	// TODO: Our audio data is always signed.  Do I need to add support for the unsigned case?
	AudioData::Format format;
	format.numChannels = this->audioSpec.channels;
	format.bitsPerSample = SDL_AUDIO_BITSIZE(this->audioSpec.format);
	format.framesPerSecond = this->audioSpec.freq;

	this->audioSink.SetAudioOutput(new ThreadSafeAudioStream(format, &this->mutex, false));

	// This will cause our callback to start getting called.
	SDL_PauseAudioDevice(this->audioDeviceID, 0);

	return true;
}

bool SDLAudioPlayer::Shutdown(Error& error)
{
	if (this->audioDeviceID != 0)
	{
		SDL_CloseAudioDevice(this->audioDeviceID);
		this->audioDeviceID = 0;
	}

	SDL_QuitSubSystem(SDL_INIT_AUDIO);
	SDL_Quit();

	return true;
}

/*static*/ void SDLCALL SDLAudioPlayer::AudioCallbackEntryPoint(void* userData, Uint8* buffer, int length)
{
	auto player = static_cast<SDLAudioPlayer*>(userData);
	player->AudioCallback(buffer, length);
}

void SDLAudioPlayer::AudioCallback(Uint8* buffer, int length)
{
	AudioStream* audioStreamOut = this->audioSink.GetAudioOutput();
	uint64_t numBytesRead = audioStreamOut->ReadBytesFromStream(buffer, uint64_t(length));
	for (uint64_t i = numBytesRead; i < uint64_t(length); i++)
		buffer[i] = this->audioSpec.silence;
}

bool SDLAudioPlayer::PlayAudio(AudioData* audioData, Error& error)
{
	this->audioSink.AddAudioInput(new AudioStream(audioData));
	return true;
}

bool SDLAudioPlayer::IsPlayingSomething()
{
	return this->audioSink.GetAudioInputCount() > 0;
}

bool SDLAudioPlayer::ManagePlayback(AudioDataLib::Error& error)
{
	this->audioSink.GenerateAudio(0.1, 0.1);
	return true;
}

//------------------------------------- SDLAudioPlayer::Mutex -------------------------------------

SDLAudioPlayer::Mutex::Mutex()
{
}

/*virtual*/ SDLAudioPlayer::Mutex::~Mutex()
{
}

/*virtual*/ void SDLAudioPlayer::Mutex::Lock()
{
	this->mutex.lock();
}

/*virtual*/ void SDLAudioPlayer::Mutex::Unlock()
{
	this->mutex.unlock();
}