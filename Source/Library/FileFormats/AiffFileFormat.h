#pragma once

#include "FileFormat.h"
#include "ChunkParser.h"

namespace AudioDataLib
{
	/**
	 * @brief This class knows how to load and save AIFF files.
	 * 
	 * I don't have it working yet.
	 */
	class AUDIO_DATA_LIB_API AiffFileFormat : public FileFormat
	{
	public:
		AiffFileFormat();
		virtual ~AiffFileFormat();

		virtual bool ReadFromStream(ByteStream& inputStream, FileData*& fileData, Error& error) override;
		virtual bool WriteToStream(ByteStream& outputStream, const FileData* fileData, Error& error) override;

	protected:
		class AiffChunkParser : public ChunkParser
		{
		public:
			AiffChunkParser();
			virtual ~AiffChunkParser();

			virtual bool ParseChunkData(ReadOnlyBufferStream& inputStream, Chunk* chunk, Error& error) override;
		};
	};
}