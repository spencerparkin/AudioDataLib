#include "RecorderSynth.h"
#include "MidiData.h"
#include "Error.h"

using namespace AudioDataLib;

RecorderSynth::RecorderSynth() : MidiSynth(false)
{
	this->midiData = nullptr;
}

/*virtual*/ RecorderSynth::~RecorderSynth()
{
}

/*virtual*/ bool RecorderSynth::ReceiveMessage(double deltaTimeSeconds, const uint8_t* message, uint64_t messageSize, Error& error)
{
	if (!this->midiData)
	{
		error.Add("No MIDI data object configured.");
		return false;
	}

	// TODO: Here we decode and put MIDI messages into a MidiData class with the appropriate time-stamps for later playback.
	return true;
}