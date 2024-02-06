#pragma once

#include "MidiMsgDestination.h"
#include "RtMidi.h"

class MidiPortDestination : public AudioDataLib::MidiMsgDestination
{
public:
	MidiPortDestination();
	virtual ~MidiPortDestination();

	virtual bool Initialize(AudioDataLib::Error& error) override;
	virtual bool Finalize(AudioDataLib::Error& error) override;
	virtual bool ReceiveMessage(double deltaTimeSeconds, const uint8_t* message, uint64_t messageSize, AudioDataLib::Error& error) override;

private:
	RtMidiOut* midiOut;
};