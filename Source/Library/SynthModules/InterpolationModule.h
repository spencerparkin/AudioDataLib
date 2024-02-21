#pragma once

#include "SynthModule.h"

namespace AudioDataLib
{
	class AUDIO_DATA_LIB_API InterpolationModule : public SynthModule
	{
	public:
		InterpolationModule();
		virtual ~InterpolationModule();

		virtual bool GenerateSound(double durationSeconds, double samplesPerSecond, WaveForm* waveForm, std::vector<WaveForm>* waveFormsArray, Error& error) override;
	};
}