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

		void AddDestination(std::shared_ptr<MidiMsgDestination> destination);
		void Clear();

		template<typename T>
		T* FindDestination()
		{
			for (auto& destination : *this->destinationArray)
			{
				T* foundDestination = dynamic_cast<T*>(destination.get());
				if (foundDestination)
					return foundDestination;
			}

			return nullptr;
		}

	protected:
		bool BroadcastMidiMessage(double deltaTimeSeconds, const uint8_t* messageBuffer, uint64_t messageBufferSize, Error& error);

		std::vector<std::shared_ptr<MidiMsgDestination>>* destinationArray;
	};
}