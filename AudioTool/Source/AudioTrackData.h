#pragma once

#include "TrackData.h"
#include "AudioData.h"

class AudioTrackData : public TrackData
{
public:
	AudioTrackData();
	virtual ~AudioTrackData();

	AudioDataLib::AudioData* GetAudioData() { return this->audioData; }
	const AudioDataLib::AudioData* GetAudioData() const { return this->audioData; }

	void SetAudioData(AudioDataLib::AudioData* audioData);

	virtual void Render(wxPaintDC& paintDC) const override;
	virtual bool Process(std::string& error) override;
	virtual bool BeginPlayback(std::string& error) override;
	virtual bool StopPlayback(std::string& error) override;
	virtual bool BeginRecording(std::string& error) override;
	virtual bool StopRecording(std::string& error) override;
	virtual State GetState() const override;

private:
	AudioDataLib::AudioData* audioData;
};