#pragma once

#include "MidiMsgDestination.h"

namespace AudioDataLib
{
	class AUDIO_DATA_LIB_API MidiMsgLogDestination : public MidiMsgDestination
	{
	public:
		MidiMsgLogDestination();
		virtual ~MidiMsgLogDestination();

		virtual bool ReceiveMessage(double deltaTimeSeconds, const uint8_t* message, uint64_t messageSize, Error& error) override;

		// The user should override this to provide a place for the log output to go.
		virtual void LogMessage(const std::string& message) = 0;
	};
};