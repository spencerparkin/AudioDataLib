#pragma once

#include "AudioDataLib/SynthModules/SynthModule.h"
#include "AudioDataLib/WaveForm.h"

namespace AudioDataLib
{
	// This module simply passes on its signal at a constant delay.
	// This, combined with the DuplicationModule, could be employed
	// to implement, for example, the Haas Effect, turning a mono
	// signal into a convincing stereo signal.
	class AUDIO_DATA_LIB_API DelayModule : public SynthModule
	{
	public:
		DelayModule();
		virtual ~DelayModule();

		virtual bool GenerateSound(double durationSeconds, double samplesPerSecond, WaveForm& waveForm, SynthModule* callingModule, Error& error) override;
		virtual bool MoreSoundAvailable() override;

		void SetDelay(double delaySeconds) { this->delaySeconds = delaySeconds; }
		double GetDelay() const { return this->delaySeconds; }

	private:
		WaveFormStream waveFormStream;
		double delaySeconds;
		double localTimeSeconds;
	};
}