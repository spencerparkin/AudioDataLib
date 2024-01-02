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

/*virtual*/ int FileInputStream::WriteBytesToStream(const char* buffer, int bufferSize)
{
	return 0;
}

/*virtual*/ int FileInputStream::ReadBytesFromStream(char* buffer, int bufferSize)
{
	if (!this->fp)
		return 0;

	return (int)fread(buffer, 1, bufferSize, fp);
}

int FileInputStream::NumBytesLeft()
{
	int curPos = ftell(this->fp);
	fseek(this->fp, 0, SEEK_END);
	int endPos = ftell(this->fp);
	int numBytesLeft = endPos - curPos;
	fseek(this->fp, curPos, SEEK_SET);
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

/*virtual*/ int FileOutputStream::WriteBytesToStream(const char* buffer, int bufferSize)
{
	if (!this->fp)
		return 0;

	return (int)fwrite(buffer, 1, bufferSize, fp);
}

/*virtual*/ int FileOutputStream::ReadBytesFromStream(char* buffer, int bufferSize)
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

/*virtual*/ int MemoryStream::WriteBytesToStream(const char* buffer, int bufferSize)
{
	int numBytesWritten = 0;

	if(this->chunkList->size() == 0)
		this->chunkList->push_back(new Chunk(this->chunkSize));

	while (numBytesWritten < bufferSize)
	{
		Chunk* chunk = this->chunkList->back();
		
		int numChunkBytesWritten = chunk->WriteToChunk(&buffer[numBytesWritten], bufferSize - numBytesWritten);
		if (numChunkBytesWritten > 0)
			numBytesWritten += numChunkBytesWritten;
		else
			this->chunkList->push_back(new Chunk(this->chunkSize));
	}

	return numBytesWritten;
}

/*virtual*/ int MemoryStream::ReadBytesFromStream(char* buffer, int bufferSize)
{
	if (this->readLockCount > 0)
		return 0;

	int numBytesRead = 0;

	while (numBytesRead < bufferSize && this->chunkList->size() > 0)
	{
		Chunk* chunk = *this->chunkList->begin();

		int numChunkBytesRead = chunk->ReadFromChunk(&buffer[numBytesRead], bufferSize - numBytesRead);
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

int MemoryStream::GetSize() const
{
	int totalSizeBytes = 0;

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

MemoryStream::Chunk::Chunk(int bufferSize)
{
	this->bufferSize = bufferSize;
	this->buffer = new char[this->bufferSize];
	::memset(this->buffer, 0, this->bufferSize);
	this->startOffset = 0;
	this->endOffset = 0;
}

/*virtual*/ MemoryStream::Chunk::~Chunk()
{
	delete[] this->buffer;
}

int MemoryStream::Chunk::WriteToChunk(const char* givenBuffer, int givenBufferSize)
{
	int i = 0;
	while (this->endOffset < this->bufferSize && i < givenBufferSize)
		this->buffer[this->endOffset++] = givenBuffer[i++];

	return i;
}

int MemoryStream::Chunk::ReadFromChunk(char* givenBuffer, int givenBufferSize)
{
	int i = 0;
	while (this->startOffset < this->endOffset && i < givenBufferSize)
		givenBuffer[i++] = this->buffer[this->startOffset++];

	return i;
}

int MemoryStream::Chunk::GetSize() const
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

/*virtual*/ int ReadOnlyMemStream::WriteBytesToStream(const char* buffer, int bufferSize)
{
	return 0;
}

/*virtual*/ int ReadOnlyMemStream::ReadBytesFromStream(char* buffer, int bufferSize)
{
	int numBytesRead = 0;

	while (*this->chunkIter != this->memoryStream->chunkList->end())
	{
		const MemoryStream::Chunk* chunk = **this->chunkIter;

		int i = chunk->startOffset + this->readOffset;
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