#include "MidiData.h"
#include "MidiFileFormat.h"

using namespace AudioDataLib;

//------------------------------- MidiData -------------------------------

MidiData::MidiData()
{
	this->formatType = FormatType::MULTI_TRACK;
	::memset(&this->timing, 0, sizeof(this->timing));
	this->trackArray = new std::vector<Track*>();
}

/*virtual*/ MidiData::~MidiData()
{
	this->Clear();

	delete this->trackArray;
}

/*static*/ MidiData* MidiData::Create()
{
	return new MidiData();
}

/*static*/ void MidiData::Destroy(MidiData* midiData)
{
	delete midiData;
}

void MidiData::Clear()
{
	for (Track* track : *this->trackArray)
		delete track;

	this->trackArray->clear();
}

//------------------------------- MidiData::Event -------------------------------

MidiData::Track::Track()
{
	this->eventArray = new std::vector<Event*>();
}

/*virtual*/ MidiData::Track::~Track()
{
	this->Clear();

	delete this->eventArray;
}

void MidiData::Track::Clear()
{
	for (Event* event : *this->eventArray)
		delete event;

	this->eventArray->clear();
}

//------------------------------- MidiData::Event -------------------------------

MidiData::Event::Event()
{
	this->deltaTime = 0;
}

/*virtual*/ MidiData::Event::~Event()
{
}

//------------------------------- MidiData::SystemExclusiveEvent -------------------------------

MidiData::SystemExclusiveEvent::SystemExclusiveEvent()
{
}

/*virtual*/ MidiData::SystemExclusiveEvent::~SystemExclusiveEvent()
{
}

/*virtual*/ bool MidiData::SystemExclusiveEvent::Decode(ByteStream& inputStream, std::string& error)
{
	return false;
}

/*virtual*/ bool MidiData::SystemExclusiveEvent::Encode(ByteStream& outputStream, std::string& error) const
{
	return false;
}

//------------------------------- MidiData::MetaEvent -------------------------------

MidiData::MetaEvent::MetaEvent()
{
	this->type = Type::UNKNOWN;
	this->data = nullptr;
}

/*virtual*/ MidiData::MetaEvent::~MetaEvent()
{
	switch (this->type)
	{
		case Type::SEQUENCE_NUMBER:
		{
			auto sequenceNumber = static_cast<SequenceNumber*>(this->data);
			delete sequenceNumber;
			break;
		}
		case Type::TEXT_EVENT:
		case Type::COPYRIGHT_NOTICE:
		case Type::TRACK_NAME:
		case Type::INSTRUMENT_NAME:
		case Type::LYRICS:
		case Type::MARKER:
		case Type::CUE_POINT:
		{
			auto text = static_cast<Text*>(this->data);
			delete[] text->buffer;
			delete text;
			break;
		}
		case Type::CHANNEL_PREFIX:
		{
			auto channelPrefix = static_cast<ChannelPrefix*>(this->data);
			delete channelPrefix;
			break;
		}
		case Type::SET_TEMPO:
		{
			auto setTempo = static_cast<SetTempo*>(this->data);
			delete setTempo;
			break;
		}
		case Type::SMPTE_OFFSET:
		{
			auto offset = static_cast<SMPTEOffset*>(this->data);
			delete offset;
			break;
		}
		case Type::TIME_SIGNATURE:
		{
			auto timeSignature = static_cast<TimeSignature*>(this->data);
			delete timeSignature;
			break;
		}
		case Type::KEY_SIGNATURE:
		{
			auto keySignature = static_cast<KeySignature*>(this->data);
			delete keySignature;
			break;
		}
		case Type::SEQUENCER_SPECIFIC:
		{
			auto opaque = static_cast<Opaque*>(this->data);
			delete[] opaque->buffer;
			delete opaque;
			break;
		}
	}
}

/*virtual*/ bool MidiData::MetaEvent::Decode(ByteStream& inputStream, std::string& error)
{
	if (this->type != Type::UNKNOWN || this->data != nullptr)
	{
		error = "Meta-event already decoded.";
		return false;
	}

	uint8_t eventType = 0;
	if (1 != inputStream.ReadBytesFromStream(&eventType, 1))
	{
		error = "Could not read event type byte.";
		return false;
	}

	if (eventType != 0xFF)
	{
		error = "Can't decode meta-event if type is not 0xFF.";
		return false;
	}

	uint8_t subTypeByte = 0;
	if (1 != inputStream.ReadBytesFromStream(&subTypeByte, 1))
	{
		error = "Could not read type byte.";
		return false;
	}

	this->type = (Type)subTypeByte;

	uint64_t dataLength = 0;
	if (!MidiFileFormat::DecodeVariableLengthValue(dataLength, inputStream, error))
		return false;

	switch (type)
	{
		case Type::SEQUENCE_NUMBER:
		{
			if (dataLength != 2)
			{
				error = "Expected length 2 for sequence number type.";
				return false;
			}

			auto sequenceNumber = new SequenceNumber;
			this->data = sequenceNumber;

			if (1 != inputStream.ReadBytesFromStream(&sequenceNumber->msb, 1))
				return false;

			if (1 != inputStream.ReadBytesFromStream(&sequenceNumber->lsb, 1))
				return false;

			break;
		}
		case Type::TEXT_EVENT:
		case Type::COPYRIGHT_NOTICE:
		case Type::TRACK_NAME:
		case Type::INSTRUMENT_NAME:
		case Type::LYRICS:
		case Type::MARKER:
		case Type::CUE_POINT:
		{
			auto* text = new Text;
			this->data = text;

			text->buffer = new char[(size_t)dataLength + 1];
			if (dataLength != inputStream.ReadBytesFromStream((uint8_t*)text->buffer, dataLength))
			{
				error = "Failed to read text from meta event.";
				return false;
			}

			text->buffer[uint32_t(dataLength)] = '\0';
			break;
		}
		case Type::CHANNEL_PREFIX:
		{
			auto* channelPrefix = new ChannelPrefix;
			this->data = channelPrefix;

			if (dataLength != 1)
			{
				error = "Expected 1 bytes for channel prefix.";
				return false;
			}

			if (1 != inputStream.ReadBytesFromStream((uint8_t*)&channelPrefix->channel, 1))
				return false;

			break;
		}
		case Type::END_OF_TRACK:
		{
			if (dataLength != 0)
			{
				error = "Expected 0 bytes for channel prefix.";
				return false;
			}

			break;
		}
		case Type::SET_TEMPO:
		{
			if (dataLength != 3)
			{
				error = "Expected 3 bytes for set tempo meta-event.";
				return false;
			}

			auto setTempo = new SetTempo;
			this->data = setTempo;
			if (3 != inputStream.ReadBytesFromStream((uint8_t*)&setTempo->microsecondsPerQuarterNote, 3))
				return false;

			// TODO: I think this value is still wrong.
			setTempo->microsecondsPerQuarterNote &= 0x00FFFFFF;

			break;
		}
		case Type::SMPTE_OFFSET:
		{
			if (dataLength != 5)
			{
				error = "Expected 5 bytes for SMPTE offset meta-event.";
				return false;
			}

			auto offset = new SMPTEOffset;
			this->data = offset;
			
			if (1 != inputStream.ReadBytesFromStream(&offset->hours, 1))
				return false;

			if (1 != inputStream.ReadBytesFromStream(&offset->minutes, 1))
				return false;

			if (1 != inputStream.ReadBytesFromStream(&offset->seconds, 1))
				return false;

			if (1 != inputStream.ReadBytesFromStream(&offset->frames, 1))
				return false;

			if (1 != inputStream.ReadBytesFromStream(&offset->subFrames, 1))
				return false;

			break;
		}
		case Type::TIME_SIGNATURE:
		{
			if (dataLength != 4)
			{
				error = "Expected 4 bytes for time signature meta-event.";
				return false;
			}

			auto timeSignature = new TimeSignature;
			this->data = timeSignature;

			if (1 != inputStream.ReadBytesFromStream(&timeSignature->numerator, 1))
				return false;

			if (1 != inputStream.ReadBytesFromStream(&timeSignature->denominator, 1))
				return false;

			if (1 != inputStream.ReadBytesFromStream(&timeSignature->metro, 1))
				return false;

			if (1 != inputStream.ReadBytesFromStream(&timeSignature->__32nds, 1))
				return false;

			break;
		}
		case Type::KEY_SIGNATURE:
		{
			if (dataLength != 2)
			{
				error = "Expected 2 bytes for key signature meta-events.";
				return false;
			}

			auto keySignature = new KeySignature;
			this->data = keySignature;

			if (1 != inputStream.ReadBytesFromStream(&keySignature->key, 1))
				return false;

			if (1 != inputStream.ReadBytesFromStream(&keySignature->scale, 1))
				return false;

			break;
		}
		case Type::SEQUENCER_SPECIFIC:
		{
			if (dataLength == 0)
			{
				error = "Got zero data size for manufacturer-specific data meta-event.";
				return false;
			}

			auto opaque = new Opaque;
			this->data = opaque;

			opaque->bufferSize = dataLength;
			opaque->buffer = new uint8_t[(uint32_t)dataLength];
			if (dataLength != inputStream.ReadBytesFromStream(opaque->buffer, dataLength))
			{
				error = "Failed to read manufacturer-specific data for meta-event.";
				return false;
			}

			break;
		}
	}

	return true;
}

/*virtual*/ bool MidiData::MetaEvent::Encode(ByteStream& outputStream, std::string& error) const
{
	return false;
}

//------------------------------- MidiData::ChannelEvent -------------------------------

MidiData::ChannelEvent::ChannelEvent()
{
	this->type = Type::UNKNOWN;
	this->channel = 0;
	this->param1 = 0;
	this->param2 = 0;
}

/*virtual*/ MidiData::ChannelEvent::~ChannelEvent()
{
}

/*virtual*/ bool MidiData::ChannelEvent::Decode(ByteStream& inputStream, std::string& error)
{
	if (this->type != Type::UNKNOWN)
	{
		error = "Channel event already decoded.";
		return false;
	}

	uint8_t eventType = 0;
	if (1 != inputStream.ReadBytesFromStream(&eventType, 1))
	{
		error = "Could not read channel byte/nibble.";
		return false;
	}

	this->type = Type(eventType >> 4);
	this->channel = (eventType & 0x0F);

	if (1 != inputStream.ReadBytesFromStream(&this->param1, 1))
	{
		error = "Could not read param 1.";
		return false;
	}

	if (this->type != Type::PROGRAM_CHANGE && this->type != Type::CHANNEL_AFTERTOUCH)
	{
		if (1 != inputStream.ReadBytesFromStream(&this->param2, 1))
		{
			error = "Could not read param 2.";
			return false;
		}
	}

	return true;
}

/*virtual*/ bool MidiData::ChannelEvent::Encode(ByteStream& outputStream, std::string& error) const
{
	return false;
}