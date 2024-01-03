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

			double BytesToSeconds(uint64_t numBytes) const;
			uint64_t BytesFromSeconds(double seconds) const;
			uint64_t RoundUpToNearestFrameMultiple(uint64_t numBytes) const;
			uint64_t RoundDownToNearestFrameMultiple(uint64_t numBytes) const;
			uint64_t BytesPerFrame() const;
			uint64_t BytesPerSample() const;
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