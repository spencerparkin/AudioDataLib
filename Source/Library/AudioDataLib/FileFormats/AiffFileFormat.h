#pragma once

#include "AudioDataLib/FileFormats/FileFormat.h"
#include "AudioDataLib/ChunkParser.h"

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

		virtual bool ReadFromStream(ByteStream& inputStream, std::unique_ptr<FileData>& fileData) override;
		virtual bool WriteToStream(ByteStream& outputStream, const FileData* fileData) override;

	protected:
		class AiffChunkParser : public ChunkParser
		{
		public:
			AiffChunkParser();
			virtual ~AiffChunkParser();

			virtual bool ParseChunkData(ReadOnlyBufferStream& inputStream, Chunk* chunk) override;
		};
	};
}