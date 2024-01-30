#pragma once

#include "SynthModule.h"

namespace AudioDataLib
{
		class AUDIO_DATA_LIB_API AttenuationModule : public SynthModule
	{
	public:
		AttenuationModule();
		virtual ~AttenuationModule();

		virtual bool GenerateSound(double durationSeconds, double samplesPerSecond, WaveForm& waveForm, Error& error) override;
		virtual bool CantGiveAnymoreSound() override;

		void SetDependentModule(SynthModule* synthModule);
		SynthModule* GetDependentModule();

	private:
		SynthModule* dependentModule;
		bool fallOff;
	};
}