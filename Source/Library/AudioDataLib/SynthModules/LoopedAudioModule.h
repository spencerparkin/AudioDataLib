#pragma once

#include "AudioDataLib/SynthModules/SynthModule.h"
#include "AudioDataLib/FileDatas/WaveTableData.h"
#include "AudioDataLib/WaveForm.h"

namespace AudioDataLib
{
	class AUDIO_DATA_LIB_API LoopedAudioModule : public SynthModule
	{
	public:
		LoopedAudioModule();
		virtual ~LoopedAudioModule();

		virtual bool GenerateSound(double durationSeconds, double samplesPerSecond, WaveForm& waveForm, SynthModule* callingModule) override;
		virtual bool MoreSoundAvailable() override;

		bool UseNonLoopedAudioData(const AudioData* audioData, uint16_t channel);
		bool UseLoopedAudioData(const WaveTableData::AudioSampleData* audioSampleData, uint16_t channel);
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