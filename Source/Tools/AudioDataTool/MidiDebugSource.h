#pragma once

#include "MidiMsgSource.h"
#include "Error.h"
#include "Keyboard.h"

class MidiDebugSource : public AudioDataLib::MidiMsgSource
{
public:
	MidiDebugSource();
	virtual ~MidiDebugSource();

	virtual bool Setup(AudioDataLib::Error& error) override;
	virtual bool Shutdown(AudioDataLib::Error& error) override;
	virtual bool Process(AudioDataLib::Error& error) override;

private:
	bool SendNoteMessage(uint32_t note, bool onOff, AudioDataLib::Error& error);

	Keyboard* keyboard;
};