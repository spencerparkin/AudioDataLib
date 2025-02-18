#pragma once

#include "AudioDataLib/MIDI/MidiSynth.h"
#include "AudioDataLib/SynthModules/MixerModule.h"

namespace AudioDataLib
{
	/**
	 * The idea here is to impliment a musical instrument that can be
	 * recognized as such, even if it doesn't sound interesting at all.
	 * There is nothing fancy going on here like modulation, envelopes,
	 * filters, harmonics, etc.  Can I just make something that plays
	 * musical notes at the desired pitches for the desired ranges?
	 */
	class AUDIO_DATA_LIB_API SimpleSynth : public MidiSynth
	{
		// Useful resource : https://www.cs.cmu.edu/~music/cmsip/readings/MIDI%20tutorial%20for%20programmers.html
	public:
		SimpleSynth();
		virtual ~SimpleSynth();

		virtual bool ReceiveMessage(double deltaTimeSeconds, const uint8_t* message, uint64_t messageSize) override;
		virtual SynthModule* GetRootModule(uint16_t channel) override;

	private:
		MixerModule* mixerModule;

		typedef std::map<uint8_t, uint64_t> NoteMap;
		NoteMap* noteMap;
	};
}