#include "ArticulationModule.h"

using namespace AudioDataLib;

ArticulationModule::ArticulationModule()
{
}

/*virtual*/ ArticulationModule::~ArticulationModule()
{
}

/*virtual*/ bool ArticulationModule::GenerateSound(double durationSeconds, double samplesPerSecond, WaveForm& waveForm, Error& error)
{
	// TODO: Here we run the dependent module's wave-form through an LFO to modulate volume.  (Tromolo.)
	//       Can a similar thing be done to modulate pitch by up- or down-sampling the dependent module's wave-form?  (Vibrato.)
	//       The up- or down-sampling is not uniform, it oscillates back and forth over time so that the pitch oscillates.
	//       This would not be desirable with pre-recorded sound that already has vibrato baked into it.
	return false;
}