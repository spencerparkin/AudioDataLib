#pragma once

#include "Common.h"
#include "MidiData.h"

namespace AudioDataLib
{
	// Similar to the MidiPlayer class, we realy on the user to provide access
	// to the actual MIDI device.  This class is useful as a receptical for
	// MIDI messages received from a MIDI port.
	class AUDIO_DATA_LIB_API MidiRecorder
	{
	public:
		MidiRecorder();
		virtual ~MidiRecorder();

		void SetMidiData(MidiData* midiData) { this->midiData = midiData; }
		const MidiData* GetMidiData() const { return this->midiData; }
		MidiData* GetMidiData() { return this->midiData; }

		bool ReceiveMessage(const uint8_t* message, uint64_t messageSize, std::string& error);

	protected:
		MidiData* midiData;
	};
}