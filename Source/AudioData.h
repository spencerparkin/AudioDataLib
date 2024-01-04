#pragma once

#include "AudioCommon.h"

namespace AudioDataLib
{
	class AUDIO_DATA_LIB_API AudioData
	{
	public:
		struct AUDIO_DATA_LIB_API Format
		{
			uint16_t bitsPerSample;
			uint16_t numChannels;
			uint32_t samplesPerSecond;

			double BytesToSeconds(uint64_t numBytes) const;
			uint64_t BytesFromSeconds(double seconds) const;

			uint64_t RoundUpToNearestFrameMultiple(uint64_t numBytes) const;
			uint64_t RoundDownToNearestFrameMultiple(uint64_t numBytes) const;

			uint64_t BytesPerFrame() const;
			uint64_t SamplesPerFrame() const;
			uint64_t BytesPerSample() const;
			uint64_t FramesPerSecond() const;
			uint64_t BytesPerSecond() const;

			bool operator==(const Format& format) const;
			bool operator!=(const Format& format) const;
		};

		AudioData();
		virtual ~AudioData();

		Format& GetFormat() { return this->format; }
		const Format& GetFormat() const { return this->format; }
		
		uint8_t* GetAudioBuffer() { return this->audioBuffer; }
		const uint8_t* GetAudioBuffer() const { return this->audioBuffer; }

		uint64_t GetAudioBufferSize() const { return this->audioBufferSize; }
		void SetAudioBufferSize(uint64_t audioBufferSize);

		uint64_t GetNumSamples() const;
		uint64_t GetNumSamplesPerChannel() const;

	protected:
		Format format;
		uint8_t* audioBuffer;
		uint64_t audioBufferSize;
	};
}