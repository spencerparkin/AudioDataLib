#include "SDLAudioPlayer.h"

using namespace AudioDataLib;

//------------------------------------- SDLAudioPlayer -------------------------------------

SDLAudioPlayer::SDLAudioPlayer()
{
	this->audioStream = nullptr;
	this->audioDeviceID = 0;
	::memset(&this->audioSpec, 0, sizeof(SDL_AudioSpec));
}

/*virtual*/ SDLAudioPlayer::~SDLAudioPlayer()
{
}

bool SDLAudioPlayer::Setup(Error& error)
{
	if (!this->audioStream)
	{
		error.Add("No audio stream set.");
		return false;
	}

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

	std::string chosenAudioDeviceName = SDL_GetAudioDeviceName(1, 0);		// TODO: Maybe let the user pick?
	printf("\nChosen device: %s\n\n", chosenAudioDeviceName.c_str());

	this->audioSpec.freq = 48000;
	this->audioSpec.format = AUDIO_S16LSB;
	this->audioSpec.channels = 1;
	this->audioSpec.callback = &SDLAudioPlayer::AudioCallbackEntryPoint;
	this->audioSpec.userdata = this;

	this->audioDeviceID = SDL_OpenAudioDevice(chosenAudioDeviceName.c_str(), 0, &this->audioSpec, &this->audioSpec, SDL_AUDIO_ALLOW_ANY_CHANGE);
	if (this->audioDeviceID == 0)
	{
		error.Add(FormatString("Failed to open output audio device: %s", SDL_GetError()));
		return false;
	}
	
	AudioData::Format format;
	format.numChannels = this->audioSpec.channels;
	format.bitsPerSample = SDL_AUDIO_BITSIZE(this->audioSpec.format);
	format.framesPerSecond = this->audioSpec.freq;
	
	if (SDL_AUDIO_ISFLOAT(this->audioSpec.format))
		format.sampleType = AudioData::Format::FLOAT;
	else if (SDL_AUDIO_ISSIGNED(this->audioSpec.format))	// TODO: Our audio data is always signed.  Do I need to add support for the unsigned case?
		format.sampleType = AudioData::Format::SIGNED_INTEGER;
	else
	{
		error.Add("SDL format not yet supported by AudioDataLib.");
		return false;
	}

	this->audioStream->SetFormat(format);

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
	uint64_t numBytesRead = this->audioStream->ReadBytesFromStream(buffer, uint64_t(length));
	for (uint64_t i = numBytesRead; i < uint64_t(length); i++)
		buffer[i] = this->audioSpec.silence;
}