#include "AudioDataLib/FileFormats/SoundFontFormat.h"
#include "AudioDataLib/ErrorSystem.h"

using namespace AudioDataLib;

SoundFontFormat::SoundFontFormat()
{
	this->sampleMap = new SampleMap();
}

/*virtual*/ SoundFontFormat::~SoundFontFormat()
{
	delete this->sampleMap;
}

/*virtual*/ bool SoundFontFormat::ReadFromStream(ByteStream& inputStream, FileData*& fileData)
{
	SoundFontChunkParser parser;
	if (!parser.ParseStream(inputStream))
		return false;

	SoundFontData* soundFontData = new SoundFontData();
	bool success = false;
	SoundFontData::GeneralInfo& generalInfo = soundFontData->GetGeneralInfo();

	do
	{
		this->sampleMap->clear();

		const ChunkParser::Chunk* ifilChunk = parser.FindChunk("ifil", "", false);
		if (!ifilChunk)
		{
			ErrorSystem::Get()->Add("No \"ifil\" chunk found.");
			break;
		}

		if (ifilChunk->GetBufferSize() != sizeof(SoundFontData::VersionTag))
		{
			ErrorSystem::Get()->Add("The \"ifil\" chunk is the wrong size.");
			break;
		}

		::memcpy(&generalInfo.versionTag, ifilChunk->GetBuffer(), sizeof(SoundFontData::VersionTag));

		const ChunkParser::Chunk* isngChunk = parser.FindChunk("isng", "", false);
		if (isngChunk)
			generalInfo.waveTableSoundEngine.assign((const char*)isngChunk->GetBuffer(), isngChunk->GetBufferSize());

		const ChunkParser::Chunk* inamChunk = parser.FindChunk("INAM", "", false);
		if (!inamChunk)
		{
			ErrorSystem::Get()->Add("No \"inam\" chunk found.");
			break;
		}

		generalInfo.bankName.assign((const char*)inamChunk->GetBuffer(), inamChunk->GetBufferSize());

		const ChunkParser::Chunk* iromChunk = parser.FindChunk("irom", "", false);
		if (iromChunk)
			generalInfo.waveTableSoundDataROM.assign((const char*)iromChunk->GetBuffer(), iromChunk->GetBufferSize());

		const ChunkParser::Chunk* iverChunk = parser.FindChunk("iver", "", false);
		if (iverChunk)
		{
			if (iverChunk->GetBufferSize() != sizeof(SoundFontData::VersionTag))
			{
				ErrorSystem::Get()->Add("The \"iver\" chunk size is wrong.");
				break;
			}

			::memcpy(&generalInfo.waveTableROMVersion, iverChunk->GetBuffer(), iverChunk->GetBufferSize());
		}

		const ChunkParser::Chunk* icrdChunk = parser.FindChunk("ICRD", "", false);
		if (icrdChunk)
			generalInfo.creationDate.assign((const char*)icrdChunk->GetBuffer(), icrdChunk->GetBufferSize());

		const ChunkParser::Chunk* iengChunk = parser.FindChunk("IENG", "", false);
		if (iengChunk)
			generalInfo.soundEngineerNames.assign((const char*)iengChunk->GetBuffer(), iengChunk->GetBufferSize());

		const ChunkParser::Chunk* iprdChunk = parser.FindChunk("IPRD", "", false);
		if (iprdChunk)
			generalInfo.intendedProductName.assign((const char*)iprdChunk->GetBuffer(), iprdChunk->GetBufferSize());

		const ChunkParser::Chunk* icopChunk = parser.FindChunk("ICOP", "", false);
		if (icopChunk)
			generalInfo.copyrightClaim.assign((const char*)icopChunk->GetBuffer(), icopChunk->GetBufferSize());

		const ChunkParser::Chunk* icmtChunk = parser.FindChunk("ICMT", "", false);
		if (icmtChunk)
			generalInfo.comments.assign((const char*)icmtChunk->GetBuffer(), icmtChunk->GetBufferSize());

		const ChunkParser::Chunk* isftChunk = parser.FindChunk("ISFT", "", false);
		if (isftChunk)
			generalInfo.soundFontToolRecord.assign((const char*)isftChunk->GetBuffer(), isftChunk->GetBufferSize());

		const ChunkParser::Chunk* smplChunk = parser.FindChunk("smpl", "", false);
		if (smplChunk)
		{
			const ChunkParser::Chunk* sm24Chunk = parser.FindChunk("sm24", "", false);

			const ChunkParser::Chunk* shdrChunk = parser.FindChunk("shdr", "", false);
			if (!shdrChunk)
			{
				ErrorSystem::Get()->Add("Cannot parse the sample chunk if there is no SHDR chunk.");
				break;
			}

			ReadOnlyBufferStream sampleHeaderStream(shdrChunk->GetBuffer(), shdrChunk->GetBufferSize());
			
			std::vector<SF_SampleHeader> sampleHeaderArray;
			while (true)
			{
				SF_SampleHeader header;
				if (!sampleHeaderStream.ReadBytesFromStream((uint8_t*)&header, sizeof(SF_SampleHeader)))
				{
					ErrorSystem::Get()->Add("Failed to read sample header!");
					break;
				}

				if (0 == ::strcmp((const char*)header.sampleName, "EOS"))
					break;

				sampleHeaderArray.push_back(header);
			}

			if (ErrorSystem::Get()->Errors())
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
							ErrorSystem::Get()->Add("Bad link index.");
							break;
						}

						if (processedHeaderSet.find(j) != processedHeaderSet.end())
							break;

						processedHeaderSet.insert(j);
						audioSampleIDArray.push_back(j);
						header = &sampleHeaderArray[j];
					}

					if (ErrorSystem::Get()->Errors())
						break;
				}

				// We could associate this sub-group of samples, but I'm not currently doing that, and I'm not sure I ever will.
				if (!this->ConstructAudioSamples(soundFontData, sampleHeaderArray, audioSampleIDArray, smplChunk, sm24Chunk))
					break;
			}

			if (ErrorSystem::Get()->Errors())
				break;
		}

		if (ErrorSystem::Get()->Errors())
			break;

		const ChunkParser::Chunk* igenChunk = parser.FindChunk("igen", "", false);
		if (!igenChunk)
		{
			ErrorSystem::Get()->Add("No \"igen\" chunk found.");
			return false;
		}

		if (igenChunk->GetBufferSize() % sizeof(SF_Generator) != 0)
		{
			ErrorSystem::Get()->Add("The \"igen\" chunk is not a multiple in size of the generator structure.");
			return false;
		}

		auto generatorArray = (const SF_Generator*)igenChunk->GetBuffer();
		uint32_t numGenerators = igenChunk->GetBufferSize() / sizeof(SF_Generator);
		SoundFontData::AudioSampleData::Range range;
		::memset(&range, 0, sizeof(range));
		uint8_t looperMode = 0;		// TODO: Utilize this.
		int8_t overridingRootKey = -1;
		int16_t fineTune = 0;
		for (uint32_t i = 0; i < numGenerators; i++)
		{
			const SF_Generator* generator = &generatorArray[i];
			if (generator->op == ADL_GENERATOR_OP_END)
				break;

			switch (generator->op)
			{
				case ADL_GENERATOR_OP_KEY_RANGE:
				{
					range.minKey = generator->range.min;
					range.maxKey = generator->range.max;
					break;
				}
				case ADL_GENERATOR_OP_VEL_RANGE:
				{
					range.minVel = generator->range.min;
					range.maxVel = generator->range.max;
					break;
				}
				case ADL_GENERATOR_OP_FINE_TUNE:
				{
					fineTune = generator->signedAmount;
					break;
				}
				case ADL_GENERATOR_OP_SAMPLE_MODES:
				{
					// TODO: We don't seem to ever get here, so I'm not sure if this code is correct.  Maybe because it's only going to be found in the PMOD chunk?
					looperMode = generator->amount & 0xFF;
					break;
				}
				case ADL_GENERATOR_OP_ROOT_KEY:
				{
					overridingRootKey = (int8_t)generator->signedAmount;
					break;
				}
				case ADL_GENERATOR_OP_SAMPLE_ID:
				{
					uint32_t sampleID = generator->amount;
					auto iter = this->sampleMap->find(sampleID);
					if(iter == this->sampleMap->end())
					{
						ErrorSystem::Get()->Add(std::format("Failed to find audio data with sample ID {}.", sampleID));
						break;
					}

					SoundFontData::AudioSampleData* audioSampleData = iter->second.get();

					audioSampleData->SetRange(range);
					//audioSampleData->SetMode(SoundFontData::LoopedAudioData::Mode(looperMode));
					audioSampleData->GetCharacter().originalPitch = overridingRootKey;
					audioSampleData->GetCharacter().fineTuneCents = fineTune;

					::memset(&range, 0, sizeof(range));
					overridingRootKey = -1;
					looperMode = 0;
					fineTune = 0;
					break;
				}
			}

			if (ErrorSystem::Get()->Errors())
				break;
		}

		if (ErrorSystem::Get()->Errors())
			break;

		success = true;
	} while (false);

	// TODO: What about the instrument number on each sample?

	if (success)
		fileData = soundFontData;
	else
		delete soundFontData;

	return success;
}

bool SoundFontFormat::ConstructAudioSamples(
								SoundFontData* soundFontData,
								const std::vector<SF_SampleHeader>& audioSampleHeaderArray,
								const std::vector<uint32_t>& sampleIDArray,
								const ChunkParser::Chunk* smplChunk,
								const ChunkParser::Chunk* sm24Chunk)
{
	if (sampleIDArray.size() == 0)
	{
		ErrorSystem::Get()->Add("Was given empty sample ID array.");
		return false;
	}

	// Sanity check the headers against the sample chunk.
	for (uint32_t i : sampleIDArray)
	{
		const SF_SampleHeader& header = audioSampleHeaderArray[i];

		if (!(header.sampleStart <= header.sampleEnd && header.sampleEnd <= smplChunk->GetBufferSize() / sizeof(uint16_t)))
		{
			ErrorSystem::Get()->Add("Header boundaries not in range of the chunk.");
			return false;
		}

		if (!(header.sampleStart <= header.sampleLoopStart && header.sampleLoopStart <= header.sampleEnd) ||
			!(header.sampleStart <= header.sampleLoopEnd && header.sampleLoopEnd <= header.sampleEnd))
		{
			ErrorSystem::Get()->Add("Loop boundaries not contained within sample boundaries.");
			return false;
		}
	}

	const uint16_t* sampleBuffer16 = (const uint16_t*)smplChunk->GetBuffer();
	uint64_t sampleBuffer16Size = smplChunk->GetBufferSize() / sizeof(uint16_t);

	const uint8_t* sampleBuffer8 = sm24Chunk ? (const uint8_t*)sm24Chunk->GetBuffer() : nullptr;
	uint64_t sampleBuffer8Size = sm24Chunk ? (sm24Chunk->GetBufferSize() / sizeof(uint8_t)) : 0;

	if (sampleBuffer8 && sampleBuffer8Size != sampleBuffer16Size)
	{
		ErrorSystem::Get()->Add("The supplementary buffer is no the same size as the main buffer.");
		return false;
	}

	auto audioSampleData = new SoundFontData::AudioSampleData();

	for (uint32_t i : sampleIDArray)
	{
		const SF_SampleHeader& header = audioSampleHeaderArray[i];

		std::shared_ptr<SoundFontData::AudioSampleData> audioSampleData(new SoundFontData::AudioSampleData());
		soundFontData->AddSample(audioSampleData);

		this->sampleMap->insert(std::pair<uint32_t, std::shared_ptr<SoundFontData::AudioSampleData>>(i, audioSampleData));

		audioSampleData->SetName((const char*)header.sampleName);
		audioSampleData->GetCharacter().originalPitch = (int8_t)header.originalPitch;

		if ((header.sampleType & ADL_SAMPLE_TYPE_BIT_LEFT) != 0)
			audioSampleData->SetChannelType(SoundFontData::AudioSampleData::ChannelType::LEFT_EAR);
		else if ((header.sampleType & ADL_SAMPLE_TYPE_BIT_RIGHT) != 0)
			audioSampleData->SetChannelType(SoundFontData::AudioSampleData::ChannelType::RIGHT_EAR);
		else if (sampleIDArray.size() == 1)
			audioSampleData->SetChannelType(SoundFontData::AudioSampleData::ChannelType::MONO);

		SoundFontData::AudioSampleData::Loop loop;
		loop.startFrame = header.sampleLoopStart - header.sampleStart;
		loop.endFrame = header.sampleLoopEnd - header.sampleStart;
		audioSampleData->SetLoop(loop);

		if (loop.startFrame == 0 || loop.endFrame == header.sampleEnd - header.sampleStart)
			audioSampleData->SetMode(SoundFontData::AudioSampleData::Mode::NOT_LOOPED);
		else
			audioSampleData->SetMode(SoundFontData::AudioSampleData::Mode::GETS_TRAPPED_IN_LOOP);

		AudioData::Format& format = audioSampleData->GetFormat();

		format.numChannels = 1;
		format.bitsPerSample = sm24Chunk ? 32 : 16;
		format.framesPerSecond = header.sampleRate;
		format.sampleType = AudioData::Format::SampleType::SIGNED_INTEGER;

		if (!sampleBuffer8)
		{
			audioSampleData->SetAudioBufferSize((header.sampleEnd - header.sampleStart) * format.BytesPerFrame());
			uint64_t numFrames = audioSampleData->GetNumFrames();
			uint16_t* frameBuffer = (uint16_t*)audioSampleData->GetAudioBuffer();
			for (uint64_t i = 0; i < numFrames; i++)
				frameBuffer[i] = sampleBuffer16[header.sampleStart + i];
		}
		else
		{
			ErrorSystem::Get()->Add("Not yet implimented.  Until I get a sound-font file that has this case, I can't test it.");
			break;
		}
	}

	return true;
}

/*virtual*/ bool SoundFontFormat::WriteToStream(ByteStream& outputStream, const FileData* fileData)
{
	ErrorSystem::Get()->Add("Not yet implimented.  (Probably never will be since SF files are insane.");
	return false;
}

SoundFontFormat::SoundFontChunkParser::SoundFontChunkParser()
{
}

/*virtual*/ SoundFontFormat::SoundFontChunkParser::~SoundFontChunkParser()
{
}

/*virtual*/ bool SoundFontFormat::SoundFontChunkParser::ParseChunkData(ReadOnlyBufferStream& inputStream, Chunk* chunk)
{
	if (chunk->GetName() == "RIFF")
	{
		char formType[5];
		if (4 != inputStream.ReadBytesFromStream((uint8_t*)formType, 4))
		{
			ErrorSystem::Get()->Add("Could not read form type of RIFF chunk.");
			return false;
		}

		formType[4] = '\0';

		if (0 != strcmp(formType, "sfbk"))
		{
			ErrorSystem::Get()->Add("RIFF file does not appears to be a sound-font bank file.");
			return false;
		}

		if (!chunk->ParseSubChunks(inputStream, this))
			return false;
	}
	else if (chunk->GetName() == "LIST")
	{
		char formType[5];
		if (4 != inputStream.ReadBytesFromStream((uint8_t*)formType, 4))
		{
			ErrorSystem::Get()->Add("Could not read form type of LIST chunk.");
			return false;
		}

		formType[4] = '\0';

		if (0 != strcmp(formType, "INFO") && 0 != strcmp(formType, "sdta") && 0 != strcmp(formType, "pdta"))
		{
			ErrorSystem::Get()->Add("Expected form type of LIST chunk to be INFO, \"stda\" or \"pdta\".");
			return false;
		}

		if (!chunk->ParseSubChunks(inputStream, this))
			return false;
	}
	else
	{
		if (!inputStream.SetReadOffset(inputStream.GetReadOffset() + chunk->GetBufferSize()))
		{
			ErrorSystem::Get()->Add("Could not skip over chunk data.");
			return false;
		}
	}

	return true;
}