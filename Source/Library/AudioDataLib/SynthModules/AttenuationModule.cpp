#include "AudioDataLib/SynthModules/AttenuationModule.h"
#include "AudioDataLib/ErrorSystem.h"
#include "AudioDataLib/WaveForm.h"
#include "AudioDataLib/Math/Function.h"

using namespace AudioDataLib;

AttenuationModule::AttenuationModule()
{
	this->attenuationFunction = nullptr;
	this->fallOff = false;
	this->fallOffTimeSeconds = 0.0;
}

/*virtual*/ AttenuationModule::~AttenuationModule()
{
	this->SetAttenuationFunction(nullptr);
}

/*virtual*/ bool AttenuationModule::GenerateSound(double durationSeconds, double samplesPerSecond, WaveForm& waveForm, SynthModule* callingModule)
{
	if (this->GetNumDependentModules() != 1)
	{
		ErrorSystem::Get()->Add("Attenuation module needs exactly one dependent module.");
		return false;
	}

	SynthModule* dependentModule = (*this->dependentModulesArray)[0].get();

	if (!dependentModule->GenerateSound(durationSeconds, samplesPerSecond, waveForm, this))
		return false;

	if (!this->fallOff)
		return true;

	if (!this->attenuationFunction)
	{
		ErrorSystem::Get()->Add("No attenuation function set!");
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

/*virtual*/ bool AttenuationModule::MoreSoundAvailable()
{
	if (this->GetNumDependentModules() == 0)
		return false;

	SynthModule* dependentModule = (*this->dependentModulesArray)[0].get();
	if (!dependentModule->MoreSoundAvailable())
		return false;

	return !this->fallOff || this->attenuationFunction->EvaluateAt(this->fallOffTimeSeconds) > 0.0;
}

void AttenuationModule::SetAttenuationFunction(Function* function)
{
	delete this->attenuationFunction;
	this->attenuationFunction = function;
}

void AttenuationModule::TriggerFallOff()
{
	this->fallOff = true;
}