#pragma once

#include "SynthModule.h"

namespace AudioDataLib
{
	class AUDIO_DATA_LIB_API OscillatorModule : public SynthModule
	{
	public:
		OscillatorModule();
		virtual ~OscillatorModule();

		virtual bool GenerateSound(double timeSeconds, double samplesPerSecond, WaveForm& waveForm) override;
	};
}