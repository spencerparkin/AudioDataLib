#include "AudioDataLib/SynthModules/DuplicationModule.h"
#include "AudioDataLib/ErrorSystem.h"

using namespace AudioDataLib;

DuplicationModule::DuplicationModule()
{
	this->masterModule = nullptr;
}

/*virtual*/ DuplicationModule::~DuplicationModule()
{
}

/*virtual*/ bool DuplicationModule::GenerateSound(double durationSeconds, double samplesPerSecond, WaveForm& waveForm, SynthModule* callingModule)
{
	if (this->GetNumDependentModules() != 1)
	{
		ErrorSystem::Get()->Add("Duplication module needs exactly one dependent module.");
		return false;
	}

	SynthModule* dependentModule = this->dependentModulesArray[0].get();

	if (!this->masterModule)
		this->masterModule = callingModule;

	if (callingModule == this->masterModule)
		if (!dependentModule->GenerateSound(durationSeconds, samplesPerSecond, this->cachedWaveForm, this))
			return false;

	// The main assumptions we're working with here are that...
	//   1. The number of modules dependent on this module doesn't change over time,
	//   2. Each is asking in cyclical order, and...
	//   3. Each is asking for the same duration and sample-rate each time.
	// If these assumptions are not true, then not only would this complicate the module implimentation,
	// but I would be concerned about dependent modules becoming out of sync over time.
	waveForm.Copy(&this->cachedWaveForm);
	return true;
}

/*virtual*/ bool DuplicationModule::MoreSoundAvailable()
{
	if (this->dependentModulesArray.size() != 1)
		return false;

	SynthModule* dependentModule = this->dependentModulesArray[0].get();
	return dependentModule->MoreSoundAvailable();
}