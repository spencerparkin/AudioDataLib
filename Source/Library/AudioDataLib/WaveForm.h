#pragma once

#include "AudioDataLib/FileDatas/AudioData.h"
#include "AudioDataLib/Math/ComplexNumber.h"
#include "AudioDataLib/Math/Function.h"

namespace AudioDataLib
{
	class Error;

	/**
	 * @brief This is the time-domain representation of a wave-form.
	 * 
	 * To be more specific, this is a descrete wave-form.  Since we represent the wave-form function as
	 * a sequence of plots (or samples) in the plane, this form is independent of format and sampling rate.
	 * Most math operations performed on wave-forms are done using this class, since it is format-ignostic
	 * or free from the considerations and limitations of any particular audio format.
	 * 
	 * A wave-form, therefore, is just a list of audio samples, each being a time and "amplitude" pair.
	 * Most operations assume that the samples are in chronological order.
	 */
	class AUDIO_DATA_LIB_API WaveForm : public Function
	{
		friend class Index;

	public:
		WaveForm();
		virtual ~WaveForm();

		/**
		 * Return the amplitude of the wave-form at the given time (in seconds.)
		 * Note that this has O(log N) time-complexity, which is usually reasonably,
		 * but clearly not as fast as O(1), which can be achieved with raw audio data.
		 */
		virtual double EvaluateAt(double timeSeconds) const override;

		/**
		 * Return the number of bytes required to represent this wave-form in the given format for 1 or all channels.
		 * 
		 * @param[in] format This is the format of the audio-data you're considering the wave-form being written in.
		 * @param[in] allChannels This is true if you're considering this or a similar wave-form being written to all channels of the audio data.
		 * @return This is the number of bytes required to lay down the wave-form in the desired format.
		 */
		uint64_t GetSizeBytes(const AudioData::Format& format, bool allChannels) const;

		/**
		 * Read the given raw audio data in the given format and generate from it a wave-form in the cartesian coordinate plane.
		 * 
		 * @param[in] format This is the format of the given audio data.
		 * @param[in] audioBuffer This is a buffer containing the raw audio data, presumably in the given format.
		 * @param[in] audioBufferSize This is the size in bytes of the given audio buffer.
		 * @param[in] channel This is which channel of the audio buffer to read as this wave-form.
		 * @return True is returned on success; false otherwise.
		 */
		bool ConvertFromAudioBuffer(const AudioData::Format& format, const uint8_t* audioBuffer, uint64_t audioBufferSize, uint16_t channel);

		/**
		 * Write this wave-form into the given audio buffer in the given format.
		 * 
		 * @param[in] format This is the format in which to write the audio data.
		 * @param[out] audioBuffer This is where to write the raw audio data in the given format.
		 * @param[in] audioBufferSize This is the size of the given audio buffer in bytes.
		 * @param[in] channel This is the channel of the audio buffer to write as this wave-form.
		 * @return True is returned on success; false otherwise.
		 */
		bool ConvertToAudioBuffer(const AudioData::Format& format, uint8_t* audioBuffer, uint64_t audioBufferSize, uint16_t channel) const;

		/**
		 * @brief A wave-form is simply a list of samples.
		 * 
		 * Each sample is just a time and amplitude pair.  Note that the term "amplitude" might not be quite right here.
		 * I'll change the term when I find a better one.
		 */
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
			LINEAR,		///< Use the line containing two different samples as the means of interpolation.
			CUBIC		///< Use the cubic polynomial fitting four different samples as the means of interpolation.  This is more expensive.
		};

		enum TrimSection
		{
			BEFORE,		///< Trim the beginning of the wave-form.
			AFTER		///< Trim the end of the wave-form.
		};

		/**
		 * Remove all samples from this wave-form.
		 */
		void Clear();

		/**
		 * Make this wave-form a copy of the given wave-form.
		 */
		void Copy(const WaveForm* waveForm);

		/**
		 * Add a sample to the end of this wave-form's list of samples.
		 */
		void AddSample(const Sample& sample);

		/**
		 * Make this wave-form a flat light representing silence for the given time period.
		 * 
		 * @param[in] samplesPerSecond This is the sampling rate, combined with the period, needed to determine how many samples to generate.
		 * @param[in] totalSeconds This is the desired duration (or time period) of silence to generate.
		 */
		void MakeSilence(double samplesPerSecond, double totalSeconds);

		/**
		 * Make this wave-form the sum of the given list of wave-forms.  A minimum amount of silence
		 * will be padded to all given wave-forms so that the resulting wave-form is of the length
		 * of the largest given wave-form.  The average sampling rate of all the wave-forms is used.
		 */
		void SumTogether(const std::list<WaveForm*>& waveFormList);

		/**
		 * Find the pair of adjacent samples that bounds the given time.
		 * This is an O(log N) operation, where N is the number of samples in the wave-form.
		 */
		bool FindTightestSampleBounds(double timeSeconds, SampleBounds& sampleBounds) const;

		/**
		 * Find the sample with the maximum amplitude and then divide this out from all the samples.
		 */
		bool Renormalize();

		/**
		 * Scale all samples by the given scale.  This does not change the timing of the samples.
		 */
		void Scale(double scale);

		/**
		 * Make sure that all samples are in the given range.
		 */
		void Clamp(double minAmplitude, double maxAmplitude);

		/**
		 * Trim the beginning and end of this wave-form.
		 * 
		 * @param[in] startTimeSeconds Remove everything up to this time.
		 * @param[in] stopTimeSeconds Remove everything after this time.
		 * @param[in] rebaseTime If true, once the trim has occurred, shift the wave-form back so that it starts at origin.
		 * @return False is returned if an error occurrs; true otherwise.
		 */
		bool Trim(double startTimeSeconds, double stopTimeSeconds, bool rebaseTime);

		/**
		 * Try to perform a trim that's sometimes faster than that perfomed by the Trim function.
		 * 
		 * @param[in] timeSeconds This is the point at which everything is trimmed before or after.
		 * @param[in] trimeSecion This indicates whether everything is trimmed before or after the given time point.
		 */
		void QuickTrim(double timeSeconds, TrimSection trimSection);

		/**
		 * Make sure all samples in the wave-form's list of such is sorted chronologically.
		 */
		void SortSamples();

		/**
		 * Add some silence to the end of this wave-form.
		 * 
		 * @param[in] desiredDurationSeconds This is the amount of silence to add in seconds.
		 * @param[in] sampleRate This, combined with the duration, indicates how many samples to add.
		 * @return The number of samples added is returned.
		 */
		uint64_t PadWithSilence(double desiredDurationSeconds, double sampleRate);

		/**
		 * Calculate and return the average sampling rate of this wave-form.
		 * The samples in a wave-form don't necessarily have a uniform distribution, but if they did,
		 * then this would return the sampling rate of the wave-form.
		 */
		double AverageSampleRate() const;

		/**
		 * Return the time location of the first sample in this wave-form's list of samples.
		 */
		double GetStartTime() const;

		/**
		 * Return the time location of the last sample in this wave-form's list of samples.
		 */
		double GetEndTime() const;

		/**
		 * Return the difference in time between the first and last sample of this wave-form.
		 */
		double GetTimespan() const;

		/**
		 * Return the number of samples in this wave-form's list of samples.
		 */
		uint64_t GetNumSamples() const;

		/**
		 * Find and return the most positive amplitude found in this wave-form's list of samples.
		 */
		double GetMaxAmplitude() const;

		/**
		 * Find and return the most negative amplitude found in this wave-form's list of samples.
		 */
		double GetMinAmplitude() const;

		/**
		 * This function is not very well defined.  I may remove it at some point.
		 */
		double CalcAverageVolume() const;

		/**
		 * Return true if and only if the given time is within this wave-form's time-span.
		 */
		bool ContainsTime(double timeSeconds) const;

		/**
		 * Return the interpolation method being used to evaluate this wave-form at any particular time within its time-span.
		 */
		InterpolationMethod GetInterpolationMethod() const { return this->interpMethod; }

		/**
		 * Set the interpolation method used to evaluate this wave-form at any particular time within its time-span.
		 */
		void SetInterpolateionMethod(InterpolationMethod interpMethod) { this->interpMethod = interpMethod; }

		/**
		 * Get read-only access to this wave-form's sample array.
		 */
		const std::vector<Sample>& GetSampleArray() const { return this->sampleArray; }

		/**
		 * Get read/write access to this wave-form's sample array.
		 */
		std::vector<Sample>& GetSampleArray() { return this->sampleArray; }

		/**
		 * Evaluate this wave-form at the given time, between the two given samples, using the set interpolation method.
		 * This function is typically used internally by the EvaluateAt method.  Note that the given time is typically
		 * somewhere in the time-span between the two given samples, but this is not a requirement.  Also, the two given
		 * samples are typically adjacent, but this is also not a requirement.
		 * 
		 * @param[in] sampleBounds This is the pair of sample between which to perform the interpolation.
		 * @param[in] timeSeconds This is the time (in seconds) where the wave-form function is to be evaluated.
		 * @param[out] interpolatedSample This is the sample calculated as an interpolation between the two given samples.
		 */
		void Interpolate(const SampleBounds& sampleBounds, double timeSeconds, Sample& interpolatedSample) const;

	protected:

		// TODO: Add byte-swapping here.
		template<typename T>
		void CopyIntSampleToBuffer(uint8_t* sampleBuffer, double sampleNormalized) const
		{
			constexpr int64_t minSample = std::numeric_limits<T>::min();
			constexpr int64_t maxSample = std::numeric_limits<T>::max();

			int64_t sampleWide = int64_t(sampleNormalized * double(maxSample));

			if (sampleWide > maxSample)
				sampleWide = maxSample;
			if (sampleWide < minSample)
				sampleWide = minSample;

			T sampleNarrow = T(sampleWide);
			::memcpy(sampleBuffer, (const void*)&sampleNarrow, sizeof(T));
		}

		// TODO: Add byte-swapping here.
		template<typename T>
		void CopyUIntSampleToBuffer(uint8_t* sampleBuffer, double sampleNormalized) const
		{
			constexpr uint64_t maxSample = std::numeric_limits<T>::max();

			uint64_t sampleWide = uint64_t((sampleNormalized + 1.0) * double(maxSample) / 2.0);

			if (sampleWide > maxSample)
				sampleWide = maxSample;

			T sampleNarrow = T(sampleWide);
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
		double CopyUIntSampleFromBuffer(const uint8_t* sampleBuffer)
		{
			constexpr uint64_t maxSample = std::numeric_limits<T>::max();

			T sampleNarrow = 0;
			::memcpy(&sampleNarrow, sampleBuffer, sizeof(T));

			double sampleFloat = double(sampleNarrow);
			double sampleNormalized = (sampleFloat / double(maxSample)) * 2.0 - 1.0;
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
		std::vector<Sample> sampleArray;
		InterpolationMethod interpMethod;
	};

	/**
	 * If you want to continuously add samples to a wave-form, but you also
	 * don't want it to grow without bound, then this class may be helpful.
	 */
	class AUDIO_DATA_LIB_API WaveFormStream : public Function
	{
	public:
		WaveFormStream(uint32_t maxWaveForms, double maxWaveFormSizeSeconds);
		virtual ~WaveFormStream();

		virtual double EvaluateAt(double timeSeconds) const override;

		void AddSample(const WaveForm::Sample& sample);
		void Clear();
		double GetDurationSeconds() const;
		double GetStartTimeSeconds() const;
		double GetEndTimeSeconds() const;
		bool AnyAudibleSampleFound() const;

	protected:
		
		uint32_t maxWaveForms;
		double maxWaveFormSizeSeconds;
		std::list<WaveForm*> waveFormList;
	};
}