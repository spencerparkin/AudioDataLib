#pragma once

#include "SynthModule.h"

namespace AudioDataLib
{
	class SoundFontData;

	class AUDIO_DATA_LIB_API SoundFontModule : public SynthModule
	{
	public:
		SoundFontModule();
		virtual ~SoundFontModule();

		virtual bool GenerateSound(double durationSeconds, double samplesPerSecond, WaveForm& waveForm, Error& error) override;

	private:
		SoundFontData* soundFontData;
	};
}