#pragma once

#include "AudioData.h"
#include "WaveForm.h"

class Audio
{
public:
	Audio();
	virtual ~Audio();

	virtual void Render() const = 0;
};

class WaveFormAudio : public Audio
{
public:
	WaveFormAudio();
	virtual ~WaveFormAudio();

	virtual void Render() const override;

	AudioDataLib::AudioData* audioData;
	mutable AudioDataLib::WaveForm* waveForm;
};