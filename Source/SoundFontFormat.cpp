#include "SoundFontFormat.h"
#include "SoundFontData.h"
#include "Error.h"

using namespace AudioDataLib;

SoundFontFormat::SoundFontFormat()
{
}

/*virtual*/ SoundFontFormat::~SoundFontFormat()
{
}

/*virtual*/ bool SoundFontFormat::ReadFromStream(ByteStream& inputStream, FileData*& fileData, Error& error)
{
	SoundFontChunkParser parser;
	if (!parser.ParseStream(inputStream, error))
		return false;

	SoundFontData* soundFontData = new SoundFontData();
	bool success = false;
	SoundFontData::GeneralInfo& generalInfo = soundFontData->GetGeneralInfo();

	do
	{
		const ChunkParser::Chunk* ifilChunk = parser.FindChunk("ifil", false);
		if (!ifilChunk)
		{
			error.Add("No \"ifil\" chunk found.");
			break;
		}

		if (ifilChunk->GetBufferSize() != sizeof(SoundFontData::VersionTag))
		{
			error.Add("The \"ifil\" chunk is the wrong size.");
			break;
		}

		::memcpy(&generalInfo.versionTag, ifilChunk->GetBuffer(), sizeof(SoundFontData::VersionTag));

		const ChunkParser::Chunk* isngChunk = parser.FindChunk("isng", false);
		if (!isngChunk)
		{
			error.Add("No \"isng\" chunk found.");
			break;
		}

		generalInfo.waveTableSoundEngine.assign((const char*)isngChunk->GetBuffer(), isngChunk->GetBufferSize());

		const ChunkParser::Chunk* inamChunk = parser.FindChunk("INAM", false);
		if (!inamChunk)
		{
			error.Add("No \"inam\" chunk found.");
			break;
		}

		generalInfo.bankName.assign((const char*)inamChunk->GetBuffer(), inamChunk->GetBufferSize());

		const ChunkParser::Chunk* iromChunk = parser.FindChunk("irom", false);
		if (iromChunk)
			generalInfo.waveTableSoundDataROM.assign((const char*)iromChunk->GetBuffer(), iromChunk->GetBufferSize());

		const ChunkParser::Chunk* iverChunk = parser.FindChunk("iver", false);
		if (iverChunk)
		{
			if (iverChunk->GetBufferSize() != sizeof(SoundFontData::VersionTag))
			{
				error.Add("The \"iver\" chunk size is wrong.");
				break;
			}

			::memcpy(&generalInfo.waveTableROMVersion, iverChunk->GetBuffer(), iverChunk->GetBufferSize());
		}

		const ChunkParser::Chunk* icrdChunk = parser.FindChunk("ICRD", false);
		if (icrdChunk)
			generalInfo.creationDate.assign((const char*)icrdChunk->GetBuffer(), icrdChunk->GetBufferSize());

		const ChunkParser::Chunk* iengChunk = parser.FindChunk("IENG", false);
		if (iengChunk)
			generalInfo.soundEngineerNames.assign((const char*)iengChunk->GetBuffer(), iengChunk->GetBufferSize());

		const ChunkParser::Chunk* iprdChunk = parser.FindChunk("IPRD", false);
		if (iprdChunk)
			generalInfo.intendedProductName.assign((const char*)iprdChunk->GetBuffer(), iprdChunk->GetBufferSize());

		const ChunkParser::Chunk* icopChunk = parser.FindChunk("ICOP", false);
		if (icopChunk)
			generalInfo.copyrightClaim.assign((const char*)icopChunk->GetBuffer(), icopChunk->GetBufferSize());

		const ChunkParser::Chunk* icmtChunk = parser.FindChunk("ICMT");
		if (icmtChunk)
			generalInfo.comments.assign((const char*)icmtChunk->GetBuffer(), icmtChunk->GetBufferSize());

		const ChunkParser::Chunk* isftChunk = parser.FindChunk("ISFT", false);
		if (isftChunk)
			generalInfo.soundFontToolRecord.assign((const char*)isftChunk->GetBuffer(), isftChunk->GetBufferSize());

		const ChunkParser::Chunk* smplChunk = parser.FindChunk("smpl", false);
		if (smplChunk)
		{
			const ChunkParser::Chunk* sm24Chunk = parser.FindChunk("sm24", false);

			const ChunkParser::Chunk* shdrChunk = parser.FindChunk("shdr", false);
			if (!shdrChunk)
			{
				error.Add("Cannot parse the sample chunk if there is no SHDR chunk.");
				break;
			}

			ReadOnlyBufferStream sampleHeaderStream(shdrChunk->GetBuffer(), shdrChunk->GetBufferSize());
			
			std::vector<SampleHeader> sampleHeaderArray;
			while (true)
			{
				SampleHeader header;
				if (!sampleHeaderStream.ReadBytesFromStream((uint8_t*)&header, sizeof(SampleHeader)))
				{
					error.Add("Failed to read sample header!");
					break;
				}

				if (0 == ::strcmp((const char*)header.sampleName, "EOS"))
					break;

				sampleHeaderArray.push_back(header);
			}

			if (error)
				break;

			std::set<uint32_t> processedHeaderSet;
			for (uint32_t i = 0; i < sampleHeaderArray.size(); i++)
			{
				if (processedHeaderSet.find(i) != processedHeaderSet.end())
					continue;

				processedHeaderSet.insert(i);
				std::vector<SampleHeader> audioSampleHeaderArray;
				const SampleHeader* header = &sampleHeaderArray[i];
				audioSampleHeaderArray.push_back(*header);
				
				if ((header->sampleType & ADL_SAMPLE_TYPE_BIT_ROM) != 0)
					continue;	// Just ignore ROM samples for now.

				if (header->sampleType != ADL_SAMPLE_TYPE_BIT_MONO)
				{
					while (true)
					{
						uint32_t j = header->sampleLink;
						if (j >= sampleHeaderArray.size())
						{
							error.Add("Bad link index.");
							break;
						}

						if (processedHeaderSet.find(j) != processedHeaderSet.end())
							break;

						processedHeaderSet.insert(j);
						header = &sampleHeaderArray[j];
						audioSampleHeaderArray.push_back(*header);
					}

					if (error)
						break;
				}

				SoundFontData::AudioSample* audioSample = this->ConstructAudioSample(audioSampleHeaderArray, smplChunk, sm24Chunk, error);
				if (!audioSample)
					break;

				soundFontData->audioSampleArray->push_back(audioSample);
			}

			if (error)
				break;
		}

		success = true;
	} while (false);

	if (success)
		fileData = soundFontData;
	else
		delete soundFontData;

	return success;
}

SoundFontData::AudioSample* SoundFontFormat::ConstructAudioSample(const std::vector<SampleHeader>& audioSampleHeaderArray, const ChunkParser::Chunk* smplChunk, const ChunkParser::Chunk* sm24Chunk, Error& error)
{
	if (audioSampleHeaderArray.size() == 0)
		return nullptr;

	// Sanity check the headers against the sample chunk.
	for (const SampleHeader& header : audioSampleHeaderArray)
	{
		if (!(header.sampleStart <= header.sampleEnd && header.sampleEnd <= smplChunk->GetBufferSize() / sizeof(uint16_t)))
		{
			error.Add("Header boundaries not in range of the chunk.");
			return nullptr;
		}

		if (!(header.sampleStart <= header.sampleLoopStart && header.sampleLoopStart <= header.sampleEnd) ||
			!(header.sampleStart <= header.sampleLoopEnd && header.sampleLoopEnd <= header.sampleEnd))
		{
			error.Add("Loop boundaries not contained within sample boundaries.");
			return nullptr;
		}
	}

	// I suppose this uniformity is not really necessary, and it may be up to us to resample?
	uint32_t sampleRate = audioSampleHeaderArray[0].sampleRate;
	uint64_t sampleBufferSizeFrames = audioSampleHeaderArray[0].sampleEnd - audioSampleHeaderArray[0].sampleStart;
	uint64_t loopSizeFrames = audioSampleHeaderArray[0].sampleLoopEnd - audioSampleHeaderArray[0].sampleLoopStart;
	uint64_t loopBaseFrames = audioSampleHeaderArray[0].sampleLoopStart - audioSampleHeaderArray[0].sampleStart;
	for (uint32_t i = 1; i < audioSampleHeaderArray.size(); i++)
	{
		uint32_t otherSampleRate = audioSampleHeaderArray[i].sampleRate;
		uint64_t otherSampleBufferSizeFrames = audioSampleHeaderArray[i].sampleEnd - audioSampleHeaderArray[i].sampleStart;
		uint64_t otherLoopSizeFrames = audioSampleHeaderArray[i].sampleLoopEnd - audioSampleHeaderArray[i].sampleLoopStart;
		uint64_t otherLoopBaseFrames = audioSampleHeaderArray[i].sampleLoopStart - audioSampleHeaderArray[i].sampleStart;

		if (sampleRate != otherSampleRate)
		{
			error.Add("Not all headers for a sample share the same sample-rate.");
			return nullptr;
		}

		if (sampleBufferSizeFrames != otherSampleBufferSizeFrames)
		{
			error.Add("Not all headers for a sample share the same sample buffer size.");
			return nullptr;
		}

		if (loopSizeFrames != otherLoopSizeFrames)
		{
			error.Add("Not all headers for a sample have the same loop size.");
			return nullptr;
		}

		if (loopBaseFrames != otherLoopBaseFrames)
		{
			error.Add("Not all headers for a sample have the same loop base.");
			return nullptr;
		}
	}

	const uint16_t* sampleBuffer16 = (const uint16_t*)smplChunk->GetBuffer();
	uint64_t sampleBuffer16Size = smplChunk->GetBufferSize() / sizeof(uint16_t);

	const uint8_t* sampleBuffer8 = sm24Chunk ? (const uint8_t*)sm24Chunk->GetBuffer() : nullptr;
	uint64_t sampleBuffer8Size = sm24Chunk ? (sm24Chunk->GetBufferSize() / sizeof(uint8_t)) : 0;

	if (sampleBuffer8 && sampleBuffer8Size != sampleBuffer16Size)
	{
		error.Add("The supplementary buffer is no the same size as the main buffer.");
		return nullptr;
	}

	auto audioSample = new SoundFontData::AudioSample();

	std::string name;

	for (const SampleHeader& header : audioSampleHeaderArray)
	{
		if (name.length() == 0)
			name.assign((const char*)header.sampleName);
		else
		{
			std::string otherName((const char*)header.sampleName);
			name += "|" + otherName;
		}
	}

	audioSample->SetName(name);
	audioSample->SetLoop(SoundFontData::AudioSample::Loop{ loopBaseFrames, loopBaseFrames + loopSizeFrames });

	AudioData* audioData = audioSample->GetAudioData();
	AudioData::Format& format = audioData->GetFormat();

	format.numChannels = (uint16_t)audioSampleHeaderArray.size();
	format.bitsPerSample = sm24Chunk ? 32 : 16;
	format.framesPerSecond = audioSampleHeaderArray[0].sampleRate;
	format.sampleType = AudioData::Format::SampleType::SIGNED_INTEGER;

	if (!sampleBuffer8)
	{
		audioData->SetAudioBufferSize(sampleBufferSizeFrames * format.BytesPerFrame());
		uint64_t samplesPerFrame = format.SamplesPerFrame();
		uint64_t numFrames = audioData->GetNumFrames();
		uint16_t* sampleBuffer = (uint16_t*)audioData->GetAudioBuffer();
		assert(numFrames == sampleBufferSizeFrames);
		
		for (uint64_t i = 0; i < numFrames; i++)
		{
			uint64_t j = 0;
			for (const SampleHeader& header : audioSampleHeaderArray)
				sampleBuffer[j++] = sampleBuffer16[header.sampleStart + i];

			sampleBuffer += samplesPerFrame;
		}
	}
	else
	{
		error.Add("Not yet implimented.  Until I get a sound-font file that has this case, I can't test it.");
		delete audioSample;
		audioSample = nullptr;
	}

	return audioSample;
}

/*virtual*/ bool SoundFontFormat::WriteToStream(ByteStream& outputStream, FileData* fileData, Error& error)
{
	return false;
}

SoundFontFormat::SoundFontChunkParser::SoundFontChunkParser()
{
}

/*virtual*/ SoundFontFormat::SoundFontChunkParser::~SoundFontChunkParser()
{
}

/*virtual*/ bool SoundFontFormat::SoundFontChunkParser::ParseChunkData(ReadOnlyBufferStream& inputStream, Chunk* chunk, Error& error)
{
	if (chunk->GetName() == "RIFF")
	{
		char formType[5];
		if (4 != inputStream.ReadBytesFromStream((uint8_t*)formType, 4))
		{
			error.Add("Could not read form byte of RIFF chunk.");
			return false;
		}

		formType[4] = '\0';

		if (0 != strcmp(formType, "sfbk"))
		{
			error.Add("RIFF file does not appears to be a sound-font bank file.");
			return false;
		}

		if (!chunk->ParseSubChunks(inputStream, this, error))
			return false;
	}
	else if (chunk->GetName() == "LIST")
	{
		char formType[5];
		if (4 != inputStream.ReadBytesFromStream((uint8_t*)formType, 4))
		{
			error.Add("Could not read form byte of LIST chunk.");
			return false;
		}

		formType[4] = '\0';

		if (0 != strcmp(formType, "INFO") && 0 != strcmp(formType, "sdta") && 0 != strcmp(formType, "pdta"))
		{
			error.Add("Expected form type of LIST chunk to be INFO, \"stda\" or \"pdta\".");
			return false;
		}

		if (!chunk->ParseSubChunks(inputStream, this, error))
			return false;
	}
	else
	{
		if (!inputStream.SetReadOffset(inputStream.GetReadOffset() + chunk->GetBufferSize()))
		{
			error.Add("Could not skip over chunk data.");
			return false;
		}
	}

	return true;
}