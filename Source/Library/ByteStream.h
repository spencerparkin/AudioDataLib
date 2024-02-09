#pragma once

#include "Common.h"
#include "AudioData.h"

namespace AudioDataLib
{
	class Mutex;

	class AUDIO_DATA_LIB_API ByteStream
	{
	public:
		ByteStream();
		virtual ~ByteStream();

		virtual uint64_t WriteBytesToStream(const uint8_t* buffer, uint64_t bufferSize) = 0;
		virtual uint64_t ReadBytesFromStream(uint8_t* buffer, uint64_t bufferSize) = 0;

		virtual uint64_t PeekBytesFromStream(uint8_t* buffer, uint64_t bufferSize);

		virtual uint64_t GetSize() const = 0;

		virtual bool CanRead() = 0;
		virtual bool CanWrite() = 0;

		template<typename T>
		bool ReadType(T* value)
		{
			uint64_t numBytesRead = this->ReadBytesFromStream((uint8_t*)value, sizeof(T));
			return numBytesRead == sizeof(T);
		}

		template<typename T>
		bool WriteType(const T* value)
		{
			uint64_t numBytesWritten = this->WriteBytesToStream((const uint8_t*)value, sizeof(T));
			return numBytesWritten == sizeof(T);
		}
	};

	class AUDIO_DATA_LIB_API FileStream : public ByteStream
	{
	public:
		FileStream(const char* filePath, const char* mode);
		virtual ~FileStream();

		bool IsOpen();
		void Close();

	protected:
		FILE* fp;
	};

	class AUDIO_DATA_LIB_API FileInputStream : public FileStream
	{
	public:
		FileInputStream(const char* filePath);
		virtual ~FileInputStream();

		virtual uint64_t WriteBytesToStream(const uint8_t* buffer, uint64_t bufferSize) override;
		virtual uint64_t ReadBytesFromStream(uint8_t* buffer, uint64_t bufferSize) override;

		virtual uint64_t GetSize() const override;

		virtual bool CanRead() override;
		virtual bool CanWrite() override;
	};

	class AUDIO_DATA_LIB_API FileOutputStream : public FileStream
	{
	public:
		FileOutputStream(const char* filePath);
		virtual ~FileOutputStream();

		virtual uint64_t WriteBytesToStream(const uint8_t* buffer, uint64_t bufferSize) override;
		virtual uint64_t ReadBytesFromStream(uint8_t* buffer, uint64_t bufferSize) override;

		virtual uint64_t GetSize() const override;

		virtual bool CanRead() override;
		virtual bool CanWrite() override;
	};

	class AUDIO_DATA_LIB_API ReadOnlyBufferStream : public ByteStream
	{
	public:
		ReadOnlyBufferStream(const uint8_t* buffer, uint64_t bufferSize);
		virtual ~ReadOnlyBufferStream();

		virtual uint64_t WriteBytesToStream(const uint8_t* buffer, uint64_t bufferSize) override;
		virtual uint64_t ReadBytesFromStream(uint8_t* buffer, uint64_t bufferSize) override;

		virtual uint64_t PeekBytesFromStream(uint8_t* buffer, uint64_t bufferSize) override;

		virtual uint64_t GetSize() const override;

		virtual bool CanRead() override;
		virtual bool CanWrite() override;

		uint64_t GetReadOffset() const { return this->readOffset; }
		bool SetReadOffset(uint64_t readOffset);

		const uint8_t* GetBuffer() const { return this->readOnlyBuffer; }

		void Reset();

	protected:
		const uint8_t* readOnlyBuffer;
		uint64_t readOnlyBufferSize;
		uint64_t readOffset;
	};

	class AUDIO_DATA_LIB_API WriteOnlyBufferStream : public ByteStream
	{
	public:
		WriteOnlyBufferStream(uint8_t* buffer, uint64_t bufferSize);
		virtual ~WriteOnlyBufferStream();

		virtual uint64_t WriteBytesToStream(const uint8_t* buffer, uint64_t bufferSize) override;
		virtual uint64_t ReadBytesFromStream(uint8_t* buffer, uint64_t bufferSize) override;

		virtual uint64_t GetSize() const override;

		virtual bool CanRead() override;
		virtual bool CanWrite() override;

		uint64_t GetWriteOffset() const { return this->writeOffset; }
		bool SetWriteOffset(uint64_t readOffset);

		const uint8_t* GetBuffer() const { return this->writeOnlyBuffer; }

		void Reset();

	protected:
		uint8_t* writeOnlyBuffer;
		uint64_t writeOnlyBufferSize;
		uint64_t writeOffset;
	};

	// This class by itself often acts, single-handedly, as the glue between
	// this library and one that interfaces with the actual audio hardware.
	// The low-level library will dicate the stream format, and then this library
	// with accommodate that format.  The low-level library read from (or writes to)
	// this stream while this library writes to (or reads from) it.
	class AUDIO_DATA_LIB_API AudioStream : public ByteStream
	{
	public:
		AudioStream();
		AudioStream(const AudioData* audioData);
		virtual ~AudioStream();

		virtual uint64_t WriteBytesToStream(const uint8_t* buffer, uint64_t bufferSize) override;
		virtual uint64_t ReadBytesFromStream(uint8_t* buffer, uint64_t bufferSize) override;

		virtual uint64_t GetSize() const override;

		virtual bool CanRead() override;
		virtual bool CanWrite() override;

		const AudioData::Format& GetFormat() const { return this->format; }
		void SetFormat(const AudioData::Format& format) { this->format = format; }

	protected:
		AudioData::Format format;
		ByteStream* byteStream;
	};

	class AUDIO_DATA_LIB_API ThreadSafeAudioStream : public AudioStream
	{
	public:
		ThreadSafeAudioStream(Mutex* mutex, bool ownsMutexMemory);
		virtual ~ThreadSafeAudioStream();

		virtual uint64_t WriteBytesToStream(const uint8_t* buffer, uint64_t bufferSize) override;
		virtual uint64_t ReadBytesFromStream(uint8_t* buffer, uint64_t bufferSize) override;

		virtual uint64_t GetSize() const override;

		virtual bool CanRead() override;
		virtual bool CanWrite() override;

	protected:
		Mutex* mutex;
		bool ownsMutexMemory;
	};

	class AUDIO_DATA_LIB_API MemoryStream : public ByteStream
	{
	public:
		MemoryStream();
		virtual ~MemoryStream();

		virtual uint64_t WriteBytesToStream(const uint8_t* buffer, uint64_t bufferSize) override;
		virtual uint64_t ReadBytesFromStream(uint8_t* buffer, uint64_t bufferSize) override;

		virtual uint64_t GetSize() const override;

		virtual bool CanRead() override;
		virtual bool CanWrite() override;

		void Clear();

	protected:
		class Chunk
		{
		public:
			Chunk(uint64_t bufferSize);
			virtual ~Chunk();

			uint64_t WriteToChunk(const uint8_t* givenBuffer, uint64_t givenBufferSize);
			uint64_t ReadFromChunk(uint8_t* givenBuffer, uint64_t givenBufferSize);

			uint64_t GetSize() const;

			uint8_t* buffer;
			uint64_t bufferSize;
			uint64_t startOffset;
			uint64_t endOffset;
		};

		std::list<Chunk*>* chunkList;
		uint64_t chunkSize;
	};
}