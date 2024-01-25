#pragma once

#include "SynthModule.h"

namespace AudioDataLib
{
	class AUDIO_DATA_LIB_API OscillatorModule : public SynthModule
	{
	public:
		OscillatorModule();
		virtual ~OscillatorModule();

		virtual bool GenerateSound(double durationSeconds, double samplesPerSecond, WaveForm& waveForm, Error& error) override;

		enum WaveType
		{
			SINE,
			SQUARE,
			SAWTOOTH
		};

		struct WaveParams
		{
			WaveType waveType;
			double frequency;	// TODO: Maybe replace this with a function over time?
			double amplitude;	// TODO: Maybe replace this with a function over time?
		};

		void SetWaveParams(const WaveParams& waveParams) { this->waveParams = waveParams; }
		const WaveParams& GetWaveParams() const { return this->waveParams; }

	protected:
		
		double lifeTimeSeconds;
		WaveParams waveParams;
	};
}