#pragma once

#include "AudioDataLib/SynthModules/SynthModule.h"

namespace AudioDataLib
{
	class AUDIO_DATA_LIB_API ArticulationModule : public SynthModule
	{
	public:
		ArticulationModule();
		virtual ~ArticulationModule();

		virtual bool GenerateSound(double durationSeconds, double samplesPerSecond, WaveForm& waveForm, SynthModule* callingModule, Error& error) override;
	};
}