#pragma once

#include "SynthModule.h"
#include "SoundFontData.h"

namespace AudioDataLib
{
	class AUDIO_DATA_LIB_API LoopedAudioModule : public SynthModule
	{
	public:
		LoopedAudioModule();
		virtual ~LoopedAudioModule();

		virtual bool GenerateSound(double durationSeconds, double samplesPerSecond, WaveForm& waveForm, Error& error) override;

		SoundFontData::LoopedAudioData* loopedAudioData;
	};
}