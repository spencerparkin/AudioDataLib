#include "AudioDataLib/SynthModules/ReverbModule.h"
#include "AudioDataLib/WaveForm.h"
#include "AudioDataLib/ErrorSystem.h"

using namespace AudioDataLib;

ReverbModule::ReverbModule(uint8_t variation)
{
	this->enabled = true;
	this->moreSoundAvailable = true;
	this->localTimeBaseSeconds = 0.0;

	// See: https://www.dsprelated.com/freebooks/pasp/Schroeder_Reverberators.html

	// TODO: Maybe these delays can afford to be a bit longer (to increase the size of the room),
	//       if we also make sure they're not too close to being multiples of one another?
	//       The Schroeder guy said something about coprime-ness.

	switch (variation)
	{
	case 0:
		this->combFilter[0].SetParams(RecursiveFilter::Params{ 37.0 / 1000.0, 0.805 });
		this->combFilter[1].SetParams(RecursiveFilter::Params{ 23.0 / 1000.0, 0.827 });
		this->combFilter[2].SetParams(RecursiveFilter::Params{ 29.0 / 1000.0, 0.783 });
		this->combFilter[3].SetParams(RecursiveFilter::Params{ 47.0 / 1000.0, 0.764 });
		break;
	case 1:
		this->combFilter[0].SetParams(RecursiveFilter::Params{ 19.0 / 1000.0, 0.805 });
		this->combFilter[1].SetParams(RecursiveFilter::Params{ 41.0 / 1000.0, 0.827 });
		this->combFilter[2].SetParams(RecursiveFilter::Params{ 43.0 / 1000.0, 0.783 });
		this->combFilter[3].SetParams(RecursiveFilter::Params{ 31.0 / 1000.0, 0.764 });
		break;
	}

	this->allPassFilter[0].SetParams(RecursiveFilter::Params{ 5.0 / 1000.0, 0.7 });
	this->allPassFilter[1].SetParams(RecursiveFilter::Params{ 1.7 / 1000.0, 0.7 });
	this->allPassFilter[2].SetParams(RecursiveFilter::Params{ 2.3 / 1000.0, 0.7 });
}

/*virtual*/ ReverbModule::~ReverbModule()
{
}

/*virtual*/ bool ReverbModule::GenerateSound(double durationSeconds, double samplesPerSecond, WaveForm& waveForm, SynthModule* callingModule)
{
	if (this->GetNumDependentModules() != 1)
	{
		ErrorSystem::Get()->Add("Reverb module needs exactly one dependent module.");
		return false;
	}

	SynthModule* dependentModule = (*this->dependentModulesArray)[0].get();

	if (!this->enabled)
	{
		if (!dependentModule->GenerateSound(durationSeconds, samplesPerSecond, waveForm, this))
			return false;

		this->moreSoundAvailable = dependentModule->MoreSoundAvailable();
		return true;
	}

	WaveForm originalWaveForm;
	if (!dependentModule->GenerateSound(durationSeconds, samplesPerSecond, originalWaveForm, this))
		return false;

	originalWaveForm.PadWithSilence(durationSeconds, samplesPerSecond);

	waveForm.Clear();

	for (const WaveForm::Sample& sample : originalWaveForm.GetSampleArray())
	{
		WaveForm::Sample originalSample;
		originalSample.amplitude = sample.amplitude;
		originalSample.timeSeconds = sample.timeSeconds + this->localTimeBaseSeconds;

		WaveForm::Sample reverbSample;
		reverbSample.amplitude = 0.0;
		reverbSample.timeSeconds = originalSample.timeSeconds;

		for (uint32_t i = 0; i < ADL_REVERB_NUM_COMB_FILTERS; i++)
		{
			this->combFilter[i].AddSample(originalSample);
			reverbSample.amplitude += this->combFilter[i].EvaluateAt(originalSample.timeSeconds);
		}

		reverbSample.amplitude /= double(ADL_REVERB_NUM_COMB_FILTERS);

		for (uint32_t i = 0; i < ADL_REVERB_NUM_ALLPASS_FILTERS; i++)
		{
			this->allPassFilter[i].AddSample(reverbSample);
			reverbSample.amplitude = this->allPassFilter[i].EvaluateAt(reverbSample.timeSeconds);
		}

		reverbSample.timeSeconds -= this->localTimeBaseSeconds;
		waveForm.AddSample(reverbSample);
	}

	this->localTimeBaseSeconds += originalWaveForm.GetTimespan();

	if (!dependentModule->MoreSoundAvailable())
	{
		// Wait for the reverberation to taper off.
		double averageVolume = waveForm.CalcAverageVolume();
		constexpr double threshold = 1e-3;
		if (averageVolume < threshold)
			this->moreSoundAvailable = false;
		else
			this->moreSoundAvailable = true;
	}

	return true;
}

/*virtual*/ bool ReverbModule::MoreSoundAvailable()
{
	return this->moreSoundAvailable;
}