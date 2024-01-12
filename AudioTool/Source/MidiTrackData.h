#pragma once

#include "TrackData.h"
#include "MidiData.h"
#include "MidiPlayer.h"
#include "RtMidi/RtMidi.h"

class MidiTrackData : public TrackData
{
public:
	MidiTrackData();
	virtual ~MidiTrackData();

	virtual void Render(wxPaintDC& paintDC) const override;
	virtual bool Process(std::string& error) override;
	virtual bool BeginPlayback(std::string& error) override;
	virtual bool StopPlayback(std::string& error) override;
	virtual bool BeginRecording(std::string& error) override;
	virtual bool StopRecording(std::string& error) override;
	virtual State GetState() const override;
	virtual bool GetStatusMessage(std::string& statusMsg) const override;

	void SetMidiData(AudioDataLib::MidiData* midiData);

	AudioDataLib::MidiData* GetMidiData() { return this->midiData; }
	const AudioDataLib::MidiData* GetMidiData() const { return this->midiData; }

private:

	class MidiPlayer : public AudioDataLib::MidiPlayer
	{
	public:
		MidiPlayer();
		virtual ~MidiPlayer();

		virtual bool BeginPlayback(const std::set<uint32_t>& tracksToPlaySet, std::string& error) override;
		virtual bool EndPlayback(std::string& error) override;
		virtual void SendMessage(const uint8_t* message, uint64_t messageSize) override;

	private:
		RtMidiOut* midiOut;
	};

	AudioDataLib::MidiData* midiData;
	MidiPlayer* midiPlayer;
};