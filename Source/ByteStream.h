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
	};

	class AUDIO_DATA_LIB_API FileInputStream : public ByteStream
	{
	public:
		FileInputStream(const char* filePath);
		virtual ~FileInputStream();

		virtual int WriteBytesToStream(const char* buffer, int bufferSize) override;
		virtual int ReadBytesFromStream(char* buffer, int bufferSize) override;
	};

	class AUDIO_DATA_LIB_API FileOutputStream : public ByteStream
	{
	public:
		FileOutputStream(const char* filePath);
		virtual ~FileOutputStream();

		virtual int WriteBytesToStream(const char* buffer, int bufferSize) override;
		virtual int ReadBytesFromStream(char* buffer, int bufferSize) override;
	};

	class AUDIO_DATA_LIB_API MemoryStream : public ByteStream
	{
		friend class ReadOnlyMemStream;

	public:
		MemoryStream();
		virtual ~MemoryStream();

		virtual int WriteBytesToStream(const char* buffer, int bufferSize) override;
		virtual int ReadBytesFromStream(char* buffer, int bufferSize) override;

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

	protected:
		const MemoryStream* memoryStream;
		std::list<MemoryStream::Chunk*>::iterator* chunkIter;
		int readOffset;
	};
}