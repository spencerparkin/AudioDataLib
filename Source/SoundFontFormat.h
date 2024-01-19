#pragma once

#include "FileFormat.h"
#include "ChunkParser.h"
#include "SoundFontData.h"

#define ADL_SAMPLE_TYPE_BIT_MONO			0x0001
#define ADL_SAMPLE_TYPE_BIT_RIGHT			0x0002
#define ADL_SAMPLE_TYPE_BIT_LEFT			0x0004
#define ADL_SAMPLE_TYPE_BIT_LINKED			0x0008
#define ADL_SAMPLE_TYPE_BIT_ROM				0x8000

namespace AudioDataLib
{
	class Error;

	class AUDIO_DATA_LIB_API SoundFontFormat : public FileFormat
	{
	public:
		SoundFontFormat();
		virtual ~SoundFontFormat();

		virtual bool ReadFromStream(ByteStream& inputStream, FileData*& fileData, Error& error) override;
		virtual bool WriteToStream(ByteStream& outputStream, const FileData* fileData, Error& error) override;

	private:
		class SoundFontChunkParser : public ChunkParser
		{
		public:
			SoundFontChunkParser();
			virtual ~SoundFontChunkParser();

			virtual bool ParseChunkData(ReadOnlyBufferStream& inputStream, Chunk* chunk, Error& error) override;
		};

		// This structure matches the layout of the data in the file.
#pragma pack(push, 1)
		struct SampleHeader
		{
			uint8_t sampleName[20];
			uint32_t sampleStart;
			uint32_t sampleEnd;
			uint32_t sampleLoopStart;
			uint32_t sampleLoopEnd;
			uint32_t sampleRate;
			uint8_t originalPitch;
			int8_t pitchCorrection;
			uint16_t sampleLink;
			uint16_t sampleType;
		};
#pragma pack(pop)

		SoundFontData::AudioSample* ConstructAudioSample(const std::vector<SampleHeader>& audioSampleHeaderArray, const ChunkParser::Chunk* smplChunk, const ChunkParser::Chunk* sm24Chunk, Error& error);
	};
}