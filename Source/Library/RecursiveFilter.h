#include "Function.h"
#include "WaveForm.h"

namespace AudioDataLib
{
	// See: https://ccrma.stanford.edu/~jos/pasp/Allpass_Two_Combs.html
	//      https://en.wikipedia.org/wiki/Comb_filter

	// Since we're based on the WaveForm class here, we don't have the best time-complexity, admittedly.
	// Treating the signal as a FIFO should be an O(1) operation, but here we occationally have to perform an O(N) trim operation.
	// Similarly, evaluating the signal at a given descrete time could be O(1), but we are using an O(log N) operation.
	// My view is that if it's fast enough, then I favor the simplicity that can sometimes prevail over the complications
	// that can arise in trying to be as fast as computerly-possible.  Comb and all-pass filters can sometimes be used as the
	// building-blocks of a reverberator, and that is the motivation here behind these classes.
	class AUDIO_DATA_LIB_API RecursiveFilter : public Function
	{
	public:
		RecursiveFilter();
		virtual ~RecursiveFilter();

		virtual double EvaluateAt(double timeSeconds) const override;

		virtual void AddSample(const WaveForm::Sample& sample);

		struct Params
		{
			double timeDelaySeconds;
			double scale;
		};

		void SetParams(const Params& param) { this->params = params; }
		const Params& GetParams() const { return this->params; }

		// This needs to be called occationally so that the original and filtered signals don't grow without bound.
		void TrimSignals(double timeSpanSeconds);

	protected:
		WaveForm originalSignal;
		WaveForm filteredSignal;

		Params params;
	};

	class AUDIO_DATA_LIB_API FeedBackwardCombFilter : public RecursiveFilter
	{
	public:
		FeedBackwardCombFilter();
		virtual ~FeedBackwardCombFilter();

		virtual void AddSample(const WaveForm::Sample& sample) override;
	};

	class AUDIO_DATA_LIB_API FeedForwardCombFilter : public RecursiveFilter
	{
	public:
		FeedForwardCombFilter();
		virtual ~FeedForwardCombFilter();

		virtual void AddSample(const WaveForm::Sample& sample) override;
	};

	class AUDIO_DATA_LIB_API AllPassFilter : public RecursiveFilter
	{
	public:
		AllPassFilter();
		virtual ~AllPassFilter();

		virtual void AddSample(const WaveForm::Sample& sample) override;
	};
}