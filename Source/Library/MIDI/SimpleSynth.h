#pragma once

#include "MidiSynth.h"
#include "OscillatorModule.h"

namespace AudioDataLib
{
	// The idea here is to impliment a musical instrument that can be
	// recognized as such, even if it doesn't sound interesting at all.
	// There is nothing fancy going on here like modulation, envelopes,
	// filters, harmonics, etc.  Can I just make something that plays
	// musical notes at the desired pitches for the desired ranges?
	// Useful resource: https://www.cs.cmu.edu/~music/cmsip/readings/MIDI%20tutorial%20for%20programmers.html
	class AUDIO_DATA_LIB_API SimpleSynth : public MidiSynth
	{
	public:
		SimpleSynth(bool ownsAudioStream);
		virtual ~SimpleSynth();

		virtual bool ReceiveMessage(double deltaTimeSeconds, const uint8_t* message, uint64_t messageSize, Error& error) override;
		virtual SynthModule* GetRootModule() override;

	private:
		OscillatorModule* oscilatorModule;
	};
}