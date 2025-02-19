#pragma once

#include "AudioDataLib/Common.h"
#include "AudioDataLib/ByteStream.h"

namespace AudioDataLib
{
	/**
	 * @brief Provide a convenient way to fire and forget sound FX.
	 * 
	 * What this class is trying to be is a general purpose way of mixing and/or converting audio.
	 * It can be used for synthesis or real-time purposes.  Note that to use this class in a
	 * thread-safe manner (typically for real-time playback of audio), you should use the
	 * ThreadSafeAudioStream class instead of just the regular AudioStream class for the audio output.
	 * 
	 * The real-time case is all about feeding an audio device.  The synthesis case is about converting
	 * audio from one format to another, or mixing audio.
	 */
	class AUDIO_DATA_LIB_API AudioSink
	{
	public:
		AudioSink();
		virtual ~AudioSink();

		void Clear();

		/**
		 * Make sure that an amount of audio data equivilant to the given amount
		 * of time is available for consumption in our audio output stream.
		 * This will produce silence in the audio output if necessary.  Note that
		 * too much buffered time will create latency when new audio clips are fired.
		 * But too little buffered can cause audio drop-outs due to a starved device.
		 */
		void GenerateAudio(double desiredSecondsAvailable, double minSecondsAddedPerMix);
	
		/**
		 * Start playing/mixing the given audio stream immediately.
		 */
		void AddAudioInput(std::shared_ptr<AudioStream> audioStream);

		std::shared_ptr<AudioStream> GetAudioOutput() { return this->audioStreamOut; }
		void SetAudioOutput(std::shared_ptr<AudioStream> audioStreamOut);

		/**
		 * Return the current number of simulatneously playing audio stream.
		 */
		uint32_t GetAudioInputCount() const { return (uint32_t)this->audioStreamInArray.size(); }

	protected:

		// TODO: Byte swapping?
		template<typename T>
		T CalcNetSample()
		{
			if constexpr (std::is_signed<T>())
			{
				int64_t netSampleWide = 0;

				for (auto& audioStreamIn : this->audioStreamInArray)
				{
					T sampleNarrow = 0;
					if (audioStreamIn->ReadType<T>(&sampleNarrow))
						netSampleWide += int64_t(sampleNarrow);
				}

				constexpr int64_t minSample = std::numeric_limits<T>::min();
				constexpr int64_t maxSample = std::numeric_limits<T>::max();

				if (netSampleWide > maxSample)
					netSampleWide = maxSample;
				if (netSampleWide < minSample)
					netSampleWide = minSample;

				T netSampleNarrow = T(netSampleWide);
				return netSampleNarrow;
			}
			else
			{
				// TODO: This has not yet been tested.  Test it.

				int64_t netSampleSignedWide = 0;
				constexpr int64_t delta = (1 << (sizeof(T) * 8 - 1)) - 1;

				for (auto& audioStreamIn : this->audioStreamInArray)
				{
					T sampleUnsignedNarrow = 0;
					if (audioStreamIn->ReadType<T>(&sampleUnsignedNarrow))
					{
						int64_t sampleSignedWide = int64_t(sampleUnsignedNarrow) - delta;
						netSampleSignedWide += sampleSignedWide;
					}
				}

				constexpr int64_t maxSample = 1 << (sizeof(T) * 8 - 1);
				constexpr int64_t minSample = -maxSample + 1;

				if (netSampleSignedWide > maxSample)
					netSampleSignedWide = maxSample;
				if (netSampleSignedWide < minSample)
					netSampleSignedWide = minSample;

				uint64_t netSampleUnsignedWide = uint64_t(netSampleSignedWide + delta);
				T netSampleUnsignedNarrow = T(netSampleUnsignedWide);
				return netSampleUnsignedNarrow;
			}
		}

		// TODO: Byte swapping?
		template<>
		float CalcNetSample()
		{
			double netSampleWide = 0.0;

			for (auto& audioStreamIn : this->audioStreamInArray)
			{
				float sampleNarrow = 0.0f;
				if (audioStreamIn->ReadType<float>(&sampleNarrow))
					netSampleWide += double(sampleNarrow);
			}

			netSampleWide = ADL_CLAMP(netSampleWide, -1.0, 1.0);
			float netSampleNarrow = float(netSampleWide);
			return netSampleNarrow;
		}

		// TODO: Byte swapping?
		template<>
		double CalcNetSample()
		{
			double netSample = 0.0;
			double sample = 0.0;
			for(auto& audioStreamIn : this->audioStreamInArray)
				if (audioStreamIn->ReadType<double>(&sample))
					netSample += sample;

			netSample = ADL_CLAMP(netSample, -1.0, 1.0);
			return netSample;
		}

		std::vector<std::shared_ptr<AudioStream>> audioStreamInArray;
		std::shared_ptr<AudioStream> audioStreamOut;
	};
}