#pragma once

#include "AudioData.h"

namespace AudioDataLib
{
	class AUDIO_DATA_LIB_API WaveForm
	{
	public:
		WaveForm();
		WaveForm(double totalSecondsOfSilence);
		virtual ~WaveForm();

		void ConvertFromAudioBuffer(const AudioData::Format& format, const uint8_t* audioBuffer, uint64_t audioBufferSize);
		void ConvertToAudioBuffer(const AudioData::Format& format, uint8_t*& audioBuffer, uint64_t& audioBufferSize) const;

		// If an index exists, it can speed up evaluation.
		void GenerateIndex();

		double EvaluateAt(double timeSeconds) const;

		void SumTogether(const std::list<WaveForm*>& waveFormList);
	};
}