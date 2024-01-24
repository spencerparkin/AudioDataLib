#include "PitchMixerModule.h"

using namespace AudioDataLib;

PitchMixerModule::PitchMixerModule()
{
}

/*virtual*/ PitchMixerModule::~PitchMixerModule()
{
}

/*virtual*/ bool PitchMixerModule::GenerateSound(const SoundParams& soundParams, WaveForm& waveForm)
{
	return false;
}