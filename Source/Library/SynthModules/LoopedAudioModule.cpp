#include "LoopedAudioModule.h"
#include "WaveForm.h"
#include "Error.h"

using namespace AudioDataLib;

LoopedAudioModule::LoopedAudioModule()
{
	this->loopedWaveForm = nullptr;
	this->ownsWaveForm = false;
	this->startTimeSeconds = 0.0;
	this->endTimeSeconds = 0.0;
	this->localTimeSeconds = 0.0;
	this->totalTimeSeconds = 0.0;
	this->loopEnabled = true;
}

/*virtual*/ LoopedAudioModule::~LoopedAudioModule()
{
	if (this->ownsWaveForm)
		delete this->loopedWaveForm;	// TODO: Start using shared pointers.  The DLL export crap is preventing me from doing it in the first place.
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

		if (this->loopEnabled)
		{
			if (this->localTimeSeconds > this->endTimeSeconds)
				this->localTimeSeconds -= this->endTimeSeconds - this->startTimeSeconds;
		}
		else
		{
			if (this->localTimeSeconds > this->totalTimeSeconds)
				this->localTimeSeconds = this->totalTimeSeconds;
		}
	}
	
	return true;
}

/*virtual*/ bool LoopedAudioModule::MoreSoundAvailable()
{
	if (this->loopEnabled)
		return true;

	if (this->localTimeSeconds < this->totalTimeSeconds)
		return true;

	return false;
}

void LoopedAudioModule::Release()
{
	this->loopEnabled = false;
}

bool LoopedAudioModule::UseNonLoopedAudioData(const AudioData* audioData, uint16_t channel, Error& error)
{
	WaveForm* waveForm = new WaveForm();
	if (!waveForm->ConvertFromAudioBuffer(audioData->GetFormat(), audioData->GetAudioBuffer(), audioData->GetAudioBufferSize(), channel, error))
	{
		delete waveForm;
		return false;
	}

	this->loopedWaveForm = waveForm;
	this->ownsWaveForm = true;

	this->startTimeSeconds = 0.0;
	this->endTimeSeconds = audioData->GetTimeSeconds();
	this->totalTimeSeconds = audioData->GetTimeSeconds();
	this->localTimeSeconds = 0.0;

	this->loopEnabled = false;

	return true;
}

bool LoopedAudioModule::UseLoopedAudioData(const SoundFontData::LoopedAudioData* loopedAudioData, uint16_t channel, Error& error)
{
	this->loopedWaveForm = loopedAudioData->GetCachedWaveForm(channel, error);
	if (!this->loopedWaveForm)
		return false;

	this->ownsWaveForm = false;

	const AudioData::Format& format = loopedAudioData->GetFormat();

	this->totalTimeSeconds = loopedAudioData->GetTimeSeconds();

	this->startTimeSeconds = format.BytesPerChannelToSeconds(loopedAudioData->GetLoop().startFrame * format.BytesPerFrame());
	this->endTimeSeconds = format.BytesPerChannelToSeconds(loopedAudioData->GetLoop().endFrame * format.BytesPerFrame());

	if (this->startTimeSeconds >= this->endTimeSeconds || this->startTimeSeconds < 0.0 || this->endTimeSeconds < 0.0)
	{
		error.Add(FormatString("Start time (%f) and end time (%f) don't make sense.", this->startTimeSeconds, this->endTimeSeconds));
		return false;
	}

	if (this->endTimeSeconds > this->totalTimeSeconds)
	{
		error.Add(FormatString("End time (%f) is greater than total time (%f) in looped audio sample.", this->endTimeSeconds, this->totalTimeSeconds));
		return false;
	}

	this->localTimeSeconds = 0.0;

	if (loopedAudioData->GetMode() == SoundFontData::LoopedAudioData::Mode::NOT_LOOPED)
		this->loopEnabled = false;
	else
		this->loopEnabled = true;

	return true;
}