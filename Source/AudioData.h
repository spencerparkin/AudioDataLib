#pragma once

#include "AudioCommon.h"
#include "ByteStream.h"

namespace AudioDataLib
{
	class AUDIO_DATA_LIB_API AudioData
	{
	public:
		struct Format
		{
			uint16_t bitsPerSample;
			uint16_t numChannels;
			uint32_t sampleRateBitsPerSecond;
		};

		AudioData();
		AudioData(Format format, ByteStream* audioStream);
		virtual ~AudioData();

		Format& GetFormat() { return this->format; }
		ByteStream* GetAudioStream() { return this->audioStream; }

	protected:
		Format format;
		ByteStream* audioStream;
	};
}