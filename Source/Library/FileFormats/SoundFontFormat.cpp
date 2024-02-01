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
			
			std::vector<SF_SampleHeader> sampleHeaderArray;
			while (true)
			{
				SF_SampleHeader header;
				if (!sampleHeaderStream.ReadBytesFromStream((uint8_t*)&header, sizeof(SF_SampleHeader)))
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
				std::vector<uint32_t> audioSampleIDArray;
				const SF_SampleHeader* header = &sampleHeaderArray[i];
				audioSampleIDArray.push_back(i);
				
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
						audioSampleIDArray.push_back(j);
						header = &sampleHeaderArray[j];
					}

					if (error)
						break;
				}

				SoundFontData::AudioSample* audioSample = this->ConstructAudioSample(sampleHeaderArray, audioSampleIDArray, smplChunk, sm24Chunk, error);
				if (!audioSample)
					break;

				soundFontData->audioSampleArray->push_back(audioSample);
			}

			if (error)
				break;
		}

		if (error)
			break;

		const ChunkParser::Chunk* igenChunk = parser.FindChunk("igen", false);
		if (!igenChunk)
		{
			error.Add("No \"igen\" chunk found.");
			return false;
		}

		if (igenChunk->GetBufferSize() % sizeof(SF_Generator) != 0)
		{
			error.Add("The \"igen\" chunk is not a multiple in size of the generator structure.");
			return false;
		}

		auto generatorArray = (const SF_Generator*)igenChunk->GetBuffer();
		uint32_t numGenerators = igenChunk->GetBufferSize() / sizeof(SF_Generator);
		SoundFontData::LoopedAudioData::Location location;
		::memset(&location, 0, sizeof(location));
		for (uint32_t i = 0; i < numGenerators; i++)
		{
			const SF_Generator* generator = &generatorArray[i];
			if (generator->op == ADL_GENERATOR_OP_END)
				break;

			switch (generator->op)
			{
				case ADL_GENERATOR_OP_KEY_RANGE:
				{
					location.minKey = generator->range.min;
					location.maxKey = generator->range.max;
					break;
				}
				case ADL_GENERATOR_OP_VEL_RANGE:
				{
					location.minVel = generator->range.min;
					location.maxVel = generator->range.max;
					break;
				}
				case ADL_GENERATOR_OP_SAMPLE_ID:
				{
					uint32_t sampleID = generator->amount;
					SoundFontData::LoopedAudioData* audioData = soundFontData->FindLoopedAudioData(sampleID);
					if (!audioData)
					{
						error.Add(FormatString("Failed to find audio data with sample ID %d", sampleID));
						break;
					}

					audioData->SetLocation(location);
					::memset(&location, 0, sizeof(location));
					break;
				}
			}

			if (error)
				break;
		}

		if (error)
			break;

		success = true;
	} while (false);

	if (success)
		fileData = soundFontData;
	else
		delete soundFontData;

	return success;
}

SoundFontData::AudioSample* SoundFontFormat::ConstructAudioSample(
	const std::vector<SF_SampleHeader>& audioSampleHeaderArray,
	const std::vector<uint32_t>& sampleIDArray,
	const ChunkParser::Chunk* smplChunk,
	const ChunkParser::Chunk* sm24Chunk,
	Error& error)
{
	if (sampleIDArray.size() == 0)
		return nullptr;

	// Sanity check the headers against the sample chunk.
	for (uint32_t i : sampleIDArray)
	{
		const SF_SampleHeader& header = audioSampleHeaderArray[i];

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

	// I'm not even going to check here that all the headers specify the same pitch.
	// It seems as though this is always set to 60 no matter what.
	audioSample->SetMIDIPitch(audioSampleHeaderArray[0].originalPitch);

	for (uint32_t i : sampleIDArray)
	{
		const SF_SampleHeader& header = audioSampleHeaderArray[i];

		auto loopedAudioData = new SoundFontData::LoopedAudioData();
		audioSample->loopedAudioDataArray->push_back(loopedAudioData);

		loopedAudioData->SetSampleID(i);
		loopedAudioData->SetName((const char*)header.sampleName);

		if ((header.sampleType & ADL_SAMPLE_TYPE_BIT_LEFT) != 0)
			loopedAudioData->SetChannelType(SoundFontData::LoopedAudioData::ChannelType::LEFT_EAR);
		else if((header.sampleType & ADL_SAMPLE_TYPE_BIT_RIGHT) != 0)
			loopedAudioData->SetChannelType(SoundFontData::LoopedAudioData::ChannelType::RIGHT_EAR);

		SoundFontData::LoopedAudioData::Loop loop;
		loop.startFrame = header.sampleLoopStart - header.sampleStart;
		loop.endFrame = header.sampleLoopEnd - header.sampleStart;
		loopedAudioData->SetLoop(loop);

		AudioData::Format& format = loopedAudioData->GetFormat();

		format.numChannels = 1;
		format.bitsPerSample = sm24Chunk ? 32 : 16;
		format.framesPerSecond = header.sampleRate;
		format.sampleType = AudioData::Format::SampleType::SIGNED_INTEGER;

		if (!sampleBuffer8)
		{
			loopedAudioData->SetAudioBufferSize((header.sampleEnd - header.sampleStart) * format.BytesPerFrame());
			uint64_t numFrames = loopedAudioData->GetNumFrames();
			uint16_t* frameBuffer = (uint16_t*)loopedAudioData->GetAudioBuffer();
			for (uint64_t i = 0; i < numFrames; i++)
				frameBuffer[i] = sampleBuffer16[header.sampleStart + i];
		}
		else
		{
			error.Add("Not yet implimented.  Until I get a sound-font file that has this case, I can't test it.");
			break;
		}
	}

	if (error)
	{
		delete audioSample;
		audioSample = nullptr;
	}

	return audioSample;
}

/*virtual*/ bool SoundFontFormat::WriteToStream(ByteStream& outputStream, const FileData* fileData, Error& error)
{
	error.Add("Not yet implimented.  (Probably never will be since SF files are insane.");
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