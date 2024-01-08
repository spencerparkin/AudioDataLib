#pragma once

#include "TrackData.h"
#include "MidiData.h"

class MidiTrackData : public TrackData
{
public:
	MidiTrackData();
	virtual ~MidiTrackData();

	virtual void Render(wxPaintDC& paintDC) const override;

	void SetMidiData(AudioDataLib::MidiData* midiData);

	AudioDataLib::MidiData* GetMidiData() { return this->midiData; }
	const AudioDataLib::MidiData* GetMidiData() const { return this->midiData; }

private:
	AudioDataLib::MidiData* midiData;
};