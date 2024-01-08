#include "MidiRecorder.h"

using namespace AudioDataLib;

MidiRecorder::MidiRecorder()
{
	this->midiData = nullptr;
}

/*virtual*/ MidiRecorder::~MidiRecorder()
{
}

bool MidiRecorder::ReceiveMessage(const uint8_t* message, uint64_t messageSize, std::string& error)
{
	if (!this->midiData)
	{
		error = "No MIDI data object set in which we can store MIDI messages.";
		return false;
	}

	// TODO: Decode the given MIDI message and then store it appropriately in our MIDI data object.

	error = "No yet written.";
	return false;
}