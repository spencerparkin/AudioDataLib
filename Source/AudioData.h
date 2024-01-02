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
			int bitsPerSample;
			int numChannels;
			int sampleRateBitsPerSecond;
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