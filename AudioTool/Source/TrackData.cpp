#include "TrackData.h"
#include "AudioData.h"
#include "MidiData.h"
#include "AudioTrackData.h"
#include "MidiTrackData.h"

using namespace AudioDataLib;

TrackData::TrackData()
{
	this->name = "unnamed";
}

/*virtual*/ TrackData::~TrackData()
{
}

/*static*/ TrackData* TrackData::MakeTrackDataFor(AudioDataLib::FileData* fileData)
{
	TrackData* trackData = nullptr;

	if (dynamic_cast<AudioData*>(fileData))
	{
		auto audioTrackData = new AudioTrackData();
		audioTrackData->SetAudioData(dynamic_cast<AudioData*>(fileData));
		trackData = audioTrackData;
	}
	else if (dynamic_cast<MidiData*>(fileData))
	{
		auto midiTrackData = new MidiTrackData();
		midiTrackData->SetMidiData(dynamic_cast<MidiData*>(fileData));
		return midiTrackData;
	}

	return trackData;
}