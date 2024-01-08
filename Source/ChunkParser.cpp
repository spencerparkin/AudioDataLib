#include "ChunkParser.h"

using namespace AudioDataLib;

//------------------------------ ChunkParser ------------------------------

ChunkParser::ChunkParser()
{
	this->rootChunk = nullptr;
	this->buffer = nullptr;
	this->bufferSize = 0;
}

/*virtual*/ ChunkParser::~ChunkParser()
{
	this->Clear();
}

void ChunkParser::Clear()
{
	delete this->rootChunk;
	this->rootChunk = nullptr;

	delete[] this->buffer;
	this->buffer = nullptr;
	this->bufferSize = 0;
}

bool ChunkParser::ParseStream(ByteStream& inputStream, std::string& error)
{
	this->Clear();

	this->bufferSize = (uint32_t)inputStream.GetSize();
	if (this->bufferSize == 0)
	{
		error = "No bytes to read.";
		return false;
	}

	this->buffer = new uint8_t[this->bufferSize];
	uint64_t numBytesRead = inputStream.ReadBytesFromStream(this->buffer, this->bufferSize);
	if (numBytesRead != this->bufferSize)
	{
		error = "Couldn't read the entire stream.";
		return false;
	}

	BufferStream bufferStream(this->buffer, this->bufferSize);

	this->rootChunk = new Chunk();
	*this->rootChunk->name = "root";
	if (!this->rootChunk->ParseSubChunks(bufferStream, this, error))
		return false;

	return true;
}

/*virtual*/ bool ChunkParser::ParseChunkData(BufferStream& inputStream, Chunk* chunk, std::string& error)
{
	// Unfortunately, the RIFF format is not so generic that the parser can do
	// everything entirely by itself.  Sub-chunks may exist, but it is up to
	// the user of the parser to figure out if and where sub-chunks exist in the
	// body of a chunk.  Hence, we deligate here to the user of the parser to
	// figure out how to parse the chunk data/body section of the chunk.
	return true;
}

const ChunkParser::Chunk* ChunkParser::FindChunk(const std::string& chunkName) const
{
	if (!this->rootChunk)
		return nullptr;

	return this->rootChunk->FindChunk(chunkName);
}

void ChunkParser::FindAllChunks(const std::string& chunkName, std::vector<const Chunk*>& chunkArray) const
{
	chunkArray.clear();
	if (!this->rootChunk)
		return;

	this->rootChunk->FindAllChunks(chunkName, chunkArray);
}

//------------------------------ ChunkParser::Chunk ------------------------------

ChunkParser::Chunk::Chunk()
{
	this->name = new std::string();
	this->buffer = nullptr;
	this->bufferSize = 0;
	this->subChunkArray = new std::vector<Chunk*>();
}

/*virtual*/ ChunkParser::Chunk::~Chunk()
{
	delete this->name;

	for (Chunk* chunk : *this->subChunkArray)
		delete chunk;

	delete this->subChunkArray;
}

const ChunkParser::Chunk* ChunkParser::Chunk::FindChunk(const std::string& chunkName) const
{
	if (*this->name == chunkName)
		return this;

	for (const Chunk* subChunk : *this->subChunkArray)
	{
		const Chunk* foundChunk = subChunk->FindChunk(chunkName);
		if (foundChunk)
			return foundChunk;
	}

	return nullptr;
}

void ChunkParser::Chunk::FindAllChunks(const std::string& chunkName, std::vector<const Chunk*>& chunkArray) const
{
	if (*this->name == chunkName)
		chunkArray.push_back(this);

	for (const Chunk* subChunk : *this->subChunkArray)
		subChunk->FindAllChunks(chunkName, chunkArray);
}

bool ChunkParser::Chunk::ParseStream(BufferStream& inputStream, ChunkParser* chunkParser, std::string& error)
{
	char nameBuf[5];
	if (4 != inputStream.ReadBytesFromStream((uint8_t*)&nameBuf, 4))
	{
		error = "Failed to read chunk name.";
		return false;
	}

	nameBuf[4] = '\0';
	*this->name = nameBuf;

	if (4 != inputStream.ReadBytesFromStream((uint8_t*)&this->bufferSize, sizeof(uint32_t)))
	{
		error = "Could not read chunk size.";
		return false;
	}

	this->bufferSize = chunkParser->byteSwapper.Resolve(this->bufferSize);
	this->buffer = inputStream.GetBuffer() + inputStream.GetReadOffset();

	BufferStream subInputStream(this->buffer, this->bufferSize);

	if (!chunkParser->ParseChunkData(subInputStream, this, error))
		return false;

	inputStream.SetReadOffset(inputStream.GetReadOffset() + this->bufferSize);

	return true;
}

bool ChunkParser::Chunk::ParseSubChunks(BufferStream& inputStream, ChunkParser* chunkParser, std::string& error)
{
	while (inputStream.CanRead())
	{
		Chunk* chunk = new Chunk();
		this->subChunkArray->push_back(chunk);
		if (!chunk->ParseStream(inputStream, chunkParser, error))
			return false;
	}

	return true;
}