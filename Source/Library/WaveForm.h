#pragma once

#include "AudioData.h"
#include "ComplexNumber.h"
#include "Function.h"

namespace AudioDataLib
{
	class Error;

	// To be more specific, this is a descrete wave-form.
	class AUDIO_DATA_LIB_API WaveForm : public Function
	{
		friend class Index;

	public:
		WaveForm();
		virtual ~WaveForm();

		virtual double EvaluateAt(double timeSeconds) const override;

		uint64_t GetSizeBytes(const AudioData::Format& format, bool allChannels) const;

		bool ConvertFromAudioBuffer(const AudioData::Format& format, const uint8_t* audioBuffer, uint64_t audioBufferSize, uint16_t channel, Error& error);
		bool ConvertToAudioBuffer(const AudioData::Format& format, uint8_t* audioBuffer, uint64_t audioBufferSize, uint16_t channel, Error& error) const;

		struct Sample
		{
			double timeSeconds;
			double amplitude;	// TODO: This term isn't quite right.  What's a better term?  "Value"?
		};

		struct SampleBounds
		{
			const Sample* minSample;
			const Sample* maxSample;
			
			bool ContainsTime(double timeSeconds) const;
		};

		enum InterpolationMethod
		{
			LINEAR,
			CUBIC
		};

		void Clear();
		void Copy(const WaveForm* waveForm);
		void AddSample(const Sample& sample);
		void MakeSilence(double samplesPerSecond, double totalSeconds);
		void SumTogether(const std::list<WaveForm*>& waveFormList);
		bool FindTightestSampleBounds(double timeSeconds, SampleBounds& sampleBounds) const;
		bool Renormalize();
		void Scale(double scale);
		void Clamp(double minAmplitude, double maxAmplitude);
		bool Trim(double startTimeSeconds, double stopTimeSeconds, bool rebaseTime, Error& error);
		void SortSamples();
		double AverageSampleRate() const;
		double GetStartTime() const;
		double GetEndTime() const;
		double GetTimespan() const;
		uint64_t GetNumSamples() const;
		double GetMaxAmplitude() const;
		double GetMinAmplitude() const;
		double CalcAverageVolume() const;
		InterpolationMethod GetInterpolationMethod() const { return this->interpMethod; }
		void SetInterpolateionMethod(InterpolationMethod interpMethod) { this->interpMethod = interpMethod; }
		const std::vector<Sample>& GetSampleArray() const { return *this->sampleArray; }
		std::vector<Sample>& GetSampleArray() { return *this->sampleArray; }

	protected:

		// TODO: Add byte-swapping here.
		template<typename T>
		void CopyIntSampleToBuffer(uint8_t* sampleBuffer, double sampleNormalized) const
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

		// TODO: Add byte-swapping here.
		template<typename T>
		void CopyFloatSampleToBuffer(uint8_t* sampleBuffer, double sampleNormalized) const
		{
			T sampleFloat = T(sampleNormalized);
			::memcpy(sampleBuffer, (const void*)&sampleFloat, sizeof(float));
		}

		// TODO: Add byte-swapping here.
		template<typename T>
		double CopyIntSampleFromBuffer(const uint8_t* sampleBuffer)
		{
			constexpr int64_t maxSample = std::numeric_limits<T>::max();

			T sampleNarrow = 0;
			::memcpy(&sampleNarrow, sampleBuffer, sizeof(T));

			double sampleFloat = double(sampleNarrow);
			double sampleNormalized = sampleFloat / double(maxSample);
			return sampleNormalized;
		}

		// TODO: Add byte-swapping here.
		template<typename T>
		double CopyFloatSampleFromBuffer(const uint8_t* sampleBuffer)
		{
			T sampleFloat = 0.0f;
			::memcpy(&sampleFloat, sampleBuffer, sizeof(float));
			double sampleNormalized = double(sampleFloat);
			return sampleNormalized;
		}

		// We assume the samples are all in order according to time.
		std::vector<Sample>* sampleArray;
		InterpolationMethod interpMethod;
	};
}