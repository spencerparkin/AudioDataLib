#pragma once

#include "SynthModule.h"
#include "RecursiveFilter.h"

#define ADL_REVERB_NUM_COMB_FILTERS		4
#define ADL_REVERB_NUM_ALLPASS_FILTERS	3

namespace AudioDataLib
{
	// This is an attempt to implement the Schroeder reverberator.
	class AUDIO_DATA_LIB_API ReverbModule : public SynthModule
	{
	public:
		ReverbModule();
		virtual ~ReverbModule();

		virtual bool GenerateSound(double durationSeconds, double samplesPerSecond, WaveForm& waveForm, Error& error) override;
		virtual bool CantGiveAnymoreSound() override;

		void SetDependentModule(SynthModule* synthModule);
		SynthModule* GetDependentModule();

	private:
		SynthModule* dependentModule;

		FeedBackwardCombFilter combFilter[ADL_REVERB_NUM_COMB_FILTERS];
		AllPassFilter allPassFilter[ADL_REVERB_NUM_ALLPASS_FILTERS];

		bool noMoreSound;
	};
}