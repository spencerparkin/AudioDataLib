#include "AudioDataLib/Math/Function.h"
#include "AudioDataLib/WaveForm.h"

namespace AudioDataLib
{
	/**
	 * @brief This is a function that uses itself recursively to filter audio in the time-domain.
	 *
	 * The resursion involved here is not that in the computer-science sense, but in the sense that
	 * results of previous evaluations of the function are used to produce current evaluations.
	 * 
	 * Since we're based on the WaveForm class here, we don't have the best time-complexity, admittedly.
	 * Evaluating the signal at a given descrete time could be O(1), but we are using an O(log N) operation.
	 * My view is that if it's fast enough, then I favor the simplicity that can sometimes prevail over the complications
	 * that can arise in trying to be as fast as computerly-possible.  Comb and all-pass filters can sometimes be used as the
	 * building-blocks of a reverberator, and that is the motivation here behind this class and its derivatives.
	 */
	class AUDIO_DATA_LIB_API RecursiveFilter : public Function
	{
		// See: https://ccrma.stanford.edu/~jos/pasp/Allpass_Two_Combs.html
		//      https://en.wikipedia.org/wiki/Comb_filter
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

		void SetParams(const Params& params) { this->params = params; }
		const Params& GetParams() const { return this->params; }

	protected:
		WaveFormStream originalSignal;
		WaveFormStream filteredSignal;

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