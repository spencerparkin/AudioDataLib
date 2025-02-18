#include "AudioDataLib/SynthModules/LoopedAudioModule.h"
#include "AudioDataLib/WaveForm.h"
#include "AudioDataLib/ErrorSystem.h"

using namespace AudioDataLib;

LoopedAudioModule::LoopedAudioModule()
{
	this->loopedWaveForm = new std::shared_ptr<WaveForm>();
	this->startTimeSeconds = 0.0;
	this->endTimeSeconds = 0.0;
	this->localTimeSeconds = 0.0;
	this->totalTimeSeconds = 0.0;
	this->loopEnabled = true;
}

/*virtual*/ LoopedAudioModule::~LoopedAudioModule()
{
	delete this->loopedWaveForm;
}

/*virtual*/ bool LoopedAudioModule::GenerateSound(double durationSeconds, double samplesPerSecond, WaveForm& waveForm, SynthModule* callingModule)
{
	if (!this->loopedWaveForm)
	{
		ErrorSystem::Get()->Add("No looped wave-form we can use to generate audio.");
		return false;
	}

	waveForm.Clear();

	double generatedSoundTimeSeconds = 0.0;
	double deltaTimeSeconds = 1.0 / samplesPerSecond;

	while (generatedSoundTimeSeconds <= durationSeconds)
	{
		WaveForm::Sample sample;
		sample.timeSeconds = generatedSoundTimeSeconds;
		sample.amplitude = (*this->loopedWaveForm)->EvaluateAt(this->localTimeSeconds);
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

bool LoopedAudioModule::UseNonLoopedAudioData(const AudioData* audioData, uint16_t channel)
{
	std::shared_ptr<WaveForm> waveForm(new WaveForm());
	if (!waveForm->ConvertFromAudioBuffer(audioData->GetFormat(), audioData->GetAudioBuffer(), audioData->GetAudioBufferSize(), channel))
		return false;

	*this->loopedWaveForm = waveForm;

	this->startTimeSeconds = 0.0;
	this->endTimeSeconds = audioData->GetTimeSeconds();
	this->totalTimeSeconds = audioData->GetTimeSeconds();
	this->localTimeSeconds = 0.0;

	this->loopEnabled = false;

	return true;
}

bool LoopedAudioModule::UseLoopedAudioData(const WaveTableData::AudioSampleData* audioSampleData, uint16_t channel)
{
	*this->loopedWaveForm = audioSampleData->GetCachedWaveForm(channel);
	if (!this->loopedWaveForm->get())
		return false;

	const AudioData::Format& format = audioSampleData->GetFormat();

	this->totalTimeSeconds = audioSampleData->GetTimeSeconds();

	this->startTimeSeconds = format.BytesPerChannelToSeconds(audioSampleData->GetLoop().startFrame * format.BytesPerFrame());
	this->endTimeSeconds = format.BytesPerChannelToSeconds(audioSampleData->GetLoop().endFrame * format.BytesPerFrame());

	if (this->startTimeSeconds >= this->endTimeSeconds || this->startTimeSeconds < 0.0 || this->endTimeSeconds < 0.0)
	{
		ErrorSystem::Get()->Add(std::format("Start time ({}) and end time ({}) don't make sense.", this->startTimeSeconds, this->endTimeSeconds));
		return false;
	}

	if (this->endTimeSeconds > this->totalTimeSeconds)
	{
		ErrorSystem::Get()->Add(std::format("End time ({}) is greater than total time ({}) in looped audio sample.", this->endTimeSeconds, this->totalTimeSeconds));
		return false;
	}

	this->localTimeSeconds = 0.0;

	if (audioSampleData->GetMode() == SoundFontData::AudioSampleData::Mode::NOT_LOOPED)
		this->loopEnabled = false;
	else
		this->loopEnabled = true;

	return true;
}