#pragma once

#include "Common.h"

namespace AudioDataLib
{
	class Error;

	// This is any class that does something with MIDI messages.
	// The user might provide a derivatives of this class that
	// sends the given messages to a MIDI port.
	class AUDIO_DATA_LIB_API MidiMsgDestination
	{
	public:
		MidiMsgDestination();
		virtual ~MidiMsgDestination();

		virtual bool ReceiveMessage(double deltaTimeSeconds, const uint8_t* message, uint64_t messageSize, Error& error);
		virtual bool Initialize(Error& error);
		virtual bool Finalize(Error& error);
		virtual bool Process(Error& error);
	};
}