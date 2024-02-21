#include "DuplicationModule.h"

using namespace AudioDataLib;

DuplicationModule::DuplicationModule()
{
}

/*virtual*/ DuplicationModule::~DuplicationModule()
{
}

/*virtual*/ bool DuplicationModule::GenerateSound(double durationSeconds, double samplesPerSecond, WaveForm& waveForm, SynthModule* callingModule, Error& error)
{
	// TODO: Use the callingModule parameter to setup a stream for the caller from which it can draw.

	return false;
}

/*virtual*/ bool DuplicationModule::MoreSoundAvailable()
{
	return false;
}