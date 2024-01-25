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