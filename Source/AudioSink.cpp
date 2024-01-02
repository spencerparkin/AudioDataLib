#include "AudioSink.h"

using namespace AudioDataLib;

AudioSink::AudioSink()
{
	this->audioDataInArray = new std::vector<AudioData*>();
}

/*virtual*/ AudioSink::~AudioSink()
{
	for (AudioData* audioData : *this->audioDataInArray)
		delete audioData;

	delete this->audioDataInArray;
}

void AudioSink::MixAudio(int numBytes)
{
	//...
}

void AudioSink::AddAudioSource(AudioData* audioData)
{
	this->audioDataInArray->push_back(audioData);
}

AudioData& AudioSink::GetAudioOutput()
{
	return this->audioDataOut;
}