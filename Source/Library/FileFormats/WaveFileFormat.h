#pragma once

#include "FileFormat.h"
#include "ChunkParser.h"

namespace AudioDataLib
{
	class Error;

	/**
	 * @brief This class knows how to read and write WAVE files.
	 * 
	 * Wave files are a common, uncompressed (and therefore lossless) audio format,
	 * containing audio data in PCM (pulse code modulation) or floating-point format.
	 * I still need to add support unsigned PCM once I get my hands on an example file.
	 */
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

		virtual bool ReadFromStream(ByteStream& inputStream, FileData*& fileData, Error& error) override;
		virtual bool WriteToStream(ByteStream& outputStream, const FileData* fileData, Error& error) override;

	protected:
		class WaveChunkParser : public ChunkParser
		{
		public:
			WaveChunkParser();
			virtual ~WaveChunkParser();

			virtual bool ParseChunkData(ReadOnlyBufferStream& inputStream, Chunk* chunk, Error& error) override;
		};
	};
}