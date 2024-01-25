#pragma once

#include "MidiSynth.h"
#include "RtMidi.h"
#include "Error.h"

class RtMidiSynth
{
public:
	RtMidiSynth();
	virtual ~RtMidiSynth();

	void AddSynth(AudioDataLib::MidiSynth* synth);
	void Clear();

	bool Setup(const std::string& desiredPortName, AudioDataLib::Error& error);
	bool Shutdown(AudioDataLib::Error& error);
	bool Process(AudioDataLib::Error& error);

private:

	RtMidiIn* midiIn;

	std::vector<AudioDataLib::MidiSynth*> synthArray;
};