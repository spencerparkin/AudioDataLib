#include "PitchShiftModule.h"
#include "Error.h"
#include "WaveForm.h"

using namespace AudioDataLib;

PitchShiftModule::PitchShiftModule()
{
	this->sourceFrequency = 0.0;
	this->targetFrequency = 0.0;
	this->dependentModule = nullptr;
}

/*virtual*/ PitchShiftModule::~PitchShiftModule()
{
}

/*virtual*/ bool PitchShiftModule::GenerateSound(double durationSeconds, double samplesPerSecond, WaveForm& waveForm, Error& error)
{
	if (!this->dependentModule)
	{
		error.Add("No dependent module set!");
		return false;
	}

	if (this->targetFrequency == this->sourceFrequency)
	{
		// We're just a no-op/pass-through in this case.
		return this->dependentModule->GenerateSound(durationSeconds, samplesPerSecond, waveForm, error);
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

	if (!this->dependentModule->GenerateSound(neededTimeSeconds, samplesPerSecond, waveForm, error))
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

void PitchShiftModule::SetSourceAndTargetFrequencies(double sourceFrequency, double targetFrequency)
{
	this->sourceFrequency = sourceFrequency;
	this->targetFrequency = targetFrequency;
}