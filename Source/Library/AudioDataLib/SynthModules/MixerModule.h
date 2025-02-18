#pragma once

#include "AudioDataLib/SynthModules/SynthModule.h"

namespace AudioDataLib
{
	class AUDIO_DATA_LIB_API MixerModule : public SynthModule
	{
	public:
		MixerModule();
		virtual ~MixerModule();

		virtual bool GenerateSound(double durationSeconds, double samplesPerSecond, WaveForm& waveForm, SynthModule* callingModule, Error& error) override;
	};
}