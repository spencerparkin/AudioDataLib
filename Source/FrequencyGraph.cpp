#include "FrequencyGraph.h"
#include "WaveForm.h"
#include "ComplexVector.h"
#include "Error.h"

using namespace AudioDataLib;

FrequencyGraph::FrequencyGraph()
{
	this->frequencyArray = new std::vector<double>();
}

/*virtual*/ FrequencyGraph::~FrequencyGraph()
{
	delete this->frequencyArray;
}

void FrequencyGraph::Clear()
{
	this->frequencyArray->clear();
}

bool FrequencyGraph::FromWaveForm(const WaveForm& waveForm, Error& error)
{
	this->Clear();

	ComplexVector sampleVector;

	double startTimeSeconds = waveForm.GetStartTime();
	double endTimeSeconds = waveForm.GetEndTime();

	uint32_t numSamples = 4096;

	for (uint32_t i = 0; i < numSamples; i++)
	{
		double timeSeconds = startTimeSeconds + (double(i) / double(numSamples - 1)) * (endTimeSeconds - startTimeSeconds);
		double amplitude = waveForm.EvaluateAt(timeSeconds);
		sampleVector.Add(ComplexNumber(amplitude, 0.0));
	}

	ComplexVector frequencyVector;
	if (!frequencyVector.FFT(sampleVector, false, error))
		return false;

	double scale = 10.0;		// TODO: What should the scale be?

	for (uint32_t i = 0; i < frequencyVector.Size(); i++)
	{
		double frequencyStrength = frequencyVector[i].Magnitude() * scale;
		this->frequencyArray->push_back(frequencyStrength);
	}

	return true;
}

bool FrequencyGraph::ToWaveForm(WaveForm& waveForm, Error& error) const
{
	error.Add("Not yet implimented.");
	return false;
}