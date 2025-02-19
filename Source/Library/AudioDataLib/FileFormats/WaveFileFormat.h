#pragma once

#include "AudioDataLib/FileFormats/FileFormat.h"
#include "AudioDataLib/ChunkParser.h"

namespace AudioDataLib
{
	/**
	 * @brief This class knows how to read and write WAVE files.
	 * 
	 * Wave files are a common, uncompressed (and therefore lossless) audio format,
	 * containing audio data in PCM (pulse code modulation) or floating-point format.
	 */
	class AUDIO_DATA_LIB_API WaveFileFormat : public FileFormat
	{
		friend class DownloadableSoundFormat;

	public:
		WaveFileFormat();
		virtual ~WaveFileFormat();

		enum SampleFormat
		{
			PCM = 1,
			IEEE_FLOAT = 3,
			A_LAW = 6,
			U_LAW = 7,
			EXTENSIBLE = 0xFFFE
		};

		virtual bool ReadFromStream(ByteStream& inputStream, std::unique_ptr<FileData>& fileData) override;
		virtual bool WriteToStream(ByteStream& outputStream, const FileData* fileData) override;

	protected:
		static bool LoadWaveData(AudioData* audioData, const ChunkParser::Chunk* waveChunk);

		class WaveChunkParser : public ChunkParser
		{
		public:
			WaveChunkParser();
			virtual ~WaveChunkParser();

			virtual bool ParseChunkData(ReadOnlyBufferStream& inputStream, Chunk* chunk) override;
		};
	};
}