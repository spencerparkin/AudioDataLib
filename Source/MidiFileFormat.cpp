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
		if ((timingDivision & 0x8000) != 0)
		{
			midiData->timing.type = MidiData::Timing::Type::TICKS_PER_BEAT;
			midiData->timing.ticksPerBeat = (timingDivision & 0x7FFF);
		}
		else
		{
			midiData->timing.type = MidiData::Timing::Type::FRAMES_PER_SECOND;
			midiData->timing.framesPerSecond = (timingDivision & 0x7F00) >> 8;
			midiData->timing.ticksPerFrame = (timingDivision & 0x00FF);
		}

		// TODO: Internalize the tracks here.

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

void MidiFileFormat::EncodeVariableLengthValue(uint64_t value, uint8_t* buffer)
{
	// TODO: Write this once you need it.
}

void MidiFileFormat::DecodeVariableLengthValue(uint64_t& value, const uint8_t* buffer)
{
	std::vector<uint8_t> componentsArray;
	for(uint32_t i = 0; i < 4; i++)
	{
		uint8_t byte = buffer[i];
		componentsArray.push_back(byte & 0x7F);
		if ((byte & 0x80) == 0)
			break;
	}

	value = 0ULL;
	uint64_t shift = 0;
	while (componentsArray.size() > 0)
	{
		uint64_t component = (uint64_t)componentsArray.back();
		componentsArray.pop_back();
		value |= (component << shift);
		shift += 7;
	}
}