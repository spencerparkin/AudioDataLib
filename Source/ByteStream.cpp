#include "ByteStream.h"

using namespace AudioDataLib;

//------------------------- ByteStream -------------------------

ByteStream::ByteStream()
{
}

/*virtual*/ ByteStream::~ByteStream()
{
}

//------------------------- FileStream -------------------------

FileStream::FileStream(const char* filePath, const char* mode)
{
	this->fp = fopen(filePath, mode);
}

/*virtual*/ FileStream::~FileStream()
{
	if (this->fp)
	{
		fclose(this->fp);
		this->fp = nullptr;
	}
}

bool FileStream::IsOpen()
{
	return this->fp != nullptr;
}

//------------------------- FileInputStream -------------------------

FileInputStream::FileInputStream(const char* filePath) : FileStream(filePath, "rb")
{
}

/*virtual*/ FileInputStream::~FileInputStream()
{
}

/*virtual*/ uint64_t FileInputStream::WriteBytesToStream(const uint8_t* buffer, uint64_t bufferSize)
{
	return 0;
}

/*virtual*/ uint64_t FileInputStream::ReadBytesFromStream(uint8_t* buffer, uint64_t bufferSize)
{
	if (!this->fp)
		return 0;

	return (uint64_t)fread(buffer, 1, (size_t)bufferSize, fp);
}

uint64_t FileInputStream::NumBytesLeft()
{
	uint64_t curPos = ftell(this->fp);
	fseek(this->fp, 0, SEEK_END);
	uint64_t endPos = ftell(this->fp);
	uint64_t numBytesLeft = endPos - curPos;
	fseek(this->fp, (long)curPos, SEEK_SET);
	return numBytesLeft;
}

/*virtual*/ bool FileInputStream::CanRead()
{
	return this->NumBytesLeft() > 0;
}

/*virtual*/ bool FileInputStream::CanWrite()
{
	return false;
}

//------------------------- FileOutputStream -------------------------

FileOutputStream::FileOutputStream(const char* filePath) : FileStream(filePath, "wb")
{
}

/*virtual*/ FileOutputStream::~FileOutputStream()
{
}

/*virtual*/ uint64_t FileOutputStream::WriteBytesToStream(const uint8_t* buffer, uint64_t bufferSize)
{
	if (!this->fp)
		return 0;

	return (uint64_t)fwrite(buffer, 1, (size_t)bufferSize, fp);
}

/*virtual*/ uint64_t FileOutputStream::ReadBytesFromStream(uint8_t* buffer, uint64_t bufferSize)
{
	return 0;
}

/*virtual*/ bool FileOutputStream::CanRead()
{
	return false;
}

/*virtual*/ bool FileOutputStream::CanWrite()
{
	return true;
}

//------------------------- MemoryStream -------------------------

MemoryStream::MemoryStream()
{
	this->chunkSize = 5 * 1024;
	this->chunkList = new std::list<Chunk*>();
	this->readLockCount = 0;
}

/*virtual*/ MemoryStream::~MemoryStream()
{
	this->Clear();
	delete this->chunkList;
	assert(this->readLockCount == 0);
}

void MemoryStream::Clear()
{
	while (this->chunkList->size() > 0)
	{
		auto iter = this->chunkList->begin();
		Chunk* chunk = *iter;
		delete chunk;
		this->chunkList->erase(iter);
	}
}

/*virtual*/ uint64_t MemoryStream::WriteBytesToStream(const uint8_t* buffer, uint64_t bufferSize)
{
	uint64_t numBytesWritten = 0;

	if(this->chunkList->size() == 0)
		this->chunkList->push_back(new Chunk(this->chunkSize));

	while (numBytesWritten < bufferSize)
	{
		Chunk* chunk = this->chunkList->back();
		
		uint64_t numChunkBytesWritten = chunk->WriteToChunk(&buffer[numBytesWritten], bufferSize - numBytesWritten);
		if (numChunkBytesWritten > 0)
			numBytesWritten += numChunkBytesWritten;
		else
			this->chunkList->push_back(new Chunk(this->chunkSize));
	}

	return numBytesWritten;
}

/*virtual*/ uint64_t MemoryStream::ReadBytesFromStream(uint8_t* buffer, uint64_t bufferSize)
{
	if (this->readLockCount > 0)
		return 0;

	uint64_t numBytesRead = 0;

	while (numBytesRead < bufferSize && this->chunkList->size() > 0)
	{
		Chunk* chunk = *this->chunkList->begin();

		uint64_t numChunkBytesRead = chunk->ReadFromChunk(&buffer[numBytesRead], bufferSize - numBytesRead);
		if (numChunkBytesRead > 0)
			numBytesRead += numChunkBytesRead;
		else
		{
			delete chunk;
			this->chunkList->erase(this->chunkList->begin());
		}
	}

	return numBytesRead;
}

uint64_t MemoryStream::GetSize() const
{
	uint64_t totalSizeBytes = 0;

	for (const Chunk* chunk : *this->chunkList)
		totalSizeBytes += chunk->GetSize();

	return totalSizeBytes;
}

/*virtual*/ bool MemoryStream::CanRead()
{
	return this->GetSize() > 0;
}

/*virtual*/ bool MemoryStream::CanWrite()
{
	return true;
}

//------------------------- MemoryStream::Chunk -------------------------

MemoryStream::Chunk::Chunk(uint64_t bufferSize)
{
	this->bufferSize = bufferSize;
	this->buffer = new uint8_t[(uint32_t)this->bufferSize];
	::memset(this->buffer, 0, (uint32_t)this->bufferSize);
	this->startOffset = 0;
	this->endOffset = 0;
}

/*virtual*/ MemoryStream::Chunk::~Chunk()
{
	delete[] this->buffer;
}

uint64_t MemoryStream::Chunk::WriteToChunk(const uint8_t* givenBuffer, uint64_t givenBufferSize)
{
	uint64_t i = 0;
	while (this->endOffset < this->bufferSize && i < givenBufferSize)
		this->buffer[this->endOffset++] = givenBuffer[i++];

	return i;
}

uint64_t MemoryStream::Chunk::ReadFromChunk(uint8_t* givenBuffer, uint64_t givenBufferSize)
{
	uint64_t i = 0;
	while (this->startOffset < this->endOffset && i < givenBufferSize)
		givenBuffer[i++] = this->buffer[this->startOffset++];

	return i;
}

uint64_t MemoryStream::Chunk::GetSize() const
{
	return this->endOffset - this->startOffset;
}

//------------------------- ReadOnlyMemStream -------------------------

ReadOnlyMemStream::ReadOnlyMemStream(const MemoryStream* memoryStream)
{
	this->memoryStream = memoryStream;
	this->memoryStream->readLockCount++;
	this->chunkIter = new std::list<MemoryStream::Chunk*>::iterator();
	*this->chunkIter = this->memoryStream->chunkList->begin();
	this->readOffset = 0;
}

/*virtual*/ ReadOnlyMemStream::~ReadOnlyMemStream()
{
	this->memoryStream->readLockCount--;
	delete this->chunkIter;
}

/*virtual*/ uint64_t ReadOnlyMemStream::WriteBytesToStream(const uint8_t* buffer, uint64_t bufferSize)
{
	return 0;
}

/*virtual*/ uint64_t ReadOnlyMemStream::ReadBytesFromStream(uint8_t* buffer, uint64_t bufferSize)
{
	uint64_t numBytesRead = 0;

	while (*this->chunkIter != this->memoryStream->chunkList->end())
	{
		const MemoryStream::Chunk* chunk = **this->chunkIter;

		uint64_t i = chunk->startOffset + this->readOffset;
		while (numBytesRead < bufferSize && i < chunk->endOffset)
			buffer[numBytesRead++] = chunk->buffer[i++];

		if (numBytesRead == bufferSize)
			break;

		this->readOffset = 0;
		(*this->chunkIter)++;
	}

	return numBytesRead;
}

/*virtual*/ bool ReadOnlyMemStream::CanRead()
{
	if (!this->memoryStream)
		return false;

	if (*this->chunkIter == this->memoryStream->chunkList->end())
		return false;

	const MemoryStream::Chunk* chunk = **this->chunkIter;
	return chunk->GetSize() > 0;
}

/*virtual*/ bool ReadOnlyMemStream::CanWrite()
{
	return false;
}