#pragma once

#include "Common.h"

namespace AudioDataLib
{
	class Error;
	class WaveForm;

	// See: https://www.youtube.com/watch?v=spUNpyF58BY
	class AUDIO_DATA_LIB_API FrequencyGraph
	{
	public:
		FrequencyGraph();
		virtual ~FrequencyGraph();

		void Clear();

		bool FromWaveForm(const WaveForm& waveForm, uint32_t numSamples, Error& error);
		bool ToWaveForm(WaveForm& waveForm, Error& error) const;

		struct Plot
		{
			double frequency;
			double phase;
			double strength;
		};

		const std::vector<Plot>& GetPlotArray() const { return *this->plotArray; }

		double FindDominantFrequency(double* dominantStrength = nullptr) const;

	protected:
		
		std::vector<Plot>* plotArray;
	};
}