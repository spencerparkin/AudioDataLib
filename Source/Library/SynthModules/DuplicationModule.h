#pragma once

#include "SynthModule.h"

namespace AudioDataLib
{
	// This module duplicates the signal it's given so that it can be passed
	// on to other modules.  In general, (and this may be thought of as a
	// limitation of my design), no two distinct modules can depend on a
	// single module.  However, this module is designed to overcome that
	// limitation by simply being inserted between the desired module and all
	// those other modules that we want to depend upon it.
	class AUDIO_DATA_LIB_API DuplicationModule : public SynthModule
	{
	public:
		DuplicationModule();
		virtual ~DuplicationModule();

		virtual bool GenerateSound(double durationSeconds, double samplesPerSecond, WaveForm& waveForm, SynthModule* callingModule, Error& error) override;
		virtual bool MoreSoundAvailable() override;
	};
}