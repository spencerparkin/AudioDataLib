#include "WaveForm.h"

using namespace AudioDataLib;

WaveForm::WaveForm()
{
	this->sampleArray = new std::vector<Sample>();
}

/*virtual*/ WaveForm::~WaveForm()
{
	delete this->sampleArray;
}

void WaveForm::ConvertFromAudioBuffer(const AudioData::Format& format, const uint8_t* audioBuffer, uint64_t audioBufferSize, uint16_t channel)
{
}

void WaveForm::ConvertToAudioBuffer(const AudioData::Format& format, uint8_t* audioBuffer, uint64_t audioBufferSize, uint16_t channel) const
{
}

double WaveForm::EvaluateAt(double timeSeconds) const
{
	const Sample* minSample = nullptr;
	const Sample* maxSample = nullptr;

	if (!this->FindSampleBounds(timeSeconds, minSample, maxSample))
		return 0.0;

	double lerpAlpha = (timeSeconds - minSample->timeSeconds) / (maxSample->timeSeconds - minSample->timeSeconds);
	double interpolatedAmplitude = minSample->amplitude + lerpAlpha * (maxSample->amplitude - minSample->amplitude);
	return interpolatedAmplitude;
}

bool WaveForm::FindSampleBounds(double timeSeconds, const Sample*& minSample, const Sample*& maxSample) const
{
	// TODO: Use an index if we have one.  Otherwise, do a linear search.

	for (uint32_t i = 0; i < this->sampleArray->size() - 1; i++)
	{
		minSample = &(*this->sampleArray)[i];
		maxSample = &(*this->sampleArray)[i + 1];

		if (minSample->timeSeconds <= timeSeconds && timeSeconds <= maxSample->timeSeconds)
			return true;
	}

	return false;
}

void WaveForm::GenerateIndex()
{
}

void WaveForm::SumTogether(const std::list<WaveForm*>& waveFormList)
{
	// find minimum time value.
	// walk the samples left to right, adding as we go.
}