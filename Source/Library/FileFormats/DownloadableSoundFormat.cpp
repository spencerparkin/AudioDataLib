#include "DownloadableSoundFormat.h"
#include "Error.h"

using namespace AudioDataLib;

//---------------------------------------- DownloadableSoundFormat ----------------------------------------

DownloadableSoundFormat::DownloadableSoundFormat()
{
}

/*virtual*/ DownloadableSoundFormat::~DownloadableSoundFormat()
{
}

/*virtual*/ bool DownloadableSoundFormat::ReadFromStream(ByteStream& inputStream, FileData*& fileData, Error& error)
{
	ChunkParser parser;
	parser.RegisterSubChunks("RIFF");
	parser.RegisterSubChunks("LIST");
	if (!parser.ParseStream(inputStream, error))
		return false;

	const ChunkParser::Chunk* chunk = parser.FindChunk("RIFF");
	if (!chunk)
	{
		error.Add("No RIFF chunk found!");
		return false;
	}

	if (chunk->GetFormType() != "DLS ")
	{
		error.Add("RIFF file does not appear to be a DLS file.");
		return false;
	}

	// TODO: Write remainder of read function here...
	//       I'm not sure I'm going to able to ever write this.  It seems as though DLS is proprietary or I just can't find documentation on it.

	error.Add("Implimentation not yet finished.");
	return false;
}

/*virtual*/ bool DownloadableSoundFormat::WriteToStream(ByteStream& outputStream, const FileData* fileData, Error& error)
{
	error.Add("Not yet implimented.");
	return false;
}