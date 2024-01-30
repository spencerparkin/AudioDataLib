#include "AttenuationModule.h"
#include "Error.h"
#include "WaveForm.h"
#include "Function.h"

using namespace AudioDataLib;

AttenuationModule::AttenuationModule()
{
	this->dependentModule = nullptr;
	this->attenuationFunction = nullptr;
	this->fallOff = false;
	this->fallOffTimeSeconds = 0.0;
}

/*virtual*/ AttenuationModule::~AttenuationModule()
{
	this->SetDependentModule(nullptr);
	this->SetAttenuationFunction(nullptr);
}

/*virtual*/ bool AttenuationModule::GenerateSound(double durationSeconds, double samplesPerSecond, WaveForm& waveForm, Error& error)
{
	if (!this->dependentModule)
	{
		error.Add("No dependent module set.");
		return false;
	}

	if (!this->dependentModule->GenerateSound(durationSeconds, samplesPerSecond, waveForm, error))
		return false;

	if (!this->fallOff)
		return true;

	if (!this->attenuationFunction)
	{
		error.Add("No attenuation function set!");
		return false;
	}

	std::vector<WaveForm::Sample>& sampleArray = waveForm.GetSampleArray();
	for (uint32_t i = 0; i < sampleArray.size(); i++)
	{
		double attenuationFactor = this->attenuationFunction->EvaluateAt(this->fallOffTimeSeconds);
		WaveForm::Sample& sample = sampleArray[i];
		sample.amplitude *= attenuationFactor;
		if (i + 1 < sampleArray.size())
			this->fallOffTimeSeconds += sampleArray[i + 1].timeSeconds - sample.timeSeconds;
	}

	return true;
}

/*virtual*/ bool AttenuationModule::CantGiveAnymoreSound()
{
	if (!this->fallOff)
		return false;

	return this->attenuationFunction->EvaluateAt(this->fallOffTimeSeconds) <= 0.0;
}

void AttenuationModule::SetAttenuationFunction(Function* function)
{
	delete this->attenuationFunction;
	this->attenuationFunction = function;
}

void AttenuationModule::SetDependentModule(SynthModule* synthModule)
{
	delete this->dependentModule;
	this->dependentModule = synthModule;
}

SynthModule* AttenuationModule::GetDependentModule()
{
	return this->dependentModule;
}

void AttenuationModule::TriggerFallOff()
{
	this->fallOff = true;
}