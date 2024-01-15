#include "MidiSynth.h"
#include "Error.h"

using namespace AudioDataLib;

MidiSynth::MidiSynth(const AudioData::Format& format)
{
	this->audioStreamOut = new AudioStream(format);
}

/*virtual*/ MidiSynth::~MidiSynth()
{
	delete this->audioStreamOut;
}

/*virtual*/ bool MidiSynth::ReceiveMessage(const uint8_t* message, uint64_t messageSize, Error& error)
{
	error.Add("Method not overridden.");
	return false;
}