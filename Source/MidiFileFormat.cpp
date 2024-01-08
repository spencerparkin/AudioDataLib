#include "MidiFileFormat.h"

using namespace AudioDataLib;

MidiFileFormat::MidiFileFormat()
{
}

/*virtual*/ MidiFileFormat::~MidiFileFormat()
{
}

/*virtual*/ bool MidiFileFormat::ReadFromStream(ByteStream& inputStream, FileData*& fileData, std::string& error)
{
	ChunkParser chunkParser;
	chunkParser.byteSwapper.swapsNeeded = true;
	if (!chunkParser.ParseStream(inputStream, error))
		return false;

	// See: https://github.com/colxi/midi-parser-js/wiki/MIDI-File-Format-Specifications
	return true;
}

/*virtual*/ bool MidiFileFormat::WriteToStream(ByteStream& outputStream, FileData* fileData, std::string& error)
{
	return true;
}