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

bool FrequencyGraph::FromWaveForm(const WaveForm& waveForm, uint32_t numSamples, Error& error)
{
	this->Clear();

	ComplexVector sampleVector;

	double startTimeSeconds = waveForm.GetStartTime();
	double endTimeSeconds = waveForm.GetEndTime();
	double durationTimeSeconds = endTimeSeconds - startTimeSeconds;

	if ((numSamples & (numSamples - 1)) != 0)
	{
		error.Add("The given number of samples must be a power of 2.");
		return false;
	}

	for (uint32_t i = 0; i < numSamples; i++)
	{
		double timeSeconds = startTimeSeconds + (double(i) / double(numSamples - 1)) * durationTimeSeconds;
		double amplitude = waveForm.EvaluateAt(timeSeconds);
		sampleVector.Add(ComplexNumber(amplitude, 0.0));
	}

	ComplexVector complexVector;
	if (!complexVector.FFT(sampleVector, false, error))
		return false;

	for (uint32_t i = 0; i < complexVector.Size(); i++)
	{
		const ComplexNumber& complexNumber = complexVector[i];
		Plot plot;
		plot.strength = complexNumber.Magnitude();
		plot.phase = complexNumber.Angle();
		plot.frequency = double(i) / durationTimeSeconds;
		this->plotArray->push_back(plot);
	}

	return true;
}

bool FrequencyGraph::ToWaveForm(WaveForm& waveForm, Error& error) const
{
	error.Add("Not yet implimented.");
	return false;
}

void FrequencyGraph::GenerateSmootherGraph(FrequencyGraph& smootherGraph, double frequencyRadius) const
{
	smootherGraph.Clear();

	for (uint32_t i = 0; i < this->plotArray->size(); i++)
	{
		const Plot& plot = (*this->plotArray)[i];

		Plot smoothPlot = plot;
		uint32_t count = 1;

		for(uint32_t j = 1; j < this->plotArray->size(); j++)
		{
			bool countBumped = false;

			if (i >= j)
			{
				const Plot& localPlot = (*this->plotArray)[i - j];
				if (::abs(plot.frequency - localPlot.frequency) <= frequencyRadius)
				{
					smoothPlot.strength += localPlot.strength;
					count++;
					countBumped = true;
				}
			}

			if (i + j < this->plotArray->size())
			{
				const Plot& localPlot = (*this->plotArray)[i + j];
				if (::abs(plot.frequency - localPlot.frequency) <= frequencyRadius)
				{
					smoothPlot.strength += localPlot.strength;
					count++;
					countBumped = true;
				}
			}

			if (!countBumped)
				break;
		}

		smoothPlot.strength /= double(count);
		smootherGraph.plotArray->push_back(smoothPlot);
	}
}

double FrequencyGraph::EstimateFundamentalFrequency(double strengthThreshold /*= 26.0*/, double frequencyRadius /*= 8.0*/) const
{
	FrequencyGraph smootherGraph;
	this->GenerateSmootherGraph(smootherGraph, frequencyRadius);

	for (uint32_t i = 1; i < smootherGraph.plotArray->size() - 1; i++)
	{
		const Plot* plot[3] =
		{
			&(*smootherGraph.plotArray)[i - 1],
			&(*smootherGraph.plotArray)[i],
			&(*smootherGraph.plotArray)[i + 1]
		};

		if (plot[1]->strength >= strengthThreshold)
		{
			double derivativeA = (plot[1]->strength - plot[0]->strength) / (plot[1]->frequency - plot[0]->frequency);
			double derivativeB = (plot[2]->strength - plot[1]->strength) / (plot[2]->frequency - plot[1]->frequency);

			if (derivativeA > 0.0 && derivativeB < 0.0)
			{
				return plot[1]->frequency;
			}
		}
	}

	return 0.0;
}