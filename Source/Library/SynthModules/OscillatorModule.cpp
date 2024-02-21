#include "OscillatorModule.h"
#include "WaveForm.h"

using namespace AudioDataLib;

OscillatorModule::OscillatorModule()
{
	this->waveParams.waveType = WaveType::SINE;
	this->waveParams.amplitude = 0.1;
	this->waveParams.frequency = 440.0;
	this->lifeTimeSeconds = 0.0;
}

/*virtual*/ OscillatorModule::~OscillatorModule()
{
}

/*virtual*/ bool OscillatorModule::GenerateSound(double durationSeconds, double samplesPerSecond, WaveForm& waveForm, SynthModule* callingModule, Error& error)
{
	waveForm.Clear();

	uint64_t numSamples = ::floor(durationSeconds * samplesPerSecond);

	switch (this->waveParams.waveType)
	{
		case WaveType::SINE:
		{
			for (uint64_t i = 0; i < numSamples; i++)
			{
				WaveForm::Sample sample;
				sample.timeSeconds = (double(i) / double(numSamples)) * durationSeconds;
				sample.amplitude = this->waveParams.amplitude * ::sin(2.0 * ADL_PI * (this->lifeTimeSeconds + sample.timeSeconds) * this->waveParams.frequency);
				waveForm.AddSample(sample);
			}
			break;
		}
		case WaveType::SQUARE:
		{
			for (uint64_t i = 0; i < numSamples; i++)
			{
				WaveForm::Sample sample;
				sample.timeSeconds = (double(i) / double(numSamples)) * durationSeconds;
				sample.amplitude = ::sin(2.0 * ADL_PI * (this->lifeTimeSeconds + sample.timeSeconds) * this->waveParams.frequency);
				if (sample.amplitude >= 0.0)
					sample.amplitude = this->waveParams.amplitude;
				else
					sample.amplitude = -this->waveParams.amplitude;
				waveForm.AddSample(sample);
			}
			break;
		}
		case WaveType::SAWTOOTH:
		{
			for (uint64_t i = 0; i < numSamples; i++)
			{
				WaveForm::Sample sample;
				sample.timeSeconds = (double(i) / double(numSamples)) * durationSeconds;
				double angle = ::fmod(2.0 * ADL_PI * (this->lifeTimeSeconds + sample.timeSeconds) * this->waveParams.frequency, 2.0 * ADL_PI);
				sample.amplitude = (1.0 - angle / ADL_PI) * this->waveParams.amplitude;
				waveForm.AddSample(sample);
			}
			break;
		}
	}

	this->lifeTimeSeconds += durationSeconds;

	return true;
}