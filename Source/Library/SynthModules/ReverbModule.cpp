#include "ReverbModule.h"
#include "WaveForm.h"
#include "Error.h"

using namespace AudioDataLib;

ReverbModule::ReverbModule()
{
	this->dependentModule = nullptr;
	this->noMoreSound = false;

	// See: https://www.dsprelated.com/freebooks/pasp/Schroeder_Reverberators.html

	this->combFilter[0].SetParams(RecursiveFilter::Params{ 29.7 / 1000.0, 0.805 });
	this->combFilter[1].SetParams(RecursiveFilter::Params{ 37.1 / 1000.0, 0.827 });
	this->combFilter[2].SetParams(RecursiveFilter::Params{ 41.1 / 1000.0, 0.783 });
	this->combFilter[3].SetParams(RecursiveFilter::Params{ 43.7 / 1000.0, 0.764 });

	this->allPassFilter[0].SetParams(RecursiveFilter::Params{ 5.0 / 1000.0, 0.7 });
	this->allPassFilter[1].SetParams(RecursiveFilter::Params{ 1.7 / 1000.0, 0.7 });
	this->allPassFilter[2].SetParams(RecursiveFilter::Params{ 2.3 / 1000.0, 0.7 });
}

/*virtual*/ ReverbModule::~ReverbModule()
{
	delete this->dependentModule;
}

void ReverbModule::SetDependentModule(SynthModule* synthModule)
{
	delete this->dependentModule;
	this->dependentModule = synthModule;
}

SynthModule* ReverbModule::GetDependentModule()
{
	return this->dependentModule;
}

/*virtual*/ bool ReverbModule::GenerateSound(double durationSeconds, double samplesPerSecond, WaveForm& waveForm, Error& error)
{
	if (!this->dependentModule)
	{
		error.Add("No dependent module set.");
		return false;
	}

	WaveForm originalWaveForm;
	if (!this->dependentModule->GenerateSound(durationSeconds, samplesPerSecond, originalWaveForm, error))
		return false;

	waveForm.Clear();

	for (const WaveForm::Sample& sample : originalWaveForm.GetSampleArray())
	{
		WaveForm::Sample reverbSample;
		reverbSample.timeSeconds = sample.timeSeconds;
		reverbSample.amplitude = 0.0;

		for (uint32_t i = 0; i < ADL_REVERB_NUM_COMB_FILTERS; i++)
		{
			this->combFilter[i].AddSample(sample);
			reverbSample.amplitude += this->combFilter[i].EvaluateAt(sample.timeSeconds);
		}

		reverbSample.amplitude /= double(ADL_REVERB_NUM_COMB_FILTERS);	// TODO: Do I need this?

		for (uint32_t i = 0; i < ADL_REVERB_NUM_ALLPASS_FILTERS; i++)
		{
			this->allPassFilter[i].AddSample(reverbSample);
			reverbSample.amplitude = this->allPassFilter[i].EvaluateAt(reverbSample.timeSeconds);
		}

		waveForm.AddSample(reverbSample);
	}

	double averageVolume = waveForm.CalcAverageVolume();
	constexpr double threshold = 1e-7;
	if (averageVolume < threshold)
		this->noMoreSound = true;
	else
		this->noMoreSound = false;

	return true;
}

/*virtual*/ bool ReverbModule::CantGiveAnymoreSound()
{
	return this->noMoreSound;
}