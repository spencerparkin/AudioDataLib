#pragma once

#include "SynthModule.h"

namespace AudioDataLib
{
	class AUDIO_DATA_LIB_API PitchMixerModule : public SynthModule
	{
	public:
		PitchMixerModule();
		virtual ~PitchMixerModule();

		virtual bool GenerateSound(double timeSeconds, double samplesPerSecond, WaveForm& waveForm) override;
	};
}