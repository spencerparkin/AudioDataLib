#pragma once

#include "AudioData.h"
#include "WaveForm.h"
#include "FrequencyGraph.h"
#include "Math2D.h"

class Audio
{
public:
	Audio();
	virtual ~Audio();

	virtual void Render() const = 0;
	virtual Box2D CalcBoundingBox() const = 0;

	bool IsSelected() const { return this->selected; }
	void SetSelected(bool selected) { this->selected = selected; }

protected:
	bool selected;
};

class WaveFormAudio : public Audio
{
public:
	WaveFormAudio();
	virtual ~WaveFormAudio();

	virtual void Render() const override;
	virtual Box2D CalcBoundingBox() const override;

	void SetAudioData(AudioDataLib::AudioData* audioData);

	void SetWaveForm(AudioDataLib::WaveForm* waveForm);
	const AudioDataLib::WaveForm* GetWaveForm() const;

private:
	AudioDataLib::AudioData* audioData;
	mutable AudioDataLib::WaveForm* waveForm;
};

class FrequencyGraphAudio : public Audio
{
public:
	FrequencyGraphAudio();
	virtual ~FrequencyGraphAudio();

	void SetFrequencyGraph(AudioDataLib::FrequencyGraph* frequencyGraph);

	virtual void Render() const override;
	virtual Box2D CalcBoundingBox() const override;

private:
	AudioDataLib::FrequencyGraph* frequencyGraph;
};