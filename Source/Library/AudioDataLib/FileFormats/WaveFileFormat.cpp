#include "AudioDataLib/FileFormats/WaveFileFormat.h"
#include "AudioDataLib/ByteStream.h"
#include "AudioDataLib/FileDatas/AudioData.h"
#include "AudioDataLib/Error.h"

using namespace AudioDataLib;

WaveFileFormat::WaveFileFormat()
{
}

/*virtual*/ WaveFileFormat::~WaveFileFormat()
{
}

/*virtual*/ bool WaveFileFormat::ReadFromStream(ByteStream& inputStream, FileData*& fileData, Error& error)
{
	WaveChunkParser parser;
	if (!parser.ParseStream(inputStream, error))
		return false;

	auto audioData = new AudioData();
	if (!this->LoadWaveData(audioData, parser.GetRootChunk(), error))
	{
		delete audioData;
		return false;
	}
	
	fileData = audioData;
	return true;
}

/*static*/ bool WaveFileFormat::LoadWaveData(AudioData* audioData, const ChunkParser::Chunk* waveChunk, Error& error)
{
	const WaveChunkParser::Chunk* fmtChunk = waveChunk->FindChunk("fmt ", "", true);
	if (!fmtChunk)
	{
		error.Add("Failed to find format chunk.");
		return false;
	}

	const WaveChunkParser::Chunk* dataChunk = waveChunk->FindChunk("data", "", true);
	if (!dataChunk)
	{
		error.Add("Failed to find data chunk.");
		return false;
	}

	ReadOnlyBufferStream formatStream(fmtChunk->GetBuffer(), fmtChunk->GetBufferSize());

	uint16_t type = 0;
	if (2 != formatStream.ReadBytesFromStream((uint8_t*)&type, 2))
	{
		error.Add("Failed to read format type.");
		return false;
	}

	uint16_t numChannels = 0;
	if (2 != formatStream.ReadBytesFromStream((uint8_t*)&numChannels, 2))
	{
		error.Add("Failed to read number of channels.");
		return false;
	}

	uint32_t sampleRateSamplesPerSecondPerChannel = 0;
	if (4 != formatStream.ReadBytesFromStream((uint8_t*)&sampleRateSamplesPerSecondPerChannel, 4))
	{
		error.Add("Failed to read sample rate.");
		return false;
	}

	uint32_t redundantData = 0;
	if (4 != formatStream.ReadBytesFromStream((uint8_t*)&redundantData, 4))
	{
		error.Add("Failed to read redundant data.");
		return false;
	}

	uint16_t moreRedundantData = 0;
	if (2 != formatStream.ReadBytesFromStream((uint8_t*)&moreRedundantData, 2))
	{
		error.Add("Failed to read more redundant data.");
		return false;
	}

	uint16_t bitsPerSample = 0;
	if (2 != formatStream.ReadBytesFromStream((uint8_t*)&bitsPerSample, 2))
	{
		error.Add("Failed to read bits per sample.");
		return false;
	}

	AudioData::Format format;
	format.numChannels = numChannels;
	format.framesPerSecond = sampleRateSamplesPerSecondPerChannel;
	format.bitsPerSample = bitsPerSample;

	switch (type)
	{
		case SampleFormat::PCM:
		{
			// Apparently, with WAV, 8-bit is always unsigned, and anything higher is signed.
			// WAV does not have a way to indicate the signed-ness of the data, which is dumb.
			if (bitsPerSample == 8)
				format.sampleType = AudioData::Format::UNSIGNED_INTEGER;
			else
				format.sampleType = AudioData::Format::SIGNED_INTEGER;

			break;
		}
		case SampleFormat::IEEE_FLOAT:
		{
			format.sampleType = AudioData::Format::FLOAT;

			if (format.bitsPerSample != 32 && format.bitsPerSample != 64)
			{
				error.Add("Don't yet know how to support floating-point samples if they're not 32-bit or 64-bit.");
				return false;
			}

			break;
		}
		case SampleFormat::EXTENSIBLE:
		{
			uint16_t extensionSize = 0;
			if (2 != formatStream.ReadBytesFromStream((uint8_t*)&extensionSize, 2))
			{
				error.Add("Failed to read format extension size.");
				return false;
			}

			if (extensionSize != 22)
			{
				error.Add("Expected extension size to be 22 bytes.");
				return false;
			}

			uint16_t validBitsPerSample = 0;
			if (2 != formatStream.ReadBytesFromStream((uint8_t*)&validBitsPerSample, 2))
			{
				error.Add("Failed to read valid bits per sample.");
				return false;
			}

			if (validBitsPerSample != bitsPerSample)
			{
				error.Add("This software does not yet handle the case when the extension's bits-per-sample mismatches the regular bits-per-sample.");
				return false;
			}

			uint32_t channelMask = 0;		// TODO: Use this at some point.
			if (4 != formatStream.ReadBytesFromStream((uint8_t*)&channelMask, 4))
			{
				error.Add("Failed to read channel mask.");
				return false;
			}

			uint16_t subFormat = 0;
			if (2 != formatStream.ReadBytesFromStream((uint8_t*)&subFormat, 2))
			{
				error.Add("Failed to read sub-format.");
				return false;
			}

			if (subFormat != SampleFormat::PCM)
			{
				error.Add(FormatString("Can't yet handle sub-formats of type %d.", subFormat));
				return false;
			}

			if (bitsPerSample == 8)
				format.sampleType = AudioData::Format::UNSIGNED_INTEGER;
			else
				format.sampleType = AudioData::Format::SIGNED_INTEGER;

			break;
		}
		default:
		{
			error.Add(FormatString("Format type %d not supported.", type));
			return false;
		}
	}

	audioData->SetFormat(format);
	audioData->SetAudioBufferSize(dataChunk->GetBufferSize());
	::memcpy(audioData->GetAudioBuffer(), dataChunk->GetBuffer(), dataChunk->GetBufferSize());
	return true;
}

/*virtual*/ bool WaveFileFormat::WriteToStream(ByteStream& outputStream, const FileData* fileData, Error& error)
{
	const AudioData* audioData = dynamic_cast<const AudioData*>(fileData);
	if (!audioData)
	{
		error.Add("Can't make a WAV file with something other than AudioData.");
		return false;
	}

	if (4 != outputStream.WriteBytesToStream((const uint8_t*)"RIFF", 4))
	{
		error.Add("Could not write RIFF.");
		return false;
	}

	uint32_t riffChunkSize = (uint32_t)audioData->GetAudioBufferSize() + 36;
	if (4 != outputStream.WriteBytesToStream((const uint8_t*)&riffChunkSize, 4))
	{
		error.Add("Could not write RIFF chunk size.");
		return false;
	}

	if (4 != outputStream.WriteBytesToStream((const uint8_t*)"WAVE", 4))
	{
		error.Add("Could not write WAVE.");
		return false;
	}

	if (4 != outputStream.WriteBytesToStream((const uint8_t*)"fmt ", 4))
	{
		error.Add("Could not write \"fmt \" chunk header ID.");
		return false;
	}

	uint32_t fmtChunkSize = 16;
	if (4 != outputStream.WriteBytesToStream((const uint8_t*)&fmtChunkSize, 4))
	{
		error.Add("Could not write format chunk size.");
		return false;
	}

	uint16_t type = 0;
	switch (audioData->GetFormat().sampleType)
	{
		case AudioData::Format::SIGNED_INTEGER:
		{
			type = SampleFormat::PCM;
			break;
		}
		case AudioData::Format::FLOAT:
		{
			type = SampleFormat::IEEE_FLOAT;
			break;
		}
		default:
		{
			error.Add("Could not write sample format type.");
			break;
		}
	}

	if (2 != outputStream.WriteBytesToStream((const uint8_t*)&type, 2))
	{
		error.Add("Could not write PCM type.");
		return false;
	}

	uint16_t numChannels = uint16_t(audioData->GetFormat().numChannels);
	if (2 != outputStream.WriteBytesToStream((const uint8_t*)&numChannels, 2))
	{
		error.Add("Could not write number of channels.");
		return false;
	}

	uint32_t sampleRate = uint32_t(audioData->GetFormat().framesPerSecond);
	if (4 != outputStream.WriteBytesToStream((const uint8_t*)&sampleRate, 4))
	{
		error.Add("Could not write sample rate.");
		return false;
	}

	uint32_t bytesPerSecond = uint32_t(audioData->GetFormat().BytesPerSecond());
	if (4 != outputStream.WriteBytesToStream((const uint8_t*)&bytesPerSecond, 4))
	{
		error.Add("Could not write bytes per second.");
		return false;
	}

	// Samples need to be aligned on an address divisible by this value?
	uint16_t blockAlign = audioData->GetFormat().bitsPerSample * audioData->GetFormat().numChannels;
	if (2 != outputStream.WriteBytesToStream((const uint8_t*)&blockAlign, 2))
	{
		error.Add("Could not write block align.");
		return false;
	}

	uint16_t bitsPerSample = audioData->GetFormat().bitsPerSample;
	if (2 != outputStream.WriteBytesToStream((const uint8_t*)&bitsPerSample, 2))
	{
		error.Add("Could not write bits per sample.");
		return false;
	}

	if (4 != outputStream.WriteBytesToStream((const uint8_t*)"data", 4))
	{
		error.Add("Could not write \"data\" chunk header ID.");
		return false;
	}

	uint32_t audioBufferSize = (uint32_t)audioData->GetAudioBufferSize();
	if (4 != outputStream.WriteBytesToStream((const uint8_t*)&audioBufferSize, 4))
	{
		error.Add("Could not write audio buffer size.");
		return false;
	}

	uint64_t numBytesWritten = outputStream.WriteBytesToStream(audioData->GetAudioBuffer(), audioBufferSize);
	if (numBytesWritten != audioData->GetAudioBufferSize())
	{
		error.Add("Could not write audio buffer.");
		return false;
	}

	return true;
}

//------------------------------- WaveFileFormat::WaveChunkParser -------------------------------

WaveFileFormat::WaveChunkParser::WaveChunkParser()
{
}

/*virtual*/ WaveFileFormat::WaveChunkParser::~WaveChunkParser()
{
}

/*virtual*/ bool WaveFileFormat::WaveChunkParser::ParseChunkData(ReadOnlyBufferStream& inputStream, Chunk* chunk, Error& error)
{
	if (chunk->GetName() == "RIFF")
	{
		char formType[5];
		if (4 != inputStream.ReadBytesFromStream((uint8_t*)formType, 4))
		{
			error.Add("Could not read form type of RIFF.");
			return false;
		}

		formType[4] = '\0';
		if (0 != strcmp(formType, "WAVE"))
		{
			error.Add("RIFF file does not appears to be a WAVE file.");
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