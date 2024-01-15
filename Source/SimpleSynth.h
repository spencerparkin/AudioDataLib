#pragma once

#include "MidiSynth.h"

namespace AudioDataLib
{
	class AudioSink;

	// The idea here is to impliment a musical instrument that can be
	// recognized as such, even if it doesn't sound all that great.
	// There is nothing fancy going on here like modulation, envelopes,
	// filters, harmonics, etc.  Can I just make something that plays
	// musical notes at the desired pitches for the desired ranges?
	// Useful resource: https://www.cs.cmu.edu/~music/cmsip/readings/MIDI%20tutorial%20for%20programmers.html
	class AUDIO_DATA_LIB_API SimpleSynth : public MidiSynth
	{
	public:
		SimpleSynth(const AudioData::Format& format);
		virtual ~SimpleSynth();

		virtual bool ReceiveMessage(const uint8_t* message, uint64_t messageSize, Error& error) override;
		virtual bool GenerateAudio(Error& error) override;
	};
}