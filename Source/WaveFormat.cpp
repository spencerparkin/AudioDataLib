#include "WaveFormat.h"
#include "ByteStream.h"
#include "AudioData.h"

using namespace AudioDataLib;

WaveFormat::WaveFormat()
{
}

/*virtual*/ WaveFormat::~WaveFormat()
{
}

/*virtual*/ bool WaveFormat::ReadFromStream(ByteStream& inputStream, AudioData*& audioData, std::string& error)
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

bool WaveFormat::ProcessChunk(ByteStream& inputStream, AudioData* audioData, std::string& error)
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

		if (type != 1)
		{
			error = "Non-PCM data not yet supported.";
			return false;
		}

		uint16_t numChannels = 0;
		if (2 != inputStream.ReadBytesFromStream((uint8_t*)&numChannels, 2))
		{
			error = "Failed to read number of channels.";
			return false;
		}

		uint32_t sampleRateBitsPerSecond = 0;
		if (4 != inputStream.ReadBytesFromStream((uint8_t*)&sampleRateBitsPerSecond, 4))
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
		format.sampleRateBitsPerSecond = sampleRateBitsPerSecond;
		format.bitsPerSample = bitsPerSample;

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
		uint8_t* waveDataBuffer = new uint8_t[chunkDataSize];
		uint64_t numBytesRead = inputStream.ReadBytesFromStream(waveDataBuffer, chunkDataSize);
		if (chunkDataSize != numBytesRead)
		{
			error = "Failed to read wave data buffer from data chunk.";
			delete[] waveDataBuffer;
			return false;
		}

		ByteStream* audioStream = audioData->GetAudioStream();
		uint64_t numBytesWritten = audioStream->WriteBytesToStream(waveDataBuffer, chunkDataSize);
		if (chunkDataSize != numBytesWritten)
		{
			error = "Failed to write wave data buffer to audio stream.";
			delete[] waveDataBuffer;
			return false;
		}

		delete[] waveDataBuffer;
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

/*virtual*/ bool WaveFormat::WriteToStream(ByteStream& outputStream, AudioData* audioData, std::string& error)
{
	return false;
}