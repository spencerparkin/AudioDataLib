#include "DelayModule.h"

using namespace AudioDataLib;

DelayModule::DelayModule()
{
}

/*virtual*/ DelayModule::~DelayModule()
{
}

/*virtual*/ bool DelayModule::GenerateSound(double durationSeconds, double samplesPerSecond, WaveForm& waveForm, SynthModule* callingModule, Error& error)
{
	return false;
}

/*virtual*/ bool DelayModule::MoreSoundAvailable()
{
	return false;
}