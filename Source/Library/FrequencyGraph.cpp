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

// TODO: I've learned just now that the "dominant frequency" as I've defined it here,
//       is not necessarily (and probably *not* in most cases) the *percieved* pitch of
//       a given sound sample.  It is sometimes the GCD of the dominant frequencies
//       above some threshold, I think, and is called the residue pitch, but this is
//       not always the case.  Note that this is sometimes a pitch not present at all
//       as showing up as a spike in the graph!  A few minutes of trying to research
//       this topic has been overwelming.  I could try pinning down the perceived pitch
//       of all my samples and then look for patterns in the graph data, but I think
//       that would probably be a waste of time.  (Like me writing down prime numbers
//       and then seeing if I could find a pattern in those!)  In the end, I may just
//       have to determine the perceived pitches of all the font-data samples by ear,
//       because I just have no idea how to calculate it analytically or get it from
//       the font-data itself (which is ridiculous, by the way.)
double FrequencyGraph::FindDominantFrequency(double* dominantStrength /*= nullptr*/) const
{
	Plot dominantPlot{ 0.0, 0.0 };

	// I'm not sure why the graph is always symmetric about the center.  I really need a better handle on the math here.
	for (uint32_t i = 0; i < this->plotArray->size() / 2; i++)
	{
		const Plot& plot = (*this->plotArray)[i];
		if (plot.strength > dominantPlot.strength)
			dominantPlot = plot;
	}

	if (dominantStrength)
		*dominantStrength = dominantPlot.strength;

	return dominantPlot.frequency;
}