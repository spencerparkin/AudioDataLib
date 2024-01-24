#pragma once

#include "MidiSynth.h"

namespace AudioDataLib
{
	class MidiData;
	
	// This class is designed to capture MIDI messages (e.g., from a MIDI keyboard) to
	// be stored in the given MidiData object.
	class AUDIO_DATA_LIB_API RecorderSynth : public MidiSynth
	{
	public:
		RecorderSynth();
		virtual ~RecorderSynth();

		virtual bool ReceiveMessage(double deltaTimeSeconds, const uint8_t* message, uint64_t messageSize, Error& error) override;

		void SetMidiData(MidiData* midiData) { this->midiData = midiData; }
		MidiData* GetMidiData() { return this->midiData; }
		const MidiData* GetMidiData() const { return this->midiData; }

	private:
		MidiData* midiData;
	};
}