#include "ByteStream.h"
#include "Mutex.h"

using namespace AudioDataLib;

//------------------------- ByteStream -------------------------

ByteStream::ByteStream()
{
}

/*virtual*/ ByteStream::~ByteStream()
{
}

/*virtual*/ uint64_t ByteStream::PeekBytesFromStream(uint8_t* buffer, uint64_t bufferSize)
{
	return 0;
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

/*virtual*/ uint64_t FileInputStream::GetSize() const
{
	if (!this->fp)
		return 0;
	uint64_t curPos = ftell(this->fp);
	fseek(this->fp, 0, SEEK_END);
	uint64_t endPos = ftell(this->fp);
	uint64_t numBytesLeft = endPos - curPos;
	fseek(this->fp, (long)curPos, SEEK_SET);
	return numBytesLeft;
}

/*virtual*/ bool FileInputStream::CanRead()
{
	return this->GetSize() > 0;
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

/*virtual*/ uint64_t FileOutputStream::GetSize() const
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

//------------------------- ReadOnlyBufferStream -------------------------

ReadOnlyBufferStream::ReadOnlyBufferStream(const uint8_t* buffer, uint64_t bufferSize)
{
	this->readOnlyBuffer = buffer;
	this->readOnlyBufferSize = bufferSize;
	this->readOffset = 0;
}

/*virtual*/ ReadOnlyBufferStream::~ReadOnlyBufferStream()
{
}

void ReadOnlyBufferStream::Reset()
{
	this->readOffset = 0;
}

/*virtual*/ uint64_t ReadOnlyBufferStream::WriteBytesToStream(const uint8_t* buffer, uint64_t bufferSize)
{
	return 0;
}

/*virtual*/ uint64_t ReadOnlyBufferStream::ReadBytesFromStream(uint8_t* buffer, uint64_t bufferSize)
{
	uint64_t numBytesRead = 0;
	while (numBytesRead < bufferSize && this->readOffset < this->readOnlyBufferSize)
		buffer[numBytesRead++] = this->readOnlyBuffer[this->readOffset++];

	return numBytesRead;
}

/*virtual*/ uint64_t ReadOnlyBufferStream::PeekBytesFromStream(uint8_t* buffer, uint64_t bufferSize)
{
	uint64_t numBytesPeeked = 0;
	uint64_t i = this->readOffset;
	while (numBytesPeeked < bufferSize && i < this->readOnlyBufferSize)
		buffer[numBytesPeeked++] = this->readOnlyBuffer[i++];

	return numBytesPeeked;
}

/*virtual*/ uint64_t ReadOnlyBufferStream::GetSize() const
{
	return this->readOnlyBufferSize - this->readOffset;
}

/*virtual*/ bool ReadOnlyBufferStream::CanRead()
{
	return this->GetSize() > 0;
}

/*virtual*/ bool ReadOnlyBufferStream::CanWrite()
{
	return false;
}

bool ReadOnlyBufferStream::SetReadOffset(uint64_t readOffset)
{
	if (readOffset > this->readOnlyBufferSize)
		return false;

	this->readOffset = readOffset;
	return true;
}

//------------------------- WriteOnlyBufferStream -------------------------

WriteOnlyBufferStream::WriteOnlyBufferStream(uint8_t* buffer, uint64_t bufferSize)
{
	this->writeOnlyBuffer = buffer;
	this->writeOnlyBufferSize = bufferSize;
	this->writeOffset = 0;
}

/*virtual*/ WriteOnlyBufferStream::~WriteOnlyBufferStream()
{
}

void WriteOnlyBufferStream::Reset()
{
	this->writeOffset = 0;
}

/*virtual*/ uint64_t WriteOnlyBufferStream::WriteBytesToStream(const uint8_t* buffer, uint64_t bufferSize)
{
	uint64_t numBytesWritten = 0;
	while (numBytesWritten < bufferSize && this->writeOffset < this->writeOnlyBufferSize)
		this->writeOnlyBuffer[this->writeOffset++] = buffer[numBytesWritten++];

	return numBytesWritten;
}

/*virtual*/ uint64_t WriteOnlyBufferStream::ReadBytesFromStream(uint8_t* buffer, uint64_t bufferSize)
{
	return 0;
}

/*virtual*/ uint64_t WriteOnlyBufferStream::GetSize() const
{
	return this->writeOffset;
}

/*virtual*/ bool WriteOnlyBufferStream::CanRead()
{
	return false;
}

/*virtual*/ bool WriteOnlyBufferStream::CanWrite()
{
	return this->GetSize() < this->writeOnlyBufferSize;
}

//------------------------- AudioStream -------------------------

AudioStream::AudioStream(const AudioData::Format& format)
{
	this->format = format;
	this->byteStream = new MemoryStream();
}

AudioStream::AudioStream(const AudioData* audioData)
{
	this->format = audioData->GetFormat();
	this->byteStream = new ReadOnlyBufferStream(audioData->GetAudioBuffer(), audioData->GetAudioBufferSize());
}

/*virtual*/ AudioStream::~AudioStream()
{
	delete this->byteStream;
}

/*virtual*/ uint64_t AudioStream::WriteBytesToStream(const uint8_t* buffer, uint64_t bufferSize)
{
	return this->byteStream->WriteBytesToStream(buffer, bufferSize);
}

/*virtual*/ uint64_t AudioStream::ReadBytesFromStream(uint8_t* buffer, uint64_t bufferSize)
{
	return this->byteStream->ReadBytesFromStream(buffer, bufferSize);
}

/*virtual*/ uint64_t AudioStream::GetSize() const
{
	return this->byteStream->GetSize();
}

/*virtual*/ bool AudioStream::CanRead()
{
	return this->byteStream->CanRead();
}

/*virtual*/ bool AudioStream::CanWrite()
{
	return this->byteStream->CanWrite();
}

//------------------------- ThreadSafeAudioStream -------------------------

ThreadSafeAudioStream::ThreadSafeAudioStream(const AudioData::Format& format, Mutex* mutex, bool ownsMutexMemory) : AudioStream(format)
{
	this->mutex = mutex;
	this->ownsMutexMemory = ownsMutexMemory;
}

/*virtual*/ ThreadSafeAudioStream::~ThreadSafeAudioStream()
{
	if (this->ownsMutexMemory)
		delete this->mutex;
}

/*virtual*/ uint64_t ThreadSafeAudioStream::WriteBytesToStream(const uint8_t* buffer, uint64_t bufferSize)
{
	MutexScopeLock scopeLock(this->mutex);
	return AudioStream::WriteBytesToStream(buffer, bufferSize);
}

/*virtual*/ uint64_t ThreadSafeAudioStream::ReadBytesFromStream(uint8_t* buffer, uint64_t bufferSize)
{
	MutexScopeLock scopeLock(this->mutex);
	return AudioStream::ReadBytesFromStream(buffer, bufferSize);
}

/*virtual*/ uint64_t ThreadSafeAudioStream::GetSize() const
{
	MutexScopeLock scopeLock(this->mutex);
	return AudioStream::GetSize();
}

/*virtual*/ bool ThreadSafeAudioStream::CanRead()
{
	MutexScopeLock scopeLock(this->mutex);
	return AudioStream::CanRead();
}

/*virtual*/ bool ThreadSafeAudioStream::CanWrite()
{
	MutexScopeLock scopeLock(this->mutex);
	return AudioStream::CanWrite();
}

//------------------------- MemoryStream -------------------------

MemoryStream::MemoryStream()
{
	this->chunkSize = 5 * 1024;
	this->chunkList = new std::list<Chunk*>();
}

/*virtual*/ MemoryStream::~MemoryStream()
{
	this->Clear();
	delete this->chunkList;
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

/*virtual*/ uint64_t MemoryStream::GetSize() const
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