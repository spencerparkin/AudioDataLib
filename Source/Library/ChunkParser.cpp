#include "ChunkParser.h"
#include "Error.h"

using namespace AudioDataLib;

//------------------------------ ChunkParser ------------------------------

ChunkParser::ChunkParser()
{
	this->rootChunk = nullptr;
	this->buffer = nullptr;
	this->bufferSize = 0;
	this->subChunkSet = new std::set<std::string>();
}

/*virtual*/ ChunkParser::~ChunkParser()
{
	this->Clear();

	delete this->subChunkSet;
}

void ChunkParser::Clear()
{
	delete this->rootChunk;
	this->rootChunk = nullptr;

	delete[] this->buffer;
	this->buffer = nullptr;
	this->bufferSize = 0;
}

bool ChunkParser::ParseStream(ByteStream& inputStream, Error& error)
{
	this->Clear();

	this->bufferSize = (uint32_t)inputStream.GetSize();
	if (this->bufferSize == 0)
	{
		error.Add("No bytes to read.");
		return false;
	}

	this->buffer = new uint8_t[this->bufferSize];
	uint64_t numBytesRead = inputStream.ReadBytesFromStream(this->buffer, this->bufferSize);
	if (numBytesRead != this->bufferSize)
	{
		error.Add("Couldn't read the entire stream.");
		return false;
	}

	ReadOnlyBufferStream bufferStream(this->buffer, this->bufferSize);

	this->rootChunk = new Chunk();
	*this->rootChunk->name = "root";
	if (!this->rootChunk->ParseSubChunks(bufferStream, this, error))
		return false;

	return true;
}

/*virtual*/ bool ChunkParser::ParseChunkData(ReadOnlyBufferStream& inputStream, Chunk* chunk, Error& error)
{
	std::set<std::string>::iterator iter = this->subChunkSet->find(chunk->GetName());
	if (iter != this->subChunkSet->end())
	{
		char formType[5];
		if (4 != inputStream.ReadBytesFromStream((uint8_t*)formType, 4))
		{
			error.Add(FormatString("Could not read form type of %s chunk.", chunk->GetName().c_str()));
			return false;
		}

		formType[4] = '\0';
		*chunk->formType = formType;

		if (0 == strcmp(formType, "INFO"))
		{
			int b = 0;
			b++;
		}

		if (!chunk->ParseSubChunks(inputStream, this, error))
			return false;
	}
	else
	{
		if (!inputStream.SetReadOffset(inputStream.GetReadOffset() + chunk->GetBufferSize()))
		{
			error.Add(FormatString("Could not skip over %s chunk data.", chunk->GetName().c_str()));
			return false;
		}
	}

	return true;
}

void ChunkParser::RegisterSubChunks(const std::string& chunkName)
{
	this->subChunkSet->insert(chunkName);
}

const ChunkParser::Chunk* ChunkParser::FindChunk(const std::string& chunkName, bool caseSensative /*= true*/) const
{
	if (!this->rootChunk)
		return nullptr;

	return this->rootChunk->FindChunk(chunkName, caseSensative);
}

void ChunkParser::FindAllChunks(const std::string& chunkName, std::vector<const Chunk*>& chunkArray, bool caseSensative /*= true*/) const
{
	chunkArray.clear();
	if (!this->rootChunk)
		return;

	this->rootChunk->FindAllChunks(chunkName, chunkArray, caseSensative);
}

//------------------------------ ChunkParser::Chunk ------------------------------

ChunkParser::Chunk::Chunk()
{
	this->name = new std::string();
	this->formType = new std::string();
	this->buffer = nullptr;
	this->bufferSize = 0;
	this->subChunkArray = new std::vector<Chunk*>();
}

/*virtual*/ ChunkParser::Chunk::~Chunk()
{
	delete this->name;
	delete this->formType;

	for (Chunk* chunk : *this->subChunkArray)
		delete chunk;

	delete this->subChunkArray;
}

const ChunkParser::Chunk* ChunkParser::Chunk::FindChunk(const std::string& chunkName, bool caseSensative) const
{
	if (this->MatchesName(chunkName, caseSensative))
		return this;

	for (const Chunk* subChunk : *this->subChunkArray)
	{
		const Chunk* foundChunk = subChunk->FindChunk(chunkName, caseSensative);
		if (foundChunk)
			return foundChunk;
	}

	return nullptr;
}

void ChunkParser::Chunk::FindAllChunks(const std::string& chunkName, std::vector<const Chunk*>& chunkArray, bool caseSensative) const
{
	if (this->MatchesName(chunkName, caseSensative))
		chunkArray.push_back(this);

	for (const Chunk* subChunk : *this->subChunkArray)
		subChunk->FindAllChunks(chunkName, chunkArray, caseSensative);
}

bool ChunkParser::Chunk::MatchesName(const std::string& chunkName, bool caseSensative) const
{
	if (caseSensative)
		return *this->name == chunkName;

	return 0 == ::_stricmp(this->name->c_str(), chunkName.c_str());
}

bool ChunkParser::Chunk::ParseStream(ReadOnlyBufferStream& inputStream, ChunkParser* chunkParser, Error& error)
{
	char nameBuf[5];
	if (4 != inputStream.ReadBytesFromStream((uint8_t*)&nameBuf, 4))
	{
		error.Add("Failed to read chunk name.");
		return false;
	}

	nameBuf[4] = '\0';
	*this->name = nameBuf;

	if (4 != inputStream.ReadBytesFromStream((uint8_t*)&this->bufferSize, sizeof(uint32_t)))
	{
		error.Add("Could not read chunk size.");
		return false;
	}

	this->bufferSize = chunkParser->byteSwapper.Resolve(this->bufferSize);
	this->buffer = inputStream.GetBuffer() + inputStream.GetReadOffset();

	ReadOnlyBufferStream subInputStream(this->buffer, this->bufferSize);

	if (!chunkParser->ParseChunkData(subInputStream, this, error))
		return false;

	if (!inputStream.SetReadOffset(inputStream.GetReadOffset() + this->bufferSize))
	{
		error.Add("Could not walk over data section of chunk.");
		return false;
	}

	// Sometimes a chunk is just a string, but the chunk size does not reflect the null-byte at the end.
	// Consume the null byte now if that is the case.
	if (inputStream.CanRead())
	{
		char ch = 0;
		inputStream.PeekBytesFromStream((uint8_t*)&ch, 1);
		if (ch == '\0')
			inputStream.ReadBytesFromStream((uint8_t*)&ch, 1);
	}

	return true;
}

bool ChunkParser::Chunk::ParseSubChunks(ReadOnlyBufferStream& inputStream, ChunkParser* chunkParser, Error& error)
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