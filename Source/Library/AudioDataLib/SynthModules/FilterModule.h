#pragma once

#include "AudioDataLib/SynthModules/SynthModule.h"

namespace AudioDataLib
{
	// This module does almost the opposite of the mixer module.  It will transform
	// the wave-form of the dependent module into the frequency domain, shave off
	// some of the frequencies according to how it's configured, and then transform
	// it back into the time-domain to be returned to the caller.  This is a tall
	// order, and so I'll only get around to implimenting this if I feel like everything
	// else I've tried to do seems to be working well-enough.
	class AUDIO_DATA_LIB_API FilterModule : public SynthModule
	{
	public:
		FilterModule();
		virtual ~FilterModule();

		virtual bool GenerateSound(double durationSeconds, double samplesPerSecond, WaveForm& waveForm, SynthModule* callingModule, Error& error) override;
	};
}