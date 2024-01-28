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
		WaveForm* loopedWaveForm;
		double startTimeSeconds;
		double endTimeSeconds;
		double localTimeSeconds;
		// NOTE: Some are supposed to fade out, and then we return true for CantGiveAnymoreAudio(); e.g., a piano key.
		//       Others, like an organ or the basoon, can play indefinitely.
	};
}