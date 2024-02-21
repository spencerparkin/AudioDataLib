#pragma once

#include "SynthModule.h"

namespace AudioDataLib
{
	class Function;

	class AUDIO_DATA_LIB_API AttenuationModule : public SynthModule
	{
	public:
		AttenuationModule();
		virtual ~AttenuationModule();

		virtual bool GenerateSound(double durationSeconds, double samplesPerSecond, WaveForm& waveForm, SynthModule* callingModule, Error& error) override;
		virtual bool MoreSoundAvailable() override;

		void SetAttenuationFunction(Function* function);
		Function* GetAttenuationFunction() { return this->attenuationFunction; }

		void TriggerFallOff();

	private:
		bool fallOff;
		Function* attenuationFunction;
		double fallOffTimeSeconds;
	};
}