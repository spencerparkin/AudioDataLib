#pragma once

#include "SynthModule.h"

namespace AudioDataLib
{
	class AUDIO_DATA_LIB_API ArticulationModule : public SynthModule
	{
	public:
		ArticulationModule();
		virtual ~ArticulationModule();

		virtual bool GenerateSound(double durationSeconds, double samplesPerSecond, WaveForm& waveForm, Error& error) override;
	};
}