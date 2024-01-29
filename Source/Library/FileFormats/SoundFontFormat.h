#pragma once

#include "FileFormat.h"
#include "ChunkParser.h"
#include "SoundFontData.h"

#define ADL_SAMPLE_TYPE_BIT_MONO			0x0001
#define ADL_SAMPLE_TYPE_BIT_RIGHT			0x0002
#define ADL_SAMPLE_TYPE_BIT_LEFT			0x0004
#define ADL_SAMPLE_TYPE_BIT_LINKED			0x0008
#define ADL_SAMPLE_TYPE_BIT_ROM				0x8000

#define ADL_GENERATOR_OP_KEY_RANGE			43
#define ADL_GENERATOR_OP_VEL_RANGE			44
#define ADL_GENERATOR_OP_SAMPLE_ID			53

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

		bool ReadCrazyData(ChunkParser& parser, char prefix, Error& error);

#pragma pack(push, 1)
		struct SF_Instrument
		{
			int8_t name[20];
			uint16_t bagIndex;
		};
		
		struct SF_Preset
		{
			int8_t name[20];
			uint16_t preset;
			uint16_t bank;
			uint16_t bagIndex;
			uint32_t library;
			uint32_t genre;
			uint32_t morphology;
		};

		struct SF_Bag
		{
			uint16_t generatorIndex;
			uint16_t modulatorIndex;
		};

		struct SF_Modulator
		{
			uint16_t sourceOp;
			uint16_t destinationOp;
			int16_t amount;
			uint16_t amountSrcOp;
			uint16_t transOp;
		};

		struct SF_Generator
		{
			uint16_t op;
			union
			{
				uint16_t amount;
				int16_t signedAmount;
				struct
				{
					uint8_t min;
					uint8_t max;
				} range;
			};
		};

		struct SF_SampleHeader
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

		SoundFontData::PitchData* ConstructPitchData(const std::vector<SF_SampleHeader>& pitchSampleHeaderArray, const ChunkParser::Chunk* smplChunk, const ChunkParser::Chunk* sm24Chunk, Error& error);
	};
}