#pragma once

#include "AudioCommon.h"

namespace AudioDataLib
{
	class AUDIO_DATA_LIB_API ByteStream
	{
	public:
		ByteStream();
		virtual ~ByteStream();

		virtual int WriteBytesToStream(const char* buffer, int bufferSize) = 0;
		virtual int ReadBytesFromStream(char* buffer, int bufferSize) = 0;

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

		virtual int WriteBytesToStream(const char* buffer, int bufferSize) override;
		virtual int ReadBytesFromStream(char* buffer, int bufferSize) override;

		virtual bool CanRead() override;
		virtual bool CanWrite() override;

		int NumBytesLeft();
	};

	class AUDIO_DATA_LIB_API FileOutputStream : public FileStream
	{
	public:
		FileOutputStream(const char* filePath);
		virtual ~FileOutputStream();

		virtual int WriteBytesToStream(const char* buffer, int bufferSize) override;
		virtual int ReadBytesFromStream(char* buffer, int bufferSize) override;

		virtual bool CanRead() override;
		virtual bool CanWrite() override;
	};

	class AUDIO_DATA_LIB_API MemoryStream : public ByteStream
	{
		friend class ReadOnlyMemStream;

	public:
		MemoryStream();
		virtual ~MemoryStream();

		virtual int WriteBytesToStream(const char* buffer, int bufferSize) override;
		virtual int ReadBytesFromStream(char* buffer, int bufferSize) override;

		virtual bool CanRead() override;
		virtual bool CanWrite() override;

		void Clear();
		int GetSize() const;

	protected:
		class Chunk
		{
		public:
			Chunk(int bufferSize);
			virtual ~Chunk();

			int WriteToChunk(const char* givenBuffer, int givenBufferSize);
			int ReadFromChunk(char* givenBuffer, int givenBufferSize);

			int GetSize() const;

			char* buffer;
			int bufferSize;
			int startOffset;
			int endOffset;
		};

		std::list<Chunk*>* chunkList;
		int chunkSize;
		mutable int readLockCount;
	};

	// This stream lets you read bytes from a memory stream without modifying it.
	class AUDIO_DATA_LIB_API ReadOnlyMemStream : public ByteStream
	{
	public:
		ReadOnlyMemStream(const MemoryStream* memoryStream);
		virtual ~ReadOnlyMemStream();

		virtual int WriteBytesToStream(const char* buffer, int bufferSize) override;
		virtual int ReadBytesFromStream(char* buffer, int bufferSize) override;

		virtual bool CanRead() override;
		virtual bool CanWrite() override;

	protected:
		const MemoryStream* memoryStream;
		std::list<MemoryStream::Chunk*>::iterator* chunkIter;
		int readOffset;
	};
}