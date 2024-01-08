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

/*virtual*/ bool AudioTrackData::Process(std::string& error)
{
	return true;
}

/*virtual*/ bool AudioTrackData::BeginPlayback(std::string& error)
{
	return false;
}

/*virtual*/ bool AudioTrackData::StopPlayback(std::string& error)
{
	return false;
}

/*virtual*/ bool AudioTrackData::BeginRecording(std::string& error)
{
	return false;
}

/*virtual*/ bool AudioTrackData::StopRecording(std::string& error)
{
	return false;
}

/*virtual*/ TrackData::State AudioTrackData::GetState() const
{
	return State::HAPPY;
}