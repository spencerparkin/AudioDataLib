#pragma once

#include "Common.h"

namespace AudioDataLib
{
	class Error;
	class MidiMsgDestination;

	// This is any source of MIDI messages.  The user might provide a derivative
	// of this class that receives messages from a MIDI port.
	class AUDIO_DATA_LIB_API MidiMsgSource
	{
	public:
		MidiMsgSource();
		virtual ~MidiMsgSource();

		virtual bool Setup(Error& error);
		virtual bool Shutdown(Error& error);
		virtual bool Process(Error& error);

		void AddDestination(MidiMsgDestination* destination);
		void Clear();

	protected:
		bool BroadcastMidiMessage(double deltaTimeSeconds, const uint8_t* messageBuffer, uint64_t messageBufferSize, Error& error);

		std::vector<MidiMsgDestination*>* destinationArray;
	};
}