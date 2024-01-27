#include "FilterModule.h"

using namespace AudioDataLib;

FilterModule::FilterModule()
{
	this->dependentModule = nullptr;
}

/*virtual*/ FilterModule::~FilterModule()
{
	this->SetDependentModule(nullptr);
}

/*virtual*/ bool FilterModule::GenerateSound(double durationSeconds, double samplesPerSecond, WaveForm& waveForm, Error& error)
{
	// TODO: Here we apply an FFT to a dependent module's sound, shave off (or filter) frequencies as configured, then
	//       apply an inverse FFT to produce sound for the caller.
	return false;
}

void FilterModule::SetDependentModule(SynthModule* synthModule)
{
	if (this->dependentModule)
		delete this->dependentModule;

	this->dependentModule = synthModule;
}

SynthModule* FilterModule::GetDependentModule()
{
	return this->dependentModule;
}