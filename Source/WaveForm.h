#pragma once

#include "AudioData.h"

namespace AudioDataLib
{
	// TODO: Might be interesting to one day be able to convert to/from another form using FFT.
	class AUDIO_DATA_LIB_API WaveForm
	{
		friend class Index;

	public:
		WaveForm();
		virtual ~WaveForm();

		uint64_t GetSizeBytes(const AudioData::Format& format, bool allChannels) const;

		bool ConvertFromAudioBuffer(const AudioData::Format& format, const uint8_t* audioBuffer, uint64_t audioBufferSize, uint16_t channel, std::string& error);
		bool ConvertToAudioBuffer(const AudioData::Format& format, uint8_t* audioBuffer, uint64_t audioBufferSize, uint16_t channel, std::string& error) const;

		struct Sample
		{
			double timeSeconds;
			double amplitude;
		};

		struct SampleBounds
		{
			const Sample* minSample;
			const Sample* maxSample;
			
			bool ContainsTime(double timeSeconds) const;
		};

		void Clear();
		void Copy(const WaveForm* waveForm);
		double EvaluateAt(double timeSeconds) const;
		void SumTogether(const std::list<WaveForm*>& waveFormList);
		bool FindTightestSampleBounds(double timeSeconds, SampleBounds& sampleBounds) const;
		bool Renormalize();
		void Scale(double scale);
		void Clamp(double minAmplitude, double maxAmplitude);
		double AverageSampleRate() const;
		double GetStartTime() const;
		double GetEndTime() const;
		double GetTimespan() const;
		uint64_t GetNumSamples() const;
		double GetMaxAmplitude() const;
		double GetMinAmplitude() const;

	protected:

		// TODO: Add byte-swapping here.
		template<typename T>
		void CopySampleToBuffer(uint8_t* sampleBuffer, double sampleNormalized) const
		{
			constexpr int64_t minSample = std::numeric_limits<T>::min();
			constexpr int64_t maxSample = std::numeric_limits<T>::max();

			int64_t sampleWide = int64_t(sampleNormalized * double(maxSample));

			int64_t sampleWideClamped = sampleWide;
			if (sampleWideClamped > maxSample)
				sampleWideClamped = maxSample;
			if (sampleWideClamped < minSample)
				sampleWideClamped = minSample;

			T sampleNarrow = T(sampleWideClamped);
			::memcpy(sampleBuffer, (const void*)&sampleNarrow, sizeof(T));
		}

		// TODO: Byte-swapping?
		void CopyFloatSampleToBuffer(uint8_t* sampleBuffer, double sampleNormalized) const
		{
			constexpr double minSample = (double)std::numeric_limits<float>::min();
			constexpr double maxSample = (double)std::numeric_limits<float>::max();

			double sampleWide = sampleNormalized * maxSample;

			double sampleWideClamped = sampleWide;
			if (sampleWideClamped > maxSample)
				sampleWideClamped = maxSample;
			if (sampleWideClamped < minSample)
				sampleWideClamped = minSample;

			float sampleNarrow = float(sampleWideClamped);
			::memcpy(sampleBuffer, (const void*)&sampleNarrow, sizeof(float));
		}

		// TODO: Add byte-swapping here.
		template<typename T>
		double CopySampleFromBuffer(const uint8_t* sampleBuffer)
		{
			constexpr int64_t maxSample = std::numeric_limits<T>::max();

			T sampleNarrow = 0;
			::memcpy(&sampleNarrow, sampleBuffer, sizeof(T));

			double sampleFloat = double(sampleNarrow);
			double sampleNormalized = sampleFloat / double(maxSample);
			return sampleNormalized;
		}

		// TODO: Byte-swapping?
		double CopyFloatSampleFromBuffer(const uint8_t* sampleBuffer)
		{
			float sampleFloat = 0.0;
			::memcpy(&sampleFloat, sampleBuffer, sizeof(float));
			double sampleNormalized = double(sampleFloat) / double(std::numeric_limits<float>::max());
			return sampleNormalized;
		}

		// We assume the samples are all in order according to time.
		std::vector<Sample>* sampleArray;
	};
}