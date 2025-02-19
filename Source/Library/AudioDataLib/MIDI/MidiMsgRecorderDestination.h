#pragma once

#include "AudioDataLib/MIDI/MidiMsgDestination.h"

namespace AudioDataLib
{
	class MidiData;
	
	/**
	 * @brief This class collects MIDI messages it receives so that it can later compile them into a given MidiData object,
	 *        which can be, in turn, written to disk, if desired.
	 * 
	 * If you wanted to capture MIDI messages from a MIDI device (e.g., a MIDI keyboard) and then save the result to a MIDI file, then this class helps you do that.
	 * You would add an instance of this class to a MidiMsgSource derivative which can produce the MIDI messages from the port.  Of course,
	 * you would define and provide this derivative, because one of the goals of AudioDataLib is to be dependency-free (apart from the standard
	 * C++ library, of course.)  There are many available APIs that can open, close and process MIDI ports.
	 */
	class AUDIO_DATA_LIB_API MidiMsgRecorderDestination : public MidiMsgDestination
	{
	public:
		MidiMsgRecorderDestination();
		virtual ~MidiMsgRecorderDestination();

		/**
		 * This method caches the given MIDI message for later processing in the Finalize method.
		 */
		virtual bool ReceiveMessage(double deltaTimeSeconds, const uint8_t* message, uint64_t messageSize) override;

		/**
		 * Compile all cached MIDI messages into the MidiData instance owned by this class.
		 */
		virtual bool Finalize() override;

		/**
		 * Set the MidiData instance that this class will populate.  Ownership of the memory is not taken here.
		 */
		void SetMidiData(MidiData* midiData) { this->midiData = midiData; }

		/**
		 * Return the owned MidiData instance pointer set with SetMidiData.
		 */
		MidiData* GetMidiData() { return this->midiData; }

		/**
		 * Return a read-only pointer to the ownd MidiData instance.
		 */
		const MidiData* GetMidiData() const { return this->midiData; }

		/**
		 * Clear the MIDI message cache.
		 */
		void Clear();

	private:
		MidiData* midiData;

		struct MessageData
		{
			double deltaTimeSeconds;
			std::vector<uint8_t> messageBuffer;
		};

		std::vector<MessageData*> messageDataArray;
	};
}