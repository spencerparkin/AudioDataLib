#include "WaveTableSynth.h"
#include "SoundFontData.h"

using namespace AudioDataLib;

WaveTableSynth::WaveTableSynth(bool ownsAudioStream, bool ownsSoundFontData) : MidiSynth(ownsAudioStream)
{
}

/*virtual*/ WaveTableSynth::~WaveTableSynth()
{
}

/*virtual*/ bool WaveTableSynth::ReceiveMessage(double deltaTimeSeconds, const uint8_t* message, uint64_t messageSize, Error& error)
{
	// TODO: Write this.  Not sure if I'll ever get around to it, but the idea is here.
	return false;
}