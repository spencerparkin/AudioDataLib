#pragma once

#include "AudioCommon.h"

namespace AudioDataLib
{
	class AUDIO_DATA_LIB_API ByteStream
	{
	public:
		ByteStream();
		virtual ~ByteStream();

		virtual uint64_t WriteBytesToStream(const uint8_t* buffer, uint64_t bufferSize) = 0;
		virtual uint64_t ReadBytesFromStream(uint8_t* buffer, uint64_t bufferSize) = 0;

		virtual uint64_t GetSize() const = 0;

		virtual bool CanRead() = 0;
		virtual bool CanWrite() = 0;
	};

	class AUDIO_DATA_LIB_API FileStream : public ByteStream
	{
	public:
		FileStream(const char* filePath, const char* mode);
		virtual ~FileStream();

		bool IsOpen();

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

	class AUDIO_DATA_LIB_API MemoryStream : public ByteStream
	{
		friend class ReadOnlyMemStream;

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
		mutable uint32_t readLockCount;
	};

	// This stream lets you read bytes from a memory stream without modifying it.
	class AUDIO_DATA_LIB_API ReadOnlyMemStream : public ByteStream
	{
	public:
		ReadOnlyMemStream(const MemoryStream* memoryStream);
		virtual ~ReadOnlyMemStream();

		virtual uint64_t WriteBytesToStream(const uint8_t* buffer, uint64_t bufferSize) override;
		virtual uint64_t ReadBytesFromStream(uint8_t* buffer, uint64_t bufferSize) override;

		virtual uint64_t GetSize() const override;

		virtual bool CanRead() override;
		virtual bool CanWrite() override;

	protected:
		const MemoryStream* memoryStream;
		std::list<MemoryStream::Chunk*>::iterator* chunkIter;
		uint64_t readOffset;
	};
}