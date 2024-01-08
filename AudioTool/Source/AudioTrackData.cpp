#include "AudioTrackData.h"

using namespace AudioDataLib;

AudioTrackData::AudioTrackData()
{
	this->audioData = nullptr;
}

/*virtual*/ AudioTrackData::~AudioTrackData()
{
	AudioData::Destroy(this->audioData);
}

void AudioTrackData::SetAudioData(AudioDataLib::AudioData* audioData)
{
	if (this->audioData)
		AudioData::Destroy(this->audioData);

	this->audioData = audioData;
}

/*virtual*/ void AudioTrackData::Render(wxPaintDC& paintDC) const
{
	//...
}