#pragma once

#include "SynthModule.h"
#include "SoundFontData.h"
#include "WaveForm.h"

namespace AudioDataLib
{
	class AUDIO_DATA_LIB_API LoopedAudioModule : public SynthModule
	{
	public:
		LoopedAudioModule();
		virtual ~LoopedAudioModule();

		virtual bool GenerateSound(double durationSeconds, double samplesPerSecond, WaveForm& waveForm, Error& error) override;

		bool UseLoopedAudioData(const SoundFontData::LoopedAudioData* loopedAudioData, uint16_t channel, Error& error);

	private:
		const WaveForm* loopedWaveForm;
		double startTimeSeconds;
		double endTimeSeconds;
		double localTimeSeconds;
	};
}