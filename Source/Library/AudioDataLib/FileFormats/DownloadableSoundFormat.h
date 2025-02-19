#pragma once

#include "AudioDataLib/FileFormats/FileFormat.h"
#include "AudioDataLib/ChunkParser.h"

namespace AudioDataLib
{
	class WaveTableData;
	class Error;

	/**
	 * @brief This class knows how to read (and one day write) DLS files.
	 *
	 * DLS stands for downloadable sound.  It's a RIFF-based file that contains a bunch
	 * of instruments (timbers, you could say) and their associated sound samples, and
	 * details how those sounds are to be replayed as a function of the MIDI protocol.
	 */
	class AUDIO_DATA_LIB_API DownloadableSoundFormat : public FileFormat
	{
	public:
		DownloadableSoundFormat();
		virtual ~DownloadableSoundFormat();

		virtual bool ReadFromStream(ByteStream& inputStream, std::unique_ptr<FileData>& fileData) override;
		virtual bool WriteToStream(ByteStream& outputStream, const FileData* fileData) override;

	private:

#pragma pack(push, 1)
		struct DSL_Locale
		{
			uint32_t bank;
			uint32_t instrument;
		};

		struct DSL_InstrumentHeader
		{
			uint32_t numRegions;
			DSL_Locale midiLocale;
		};

		struct DSL_Range
		{
			uint16_t min;
			uint16_t max;
		};

		struct DSL_RegionHeader
		{
			DSL_Range keyRange;
			DSL_Range velRange;
			uint16_t synthOptions;
			uint16_t keyGroup;
		};

		struct DSL_WaveSample
		{
			uint32_t size;
			uint16_t unityNote;
			int16_t fineTune;
			int32_t attenuation;
			uint32_t synthOptions;
			uint32_t numSampleLoops;
		};

		struct DSL_WaveSampleLoop
		{
			uint32_t size;
			uint32_t loopType;
			uint32_t loopStart;
			uint32_t loopLength;
		};

		struct DSL_WaveLink
		{
			uint16_t synthOptions;
			uint16_t phaseGroup;
			uint32_t channel;
			uint32_t tableIndex;
		};

		struct DSL_PoolTableHeader
		{
			uint32_t size;
			uint32_t numCues;
		};
#pragma pack(pop)

		bool LoadInstrument(
					const ChunkParser::Chunk* instrumentChunk,
					const ChunkParser::Chunk* wavePoolChunk,
					WaveTableData* waveTableData);
	};
}