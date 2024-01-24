#pragma once

#include "SynthModule.h"

namespace AudioDataLib
{
	class AUDIO_DATA_LIB_API PitchMixerModule : public SynthModule
	{
	public:
		PitchMixerModule();
		virtual ~PitchMixerModule();

		virtual bool GenerateSound(const SoundParams& soundParams, WaveForm& waveForm) override;
	};
}