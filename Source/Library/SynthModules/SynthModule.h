#pragma once

#include "Common.h"

namespace AudioDataLib
{
	class WaveForm;
	class Error;

	// These are the building-blocks of sound synthesis.
	class AUDIO_DATA_LIB_API SynthModule
	{
	public:
		SynthModule();
		virtual ~SynthModule();

		virtual bool GenerateSound(double durationSeconds, double samplesPerSecond, WaveForm& waveForm, Error& error) = 0;
		virtual bool CantGiveAnymoreSound();		// TODO: Rename this so I'm not thinking of a double-negative everywhere.
	};

	// TODO: It would be really interesting to try to provide a ReverbModule derivative.
	//       See: https://github.com/radoslawregula/reverb-algorithms
	//       This source shows some work someone did in Python.  It's extremely cryptic, in my opinion.
	//       I've also added a PDF to my repo: Moore-Reverb-CMJ-1979.pdf.  This explains some of the math involved
	//       in various types of reverb algorithms, but using graphs and some notation that, again, seems very
	//       cryptic to me.  There seems to be a language behind all this DSP (digital signal processing) stuff
	//       that is completely alien to me.  I can kind-of glimpse some of the ideas, like taking a signal
	//       and feeding it back on itself with a delay and scale.  Perhaps a good first step is just to see
	//       if I can make an echo effect, then to see if I can make a single comb-filter, then a single all-pass
	//       filter, etc.  Instances of these latter two are combined in series and in parallel to make one of the
	//       paper's proposed reverberators.  Filters in general are something of which I lack greatly in knowledge.
	//       For example, how does a high-pass or low-pass filter work?  Do these require an FFT and an inverse FFT?
	//       I have no idea.
}