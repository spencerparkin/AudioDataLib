#include "SoundFontFormat.h"

using namespace AudioDataLib;

SoundFontFormat::SoundFontFormat()
{
}

/*virtual*/ SoundFontFormat::~SoundFontFormat()
{
}

/*virtual*/ bool SoundFontFormat::ReadFromStream(ByteStream& inputStream, FileData*& fileData, Error& error)
{
	ChunkParser parser;
	if (!parser.ParseStream(inputStream, error))
		return false;

	return false;
}

/*virtual*/ bool SoundFontFormat::WriteToStream(ByteStream& outputStream, FileData* fileData, Error& error)
{
	return false;
}