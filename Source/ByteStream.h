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
		virtual int ReadBytesFromString(char* buffer, int bufferSize) = 0;
	};

	class AUDIO_DATA_LIB_API MemoryStream : public ByteStream
	{
	public:
		MemoryStream();
		virtual ~MemoryStream();

		virtual int WriteBytesToStream(const char* buffer, int bufferSize) override;
		virtual int ReadBytesFromString(char* buffer, int bufferSize) override;

		void Clear();

	protected:
		class Chunk
		{
		public:
			Chunk(int bufferSize);
			virtual ~Chunk();

			int WriteToChunk(const char* givenBuffer, int givenBufferSize);
			int ReadFromChunk(char* givenBuffer, int givenBufferSize);

			char* buffer;
			int bufferSize;
			int startOffset;
			int endOffset;
		};

		std::list<Chunk*>* chunkList;
		int chunkSize;
	};
}