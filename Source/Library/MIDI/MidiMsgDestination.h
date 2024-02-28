#pragma once

#include "Common.h"

namespace AudioDataLib
{
	class Error;

	/**
	 * @brief Derivatives of this class are those that do something (anything) with MIDI messages.
	 * 
	 * For example, the user might provide a derivative of this class that sends the given messages
	 * to a MIDI port for synthesis.
	 */
	class AUDIO_DATA_LIB_API MidiMsgDestination
	{
	public:
		MidiMsgDestination();
		virtual ~MidiMsgDestination();

		/**
		 * This method will be called when this destination is to receive the given MIDI message.
		 * Note that it's important that no heavy processing be done in this method (such as is
		 * the mentality when writing an audio callback), because the timing of MIDI message reception
		 * and processing can effect the quality of audio playback.
		 * 
		 * @param[in] deltaTimeSeconds This is typically, but not always, set to the amount of time (in seconds) between now and the last time the function was called.
		 * @param[in] message This is the payload of the message, which can be decoded using a derivative of the MidiData::Event class.
		 * @param[in] messageSize This is the size of the payload in bytes.
		 * @param[out] error This should be populated with error information if false is returned.
		 * @return True should be returned on success; false otherwise.
		 */
		virtual bool ReceiveMessage(double deltaTimeSeconds, const uint8_t* message, uint64_t messageSize, Error& error);

		/**
		 * This method will get called by the MidiMsgSource class during its own initialization.
		 * 
		 * @param[out] error This should be populated with error information if false is returned.
		 * @return True should be returned on success; false otherwise.
		 */
		virtual bool Initialize(Error& error);

		/**
		 * This method will get called by the MidiMsgSource class during its own finalization.
		 *
		 * @param[out] error This should be populated with error information if false is returned.
		 * @return True should be returned on success; false otherwise.
		 */
		virtual bool Finalize(Error& error);

		/**
		 * This method will get called by the MidiMsgSource class during it's own processing, which
		 * should get get periodically a program's main loop, or perhaps a thread.
		 *
		 * @param[out] error This should be populated with error information if false is returned.
		 * @return True should be returned on success; false otherwise.
		 */
		virtual bool Process(Error& error);
	};
}