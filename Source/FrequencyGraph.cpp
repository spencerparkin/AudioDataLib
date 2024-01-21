#include "FrequencyGraph.h"
#include "WaveForm.h"
#include "ComplexVector.h"
#include "Error.h"

using namespace AudioDataLib;

FrequencyGraph::FrequencyGraph()
{
	this->plotArray = new std::vector<Plot>();
}

/*virtual*/ FrequencyGraph::~FrequencyGraph()
{
	delete this->plotArray;
}

void FrequencyGraph::Clear()
{
	this->plotArray->clear();
}

bool FrequencyGraph::FromWaveForm(const WaveForm& waveForm, Error& error)
{
	this->Clear();

	ComplexVector sampleVector;

	double startTimeSeconds = waveForm.GetStartTime();
	double endTimeSeconds = waveForm.GetEndTime();
	double durationTimeSeconds = endTimeSeconds - startTimeSeconds;

	uint32_t numSamples = 4096;

	for (uint32_t i = 0; i < numSamples; i++)
	{
		double timeSeconds = startTimeSeconds + (double(i) / double(numSamples - 1)) * durationTimeSeconds;
		double amplitude = waveForm.EvaluateAt(timeSeconds);
		sampleVector.Add(ComplexNumber(amplitude, 0.0));
	}

	ComplexVector complexVector;
	if (!complexVector.FFT(sampleVector, false, error))
		return false;

	double scale = 1.0;		// TODO: What should the scale be?

	for (uint32_t i = 0; i < complexVector.Size(); i++)
	{
		Plot plot;
		plot.strength = complexVector[i].Magnitude() * scale;
		plot.frequency = double(i) / 2.0;		// TODO: I really don't know why I have to divide by two.
		this->plotArray->push_back(plot);
	}

	return true;
}

bool FrequencyGraph::ToWaveForm(WaveForm& waveForm, Error& error) const
{
	error.Add("Not yet implimented.");
	return false;
}