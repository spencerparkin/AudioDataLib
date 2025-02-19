#pragma once

#include "AudioDataLib/Common.h"

namespace AudioDataLib
{
	class MidiMsgDestination;

	/**
	 * @brief Instances of this class represent any source of MIDI messages.
	 * 
	 * The user might provide a derivative of this class, for example, that can receive messages from a MIDI port.
	 * One is not provided by this library, because one of the goals of this library is to depend at-most on the
	 * standard C++ library.  Details of interfacing with hardware are left to 3rd-party libraries.  You can find
	 * an example of using such a library in the same Git repo that houses AudioDataLib.
	 * 
	 * Rather, what this class does is manage a source and any number of destinations connected to it.  As source
	 * messages are produced, they are boardcast to all destinations, or instance of the MidiMsgDestination class
	 * which, in turn, know what to do with the messages (e.g., synthesize them into sound.)
	 */
	class AUDIO_DATA_LIB_API MidiMsgSource
	{
	public:
		MidiMsgSource();
		virtual ~MidiMsgSource();

		/**
		 * Perform any setup that is required, but also call this base-class method as it will
		 * attempt to initialize each MidiMsgDestination added to this class instance.  If any
		 * one of those fails, then setup should fail.
		 */
		virtual bool Setup();

		/**
		 * Perform any shutdown that is required, but also call this base-class method as it will
		 * shutdown each MidiMsgDestination added to this class instance.  Though setup and shutdown
		 * routines (or, using alternative terms, initialization and finalization routines) have the
		 * same signature, it is convention that shutdown routines always succeed, at least as far as
		 * doing any clean-up needed to reclaim the state of the program after a successful or failed
		 * initialization.  Still failure can be returned here if it's useful, and error information
		 * can be provided, but the return value is usually not checked.
		 */
		virtual bool Shutdown();

		/**
		 * This is called once per frame in a program's main loop, and should do any necessary
		 * processing of the MIDI message source (e.g., poll messages from a port), but should
		 * also call the base-class method here to process each connected MidiMsgDestination class.
		 */
		virtual bool Process();

		/**
		 * Add a MIDI message destination.  The given MidiMsgDestination class will receive
		 * messages from this source.
		 */
		void AddDestination(std::shared_ptr<MidiMsgDestination> destination);

		/**
		 * Remove all connected MidiMsgDestination classes.
		 */
		void Clear();

		/**
		 * Find and return a pointer to the first MidiMsgDestination class that successfully
		 * casts to the given type.
		 */
		template<typename T>
		T* FindDestination()
		{
			for (auto& destination : this->destinationArray)
			{
				T* foundDestination = dynamic_cast<T*>(destination.get());
				if (foundDestination)
					return foundDestination;
			}

			return nullptr;
		}

	protected:

		/**
		 * This should be called by the derived class whenever a MIDI message is produced or received from
		 * the source represented by this class.
		 */
		bool BroadcastMidiMessage(double deltaTimeSeconds, const uint8_t* messageBuffer, uint64_t messageBufferSize);

		std::vector<std::shared_ptr<MidiMsgDestination>> destinationArray;
	};
}