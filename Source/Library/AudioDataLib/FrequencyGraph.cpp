#include "AudioDataLib/FrequencyGraph.h"
#include "AudioDataLib/WaveForm.h"
#include "AudioDataLib/Math/ComplexVector.h"
#include "AudioDataLib/ErrorSystem.h"

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

bool FrequencyGraph::FromWaveForm(const WaveForm& waveForm, uint32_t numSamples)
{
	this->Clear();

	ComplexVector sampleVector;

	double startTimeSeconds = waveForm.GetStartTime();
	double endTimeSeconds = waveForm.GetEndTime();
	double durationTimeSeconds = endTimeSeconds - startTimeSeconds;

	if ((numSamples & (numSamples - 1)) != 0)
	{
		ErrorSystem::Get()->Add("The given number of samples must be a power of 2.");
		return false;
	}

	for (uint32_t i = 0; i < numSamples; i++)
	{
		double timeSeconds = startTimeSeconds + (double(i) / double(numSamples - 1)) * durationTimeSeconds;
		double amplitude = waveForm.EvaluateAt(timeSeconds);
		sampleVector.Add(ComplexNumber(amplitude, 0.0));
	}

	ComplexVector complexVector;
	if (!complexVector.FFT(sampleVector, false))
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

bool FrequencyGraph::ToWaveForm(WaveForm& waveForm) const
{
	ErrorSystem::Get()->Add("Not yet implimented.");
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

double FrequencyGraph::EstimateFundamentalFrequency(double strengthThreshold /*= 35.0*/) const
{
	constexpr uint32_t maxScaleUpAttempts = 5;
	double scale = 1.0;
	std::vector<Plot> scaledPlotArray;

	for (uint32_t i = 0; i < maxScaleUpAttempts; i++)
	{
		for (const Plot& plot : *this->plotArray)
		{
			Plot scaledPlot = plot;
			scaledPlot.strength *= scale;
			scaledPlotArray.push_back(scaledPlot);
		}

		for (uint32_t j = 1; j < scaledPlotArray.size() - 1; j++)
		{
			const Plot* plot[3] =
			{
				&scaledPlotArray[j - 1],
				&scaledPlotArray[j],
				&scaledPlotArray[j + 1]
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

		scale += 0.5;
		scaledPlotArray.clear();
	}

	return 0.0;
}