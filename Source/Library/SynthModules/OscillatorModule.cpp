#include "OscillatorModule.h"
#include "WaveForm.h"

using namespace AudioDataLib;

OscillatorModule::OscillatorModule()
{
	this->waveType = WaveType::SINE;
	this->localTimeSeconds = 0.0;
}

/*virtual*/ OscillatorModule::~OscillatorModule()
{
}

/*virtual*/ bool OscillatorModule::GenerateSound(const SoundParams& soundParams, WaveForm& waveForm)
{
	waveForm.Clear();

	uint64_t numSamples = ::round(soundParams.durationSeconds * soundParams.samplesPerSecond);

	switch (this->waveType)
	{
		case WaveType::SINE:
		{
			for (uint64_t i = 0; i < numSamples; i++)
			{
				WaveForm::Sample sample;
				sample.timeSeconds = (double(i) / double(numSamples - 1)) * soundParams.durationSeconds;
				sample.amplitude = soundParams.generalAmplitude * ::sin(2.0 * ADL_PI * (this->localTimeSeconds + sample.timeSeconds) * soundParams.generalFrequency);
				waveForm.AddSample(sample);
			}
			break;
		}
		case WaveType::SQUARE:
		{
			for (uint64_t i = 0; i < numSamples; i++)
			{
				WaveForm::Sample sample;
				sample.timeSeconds = (double(i) / double(numSamples - 1)) * soundParams.durationSeconds;
				sample.amplitude = ::sin(2.0 * ADL_PI * (this->localTimeSeconds + sample.timeSeconds) * soundParams.generalFrequency);
				if (sample.amplitude >= 0.0)
					sample.amplitude = soundParams.generalAmplitude;
				else
					sample.amplitude = -soundParams.generalAmplitude;
				waveForm.AddSample(sample);
			}
			break;
		}
		case WaveType::SAWTOOTH:
		{
			for (uint64_t i = 0; i < numSamples; i++)
			{
				WaveForm::Sample sample;
				sample.timeSeconds = (double(i) / double(numSamples - 1)) * soundParams.durationSeconds;
				double angle = ::fmod(2.0 * ADL_PI * (this->localTimeSeconds + sample.timeSeconds) * soundParams.generalFrequency, 2.0 * ADL_PI);
				sample.amplitude = (1.0 - angle / ADL_PI) * soundParams.generalAmplitude;
				waveForm.AddSample(sample);
			}
			break;
		}
	}

	this->localTimeSeconds += soundParams.durationSeconds;

	return true;
}