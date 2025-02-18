#pragma once

#include "AudioDataLib/MIDI/MidiMsgSource.h"
#include "RtMidi.h"
#include "AudioDataLib/Error.h"
#include "Keyboard.h"

class MidiPortSource : public AudioDataLib::MidiMsgSource
{
public:
	MidiPortSource(const std::string& desiredPortName);
	virtual ~MidiPortSource();

	virtual bool Setup(AudioDataLib::Error& error) override;
	virtual bool Shutdown(AudioDataLib::Error& error) override;
	virtual bool Process(AudioDataLib::Error& error) override;

private:
	RtMidiIn* midiIn;
	std::string desiredPortName;
};