#include "AudioDataLib/SynthModules/FilterModule.h"
#include "AudioDataLib/Error.h"

using namespace AudioDataLib;

FilterModule::FilterModule()
{
}

/*virtual*/ FilterModule::~FilterModule()
{
}

/*virtual*/ bool FilterModule::GenerateSound(double durationSeconds, double samplesPerSecond, WaveForm& waveForm, SynthModule* callingModule, Error& error)
{
	// TODO: Here we apply an FFT to a dependent module's sound, shave off (or filter) frequencies as configured, then
	//       apply an inverse FFT to produce sound for the caller.
	error.Add("Not yet implemented.");
	return false;
}