#include "AiffFileFormat.h"
#include "Error.h"

using namespace AudioDataLib;

//------------------------------- AiffFileFormat -------------------------------

AiffFileFormat::AiffFileFormat()
{
}

/*virtual*/ AiffFileFormat::~AiffFileFormat()
{
}

/*virtual*/ bool AiffFileFormat::ReadFromStream(ByteStream& inputStream, FileData*& fileData, Error& error)
{
	// https://paulbourke.net/dataformats/audio/

	AiffChunkParser parser;
	parser.byteSwapper.swapsNeeded = true;

	if (!parser.ParseStream(inputStream, error))
		return false;

	// TODO: Produce AudioData here and assign it to fileData.

	return true;
}

/*virtual*/ bool AiffFileFormat::WriteToStream(ByteStream& outputStream, const FileData* fileData, Error& error)
{
	error.Add("Not yet implemented.");
	return false;
}

//------------------------------- AiffFileFormat::AiffChunkParser -------------------------------

AiffFileFormat::AiffChunkParser::AiffChunkParser()
{
}

/*virtual*/ AiffFileFormat::AiffChunkParser::~AiffChunkParser()
{
}

/*virtual*/ bool AiffFileFormat::AiffChunkParser::ParseChunkData(ReadOnlyBufferStream& inputStream, Chunk* chunk, Error& error)
{
	if (chunk->GetName() == "FORM")
	{
		char formType[5];
		if (4 != inputStream.ReadBytesFromStream((uint8_t*)formType, 4))
		{
			error.Add("Could not read form type of FORM chunk.");
			return false;
		}

		formType[4] = '\0';
		if (0 != strcmp(formType, "AIFF") && 0 != strcmp(formType, "AIFC"))
		{
			error.Add("File does not appears to be an AIFF file.");
			return false;
		}

		if (!chunk->ParseSubChunks(inputStream, this, error))
			return false;
	}
	else
	{
		if (!inputStream.SetReadOffset(inputStream.GetReadOffset() + chunk->GetBufferSize()))
		{
			error.Add("Could not skip over chunk data.");
			return false;
		}
	}

	return true;
}