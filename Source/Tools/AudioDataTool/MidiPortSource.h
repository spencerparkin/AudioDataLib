#pragma once

#include "AudioDataLib/MIDI/MidiMsgSource.h"
#include "RtMidi.h"
#include "AudioDataLib/ErrorSystem.h"
#include "Keyboard.h"

class MidiPortSource : public AudioDataLib::MidiMsgSource
{
public:
	MidiPortSource(const std::string& desiredPortName);
	virtual ~MidiPortSource();

	virtual bool Setup() override;
	virtual bool Shutdown() override;
	virtual bool Process() override;

private:
	RtMidiIn* midiIn;
	std::string desiredPortName;
};