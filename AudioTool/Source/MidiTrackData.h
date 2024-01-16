#pragma once

#include "TrackData.h"
#include "Timer.h"
#include "MidiData.h"
#include "MidiPlayer.h"
#include "RtMidi/RtMidi.h"

class MidiTrackData : public TrackData
{
public:
	MidiTrackData();
	virtual ~MidiTrackData();

	virtual void Render(wxPaintDC& paintDC) const override;
	virtual bool Process(AudioDataLib::Error& error) override;
	virtual bool BeginPlayback(AudioDataLib::Error& error) override;
	virtual bool StopPlayback(AudioDataLib::Error& error) override;
	virtual bool BeginRecording(AudioDataLib::Error& error) override;
	virtual bool StopRecording(AudioDataLib::Error& error) override;
	virtual State GetState() const override;
	virtual bool GetStatusMessage(std::string& statusMsg) const override;

	void SetMidiData(AudioDataLib::MidiData* midiData);

	AudioDataLib::MidiData* GetMidiData() { return this->midiData; }
	const AudioDataLib::MidiData* GetMidiData() const { return this->midiData; }

private:

	class MidiPlayer : public AudioDataLib::MidiPlayer
	{
	public:
		MidiPlayer(AudioDataLib::Timer* timer);
		virtual ~MidiPlayer();

		virtual bool BeginPlayback(const std::set<uint32_t>& tracksToPlaySet, AudioDataLib::Error& error) override;
		virtual bool EndPlayback(AudioDataLib::Error& error) override;
		virtual bool SendMessage(const uint8_t* message, uint64_t messageSize, AudioDataLib::Error& error) override;

	private:
		RtMidiOut* midiOut;
	};

	AudioDataLib::MidiData* midiData;
	MidiPlayer* midiPlayer;
};