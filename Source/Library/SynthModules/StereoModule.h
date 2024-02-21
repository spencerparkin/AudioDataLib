#pragma once

#include "SynthModule.h"
#include "WaveForm.h"

namespace AudioDataLib
{
	// This module is designed to take a mono signal and from it
	// produce a stereo signal using the Hass Effect.
	class AUDIO_DATA_LIB_API StereoModule : public SynthModule
	{
	public:
		StereoModule();
		virtual ~StereoModule();

		virtual bool GenerateSound(double durationSeconds, double samplesPerSecond, WaveForm* waveForm, std::vector<WaveForm>* waveFormsArray, Error& error) override;
		virtual bool MoreSoundAvailable() override;

		void SetDelayMilliseconds(double delayMilliseconds) { this->delayMilliseconds = delayMilliseconds; }
		double GetDelayMilliseconds() const { return this->delayMilliseconds; }

	private:
		double delayMilliseconds;
		double localTimeBaseSeconds;
		WaveFormStream signalStream;
	};
}