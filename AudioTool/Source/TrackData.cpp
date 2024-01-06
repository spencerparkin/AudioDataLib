#include "TrackData.h"
#include "AudioData.h"

using namespace AudioDataLib;

TrackData::TrackData()
{
	this->audioData = nullptr;
}

/*virtual*/ TrackData::~TrackData()
{
	AudioData::Destroy(this->audioData);
}

void TrackData::SetAudioData(AudioDataLib::AudioData* audioData)
{
	if (this->audioData)
		AudioData::Destroy(this->audioData);
	
	this->audioData = audioData;
}