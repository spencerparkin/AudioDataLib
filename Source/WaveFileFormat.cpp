#include "WaveFileFormat.h"
#include "ByteStream.h"
#include "AudioData.h"

using namespace AudioDataLib;

WaveFileFormat::WaveFileFormat()
{
}

/*virtual*/ WaveFileFormat::~WaveFileFormat()
{
}

/*virtual*/ bool WaveFileFormat::ReadFromStream(ByteStream& inputStream, AudioData*& audioData, std::string& error)
{
	char riffStr[5];
	if (4 != inputStream.ReadBytesFromStream((uint8_t*)riffStr, 4))
	{
		error = "Failed to read RIFF.";
		return false;
	}

	riffStr[4] = '\0';
	if (0 != strcmp(riffStr, "RIFF"))
	{
		error = "RIFF not found as initial 4 bytes of file.";
		return false;
	}

	uint32_t fileSize = 0;
	if (4 != inputStream.ReadBytesFromStream((uint8_t*)&fileSize, 4))
	{
		error = "Failed to read file size.";
		return false;
	}

	char waveStr[5];
	if (4 != inputStream.ReadBytesFromStream((uint8_t*)waveStr, 4))
	{
		error = "Failed to read WAVE.";
		return false;
	}

	waveStr[4] = '\0';
	if (0 != strcmp(waveStr, "WAVE"))
	{
		error = "WAVE string not found in file.";
		return false;
	}

	audioData = new AudioData();

	while (inputStream.CanRead())
	{
		if (!this->ProcessChunk(inputStream, audioData, error))
		{
			delete audioData;
			audioData = nullptr;
			return false;
		}
	}

	return true;
}

bool WaveFileFormat::ProcessChunk(ByteStream& inputStream, AudioData* audioData, std::string& error)
{
	char chunkIdStr[5];
	if (4 != inputStream.ReadBytesFromStream((uint8_t*)chunkIdStr, 4))
	{
		error = "Failed to read chunk ID string.";
		return false;
	}

	chunkIdStr[4] = '\0';

	uint32_t chunkDataSize = 0;
	if (4 != inputStream.ReadBytesFromStream((uint8_t*)&chunkDataSize, 4))
	{
		error = "Failed to read chunk data size.";
		return false;
	}

	if (0 == strcmp(chunkIdStr, "fmt "))
	{
		uint16_t type = 0;
		if (2 != inputStream.ReadBytesFromStream((uint8_t*)&type, 2))
		{
			error = "Failed to read format type.";
			return false;
		}

		uint16_t numChannels = 0;
		if (2 != inputStream.ReadBytesFromStream((uint8_t*)&numChannels, 2))
		{
			error = "Failed to read number of channels.";
			return false;
		}

		uint32_t sampleRateSamplesPerSecondPerChannel = 0;
		if (4 != inputStream.ReadBytesFromStream((uint8_t*)&sampleRateSamplesPerSecondPerChannel, 4))
		{
			error = "Failed to read sample rate.";
			return false;
		}

		uint32_t redundantData = 0;
		if (4 != inputStream.ReadBytesFromStream((uint8_t*)&redundantData, 4))
		{
			error = "Failed to read redundant data.";
			return false;
		}

		uint16_t moreRedundantData = 0;
		if (2 != inputStream.ReadBytesFromStream((uint8_t*)&moreRedundantData, 2))
		{
			error = "Failed to read more redundant data.";
			return false;
		}

		uint16_t bitsPerSample = 0;
		if (2 != inputStream.ReadBytesFromStream((uint8_t*)&bitsPerSample, 2))
		{
			error = "Failed to read bits per sample.";
			return false;
		}

		AudioData::Format& format = audioData->GetFormat();
		format.numChannels = numChannels;
		format.framesPerSecond = sampleRateSamplesPerSecondPerChannel;
		format.bitsPerSample = bitsPerSample;

		switch (type)
		{
			case SampleFormat::PCM:
			{
				format.sampleType = AudioData::Format::SIGNED_INTEGER;
				break;
			}
			case SampleFormat::IEEE_FLOAT:
			{
				format.sampleType = AudioData::Format::FLOAT;

				if (format.bitsPerSample != 32)
				{
					error = "Don't yet know how to support floating-point samples if they're not 32-bit.";
					return false;
				}

				break;
			}
			default:
			{
				char errorBuf[128];
				sprintf(errorBuf, "Format type %d not supported.", type);
				error = errorBuf;
				return false;
			}
		}

		chunkDataSize -= 16;
		while (chunkDataSize > 0)
		{
			uint8_t byte = 0;
			if (1 != inputStream.ReadBytesFromStream(&byte, 1))
			{
				error = "Failed to read pad bytes from format chunk.";
				return false;
			}

			chunkDataSize--;
		}
	}
	else if (0 == strcmp(chunkIdStr, "data"))
	{
		audioData->SetAudioBufferSize(chunkDataSize);
		uint64_t numBytesRead = inputStream.ReadBytesFromStream(audioData->GetAudioBuffer(), chunkDataSize);
		if (chunkDataSize != numBytesRead)
		{
			error = "Failed to read wave data buffer from data chunk.";
			return false;
		}
	}
	else
	{
		// We don't recognize the chunk.  Just try to skip it.
		while (chunkDataSize > 0)
		{
			uint8_t byte = 0;
			if (1 != inputStream.ReadBytesFromStream(&byte, 1))
			{
				error = "Failed to read byte from stream in unknown chunk.";
				return false;
			}

			chunkDataSize--;
		}
	}

	return true;
}

/*virtual*/ bool WaveFileFormat::WriteToStream(ByteStream& outputStream, AudioData* audioData, std::string& error)
{
	if (4 != outputStream.WriteBytesToStream((const uint8_t*)"RIFF", 4))
	{
		error = "Could not write RIFF.";
		return false;
	}

	uint32_t riffChunkSize = (uint32_t)audioData->GetAudioBufferSize() + 34;
	if (4 != outputStream.WriteBytesToStream((const uint8_t*)&riffChunkSize, 4))
	{
		error = "Could not write RIFF chunk size.";
		return false;
	}

	if (4 != outputStream.WriteBytesToStream((const uint8_t*)"WAVE", 4))
	{
		error = "Could not write WAVE.";
		return false;
	}

	if (4 != outputStream.WriteBytesToStream((const uint8_t*)"fmt ", 4))
	{
		error = "Could not write \"fmt \" chunk header ID.";
		return false;
	}

	uint32_t fmtChunkSize = 16;
	if (4 != outputStream.WriteBytesToStream((const uint8_t*)&fmtChunkSize, 4))
	{
		error = "Could not write format chunk size.";
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
			error = "Could not write sample format type.";
			break;
		}
	}

	if (2 != outputStream.WriteBytesToStream((const uint8_t*)&type, 2))
	{
		error = "Could not write PCM type.";
		return false;
	}

	uint16_t numChannels = uint16_t(audioData->GetFormat().numChannels);
	if (2 != outputStream.WriteBytesToStream((const uint8_t*)&numChannels, 2))
	{
		error = "Could not write number of channels.";
		return false;
	}

	uint32_t sampleRate = uint32_t(audioData->GetFormat().framesPerSecond);
	if (4 != outputStream.WriteBytesToStream((const uint8_t*)&sampleRate, 4))
	{
		error = "Could not write sample rate.";
		return false;
	}

	uint32_t bytesPerSecond = uint32_t(audioData->GetFormat().BytesPerSecond());
	if (4 != outputStream.WriteBytesToStream((const uint8_t*)&bytesPerSecond, 4))
	{
		error = "Could not write bytes per second.";
		return false;
	}

	// Samples need to be aligned on an address divisible by this value?
	uint16_t blockAlign = audioData->GetFormat().bitsPerSample * audioData->GetFormat().numChannels;
	if (2 != outputStream.WriteBytesToStream((const uint8_t*)&blockAlign, 2))
	{
		error = "Could not write block align.";
		return false;
	}

	uint16_t bitsPerSample = audioData->GetFormat().bitsPerSample;
	if (2 != outputStream.WriteBytesToStream((const uint8_t*)&bitsPerSample, 2))
	{
		error = "Could not write bits per sample.";
		return false;
	}

	if (4 != outputStream.WriteBytesToStream((const uint8_t*)"data", 4))
	{
		error = "Could not write \"data\" chunk header ID.";
		return false;
	}

	uint32_t audioBufferSize = (uint32_t)audioData->GetAudioBufferSize();
	if (4 != outputStream.WriteBytesToStream((const uint8_t*)&audioBufferSize, 4))
	{
		error = "Could not write audio buffer size.";
		return false;
	}

	uint64_t numBytesWritten = outputStream.WriteBytesToStream(audioData->GetAudioBuffer(), audioBufferSize);
	if (numBytesWritten != audioData->GetAudioBufferSize())
	{
		error = "Could not write audio buffer.";
		return false;
	}

	return true;
}