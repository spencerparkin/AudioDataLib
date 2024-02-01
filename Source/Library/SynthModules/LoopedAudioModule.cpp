#include "LoopedAudioModule.h"
#include "WaveForm.h"
#include "Error.h"

using namespace AudioDataLib;

LoopedAudioModule::LoopedAudioModule()
{
	this->loopedWaveForm = nullptr;
	this->startTimeSeconds = 0.0;
	this->endTimeSeconds = 0.0;
	this->localTimeSeconds = 0.0;
}

/*virtual*/ LoopedAudioModule::~LoopedAudioModule()
{
}

/*virtual*/ bool LoopedAudioModule::GenerateSound(double durationSeconds, double samplesPerSecond, WaveForm& waveForm, Error& error)
{
	if (!this->loopedWaveForm)
	{
		error.Add("No looped wave-form we can use to generate audio.");
		return false;
	}

	waveForm.Clear();

	double generatedSoundTimeSeconds = 0.0;
	double deltaTimeSeconds = 1.0 / samplesPerSecond;

	while (generatedSoundTimeSeconds <= durationSeconds)
	{
		WaveForm::Sample sample;
		sample.timeSeconds = generatedSoundTimeSeconds;
		sample.amplitude = this->loopedWaveForm->EvaluateAt(this->localTimeSeconds);
		waveForm.AddSample(sample);

		if (generatedSoundTimeSeconds == durationSeconds)
			break;

		if (generatedSoundTimeSeconds + deltaTimeSeconds > durationSeconds)
		{
			deltaTimeSeconds = durationSeconds - generatedSoundTimeSeconds;
			generatedSoundTimeSeconds = durationSeconds;
		}
		else
		{
			generatedSoundTimeSeconds += deltaTimeSeconds;
		}

		this->localTimeSeconds += deltaTimeSeconds;

		if (this->localTimeSeconds > this->endTimeSeconds)
			this->localTimeSeconds -= this->endTimeSeconds - this->startTimeSeconds;
	}
	
	return true;
}

bool LoopedAudioModule::UseLoopedAudioData(const SoundFontData::LoopedAudioData* loopedAudioData, uint16_t channel, Error& error)
{
	this->loopedWaveForm = loopedAudioData->GetCachedWaveForm(channel, error);
	if (!this->loopedWaveForm)
		return false;

	const AudioData::Format& format = loopedAudioData->GetFormat();

	double totalTimeSeconds = loopedAudioData->GetTimeSeconds();

	this->startTimeSeconds = format.BytesPerChannelToSeconds(loopedAudioData->GetLoop().startFrame * format.BytesPerFrame());
	this->endTimeSeconds = format.BytesPerChannelToSeconds(loopedAudioData->GetLoop().endFrame * format.BytesPerFrame());

	if (this->startTimeSeconds >= this->endTimeSeconds || this->startTimeSeconds < 0.0 || this->endTimeSeconds < 0.0)
	{
		error.Add(FormatString("Start time (%f) and end time (%f) don't make sense.", this->startTimeSeconds, this->endTimeSeconds));
		return false;
	}

	if (this->endTimeSeconds > totalTimeSeconds)
	{
		error.Add(FormatString("End time (%f) is greater than total time (%f) in looped audio sample.", this->endTimeSeconds, totalTimeSeconds));
		return false;
	}

	this->localTimeSeconds = 0.0;

	return true;
}