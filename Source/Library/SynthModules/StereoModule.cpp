#include "StereoModule.h"
#include "Error.h"

using namespace AudioDataLib;

StereoModule::StereoModule() : signalStream(2, 0.5)
{
	this->delayMilliseconds = 0.0;
	this->localTimeBaseSeconds = 0.0;
}

/*virtual*/ StereoModule::~StereoModule()
{
}

/*virtual*/ bool StereoModule::GenerateSound(double durationSeconds, double samplesPerSecond, WaveForm* waveForm, std::vector<WaveForm>* waveFormsArray, Error& error)
{
	if (this->GetNumDependentModules() != 1)
	{
		error.Add("Stereo module needs exactly one dependent module.");
		return false;
	}

	if (waveForm || !waveFormsArray)
	{
		error.Add("Stereo module is meant to receive a pointer to an array of wave-forms and only such a pointer.");
		return false;
	}

	if (this->delayMilliseconds < 0.0)
	{
		error.Add(FormatString("Given delay (%f) is negative, but should be non-negative.", this->delayMilliseconds));
		return false;
	}

	SynthModule* dependentModule = (*this->dependentModulesArray)[0].get();

	waveFormsArray->resize(2);
	if (!dependentModule->GenerateSound(durationSeconds, samplesPerSecond, &(*waveFormsArray)[0], nullptr, error))
		return false;

	double padTimeSeconds = durationSeconds - (*waveFormsArray)[0].GetTimespan();
	if (padTimeSeconds > 0.0)
		(*waveFormsArray)[0].PadWithSilence(padTimeSeconds, samplesPerSecond);

	for (const WaveForm::Sample& sample : (*waveFormsArray)[0].GetSampleArray())
		this->signalStream.AddSample(WaveForm::Sample{ sample.amplitude, sample.timeSeconds + this->localTimeBaseSeconds });
	
	double delaySeconds = this->delayMilliseconds / 1000.0;
	double timeSeconds = this->localTimeBaseSeconds - delaySeconds;
	double deltaTimeSeconds = 1.0 / samplesPerSecond;
	WaveForm::Sample sample;
	sample.timeSeconds = 0.0;
	for (uint64_t i = 0; i < (*waveFormsArray)[0].GetSampleArray().size(); i++)
	{
		sample.amplitude = this->signalStream.EvaluateAt(timeSeconds);
		(*waveFormsArray)[1].AddSample(sample);
		timeSeconds += deltaTimeSeconds;
		sample.timeSeconds += deltaTimeSeconds;
	}

	this->localTimeBaseSeconds += (*waveFormsArray)[0].GetTimespan();

	return true;
}

/*virtual*/ bool StereoModule::MoreSoundAvailable()
{
	if (this->dependentModulesArray->size() == 0)
		return false;

	SynthModule* dependentModule = (*this->dependentModulesArray)[0].get();
	if (dependentModule->MoreSoundAvailable())
		return true;

	// TODO: This is not quite right.  We really need to say yes here until our signal stream is all silence.
	return false;
}