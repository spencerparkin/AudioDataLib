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

		bool FromWaveForm(const WaveForm& waveForm, Error& error);
		bool ToWaveForm(WaveForm& waveForm, Error& error) const;

		const std::vector<double>& GetFrequencyArray() const { return *this->frequencyArray; }

		// TODO: Add methods here for picking out the spikes or dominant frequencies.

	protected:
		// Here, the indices (or offsets) into the array are the frequencies,
		// and the values of the array of the strengths of those frequencies
		// that we find in a given signal.
		std::vector<double>* frequencyArray;
	};
}