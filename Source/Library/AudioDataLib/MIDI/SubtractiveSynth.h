#pragma once

#include "AudioDataLib/MIDI/MidiSynth.h"

namespace AudioDataLib
{
	class AUDIO_DATA_LIB_API SubtractiveSynth : public MidiSynth
	{
	public:
		SubtractiveSynth();
		virtual ~SubtractiveSynth();

		virtual bool ReceiveMessage(double deltaTimeSeconds, const uint8_t* message, uint64_t messageSize, Error& error) override;
		virtual SynthModule* GetRootModule(uint16_t channel) override;
	};
}