#include "PitchShiftModule.h"
#include "Error.h"
#include "WaveForm.h"

using namespace AudioDataLib;

PitchShiftModule::PitchShiftModule()
{
	this->sourceFrequency = 0.0;
	this->targetFrequency = 0.0;
}

/*virtual*/ PitchShiftModule::~PitchShiftModule()
{
}

/*virtual*/ bool PitchShiftModule::GenerateSound(double durationSeconds, double samplesPerSecond, WaveForm& waveForm, SynthModule* callingModule, Error& error)
{
	if (this->GetNumDependentModules() != 1)
	{
		error.Add("Pitch shift module needs exactly one dependent module.");
		return false;
	}

	SynthModule* dependentModule = (*this->dependentModulesArray)[0].get();

	if (this->targetFrequency == this->sourceFrequency)
	{
		// We're just a no-op/pass-through in this case.
		return dependentModule->GenerateSound(durationSeconds, samplesPerSecond, waveForm, this, error);
	}
	
	if (this->sourceFrequency == 0.0)
	{
		error.Add("Source frequency of zero encountered!");
		return false;
	}

	double neededTimeSeconds = (this->targetFrequency / this->sourceFrequency) * durationSeconds;

	if (::isinf(neededTimeSeconds) || ::isnan(neededTimeSeconds))
	{
		error.Add("Needed time calculation results in inf/nan.");
		return false;
	}

	if (!dependentModule->GenerateSound(neededTimeSeconds, samplesPerSecond, waveForm, this, error))
		return false;

	double startTimeSeconds = waveForm.GetStartTime();
	double endTimeSeconds = waveForm.GetEndTime();

	for (WaveForm::Sample& sample : waveForm.GetSampleArray())
	{
		double alpha = (sample.timeSeconds - startTimeSeconds) / (endTimeSeconds - startTimeSeconds);
		sample.timeSeconds = alpha * durationSeconds;
	}

	return true;
}

/*virtual*/ bool PitchShiftModule::MoreSoundAvailable()
{
	if (this->dependentModulesArray->size() == 0)
		return false;

	SynthModule* dependentModule = (*this->dependentModulesArray)[0].get();
	return dependentModule->MoreSoundAvailable();
}

void PitchShiftModule::SetSourceAndTargetFrequencies(double sourceFrequency, double targetFrequency)
{
	this->sourceFrequency = sourceFrequency;
	this->targetFrequency = targetFrequency;
}