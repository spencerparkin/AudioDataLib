#include "AudioDataLib/FileDatas/MidiData.h"
#include "AudioDataLib/FileFormats/MidiFileFormat.h"
#include "AudioDataLib/Error.h"

using namespace AudioDataLib;

//------------------------------- MidiData -------------------------------

MidiData::MidiData()
{
	this->formatType = FormatType::MULTI_TRACK;
	this->timing.ticksPerQuarterNote = 48;
	this->timing.type = Timing::Type::TICKS_PER_QUARTER_NOTE;
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

/*virtual*/ FileData* MidiData::Clone() const
{
	return nullptr;
}

/*virtual*/ void MidiData::DumpInfo(FILE* fp) const
{
	const char* formatTypeStr = nullptr;

	switch (this->formatType)
	{
	case FormatType::SINGLE_TRACK:
		formatTypeStr = "Single-track";
		break;
	case FormatType::MULTI_TRACK:
		formatTypeStr = "Multi-track";
		break;
	case FormatType::VARIOUS_TRACKS:
		formatTypeStr = "Various-tracks";
		break;
	default:
		formatTypeStr = "?";
		break;
	}

	const char* timingTypeStr = nullptr;

	switch (this->timing.type)
	{
	case Timing::Type::FRAMES_PER_SECOND:
		timingTypeStr = "FPS";
		break;
	case Timing::Type::TICKS_PER_QUARTER_NOTE:
		timingTypeStr = "TPQ";
		break;
	default:
		timingTypeStr = "?";
		break;
	}

	if (this->formatType == FormatType::MULTI_TRACK)
	{
		const Track* infoTrack = this->GetTrack(0);
		if (infoTrack)
		{
			const MetaEvent* metaEvent = infoTrack->FindMetaEventOfType(MetaEvent::Type::COPYRIGHT_NOTICE);
			if (metaEvent)
				fprintf(fp, "Copyright notice: %s\n", metaEvent->GetData<const char>());

			metaEvent = infoTrack->FindMetaEventOfType(MetaEvent::Type::KEY_SIGNATURE);
			if (metaEvent)
			{
				std::string keySigStr = *metaEvent->GetData<MetaEvent::KeySignature>();
				fprintf(fp, "Key signature: %s\n", keySigStr.c_str());
			}

			metaEvent = infoTrack->FindMetaEventOfType(MetaEvent::Type::TIME_SIGNATURE);
			if (metaEvent)
			{
				std::string timeSigStr = *metaEvent->GetData<MetaEvent::TimeSignature>();
				fprintf(fp, "Time signature: %s\n", timeSigStr.c_str());
			}
		}
	}

	fprintf(fp, "Format type: %s\n", formatTypeStr);
	fprintf(fp, "Num. tracks: %d\n", uint32_t(this->trackArray->size()));
	fprintf(fp, "Timing type: %s\n", timingTypeStr);

	if (this->timing.type == Timing::Type::FRAMES_PER_SECOND)
	{
		fprintf(fp, "Frame-rate: %d\n", this->timing.framesPerSecond);
		fprintf(fp, "Ticks-per-frame: %d\n", this->timing.ticksPerFrame);
	}
	else if (this->timing.type == Timing::Type::TICKS_PER_QUARTER_NOTE)
		fprintf(fp, "Ticks-per-QN: %d\n", this->timing.ticksPerQuarterNote);

	for (uint32_t i = 0; i < this->trackArray->size(); i++)
	{
		const Track* track = this->GetTrack(i);
		fprintf(fp, "-----------------------------------\n");
		fprintf(fp, "Track %d has %d events.\n", i, uint32_t(track->GetEventArray().size()));

		const MetaEvent* metaEvent = track->FindMetaEventOfType(MetaEvent::Type::INSTRUMENT_NAME);
		if (metaEvent)
			fprintf(fp, "Instruction name: %s\n", metaEvent->GetData<const char>());

		if (this->formatType != FormatType::MULTI_TRACK || i > 0)
		{
			Error error;
			double trackTimeSeconds = 0.0;
			this->CalculateTrackLengthInSeconds(i, trackTimeSeconds, error);
			fprintf(fp, "Track time: %f\n", trackTimeSeconds);
		}
	}
}

/*virtual*/ void MidiData::DumpCSV(FILE* fp) const
{
}

const MidiData::Track* MidiData::GetTrack(uint32_t i) const
{
	return const_cast<MidiData*>(this)->GetTrack(i);
}

MidiData::Track* MidiData::GetTrack(uint32_t i)
{
	if (0 <= i && i < this->GetNumTracks())
		return (*this->trackArray)[i];

	return nullptr;
}

void MidiData::AddTrack(Track* track)
{
	this->trackArray->push_back(track);
}

bool MidiData::RemoveTrack(uint32_t i)
{
	if (0 <= i && i < this->GetNumTracks())
	{
		Track* track = (*this->trackArray)[i];
		delete track;
		if (i != this->GetNumTracks() - 1)
			(*this->trackArray)[i] = (*this->trackArray)[this->GetNumTracks() - 1];
		this->trackArray->pop_back();
		return true;
	}

	return false;
}

bool MidiData::CalculateTrackLengthInSeconds(uint32_t i, double& totalTimeSeconds, Error& error) const
{
	totalTimeSeconds = 0.0;

	const Track* track = this->GetTrack(i);
	if (!track)
	{
		error.Add(FormatString("Track (%d) not found.", i));
		return false;
	}
	
	MetaEvent::TimeSignature currentTimeSig;	// TODO: Do we not need this at all here or in a MIDI play-back machine?
	currentTimeSig.numerator = 4;
	currentTimeSig.denominator = 2;
	currentTimeSig.metro = 24;
	currentTimeSig.__32nds = 8;

	MetaEvent::Tempo currentTempo;
	currentTempo.microsecondsPerQuarterNote = 500000;

	if (this->formatType == FormatType::MULTI_TRACK)
	{
		if (i == 0)
		{
			error.Add("Doesn't make sense to measure length of the info track.");
			return false;
		}

		const Track* infoTrack = this->GetTrack(0);
		if (!infoTrack)
		{
			error.Add("Info track not found.");
			return false;
		}

		const MetaEvent* tempoEvent = infoTrack->FindMetaEventOfType(MetaEvent::Type::SET_TEMPO);
		if (tempoEvent)
			currentTempo = *tempoEvent->GetData<MetaEvent::Tempo>();

		const MetaEvent* timeSigEvent = infoTrack->FindMetaEventOfType(MetaEvent::Type::TIME_SIGNATURE);
		if (timeSigEvent)
			currentTimeSig = *timeSigEvent->GetData<MetaEvent::TimeSignature>();
	}

	for (const Event* event : track->GetEventArray())
	{
		const MetaEvent* metaEvent = dynamic_cast<const MetaEvent*>(event);
		const ChannelEvent* channelEvent = dynamic_cast<const ChannelEvent*>(event);

		if (metaEvent)
		{
			switch (metaEvent->type)
			{
				case MetaEvent::Type::SET_TEMPO:
				{
					currentTempo = *metaEvent->GetData<MetaEvent::Tempo>();
					break;
				}
				case MetaEvent::Type::TIME_SIGNATURE:
				{
					currentTimeSig = *metaEvent->GetData<MetaEvent::TimeSignature>();
					break;
				}
                default:
                {
                    break;
                }
			}
		}
		else if (channelEvent)
		{
			double deltaTimeSeconds = 0.0;
			double microsecondsPerTick = double(currentTempo.microsecondsPerQuarterNote) / double(this->timing.ticksPerQuarterNote);

			if (this->timing.type == Timing::Type::TICKS_PER_QUARTER_NOTE)
			{
				double deltaTimeMicroseconds = double(channelEvent->deltaTimeTicks) * microsecondsPerTick;
				double microsecondsPerSecond = 1000000.0;
				deltaTimeSeconds = deltaTimeMicroseconds / microsecondsPerSecond;
			}
			else if (this->timing.type == Timing::FRAMES_PER_SECOND)
			{
				error.Add("Calculating time with FPS is not yet supported.");
				return false;
			}

			totalTimeSeconds += deltaTimeSeconds;
		}
	}

	return true;
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

const MidiData::Event* MidiData::Track::GetEvent(uint32_t i) const
{
	if (i >= this->eventArray->size())
		return nullptr;

	return (*this->eventArray)[i];
}

//------------------------------- MidiData::Event -------------------------------

MidiData::Event::Event()
{
	this->deltaTimeTicks = 0;
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

/*virtual*/ bool MidiData::SystemExclusiveEvent::Decode(ByteStream& inputStream, Error& error)
{
	return false;
}

/*virtual*/ bool MidiData::SystemExclusiveEvent::Encode(ByteStream& outputStream, Error& error) const
{
	return false;
}

/*virtual*/ std::string MidiData::SystemExclusiveEvent::LogMessage() const
{
	return "?";
}

//------------------------------- MidiData::MetaEvent -------------------------------

MidiData::MetaEvent::MetaEvent()
{
	this->type = Type::UNKNOWN;
	this->data = nullptr;
}

MidiData::MetaEvent::KeySignature::operator std::string() const
{
	std::string keySigStr;

	switch (this->key)
	{
		// TODO: "key" can be -7 to +7.  This is the number of flats/sharps.  But what's the letter?
	case 0:
		keySigStr = "C";
		break;
	default:
		keySigStr = "?";
		break;
	}

	if (this->scale == 0)
		keySigStr += " major";
	else
		keySigStr += " minor";

	return keySigStr;
}

MidiData::MetaEvent::TimeSignature::operator std::string() const
{
	std::string timeSigStr;
	uint32_t actualDenom = 1 << this->denominator;
	char timeSigBuf[64];
	sprintf(timeSigBuf, "%d / %d", this->numerator, actualDenom);
	timeSigStr = timeSigBuf;
	return timeSigStr;
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
			auto tempo = static_cast<Tempo*>(this->data);
			delete tempo;
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
        default:
        {
            break;
        }
	}
}

/*virtual*/ bool MidiData::MetaEvent::Decode(ByteStream& inputStream, Error& error)
{
	if (this->type != Type::UNKNOWN || this->data != nullptr)
	{
		error.Add("Meta-event already decoded.");
		return false;
	}

	uint8_t eventType = 0;
	if (1 != inputStream.ReadBytesFromStream(&eventType, 1))
	{
		error.Add("Could not read event type byte.");
		return false;
	}

	if (eventType != 0xFF)
	{
		error.Add("Can't decode meta-event if type is not 0xFF.");
		return false;
	}

	uint8_t subTypeByte = 0;
	if (1 != inputStream.ReadBytesFromStream(&subTypeByte, 1))
	{
		error.Add("Could not read type byte.");
		return false;
	}

	this->type = (Type)subTypeByte;

	uint64_t dataLength = 0;
	if (!MidiFileFormat::DecodeVariableLengthValue(dataLength, inputStream, error))
		return false;

	switch (this->type)
	{
		case Type::SEQUENCE_NUMBER:
		{
			if (dataLength != 2)
			{
				error.Add("Expected length 2 for sequence number type.");
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
				error.Add("Failed to read text from meta event.");
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
				error.Add("Expected 1 bytes for channel prefix.");
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
				error.Add("Expected 0 bytes for channel prefix.");
				return false;
			}

			break;
		}
		case Type::SET_TEMPO:
		{
			if (dataLength != 3)
			{
				error.Add("Expected 3 bytes for set tempo meta-event.");
				return false;
			}

			uint8_t msb = 0, nmsb = 0, lsb = 0;

			if (1 != inputStream.ReadBytesFromStream((uint8_t*)&msb, 1))
				return false;

			if (1 != inputStream.ReadBytesFromStream((uint8_t*)&nmsb, 1))
				return false;

			if (1 != inputStream.ReadBytesFromStream((uint8_t*)&lsb, 1))
				return false;

			auto tempo = new Tempo;
			tempo->microsecondsPerQuarterNote = (uint32_t(msb) << 16) | (uint32_t(nmsb) << 8) | uint32_t(lsb);
			this->data = tempo;

			break;
		}
		case Type::SMPTE_OFFSET:
		{
			if (dataLength != 5)
			{
				error.Add("Expected 5 bytes for SMPTE offset meta-event.");
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
				error.Add("Expected 4 bytes for time signature meta-event.");
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
				error.Add("Expected 2 bytes for key signature meta-events.");
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
				error.Add("Got zero data size for manufacturer-specific data meta-event.");
				return false;
			}

			auto opaque = new Opaque;
			this->data = opaque;

			opaque->bufferSize = dataLength;
			opaque->buffer = new uint8_t[(uint32_t)dataLength];
			if (dataLength != inputStream.ReadBytesFromStream(opaque->buffer, dataLength))
			{
				error.Add("Failed to read manufacturer-specific data for meta-event.");
				return false;
			}

			break;
		}
        default:
        {
            break;
        }
	}

	return true;
}

/*virtual*/ bool MidiData::MetaEvent::Encode(ByteStream& outputStream, Error& error) const
{
	uint8_t eventType = 0xFF;
	if (1 != outputStream.WriteBytesToStream((const uint8_t*)&eventType, 1))
	{
		error.Add("Could not write meta-event type.");
		return false;
	}

	uint8_t subEventType = uint8_t(this->type);
	if (1 != outputStream.WriteBytesToStream((const uint8_t*)&subEventType, 1))
	{
		error.Add("Could not write meta-event sub-type.");
		return false;
	}

	bool successfulEncoding = false;
	switch (this->type)
	{
		case Type::SEQUENCE_NUMBER:
		{
			uint64_t dataLength = 2;
			if (!MidiFileFormat::EncodeVariableLengthValue(dataLength, outputStream, error))
				break;

			auto sequenceNumber = static_cast<const SequenceNumber*>(this->data);

			if (1 != outputStream.WriteBytesToStream(&sequenceNumber->msb, 1))
				break;

			if (1 != outputStream.WriteBytesToStream(&sequenceNumber->lsb, 1))
				break;

			successfulEncoding = true;
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
			auto text = static_cast<const Text*>(this->data);

			uint64_t dataLength = ::strlen(text->buffer);
			if (!MidiFileFormat::EncodeVariableLengthValue(dataLength, outputStream, error))
				break;

			if (dataLength != outputStream.WriteBytesToStream((const uint8_t*)text->buffer, dataLength))
			{
				error.Add("Failed to write text from meta event.");
				break;
			}

			successfulEncoding = true;
			break;
		}
		case Type::CHANNEL_PREFIX:
		{
			uint64_t dataLength = 1;
			if (!MidiFileFormat::EncodeVariableLengthValue(dataLength, outputStream, error))
				break;

			auto channelPrefix = static_cast<const ChannelPrefix*>(this->data);

			if (1 != outputStream.WriteBytesToStream((const uint8_t*)&channelPrefix->channel, 1))
				break;

			successfulEncoding = true;
			break;
		}
		case Type::END_OF_TRACK:
		{
			if (!MidiFileFormat::EncodeVariableLengthValue(0, outputStream, error))
				break;

			successfulEncoding = true;
			break;
		}
		case Type::SET_TEMPO:
		{
			uint64_t dataLength = 3;
			if (!MidiFileFormat::EncodeVariableLengthValue(dataLength, outputStream, error))
				break;

			auto tempo = static_cast<const Tempo*>(this->data);

			uint8_t msb = uint8_t((tempo->microsecondsPerQuarterNote >> 16) & 0xFF);
			uint8_t nmsb = uint8_t((tempo->microsecondsPerQuarterNote >> 8) & 0xFF);
			uint8_t lsb = uint8_t(tempo->microsecondsPerQuarterNote & 0xFF);

			if (1 != outputStream.WriteBytesToStream((const uint8_t*)&msb, 1))
				break;

			if (1 != outputStream.WriteBytesToStream((const uint8_t*)&nmsb, 1))
				break;

			if (1 != outputStream.WriteBytesToStream((const uint8_t*)&lsb, 1))
				break;

			successfulEncoding = true;
			break;
		}
		case Type::SMPTE_OFFSET:
		{
			uint64_t dataLength = 5;
			if (!MidiFileFormat::EncodeVariableLengthValue(dataLength, outputStream, error))
				break;

			auto offset = static_cast<const SMPTEOffset*>(this->data);

			if (1 != outputStream.WriteBytesToStream(&offset->hours, 1))
				break;

			if (1 != outputStream.WriteBytesToStream(&offset->minutes, 1))
				break;

			if (1 != outputStream.WriteBytesToStream(&offset->seconds, 1))
				break;

			if (1 != outputStream.WriteBytesToStream(&offset->frames, 1))
				break;

			if (1 != outputStream.WriteBytesToStream(&offset->subFrames, 1))
				break;

			successfulEncoding = true;
			break;
		}
		case Type::TIME_SIGNATURE:
		{
			uint64_t dataLength = 4;
			if (!MidiFileFormat::EncodeVariableLengthValue(dataLength, outputStream, error))
				break;

			auto timeSignature = static_cast<const TimeSignature*>(this->data);

			if (1 != outputStream.WriteBytesToStream(&timeSignature->numerator, 1))
				break;

			if (1 != outputStream.WriteBytesToStream(&timeSignature->denominator, 1))
				break;

			if (1 != outputStream.WriteBytesToStream(&timeSignature->metro, 1))
				break;

			if (1 != outputStream.WriteBytesToStream(&timeSignature->__32nds, 1))
				break;

			successfulEncoding = true;
			break;
		}
		case Type::KEY_SIGNATURE:
		{
			uint64_t dataLength = 2;
			if (!MidiFileFormat::EncodeVariableLengthValue(dataLength, outputStream, error))
				break;

			auto keySignature = static_cast<const KeySignature*>(this->data);

			if (1 != outputStream.WriteBytesToStream(&keySignature->key, 1))
				break;

			if (1 != outputStream.WriteBytesToStream(&keySignature->scale, 1))
				break;

			successfulEncoding = true;
			break;
		}
		case Type::SEQUENCER_SPECIFIC:
		{
			auto opaque = static_cast<const Opaque*>(this->data);

			uint64_t dataLength = opaque->bufferSize;
			if (dataLength == 0)
			{
				error.Add("Got zero data size for manufacturer-specific data meta-event.");
				break;
			}

			if (!MidiFileFormat::EncodeVariableLengthValue(dataLength, outputStream, error))
				break;

			if (dataLength != outputStream.WriteBytesToStream(opaque->buffer, dataLength))
			{
				error.Add("Failed to write manufacturer-specific data for meta-event.");
				break;
			}

			successfulEncoding = true;
			break;
		}
		default:
		{
			break;
		}
	}

	if (!successfulEncoding)
	{
		error.Add("Failed to encode meta-event.");
		return false;
	}

	return true;
}

/*virtual*/ std::string MidiData::MetaEvent::LogMessage() const
{
	return "?";
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

/*virtual*/ bool MidiData::ChannelEvent::Decode(ByteStream& inputStream, Error& error)
{
	if (this->type != Type::UNKNOWN)
	{
		error.Add("Channel event already decoded.");
		return false;
	}

	uint8_t eventType = 0;
	if (1 != inputStream.ReadBytesFromStream(&eventType, 1))
	{
		error.Add("Could not read event-type/channel byte.");
		return false;
	}

	this->type = Type(eventType >> 4);
	this->channel = (eventType & 0x0F);

	if (1 != inputStream.ReadBytesFromStream(&this->param1, 1))
	{
		error.Add("Could not read param 1.");
		return false;
	}

	if (this->type != Type::PROGRAM_CHANGE && this->type != Type::CHANNEL_AFTERTOUCH)
	{
		if (1 != inputStream.ReadBytesFromStream(&this->param2, 1))
		{
			error.Add("Could not read param 2.");
			return false;
		}
	}

	return true;
}

/*virtual*/ bool MidiData::ChannelEvent::Encode(ByteStream& outputStream, Error& error) const
{
	if (this->type == Type::UNKNOWN)
	{
		error.Add("Can't encode unknown channel event type.");
		return false;
	}

	uint8_t eventType = (uint8_t(this->type) << 4) | this->channel;
	if (1 != outputStream.WriteBytesToStream((const uint8_t*)&eventType, 1))
	{
		error.Add("Couldn't write event-type/channel byte.");
		return false;
	}

	if (1 != outputStream.WriteBytesToStream(&this->param1, 1))
	{
		error.Add("Could not write param 1.");
		return false;
	}

	if (this->type != Type::PROGRAM_CHANGE && this->type != Type::CHANNEL_AFTERTOUCH)
	{
		if (1 != outputStream.WriteBytesToStream(&this->param2, 1))
		{
			error.Add("Could not write param 2.");
			return false;
		}
	}

	return true;
}

/*virtual*/ std::string MidiData::ChannelEvent::LogMessage() const
{
	const char* typeStr = nullptr;

	switch (this->type)
	{
		case Type::NOTE_OFF: typeStr = "NOTE_OFF"; break;
		case Type::NOTE_ON: typeStr = "NOTE_ON"; break;
		case Type::NOTE_AFTERTOUCH: typeStr = "NOTE_AFTERTOUCH"; break;
		case Type::CONTROLLER: typeStr = "CONTROLLER"; break;
		case Type::PROGRAM_CHANGE: typeStr = "PROGRAM_CHANGE"; break;
		case Type::CHANNEL_AFTERTOUCH: typeStr = "CHANNEL_AFTERTOUCH"; break;
		case Type::PITCH_BEND: typeStr = "PITCH_BEND"; break;
		case Type::UNKNOWN: typeStr = "UNKNOWN"; break;
	}

	return FormatString("%s: %d, %d (channel: %d)", typeStr, this->param1, this->param2, this->channel);
}