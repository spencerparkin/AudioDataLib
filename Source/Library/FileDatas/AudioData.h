#pragma once

#include "FileData.h"

namespace AudioDataLib
{
	class AUDIO_DATA_LIB_API AudioData : public FileData
	{
	public:
		struct AUDIO_DATA_LIB_API Format
		{
			enum SampleType
			{
				SIGNED_INTEGER,
				FLOAT
			};

			uint16_t bitsPerSample;
			uint16_t numChannels;
			uint32_t framesPerSecond;
			SampleType sampleType;

			double BytesToSeconds(uint64_t numBytes) const;
			uint64_t BytesFromSeconds(double seconds) const;

			double BytesPerChannelToSeconds(uint64_t numBytes) const;
			uint64_t BytesPerChannelFromSeconds(double seconds) const;

			uint64_t RoundUpToNearestFrameMultiple(uint64_t numBytes) const;
			uint64_t RoundDownToNearestFrameMultiple(uint64_t numBytes) const;

			uint64_t BytesPerFrame() const;
			uint64_t SamplesPerFrame() const;
			uint64_t BytesPerChannel(uint64_t audioBufferSize) const;
			uint64_t BytesPerSample() const;
			uint64_t SamplesPerSecond() const;
			uint64_t SamplesPerSecondPerChannel() const;
			uint64_t BytesPerSecond() const;
			uint64_t BytesPerSecondPerChannel() const;

			bool operator==(const Format& format) const;
			bool operator!=(const Format& format) const;
		};

		AudioData();
		virtual ~AudioData();

		virtual void DumpInfo(FILE* fp) const override;
		virtual void DumpCSV(FILE* fp) const override;
		virtual FileData* Clone() const override;

		// The main purpose of these is to make sure that memory is created or freed in the proper heap.
		// They can also be useful when writing template functions.
		static AudioData* Create();
		static void Destroy(AudioData* audioData);

		Format& GetFormat() { return this->format; }
		const Format& GetFormat() const { return this->format; }
		void SetFormat(const Format& format) { this->format = format; }
		
		uint8_t* GetAudioBuffer() { return this->audioBuffer; }
		const uint8_t* GetAudioBuffer() const { return this->audioBuffer; }

		uint64_t GetAudioBufferSize() const { return this->audioBufferSize; }
		void SetAudioBufferSize(uint64_t audioBufferSize);

		uint64_t GetNumSamples() const;
		uint64_t GetNumSamplesPerChannel() const;
		uint64_t GetNumFrames() const;

		double GetTimeSeconds() const;

	protected:
		Format format;
		uint8_t* audioBuffer;
		uint64_t audioBufferSize;
	};
}