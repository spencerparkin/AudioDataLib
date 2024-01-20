#pragma once

#include "AudioData.h"
#include "WaveForm.h"
#include "Math2D.h"

class Audio
{
public:
	Audio();
	virtual ~Audio();

	virtual void Render() const = 0;
	virtual Box2D CalcBoundingBox() const = 0;
};

class WaveFormAudio : public Audio
{
public:
	WaveFormAudio();
	virtual ~WaveFormAudio();

	virtual void Render() const override;
	virtual Box2D CalcBoundingBox() const override;

	void SetAudioData(AudioDataLib::AudioData* audioData);
	const AudioDataLib::WaveForm* GetWaveForm() const;

private:
	AudioDataLib::AudioData* audioData;
	mutable AudioDataLib::WaveForm* waveForm;
};