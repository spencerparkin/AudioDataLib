#include "AudioDataLib/MIDI/SubtractiveSynth.h"

using namespace AudioDataLib;

SubtractiveSynth::SubtractiveSynth()
{
}

/*virtual*/ SubtractiveSynth::~SubtractiveSynth()
{
}

/*virtual*/ bool SubtractiveSynth::ReceiveMessage(double deltaTimeSeconds, const uint8_t* message, uint64_t messageSize)
{
	// TODO: Here we're going to try to use a FilterModule to perform subtractive synthesis.
	return false;
}

/*virtual*/ SynthModule* SubtractiveSynth::GetRootModule(uint16_t channel)
{
	return nullptr;
}