#include "SimpleSynth.h"

using namespace AudioDataLib;

SimpleSynth::SimpleSynth(const AudioData::Format& format) : MidiSynth(format)
{
}

/*virtual*/ SimpleSynth::~SimpleSynth()
{
}

/*virtual*/ bool SimpleSynth::ReceiveMessage(const uint8_t* message, uint64_t messageSize, Error& error)
{
	return false;
}