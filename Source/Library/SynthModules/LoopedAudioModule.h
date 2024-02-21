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

		virtual bool GenerateSound(double durationSeconds, double samplesPerSecond, WaveForm& waveForm, SynthModule* callingModule, Error& error) override;
		virtual bool MoreSoundAvailable() override;

		bool UseNonLoopedAudioData(const AudioData* audioData, uint16_t channel, Error& error);
		bool UseLoopedAudioData(const SoundFontData::LoopedAudioData* loopedAudioData, uint16_t channel, Error& error);
		void Release();

	private:
		std::shared_ptr<WaveForm>* loopedWaveForm;
		double startTimeSeconds;
		double endTimeSeconds;
		double localTimeSeconds;
		double totalTimeSeconds;
		bool loopEnabled;
	};
}