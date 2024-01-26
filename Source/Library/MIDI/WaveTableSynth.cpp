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
	// TODO: I misunderstood what a wave-table synth was for quite some time.  I thought it was
	//       something that produced sounds from pre-recorded samples, but that's not the case.
	//       Anyhow, I'll just leave this file here as an idea to follow-up on later if I ever
	//       get around to understanding it better.
	return false;
}