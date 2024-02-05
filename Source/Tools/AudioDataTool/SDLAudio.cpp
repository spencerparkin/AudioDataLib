#include "SDLAudio.h"

using namespace AudioDataLib;

//------------------------------------- SDLAudio -------------------------------------

SDLAudio::SDLAudio(AudioDirection audioDirection)
{
	this->audioDirection = audioDirection;
	this->audioStream = nullptr;
	this->audioDeviceID = 0;
	::memset(&this->audioSpec, 0, sizeof(SDL_AudioSpec));
}

/*virtual*/ SDLAudio::~SDLAudio()
{
}

bool SDLAudio::Setup(const std::string& deviceSubStr, Error& error)
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

	int isCapture = (this->audioDirection == AudioDirection::SOUND_IN) ? 1 : 0;
	int numAudioDevices = SDL_GetNumAudioDevices(isCapture);
	if (numAudioDevices == 0)
	{
		error.Add("SDL did not detect any audio output devices.");
		return false;
	}

	if (this->audioDirection == AudioDirection::SOUND_OUT)
		printf("Configured to produce audio output.\n");
	else
		printf("Configured to receive audio input.\n");

	printf("Found %d audio device(s)...\n", numAudioDevices);
	int j = 0;
	for (int i = 0; i < numAudioDevices; i++)
	{
		std::string audioDeviceName = SDL_GetAudioDeviceName(i, isCapture);
		printf("%d: %s\n", i + 1, audioDeviceName.c_str());

		if (deviceSubStr.length() > 0 && audioDeviceName.find(deviceSubStr) != std::string::npos)
			j = i;
	}

	std::string chosenAudioDeviceName = SDL_GetAudioDeviceName(j, isCapture);
	printf("\nChosen device: %s\n\n", chosenAudioDeviceName.c_str());

	this->audioSpec.freq = 48000;
	this->audioSpec.format = AUDIO_S16LSB;
	this->audioSpec.channels = 1;
	this->audioSpec.callback = &SDLAudio::AudioCallbackEntryPoint;
	this->audioSpec.userdata = this;

	this->audioDeviceID = SDL_OpenAudioDevice(chosenAudioDeviceName.c_str(), 0, &this->audioSpec, &this->audioSpec, SDL_AUDIO_ALLOW_ANY_CHANGE);
	if (this->audioDeviceID == 0)
	{
		error.Add(FormatString("Failed to open audio device: %s", SDL_GetError()));
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

bool SDLAudio::Shutdown(Error& error)
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

/*static*/ void SDLCALL SDLAudio::AudioCallbackEntryPoint(void* userData, Uint8* buffer, int length)
{
	auto player = static_cast<SDLAudio*>(userData);
	player->AudioCallback(buffer, length);
}

/*virtual*/ void SDLAudio::AudioCallback(Uint8* buffer, int length)
{
	switch (this->audioDirection)
	{
		case AudioDirection::SOUND_OUT:
		{
			uint64_t numBytesRead = this->audioStream->ReadBytesFromStream(buffer, uint64_t(length));
			for (uint64_t i = numBytesRead; i < uint64_t(length); i++)
				buffer[i] = this->audioSpec.silence;
			break;
		}
		case AudioDirection::SOUND_IN:
		{
			uint64_t numBytesWritten = this->audioStream->WriteBytesToStream(buffer, uint64_t(length));
			break;
		}
	}
}