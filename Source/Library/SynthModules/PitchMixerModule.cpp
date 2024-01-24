#include "PitchMixerModule.h"

using namespace AudioDataLib;

PitchMixerModule::PitchMixerModule()
{
}

/*virtual*/ PitchMixerModule::~PitchMixerModule()
{
}

/*virtual*/ bool PitchMixerModule::GenerateSound(double timeSeconds, double samplesPerSecond, WaveForm& waveForm)
{
	return false;
}