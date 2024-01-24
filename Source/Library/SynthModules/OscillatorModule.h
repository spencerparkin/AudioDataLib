#pragma once

#include "SynthModule.h"

namespace AudioDataLib
{
	class AUDIO_DATA_LIB_API OscillatorModule : public SynthModule
	{
	public:
		OscillatorModule();
		virtual ~OscillatorModule();

		virtual bool GenerateSound(const SoundParams& soundParams, WaveForm& waveForm) override;

		enum WaveType
		{
			SINE,
			SQUARE,
			SAWTOOTH
		};

	protected:

		WaveType waveType;
		double localTimeSeconds;
	};
}