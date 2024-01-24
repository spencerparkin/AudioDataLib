#pragma once

#include "MidiPlayer.h"
#include "RtMidi.h"

class RtMidiPlayer : public AudioDataLib::MidiPlayer
{
public:
	RtMidiPlayer(AudioDataLib::Timer* timer);
	virtual ~RtMidiPlayer();

	virtual bool BeginPlayback(const std::set<uint32_t>& tracksToPlaySet, AudioDataLib::Error& error) override;
	virtual bool EndPlayback(AudioDataLib::Error& error) override;
	virtual bool ManagePlayback(AudioDataLib::Error& error) override;
	virtual bool SendMessage(const uint8_t* message, uint64_t messageSize, AudioDataLib::Error& error) override;

private:
	RtMidiOut* midiOut;
};