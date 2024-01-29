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

#if false		// TODO: Right now, I don't know how to make heads or tails of this data.
		if (!this->ReadCrazyData(parser, 'i', error))
			break;

		if (!this->ReadCrazyData(parser, 'p', error))
			break;
#endif

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
				std::vector<SF_SampleHeader> pitchSampleHeaderArray;
				const SF_SampleHeader* header = &sampleHeaderArray[i];
				pitchSampleHeaderArray.push_back(*header);
				
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
						pitchSampleHeaderArray.push_back(*header);
					}

					if (error)
						break;
				}

				SoundFontData::PitchData* pitchData = this->ConstructPitchData(pitchSampleHeaderArray, smplChunk, sm24Chunk, error);
				if (!pitchData)
					break;

				soundFontData->pitchDataArray->push_back(pitchData);
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

bool SoundFontFormat::ReadCrazyData(ChunkParser& parser, char prefix, Error& error)
{
	if (prefix != 'i' && prefix != 'p')
	{
		error.Add("Prefix must be 'i' or 'p'.");
		return false;
	}

	const char* headerChunkName = (prefix == 'i') ? "inst" : "phdr";
	uint32_t headerSize = (prefix == 'i') ? sizeof(SF_Instrument) : sizeof(SF_Preset);

	const ChunkParser::Chunk* headerChunk = parser.FindChunk(headerChunkName, false);
	if (!headerChunk)
	{
		error.Add(FormatString("No \"%s\" chunk found.", headerChunkName));
		return false;
	}

	if (headerChunk->GetBufferSize() % headerSize != 0)
	{
		error.Add(FormatString("The \"%s\" chunk is not a multiple of the intrument structure size.", headerChunkName));
		return false;
	}

	uint32_t numHeaders = headerChunk->GetBufferSize() / headerSize;

	const char* bagChunkName = (prefix == 'i') ? "ibag" : "pbag";

	const ChunkParser::Chunk* ibagChunk = parser.FindChunk(bagChunkName, false);
	if (!ibagChunk)
	{
		error.Add(FormatString("No \"%s\" chunk found.", bagChunkName));
		return false;
	}

	if (ibagChunk->GetBufferSize() % sizeof(SF_Bag) != 0)
	{
		error.Add(FormatString("The \"%s\" chunk is not a multiple of the bag structure size.", bagChunkName));
		return false;
	}

	auto bagArray = (const SF_Bag*)ibagChunk->GetBuffer();
	uint32_t numBags = ibagChunk->GetBufferSize() / sizeof(SF_Bag);

	const char* modChunkName = (prefix == 'i') ? "imod" : "pmod";

	const ChunkParser::Chunk* modChunk = parser.FindChunk(modChunkName, false);
	if (!modChunk)
	{
		error.Add(FormatString("No \"%s\" chunk found.", modChunkName));
		return false;
	}

	if (modChunk->GetBufferSize() % sizeof(SF_Modulator) != 0)
	{
		error.Add(FormatString("The \"%s\" chunk is not a multiple in size of the modulator structure.", modChunkName));
		return false;
	}

	auto modulatorArray = (const SF_Modulator*)modChunk->GetBuffer();
	uint32_t numModulators = modChunk->GetBufferSize() / sizeof(SF_Modulator);

	const char* genChunkName = (prefix == 'i') ? "igen" : "pgen";

	const ChunkParser::Chunk* igenChunk = parser.FindChunk(genChunkName, false);
	if (!igenChunk)
	{
		error.Add(FormatString("No \"%s\" chunk found.", genChunkName));
		return false;
	}

	if (igenChunk->GetBufferSize() % sizeof(SF_Generator) != 0)
	{
		error.Add(FormatString("The \"%s\" chunk is not a multiple in size of the generator structure.", genChunkName));
		return false;
	}

	auto generatorArray = (const SF_Generator*)igenChunk->GetBuffer();
	uint32_t numGenerators = igenChunk->GetBufferSize() / sizeof(SF_Generator);

	for (uint32_t i = 0; i < numHeaders; i++)
	{
		uint32_t numZones = 0;
		uint32_t baseBagIndex = 0;

		if (prefix == 'i')
		{
			auto instrumentArray = (const SF_Instrument*)headerChunk->GetBuffer();
			const SF_Instrument& instrument = instrumentArray[i];
			if (::strcmp((const char*)instrument.name, "EOI") == 0)
				break;

			numZones = instrumentArray[i + 1].bagIndex - instrument.bagIndex;
			baseBagIndex = instrument.bagIndex;
		}
		else if (prefix == 'p')
		{
			auto presetArray = (const SF_Preset*)headerChunk->GetBuffer();
			const SF_Preset& preset = presetArray[i];
			if (::strcmp((const char*)preset.name, "EOP") == 0)
				break;

			numZones = presetArray[i + 1].bagIndex - preset.bagIndex;
			baseBagIndex = preset.bagIndex;
		}

		for (uint32_t j = 0; j < numZones; j++)
		{
			const SF_Bag& bag = bagArray[baseBagIndex + j];

			if (bag.generatorIndex >= numGenerators)
			{
				error.Add("Generator index out of range.");
				break;
			}

			const SF_Generator& generator = generatorArray[bag.generatorIndex];
			switch (generator.op)
			{
			case ADL_GENERATOR_OP_KEY_RANGE:
				printf("Key range: %d, %d\n", generator.range.min, generator.range.max);
				break;
			case ADL_GENERATOR_OP_VEL_RANGE:
				printf("Vel range: %d, %d\n", generator.range.min, generator.range.max);
				break;
			case ADL_GENERATOR_OP_SAMPLE_ID:
				printf("SampleID = %d\n", generator.amount);
				break;
			}
		}

		if (error)
			return false;
	}

	if (error)
		return false;

	return true;
}

SoundFontData::PitchData* SoundFontFormat::ConstructPitchData(const std::vector<SF_SampleHeader>& pitchSampleHeaderArray, const ChunkParser::Chunk* smplChunk, const ChunkParser::Chunk* sm24Chunk, Error& error)
{
	if (pitchSampleHeaderArray.size() == 0)
		return nullptr;

	// Sanity check the headers against the sample chunk.
	for (const SF_SampleHeader& header : pitchSampleHeaderArray)
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

	const uint16_t* sampleBuffer16 = (const uint16_t*)smplChunk->GetBuffer();
	uint64_t sampleBuffer16Size = smplChunk->GetBufferSize() / sizeof(uint16_t);

	const uint8_t* sampleBuffer8 = sm24Chunk ? (const uint8_t*)sm24Chunk->GetBuffer() : nullptr;
	uint64_t sampleBuffer8Size = sm24Chunk ? (sm24Chunk->GetBufferSize() / sizeof(uint8_t)) : 0;

	if (sampleBuffer8 && sampleBuffer8Size != sampleBuffer16Size)
	{
		error.Add("The supplementary buffer is no the same size as the main buffer.");
		return nullptr;
	}

	auto pitchData = new SoundFontData::PitchData();

	// I'm not even going to check here that all the headers specify the same pitch.
	// It seems as though this is always set to 60 no matter what.
	pitchData->SetMIDIPitch(pitchSampleHeaderArray[0].originalPitch);

	for (const SF_SampleHeader& header : pitchSampleHeaderArray)
	{
		auto loopedAudioData = new SoundFontData::LoopedAudioData();
		pitchData->loopedAudioDataArray->push_back(loopedAudioData);

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
		delete pitchData;
		pitchData = nullptr;
	}

	return pitchData;
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