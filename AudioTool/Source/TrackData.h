#pragma once

#include "AudioData.h"
#include <wx/string.h>

class TrackData
{
public:
	TrackData();
	virtual ~TrackData();

	const wxString& GetName() const { return this->name; }
	void SetName(const wxString& name) { this->name = name; }

	AudioDataLib::AudioData* GetAudioData() { return this->audioData; }
	const AudioDataLib::AudioData* GetAudioData() const { return this->audioData; }

	void SetAudioData(AudioDataLib::AudioData* audioData);

private:
	wxString name;
	AudioDataLib::AudioData* audioData;
};