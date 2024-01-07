#pragma once

#include "AudioCommon.h"

namespace AudioDataLib
{
	class MidiData;

	// This class is meant to be used in conjection with some other library that provides
	// access to a MIDI device.  Here we provide a convenient way to feed the MIDI device
	// appropriately for real-time synthesis and audio playback of the MIDI track data.
	class AUDIO_DATA_LIB_API MidiPlayer
	{
	public:
		MidiPlayer();
		virtual ~MidiPlayer();

		void SetMidiData(const MidiData* midiData) { this->midiData = midiData; }
		const MidiData* GetMidiData() const { return this->midiData; }

		// This gets called every frame.  This class keeps time and knows
		// when to fire the SendMessage() method and with what data.
		void Play();

		// This should get implemented by a user-drived class to send the given
		// message to the MIDI device.
		virtual void SendMessage(uint32_t message);

	protected:
		const MidiData* midiData;
	};
}