#include "AudioDataLib/FileFormats/MidiFileFormat.h"
#include "AudioDataLib/FileDatas/MidiData.h"
#include "AudioDataLib/Error.h"

using namespace AudioDataLib;

MidiFileFormat::MidiFileFormat()
{
}

/*virtual*/ MidiFileFormat::~MidiFileFormat()
{
}

/*virtual*/ bool MidiFileFormat::ReadFromStream(ByteStream& inputStream, FileData*& fileData, Error& error)
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
			error.Add("Failed to find MIDI header chunk.");
			break;
		}

		std::vector<const ChunkParser::Chunk*> trackChunkArray;
		chunkParser.FindAllChunks("MTrk", trackChunkArray);
		if (trackChunkArray.size() == 0)
		{
			error.Add("Didn't find any MIDI track chunks.");
			break;
		}

		midiData = new MidiData();

		ReadOnlyBufferStream bufferStream(headerChunk->GetBuffer(), headerChunk->GetBufferSize());

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
			error.Add(FormatString("Number of tracks specified in header (%d) does not match actual number of tracks (%d).", numTracks, trackChunkArray.size()));
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

			ReadOnlyBufferStream bufferStream(chunk->GetBuffer(), chunk->GetBufferSize());
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
	} while (false);

	if(success)
		fileData = midiData;
	else
		delete midiData;

	return success;
}

/*virtual*/ bool MidiFileFormat::WriteToStream(ByteStream& outputStream, const FileData* fileData, Error& error)
{
	bool success = false;
	const MidiData* midiData = nullptr;
	ByteSwapper byteSwapper;
	byteSwapper.swapsNeeded = true;

	do
	{
		midiData = dynamic_cast<const MidiData*>(fileData);
		if (!midiData)
		{
			error.Add("Can only write MIDI data to a MIDI file.");
			break;
		}

		if (4 != outputStream.WriteBytesToStream((const uint8_t*)"MThd", 4))
		{
			error.Add("Failed to write header chunk ID.");
			break;
		}

		uint32_t chunkSize = byteSwapper.Resolve(uint32_t(6));
		if (4 != outputStream.WriteBytesToStream((const uint8_t*)&chunkSize, 4))
		{
			error.Add("Failed to write header chunk size.");
			break;
		}

		uint16_t formatType = byteSwapper.Resolve(uint16_t(midiData->GetFormatType()));
		if (2 != outputStream.WriteBytesToStream((const uint8_t*)&formatType, 2))
			break;

		uint16_t numTracks = midiData->GetNumTracks();
		if (numTracks == 0)
		{
			error.Add("No MIDI tracks found in MIDI data object.");
			break;
		}

		if (numTracks != 1 && midiData->GetFormatType() == MidiData::FormatType::SINGLE_TRACK)
		{
			error.Add(FormatString("MIDI data is set to single-tracks, but multiples tracks (%d) are stored.", numTracks));
			break;
		}

		numTracks = byteSwapper.Resolve(numTracks);
		if (2 != outputStream.WriteBytesToStream((const uint8_t*)&numTracks, 2))
			break;

		uint16_t timingDivision = 0;
		const MidiData::Timing& timing = midiData->GetTiming();
		if (timing.type == MidiData::Timing::TICKS_PER_QUARTER_NOTE)
		{
			if ((timing.ticksPerQuarterNote & 0x8000) != 0)
			{
				error.Add(FormatString("Ticks per quarter note value (%d) won't fit into a 15-bit unsigned integer.", timing.ticksPerQuarterNote));
				break;
			}

			timingDivision = timing.ticksPerQuarterNote;
		}
		else if (timing.type == MidiData::Timing::FRAMES_PER_SECOND)
		{
			if ((timing.framesPerSecond & 0x80) != 0)
			{
				error.Add(FormatString("Frames per second (%d) won't fit into a 7-bit unsigned integer.", timing.framesPerSecond));
				break;
			}

			timingDivision = (uint16_t(timing.framesPerSecond) << 8) | uint16_t(timing.ticksPerFrame) | 0x8000;
		}
		else
		{
			error.Add(FormatString("Timging type (%d) unknown.", timing.type));
			break;
		}

		timingDivision = byteSwapper.Resolve(timingDivision);
		if (2 != outputStream.WriteBytesToStream((const uint8_t*)&timingDivision, 2))
		{
			error.Add("Could not write timing division.");
			break;
		}

		for (uint32_t i = 0; i < midiData->GetNumTracks(); i++)
		{
			const MidiData::Track* track = midiData->GetTrack(i);

			MemoryStream memoryStream;
			const std::vector<MidiData::Event*>& eventArray = track->GetEventArray();
			for (const MidiData::Event* event : eventArray)
			{
				if (!EncodeEvent(memoryStream, event, error))
				{
					error.Add("Failed to encode track event.");
					break;
				}
			}

			if (error)
				break;

			if (4 != outputStream.WriteBytesToStream((const uint8_t*)"MTrk", 4))
			{
				error.Add("Could not write track chunk header ID.");
				break;
			}

			uint64_t trackChunkSize64 = memoryStream.GetSize();
			if ((trackChunkSize64 & 0xFFFFFFFF00000000) != 0)
			{
				error.Add(FormatString("Track chunk size (%lld) won't fit into 32-bit unsigned integer.", trackChunkSize64));
				break;
			}

			uint32_t trackChunkSize = byteSwapper.Resolve(uint32_t(trackChunkSize64));
			if (4 != outputStream.WriteBytesToStream((const uint8_t*)&trackChunkSize, 4))
			{
				error.Add("Could not write track chunk size.");
				break;
			}

			while (memoryStream.GetSize() > 0)
			{
				uint8_t byte = 0;
				if (1 != memoryStream.ReadBytesFromStream(&byte, 1))
				{
					error.Add("Failed to read track byte from stream.");
					break;
				}

				if (1 != outputStream.WriteBytesToStream(&byte, 1))
				{
					error.Add("Failed to write track byte to stream.");
					break;
				}
			}

			if (error)
				break;
		}

		if (error)
			break;

		success = true;
	} while (false);

	return success;
}

/*static*/ bool MidiFileFormat::DecodeVariableLengthValue(uint64_t& value, ByteStream& inputStream, Error& error)
{
	std::vector<uint8_t> componentsArray;
	uint8_t byte = 0;
	do
	{
		if (1 != inputStream.ReadBytesFromStream(&byte, 1))
		{
			error.Add("Failed to read byte for variable-length value.");
			return false;
		}

		componentsArray.push_back(byte & 0x7F);
	} while ((byte & 0x80) != 0);

	if (componentsArray.size() > 4)
	{
		error.Add("Variable-length value won't fit in 64-bits.");
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

/*static*/ bool MidiFileFormat::EncodeVariableLengthValue(uint64_t value, ByteStream& outputStream, Error& error)
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
			error.Add("Failed to write byte for variable-length value.");
			return false;
		}
	}

	return true;
}

/*static*/ bool MidiFileFormat::DecodeEvent(ByteStream& inputStream, MidiData::Event*& event, Error& error)
{
	uint64_t deltaTimeTicks = 0;
	event = nullptr;

	if (!DecodeVariableLengthValue(deltaTimeTicks, inputStream, error))
	{
		error.Add("Could not decode delta-time.");
		return false;
	}

	uint8_t eventType = 0;
	if (1 != inputStream.PeekBytesFromStream(&eventType, 1))
	{
		error.Add("Could not peek event type.");
		return false;
	}
	
	eventType = (eventType & 0xF0) >> 4;

	if (0x8 <= eventType && eventType <= 0xE)
		event = new MidiData::ChannelEvent();
	else if (eventType == 0xF)
	{
		inputStream.PeekBytesFromStream(&eventType, 1);

		if (eventType == 0xFF)
			event = new MidiData::MetaEvent();
		else if (eventType == 0xF0 || eventType == 0xF7)
			event = new MidiData::SystemExclusiveEvent();
	}

	if (!event)
	{
		error.Add(FormatString("Could not resolve event type %d.", eventType));
		return false;
	}

	if (!event->Decode(inputStream, error))
	{
		error.Add("Failed to decode event type.");
		delete event;
		event = nullptr;
		return false;
	}
	
	event->deltaTimeTicks = deltaTimeTicks;
	return true;
}

/*static*/ bool MidiFileFormat::EncodeEvent(ByteStream& outputStream, const MidiData::Event* event, Error& error)
{
	if (!EncodeVariableLengthValue(event->deltaTimeTicks, outputStream, error))
	{
		error.Add("Failed to encode delta-time ticks.");
		return false;
	}

	if (!event->Encode(outputStream, error))
	{
		error.Add("Failed to encode event.");
		return false;
	}

	return true;
}