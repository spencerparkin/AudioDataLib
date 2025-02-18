#pragma once

#include "AudioDataLib/FileDatas/FileData.h"
#include "AudioDataLib/ByteStream.h"

namespace AudioDataLib
{
	/**
	 * @brief This data representing the contents of a MIDI file.
	 * 
	 * Instances of this class can be used to read or write MIDI file data.  They can also be
	 * passed to the MidiPlayer class or the MidiMsgRecorderDestination class for playback or
	 * recording, respectively.
	 */
	class AUDIO_DATA_LIB_API MidiData : public FileData
	{
		friend class MidiFileFormat;

	public:
		MidiData();
		virtual ~MidiData();

		virtual void DumpInfo(FILE* fp) const override;
		virtual void DumpCSV(FILE* fp) const override;
		virtual FileData* Clone() const override;

		/**
		 * Remove all tracks from this MIDI file data object.
		 */
		void Clear();

		/**
		 * Calculate and return the time duration (in seconds) of the MIDI data for a given track.
		 * 
		 * @param[in] i This is the zero-based track number.  Use GetNumTracks to know how many tracks there are.
		 * @param[out] totalTimeSeconds The playback length of the track measured in seconds.
		 * @return True is returned on success, otherwise false, and the error parameter should have reasons for the failure.
		 */
		bool CalculateTrackLengthInSeconds(uint32_t i, double& totalTimeSeconds) const;

		static MidiData* Create();
		static void Destroy(MidiData* midiData);

		/**
		 * This is the type of MIDI data you're dealing with.  As of this writing,
		 * there is no support for VARIOUS_TRACKS, but the others are supported.
		 */
		enum FormatType
		{
			SINGLE_TRACK = 0,
			MULTI_TRACK = 1,
			VARIOUS_TRACKS = 2
		};

		/**
		 * This structure defines how the MIDI data is timed.  MIDI files can be
		 * timed in two different ways.  As of this writing, only the TICKS_PER_QUARTER_NOTE
		 * method is supported.  For now, there isn't much care about how many ticks there
		 * are in a given quarter-note.  Rather, this value, in combination with the tempo
		 * (expressed in microseconds per quarter-note), helps us determine the number of
		 * ticks per microsecond which, in turn, helps us know when to execute MIDI messages.
		 */
		struct Timing
		{
			union
			{
				uint16_t ticksPerQuarterNote;
				struct
				{
					int8_t framesPerSecond;
					uint8_t ticksPerFrame;
				};
			};

			enum Type
			{
				TICKS_PER_QUARTER_NOTE,
				FRAMES_PER_SECOND
			};

			Type type;
		};

		class Event;
		class MetaEvent;

		/**
		 * @brief This is a single time-line of MIDI messages (also called events), or a sequence of MIDI messages meant to execute one after another.
		 * 
		 * A MIDI file can contain one or more tracks, all meant to play in parallel with one another.
		 * That is, unless the FormatType is VARIOUS_TRACKS, in which case, the tracks are independent, I believe.
		 */
		class AUDIO_DATA_LIB_API Track
		{
		public:
			Track();
			virtual ~Track();

			/**
			 * Remove all MIDI messages from this track.
			 */
			void Clear();

			/**
			 * Find a MIDI message in the track of the given type using the given predicate.
			 * 
			 * @param[in] T The type of MIDI message.
			 * @param[in] matchFunc A lambda to return true if the given event is the one for which you are looking.
			 * @return Returns a pointer to the found event, if any, or nullptr if not found.
			 */
			template<typename T>
			const T* FindEvent(std::function<bool(T*)> matchFunc) const
			{
				for (Event* event : *this->eventArray)
				{
					T* eventT = dynamic_cast<T*>(event);
					if (eventT && matchFunc(eventT))
						return eventT;
				}

				return nullptr;
			}

			/**
			 * Find a meta-event of the given type in this track.
			 * 
			 * @param[in] type The type of meta-event.  See the MetaEvent::Type enumeration.
			 * @return Returns a pointer to the found event, if any, or nullptr if not found.
			 */
			const MetaEvent* FindMetaEventOfType(uint8_t type) const
			{
				return this->FindEvent<MetaEvent>([=](MetaEvent* event) -> bool { return event->type == type; });
			}

			/**
			 * Get a pointer to an event in the track at the given offset.
			 * 
			 * @param[in] i Events are stored in an array.  This is the offset into that array to the desired event.
			 * @return The desired event at the given offset is returned, or nullptr if the given offset is out of range.
			 */
			const Event* GetEvent(uint32_t i) const;

			/**
			 * For convenient, get a reference to the event array owned by the Track class.
			 */
			const std::vector<Event*>& GetEventArray() const { return *this->eventArray; }

			/**
			 * Append the given event object to this track's event array.  Note that this
			 * class takes ownership of the event memory, and it is expected that the given
			 * event is allocated on the heap.
			 */
			void AddEvent(Event* event) { this->eventArray->push_back(event); }

		private:
			std::vector<Event*>* eventArray;
		};

		/**
		 * Instances of the Event class are MIDI messages.  There are three different
		 * types of such messages, and so there are three derivatives of this class;
		 * namely, MetaEvent, ChannelEvent and SystemExclusiveEvent.  A common base
		 * ensures here that each derivative impliments an encoder and a decoder
		 * for data typically seen going to, or coming from, a MIDI port.
		 * 
		 * Encoding/decoding of events seems like it's something that should only
		 * be part of the MidiFileFormat class, but in other contexts we need to be
		 * able to encode an event (e.g., before we send it down to a MIDI device on
		 * a MIDI port for synthesis); and similarly, we also need to be able to decode
		 * a MIDI message (e.g., that we receive from a MIDI port for storage.)
		 */
		class AUDIO_DATA_LIB_API Event
		{
		public:
			Event();
			virtual ~Event();

			/**
			 * Try to read and interpret bytes from the given stream as a MIDI message
			 * of the derived class type.
			 * 
			 * @param[in,out] inputStream The byte stream from which to read bytes.
			 * @return True is returned on success; false on failure.
			 */
			virtual bool Decode(ByteStream& inputStream) = 0;

			/**
			 * Try to write this MIDI message (of the derived class type) as a sequence
			 * of bytes to the given stream.  This is how the message should be stored in
			 * a MIDI file or how it should appear when handed to, or received from, a
			 * MIDI device.
			 * 
			 * @param[in,out] outputStream The byte stream to which bytes are to be written.
			 * @return True is returned on success; false on failure.
			 */
			virtual bool Encode(ByteStream& outputStream) const = 0;

			/**
			 * Return a one-line, human-readable message for logging purposes.
			 */
			virtual std::string LogMessage() const = 0;

			uint64_t deltaTimeTicks;
		};

		class AUDIO_DATA_LIB_API SystemExclusiveEvent : public Event
		{
		public:
			SystemExclusiveEvent();
			virtual ~SystemExclusiveEvent();

			virtual bool Decode(ByteStream& inputStream) override;
			virtual bool Encode(ByteStream& outputStream) const override;

			virtual std::string LogMessage() const override;
		};

		class AUDIO_DATA_LIB_API MetaEvent : public Event
		{
		public:
			MetaEvent();
			virtual ~MetaEvent();

			virtual bool Decode(ByteStream& inputStream) override;
			virtual bool Encode(ByteStream& outputStream) const override;

			virtual std::string LogMessage() const override;

			enum Type : uint8_t
			{
				SEQUENCE_NUMBER		= 0x00,
				TEXT_EVENT			= 0x01,
				COPYRIGHT_NOTICE	= 0x02,
				TRACK_NAME			= 0x03,
				INSTRUMENT_NAME		= 0x04,
				LYRICS				= 0x05,
				MARKER				= 0x06,
				CUE_POINT			= 0x07,
				CHANNEL_PREFIX		= 0x20,
				END_OF_TRACK		= 0x2F,
				SET_TEMPO			= 0x51,
				SMPTE_OFFSET		= 0x54,
				TIME_SIGNATURE		= 0x58,
				KEY_SIGNATURE		= 0x59,
				SEQUENCER_SPECIFIC	= 0x7F,
				UNKNOWN				= 0xFF
			};

			Type type;
			void* data;

			struct SequenceNumber
			{
				uint8_t msb, lsb;
			};

			struct Text
			{
				char* buffer;
			};

			struct ChannelPrefix
			{
				uint8_t channel;
			};

			struct Tempo
			{
				uint32_t microsecondsPerQuarterNote;
			};

			struct SMPTEOffset
			{
				uint8_t hours;
				uint8_t minutes;
				uint8_t seconds;
				uint8_t frames;
				uint8_t subFrames;
			};

			struct TimeSignature
			{
				uint8_t numerator;
				uint8_t denominator;
				uint8_t metro;
				uint8_t __32nds;

				operator std::string() const;
			};

			struct KeySignature
			{
				uint8_t key;
				uint8_t scale;

				operator std::string() const;
			};

			struct Opaque
			{
				uint8_t* buffer;
				uint64_t bufferSize;
			};

			template<typename T>
			const T* GetData() const
			{
				return nullptr;
			}

			template<>
			const char* GetData() const
			{
				switch (this->type)
				{
					case Type::TEXT_EVENT:
					case Type::COPYRIGHT_NOTICE:
					case Type::TRACK_NAME:
					case Type::INSTRUMENT_NAME:
					case Type::LYRICS:
					case Type::MARKER:
					case Type::CUE_POINT:
					{
						return ((const Text*)this->data)->buffer;
					}
                    default:
                    {
                        break;
                    }
				}

				return nullptr;
			}

			template<>
			const SequenceNumber* GetData() const
			{
				return (this->type == Type::SEQUENCE_NUMBER) ? static_cast<const SequenceNumber*>(this->data) : nullptr;
			}

			template<>
			const ChannelPrefix* GetData() const
			{
				return (this->type == Type::CHANNEL_PREFIX) ? static_cast<const ChannelPrefix*>(this->data) : nullptr;
			}

			template<>
			const Tempo* GetData() const
			{
				return (this->type == Type::SET_TEMPO) ? static_cast<const Tempo*>(this->data) : nullptr;
			}

			template<>
			const SMPTEOffset* GetData() const
			{
				return (this->type == Type::SMPTE_OFFSET) ? static_cast<const SMPTEOffset*>(this->data) : nullptr;
			}

			template<>
			const TimeSignature* GetData() const
			{
				return (this->type == Type::TIME_SIGNATURE) ? static_cast<const TimeSignature*>(this->data) : nullptr;
			}

			template<>
			const KeySignature* GetData() const
			{
				return (this->type == Type::KEY_SIGNATURE) ? static_cast<const KeySignature*>(this->data) : nullptr;
			}

			template<>
			const Opaque* GetData() const
			{
				return (this->type == Type::SEQUENCER_SPECIFIC) ? static_cast<const Opaque*>(this->data) : nullptr;
			}
		};

		class AUDIO_DATA_LIB_API ChannelEvent : public Event
		{
		public:
			ChannelEvent();
			virtual ~ChannelEvent();

			virtual bool Decode(ByteStream& inputStream) override;
			virtual bool Encode(ByteStream& outputStream) const override;

			virtual std::string LogMessage() const override;

			enum Type : uint8_t
			{
				NOTE_OFF			= 0x8,
				NOTE_ON				= 0x9,
				NOTE_AFTERTOUCH		= 0xA,
				CONTROLLER			= 0xB,
				PROGRAM_CHANGE		= 0xC,
				CHANNEL_AFTERTOUCH	= 0xD,
				PITCH_BEND			= 0xE,
				UNKNOWN				= 0xF
			};

			Type type;
			uint8_t channel;
			uint8_t param1, param2;
		};

		FormatType GetFormatType() const { return this->formatType; }
		void SetFormatType(FormatType formatType) { this->formatType = formatType; }
		const Timing& GetTiming() const { return this->timing; }
		void SetTiming(Timing timing) { this->timing = timing; }
		const Track* GetTrack(uint32_t i) const;
		Track* GetTrack(uint32_t i);
		void AddTrack(Track* track);
		bool RemoveTrack(uint32_t i);
		uint32_t GetNumTracks() const { return (uint32_t)this->trackArray->size(); }

	protected:
		FormatType formatType;
		Timing timing;
		std::vector<Track*>* trackArray;
	};
}