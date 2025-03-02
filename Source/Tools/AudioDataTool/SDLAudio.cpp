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

bool SDLAudio::Setup(const std::string& deviceSubStr)
{
	if (!this->audioStream)
	{
		ErrorSystem::Get()->Add("No audio stream set.");
		return false;
	}

	int result = SDL_Init(SDL_INIT_AUDIO);
	if (result != 0)
	{
		ErrorSystem::Get()->Add(std::format("Failed to initialized SDL audio subsystem: {}", SDL_GetError()));
		return false;
	}

	int numAudioDevices = 0;
	SDL_AudioDeviceID* audioDeviceIDArray = nullptr;
	switch (this->audioDirection)
	{
	case AudioDirection::SOUND_IN:
		audioDeviceIDArray = SDL_GetAudioRecordingDevices(&numAudioDevices);
		break;
	case AudioDirection::SOUND_OUT:
		audioDeviceIDArray = SDL_GetAudioPlaybackDevices(&numAudioDevices);
		break;
	}

	if (numAudioDevices == 0)
	{
		ErrorSystem::Get()->Add("SDL did not detect any audio output devices.");
		return false;
	}

	if (this->audioDirection == AudioDirection::SOUND_OUT)
		printf("Configured to produce audio output.\n");
	else
		printf("Configured to receive audio input.\n");

	printf("Found %d audio device(s)...\n", numAudioDevices);
	this->audioDeviceID = 0;
	for (int i = 0; i < numAudioDevices; i++)
	{
		std::string audioDeviceName = SDL_GetAudioDeviceName(audioDeviceIDArray[i]);
		printf("%d: %s\n", int(audioDeviceIDArray[i]), audioDeviceName.c_str());

		if (deviceSubStr.length() > 0 && audioDeviceName.find(deviceSubStr) != std::string::npos)
			this->audioDeviceID = audioDeviceIDArray[i];
	}

	if (this->audioDeviceID == 0)
	{
		ErrorSystem::Get()->Add("Did not find desired audio device.");
		return false;
	}

	std::string chosenAudioDeviceName = SDL_GetAudioDeviceName(this->audioDeviceID);
	printf("\nChosen device: %s\n\n", chosenAudioDeviceName.c_str());

	this->audioSpec.freq = 48000;
	this->audioSpec.format = SDL_AUDIO_S16LE;
	this->audioSpec.channels = 1;

	SDL_AudioStream* audioStream = SDL_OpenAudioDeviceStream(this->audioDeviceID, &this->audioSpec, &SDLAudio::AudioCallbackEntryPoint, this);
	if (!audioStream)
	{
		ErrorSystem::Get()->Add(std::format("Failed to open audio device: {}", SDL_GetError()));
		return false;
	}

	AudioData::Format format;
	format.numChannels = this->audioSpec.channels;
	format.bitsPerSample = SDL_AUDIO_BITSIZE(this->audioSpec.format);
	format.framesPerSecond = this->audioSpec.freq;

	// TODO: What about big/little endian?
	if (SDL_AUDIO_ISFLOAT(this->audioSpec.format))
		format.sampleType = AudioData::Format::FLOAT;
	else if (SDL_AUDIO_ISSIGNED(this->audioSpec.format))
		format.sampleType = AudioData::Format::SIGNED_INTEGER;
	else
		format.sampleType = AudioData::Format::UNSIGNED_INTEGER;

	this->audioStream->SetFormat(format);

	if (this->recordedAudioStream.get())
		this->recordedAudioStream->SetFormat(format);

	// This will cause our callback to start getting called.
	SDL_PauseAudioDevice(this->audioDeviceID);

	return true;
}

bool SDLAudio::Shutdown()
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

//void* userData, Uint8* buffer, int length
/*static*/ void SDLCALL SDLAudio::AudioCallbackEntryPoint(void* userData, SDL_AudioStream* stream, int additionalAmount, int totalAmount)
{
	auto player = static_cast<SDLAudio*>(userData);
	player->AudioCallback(stream, additionalAmount, totalAmount);
}

/*virtual*/ void SDLAudio::AudioCallback(SDL_AudioStream* stream, int additionalAmount, int totalAmount)
{
	this->audioBuffer.resize(additionalAmount);

	switch (this->audioDirection)
	{
		case AudioDirection::SOUND_OUT:
		{
			uint64_t numBytesRead = this->audioStream->ReadBytesFromStream(this->audioBuffer.data(), uint64_t(this->audioBuffer.size()));
			for (uint64_t i = numBytesRead; i < uint64_t(this->audioBuffer.size()); i++)
				this->audioBuffer.data()[i] = 0; //this->audioSpec.silence;

			bool succeeded = SDL_PutAudioStreamData(stream, this->audioBuffer.data(), int(this->audioBuffer.size()));

			if (this->recordedAudioStream.get())
				this->recordedAudioStream->WriteBytesToStream(this->audioBuffer.data(), uint64_t(this->audioBuffer.size()));

			break;
		}
		case AudioDirection::SOUND_IN:
		{
			bool succeeded = SDL_GetAudioStreamData(stream, this->audioBuffer.data(), int(this->audioBuffer.size()));

			uint64_t numBytesWritten = this->audioStream->WriteBytesToStream(this->audioBuffer.data(), uint64_t(this->audioBuffer.size()));
			break;
		}
	}
}