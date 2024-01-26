#pragma once

#include "MidiSynth.h"

namespace AudioDataLib
{
	class AUDIO_DATA_LIB_API SubtractiveSynth : public MidiSynth
	{
	public:
		SubtractiveSynth(bool ownsAudioStream);
		virtual ~SubtractiveSynth();

		virtual bool ReceiveMessage(double deltaTimeSeconds, const uint8_t* message, uint64_t messageSize, Error& error) override;
		virtual SynthModule* GetRootModule(uint16_t channel) override;
	};
}