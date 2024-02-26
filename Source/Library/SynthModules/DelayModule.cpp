#include "DelayModule.h"
#include "Error.h"

using namespace AudioDataLib;

DelayModule::DelayModule() : waveFormStream(2, 0.5)
{
	this->delaySeconds = 0.0;
	this->localTimeSeconds = 0.0;
}

/*virtual*/ DelayModule::~DelayModule()
{
}

/*virtual*/ bool DelayModule::GenerateSound(double durationSeconds, double samplesPerSecond, WaveForm& waveForm, SynthModule* callingModule, Error& error)
{
	if (this->GetNumDependentModules() != 1)
	{
		error.Add("Delay module needs exactly one dependent module.");
		return false;
	}

	SynthModule* dependentModule = (*this->dependentModulesArray)[0].get();

	WaveForm dependentWaveForm;
	if (!dependentModule->GenerateSound(durationSeconds, samplesPerSecond, dependentWaveForm, this, error))
		return false;

	dependentWaveForm.PadWithSilence(durationSeconds, samplesPerSecond);

	double endTimeSeconds = this->waveFormStream.GetEndTimeSeconds();

	for (const WaveForm::Sample& sample : dependentWaveForm.GetSampleArray())
	{
		WaveForm::Sample newSample;
		newSample.amplitude = sample.amplitude;
		newSample.timeSeconds = sample.timeSeconds + endTimeSeconds;
		this->waveFormStream.AddSample(newSample);
	}

	waveForm.Clear();

	double timeSeconds = 0.0;
	double deltaTimeSeconds = 1.0 / samplesPerSecond;
	while (timeSeconds <= durationSeconds)
	{
		WaveForm::Sample delayedSample;
		delayedSample.timeSeconds = timeSeconds;
		delayedSample.amplitude = this->waveFormStream.EvaluateAt(this->localTimeSeconds - this->delaySeconds + timeSeconds);
		waveForm.AddSample(delayedSample);
		if (timeSeconds == durationSeconds)
			break;
		timeSeconds += deltaTimeSeconds;
		if (timeSeconds > durationSeconds)
			timeSeconds = durationSeconds;
	}

	this->localTimeSeconds += durationSeconds;
	return true;
}

/*virtual*/ bool DelayModule::MoreSoundAvailable()
{
	if (this->dependentModulesArray->size() == 0)
		return false;

	SynthModule* dependentModule = (*this->dependentModulesArray)[0].get();
	if (dependentModule->MoreSoundAvailable())
		return true;

	return this->waveFormStream.AnyAudibleSampleFound();
}