#pragma once

#include "Common.h"

namespace AudioDataLib
{
	class Error;
	class WaveForm;

	/**
	 * @brief This class is the product of performing an FFT on a given WaveForm instance.
	 * 
	 * While audio data in the time-domain is represented by the WaveForm class, this class
	 * represents such data in the frequency-domain.
	 */
	class AUDIO_DATA_LIB_API FrequencyGraph
	{
		// See: https://www.youtube.com/watch?v=spUNpyF58BY
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

		void GenerateSmootherGraph(FrequencyGraph& smootherGraph, double frequencyRadius) const;

		/**
		 * Analyze this graph to determine the fundamental frequency, assuming
		 * the wave-form that was used was a monophonic signal.  Note that the
		 * fundamental frequency is not always the perceived pitch, but it
		 * often is.
		 */
		double EstimateFundamentalFrequency(double strengthThreshold = 35.0) const;

	protected:
		
		std::vector<Plot>* plotArray;
	};
}