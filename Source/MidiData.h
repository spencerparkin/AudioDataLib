#pragma once

#include "FileData.h"
#include "ByteStream.h"

namespace AudioDataLib
{
	class Error;

	class AUDIO_DATA_LIB_API MidiData : public FileData
	{
		friend class MidiFileFormat;

	public:
		MidiData();
		virtual ~MidiData();

		virtual void DumpInfo(FILE* fp) const override;

		void Clear();

		bool CalculateTrackLengthInSeconds(uint32_t i, double& totalTimeSeconds, Error& error) const;

		static MidiData* Create();
		static void Destroy(MidiData* midiData);

		enum FormatType
		{
			SINGLE_TRACK = 0,
			MULTI_TRACK = 1,
			VARIOUS_TRACKS = 2
		};

		struct Timing
		{
			union
			{
				uint32_t ticksPerQuarterNote;
				struct
				{
					int16_t framesPerSecond;
					uint16_t ticksPerFrame;
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

		class AUDIO_DATA_LIB_API Track
		{
		public:
			Track();
			virtual ~Track();

			void Clear();

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

			const MetaEvent* FindMetaEventOfType(uint8_t type) const
			{
				return this->FindEvent<MetaEvent>([=](MetaEvent* event) -> bool { return event->type == type; });
			}

			const Event* GetEvent(uint32_t i) const;
			const std::vector<Event*>& GetEventArray() const { return *this->eventArray; }
			void AddEvent(Event* event) { this->eventArray->push_back(event); }

		private:
			std::vector<Event*>* eventArray;
		};

		// Encoding/decoding of events seems like it's something that should only
		// be part of the MidiFileFormat class, but we may need to encode an event
		// before we send it down to the MIDI device on a MIDI port for synthesis,
		// and similarly, we also need to be able to decode a MIDI message that we
		// receive from a MIDI port for storage.

		class AUDIO_DATA_LIB_API Event
		{
		public:
			Event();
			virtual ~Event();

			virtual bool Decode(ByteStream& inputStream, Error& error) = 0;
			virtual bool Encode(ByteStream& outputStream, Error& error) const = 0;

			virtual std::string LogMessage() const = 0;

			uint64_t deltaTimeTicks;
		};

		class AUDIO_DATA_LIB_API SystemExclusiveEvent : public Event
		{
		public:
			SystemExclusiveEvent();
			virtual ~SystemExclusiveEvent();

			virtual bool Decode(ByteStream& inputStream, Error& error) override;
			virtual bool Encode(ByteStream& outputStream, Error& error) const override;

			virtual std::string LogMessage() const override;
		};

		class AUDIO_DATA_LIB_API MetaEvent : public Event
		{
		public:
			MetaEvent();
			virtual ~MetaEvent();

			virtual bool Decode(ByteStream& inputStream, Error& error) override;
			virtual bool Encode(ByteStream& outputStream, Error& error) const override;

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

			virtual bool Decode(ByteStream& inputStream, Error& error) override;
			virtual bool Encode(ByteStream& outputStream, Error& error) const override;

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