#pragma once

#include "AudioFileFormat.h"
#include "ChunkParser.h"

namespace AudioDataLib
{
	class AUDIO_DATA_LIB_API WaveFileFormat : public AudioFileFormat
	{
	public:
		WaveFileFormat();
		virtual ~WaveFileFormat();

		enum SampleFormat
		{
			PCM = 1,
			IEEE_FLOAT = 3
		};

		virtual bool ReadFromStream(ByteStream& inputStream, AudioData*& audioData, std::string& error) override;
		virtual bool WriteToStream(ByteStream& outputStream, AudioData* audioData, std::string& error) override;

	protected:
		class WaveChunkParser : public ChunkParser
		{
		public:
			WaveChunkParser();
			virtual ~WaveChunkParser();

			virtual bool ParseChunkData(BufferStream& inputStream, Chunk* chunk, std::string& error) override;
		};
	};
}