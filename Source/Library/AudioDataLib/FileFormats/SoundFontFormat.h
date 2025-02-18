#pragma once

#include "AudioDataLib/FileFormats/FileFormat.h"
#include "AudioDataLib/ChunkParser.h"
#include "AudioDataLib/FileDatas/WaveTableData.h"

#define ADL_SAMPLE_TYPE_BIT_MONO			0x0001
#define ADL_SAMPLE_TYPE_BIT_RIGHT			0x0002
#define ADL_SAMPLE_TYPE_BIT_LEFT			0x0004
#define ADL_SAMPLE_TYPE_BIT_LINKED			0x0008
#define ADL_SAMPLE_TYPE_BIT_ROM				0x8000

#define ADL_GENERATOR_OP_INSTRUMENT			41
#define ADL_GENERATOR_OP_KEY_RANGE			43
#define ADL_GENERATOR_OP_VEL_RANGE			44
#define ADL_GENERATOR_OP_FINE_TUNE			52
#define ADL_GENERATOR_OP_SAMPLE_ID			53
#define ADL_GENERATOR_OP_SAMPLE_MODES		54
#define ADL_GENERATOR_OP_ROOT_KEY			58
#define ADL_GENERATOR_OP_END				60

namespace AudioDataLib
{
	class Error;

	/**
	 * @brief This class knows how to read (and one day write) sound-font files.
	 * 
	 * Not all sound-font file features are supported here, but enough are to be useful in
	 * the goal of sound synthesis.  The SF2 file specification is quite extensive and lays
	 * out its own synthesis model.  I chose not to follow this model and instead, just extract
	 * just enough of the relevant information needed to perform my own method of synthesis.
	 * This means getting at the sample data, knowing how it's looped, knowing what the key
	 * and velocity ranges are, etc.  For now, I assume only one instrament per sound-font file,
	 * which I know is not generally correct.  I plan to revise this class once I get my hands
	 * on a sound-font file that contains multiple instruments.
	 */
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

		bool ConstructAudioSamples(
			SoundFontData* soundFontData,
			const std::vector<SF_SampleHeader>& audioSampleHeaderArray,
			const std::vector<uint32_t>& sampleIDArray,
			const ChunkParser::Chunk* smplChunk,
			const ChunkParser::Chunk* sm24Chunk,
			Error& error);

		typedef std::map<uint32_t, std::shared_ptr<SoundFontData::AudioSampleData>> SampleMap;
		SampleMap* sampleMap;
	};
}