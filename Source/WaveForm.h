#pragma once

#include "AudioData.h"

namespace AudioDataLib
{
	class AUDIO_DATA_LIB_API WaveForm
	{
	public:
		WaveForm();
		virtual ~WaveForm();

		void ConvertFromAudioBuffer(const AudioData::Format& format, const uint8_t* audioBuffer, uint64_t audioBufferSize, uint16_t channel);
		void ConvertToAudioBuffer(const AudioData::Format& format, uint8_t* audioBuffer, uint64_t audioBufferSize, uint16_t channel) const;

		// If an index exists, it can speed up evaluation.
		void GenerateIndex();

		double EvaluateAt(double timeSeconds) const;

		void SumTogether(const std::list<WaveForm*>& waveFormList);

		struct Sample
		{
			double timeSeconds;
			double amplitude;
		};

		bool FindSampleBounds(double timeSeconds, const Sample*& minSample, const Sample*& maxSample) const;

	protected:

		// We assume the samples are all in order according to time.
		std::vector<Sample>* sampleArray;
	};
}