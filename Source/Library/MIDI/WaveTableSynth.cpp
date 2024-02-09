#include "WaveTableSynth.h"
#include "SoundFontData.h"

using namespace AudioDataLib;

WaveTableSynth::WaveTableSynth()
{
}

/*virtual*/ WaveTableSynth::~WaveTableSynth()
{
}

/*virtual*/ bool WaveTableSynth::ReceiveMessage(double deltaTimeSeconds, const uint8_t* message, uint64_t messageSize, Error& error)
{
	// TODO: Write this.  Not sure if I'll ever get around to it, but the idea is here.
	//       From my reading, it appears that a wave-table synth is just like a sample-based
	//       synth, but with the added feature of allowing a sound to morph from one type
	//       of instrument to another.  I can see how this could be easily done with an
	//       interpolator module and an LFO.  I'm not sure how the blending will sound, though.
	return false;
}