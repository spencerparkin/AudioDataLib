#pragma once

#include "FileFormat.h"
#include "ChunkParser.h"

namespace AudioDataLib
{
	class AUDIO_DATA_LIB_API WaveFileFormat : public FileFormat
	{
	public:
		WaveFileFormat();
		virtual ~WaveFileFormat();

		enum SampleFormat
		{
			PCM = 1,
			IEEE_FLOAT = 3
		};

		virtual bool ReadFromStream(ByteStream& inputStream, FileData*& fileData, std::string& error) override;
		virtual bool WriteToStream(ByteStream& outputStream, FileData* fileData, std::string& error) override;

	protected:
		class WaveChunkParser : public ChunkParser
		{
		public:
			WaveChunkParser();
			virtual ~WaveChunkParser();

			virtual bool ParseChunkData(ReadOnlyBufferStream& inputStream, Chunk* chunk, std::string& error) override;
		};
	};
}