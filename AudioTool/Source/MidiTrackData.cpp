#include "MidiTrackData.h"

using namespace AudioDataLib;

MidiTrackData::MidiTrackData()
{
	this->midiData = nullptr;
}

/*virtual*/ MidiTrackData::~MidiTrackData()
{
	MidiData::Destroy(this->midiData);
}

/*virtual*/ void MidiTrackData::Render(wxPaintDC& paintDC) const
{
	//...
}

void MidiTrackData::SetMidiData(MidiData* midiData)
{
	if(this->midiData)
		MidiData::Destroy(this->midiData);

	this->midiData = midiData;
}