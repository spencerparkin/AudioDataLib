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
	if (4 != inputStream.ReadBytesFromStream(riffStr, 4))
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

	int fileSize = 0;
	if (4 != inputStream.ReadBytesFromStream((char*)&fileSize, 4))
	{
		error = "Failed to read file size.";
		return false;
	}

	char waveStr[5];
	if (4 != inputStream.ReadBytesFromStream(waveStr, 4))
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

	bool chunkProcessed = true;
	while (chunkProcessed)
	{
		if (!this->ProcessChunk(inputStream, audioData, chunkProcessed, error))
		{
			delete audioData;
			audioData = nullptr;
			return false;
		}
	}

	return true;
}

bool WaveFormat::ProcessChunk(ByteStream& inputStream, AudioData* audioData, bool& chunkProcessed, std::string& error)
{
	chunkProcessed = true;
	
	char chunkIdStr[5];
	int numBytesRead = inputStream.ReadBytesFromStream(chunkIdStr, 4);
	if (numBytesRead == 0)
	{
		chunkProcessed = false;
		return true;
	}

	if (4 != numBytesRead)
	{
		error = "Failed to read chunk ID string.";
		return false;
	}

	chunkIdStr[4] = '\0';

	int chunkDataSize = 0;
	if (4 != inputStream.ReadBytesFromStream((char*)&chunkDataSize, 4))
	{
		error = "Failed to read chunk data size.";
		return false;
	}

	if (0 == strcmp(chunkIdStr, "fmt "))
	{
		short type = 0;
		if (2 != inputStream.ReadBytesFromStream((char*)&type, 2))
		{
			error = "Failed to read format type.";
			return false;
		}

		short numChannels = 0;
		if (2 != inputStream.ReadBytesFromStream((char*)&numChannels, 2))
		{
			error = "Failed to read number of channels.";
			return false;
		}

		int sampleRateBitsPerSecond = 0;
		if (4 != inputStream.ReadBytesFromStream((char*)&sampleRateBitsPerSecond, 4))
		{
			error = "Failed to read sample rate.";
			return false;
		}

		int redundantData = 0;
		if (4 != inputStream.ReadBytesFromStream((char*)&redundantData, 4))
		{
			error = "Failed to read redundant data.";
			return false;
		}

		short moreRedundantData = 0;
		if (2 != inputStream.ReadBytesFromStream((char*)&moreRedundantData, 2))
		{
			error = "Failed to read more redundant data.";
			return false;
		}

		short bitsPerSample = 0;
		if (2 != inputStream.ReadBytesFromStream((char*)&bitsPerSample, 2))
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
			char byte = 0;
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
		char* waveDataBuffer = new char[chunkDataSize];
		int numBytesRead = inputStream.ReadBytesFromStream(waveDataBuffer, chunkDataSize);
		if (chunkDataSize != numBytesRead)
		{
			error = "Failed to read wave data buffer from data chunk.";
			delete[] waveDataBuffer;
			return false;
		}

		ByteStream* audioStream = audioData->GetAudioStream();
		int numBytesWritten = audioStream->WriteBytesToStream(waveDataBuffer, chunkDataSize);
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
			char byte = 0;
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