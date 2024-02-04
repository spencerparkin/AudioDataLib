#pragma once

#include "Common.h"

namespace AudioDataLib
{
	class WaveForm;
	class Error;

	// These are the building-blocks of sound synthesis.  No matter what
	// the module derivative does (e.g., filter, oscillate, mix, whatever),
	// it supports the fundamental operation of providing a given time-span
	// of sound at a given pitch.  To do that, it may need to first request
	// such a thing from dependent modules before it can do its work.  Note
	// that a module is responsible for maintaining its notion of continuity
	// with the provided wave-form.  Also, a mixer module might ignore the
	// pitch parameter, because it's mixing dependent modules at various
	// pitches.  A single note might be turned on or off by adding or removing a
	// whole module configuration (sub-tree) to/from a mixer module.  Collectively,
	// this subtree configuration makes up the sound characteristics of the
	// instrument, along with how each module is individually configured.
	// Also notice that modules work primarily in the wave-form space.  (A
	// filter module may also work in the frequency domain space too.)
	// All calculations are done in this space, and we only convert to a format
	// that the sound-hardware wants at the last moment when it's time to
	// feed the audio call-back.  Lastly, a module might occationally ask its
	// dependents for more time than of which it is being requested, then buffer
	// the data, because it needs to operate on a slightly larger set of data.
	// I'll try not to do this if it really isn't necessary, for the sake of
	// simplicity.
	class AUDIO_DATA_LIB_API SynthModule
	{
	public:
		SynthModule();
		virtual ~SynthModule();

		virtual bool GenerateSound(double durationSeconds, double samplesPerSecond, WaveForm& waveForm, Error& error) = 0;
		virtual bool CantGiveAnymoreSound();
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
	//       For example, how does a high-pass or low-pass filter work?  Does these require an FFT and an inverse FFT?
	//       I have no idea.
}