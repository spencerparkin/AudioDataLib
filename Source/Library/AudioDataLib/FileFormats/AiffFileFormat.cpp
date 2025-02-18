#include "AudioDataLib/FileFormats/AiffFileFormat.h"
#include "AudioDataLib/FileDatas/WaveTableData.h"
#include "AudioDataLib/Codecs/Codec.h"
#include "AudioDataLib/Codecs/ALawCodec.h"
#include "AudioDataLib/Codecs/uLawCodec.h"
#include "AudioDataLib/Error.h"

using namespace AudioDataLib;

//------------------------------- AiffFileFormat -------------------------------

AiffFileFormat::AiffFileFormat()
{
}

/*virtual*/ AiffFileFormat::~AiffFileFormat()
{
}

/*virtual*/ bool AiffFileFormat::ReadFromStream(ByteStream& inputStream, FileData*& fileData, Error& error)
{
	// https://paulbourke.net/dataformats/audio/

	AiffChunkParser parser;

	// Some file formats have a special magic value in a header that can be used to
	// test whether byte-swapping is needed or not, but for now, assume it is.
	parser.byteSwapper.swapsNeeded = true;

	if (!parser.ParseStream(inputStream, error))
		return false;

	const ChunkParser::Chunk* formChunk = parser.FindChunk("FORM", "", false);
	if (!formChunk)
	{
		error.Add("Did not find the form chunk.");
		return false;
	}

	const ChunkParser::Chunk* commonChunk = parser.FindChunk("COMM", "", false);
	if (!commonChunk)
	{
		error.Add("Did not find a common chunk.");
		return false;
	}

	ReadOnlyBufferStream commonStream(commonChunk->GetBuffer(), commonChunk->GetBufferSize());
	
	int16_t numChannels = 0;
	if (sizeof(numChannels) != commonStream.ReadBytesFromStream((uint8_t*)&numChannels, sizeof(numChannels)))
	{
		error.Add("Failed to read number of channels.");
		return false;
	}

	numChannels = parser.byteSwapper.Resolve(numChannels);

	uint32_t numFrames = 0;
	if (sizeof(numFrames) != commonStream.ReadBytesFromStream((uint8_t*)&numFrames, sizeof(numFrames)))
	{
		error.Add("Failed to read number of frames.");
		return false;
	}

	numFrames = parser.byteSwapper.Resolve(numFrames);

	int16_t sampleSizeBits = 0;
	if (sizeof(sampleSizeBits) != commonStream.ReadBytesFromStream((uint8_t*)&sampleSizeBits, sizeof(sampleSizeBits)))
	{
		error.Add("Failed to read sample size.");
		return false;
	}

	sampleSizeBits = parser.byteSwapper.Resolve(sampleSizeBits);

	uint16_t sampleSizeBytes = sampleSizeBits / 8;

	uint8_t sampleRateBuffer[10];
	if (sizeof(sampleRateBuffer) != commonStream.ReadBytesFromStream((uint8_t*)sampleRateBuffer, sizeof(sampleRateBuffer)))
	{
		error.Add("Failed to read sample rate buffer.");
		return false;
	}

	uint8_t sign = sampleRateBuffer[0] & 0x80;
	if (sign != 0)
	{
		error.Add("Doesn't make sense for sample rate to be negative.");
		return false;
	}

	int32_t exponent = int32_t((uint16_t(sampleRateBuffer[0] & 0x7F) << 8) | uint16_t(sampleRateBuffer[1]));
	int32_t shift = exponent - 16383;
	if (shift < 7)
	{
		error.Add("Was given a sample rate with a non-zero fractional part.  Expected a whole-number for the sample rate.");
		return false;
	}

	uint64_t sampleRate =
		(uint64_t(sampleRateBuffer[2]) << 40) |
		(uint64_t(sampleRateBuffer[3]) << 32) |
		(uint64_t(sampleRateBuffer[4]) << 24) |
		(uint64_t(sampleRateBuffer[5]) << 16) |
		(uint64_t(sampleRateBuffer[6]) << 8) |
		(uint64_t(sampleRateBuffer[7]) << 0);

	sampleRate >>= 47 - shift;

	char compressionType[5] = "";
	std::string compressionTypeName;

	if (formChunk->GetFormType() == "AIFC")
	{
		if (4 != commonStream.ReadBytesFromStream((uint8_t*)&compressionType, 4))
		{
			error.Add("Failed to read compression type ID from common chunk.");
			return false;
		}

		compressionType[4] = '\0';

		uint8_t compressionTypeStringLength = 0;
		if (1 != commonStream.ReadBytesFromStream(&compressionTypeStringLength, 1))
		{
			error.Add("Failed to read compression type string length from common chunk.");
			return false;
		}

		for (uint8_t i = 0; i < compressionTypeStringLength; i++)
		{
			char ch = 0;
			if (1 != commonStream.ReadBytesFromStream((uint8_t*)&ch, 1))
			{
				error.Add(FormatString("Failed to read compression type string character %d of %d from common chunk.", i, compressionTypeStringLength));
				return false;
			}

			compressionTypeName += ch;
		}
	}

	std::shared_ptr<Codec> codec;

	if (0 == ::strlen(compressionType) || 0 == ::strcmp(compressionType, "NONE"))
		codec.reset(new ByteSwappedAudioCodec(&parser.byteSwapper));
	else if (0 == ::strcmp(compressionType, "ulaw"))
		codec.reset(new uLawCodec());
	else if (0 == ::strcmp(compressionType, "alaw"))
		codec.reset(new ALawCodec());

	if (!codec.get())
	{
		error.Add(FormatString("An audio codec could not be determined for this file.  The comperssion type name is \"%s\".", compressionTypeName.c_str()));
		return false;
	}

	const ChunkParser::Chunk* soundChunk = parser.FindChunk("SSND", "", false);
	if (!soundChunk)
	{
		error.Add("No sound chunk found.");
		return false;
	}

	ReadOnlyBufferStream soundStream(soundChunk->GetBuffer(), soundChunk->GetBufferSize());

	uint32_t offset = 0;
	if (sizeof(offset) != soundStream.ReadBytesFromStream((uint8_t*)&offset, sizeof(offset)))
	{
		error.Add("Failed to read sound buffer offset.");
		return false;
	}

	uint32_t blockSize = 0;
	if (sizeof(blockSize) != soundStream.ReadBytesFromStream((uint8_t*)&blockSize, sizeof(blockSize)))
	{
		error.Add("Failed to read block size from sound chunk.");
		return false;
	}

	while (offset-- > 0)
	{
		uint8_t padByte = 0;
		if (1 != soundStream.ReadBytesFromStream(&padByte, 1))
		{
			error.Add("Failed to read pad-byte.");
			return false;
		}
	}

	AudioData* audioData = nullptr;

	const ChunkParser::Chunk* instrumentChunk = parser.FindChunk("INST", "", false);
	if (!instrumentChunk)
		audioData = new AudioData();
	else
	{
		WaveTableData::AudioSampleData* audioSampleData = new WaveTableData::AudioSampleData();
		
		// TODO: Fill-out looping information here.

		audioData = audioSampleData;
	}

	AudioData::Format format;
	format.bitsPerSample = sampleSizeBits;
	format.sampleType = AudioData::Format::SampleType::SIGNED_INTEGER;
	format.numChannels = numChannels;
	format.framesPerSecond = sampleRate;
	audioData->SetFormat(format);

	if(!codec->Decode(soundStream, *audioData, error))
	{
		delete audioData;
		return false;
	}

	fileData = audioData;
	return true;
}

/*virtual*/ bool AiffFileFormat::WriteToStream(ByteStream& outputStream, const FileData* fileData, Error& error)
{
	error.Add("Not yet implemented.");
	return false;
}

//------------------------------- AiffFileFormat::AiffChunkParser -------------------------------

AiffFileFormat::AiffChunkParser::AiffChunkParser()
{
}

/*virtual*/ AiffFileFormat::AiffChunkParser::~AiffChunkParser()
{
}

/*virtual*/ bool AiffFileFormat::AiffChunkParser::ParseChunkData(ReadOnlyBufferStream& inputStream, Chunk* chunk, Error& error)
{
	if (chunk->GetName() == "FORM")
	{
		char formType[5];
		if (4 != inputStream.ReadBytesFromStream((uint8_t*)formType, 4))
		{
			error.Add("Could not read form type of FORM chunk.");
			return false;
		}

		formType[4] = '\0';
		if (0 != strcmp(formType, "AIFF") && 0 != strcmp(formType, "AIFC"))
		{
			error.Add("File does not appears to be an AIFF file.");
			return false;
		}

		chunk->SetFormType(formType);

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