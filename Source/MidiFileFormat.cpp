#include "MidiFileFormat.h"
#include "MidiData.h"

using namespace AudioDataLib;

MidiFileFormat::MidiFileFormat()
{
}

/*virtual*/ MidiFileFormat::~MidiFileFormat()
{
}

/*virtual*/ bool MidiFileFormat::ReadFromStream(ByteStream& inputStream, FileData*& fileData, std::string& error)
{
	// See: https://github.com/colxi/midi-parser-js/wiki/MIDI-File-Format-Specifications
	//      https://majicdesigns.github.io/MD_MIDIFile/page_timing.html

	bool success = false;
	fileData = nullptr;
	MidiData* midiData = nullptr;
	
	do
	{
		ChunkParser chunkParser;
		chunkParser.byteSwapper.swapsNeeded = true;
		if (!chunkParser.ParseStream(inputStream, error))
			break;

		const ChunkParser::Chunk* headerChunk = chunkParser.FindChunk("MThd");
		if (!headerChunk)
		{
			error = "Failed to find MIDI header chunk.";
			break;
		}

		std::vector<const ChunkParser::Chunk*> trackChunkArray;
		chunkParser.FindAllChunks("MTrk", trackChunkArray);
		if (trackChunkArray.size() == 0)
		{
			error = "Didn't find any MIDI track chunks.";
			break;
		}

		midiData = new MidiData();

		BufferStream bufferStream(headerChunk->GetBuffer(), headerChunk->GetBufferSize());

		uint16_t formatType = 0;
		if (2 != bufferStream.ReadBytesFromStream((uint8_t*)&formatType, 2))
			break;

		formatType = chunkParser.byteSwapper.Resolve(formatType);
		midiData->formatType = (MidiData::FormatType)formatType;

		uint16_t numTracks = 0;
		if (2 != bufferStream.ReadBytesFromStream((uint8_t*)&numTracks, 2))
			break;

		numTracks = chunkParser.byteSwapper.Resolve(numTracks);
		if (numTracks != trackChunkArray.size())
		{
			error = "Number of tracks specified in header does not match actual number of tracks.";
			break;
		}

		uint16_t timingDivision = 0;
		if (2 != bufferStream.ReadBytesFromStream((uint8_t*)&timingDivision, 2))
			break;

		timingDivision = chunkParser.byteSwapper.Resolve(timingDivision);
		if ((timingDivision & 0x8000) == 0)
		{
			midiData->timing.type = MidiData::Timing::Type::TICKS_PER_QUARTER_NOTE;
			midiData->timing.ticksPerQuarterNote = (timingDivision & 0x7FFF);
		}
		else
		{
			midiData->timing.type = MidiData::Timing::Type::FRAMES_PER_SECOND;
			midiData->timing.framesPerSecond = (timingDivision & 0x7F00) >> 8;
			midiData->timing.ticksPerFrame = (timingDivision & 0x00FF);
		}

		bool decodeFailureOccurred = false;

		for (const ChunkParser::Chunk* chunk : trackChunkArray)
		{
			auto track = new MidiData::Track();
			midiData->trackArray->push_back(track);

			BufferStream bufferStream(chunk->GetBuffer(), chunk->GetBufferSize());
			while (bufferStream.CanRead())
			{
				MidiData::Event* event = nullptr;
				if (!DecodeEvent(bufferStream, event, error))
				{
					decodeFailureOccurred = true;
					break;
				}

				track->AddEvent(event);
			}

			if (decodeFailureOccurred)
				break;
		}

		if (decodeFailureOccurred)
			break;

		success = true;
		fileData = midiData;
	} while (false);

	if (!success)
		delete midiData;

	return success;
}

/*virtual*/ bool MidiFileFormat::WriteToStream(ByteStream& outputStream, FileData* fileData, std::string& error)
{
	return true;
}

/*static*/ bool MidiFileFormat::DecodeVariableLengthValue(uint64_t& value, ByteStream& inputStream, std::string& error)
{
	std::vector<uint8_t> componentsArray;
	uint8_t byte = 0;
	do
	{
		if (1 != inputStream.ReadBytesFromStream(&byte, 1))
		{
			error = "Failed to read byte for variable-length value.";
			return false;
		}

		componentsArray.push_back(byte & 0x7F);
	} while ((byte & 0x80) != 0);

	if (componentsArray.size() > 4)
	{
		error = "Variable-length value won't fit in 64-bits.";
		return false;
	}

	value = 0;
	uint64_t shift = 0;
	while (componentsArray.size() > 0)
	{
		uint64_t component = (uint64_t)componentsArray.back();
		componentsArray.pop_back();
		value |= (component << shift);
		shift += 7;
	}

	return true;
}

/*static*/ bool MidiFileFormat::EncodeVariableLengthValue(uint64_t value, ByteStream& outputStream, std::string& error)
{
	std::vector<uint8_t> componentsArray;
	do
	{
		uint8_t component = uint8_t(value & 0x7F);
		componentsArray.push_back(component);
		value >>= 7;
	} while (value != 0);

	while (componentsArray.size() > 0)
	{
		uint8_t component = componentsArray.back();
		componentsArray.pop_back();
		uint8_t byte = component;
		if (componentsArray.size() > 0)
			byte |= 0x80;

		if (1 != outputStream.WriteBytesToStream(&byte, 1))
		{
			error = "Failed to write byte for variable-length value.";
			return false;
		}
	}

	return true;
}

/*static*/ bool MidiFileFormat::DecodeEvent(ByteStream& inputStream, MidiData::Event*& event, std::string& error)
{
	uint64_t deltaTimeTicks = 0;
	event = nullptr;

	if (!DecodeVariableLengthValue(deltaTimeTicks, inputStream, error))
	{
		error += "  Could not decode delta-time.";
		return false;
	}

	uint8_t eventType = 0;
	if (1 != inputStream.PeekBytesFromStream(&eventType, 1))
	{
		error = "Could not peek event type.";
		return false;
	}
	
	eventType = (eventType & 0xF0) >> 4;

	if (0x8 <= eventType && eventType <= 0xE)
	{
		auto channelEvent = new MidiData::ChannelEvent();
		if (!channelEvent->Decode(inputStream, error))
		{
			delete channelEvent;
			channelEvent = nullptr;
		}

		event = channelEvent;
	}
	else if (eventType == 0xF)
	{
		inputStream.PeekBytesFromStream(&eventType, 1);

		if (eventType == 0xFF)
		{
			auto metaEvent = new MidiData::MetaEvent();
			if (!metaEvent->Decode(inputStream, error))
			{
				delete metaEvent;
				metaEvent = nullptr;
			}

			event = metaEvent;
		}
		else if (eventType == 0xF0 || eventType == 0xF7)
		{
			auto sysExEvent = new MidiData::SystemExclusiveEvent();
			if (!sysExEvent->Decode(inputStream, error))
			{
				delete sysExEvent;
				sysExEvent = nullptr;
			}

			event = sysExEvent;
		}
	}

	if (!event)
	{
		error = "Could not resolve event type.";
		return false;
	}
	
	event->deltaTimeTicks = deltaTimeTicks;
	return true;
}

/*static*/ bool MidiFileFormat::EncodeEvent(ByteStream& outputStream, const MidiData::Event* event, std::string& error)
{
	return false;
}