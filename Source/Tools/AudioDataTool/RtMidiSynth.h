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

	static void CallbackEntryPoint(double timeStamp, std::vector<unsigned char>* message, void* userData);

	void Callback(double timeStamp, std::vector<unsigned char>* message);

	RtMidiIn* midiIn;

	std::vector<AudioDataLib::MidiSynth*> synthArray;
};