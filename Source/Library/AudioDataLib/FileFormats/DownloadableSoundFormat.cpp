#include "AudioDataLib/FileFormats/DownloadableSoundFormat.h"
#include "AudioDataLib/FileDatas/WaveTableData.h"
#include "AudioDataLib/FileFormats/WaveFileFormat.h"
#include "AudioDataLib/ErrorSystem.h"

using namespace AudioDataLib;

//---------------------------------------- DownloadableSoundFormat ----------------------------------------

DownloadableSoundFormat::DownloadableSoundFormat()
{
}

/*virtual*/ DownloadableSoundFormat::~DownloadableSoundFormat()
{
}

/*virtual*/ bool DownloadableSoundFormat::ReadFromStream(ByteStream& inputStream, std::unique_ptr<FileData>& fileData)
{
	fileData = nullptr;

	ChunkParser parser;
	parser.RegisterSubChunks("RIFF");
	parser.RegisterSubChunks("LIST");
	if (!parser.ParseStream(inputStream))
		return false;

	const ChunkParser::Chunk* chunk = parser.FindChunk("RIFF");
	if (!chunk)
	{
		ErrorSystem::Get()->Add("No RIFF chunk found!");
		return false;
	}

	if (chunk->GetFormType() != "DLS ")
	{
		ErrorSystem::Get()->Add("RIFF file does not appear to be a DLS file.");
		return false;
	}

	const ChunkParser::Chunk* headerChunk = parser.FindChunk("colh");
	if (!headerChunk)
	{
		ErrorSystem::Get()->Add("Did not find a header chunk.");
		return false;
	}

	if (headerChunk->GetBufferSize() != sizeof(uint32_t))
	{
		ErrorSystem::Get()->Add("Header size was not 4 bytes.");
		return false;
	}

	uint32_t numInstruments = *(uint32_t*)headerChunk->GetBuffer();

	const ChunkParser::Chunk* instrumentListChunk = chunk->FindChunk("LIST", "lins", true);
	if (!instrumentListChunk)
	{
		ErrorSystem::Get()->Add("Did not find instrument list chunk.");
		return false;
	}

	if (instrumentListChunk->GetNumSubChunks() != numInstruments)
	{
		ErrorSystem::Get()->Add(std::format("Header says there are {} instruments, but instrument list chunk has {} sub-chunks", numInstruments, instrumentListChunk->GetNumSubChunks()));
		return false;
	}

	const ChunkParser::Chunk* wavePoolChunk = chunk->FindChunk("LIST", "wvpl", true);
	if (!wavePoolChunk)
	{
		ErrorSystem::Get()->Add("Could not find wave-pool chunk.");
		return false;
	}

	std::unique_ptr<WaveTableData> waveTableData(new WaveTableData());

	for (uint32_t i = 0; i < numInstruments; i++)
	{
		const ChunkParser::Chunk* instrumentChunk = instrumentListChunk->GetSubChunkArray()[i];
		if (!this->LoadInstrument(instrumentChunk, wavePoolChunk, waveTableData.get()))
			break;
	}

	if (ErrorSystem::Get()->Errors())
		return false;

	fileData.reset(waveTableData.release());
	return true;
}

bool DownloadableSoundFormat::LoadInstrument(
				const ChunkParser::Chunk* instrumentChunk,
				const ChunkParser::Chunk* wavePoolChunk,
				WaveTableData* waveTableData)
{
	const ChunkParser::Chunk* instrumentHeaderChunk = instrumentChunk->FindChunk("insh", "", true);
	if (!instrumentHeaderChunk)
	{
		ErrorSystem::Get()->Add("Failed to find instrument header chunk.");
		return false;
	}

	DSL_InstrumentHeader instrumentHeader;

	if (instrumentHeaderChunk->GetBufferSize() != sizeof(instrumentHeader))
	{
		ErrorSystem::Get()->Add(std::format("Instrument header chunk size was {}, but expected {}.", instrumentHeaderChunk->GetBufferSize(), sizeof(instrumentHeader)));
		return false;
	}

	::memcpy(&instrumentHeader, instrumentHeaderChunk->GetBuffer(), sizeof(instrumentHeader));

	std::string instrumentName = "?";
	const ChunkParser::Chunk* nameChunk = instrumentChunk->FindChunk("INAM", "", true);
	if (nameChunk)
		instrumentName.assign((const char*)nameChunk->GetBuffer(), nameChunk->GetBufferSize());

	const ChunkParser::Chunk* regionListChunk = instrumentChunk->FindChunk("LIST", "lrgn", true);
	if (!regionListChunk)
	{
		ErrorSystem::Get()->Add("Did not find region list chunk.");
		return false;
	}

	if (regionListChunk->GetNumSubChunks() != instrumentHeader.numRegions)
	{
		ErrorSystem::Get()->Add(std::format("Instrument header says there are {} region, but there are {} sub-chunks of the region list chunk.", instrumentHeader.numRegions, regionListChunk->GetNumSubChunks()));
		return false;
	}

	for (uint32_t i = 0; i < instrumentHeader.numRegions; i++)
	{
		const ChunkParser::Chunk* regionChunk = regionListChunk->GetSubChunkArray()[i];

		const ChunkParser::Chunk* regionHeaderChunk = regionChunk->FindChunk("rgnh", "", true);
		if (!regionHeaderChunk)
		{
			ErrorSystem::Get()->Add("Failed to find region header chunk.");
			return false;
		}

		DSL_RegionHeader regionHeader;

		if (regionHeaderChunk->GetBufferSize() != sizeof(regionHeader))
		{
			ErrorSystem::Get()->Add(std::format("Expected region header size to be {}, but region header chunk size is {}.", sizeof(regionHeader), regionHeaderChunk->GetBufferSize()));
			return false;
		}

		::memcpy(&regionHeader, regionHeaderChunk->GetBuffer(), sizeof(regionHeader));

		const ChunkParser::Chunk* waveSampleChunk = regionChunk->FindChunk("wsmp", "", true);
		if (!waveSampleChunk)
		{
			ErrorSystem::Get()->Add("Failed to find wave-sample chunk.");
			return false;
		}

		DSL_WaveSample waveSample;

		if (waveSampleChunk->GetBufferSize() < sizeof(waveSample))
		{
			ErrorSystem::Get()->Add(std::format("Sizeof wave sample structure ({}) is bigger than the buffer size ({}).", sizeof(waveSample), waveSampleChunk->GetBufferSize()));
			return false;
		}

		::memcpy(&waveSample, waveSampleChunk->GetBuffer(), sizeof(waveSample));

		if (waveSampleChunk->GetBufferSize() != sizeof(waveSample) + waveSample.numSampleLoops * sizeof(DSL_WaveSampleLoop))
		{
			ErrorSystem::Get()->Add(std::format("Wave sample chunk has a size ({}) inconsistent with how many loops ({}) it claims to have.", waveSampleChunk->GetBufferSize(), waveSample.numSampleLoops));
			return false;
		}

		if (waveSample.numSampleLoops != 0 && waveSample.numSampleLoops != 1)
		{
			ErrorSystem::Get()->Add(std::format("Can't yet handle wave samples with {} loops.  Only zero or one is understood for now.", waveSample.numSampleLoops));
			return false;
		}

		auto waveSampleLoop = (waveSample.numSampleLoops == 1) ? (const DSL_WaveSampleLoop*)(waveSampleChunk->GetBuffer() + sizeof(waveSample)) : nullptr;

		const ChunkParser::Chunk* waveLinkChunk = regionChunk->FindChunk("wlnk", "", true);
		if (!waveLinkChunk)
		{
			ErrorSystem::Get()->Add("Failed to find wave-link chunk.");
			return false;
		}

		DSL_WaveLink waveLink;

		if (waveLinkChunk->GetBufferSize() != sizeof(waveLink))
		{
			ErrorSystem::Get()->Add(std::format("Expected wave-link chunk to be of size {}, but it is of size {}.", sizeof(waveLink), waveLinkChunk->GetBufferSize()));
			return false;
		}

		::memcpy(&waveLink, waveLinkChunk->GetBuffer(), sizeof(waveLink));

		std::shared_ptr<WaveTableData::AudioSampleData> audioSampleData(new WaveTableData::AudioSampleData());
		WaveTableData::AudioSampleData::Character character;
		character.instrument = instrumentHeader.midiLocale.instrument;
		character.originalPitch = waveSample.unityNote;
		character.fineTuneCents = waveSample.fineTune;
		audioSampleData->SetCharacter(character);

		WaveTableData::AudioSampleData::Range range;
		range.minKey = regionHeader.keyRange.min;
		range.maxKey = regionHeader.keyRange.max;
		range.minVel = regionHeader.velRange.min;
		range.maxVel = regionHeader.velRange.max;
		audioSampleData->SetRange(range);

		WaveTableData::AudioSampleData::Mode mode = WaveTableData::AudioSampleData::Mode::NOT_LOOPED;
		if (waveSampleLoop)
			mode = WaveTableData::AudioSampleData::Mode::GETS_TRAPPED_IN_LOOP;
		audioSampleData->SetMode(mode);

		if (waveSampleLoop)
		{
			WaveTableData::AudioSampleData::Loop loop;
			loop.startFrame = waveSampleLoop->loopStart;
			loop.endFrame = waveSampleLoop->loopStart + waveSampleLoop->loopLength;
			audioSampleData->SetLoop(loop);
		}

		if (waveLink.tableIndex >= wavePoolChunk->GetNumSubChunks())
		{
			ErrorSystem::Get()->Add(std::format("Cue index ({}) is out of range (0 -- {}).", waveLink.tableIndex, wavePoolChunk->GetNumSubChunks()));
			return false;
		}

		const ChunkParser::Chunk* waveChunk = wavePoolChunk->GetSubChunkArray()[waveLink.tableIndex];

		if (!WaveFileFormat::LoadWaveData(audioSampleData.get(), waveChunk))
		{
			ErrorSystem::Get()->Add(std::format("Failed to load wave data for instrument {}.", audioSampleData->GetCharacter().instrument));
			return false;
		}

		char sampleName[256];
		sprintf(sampleName, "%s_sample_%d", instrumentName.c_str(), i);
		audioSampleData->SetName(sampleName);

		waveTableData->AddSample(audioSampleData);
	}

	return true;
}

/*virtual*/ bool DownloadableSoundFormat::WriteToStream(ByteStream& outputStream, const FileData* fileData)
{
	ErrorSystem::Get()->Add("Not yet implimented.");
	return false;
}