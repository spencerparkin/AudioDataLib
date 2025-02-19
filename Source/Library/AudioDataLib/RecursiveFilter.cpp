#include "AudioDataLib/RecursiveFilter.h"

using namespace AudioDataLib;

//--------------------------------- RecursiveFilter ---------------------------------

RecursiveFilter::RecursiveFilter() : originalSignal(2, 0.5), filteredSignal(2, 0.5)
{
	this->params.scale = 1.0;
	this->params.timeDelaySeconds = 0.0;
}

/*virtual*/ RecursiveFilter::~RecursiveFilter()
{
}

/*virtual*/ double RecursiveFilter::EvaluateAt(double timeSeconds) const
{
	return this->filteredSignal.EvaluateAt(timeSeconds);
}

/*virtual*/ void RecursiveFilter::AddSample(const WaveForm::Sample& sample)
{
	this->originalSignal.AddSample(sample);
}

//--------------------------------- FeedBackwardCombFilter ---------------------------------

FeedBackwardCombFilter::FeedBackwardCombFilter()
{
}

/*virtual*/ FeedBackwardCombFilter::~FeedBackwardCombFilter()
{
}

/*virtual*/ void FeedBackwardCombFilter::AddSample(const WaveForm::Sample& sample)
{
	RecursiveFilter::AddSample(sample);

	WaveForm::Sample filteredSample;

	filteredSample.timeSeconds = sample.timeSeconds;
	filteredSample.amplitude = this->originalSignal.EvaluateAt(sample.timeSeconds) +
		this->params.scale * this->filteredSignal.EvaluateAt(sample.timeSeconds - this->params.timeDelaySeconds);

	this->filteredSignal.AddSample(filteredSample);
}

//--------------------------------- FeedForwardCombFilter ---------------------------------

FeedForwardCombFilter::FeedForwardCombFilter()
{
}

/*virtual*/ FeedForwardCombFilter::~FeedForwardCombFilter()
{
}

/*virtual*/ void FeedForwardCombFilter::AddSample(const WaveForm::Sample& sample)
{
	RecursiveFilter::AddSample(sample);

	WaveForm::Sample filteredSample;

	filteredSample.timeSeconds = sample.timeSeconds;
	filteredSample.amplitude = this->originalSignal.EvaluateAt(sample.timeSeconds) +
		this->params.scale * this->originalSignal.EvaluateAt(sample.timeSeconds - this->params.timeDelaySeconds);

	this->filteredSignal.AddSample(filteredSample);
}

//--------------------------------- AllPassFilter ---------------------------------

AllPassFilter::AllPassFilter()
{
}

/*virtual*/ AllPassFilter::~AllPassFilter()
{
}

/*virtual*/ void AllPassFilter::AddSample(const WaveForm::Sample& sample)
{
	RecursiveFilter::AddSample(sample);

	WaveForm::Sample filteredSample;

	filteredSample.timeSeconds = sample.timeSeconds;
	filteredSample.amplitude = this->params.scale * this->originalSignal.EvaluateAt(sample.timeSeconds) +
		this->originalSignal.EvaluateAt(sample.timeSeconds - this->params.timeDelaySeconds) -
		this->params.scale * this->filteredSignal.EvaluateAt(sample.timeSeconds - this->params.timeDelaySeconds);

	this->filteredSignal.AddSample(filteredSample);
}