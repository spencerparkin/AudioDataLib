#pragma once

#include "AudioDataLib/MIDI/MidiMsgDestination.h"
#include "RtMidi.h"

class MidiPortDestination : public AudioDataLib::MidiMsgDestination
{
public:
	MidiPortDestination();
	virtual ~MidiPortDestination();

	virtual bool Initialize() override;
	virtual bool Finalize() override;
	virtual bool ReceiveMessage(double deltaTimeSeconds, const uint8_t* message, uint64_t messageSize) override;

private:
	RtMidiOut* midiOut;
};