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
	virtual bool Process(AudioDataLib::Error& error) override;
	virtual bool BeginPlayback(AudioDataLib::Error& error) override;
	virtual bool StopPlayback(AudioDataLib::Error& error) override;
	virtual bool BeginRecording(AudioDataLib::Error& error) override;
	virtual bool StopRecording(AudioDataLib::Error& error) override;
	virtual State GetState() const override;
	virtual bool GetStatusMessage(std::string& statusMsg) const override;

private:
	AudioDataLib::AudioData* audioData;
};