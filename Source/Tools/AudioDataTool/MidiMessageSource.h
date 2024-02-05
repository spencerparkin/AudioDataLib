#pragma once

#include "MidiSynth.h"
#include "RtMidi.h"
#include "Error.h"
#include "Keyboard.h"

class MidiMessageSource
{
public:
	MidiMessageSource();
	virtual ~MidiMessageSource();

	virtual bool Setup(AudioDataLib::Error& error);
	virtual bool Shutdown(AudioDataLib::Error& error);
	virtual bool Process(AudioDataLib::Error& error);

	void AddSynth(AudioDataLib::MidiSynth* synth);
	void Clear();

protected:
	bool BroadcastMidiMessage(double deltaTimeSeconds, const uint8_t* messageBuffer, uint64_t messageBufferSize, AudioDataLib::Error& error);

	std::vector<AudioDataLib::MidiSynth*> synthArray;
};

class RtMidiMessageSource : public MidiMessageSource
{
public:
	RtMidiMessageSource(const std::string& desiredPortName);
	virtual ~RtMidiMessageSource();

	virtual bool Setup(AudioDataLib::Error& error) override;
	virtual bool Shutdown(AudioDataLib::Error& error) override;
	virtual bool Process(AudioDataLib::Error& error) override;

private:
	RtMidiIn* midiIn;
	std::string desiredPortName;
};

class KeyboardMidiMessageSource : public MidiMessageSource
{
public:
	KeyboardMidiMessageSource();
	virtual ~KeyboardMidiMessageSource();

	virtual bool Setup(AudioDataLib::Error& error) override;
	virtual bool Shutdown(AudioDataLib::Error& error) override;
	virtual bool Process(AudioDataLib::Error& error) override;

private:
	bool SendNoteMessage(uint32_t note, bool onOff, AudioDataLib::Error& error);

	Keyboard* keyboard;
};