#pragma once

#include "AudioDataLib/Common.h"

namespace AudioDataLib
{
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

		/**
		 * Remove all plots from this graph's list of such.
		 */
		void Clear();

		/**
		 * Generate a frequency graph from the given wave-form using an FFT.
		 * 
		 * @param waveForm This is the wave-form upon which to perform the FFT.
		 * @param numSamples This is the number of samples to evaluate from the given wave-form and how many plots in the graph that will be generated.  It must be a power of 2.
		 * @return True is returned on success; false otherwise.
		 */
		bool FromWaveForm(const WaveForm& waveForm, uint32_t numSamples);

		/**
		 * Generate a wave-form from this frequency-graph using an inverse FFT.
		 * This function has not yet been test or even written, but it's on my list of things to do.
		 * I think that additional parameters may be needed to make it work.
		 */
		bool ToWaveForm(WaveForm& waveForm) const;

		/**
		 * The frequency graph is a list of Plot structures.  Each is a frequency paired with a strength and phase pair.
		 * When a signal is broken down into frequency components, the strength (or amplitude) and phase of each component is needed
		 * to fully scribe the component, which can be thought of as a single sine-wave.  The original signal can be thought of
		 * as the sum of all these different components.  A study of FFTs gives additional insights into the matter.
		 */
		struct Plot
		{
			double frequency;			///< This is the X-axis coordinate of a plot.
			double phase;				///< This is the shift value of the sine-wave component that would have the associated frequency and strength (amplitude.)
			double strength;			///< This is typically the Y-axis coordinate of a plot, and tells us how priminant the associated frequency is.
		};

		/**
		 * Get read-only access to this graph's plot array.
		 */
		const std::vector<Plot>& GetPlotArray() const { return this->plotArray; }

		/**
		 * This creates a smoother-looking version of this frequency graph that might be easier to look at.
		 * It may have no real practical use.
		 */
		void GenerateSmootherGraph(FrequencyGraph& smootherGraph, double frequencyRadius) const;

		/**
		 * Analyze this graph to determine the fundamental frequency, assuming
		 * the wave-form that was used was a monophonic signal.  Note that the
		 * fundamental frequency is not always the perceived pitch, but it
		 * often is.
		 */
		double EstimateFundamentalFrequency(double strengthThreshold = 35.0) const;

	protected:
		
		std::vector<Plot> plotArray;
	};
}