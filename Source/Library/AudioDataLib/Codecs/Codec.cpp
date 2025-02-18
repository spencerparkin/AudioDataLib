#include "AudioDataLib/Codecs/Codec.h"
#include "AudioDataLib/ByteStream.h"
#include "AudioDataLib/FileDAtas/AudioData.h"
#include "AudioDataLib/ByteSwapper.h"
#include "AudioDataLib/Error.h"

using namespace AudioDataLib;

//---------------------------------- Codec ----------------------------------

Codec::Codec()
{
}

/*virtual*/ Codec::~Codec()
{
}

//---------------------------------- RawAudioCodec ----------------------------------

RawAudioCodec::RawAudioCodec()
{
}

/*virtual*/ RawAudioCodec::~RawAudioCodec()
{
}

/*virtual*/ bool RawAudioCodec::Decode(ByteStream& inputStream, AudioData& audioOut, Error& error)
{
	uint64_t audioBufferSizeBytes = inputStream.GetSize();
	audioOut.SetAudioBufferSize(audioBufferSizeBytes);

	if (audioBufferSizeBytes != inputStream.ReadBytesFromStream(audioOut.GetAudioBuffer(), audioBufferSizeBytes))
	{
		error.Add(FormatString("Failed to read %d bytes of audio data from the stream.", audioBufferSizeBytes));
		return false;
	}

	return true;
}

/*virtual*/ bool RawAudioCodec::Encode(ByteStream& outputStream, const AudioData& audioIn, Error& error)
{
	uint64_t audioBufferSizeBytes = audioIn.GetAudioBufferSize();

	if (audioBufferSizeBytes != outputStream.WriteBytesToStream(audioIn.GetAudioBuffer(), audioBufferSizeBytes))
	{
		error.Add(FormatString("Failed to write %d bytes of audio data to the stream.", audioBufferSizeBytes));
		return false;
	}

	return true;
}

//---------------------------------- ByteSwappedAudioCodec ----------------------------------

ByteSwappedAudioCodec::ByteSwappedAudioCodec(ByteSwapper* byteSwapper)
{
	this->byteSwapper = byteSwapper;
}

/*virtual*/ ByteSwappedAudioCodec::~ByteSwappedAudioCodec()
{
}

/*virtual*/ bool ByteSwappedAudioCodec::Decode(ByteStream& inputStream, AudioData& audioOut, Error& error)
{
	uint64_t sampleSizeBytes = audioOut.GetFormat().BytesPerSample();

	uint64_t audioStreamSize = inputStream.GetSize();
	uint64_t numSamples = audioStreamSize / sampleSizeBytes;

	uint64_t audioBufferSize = numSamples * sampleSizeBytes;
	audioOut.SetAudioBufferSize(audioBufferSize);

	uint8_t* sampleBuffer = audioOut.GetAudioBuffer();

	for (uint64_t i = 0; i < numSamples; i++)
	{
		if (sampleSizeBytes != inputStream.ReadBytesFromStream(sampleBuffer, sampleSizeBytes))
		{
			error.Add(FormatString("Failed to read sample of size %d bytes from the stream.", sampleSizeBytes));
			return false;
		}

		this->byteSwapper->Resolve(sampleBuffer, sampleSizeBytes);

		sampleBuffer += sampleSizeBytes;
	}

	return true;
}

/*virtual*/ bool ByteSwappedAudioCodec::Encode(ByteStream& outputStream, const AudioData& audioIn, Error& error)
{
	error.Add("Not written yet.");
	return false;
}