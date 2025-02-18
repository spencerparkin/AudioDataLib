#pragma once

#include "AudioDataLib/MIDI/MidiMsgDestination.h"

namespace AudioDataLib
{
	/**
	 * @brief This class is used mainly for diagnostic purposes, providing log message output
	 *        as MIDI message input is received.
	 */
	class AUDIO_DATA_LIB_API MidiMsgLogDestination : public MidiMsgDestination
	{
	public:
		MidiMsgLogDestination();
		virtual ~MidiMsgLogDestination();

		/**
		 * This will call the LogMessage method with an appropriately formatted log message string.
		 */
		virtual bool ReceiveMessage(double deltaTimeSeconds, const uint8_t* message, uint64_t messageSize) override;

		/**
		 * The user should override this to provide a place for the log output to go.
		 * 
		 * @param[in] message This is a string contain information about the MIDI message recieved.
		 */
		virtual void LogMessage(const std::string& message) = 0;
	};
};