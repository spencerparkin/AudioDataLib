#pragma once

#include "MidiMsgDestination.h"

namespace AudioDataLib
{
	class MidiData;
	
	// This class is designed to capture MIDI messages (e.g., from a MIDI keyboard) to
	// be stored in the given MidiData object.
	class AUDIO_DATA_LIB_API MidiMsgRecorderDestination : public MidiMsgDestination
	{
	public:
		MidiMsgRecorderDestination();
		virtual ~MidiMsgRecorderDestination();

		virtual bool ReceiveMessage(double deltaTimeSeconds, const uint8_t* message, uint64_t messageSize, Error& error) override;
		virtual bool Finalize(Error& error) override;

		void SetMidiData(MidiData* midiData) { this->midiData = midiData; }
		MidiData* GetMidiData() { return this->midiData; }
		const MidiData* GetMidiData() const { return this->midiData; }

		void Clear();

	private:
		MidiData* midiData;

		struct MessageData
		{
			double deltaTimeSeconds;
			std::vector<uint8_t> messageBuffer;
		};

		std::vector<MessageData*>* messageDataArray;
	};
}