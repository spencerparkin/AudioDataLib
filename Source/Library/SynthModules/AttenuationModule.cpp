#include "AttenuationModule.h"
#include "Error.h"

using namespace AudioDataLib;

AttenuationModule::AttenuationModule()
{
	this->dependentModule = nullptr;
	this->fallOff = false;
}

/*virtual*/ AttenuationModule::~AttenuationModule()
{
	this->SetDependentModule(nullptr);
}

/*virtual*/ bool AttenuationModule::GenerateSound(double durationSeconds, double samplesPerSecond, WaveForm& waveForm, Error& error)
{
	if (!this->dependentModule)
	{
		error.Add("No dependent module set.");
		return false;
	}
	if (!this->fallOff)
		return this->dependentModule->GenerateSound(durationSeconds, samplesPerSecond, waveForm, error);

	// TODO: Write this.
	return false;
}

/*virtual*/ bool AttenuationModule::CantGiveAnymoreSound()
{
	// TODO: Once we're fully attenuating, then the answer is yes.
	return false;
}

void AttenuationModule::SetDependentModule(SynthModule* synthModule)
{
	if (this->dependentModule)
		delete this->dependentModule;

	this->dependentModule = synthModule;
}

SynthModule* AttenuationModule::GetDependentModule()
{
	return this->dependentModule;
}