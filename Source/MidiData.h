#pragma once

#include "FileData.h"
#include "ByteStream.h"

namespace AudioDataLib
{
	class AUDIO_DATA_LIB_API MidiData : public FileData
	{
	public:
		MidiData();
		virtual ~MidiData();

		void Clear();

		void CalculateTrackLengthsInSeconds(std::vector<double>& trackLengthsArray) const;

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
				uint32_t ticksPerBeat;
				struct
				{
					uint16_t framesPerSecond;
					uint16_t ticksPerFrame;
				};
			};

			enum Type
			{
				TICKS_PER_BEAT,
				FRAMES_PER_SECOND
			};

			Type type;
		};

		class Event;

		class AUDIO_DATA_LIB_API Track
		{
		public:
			Track();
			virtual ~Track();

			void Clear();

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

			virtual bool Decode(ByteStream& inputStream, std::string& error) = 0;
			virtual bool Encode(ByteStream& outputStream, std::string& error) const = 0;

			uint64_t deltaTime;
		};

		class AUDIO_DATA_LIB_API SystemExclusiveEvent : public Event
		{
		public:
			SystemExclusiveEvent();
			virtual ~SystemExclusiveEvent();

			virtual bool Decode(ByteStream& inputStream, std::string& error) override;
			virtual bool Encode(ByteStream& outputStream, std::string& error) const override;
		};

		class AUDIO_DATA_LIB_API MetaEvent : public Event
		{
		public:
			MetaEvent();
			virtual ~MetaEvent();

			virtual bool Decode(ByteStream& inputStream, std::string& error) override;
			virtual bool Encode(ByteStream& outputStream, std::string& error) const override;

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

			struct SetTempo
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
			};

			struct KeySignature
			{
				uint8_t key;
				uint8_t scale;
			};

			struct Opaque
			{
				uint8_t* buffer;
				uint64_t bufferSize;
			};
		};

		class AUDIO_DATA_LIB_API ChannelEvent : public Event
		{
		public:
			ChannelEvent();
			virtual ~ChannelEvent();

			virtual bool Decode(ByteStream& inputStream, std::string& error) override;
			virtual bool Encode(ByteStream& outputStream, std::string& error) const override;

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

		FormatType formatType;
		Timing timing;
		std::vector<Track*>* trackArray;
	};
}