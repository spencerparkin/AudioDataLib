#pragma once

#include "AudioDataLib/MIDI/MidiMsgSource.h"
#include "AudioDataLib/ErrorSystem.h"
#include "Keyboard.h"

class MidiDebugSource : public AudioDataLib::MidiMsgSource
{
public:
	MidiDebugSource();
	virtual ~MidiDebugSource();

	virtual bool Setup() override;
	virtual bool Shutdown() override;
	virtual bool Process() override;

private:
	bool SendNoteMessage(uint32_t note, bool onOff);

	Keyboard* keyboard;
};