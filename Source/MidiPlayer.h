#pragma once

#include "Common.h"

namespace AudioDataLib
{
	class MidiData;

	// This class is meant to be used in conjection with some other library that provides
	// access to a MIDI device.  Here we provide a convenient way to feed the MIDI device
	// appropriately for real-time synthesis and audio playback of the MIDI track data.
	// That is to say, we handle the timing of when to send the MIDI messages.
	class AUDIO_DATA_LIB_API MidiPlayer
	{
	public:
		MidiPlayer();
		virtual ~MidiPlayer();

		void SetMidiData(const MidiData* midiData) { this->midiData = midiData; }
		const MidiData* GetMidiData() const { return this->midiData; }

		void SetTimeSeconds(double timeSeconds) { this->timeSeconds = timeSeconds; }
		double GetTimeSeconds() const { return this->timeSeconds; }

		virtual bool BeginPlayback(std::string& error);
		virtual bool EndPlayback(std::string& error);

		// This gets called every iteration of the user's main program loop.  This class
		// keeps time and knows when to fire the SendMessage() method and with what data.
		// If this method is not called often enough, it can result in faulty play-back.
		virtual void ManagePlayback();

		// This should get implemented by a user-drived class to send the
		// given message to the MIDI device.
		virtual void SendMessage(const uint8_t* message, uint64_t messageSize);

	protected:
		double timeSeconds;
		const MidiData* midiData;
	};
}